#!/usr/bin/env python3
from pathlib import Path
import shlex,subprocess,json,hashlib
H=Path(__file__).resolve().parent;W=H.parents[1]/'vulkan/build-audit/wine-build';B=H/'build/config-audit';B.mkdir(exist_ok=True)
commands=[shlex.split(x) for x in (H/'target-plan.log').read_text().replace('\\\n',' ').splitlines() if x and not x.startswith('make:')]
cmds=[c for c in commands if c[0]=='clang' and any(a.startswith('../wine-source/dlls/ntdll/') and a.endswith('.c') for a in c) and c[c.index('-target')+1]=='arm64ec-windows']
rows=[]
for c in cmds:
 source=next(a for a in c if a.startswith('../wine-source/dlls/ntdll/') and a.endswith('.c'))
 out=B/(Path(source).name+'.d');p=c.copy();p.remove('-c');p[p.index('-o')+1]=str(out);p+=['-M','-MT','audit']
 r=subprocess.run(p,cwd=W,capture_output=True,text=True,check=True)
 dep=out.read_text().replace('\\\n',' ');paths=shlex.split(dep.split(':',1)[1]);config=[a for a in paths if a.endswith('/config.h') or a=='config.h']
 rows.append({'source':source,'sha256':hashlib.sha256((W/source).read_bytes()).hexdigest(),'host_config_h_dependencies':config})
c=cmds[0].copy();c.remove('-c');c[c.index('-o')+1]=str(B/'target-macros.txt');c+=['-E','-dM'];subprocess.run(c,cwd=W,capture_output=True,check=True)
macro=(B/'target-macros.txt').read_text();wanted=['__arm64ec__','__aarch64__','_WIN32','_WIN64','__APPLE__','__linux__','FEX_IOS_HOST','__WINE_PE_BUILD','_M_ARM64EC','__ARM_ARCH']
m={key:[line for line in macro.splitlines() if line.startswith('#define '+key+' ') or line=='#define '+key] for key in wanted}
report={'compiler':subprocess.check_output(['clang','--version'],text=True).splitlines()[0],'linker':subprocess.check_output(['lld-link','--version'],text=True).strip(),'target':'arm64ec-windows','unit_count':len(rows),'units':rows,'target_macro_evidence':m,'host_config_h_used':any(r['host_config_h_dependencies'] for r in rows),'note':'PE ntdll uses Wine Windows headers and target Windows ABI, not Darwin native headers. Existing original warning categories format/declaration-after-statement retained; no host-native Wine object linked.'}
assert not report['host_config_h_used'],report
assert not m['__APPLE__'] and not m['__linux__'] and not m['FEX_IOS_HOST']
(H/'config-audit.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='units'},indent=2))
