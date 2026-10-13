#!/usr/bin/env python3
"""Exercise the real Mesa patch tool, overlapping patches and rejected drift."""
from pathlib import Path
import argparse
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--source',type=Path,default=root/'research/mesa-25.0.7')
parser.add_argument('--patch-tool',default=shutil.which('patch'))
args=parser.parse_args()
patches=sorted((root/'build/mesa-ios/patches').glob('*.patch'))
targets=set()
for p in patches:
    for line in p.read_text(encoding='utf-8').splitlines():
        if line.startswith('+++ b/'): targets.add(line[6:].split()[0])
expected={name:(args.source/name).read_bytes() for name in targets}
def seed(folder):
    for name,data in expected.items():
        path=folder/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
def undo(folder,p):
    subprocess.run([args.patch_tool,'--batch','--force','--fuzz=0','-p1','--reverse'],
        input=p.read_bytes().replace(b'\r\n',b'\n'),cwd=folder,check=True,stdout=subprocess.DEVNULL)
def run(folder,okay=True):
    result=subprocess.run([sys.executable,str(root/'tools/apply-mesa-patches.py'),'--source',str(folder),
                           '--patch-tool',args.patch_tool],capture_output=True,text=True)
    assert (result.returncode==0)==okay,result.stdout+result.stderr
    if okay: assert all((folder/name).read_bytes()==data for name,data in expected.items())
with tempfile.TemporaryDirectory(prefix='madeira-patch-regression-') as scratch:
    scratch=Path(scratch)
    full=scratch/'full';seed(full);run(full);run(full)
    fresh=scratch/'fresh';seed(fresh)
    for p in reversed(patches): undo(fresh,p)
    run(fresh)
    prefix=scratch/'prefix';seed(prefix);undo(prefix,patches[-1]);run(prefix)
    broken=scratch/'broken';seed(broken)
    path=broken/'src/gallium/frontends/osmesa/osmesa.c'
    original=path.read_bytes().replace(b'global_fscreen = CALLOC_STRUCT',b'unknown_allocator = CALLOC_STRUCT')
    path.write_bytes(original);run(broken,okay=False)
    assert path.read_bytes()==original
print('PASS: production Mesa patch series applies to clean source, recognizes overlapping applied patches, upgrades a prefix and rejects drift without source writes')
