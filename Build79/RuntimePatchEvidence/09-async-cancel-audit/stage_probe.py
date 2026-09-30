"""Add only the pinned async-cancellation diagnostic and its new desktop shortcut."""
from pathlib import Path
import hashlib, importlib.util, json
H=Path(__file__).resolve().parent;ROOT=H.parents[2]
FOLDER='/Documents/wine/drive_c/MadeiraDiagnostics/BeamNG034'
FILES={'MadeiraAsyncCancel.exe':('84136602707fa8dba09ae0d437ba935bf4e3f254b046464d519910d77214feea',6656),
       'Async cancellation test.lnk':('a5180f251910fc9357f04f1a6ea2d09d1548bea1e5c09f40e5e5c344bef620b8',359)}
data={n:(H/'probe'/n).read_bytes() for n in FILES}
for n,(digest,size) in FILES.items():
    assert len(data[n])==size and hashlib.sha256(data[n]).hexdigest()==digest
p=ROOT/'work/ipad-automation/native-control-usb/transport.py'
s=importlib.util.spec_from_file_location('async_cancel_transport',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
c=m.MadeiraDocuments(allow_guarded_rename=True);c.directory(FOLDER)
targets=[(FOLDER+'/MadeiraAsyncCancel.exe',data['MadeiraAsyncCancel.exe'])]
desktops=[]
for name in ['mythic','mobile','madeira']:
    d='/Documents/wine/drive_c/users/'+name+'/Desktop'
    info=c.info(d)
    if info is not None and info.get(b'st_ifmt')==b'S_IFDIR':
        c.directory(d);desktops.append(d);targets.append((d+'/Async cancellation test.lnk',data['Async cancellation test.lnk']))
assert desktops
for path,b in targets:
    c.collision(path)
    if c.info(path) is not None:assert c.read(path,exact=len(b))==b,'Different existing file preserved'
for path,b in targets:
    if c.info(path) is None:c.publish(path,b)
    assert c.read(path,exact=len(b))==b
report={'readback_verified':True,'files':[p for p,_ in targets],'launched':False,'game_files_changed':False}
(H/'probe-deployment.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
