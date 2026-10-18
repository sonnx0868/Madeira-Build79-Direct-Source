#!/usr/bin/env python3
"""Compare saved compute shaders with the shipping LLVM and Apple's compiler.

Input shaders remain in a temporary directory. Only counts, timings and compiler
statistics are deployed. This experiment does not produce an IPA or measure FPS.
"""
import gzip, json, os, pathlib, plistlib, re, shutil, struct, subprocess, tempfile, urllib.parse, urllib.request, zipfile

root = pathlib.Path(__file__).resolve().parents[1]
out = pathlib.Path(os.environ['BITRISE_DEPLOY_DIR'])
llvm = root / 'toolchains/llvm15-macos'

for tool in ['metal-tt', 'metal-objdump', 'metal-config', 'metal-source', 'metal-lipo']:
    found = subprocess.run(['xcrun', '--find', tool], capture_output=True, text=True)
    if found.returncode: continue
    help_result = subprocess.run(['xcrun', tool, '-help' if tool=='metal-tt' else '--help'], capture_output=True, text=True, timeout=30)
    (out/(tool+'-help.txt')).write_text(help_result.stdout + help_result.stderr)
    print('Offline Metal tool available:', tool, flush=True)
for page in ['metal-pipelines-script','metal-tt','metal-objdump']:
    man = subprocess.run(['man',page], capture_output=True, text=True, env=dict(os.environ,MANPAGER='cat'), timeout=30)
    (out/(page+'-manual.txt')).write_text(man.stdout + man.stderr)
sdk = pathlib.Path(subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip())
tt = pathlib.Path(subprocess.check_output(['xcrun','--find','metal-tt'],text=True).strip())
for man_root in [sdk/'usr/share/man',tt.parent.parent/'share/man']:
    if not man_root.exists(): continue
    for path in man_root.glob('man*/*metal*'):
        if 'pipelines-script' not in path.name and 'metal-tt.' not in path.name: continue
        data = gzip.decompress(path.read_bytes()) if path.suffix == '.gz' else path.read_bytes()
        (out/('sdk-'+path.name.removesuffix('.gz')+'.txt')).write_bytes(data)

def run(args, **kw):
    return subprocess.run([str(a) for a in args], check=True, **kw)

def decode_plist(data):
    archive = plistlib.loads(data)
    objects = archive.get('$objects', [])
    def value(v, depth=0):
        if depth > 64: raise ValueError('Nested statistics archive')
        if isinstance(v, plistlib.UID): return value(objects[v.data], depth + 1)
        if isinstance(v, list): return [value(i, depth + 1) for i in v]
        if isinstance(v, dict):
            if 'NS.keys' in v:
                return dict(zip(value(v['NS.keys'], depth + 1), value(v['NS.objects'], depth + 1)))
            if 'NS.objects' in v: return value(v['NS.objects'], depth + 1)
            return {k: value(x, depth + 1) for k, x in v.items() if k != '$class'}
        return v
    return value(archive['$top']['root']) if objects else archive

def compiler_stats(folder):
    stats = {}
    for path in folder.iterdir():
        data = path.read_bytes()
        if not data.startswith(b'bplist00') or b'Instruction count' not in data: continue
        decoded = decode_plist(data)
        for stage, values in decoded.items():
            if not isinstance(values, dict): continue
            name = values.get('Compile Performance', {}).get('Function Name')
            if not name or name == 'lab_marker': continue
            stats[name] = {k: v for k, v in values.items()
                           if isinstance(v, (int, float, bool)) and 'time' not in k.lower()}
    return stats

with tempfile.TemporaryDirectory(prefix='madeira-shader-loop-') as temp:
    work = pathlib.Path(temp)
    url = os.environ['MADEIRA_SHADER_LAB_URL']
    parsed = urllib.parse.urlsplit(url)
    assert parsed.scheme == 'https' and parsed.hostname == 'release-assets.githubusercontent.com'
    key = os.environ['MADEIRA_SHADER_LAB_KEY']
    assert re.fullmatch('[0-9a-f]{64}', key)
    encrypted = work / 'input.aesgcm'
    with urllib.request.urlopen(url, timeout=90) as source, encrypted.open('wb') as dest:
        count = 0
        for block in iter(lambda: source.read(1024 * 1024), b''):
            count += len(block); assert count < 64 * 1024**2
            dest.write(block)
    # CryptoKit keeps the transfer key out of command arguments and logs.
    decrypt = work / 'decrypt.swift'
    decrypt.write_text('''import Foundation
import CryptoKit
let hex=ProcessInfo.processInfo.environment["MADEIRA_SHADER_LAB_KEY"]!
let bytes=stride(from:0,to:hex.count,by:2).map { i -> UInt8 in
 let start=hex.index(hex.startIndex,offsetBy:i)
 return UInt8(hex[start..<hex.index(start,offsetBy:2)],radix:16)!
}
let box=try AES.GCM.SealedBox(combined:Data(contentsOf:URL(fileURLWithPath:CommandLine.arguments[1])))
try AES.GCM.open(box,using:SymmetricKey(data:Data(bytes))).write(to:URL(fileURLWithPath:CommandLine.arguments[2]))
''')
    run(['xcrun','swiftc',decrypt,'-o',work/'decrypt'])
    archive = work / 'input.zip'
    run([work/'decrypt',encrypted,archive])
    inputs = work / 'inputs'; inputs.mkdir()
    with zipfile.ZipFile(archive) as z:
        assert len(z.infolist()) <= 64 and sum(i.file_size for i in z.infolist()) < 64 * 1024**2
        for entry in z.infolist():
            assert re.fullmatch(r'[0-9A-F]{1,16}\.metallib', entry.filename)
            assert entry.file_size < 4 * 1024**2
        z.extractall(inputs)
    cppflags = subprocess.check_output([llvm/'bin/llvm-config','--cxxflags'], text=True).split()
    ldflags = subprocess.check_output([llvm/'bin/llvm-config','--ldflags','--system-libs','--libs','passes','bitwriter'], text=True).split()
    run(['xcrun','clang++',*cppflags,'-std=c++20','-O2','-I'+str(root/'dxmt/src/airconv'),
         root/'tools/shader-loop-lab.cpp',root/'dxmt/src/airconv/metallib_writer.cpp',
         *ldflags,'-Wl,-rpath,'+str(llvm/'lib'),'-o',work/'optimize'])
    transforms = []
    modes = ['baseline','loops','cfg','o2']
    if os.environ.get('MADEIRA_SHADER_LAB_CONSTANTS'): modes += ['specialize']
    for mode in modes:
        folder = work / mode; folder.mkdir()
        for input_path in sorted(inputs.iterdir()):
            result = run([work/'optimize',input_path,mode,folder/input_path.name], capture_output=True, text=True)
            transforms.append({'input':input_path.stem, **json.loads(result.stdout)})
    if os.environ.get('MADEIRA_SHADER_LAB_OFFLINE') == '1':
        native = []
        for row in transforms:
            if row['input'] not in ['424068078B764242','58C1B30E79A7EE95']: continue
            folder = work/row['mode']
            library = folder/(row['input']+'.metallib')
            script = folder/(row['input']+'.mtlp-json')
            binary = folder/(row['input']+'.binary.metallib')
            buffers = [{'mutability':'Default'} for _ in range(31)]
            buffers[29] = buffers[30] = {'mutability':'Immutable'}
            total_threads = row['after']['total_threads']
            assert total_threads > 0
            descriptor = {'libraries':{'paths':[{'label':'SavedShader','path':str(library)}]},
                          'pipelines':{'compute_pipelines':[{'compute_function':'alias:SavedShader#'+row['after']['function'],
                              'threadgroup_size_is_multiple_of_thread_execution_width':total_threads % 32 == 0,
                              'buffers':buffers}]}}
            script.write_text(json.dumps(descriptor))
            command = ['xcrun','--sdk','iphoneos','metal-tt',library,script,'-arch','applegpu_g14g',
                       '-target','air64-apple-ios18.0','-o',binary]
            compiled = subprocess.run([str(a) for a in command],capture_output=True,text=True,timeout=90)
            entry = {'input':row['input'],'function':row['after']['function'],'mode':row['mode'],
                     'architecture':'applegpu_g14g','target':'air64-apple-ios18.0',
                     'pipeline_configuration':descriptor['pipelines'],'returncode':compiled.returncode,
                     'diagnostic':compiled.stdout+compiled.stderr,'binary_bytes':binary.stat().st_size if binary.exists() else 0}
            prefix = row['mode']+'-'+row['input']
            if binary.exists():
                entry['binary_magic'] = binary.read_bytes()[:32].hex()
                entry['file_type'] = subprocess.check_output(['file','--brief',str(binary)],text=True).strip()
                # Small generated GPU binaries remain in private CI artifacts,
                # allowing independent format inspection when a dumper rejects it.
                shutil.copyfile(binary,out/(prefix+'.gpubin'))
                archs = subprocess.run(['xcrun','metal-lipo',str(binary),'-archs'],capture_output=True,text=True)
                entry['architectures'] = archs.stdout+archs.stderr
                blob = binary.read_bytes()
                cursor = 32
                for _ in range(struct.unpack_from('<I',blob,16)[0]):
                    cmd,cmd_size = struct.unpack_from('<II',blob,cursor)
                    if cmd==0x19:
                        for section in range(struct.unpack_from('<I',blob,cursor+64)[0]):
                            pos=cursor+72+section*80
                            if blob[pos:pos+16].split(b'\0')[0]!=b'__compute': continue
                            size,offset=struct.unpack_from('<QI',blob,pos+40)
                            inner=work/(prefix+'.compute.macho');inner.write_bytes(blob[offset:offset+size])
                            inner_dump=subprocess.run(['xcrun','metal-objdump','--macho','--disassemble',str(inner)],
                                                      capture_output=True,text=True,timeout=60)
                            (out/(prefix+'-inner-disassembly.txt')).write_text(inner_dump.stdout+inner_dump.stderr)
                            shutil.copyfile(inner,out/inner.name)
                    cursor+=cmd_size
            for option in ['--reflection','--section-headers','--disassemble']:
                if compiled.returncode or not binary.exists(): continue
                dumped = subprocess.run(['xcrun','metal-objdump','--macho',option,str(binary)],
                                        capture_output=True,text=True,timeout=60)
                (out/(prefix+option+'.txt')).write_text(dumped.stdout+dumped.stderr)
                entry[option] = {'returncode':dumped.returncode,'text_bytes':len(dumped.stdout)}
            native.append(entry)
        report = {'format':'madeira-shader-native-experiment-v1','transforms':transforms,'native':native,
                  'scope':'Offline compilation for the M2 GPU architecture. No iPad FPS measurement.',
                  'configuration':'Threadgroup-width hint and immutable buffers29/30 match the D3D11 compute pipeline.'}
        (out/'shader-native-results.json').write_text(json.dumps(report,indent=2))
        print('Offline GPU compilation results:',[(x['mode'],x['input'],x['returncode'],x['binary_bytes']) for x in native],flush=True)
        raise SystemExit(0)
    run(['xcrun','clang','-O2','-fobjc-arc',root/'tools/shader-loop-lab-metal.m',
         '-framework','Foundation','-framework','Metal','-o',work/'metal-check'])
    env = dict(os.environ, MTL_CAPTURE_ENABLED='1')
    run([work/'metal-check',work,work], env=env)
    developer = pathlib.Path(subprocess.check_output(['xcode-select','-p'], text=True).strip())
    frameworks = developer.parent/'SharedFrameworks'
    run(['xcrun','clang','-O2','-fobjc-arc','-Wno-objc-method-access',root/'tools/gpu-apple-archive-reader.m',
         '-framework','Foundation','-framework','Metal','-o',work/'reader'])
    stats = {}
    for mode in ['baseline','loops','cfg','o2']:
        env = dict(os.environ, DYLD_FRAMEWORK_PATH=str(frameworks)+':'+str(developer.parent/'Frameworks'))
        run([work/'reader',frameworks,work/(mode+'.gputrace'),work/('read-'+mode)], env=env)
        stats[mode] = compiler_stats(work/('read-'+mode+'-shader-metadata'))
    report = {'format':'madeira-shader-loop-experiment-v1','llvm':subprocess.check_output([llvm/'bin/llvm-config','--version'], text=True).strip(),
              'scope':'Saved compute shaders recompiled on Mac. These are compiler statistics, not iPad FPS or GPU execution timings.',
              'transforms':transforms,'compiler_statistics':stats}
    (out/'shader-loop-results.json').write_text(json.dumps(report,indent=2))
    print('Compiled saved compute shaders in all modes. Compiler statistic counts:', {k:len(v) for k,v in stats.items()})
