"""Shader-compatibility checkpoint; only guest renderer/resources change."""
import hashlib
import io
import json
import plistlib
import re
import struct
from pathlib import Path
import subprocess
import sys
import tarfile
from zipfile import ZipFile, ZIP_DEFLATED

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
DX11=HERE.parent/'dx11-opt'
CONTROL=DX11/'control/.github/dx11-opt'
ARTIFACT=DX11/'ci-shader79'
BASE=ROOT/'outputs/release-build78/Madeira.ipa'
OUT=ROOT/'outputs/checkpoints/build79-shader/Madeira.ipa'
sys.path.insert(0,str(CONTROL))
from verify_pe import verify as verify_pe
from verify_metallib import verify as verify_metallib
from verify_metallib import inspect as inspect_metallib
from shader_compat import POLICY
def sha(data):return hashlib.sha256(data).hexdigest()
def inspect(path):return subprocess.check_output(['llvm-readobj','--coff-imports','--coff-exports',str(path)],text=True)
def exports(text):return re.findall(r'Ordinal: (\d+)\s+Name: ([^\n]+)',text)
def imports(text):return {name:re.findall(r'Symbol: ([^\s]+)',body) for name,body in
    re.findall(r'Import \{\s+Name: ([^\n]+)\n(.*?)\n\}',text,re.S)}
assert sha(BASE.read_bytes())=='3ca9b32176d875c16f6f6d3eb6b26e312eef4ef1ec55bb23f62b74bc1749f5d7'
report=json.loads((ARTIFACT/'report.json').read_text())
assert report['status']=='success' and report['patch']==json.loads((DX11/'patch-manifest.json').read_text())
assert report['shaders']['policy']==POLICY
helper=(ARTIFACT/'shaders/dxmt_command.metallib').read_bytes()
assert sha(helper)==report['shaders']['metallib_sha256']
shader_inspection=verify_metallib(helper)
if (shader_inspection['platform']!=1 or shader_inspection['os']!=0x82
        or shader_inspection['os_version']!=[17,0]):
    raise ValueError('Helper metadata does not match the verified iOS17 compilation baseline')
if any(tuple(f['air_version'])>(2,6) for f in shader_inspection['functions']):
    raise ValueError('Helper AIR version exceeds the verified iOS17/MSL3.1 baseline')
app='Payload/Madeira.app/'
replacement={}
OUT.parent.mkdir(parents=True,exist_ok=True)
with ZipFile(BASE) as baseline:
    info=plistlib.loads(baseline.read(app+'Info.plist'))
    assert info['CFBundleVersion']=='78' and info['MinimumOSVersion']=='17.0'
    assert info['CFBundleDisplayName']=='Madeira'
    info['CFBundleVersion']='79'
    replacement[app+'Info.plist']=plistlib.dumps(info,fmt=plistlib.FMT_BINARY,sort_keys=True)
    old_dll=baseline.read(app+'arm64ec-windows/d3d11.dll')
    assert sha(old_dll)=='29bfa0f460d2bb5f8aa0e95ca7603920fe8c8f8d92ac20c17cfc262e299e0ec9'
    old_offset=2773000
    old_size,=struct.unpack_from('<Q',old_dll,old_offset+16)
    old_helper=old_dll[old_offset:old_offset+old_size]
    assert sha(old_helper)=='b7adb9ee81645ca165688330d35ddf48d3d459729e31b9d03806fb6176d67327'
    old_inventory={(x['name'],x['type']) for x in inspect_metallib(old_helper)['functions']}
    assert {(x['name'],x['type']) for x in shader_inspection['functions']}==old_inventory,'Helper functions changed'
    for arch in ('aarch64','arm64ec'):
        directory=OUT.parent/('abi-'+arch)
        directory.mkdir(exist_ok=True)
        provider=directory/'winemetal.dll'
        provider.write_bytes(baseline.read(app+arch+'-windows/winemetal.dll'))
        provider_exports={name for _,name in exports(inspect(provider))}
        for name in ('dxgi.dll','d3d11.dll'):
            path=ARTIFACT/arch/name
            data=path.read_bytes()
            assert sha(data)==report['architectures'][arch][name]
            verify_pe(data,arch)
            old=directory/name
            old.write_bytes(baseline.read(app+arch+'-windows/'+name))
            before,after=inspect(old),inspect(path)
            assert exports(before)==exports(after),(arch,name,'export ABI')
            bi,ai=imports(before),imports(after)
            assert {n.lower() for n in ai}<={n.lower() for n in bi},'New guest dependency'
            for library,symbols in ai.items():
                if library.lower()=='winemetal.dll':assert set(symbols)<=provider_exports,'Unsupported provider import'
            if name=='d3d11.dll':assert data.count(helper)==1,'Compatible helper not embedded exactly once'
            replacement[app+arch+'-windows/'+name]=data
    # Preserve complete corresponding C++ source; update build policy/checks.
    source_key=app+'legal/DX11-Optimization-Source.tar.gz'
    updated={}
    for name in ('build_renderer.py','shader_compat.py','verify_metallib.py'):
        updated['optimization/ci/'+name]=(CONTROL/name).read_bytes()
    updated['optimization/ci/build-dx11-opt.yml']=(DX11/'control/.github/workflows/build-dx11-opt.yml').read_bytes()
    for name in ('test_shader_compat.py','test_metallib.py'):
        updated['optimization/'+name]=(DX11/name).read_bytes()
    buffer=io.BytesIO()
    with tarfile.open(fileobj=io.BytesIO(baseline.read(source_key)),mode='r:gz') as old, tarfile.open(fileobj=buffer,mode='w:gz') as new:
        assert sha(old.extractfile('dxmt/src/dxmt/dxmt_command.metal').read())==report['shaders']['source_sha256'],'Helper source changed'
        names=set()
        for member in old.getmembers():
            assert member.name not in names,'Duplicate source member'
            names.add(member.name)
            if member.name in updated:
                data=updated[member.name]
                member.size=len(data)
                new.addfile(member,io.BytesIO(data))
            else:new.addfile(member,old.extractfile(member) if member.isfile() else None)
        for name,data in updated.items():
            if name in names:continue
            member=tarfile.TarInfo(name)
            member.size=len(data);member.mode=0o644;member.mtime=0
            new.addfile(member,io.BytesIO(data))
    replacement[source_key]=buffer.getvalue()
    replacement[app+'legal/DX11-Optimization-Build.json']=json.dumps(report,indent=2).encode()
    replacement[app+'legal/ResolutionSource/build_shader79.py']=Path(__file__).read_bytes()
    for name in ('dxmt_command.metallib','compatibility.json'):
        replacement[app+'legal/ShaderCompatibility/'+name]=(ARTIFACT/'shaders'/name).read_bytes()
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in baseline.infolist():new.writestr(item,replacement.get(item.filename,baseline.read(item)))
        for name,data in replacement.items():
            if name not in baseline.namelist():new.writestr(name,data)
with ZipFile(BASE) as old,ZipFile(OUT) as new:
    assert new.testzip() is None
    assert set(new.namelist())==set(old.namelist())|set(replacement)
    for name in old.namelist():assert new.read(name)==replacement.get(name,old.read(name)),name
    for name,data in replacement.items():assert new.read(name)==data,name
    for name in ('Madeira','prefix-template.tar.gz','Frameworks/MadeiraIPadUI.dylib',
                 'Frameworks/MadeiraControllerInput.dylib','AudioSupport/xaudio2_7.dll',
                 'arm64ec-windows/xtajit64.dll','arm64ec-windows/ntdll.dll'):
        assert new.read(app+name)==old.read(app+name),'Unrelated resource changed: '+name
result={'checkpoint':79,'ipa_sha256':sha(OUT.read_bytes()),'unsigned':True,'device_tested':False,
        'policy':POLICY,'shader_inspection':shader_inspection,'optimizations_preserved':True,
        'game_files_modified':False,'clock_audio_controller_preserved':True,
        'changes':{name:sha(data) for name,data in replacement.items()}}
(OUT.parent/'checkpoint.json').write_text(json.dumps(result,indent=2))
print(json.dumps({k:v for k,v in result.items() if k not in ('shader_inspection','changes')},indent=2))
print('PASS: 35 unchanged helpers; iOS17 / MSL3.1 / AIR2.6, both guest ABIs, archive integrity and unrelated resource preservation')
