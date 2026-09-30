"""Package exact tested runtime fixes without relinking or changing game files."""
import hashlib
import json
import plistlib
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from runtime_pe_fixes import guard_fix, rpc_labels, sha

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
BASE=ROOT/'outputs/checkpoints/build77-clock/Madeira.ipa'
OUT=ROOT/'outputs/checkpoints/build78-runtime/Madeira.ipa'
assert sha(BASE.read_bytes())=='b1c728420a33298a9db6d172e8461821c0c361aa26f002697575e96fc2eb8a3c'
app='Payload/Madeira.app/'
replacement={}
with ZipFile(BASE) as old:
    replacement[app+'arm64ec-windows/xtajit64.dll']=guard_fix(old.read(app+'arm64ec-windows/xtajit64.dll'))
    replacement[app+'arm64ec-windows/ntdll.dll']=rpc_labels(old.read(app+'arm64ec-windows/ntdll.dll'))
    info=plistlib.loads(old.read(app+'Info.plist'))
    assert info['CFBundleVersion']=='77' and info['CFBundleDisplayName']=='Madeira'
    info['CFBundleVersion']='78'
    replacement[app+'Info.plist']=plistlib.dumps(info,fmt=plistlib.FMT_BINARY,sort_keys=True)
    for name in ('runtime_pe_fixes.py','build_runtime78.py','test_runtime_pe_fixes.py'):
        replacement[app+'legal/RuntimeFixes/'+name]=(HERE/name).read_bytes()
    patch=HERE.parent/'runtime-guard-fix/windows-protect-contract.patch'
    replacement[app+'legal/RuntimeFixes/windows-protect-contract.patch']=patch.read_bytes()
    # Actual RPC relabel's matching C source; no classifier/binding rewrite.
    loader=(HERE.parent/'runtime-rpc-fix/baseline/wine/dlls/ntdll/loader.c').read_bytes()
    label=b'POOL STALE  <== executing copy sees the OLD x64 thunk'
    assert loader.count(label)==1
    replacement[app+'legal/RuntimeFixes/ntdll/loader.c']=loader.replace(label,b'PRE-SYNC SNAPSHOT (call-time unverified)')
    OUT.parent.mkdir(parents=True,exist_ok=True)
    with ZipFile(OUT,'w',compression=ZIP_DEFLATED) as new:
        for item in old.infolist():new.writestr(item,replacement.get(item.filename,old.read(item)))
        for name,data in replacement.items():
            if name not in old.namelist():new.writestr(name,data)
with ZipFile(BASE) as old,ZipFile(OUT) as new:
    assert new.testzip() is None
    for name in old.namelist():assert new.read(name)==replacement.get(name,old.read(name)),name
    for name,data in replacement.items():assert new.read(name)==data,name
    for name in ('Madeira','Frameworks/MadeiraIPadUI.dylib','Frameworks/MadeiraControllerInput.dylib','prefix-template.tar.gz'):
        assert new.read(app+name)==old.read(app+name)
manifest={'checkpoint':78,'ipa_sha256':sha(OUT.read_bytes()),'unsigned':True,'device_tested':False,
          'clock_default_included':True,'virtual_protect_contract_fix_included':True,
          'rpc_change':'snapshot label only; no binding behavior change',
          'hardware_guard_redesign_included':False,'game_content_modified':False,
          'changes':{name:sha(data) for name,data in replacement.items()}}
(OUT.parent/'checkpoint.json').write_text(json.dumps(manifest,indent=2))
print(json.dumps(manifest,indent=2))
