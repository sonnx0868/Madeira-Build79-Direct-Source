"""Clock-only checkpoint. Does not claim guard/RPC/content fixes are included."""
import hashlib
import json
import plistlib
import subprocess
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import sys

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'source'
ROOT=HERE.parents[1]
BASE=ROOT/'outputs/release-build76/Madeira.ipa'
OUT=ROOT/'outputs/checkpoints/build77-clock/Madeira.ipa'
def sha(data): return hashlib.sha256(data).hexdigest()
def run(*args): subprocess.run([str(a) for a in args],check=True)
assert sha(BASE.read_bytes())=='d7936d4da5c23e2948035849ad65c2ed32f223af5b76df20e25ca8709b00b166'
sys.path.insert(0,str(SOURCE))
from verify_native_link import symbols, unresolved_owned
run('clang','-std=c11','-O2','-Wall','-Wextra','-Werror',SOURCE/'test_runtime_clock_policy.c',
    '-o',HERE/'test-runtime-clock77')
run(HERE/'test-runtime-clock77')
resource=subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()
flags=['-target','arm64-apple-ios17.0','-ffreestanding','-O2','-Wall','-Wextra','-Werror',
       '-nostdinc','-isystem',Path(resource)/'include','-I',SOURCE,'-fPIC','-fobjc-arc','-fblocks','-fno-math-errno']
obj=HERE/'MadeiraIPadUI-Runtime77.o'
library=HERE/'MadeiraIPadUI-Runtime77.dylib'
run('clang',*flags,'-c',SOURCE/'MadeiraIPadUI.m','-o',obj)
run('ld64.lld','-dylib','-arch','arm64','-platform_version','ios','17.0','17.0',
    '-undefined','dynamic_lookup','-install_name','@executable_path/Frameworks/MadeiraIPadUI.dylib',
    '-needed_library',HERE/'MadeiraControllerInput.dylib','-o',library,obj,
    HERE/'AudioDependency-AudioBundled.o',HERE/'ResolutionInterpose.o',HERE/'TouchControls.o',
    HERE/'AudioDiagnostics-Performance75.o')
defined=symbols(library,'--defined-only')|symbols(HERE/'MadeiraControllerInput.dylib','--defined-only')
assert not unresolved_owned(symbols(library,'--undefined-only'),defined)
assert b'MADEIRA_USD_TIME\0' in library.read_bytes()
app='Payload/Madeira.app/'
legal=app+'legal/ResolutionSource/'
replacement={app+'Frameworks/MadeiraIPadUI.dylib':library.read_bytes()}
for name in ('MadeiraIPadUI.m','RuntimeClockPolicy.h','test_runtime_clock_policy.c'):
    replacement[legal+name]=(SOURCE/name).read_bytes()
replacement[legal+'build_runtime77.py']=Path(__file__).read_bytes()
OUT.parent.mkdir(parents=True,exist_ok=True)
with ZipFile(BASE) as old:
    assert old.read(app+'Frameworks/MadeiraControllerInput.dylib')==(HERE/'MadeiraControllerInput.dylib').read_bytes()
    info=plistlib.loads(old.read(app+'Info.plist'))
    assert info['CFBundleVersion']=='76' and info['CFBundleDisplayName']=='Madeira'
    info['CFBundleVersion']='77'
    replacement[app+'Info.plist']=plistlib.dumps(info,fmt=plistlib.FMT_BINARY,sort_keys=True)
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in old.infolist():new.writestr(item,replacement.get(item.filename,old.read(item)))
        for name,data in replacement.items():
            if name not in old.namelist():new.writestr(name,data)
with ZipFile(BASE) as old,ZipFile(OUT) as new:
    assert new.testzip() is None
    for name in old.namelist():assert new.read(name)==replacement.get(name,old.read(name)),name
    for name,data in replacement.items():assert new.read(name)==data,name
    for name in ('Madeira','prefix-template.tar.gz','Frameworks/MadeiraControllerInput.dylib'):
        assert new.read(app+name)==old.read(app+name)
manifest={'checkpoint':77,'status':'unsigned clock-only; not device-tested without flag',
    'base_sha256':sha(BASE.read_bytes()),'ipa_sha256':sha(OUT.read_bytes()),
    'changes':{name:sha(data) for name,data in replacement.items()},
    'guard_fix_included':False,'rpc_fix_included':False,'game_content_modified':False,
    'clock_off_switch':'Documents/madeira-usd-time.txt containing 0; full restart'}
(OUT.parent/'checkpoint.json').write_text(json.dumps(manifest,indent=2))
print(json.dumps(manifest,indent=2))
