"""Preserve five private diagnostic logs and distinguish fresh reruns by mtime."""
from pathlib import Path
import hashlib,importlib.util,json,time
H=Path(__file__).resolve().parent;ROOT=H.parents[2];out=H/'device-after';out.mkdir(exist_ok=True)
s=importlib.util.spec_from_file_location('apc_test_transport',ROOT/'work/ipad-automation/native-control-usb/transport.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);c=m.MadeiraDocuments()
folder='/Documents/wine/drive_c/MadeiraDiagnostics/BeamNG034/'
names=['async-cancel-v2-test.log','cef-ipc-parent.log','cef-ipc-child.log','apc-delivery.log','io-check-suite.log']
def metadata(name):
 info=c.info(folder+name)
 if info is None:return None
 return {k.decode():v.decode()for k,v in info.items()if k in (b'st_mtime',b'st_size',b'st_ifmt')}
baseline={n:metadata(n)for n in names};(out/'baseline.json').write_text(json.dumps(baseline,indent=2)+'\n')
last={};snapshots={};deadline=time.monotonic()+900
print('Capturing five private tests; baseline mtimes recorded for freshness.',flush=True)
while time.monotonic()<deadline:
 for name in names:
  info=metadata(name)
  if info is None:continue
  data=c.read(folder+name,maximum=262144)
  if not data:continue
  digest=hashlib.sha256(data).hexdigest();key=(digest,info.get('st_mtime'))
  if last.get(name)==key:continue
  last[name]=key;fresh=baseline[name]is None or info.get('st_mtime')!=baseline[name].get('st_mtime')
  (out/name).write_bytes(data);snapshots[name]={'sha256':digest,'bytes':len(data),'remote_info':info,'fresh_since_capture_start':fresh,'captured_unix_time':time.time()}
  (out/'snapshots.json').write_text(json.dumps(snapshots,indent=2)+'\n')
  lines=data.decode(errors='replace').splitlines();important=[l for l in lines if l.startswith(('FAIL','PASS all','PASS CefIpc','PASS busy','PASS suite','SUMMARY','CHILD','DONE','RESULT'))]
  if fresh and important:
   print(name+': '+' | '.join(important[-5:]),flush=True)
   (out/(digest[:16]+'-'+name)).write_bytes(data)
 time.sleep(2)
print('Capture complete.',flush=True)
