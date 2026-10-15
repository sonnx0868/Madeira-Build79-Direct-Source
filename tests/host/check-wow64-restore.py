#!/usr/bin/env python3
"""Verify addon pins/bytes and refuse paths that could replace native inputs."""
from pathlib import Path
import hashlib,importlib.util,io,json,stat,tempfile,zipfile
from unittest.mock import patch
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('wow_restore',root/'scripts/restore-wow64-addon.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
digest=lambda x:hashlib.sha256(x).hexdigest()
temp_root=Path(tempfile.gettempdir()).resolve()
with tempfile.TemporaryDirectory(prefix='madeira-addon-restore-',dir=temp_root) as tmp:
    checkout=Path(tmp).resolve();assert checkout.parent==temp_root and checkout.name.startswith('madeira-addon-restore-')
    arm=checkout/'app/Madeira/aarch64-windows';arm.mkdir(parents=True)
    companions={}
    for name in ['xtajit.dll','wow64.dll','wow64win.dll']:
        data=('tracked '+name).encode();(arm/name).write_bytes(data);companions[name]={'sha256':digest(data)}
    core=checkout/'app/Madeira/arm64ec-windows/ntdll.dll';core.parent.mkdir();core.write_bytes(b'native-preserve')
    data=b'i386 fixture';receipt={'source_commit':'producer','wine_commit':'wine-pin','fex_commit':'fex-pin',
        'arm64_prebuilt':companions,'i386':{'ntdll.dll':{'bytes':len(data),'sha256':digest(data)}}}
    def archive(extra=None,record=None):
        out=io.BytesIO()
        with zipfile.ZipFile(out,'w') as z:
            z.writestr('build/wine-i386/addon-receipt.json',json.dumps(record or receipt))
            z.writestr('app/Madeira/i386-windows/ntdll.dll',data)
            if extra:
                z.writestr(*extra)
        path=checkout/'addon.zip';path.write_bytes(out.getvalue());return path,digest(out.getvalue())
    def fake_git(args,**kwargs):
        return 'wine-pin' if Path(args[2]).name=='wine' else 'fex-pin'
    def reject(path,sha):
        try:module.restore(checkout,path,sha)
        except ValueError:pass
        else:raise AssertionError('unsafe addon accepted')
        assert core.read_bytes()==b'native-preserve'
    with patch.object(module.subprocess,'check_output',fake_git):
        path,sha=archive();module.restore(checkout,path,sha)
        assert (checkout/'app/Madeira/i386-windows/ntdll.dll').read_bytes()==data
        reject(path,'0'*64)
        for name in ['../outside','app/Madeira/i386-windows/../../arm64ec-windows/ntdll.dll',
                     'app/Madeira/arm64ec-windows/ntdll.dll','C:/outside','/outside']:
            path,sha=archive((name,b'bad'));reject(path,sha)
        symlink=zipfile.ZipInfo('app/Madeira/i386-windows/link.dll');symlink.create_system=3;symlink.external_attr=(stat.S_IFLNK|0o777)<<16
        path,sha=archive((symlink,b'../arm64ec-windows/ntdll.dll'));reject(path,sha)
        record=dict(receipt,wine_commit='other-pin');path,sha=archive(record=record);reject(path,sha)
        (arm/'xtajit.dll').write_bytes(b'wrong companion');path,sha=archive();reject(path,sha)
print('PASS: WoW64 addon SHA/source pins, companion hashes, path traversal, native overwrite and ZIP symlink rejection')
