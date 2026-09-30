"""Build 74 on build 73, with verified XAudio 2.7 and no destructive repair."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import gzip
import hashlib
import io
import plistlib
import subprocess
import sys
import tarfile

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'source'
OUT=HERE.parents[1]/'outputs/release-build74/Madeira.ipa'
BASE=HERE.parents[1]/'outputs/Madeira-Light-Control-Row.ipa'
EXPECTED='f8636ccc37bbfc84107992b60e4226eb7237417112267ed64b08f72983ac4314'
payload=(HERE/'directx-audio/x64/XAudio2_7.dll').read_bytes()
assert hashlib.sha256(payload).hexdigest()==EXPECTED
assert hashlib.sha256(BASE.read_bytes()).hexdigest()=='057fa81f4e52668bdec03fced72379caf63335234c9c4f3e3cdf575b5aab9dbb'
sys.path.insert(0,str(SOURCE))
from verify_native_link import symbols, unresolved_owned
def run(*args):subprocess.run([str(x) for x in args],check=True)
for sanitized in (False,True):
    test=HERE/('test_audio_dependency_sanitized' if sanitized else 'test_audio_dependency')
    extra=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if sanitized else []
    run('clang','-std=c11','-O1','-Wall','-Wextra','-Werror',*extra,'-I',SOURCE,SOURCE/'test_audio_dependency.c','-o',test)
    subprocess.run([str(test)],check=True,env={**__import__('os').environ,'ASAN_OPTIONS':'detect_leaks=0'})
resource=subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()
flags=['-target','arm64-apple-ios17.0','-ffreestanding','-O2','-Wall','-Wextra','-Werror',
       '-nostdinc','-isystem',Path(resource)/'include','-I',SOURCE,'-fPIC','-fobjc-arc','-fblocks','-fno-math-errno']
for name in ('MadeiraIPadUI','AudioDependency'):
    run('clang',*flags,'-c',SOURCE/(name+'.m'),'-o',HERE/(name+'-AudioBundled.o'))
library=HERE/'MadeiraIPadUI-AudioBundled.dylib'
run('ld64.lld','-dylib','-arch','arm64','-platform_version','ios','17.0','17.0',
    '-undefined','dynamic_lookup','-install_name','@executable_path/Frameworks/MadeiraIPadUI.dylib',
    '-needed_library',HERE/'MadeiraControllerInput.dylib','-o',library,
    HERE/'MadeiraIPadUI-AudioBundled.o',HERE/'AudioDependency-AudioBundled.o',
    HERE/'ResolutionInterpose.o',HERE/'TouchControls.o',HERE/'AudioDiagnostics.o')
defined=symbols(library,'--defined-only')|symbols(HERE/'MadeiraControllerInput.dylib','--defined-only')
assert not unresolved_owned(symbols(library,'--undefined-only'),defined)
assert '_MadeiraAudioProvision' in defined and '_MadeiraAudioProperty' in defined
assert b'IOBufferDuration\0' in library.read_bytes() and b'ioBufferDuration\0' not in library.read_bytes()
app='Payload/Madeira.app/'
ui=app+'Frameworks/MadeiraIPadUI.dylib'
info=app+'Info.plist'
template=app+'prefix-template.tar.gz'
legal=app+'legal/ResolutionSource/'
replacement={ui:library.read_bytes(),app+'AudioSupport/xaudio2_7.dll':payload}
for name in ('MadeiraIPadUI.m','AudioDependency.m','AudioDependency.h','test_audio_dependency.c'):
    replacement[legal+name]=(SOURCE/name).read_bytes()
replacement[legal+'build_audio_bundled.py']=Path(__file__).read_bytes()
OUT.parent.mkdir(exist_ok=True)
with ZipFile(BASE) as old:
    assert old.read(app+'Frameworks/MadeiraControllerInput.dylib')==(HERE/'MadeiraControllerInput.dylib').read_bytes()
    # Only two scoped audio provisioning additions to the existing UI source.
    original=old.read(legal+'MadeiraIPadUI.m').decode()
    expected=original.replace('#include "ControllerIntegration.inc"\n','#include "ControllerIntegration.inc"\nextern void MadeiraAudioProvision(void);\n',1)
    expected=expected.replace('static void configureRuntime(void) {\n','static void configureRuntime(void) {\n    MadeiraAudioProvision();\n',1)
    expected=expected.replace('    controllerTryStart();\n','    controllerTryStart();\n    MadeiraAudioProvision();\n',1)
    assert expected.encode()==replacement[legal+'MadeiraIPadUI.m']
    metadata=plistlib.loads(old.read(info))
    assert metadata['CFBundleVersion']=='73' and metadata['CFBundleDisplayName']=='Madeira'
    metadata['CFBundleVersion']='74'
    replacement[info]=plistlib.dumps(metadata,fmt=plistlib.FMT_BINARY,sort_keys=True)
    buffer=io.BytesIO()
    with tarfile.open(fileobj=io.BytesIO(old.read(template)),mode='r:gz') as src, tarfile.open(fileobj=buffer,mode='w',format=tarfile.PAX_FORMAT) as dst:
        members=src.getmembers()
        assert not any(m.name.lower().endswith('/xaudio2_7.dll') for m in members)
        registry=src.extractfile('prefix/system.reg').read()
        assert b'5A508685-A254-4FBA-9B82-9A24B00306AF' in registry and b'xaudio2_7.dll' in registry
        for member in members:dst.addfile(member,src.extractfile(member) if member.isfile() else None)
        member=tarfile.TarInfo('prefix/drive_c/windows/system32/xaudio2_7.dll')
        member.size=len(payload);member.mode=0o600;member.mtime=0
        dst.addfile(member,io.BytesIO(payload))
    replacement[template]=gzip.compress(buffer.getvalue(),mtime=0)
    # Preserve the exact embedded vendor end-user terms without accepting them.
    vendor=(HERE/'directx-audio/license-inspection/dsetup32.dll').read_bytes()
    start=vendor.index(b'MICROSOFT SOFTWARE LICENSE TERMS\r\nMICROSOFT DIRECTX END USER RUNTIME')
    end=vendor.index(b'\x00',start)
    replacement[app+'AudioSupport/Microsoft-DirectX-End-User-Terms.txt']=vendor[start:end]
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in old.infolist():new.writestr(item,replacement.get(item.filename,old.read(item)))
        for name,data in replacement.items():
            if name not in old.namelist():new.writestr(name,data)
with ZipFile(OUT) as check,ZipFile(BASE) as old:
    assert check.testzip() is None
    for name in old.namelist():assert check.read(name)==replacement.get(name,old.read(name)),name
    assert check.read(app+'Madeira')==old.read(app+'Madeira')
    assert hashlib.sha256(check.read(app+'AudioSupport/xaudio2_7.dll')).hexdigest()==EXPECTED
    with tarfile.open(fileobj=io.BytesIO(check.read(template)),mode='r:gz') as new,tarfile.open(fileobj=io.BytesIO(old.read(template)),mode='r:gz') as src:
        assert len(new.getmembers())==len(src.getmembers())+1
        for m in src.getmembers():
            n=new.getmember(m.name)
            assert (m.type,m.linkname,m.mode,m.size)==(n.type,n.linkname,n.mode,n.size)
            if m.isfile():assert src.extractfile(m).read()==new.extractfile(n).read(),m.name
        assert new.extractfile('prefix/drive_c/windows/system32/xaudio2_7.dll').read()==payload
print('PASS: build74 DLL digest, fresh prefix and existing-prefix repair tests, native linking, unchanged main executable/controller/resources, ZIP integrity')
print(OUT)
print('IPA SHA256:',hashlib.sha256(OUT.read_bytes()).hexdigest())
