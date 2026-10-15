#!/usr/bin/env python3
"""Record the D3D11 query fix's source and compiled PE identity."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
sources=['patches/dxmt-query-park.patch','dxmt/src/d3d11/d3d11_context_imm.cpp',
         'dxmt/src/d3d11/d3d11_query.hpp','dxmt/src/dxmt/dxmt_command_queue.hpp']
def digest(data):return hashlib.sha256(data).hexdigest()
def main():
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--stamp',action='store_true');p.add_argument('--base',type=Path)
    a=p.parse_args();r=a.root.resolve();data=(r/'app/Madeira/arm64ec-windows/d3d11.dll').read_bytes()
    assert data[:2]==b'MZ';off=struct.unpack_from('<I',data,60)[0]
    assert data[off:off+4]==b'PE\0\0' and struct.unpack_from('<H',data,off+24)[0]==0x20b,'not PE64'
    machine=struct.unpack_from('<H',data,off+4)[0]
    assert machine in (0x8664,0xa641),'unsupported EC container'
    sections=off+24+struct.unpack_from('<H',data,off+20)[0]
    def at(rva,n):
        for i in range(struct.unpack_from('<H',data,off+6)[0]):
            va,size,raw=struct.unpack_from('<III',data,sections+i*40+12)
            if va<=rva and n<=size and rva-va<=size-n and raw+rva-va+n<=len(data):return raw+rva-va
        raise ValueError('RVA outside PE data')
    imagebase=struct.unpack_from('<Q',data,off+24+24)[0]
    loadrva,loadsize=struct.unpack_from('<II',data,off+24+112+10*8)
    assert loadsize>=208,'missing EC load configuration'
    config=at(loadrva,208);chpe=struct.unpack_from('<Q',data,config+200)[0]
    assert chpe>=imagebase,'missing ARM64EC hybrid metadata'
    metadata=at(chpe-imagebase,12);version,codemap,count=struct.unpack_from('<III',data,metadata)
    assert version==1 and 0<count<=16384,'invalid ARM64EC metadata'
    ranges=at(codemap,count*8)
    assert any(struct.unpack_from('<I',data,ranges+i*8)[0]&3==1 for i in range(count)),'no ARM64EC code range'

    assert b'query-park-v1' in data and b'MTLSharedEvent_waitUntilSignaledValue' in data
    result={'sha256':digest(data),'sources':{name:digest((r/name).read_bytes().replace(b'\r\n',b'\n')) for name in sources}}
    receipt=r/'build/dxmt-pe/query-park.json'
    if a.stamp:
        result['producer_commit']=subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip()
        if a.base:result['base_dll_sha256']=digest(a.base.read_bytes())
        receipt.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    else:
        expected=json.loads(receipt.read_text(encoding='utf-8'))
        for key,value in result.items():assert expected[key]==value,('stale D3D11 query identity',key)
    print('PASS: ARM64EC D3D11 query parking source, PE marker and Metal event wait import match')
if __name__=='__main__':main()
