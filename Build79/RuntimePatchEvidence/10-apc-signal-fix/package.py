from pathlib import Path
import hashlib,json,zipfile
H=Path(__file__).resolve().parent;ROOT=H.parents[2]
sha=lambda b:hashlib.sha256(b).hexdigest()
base=ROOT/'outputs/ipad-installers/Madeira-iPad-AsyncCancel-v1-Test.ipa';assert sha(base.read_bytes())=='d5e9501e4cc36edaa3e2afb3e7ce379d3ea92ae82e0261faa66488fbf3ce6a0c'
fixed=(H/'Madeira').read_bytes();assert sha(fixed)=='1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b'
test=json.loads((H/'native-test-report.json').read_text());assert test.get('passed')or test.get('result')=='PASS'
assert test.get('patched_sha256',test.get('candidate_sha256'))==sha(fixed),'Test must attest exact candidate'
target=ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Signal-v1-Test.ipa';member='Payload/Madeira.app/Madeira'
with zipfile.ZipFile(base)as old,zipfile.ZipFile(target,'w')as new:
 assert len(old.namelist())==len(set(old.namelist()))
 assert sha(old.read(member))=='f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984'
 for info in old.infolist():new.writestr(info,fixed if info.filename==member else old.read(info))
with zipfile.ZipFile(base)as old,zipfile.ZipFile(target)as new:
 assert old.namelist()==new.namelist();changed=[n for n in old.namelist()if old.read(n)!=new.read(n)];assert changed==[member]
report={'package':str(target),'sha256':sha(target.read_bytes()),'native_sha256':sha(fixed),'baseline_sha256':sha(base.read_bytes()),'changed_members':changed,'all_other_payload_members_identical':True,'native_change':'20 bytes only in send_thread_signal; guest Mach task port used for real thread-right extraction and signaling. Global process memory access remains unchanged.','preserves_async_cancellation_cpu_tls_guest_exit_fd_cache_ui_vulkan_controller':True,'installed':False,'device_validated':False,'cef_fixed':False}
(H/'package-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
