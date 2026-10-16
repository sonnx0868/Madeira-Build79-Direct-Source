#!/usr/bin/env python3
"""Run concurrency/failure cases against the production Objective-C cache."""
from pathlib import Path
import argparse, platform, shutil, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-only',action='store_true')
args=parser.parse_args()
folder=root/'build/dxmt-ios'
source=(folder/'pipeline_cache_ios.m').read_text(encoding='utf-8')
assert 'while (!entry.done) [state->gate wait]' in source
assert 'entry.owner == NSThread.currentThread' in source
assert 'abortBuild(state, key, ticket, exception)' in source
assert 'NSURLIsSymbolicLinkKey' in source and 'NSUUID.UUID.UUIDString' in source
assert 'live-pipeline-v2' in source
if args.source_only:
    print('PASS: live pipeline integration contracts; native concurrency fixture NOT RUN')
elif platform.system()!='Darwin':
    raise SystemExit('Native pipeline fixture requires macOS Metal/Foundation')
else:
    with tempfile.TemporaryDirectory(prefix='madeira-live-pipelines-') as temp:
        exe=Path(temp)/'check'
        subprocess.run([shutil.which('clang'),'-fobjc-arc','-fblocks','-Wno-protocol',
            '-Wno-objc-protocol-property-synthesis','-fsanitize=address,undefined',
            '-I',str(folder),'-framework','Foundation','-framework','Metal',
            str(root/'tests/host/live-pipeline-cache.m'),str(folder/'pipeline_warmup_ios.m'),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True,timeout=30)
