from pathlib import Path
import hashlib,importlib.util,json
H=Path(__file__).resolve().parent;ROOT=H.parents[2];folder='/Documents/wine/drive_c/MadeiraDiagnostics/BeamNG034'
items=[('MadeiraAsyncCancelV2.exe','work/beamng/async-cancel-audit/probe-v2/build/MadeiraAsyncCancelV2.exe','50bf0a77ab1cfe4d65847518ea7d24c8e585c2c1e8015cefcb8185d695ee3b77',8704),
 ('MadeiraCefIpc.exe','work/beamng/cef-ipc-probe/build/MadeiraCefIpc.exe','f5243c83112e0a7c049e719b226e85f28006464b451b1d4f3b2e8f209d30942b',16384),
 ('MadeiraApcDelivery.exe','work/beamng/apc-delivery-probe/build/MadeiraApcDelivery.exe','788b9075cde86bf38d8a6f4d5991f9de6402ff2fb14b68f628516f1efd269113',8192),
 ('MadeiraIoCheckSuite.exe','work/beamng/io-check-suite/build/MadeiraIoCheckSuite.exe','f5d2fe14716c2f9474bce4abc62c15c181e97992c04a294376d93e6cb85bccf7',7680),
 ('Wine IO checks.lnk','work/beamng/io-check-suite/build/Wine IO checks.lnk','95ec3b6a259f32412b24f0cc0541d9e82115913c02a4a70ee568fae8c7b8bd46',342)]
data={}
for name,path,digest,size in items:
 b=(ROOT/path).read_bytes();assert len(b)==size and hashlib.sha256(b).hexdigest()==digest;data[name]=b
s=importlib.util.spec_from_file_location('suite_transport',ROOT/'work/ipad-automation/native-control-usb/transport.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);c=m.MadeiraDocuments(allow_guarded_rename=True);c.directory(folder)
targets=[(folder+'/'+n,b)for n,b in data.items()if n.endswith('.exe')]
for who in ['mythic','mobile','madeira']:
 d='/Documents/wine/drive_c/users/'+who+'/Desktop';info=c.info(d)
 if info is not None and info.get(b'st_ifmt')==b'S_IFDIR':
  c.directory(d);targets.append((d+'/Wine IO checks.lnk',data['Wine IO checks.lnk']))
assert len(targets)>4
for p,b in targets:
 c.collision(p)
 if c.info(p)is not None:assert c.read(p,exact=len(b))==b,'Different existing file preserved'
for p,b in targets:
 if c.info(p)is None:c.publish(p,b)
 assert c.read(p,exact=len(b))==b
report={'readback_verified':True,'files':[p for p,b in targets],'launched':False,'game_files_changed':False}
(H/'suite-deployment.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
