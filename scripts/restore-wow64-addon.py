#!/usr/bin/env python3
"""Restore a verified i386 addon without overwriting any native 64-bit input."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,json,stat,subprocess,zipfile

def restore(root,archive,expected):
    root=root.resolve();raw=archive.read_bytes()
    if hashlib.sha256(raw).hexdigest()!=expected:raise ValueError('WoW64 addon SHA-256 mismatch')
    with zipfile.ZipFile(archive) as z:
        if z.testzip() is not None:raise ValueError('corrupt addon ZIP')
        if sum(i.file_size for i in z.infolist())>1024*1024*1024:raise ValueError('addon exceeds 1 GiB')
        entries={};folded=set()
        for i in z.infolist():
            path=PurePosixPath(i.filename)
            if path.is_absolute() or '..' in path.parts or '\\' in i.filename:raise ValueError('unsafe addon path')
            if stat.S_ISLNK(i.external_attr>>16):raise ValueError('addon symlink rejected')
            if i.is_dir():continue
            name=i.filename
            if not (name.startswith('app/Madeira/i386-windows/') or name=='build/wine-i386/addon-receipt.json'):
                raise ValueError('addon may not replace 64-bit inputs: '+name)
            if name.lower() in folded:raise ValueError('duplicate addon path')
            folded.add(name.lower());entries[name]=i
        receipt=json.loads(z.read('build/wine-i386/addon-receipt.json'))
        git=lambda sub:subprocess.check_output(['git','-C',str(root/sub),'rev-parse','HEAD'],text=True).strip()
        if receipt['wine_commit']!=git('wine') or receipt['fex_commit']!=git('FEX'):raise ValueError('addon source pins do not match consumer')
        if set(receipt['arm64_prebuilt'])!={'xtajit.dll','wow64.dll','wow64win.dll'}:raise ValueError('incomplete ARM64 companion identity')
        for name,info in receipt['arm64_prebuilt'].items():
            if '/' in name or '\\' in name:raise ValueError('invalid ARM64 module name')
            path=root/'app/Madeira/aarch64-windows'/name
            if hashlib.sha256(path.read_bytes()).hexdigest()!=info['sha256']:raise ValueError('ARM64 WoW64 companion differs: '+name)
        for name,info in receipt['i386'].items():
            if '/' in name or '\\' in name:raise ValueError('invalid i386 module name')
            data=z.read('app/Madeira/i386-windows/'+name)
            if len(data)!=info['bytes'] or hashlib.sha256(data).hexdigest()!=info['sha256']:raise ValueError('i386 payload differs: '+name)
        for name,i in entries.items():
            dest=root.joinpath(*PurePosixPath(name).parts)
            if not dest.resolve().is_relative_to(root):raise ValueError('addon target escapes checkout')
            if dest.is_symlink() or any(parent.is_symlink() for parent in dest.parents if parent!=root and root in parent.parents):
                raise ValueError('addon target traverses an existing symlink')
            dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(z.read(i))
    print('Restored '+str(len(receipt['i386']))+' hash-verified i386 modules from '+receipt['source_commit'])
    return receipt

def main():
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--archive',type=Path,required=True);p.add_argument('--sha256',required=True)
    a=p.parse_args();restore(a.root,a.archive,a.sha256)
if __name__=='__main__':main()
