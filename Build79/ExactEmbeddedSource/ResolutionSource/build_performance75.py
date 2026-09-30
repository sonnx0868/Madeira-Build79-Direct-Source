"""Scoped diagnostic-overhead reduction; no game or guest-runtime patches."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import os
import plistlib
import subprocess
import sys

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'source'
ROOT=HERE.parents[1]
BASE=ROOT/'outputs/release-build74/Madeira.ipa'
OUT=ROOT/'outputs/release-build75/Madeira.ipa'
assert hashlib.sha256(BASE.read_bytes()).hexdigest()=='7c8a0dd933bfa065e7c0c263809366e2a202a6bb6b8449162b769096430c1d6c'
sys.path.insert(0,str(SOURCE))
from verify_native_link import symbols, unresolved_owned
def run(*args): subprocess.run([str(x) for x in args],check=True)
for name in ('telemetry','meter','period','clock'):
    for sanitized in (False,True):
        target=HERE/f'test_audio_{name}_perf75_{int(sanitized)}'
        flags=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if sanitized else []
        run('clang','-std=c11','-O2','-Wall','-Wextra','-Werror',*flags,SOURCE/f'test_audio_{name}.c','-o',target)
        subprocess.run([str(target)],check=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
resource=subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()
flags=['-target','arm64-apple-ios17.0','-ffreestanding','-O2','-Wall','-Wextra','-Werror','-nostdinc','-isystem',Path(resource)/'include','-I',SOURCE,'-fPIC','-fobjc-arc','-fblocks','-fno-math-errno']
for name in ('MadeiraIPadUI','AudioDiagnostics'):
    extra=['-DMADEIRA_AUDIO_PERIOD_TEST=1','-DMADEIRA_AUDIO_DEVICE_CLOCK=0'] if name=='AudioDiagnostics' else []
    run('clang',*flags,*extra,'-c',SOURCE/(name+'.m'),'-o',HERE/(name+'-Performance75.o'))
# Also compile the optional clock variant so the telemetry bypass cannot regress it.
run('clang',*flags,'-DMADEIRA_AUDIO_DEVICE_CLOCK=1','-DMADEIRA_AUDIO_PERIOD_TEST=0','-c',SOURCE/'AudioDiagnostics.m','-o',HERE/'AudioDiagnostics-ClockCheck75.o')
library=HERE/'MadeiraIPadUI-Performance75.dylib'
run('ld64.lld','-dylib','-arch','arm64','-platform_version','ios','17.0','17.0','-undefined','dynamic_lookup','-install_name','@executable_path/Frameworks/MadeiraIPadUI.dylib','-needed_library',HERE/'MadeiraControllerInput.dylib','-o',library,HERE/'MadeiraIPadUI-Performance75.o',HERE/'AudioDependency-AudioBundled.o',HERE/'ResolutionInterpose.o',HERE/'TouchControls.o',HERE/'AudioDiagnostics-Performance75.o')
defined=symbols(library,'--defined-only')|symbols(HERE/'MadeiraControllerInput.dylib','--defined-only')
assert not unresolved_owned(symbols(library,'--undefined-only'),defined)
assert '_MadeiraAudioSetDiagnostics' in defined
assert b'IOBufferDuration\0' in library.read_bytes()
app='Payload/Madeira.app/'
legal=app+'legal/ResolutionSource/'
replacement={app+'Frameworks/MadeiraIPadUI.dylib':library.read_bytes()}
for name in ('MadeiraIPadUI.m','AudioDiagnostics.m','AudioTelemetry.h','test_audio_telemetry.c'):
    replacement[legal+name]=(SOURCE/name).read_bytes()
replacement[legal+'build_performance75.py']=Path(__file__).read_bytes()
OUT.parent.mkdir(exist_ok=True)
with ZipFile(BASE) as old:
    assert old.read(app+'Frameworks/MadeiraControllerInput.dylib')==(HERE/'MadeiraControllerInput.dylib').read_bytes()
    info=plistlib.loads(old.read(app+'Info.plist'))
    assert info['CFBundleVersion']=='74' and info['CFBundleDisplayName']=='Madeira'
    info['CFBundleVersion']='75'
    replacement[app+'Info.plist']=plistlib.dumps(info,fmt=plistlib.FMT_BINARY,sort_keys=True)
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in old.infolist(): new.writestr(item,replacement.get(item.filename,old.read(item)))
        for name,data in replacement.items():
            if name not in old.namelist():new.writestr(name,data)
with ZipFile(BASE) as old,ZipFile(OUT) as new:
    assert new.testzip() is None
    for name in old.namelist():assert new.read(name)==replacement.get(name,old.read(name)),name
    assert new.read(app+'Madeira')==old.read(app+'Madeira')
    assert new.read(app+'prefix-template.tar.gz')==old.read(app+'prefix-template.tar.gz')
print('PASS: audio tests/sanitizers, period/clock variants, native linking and unchanged executable/controller/prefix/dependencies/resources')
print(OUT)
print('SHA256:',hashlib.sha256(OUT.read_bytes()).hexdigest())
