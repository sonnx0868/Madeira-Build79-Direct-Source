"""Package only the reviewed native cancellation function change."""
from pathlib import Path
import hashlib,json,zipfile,sys
H=Path(__file__).resolve().parent;ROOT=H.parents[2]
sha=lambda b:hashlib.sha256(b).hexdigest()
base=ROOT/'outputs/ipad-installers/Madeira-iPad-GuestExit-v2-Test.ipa'
target=ROOT/'outputs/ipad-installers/Madeira-iPad-AsyncCancel-v1-Test.ipa'
assert sha(base.read_bytes())=='fdea8baeca2168aab800800eb3c147615461669b0d49ba81fa32d0a432baa5b9'
fixed=(H/'native/Madeira').read_bytes();report=json.loads((H/'native/build-report.json').read_text())
assert sha(fixed)==report['native_sha256']=='f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984'
for p in ['build/report.json','native/test-report.json','native/review_source_report.json']:
 r=json.loads((H/p).read_text());assert r.get('passed')or r.get('result')=='PASS',p
member='Payload/Madeira.app/Madeira'
with zipfile.ZipFile(base) as old,zipfile.ZipFile(target,'w') as new:
 assert len(old.namelist())==len(set(old.namelist()))
 assert sha(old.read(member))==report['baseline_sha256']
 for info in old.infolist():new.writestr(info,fixed if info.filename==member else old.read(info))
with zipfile.ZipFile(base) as old,zipfile.ZipFile(target) as new:
 assert old.namelist()==new.namelist()
 changed=[n for n in old.namelist()if old.read(n)!=new.read(n)];assert changed==[member]
result={'package':str(target),'sha256':sha(target.read_bytes()),'native_sha256':sha(fixed),
 'baseline_sha256':sha(base.read_bytes()),'changed_members':changed,'all_other_payload_members_identical':True,
 'native_change':'Only req_cancel_async [0x100054dec,0x10005506c); exact prologue/epilogue/unwind preserved.',
 'preserves_cpu_tls_guest_exit_fd_cache_ui_vulkan_controller':True,'installed':False,'device_validated':False,'cef_fixed':False}
(H/'package-report.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
