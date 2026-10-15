#!/usr/bin/env python3
"""Validate i386 PE types and immediate DLL import closure before packaging."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess

def pe_info(data):
    def u16(off):return struct.unpack_from('<H',data,off)[0]
    def u32(off):return struct.unpack_from('<I',data,off)[0]
    if data[:2]!=b'MZ':raise ValueError('not a PE image')
    off=u32(60)
    if off<64 or off>len(data)-264 or data[off:off+4]!=b'PE\0\0':raise ValueError('invalid PE header')
    machine=u16(off+4);sections=u16(off+6);optional=u16(off+20)
    if not 0<sections<=96:raise ValueError('section count')
    magic=u16(off+24);dd=off+24+(96 if magic==0x10b else 112)
    if magic not in (0x10b,0x20b):raise ValueError('optional header')
    table=off+24+optional;maps=[]
    for i in range(sections):
        at=table+i*40
        maps.append((u32(at+12),u32(at+16),u32(at+20)))
    def rva(address,size):
        for va,n,raw in maps:
            if va<=address and size<=n and address-va<=n-size:
                start=raw+address-va
                if start+size>len(data):break
                return data[start:start+size]
        raise ValueError('RVA outside file-backed sections')
    imports=[];directory=u32(dd+8)
    if directory:
        for i in range(4096):
            desc=rva(directory+i*20,20)
            if not any(desc):break
            name=struct.unpack_from('<I',desc,12)[0];out=bytearray()
            for j in range(256):
                char=rva(name+j,1)[0]
                if not char:break
                out.append(char)
            else:raise ValueError('unterminated DLL name')
            imports.append(out.decode('ascii').lower())
        else:raise ValueError('unterminated import directory')
    return {'machine':machine,'imports':imports}

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--root',type=Path,required=True);parser.add_argument('--write',type=Path,required=True)
    args=parser.parse_args();root=args.root.resolve();folder=root/'app/Madeira/i386-windows'
    files={p.name.lower():p for p in folder.iterdir() if p.is_file() and p.name!='.gitkeep'}
    required=['ntdll.dll','kernel32.dll','kernelbase.dll','user32.dll','win32u.dll','gdi32.dll','msvcrt.dll','mscoree.dll',
              'combase.dll','ole32.dll','oleaut32.dll','advapi32.dll','shell32.dll','shlwapi.dll','ws2_32.dll',
              'apisetschema.dll','d3d9.dll','d3d9-emulated.dll','d3d11.dll','dxgi.dll','winemetal.dll']
    absent=[name for name in required if name not in files]
    if absent:raise SystemExit('Missing required i386 DLLs: '+', '.join(absent))
    receipt={};missing=[]
    for name,path in sorted(files.items()):
        if path.suffix.lower() not in ('.dll','.exe','.drv','.cpl','.acm','.ax','.ocx','.com'):continue
        data=path.read_bytes();info=pe_info(data)
        if info['machine']!=0x14c:raise SystemExit('Not i386: '+name)
        for dll in info['imports']:
            if dll.startswith(('api-ms-win-','ext-ms-win-')):continue
            if dll not in files:missing.append(name+' -> '+dll)
        receipt[name]={'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'imports':info['imports']}
    if missing:raise SystemExit('Missing immediate DLL imports:\n'+'\n'.join(missing))
    arm={}
    for name in ['xtajit.dll','wow64.dll','wow64win.dll']:
        p=root/'app/Madeira/aarch64-windows'/name;data=p.read_bytes();info=pe_info(data)
        if info['machine']!=0xaa64:raise SystemExit('Not ARM64: '+name)
        arm[name]={'sha256':hashlib.sha256(data).hexdigest(),'provenance':'unchanged tracked ARM64 WoW64-side module'}
    git=lambda *arg:subprocess.check_output(['git','-C',str(root),*arg],text=True).strip()
    result={'source_commit':git('rev-parse','HEAD'),'wine_commit':git('-C','wine','rev-parse','HEAD'),
            'fex_commit':git('-C','FEX','rev-parse','HEAD'),'i386':receipt,'arm64_prebuilt':arm,
            'scope':'PE architecture and immediate DLL import closure only; no device or Celeste execution.'}
    args.write.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('PASS: '+str(len(receipt))+' i386 PE modules, required renderer/runtime DLLs and immediate import closure')

if __name__=='__main__':main()
