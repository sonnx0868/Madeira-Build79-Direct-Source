#!/usr/bin/env python3
"""Run real CAB extraction and the production async prerequisite/cache pipeline.

Synthetic PE data is parsed and copied, never executed. No network is needed.
On Windows, set CC and use --native-only; Mac CI runs the complete Swift core.
"""
from pathlib import Path
import ctypes, hashlib, json, os, shutil, struct, subprocess, sys, tempfile

root = Path(__file__).resolve().parents[2]
app = root/'app/Madeira'
cc = os.environ.get('CC') or shutil.which('clang')
assert cc, 'CAB verification requires clang'

def cabinet(files):
    names = b''; content = b''
    for name,data in files:
        names += struct.pack('<IIHHHH',len(data),len(content),0,0,0,0)+name.encode()+b'\0'
        content += data
    assert len(content) < 65536
    offset=36+8+len(names)
    return (b'MSCF'+struct.pack('<IIIII',0,offset+8+len(content),0,44,0)+
            struct.pack('<BBHHHHH',3,1,1,len(files),0,0,0)+struct.pack('<IHH',offset,1,0)+
            names+struct.pack('<IHH',0,len(content),len(content))+content)

def pe(bits=64, imports=()):
    d=bytearray(4096);d[:2]=b'MZ';struct.pack_into('<I',d,60,128)
    d[128:132]=b'PE\0\0';struct.pack_into('<HH',d,132,0x8664 if bits==64 else 0x14c,1)
    opt=240 if bits==64 else 224;struct.pack_into('<H',d,148,opt)
    struct.pack_into('<H',d,152,0x20b if bits==64 else 0x10b)
    directory=152+(112 if bits==64 else 96);struct.pack_into('<I',d,directory-4,16)
    struct.pack_into('<II',d,directory+8,0x1000,20*(len(imports)+1))
    section=152+opt;struct.pack_into('<III',d,section+12,0x1000,3584,512)
    for i,name in enumerate(imports):
        offset=1024+i*128;struct.pack_into('<I',d,512+i*20+12,0x1000+offset-512)
        b=name.encode()+b'\0';d[offset:offset+len(b)]=b
    return bytes(d)

with tempfile.TemporaryDirectory(prefix='madeira-runtime-check-') as scratch:
    work=Path(scratch)
    decoder=work/('cabinet.dll' if os.name=='nt' else 'libcabinet.dylib')
    sources=[app/'RuntimeCabinet.c']+[app/'vendor/mspack'/n for n in ['system.c','cabd.c','mszipd.c','lzxd.c','qtmd.c']]
    subprocess.run([cc,'-shared','-O2','-I'+str(app/'vendor/mspack'),*[str(p) for p in sources],'-o',str(decoder)],check=True)
    lib=ctypes.CDLL(str(decoder));CB=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_uint32)
    lib.madeira_cab_extract.argtypes=[ctypes.c_char_p,ctypes.c_char_p,CB,ctypes.c_void_p]
    def extract(data,folder):
        folder.mkdir();source=folder/'in.cab';source.write_bytes(data);members=[]
        @CB
        def record(ctx,name,out,size): members.append((name.decode(),Path(out.decode()).read_bytes()));return 0
        result=lib.madeira_cab_extract(str(source).encode(),str(folder).encode(),record,None)
        return result,members
    result,members=extract(cabinet([('../../../escape.dll',b'bounded payload')]),work/'traversal')
    assert result==0 and members==[('../../../escape.dll',b'bounded payload')]
    assert not (work/'escape.dll').exists()
    assert extract(cabinet([(str(i),b'') for i in range(513)]),work/'count')[0]!=0
    assert extract(cabinet([('x',b'abc')])[:-2],work/'corrupt')[0]!=0
    folder=work/'collision';folder.mkdir();(folder/'payload0000.bin').write_bytes(b'keep')
    source=folder/'in.cab';source.write_bytes(cabinet([('x',b'replace')]))
    @CB
    def no_member(*args):return 0
    assert lib.madeira_cab_extract(str(source).encode(),str(folder).encode(),no_member,None)!=0
    assert (folder/'payload0000.bin').read_bytes()==b'keep'
    print('PASS: production CAB decoder, traversal containment, archive limits, corrupt input and existing-output protection',flush=True)
    if os.name == 'nt':
        import _ctypes
        _ctypes.FreeLibrary(lib._handle)
        del lib

    swift=shutil.which('swiftc')
    if not swift:
        if '--require-swift' in sys.argv:raise SystemExit('Swift is required')
        print('SKIP: async runtime/cache fixtures need Swift on Mac CI');raise SystemExit(0)
    if '--native-only' in sys.argv:raise SystemExit(0)
    drive=work/'drive_c';bundle=work/'bundle';cache=work/'cache';game=drive/'Games/Fixture'
    game.mkdir(parents=True);(bundle/'arm64ec-windows').mkdir(parents=True)
    (game/'game.exe').write_bytes(pe(imports=['msvcp140.dll']))
    (bundle/'arm64ec-windows/kernel32.dll').write_bytes(pe())
    payloads={'msvcp140.dll':pe(imports=['vcruntime140.dll']),'vcruntime140.dll':pe(imports=['kernel32.dll'])}
    archive=b'MZ'+b'\0'*62+cabinet(list(payloads.items()))
    sha=hashlib.sha256(archive).hexdigest()
    package=dict(id='vc14-x64',family='vc14',bits=64,url='https://download.microsoft.com/fixture.exe',sha256=sha,bytes=len(archive),files=[dict(name=n,bits=64,sha256=hashlib.sha256(d).hexdigest(),bytes=len(d)) for n,d in payloads.items()])
    (bundle/'RuntimeCatalog.json').write_text(json.dumps(dict(revision=1,packages=[package])))
    path=cache/'packages'/sha;path.mkdir(parents=True);(path/'official.exe').write_bytes(archive)
    source=(app/'Library.swift').read_text(encoding='utf8')
    parser=source[source.index('    static func importNames('):source.index('\n    // MARK: session',source.index('    static func importNames('))]
    dock=(app/'DockInstallers.swift').read_text(encoding='utf8');start=dock.index('    nonisolated static func machine(');end=dock.index('\n    /// Why this build',start)
    core=(app/'RuntimePreparation.swift').read_text(encoding='utf8')
    stubs='\nenum LibraryModel {\n'+parser+'\n}\nenum DockInstallers {\n'+dock[start:end]+'\n}\n'
    test=r'''
@main struct Check {
 static func main() async throws {
    let root = URL(fileURLWithPath: CommandLine.arguments[1])
    let drive=root.appendingPathComponent("drive_c"), bundle=root.appendingPathComponent("bundle"), cache=root.appendingPathComponent("cache")
    let game=drive.appendingPathComponent("Games/Fixture"), exe=game.appendingPathComponent("game.exe")
    func run(_ force: Bool = false) async throws -> RuntimeReport {
      try await RuntimePreparer.shared.prepare(executable:exe,bits:64,drive:drive,bundle:bundle,cache:cache,force:force,progress:{ _ in })
    }
    let first=try await run();assert(!first.cached && first.missing.isEmpty && first.installed==["vc14-x64"])
    let second=try await run();assert(second.cached)
    try FileManager.default.removeItem(at:game.appendingPathComponent("vcruntime140.dll"))
    let restored=try await run();assert(!restored.cached && restored.missing.isEmpty)
    let wrong=root.appendingPathComponent("wrong.dll")
    try FileManager.default.removeItem(at:game.appendingPathComponent("msvcp140.dll"))
    try FileManager.default.copyItem(at:wrong,to:game.appendingPathComponent("msvcp140.dll"))
    let blocked=try await run();assert(!blocked.cached && blocked.missing.contains { $0.name=="msvcp140.dll" && $0.wrongArchitecture })
    assert(RuntimeFiles.machine(game.appendingPathComponent("msvcp140.dll"))==0x14c)
    let warningCache=try await run();assert(warningCache.cached)
    try FileManager.default.copyItem(at:root.appendingPathComponent("plugin.dll"),to:game.appendingPathComponent("plugin.dll"))
    let changed=try await run();assert(!changed.cached && changed.missing.contains { $0.name=="game_private.dll" })
    try FileManager.default.removeItem(at:exe)
    do { _ = try await run(); fatalError("Missing executable accepted") } catch {}
    print("PASS: production automatic install, transitive DLLs, recheck, cache hit, deletion repair, wrong-architecture preservation, directory invalidation and missing EXE")
 }
}
'''
    (work/'wrong.dll').write_bytes(pe(bits=32));(work/'plugin.dll').write_bytes(pe(imports=['game_private.dll']))
    (work/'Core.swift').write_text(core+stubs)
    (work/'Check.swift').write_text('import Foundation\n'+test)
    (work/'mono.c').write_text('int madeira_runtime_prepare_mono(const char*a,const char*b){return 0;}')
    subprocess.run([cc,'-c',str(work/'mono.c'),'-o',str(work/'mono.o')],check=True)
    (work/'bridge.h').write_text((app/'RuntimeCabinet.h').read_text()+'\nint madeira_runtime_prepare_mono(const char*,const char*);\n')
    binary=work/'check'
    subprocess.run([swift,'-parse-as-library','-swift-version','5','-import-objc-header',str(work/'bridge.h'),str(work/'Core.swift'),str(work/'Check.swift'),str(work/'mono.o'),'-L'+str(work),'-lcabinet','-Xlinker','-rpath','-Xlinker',str(work),'-o',str(binary)],check=True)
    subprocess.run([str(binary),str(work)],check=True,timeout=45)
