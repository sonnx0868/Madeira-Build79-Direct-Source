#!/usr/bin/env python3
"""Apply a verified prefix of the Mesa patch series without direction guessing."""
from pathlib import Path
import argparse
import shutil
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--source',required=True,type=Path)
parser.add_argument('--patch-tool',default=shutil.which('patch'))
parser.add_argument('--patches',type=Path,default=root/'build/mesa-ios/patches')
args=parser.parse_args()
if not args.patch_tool: raise SystemExit('patch tool required')
source=args.source.resolve()
patches=sorted(args.patches.glob('*.patch'))
payloads=[p.read_bytes().replace(b'\r\n',b'\n') for p in patches]
targets=set()
for data in payloads:
    for line in data.decode('utf-8').splitlines():
        if line.startswith('+++ b/'): targets.add(line[6:].split()[0])
for name in targets:
    target=(source/name).resolve()
    if not target.is_relative_to(source) or not target.is_file():
        raise SystemExit('Invalid Mesa source target: '+name)
original={name:(source/name).read_bytes() for name in targets}
def seed(folder):
    for name,data in original.items():
        path=folder/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
def apply(folder,data,reverse=False):
    return subprocess.run([args.patch_tool,'--batch','--force','--fuzz=0','-p1',
                           '--reverse' if reverse else '--forward'],input=data,cwd=folder,
                          stdout=subprocess.PIPE,stderr=subprocess.STDOUT)

with tempfile.TemporaryDirectory(prefix='madeira-mesa-patches-') as scratch:
    scratch=Path(scratch)
    # Later patches may edit earlier patches' added code. Undo in reverse
    # order to recognize a complete prefix, rather than testing each alone.
    for count in range(len(patches),-1,-1):
        probe=scratch/('prefix-'+str(count));probe.mkdir();seed(probe)
        if all(apply(probe,data,True).returncode==0 for data in reversed(payloads[:count])):
            candidate=scratch/'candidate';candidate.mkdir();seed(candidate)
            for i in range(count,len(patches)):
                result=apply(candidate,payloads[i])
                if result.returncode:
                    raise SystemExit('Mesa source drift at '+patches[i].name+'\n'+result.stdout.decode(errors='replace'))
            # Check the entire resulting series, including overlapping patches.
            verify=scratch/'verified';verify.mkdir()
            for name in targets:
                path=verify/name;path.parent.mkdir(parents=True,exist_ok=True)
                path.write_bytes((candidate/name).read_bytes())
            for i in range(len(patches)-1,-1,-1):
                result=apply(verify,payloads[i],True)
                if result.returncode: raise SystemExit('Mesa reverse verification failed: '+patches[i].name)
            # All probes completed before any source file is replaced.
            for name in sorted(targets):
                data=(candidate/name).read_bytes()
                if data!=original[name]:
                    target=source/name;temporary=target.with_name(target.name+'.madeira-patch.tmp')
                    temporary.write_bytes(data);temporary.replace(target)
            print(f'Verified Mesa patch series: {len(patches)} patches, already applied prefix={count}')
            break
