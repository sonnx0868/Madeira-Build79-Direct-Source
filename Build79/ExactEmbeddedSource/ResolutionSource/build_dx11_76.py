"""Package verified guest-only DX11/Metal optimizations; preserve game defaults.

Input is the successful private CI artifact, not arbitrary downloaded DLLs.
An IPA made here is unsigned/signable, not an already signed public release.
"""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import json
import plistlib
import re
import subprocess
import sys
import tarfile

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
SOURCE=HERE/'source'
DX11=HERE.parent/'dx11-opt'
ARTIFACT=DX11/'ci-final'
sys.path.insert(0,str(DX11/'control/.github/dx11-opt'))
from verify_pe import verify as verify_pe
BASE=ROOT/'outputs/release-build75/Madeira.ipa'
OUT=ROOT/'outputs/release-build76/Madeira.ipa'
assert hashlib.sha256(BASE.read_bytes()).hexdigest()=='03aa3218c808da1ebfb8e5280a3c1bc1b83ecfd8cb357a90d07544fc5fcb309b'
report=json.loads((ARTIFACT/'report.json').read_text())
manifest=json.loads((DX11/'patch-manifest.json').read_text())
assert report['status']=='success' and report['patch']==manifest
sys.path.insert(0,str(SOURCE))
from verify_native_link import symbols, unresolved_owned
def run(*args): subprocess.run([str(a) for a in args],check=True)
def sha(data): return hashlib.sha256(data).hexdigest()
def inspect(path):
    return subprocess.check_output(['llvm-readobj','--file-headers','--coff-imports','--coff-exports',str(path)],text=True)
def exports(text): return re.findall(r'Ordinal: (\d+)\s+Name: ([^\n]+)',text)
def imports(text):
    return {name:re.findall(r'Symbol: ([^\s]+)',body) for name,body in
        re.findall(r'Import \{\s+Name: ([^\n]+)\n(.*?)\n\}',text,re.S)}
replacement={}
app='Payload/Madeira.app/'
with ZipFile(BASE) as baseline:
    for arch,machine in [('aarch64',0xaa64),('arm64ec',0xa641)]:
        provider=HERE/'audio-build74-signed-app'/(arch+'-windows')/'winemetal.dll'
        assert baseline.read(app+arch+'-windows/winemetal.dll')==provider.read_bytes()
        provider_exports={name for _,name in exports(inspect(provider))}
        for name in ('dxgi.dll','d3d11.dll'):
            path=ARTIFACT/arch/name
            data=path.read_bytes()
            assert sha(data)==report['architectures'][arch][name]
            verify_pe(data,arch)
            old=HERE/('renderer-baseline76-'+arch+'-'+name)
            old.write_bytes(baseline.read(app+arch+'-windows/'+name))
            old_text,new_text=inspect(old),inspect(path)
            assert exports(old_text)==exports(new_text),(arch,name,'export ABI changed')
            before,after=imports(old_text),imports(new_text)
            assert {n.lower() for n in after}<={n.lower() for n in before},(arch,name,'new DLL dependency')
            for library,names in after.items():
                if library.lower()=='winemetal.dll':
                    assert set(names)<=provider_exports,(arch,name,'unsupported provider import')
            replacement[app+arch+'-windows/'+name]=data

resource=subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()
flags=['-target','arm64-apple-ios17.0','-ffreestanding','-O2','-Wall','-Wextra','-Werror',
       '-nostdinc','-isystem',Path(resource)/'include','-I',SOURCE,'-fPIC','-fobjc-arc','-fblocks','-fno-math-errno']
run('clang',*flags,'-c',SOURCE/'MadeiraIPadUI.m','-o',HERE/'MadeiraIPadUI-DX11-76.o')
library=HERE/'MadeiraIPadUI-DX11-76.dylib'
run('ld64.lld','-dylib','-arch','arm64','-platform_version','ios','17.0','17.0',
    '-undefined','dynamic_lookup','-install_name','@executable_path/Frameworks/MadeiraIPadUI.dylib',
    '-needed_library',HERE/'MadeiraControllerInput.dylib','-o',library,
    HERE/'MadeiraIPadUI-DX11-76.o',HERE/'AudioDependency-AudioBundled.o',
    HERE/'ResolutionInterpose.o',HERE/'TouchControls.o',HERE/'AudioDiagnostics-Performance75.o')
defined=symbols(library,'--defined-only')|symbols(HERE/'MadeiraControllerInput.dylib','--defined-only')
assert not unresolved_owned(symbols(library,'--undefined-only'),defined)
replacement[app+'Frameworks/MadeiraIPadUI.dylib']=library.read_bytes()
replacement[app+'legal/ResolutionSource/MadeiraIPadUI.m']=(SOURCE/'MadeiraIPadUI.m').read_bytes()
replacement[app+'legal/ResolutionSource/build_dx11_76.py']=Path(__file__).read_bytes()

# Full corresponding modified renderer source, with build controls and tests.
# This is a generated artifact; no game data or signing credentials are added.
OUT.parent.mkdir(exist_ok=True)
archive=OUT.parent/'Madeira-DX11-Source.tar.gz'
with tarfile.open(archive,'w:gz') as tar:
    tar.add(DX11/'source',arcname='dxmt')
    for name in ('dx11-hotpath.patch','patch-manifest.json','test.py','test_dynamic.cpp','test_census.cpp','test_checkpoint.py','test_verify_pe.py','export_patch.py','prepare_control.py','NOTES.md'):
        tar.add(DX11/name,arcname='optimization/'+name)
    tar.add(DX11/'control/.github/dx11-opt',arcname='optimization/ci',
        filter=lambda item: None if '__pycache__' in item.name else item)
    for directory in ('dx11-cache-research','dx11-memory-research','dx11-metal-encoder-research'):
        for path in sorted((HERE.parent/directory).rglob('*')):
            if path.is_file() and path.suffix in ('.md','.json','.cpp','.hpp','.h','.py','.sh','.patch','.inc'):
                tar.add(path,arcname='optimization/'+directory+'/'+str(path.relative_to(HERE.parent/directory)))
replacement[app+'legal/DX11-Optimization-Source.tar.gz']=archive.read_bytes()
replacement[app+'legal/DX11-Optimization-Build.json']=json.dumps(report,indent=2).encode()
with ZipFile(BASE) as old:
    info=plistlib.loads(old.read(app+'Info.plist'))
    assert info['CFBundleVersion']=='75' and info['CFBundleDisplayName']=='Madeira'
    info['CFBundleVersion']='76'
    replacement[app+'Info.plist']=plistlib.dumps(info,fmt=plistlib.FMT_BINARY,sort_keys=True)
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in old.infolist(): new.writestr(item,replacement.get(item.filename,old.read(item)))
        for name,data in replacement.items():
            if name not in old.namelist():new.writestr(name,data)
with ZipFile(BASE) as old,ZipFile(OUT) as new:
    assert new.testzip() is None
    for name in old.namelist():assert new.read(name)==replacement.get(name,old.read(name)),name
    for name in ('Madeira','prefix-template.tar.gz','Frameworks/MadeiraControllerInput.dylib'):
        assert new.read(app+name)==old.read(app+name)
print('PASS: two renderer architectures, export/provider ABI, native UI link, ZIP integrity, unchanged main/controller/prefix/other resources')
print(OUT)
print('SHA256:',sha(OUT.read_bytes()))
