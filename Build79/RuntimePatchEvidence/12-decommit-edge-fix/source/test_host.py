#!/usr/bin/env python3
"""Rebuild and execute the real-protection Linux fixture; no device access."""
import hashlib,json,pathlib,re,resource,subprocess
ROOT=pathlib.Path(__file__).resolve().parent
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
build=ROOT/'build';build.mkdir(exist_ok=True)
exe=build/'test_decommit'
cmd=['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-o',str(exe),str(ROOT/'test_decommit.c')]
c=subprocess.run(cmd,text=True,capture_output=True)
report={'passed':False,'compile_command':cmd,'compiler':subprocess.check_output(['cc','--version'],text=True).splitlines()[0], 'compile_returncode':c.returncode,'compile_stdout':c.stdout,'compile_stderr':c.stderr}
for label,p in [('fragment',ROOT/'decommit_pages.fragment.c'),('fixture',ROOT/'test_decommit.c')]:report[label+'_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
if c.returncode==0:
 r=subprocess.run([str(exe)],text=True,capture_output=True,timeout=30)
 report.update(returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest())
 m=re.search(r'^PASS (\d+) cases;',r.stdout,re.M)
 report['passed']=r.returncode==0 and m is not None
 report['cases']=int(m.group(1)) if m else 0
report['scope']='Real Linux mprotect/mmap with 4KiB OS pages grouped into aligned16KiB emulated host pages; same C source fragment. Baseline direct clear must SIGSEGV. Positive byte/final-protection checks and injected query/protect/silent-protect/restore/remap errors. Mach region ABI and kernel behavior are modeled, not iOS-tested. Restoration failure returns error but cannot guarantee physical rollback after kernel refusal. No ARM64 native execution.'
(ROOT/'host-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:report[k] for k in ['passed','cases','fragment_sha256','fixture_sha256']},indent=2))
raise SystemExit(0 if report['passed'] else 1)
