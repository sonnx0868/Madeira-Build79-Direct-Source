"""Add only the pinned guest-exit diagnostic and its new desktop shortcut."""
from pathlib import Path
import hashlib, importlib.util, json
H=Path(__file__).resolve().parent;ROOT=H.parents[2]
FOLDER='/Documents/wine/drive_c/MadeiraDiagnostics/BeamNG034'
FILES={'MadeiraGuestExit.exe':('45172dd8a3292e4224c0166261cf19bc59978eafc49637a1291cb8c95ce504f5',5120),
       'Guest process exit test.lnk':('17b83acf8c6737f08766ffa835704c4a30f185d91959179694c9b8f6ac820659',357)}
data={n:(H/'probe'/n).read_bytes() for n in FILES}
for n,(digest,size) in FILES.items():
    assert len(data[n])==size and hashlib.sha256(data[n]).hexdigest()==digest
p=ROOT/'work/ipad-automation/native-control-usb/transport.py'
s=importlib.util.spec_from_file_location('guest_exit_transport',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
c=m.MadeiraDocuments(allow_guarded_rename=True);c.directory(FOLDER)
targets=[(FOLDER+'/MadeiraGuestExit.exe',data['MadeiraGuestExit.exe'])]
desktops=[]
for name in ['mythic','mobile','madeira']:
    d='/Documents/wine/drive_c/users/'+name+'/Desktop'
    info=c.info(d)
    if info is not None and info.get(b'st_ifmt')==b'S_IFDIR':
        c.directory(d);desktops.append(d);targets.append((d+'/Guest process exit test.lnk',data['Guest process exit test.lnk']))
assert desktops
for path,b in targets:
    c.collision(path)
    if c.info(path) is not None:assert c.read(path,exact=len(b))==b,'Different existing file preserved'
for path,b in targets:
    if c.info(path) is None:c.publish(path,b)
    assert c.read(path,exact=len(b))==b
report={'readback_verified':True,'files':[p for p,_ in targets],'launched':False,'game_files_changed':False}
(H/'probe-deployment.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
