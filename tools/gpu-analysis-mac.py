#!/usr/bin/env python3
"""Read a privately transferred comparison using Apple's Mac analysis tools.

The input transport is an authenticated AES-GCM archive. Credentials, signed
URLs and raw textures never become build artifacts or source files.
"""
from pathlib import Path
import json, os, re, shutil, subprocess, sys, tempfile, urllib.error, urllib.parse, urllib.request, zipfile

out = Path(sys.argv[1]).resolve()
developer = Path(subprocess.check_output(['xcode-select', '-p'], text=True).strip())
contents = developer.parent
frameworks = [contents / 'SharedFrameworks', contents / 'Frameworks']
env = dict(os.environ, DYLD_FRAMEWORK_PATH=':'.join(str(p) for p in frameworks))
tool = shutil.which('gpudebug')
if not tool:
    found = subprocess.run(['xcrun', '--find', 'gpudebug'], capture_output=True, text=True)
    if found.returncode == 0: tool = found.stdout.strip()
summary = {'gpudebug': tool, 'rawCaptureFilesExported': False}

with tempfile.TemporaryDirectory(prefix='madeira-gpu-analysis-') as temp:
    work = Path(temp)
    # Inventory the installed Apple reader classes if the standalone CLI is
    # absent. This is diagnostic tooling only; no private API enters Madeira.
    source = work / 'classes.m'
    source.write_text(r'''
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
int main(int argc, char **argv) { @autoreleasepool {
 for(int i=1;i<argc;i++) {
   NSError *error=nil; BOOL ok=[[NSBundle bundleWithPath:@(argv[i])] loadAndReturnError:&error];
   printf("LOAD %s %d %s\n",argv[i],ok,error.localizedDescription.UTF8String ?: "");
 }
 unsigned count=0;Class *classes=objc_copyClassList(&count);
 for(unsigned i=0;i<count;i++) {
   NSString *name=NSStringFromClass(classes[i]);
   if(!([name hasPrefix:@"DY"] || [name hasPrefix:@"GT"] || [name hasPrefix:@"MTLTools"])) continue;
   if(!([name containsString:@"Capture"] || [name containsString:@"Trace"] || [name containsString:@"Session"] || [name containsString:@"Replay"]))continue;
   printf("CLASS %s\n",name.UTF8String);
   for(int meta=0;meta<2;meta++) {
     unsigned n=0;Method *methods=class_copyMethodList(meta?object_getClass(classes[i]):classes[i],&n);
     for(unsigned m=0;m<n;m++) printf(" %s %s %s\n",meta?"+":"-",sel_getName(method_getName(methods[m])),method_getTypeEncoding(methods[m]));
     free(methods);
   }
 }
 free(classes);
} return 0; }
''')
    subprocess.run(['xcrun', 'clang', '-fobjc-arc', str(source), '-framework', 'Foundation', '-o', str(work / 'classes')], check=True)
    paths = [frameworks[0] / (n + '.framework') for n in ['MTLTools', 'MTLToolsServices', 'MTLToolsAnalysisEngine']]
    with (out / 'apple-reader-classes.txt').open('w') as log:
        r = subprocess.run([str(work / 'classes')] + [str(p) for p in paths if p.exists()], stdout=log, stderr=subprocess.STDOUT, env=env, timeout=60)
    summary['readerInventoryExit'] = r.returncode
    url = os.environ.get('MADEIRA_GPU_TRACE_URL')
    key = os.environ.get('MADEIRA_GPU_TRACE_KEY')
    if url and key:
        parsed = urllib.parse.urlsplit(url)
        assert parsed.scheme == 'https' and parsed.hostname in ['release-assets.githubusercontent.com', 'objects.githubusercontent.com'], 'Unexpected private transfer host'
        assert re.fullmatch('[0-9a-f]{64}', key), 'Invalid transfer key'
        encrypted = work / 'input.aesgcm'
        try:
            with urllib.request.urlopen(url, timeout=90) as response, encrypted.open('wb') as dest:
                size = 0
                for block in iter(lambda: response.read(1024 * 1024), b''):
                    size += len(block)
                    assert size <= 2 * 1024**3, 'Encrypted comparison too large'
                    dest.write(block)
        except urllib.error.HTTPError as error:
            raise RuntimeError('Private trace download HTTP ' + str(error.code)) from None
        decrypt = work / 'Decrypt.swift'
        decrypt.write_text(r'''
import Foundation
import CryptoKit
let keyHex=ProcessInfo.processInfo.environment["MADEIRA_GPU_TRACE_KEY"]!
let bytes=stride(from:0,to:keyHex.count,by:2).map { i -> UInt8 in
 let start=keyHex.index(keyHex.startIndex,offsetBy:i)
 return UInt8(keyHex[start..<keyHex.index(start,offsetBy:2)],radix:16)!
}
let box=try AES.GCM.SealedBox(combined:Data(contentsOf:URL(fileURLWithPath:CommandLine.arguments[1])))
let plain=try AES.GCM.open(box,using:SymmetricKey(data:Data(bytes)))
try plain.write(to:URL(fileURLWithPath:CommandLine.arguments[2]))
''')
        subprocess.run(['xcrun', 'swiftc', str(decrypt), '-o', str(work / 'decrypt')], check=True)
        archive = work / 'input.zip'
        subprocess.run([str(work / 'decrypt'), str(encrypted), str(archive)], check=True, timeout=120)
        folder = work / 'comparison'; folder.mkdir()
        with zipfile.ZipFile(archive) as z:
            assert len(z.infolist()) <= 10000 and sum(i.file_size for i in z.infolist()) <= 4 * 1024**3
            for entry in z.infolist():
                path = folder / entry.filename
                assert path.resolve().is_relative_to(folder) and not (entry.external_attr >> 16 & 0o170000 == 0o120000), 'Unsafe ZIP member'
            z.extractall(folder)
        info = json.loads((folder / 'manifest.json').read_text())
        assert info.get('format') == 'madeira-gpu-comparison-v1'
        summary['inputPair'] = info.get('context', {}).get('testID')
        summary['privateTransferBytes'] = size
        if tool:
            with (out / 'gpudebug-help.txt').open('w') as log:
                subprocess.run([tool, '--help'], stdout=log, stderr=subprocess.STDOUT, timeout=30)
            for kind in ['smooth', 'slow']:
                with (out / (kind + '-root.txt')).open('w') as log:
                    result = subprocess.run([tool, '--oneshot', '-t', str(folder / (kind + '.gputrace')), '-c', 'list', '-c', 'go api_calls', '-c', 'list --all'], stdout=log, stderr=subprocess.STDOUT, timeout=240)
                summary[kind + 'Exit'] = result.returncode
        else:
            summary['analysisStatus'] = 'Standalone Apple gpudebug unavailable; reader inventory saved, no replay claimed'
    (out / 'gpu-analysis-summary.json').write_text(json.dumps(summary, indent=2))
    print(json.dumps(summary))
