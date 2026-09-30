#!/usr/bin/env python3
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,shlex,json,hashlib
H=Path(__file__).resolve().parent
W=H.parents[1]/'vulkan/build-audit/wine-build'
B=H/'build';B.mkdir(exist_ok=True)
# This is a version-pinned native/PE ABI experiment. Refuse changed original
# source/header dependencies rather than reusing objects under another ABI.
provenance=H/'source-provenance.json'
if provenance.exists():
 for rel,want in json.loads(provenance.read_text())['verified_against_archive'].items():
  assert hashlib.sha256((W.parent/'wine-source'/rel).read_bytes()).hexdigest()==want,rel
commands=[shlex.split(x) for x in (H/'target-plan.log').read_text().replace('\\\n',' ').splitlines() if x and not x.startswith('make:')]
assert all(c[0] in ['clang','tools/wrc/wrc','tools/winebuild/winebuild','tools/winegcc/winegcc'] for c in commands)
outputs={c[c.index('-o')+1] for c in commands}
remap={o:str(B/o) for o in outputs}

def run(c,label):
 out=Path(c[c.index('-o')+1]);out.parent.mkdir(parents=True,exist_ok=True)
 log=out.with_name(out.name+'.log')
 stamp=out.with_name(out.name+'.inputs.json')
 inputs={'argv':c,'files':{}}
 for arg in c:
  path=Path(arg) if Path(arg).is_absolute() else W/arg
  if path != out and path.is_file():inputs['files'][str(path.resolve())]=hashlib.sha256(path.read_bytes()).hexdigest()
 key=json.dumps(inputs,sort_keys=True)
 if out.exists() and log.exists() and stamp.exists() and stamp.read_text()==key and log.read_text().endswith('\nBUILD_EXIT=0\n'):return
 with log.open('w') as f:
  f.write(shlex.join(c)+'\n');f.flush()
  p=subprocess.run(c,cwd=W,stdout=f,stderr=subprocess.STDOUT)
  f.write('\nBUILD_EXIT='+str(p.returncode)+'\n')
 if p.returncode:raise RuntimeError(str(log))
 stamp.write_text(key)

def convert(c):return [remap.get(a,a) for a in c]
compiles=[convert(c) for c in commands if c[0]=='clang']
# Independent object compilations; archive and final links below depend on these.
with ThreadPoolExecutor(max_workers=4) as pool:
 for i,_ in enumerate(pool.map(lambda c:run(c,'compile'),compiles),1):
  if i%100==0:print('compiled',i,'/',len(compiles),flush=True)
for old in commands:
 if old[0]=='clang' or old[0]=='tools/winegcc/winegcc':continue
 run(convert(old),'resource/archive')
link=convert(next(c for c in commands if c[0]=='tools/winegcc/winegcc'))
# Preserve an unmodified rebuild and build the exact source patch separately.
for variant in ['original','patched']:
 c=link.copy();c[c.index('-o')+1]=str(B/variant/'ntdll-arm64x.dll')
 if variant=='patched':
  for arch in ['arm64ec']:
   old=next(c for c in commands if c[0]=='clang' and c[c.index('-o')+1]==f'dlls/ntdll/{arch}-windows/loader.o')
   p=convert(old);origout=p[p.index('-o')+1];p[p.index('-o')+1]=str(B/variant/f'{arch}-loader.o')
   source='../wine-source/dlls/ntdll/loader.c';assert source in p
   p[p.index(source)]=str(H/'source/loader.c')
   p+=['-Werror','-Wno-error=format','-Wno-error=declaration-after-statement']
   run(p,'patched loader')
   c=[p[p.index('-o')+1] if a==origout else a for a in c]
 # Native ARM64 fork has an unresolved xlate_ios_jit reference; do not fake it.
 # The installed target is stand-alone ARM64EC, built below.
 # Match Madeira's stand-alone ARM64EC module form; unchanged hybrid static
 # libraries select only EC objects. No native ARM64 implementation is included.
 e=c.copy();e.remove('-marm64x');e[e.index('-o')+1]=str(B/variant/'ntdll.dll')
 e=[a for a in e if not ('/dlls/ntdll/aarch64-windows/' in a and a.endswith('.o')) and not a.endswith('/aarch64-loader.o')]
 run(e,variant+' ARM64EC')
report={}
for f in sorted(B.glob('*/ntdll*.dll')):
 meta=subprocess.check_output(['llvm-readobj','--file-headers','--coff-exports','--coff-imports','--coff-load-config',str(f)],text=True)
 f.with_suffix('.metadata.txt').write_text(meta)
 report[str(f.relative_to(H))]={'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'bytes':f.stat().st_size,'format':next(x for x in meta.splitlines() if x.startswith('Format:'))}
(H/'build-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
