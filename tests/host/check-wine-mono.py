#!/usr/bin/env python3
"""Verify the staged runtime, its x86 imports, and real prefix placement."""
from pathlib import Path
import argparse, hashlib, importlib.util, json, re, shutil, subprocess, sys, tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(); parser.add_argument('--require-objc', action='store_true'); args = parser.parse_args()
folder = root / 'app/Madeira/mono'
receipt = json.loads((folder / 'receipt.json').read_text())
assert receipt['version'] == '11.0.0' and receipt['archive_sha256'] == '0cd723aa28897f7d7d2702eed6c72e4262255980e212bc2b3c94bfa234abe5fd'
runtime = folder / 'wine-mono-11.0.0'
assert len(receipt['files']) == 2923
for path, digest in receipt['files'].items():
    assert hashlib.sha256((runtime / path).read_bytes()).hexdigest() == digest, path
spec = importlib.util.spec_from_file_location('pe_module', root / 'tools/check-wow64-addon.py')
pe = importlib.util.module_from_spec(spec); spec.loader.exec_module(pe)
info = pe.pe_info((runtime / 'bin/libmono-2.0-x86.dll').read_bytes())
assert info['machine'] == 0x14c
farm = root / 'app/Madeira/i386-windows'
if farm.joinpath('ntdll.dll').exists():
    modules = {p.name.lower() for p in farm.iterdir()}
    contracts = {a.lower()+'.dll': b.lower() for a,b in re.findall(r'^apiset\s+(\S+)\s*=\s*(\S+)',
                 (root / 'wine/dlls/apisetschema/apisetschema.spec').read_text(), re.M)}
    missing = {dll for dll in info['imports'] if dll not in modules and contracts.get(dll) not in modules}
    assert not missing, missing
    print('PASS: Wine Mono i386 immediate imports resolve in the staged runtime farm')
elif args.require_objc: raise SystemExit('i386 runtime farm is required for the release gate')
else: print('SKIP: i386 import closure needs the restored runtime farm')
assert b'mono_jit_init_version' in (runtime / 'bin/libmono-2.0-x86.dll').read_bytes()
assert 'WineMonoPrefix.h' in (root / 'app/Madeira/WineProcessBridge.m').read_text()
assert 'mono in Resources' in (root / 'app/Madeira.xcodeproj/project.pbxproj').read_text()
assert (root / 'app/Madeira/legal/THIRD-PARTY-NOTICES-WineMono.txt').stat().st_size > 1000000
print('PASS: pinned Wine Mono 11.0.0, all 2923 official files and bundled notices')
if sys.platform != 'darwin':
    if args.require_objc: raise SystemExit('Darwin Foundation is required for prefix placement tests')
    print('SKIP: Objective-C prefix execution needs Darwin Foundation'); raise SystemExit(0)
source = r'''
#import <Foundation/Foundation.h>
#import "WineMonoPrefix.h"
#include <assert.h>
#include <stdio.h>
int main(void) { @autoreleasepool {
    NSFileManager *fm = NSFileManager.defaultManager;
    NSString *base = [NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString];
    NSString *prefix = [base stringByAppendingPathComponent:@"prefix"], *a = [base stringByAppendingPathComponent:@"A.app"], *b = [base stringByAppendingPathComponent:@"B.app"];
    for (NSString *bundle in @[a,b]) {
        for (NSString *name in @[@"bin/libmono-2.0-x86.dll", @"lib/mono/4.5/mscorlib.dll"]) {
            NSString *path = [[bundle stringByAppendingPathComponent:@"mono/wine-mono-11.0.0"] stringByAppendingPathComponent:name];
            assert([fm createDirectoryAtPath:path.stringByDeletingLastPathComponent withIntermediateDirectories:YES attributes:nil error:nil]);
            assert([[NSData dataWithBytes:"fixture" length:7] writeToFile:path atomically:YES]);
        }
    }
    assert([madeira_prepare_mono_prefix(a,prefix) isEqual:@"linked"]);
    assert([madeira_prepare_mono_prefix(a,prefix) isEqual:@"already-linked"]);
    NSString *target = [prefix stringByAppendingPathComponent:@"drive_c/windows/mono/mono-2.0"];
    assert([[fm destinationOfSymbolicLinkAtPath:target error:nil] isEqual:[a stringByAppendingPathComponent:@"mono/wine-mono-11.0.0"]]);
    assert([madeira_prepare_mono_prefix(b,prefix) isEqual:@"linked"]);
    assert([[fm destinationOfSymbolicLinkAtPath:target error:nil] hasPrefix:b]);
    assert([madeira_prepare_mono_prefix(base,prefix) isEqual:@"missing-bundle"]);
    assert([fm removeItemAtPath:target error:nil]);
    assert([fm createDirectoryAtPath:target withIntermediateDirectories:YES attributes:nil error:nil]);
    assert([madeira_prepare_mono_prefix(a,prefix) isEqual:@"existing-runtime"]);
    assert([fm removeItemAtPath:target error:nil]);
    assert([fm createSymbolicLinkAtPath:target withDestinationPath:[base stringByAppendingPathComponent:@"custom-mono"] error:nil]);
    assert([madeira_prepare_mono_prefix(a,prefix) isEqual:@"existing-runtime"]);
    assert([fm removeItemAtPath:base error:nil]);
    puts("PASS: production Mono prefix link, app reinstall refresh, missing bundle and custom installed runtime preservation");
} return 0; }
'''
with tempfile.TemporaryDirectory(prefix='madeira-mono-prefix-') as scratch:
    scratch = Path(scratch); (scratch / 'main.m').write_text(source)
    subprocess.run(['clang', '-fobjc-arc', '-framework', 'Foundation', '-I'+str(root / 'app/Madeira'), str(scratch / 'main.m'), '-o', str(scratch / 'check')], check=True)
    subprocess.run([str(scratch / 'check')], check=True)
