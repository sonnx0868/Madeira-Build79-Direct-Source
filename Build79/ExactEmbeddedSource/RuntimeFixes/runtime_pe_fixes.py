"""Exact-release transformations: correct guard BOOL handling and RPC labels.

No guard request, pool size, permissions, IAT binding, ABI or layout changes.
Reject unknown inputs. The corresponding reviewed source patch is bundled.
"""
import hashlib
import struct

XT_SHA='74032ec4c38a6da83e3dbe07a0a0abc62a58a823c4a193cc75dd24716239bfdb'
NT_SHA='e43ae5a9bc90a02c3c01e5a451e9ab7f5570b2735b93215284868a279254c27f'

def sha(data): return hashlib.sha256(data).hexdigest()

def sections(data):
    pe=struct.unpack_from('<I',data,0x3c)[0]
    assert data[pe:pe+4]==b'PE\0\0'
    count=struct.unpack_from('<H',data,pe+6)[0]
    optional_size=struct.unpack_from('<H',data,pe+20)[0]
    opt=pe+24
    assert struct.unpack_from('<H',data,opt)[0]==0x20b
    imagebase=struct.unpack_from('<Q',data,opt+24)[0]
    result=[]
    for n in range(count):
        pos=opt+optional_size+40*n
        name=data[pos:pos+8].rstrip(b'\0')
        virtual_size,rva,raw_size,raw=struct.unpack_from('<IIII',data,pos+8)
        flags=struct.unpack_from('<I',data,pos+36)[0]
        assert raw+raw_size<=len(data)
        result.append((name,rva,raw,raw_size,flags))
    return imagebase,result

def offset(data,va):
    base,table=sections(data)
    for name,rva,raw,size,flags in table:
        if rva<=va-base<rva+size:
            assert flags&0x20000000, 'instruction outside executable section'
            return raw+va-base-rva
    raise ValueError('Unmapped instruction VA')

def guard_fix(original):
    if sha(original)!=XT_SHA: raise ValueError('Unknown xtajit64 release')
    words={0x18001b4d4:(0xaa1f03e3,0x9100c3e3),  # x3 -> owned DWORD at sp+0x30
           0x18001b4dc:(0x34000100,0x35000100),  # success skips failure log
           0x1800160ac:(0xaa1f03e3,0x910083e3),  # x3 -> owned DWORD at sp+0x20
           0x1800160b4:(0x34000100,0x35000100)}
    result=bytearray(original)
    allowed=set()
    for va,(before,after) in words.items():
        pos=offset(original,va)
        assert struct.unpack_from('<I',original,pos)[0]==before
        struct.pack_into('<I',result,pos,after)
        allowed.update(range(pos,pos+4))
    assert len(result)==len(original)
    assert all(a==b or i in allowed for i,(a,b) in enumerate(zip(original,result)))
    assert sections(result)==sections(original)
    return bytes(result)

def rpc_labels(original):
    if sha(original)!=NT_SHA: raise ValueError('Unknown ntdll release')
    before=b'POOL STALE  <== executing copy sees the OLD x64 thunk'
    after=b'PRE-SYNC SNAPSHOT (call-time unverified)'
    result=bytearray(original)
    pos=original.find(before+b'\0')
    assert pos>=0 and original.count(before+b'\0')==1
    _,table=sections(original)
    assert any(raw<=pos and pos+len(before)<raw+size and not(flags&0x20000000)
               for name,rva,raw,size,flags in table)
    assert len(after)<=len(before)
    result[pos:pos+len(before)]=after+b'\0'*(len(before)-len(after))
    assert len(result)==len(original) and sections(result)==sections(original)
    assert all(a==b or pos<=i<pos+len(before) for i,(a,b) in enumerate(zip(original,result)))
    return bytes(result)
