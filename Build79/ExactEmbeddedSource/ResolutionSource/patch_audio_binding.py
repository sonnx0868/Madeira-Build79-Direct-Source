"""Scoped import redirection; reject anything except our inspected build 66."""
import hashlib
import struct
EXPECTED="524c59c1492217d5bbe6a17bf566d582fcc7e81982f90fad55c89692419798e9"
NAME=b"_AudioUnitSetProperty"
ALIAS=b"_MadeiraAudioProperty"
def patch_audio_binding(original):
    if hashlib.sha256(original).hexdigest()!=EXPECTED:raise ValueError("Audio probe requires exact build-66 executable")
    assert len(NAME)==len(ALIAS)
    start=0x4000
    assert struct.unpack_from("<I",original,start)[0]==0xfeedfacf
    offset=start+32; libraries=[]; symbols=fixups=None
    for _ in range(struct.unpack_from("<I",original,start+16)[0]):
        command,size=struct.unpack_from("<II",original,offset)
        if command in (0xc,0x80000018,0x8000001f,0x80000023):
            name=offset+struct.unpack_from("<I",original,offset+8)[0]
            libraries.append(original[name:original.index(0,name)])
        if command==2:symbols=struct.unpack_from("<IIII",original,offset+8)
        if command==0x80000034:fixups=struct.unpack_from("<II",original,offset+8)
        offset+=size
    assert libraries[36]==b"@executable_path/Frameworks/MadeiraIPadUI.dylib"
    assert symbols and fixups
    result=bytearray(original); allowed=set(); imported=[]; nlisted=[]
    header=start+fixups[0]
    version,_,imports_offset,names_offset,count,fmt,names_fmt=struct.unpack_from("<7I",original,header)
    assert version==0 and fmt==1 and names_fmt==0
    for i in range(count):
        word_at=header+imports_offset+4*i
        word=struct.unpack_from("<I",original,word_at)[0]
        name_at=header+names_offset+(word>>9)
        if original[name_at:original.index(0,name_at)]==NAME:
            ordinal=word&255
            assert libraries[ordinal-1].endswith(b"/AudioToolbox")
            assert not word&0x100
            result[word_at]=37; result[name_at:name_at+len(NAME)]=ALIAS
            allowed.add(word_at); allowed.update(range(name_at,name_at+len(NAME))); imported.append(word_at)
    symoff,nsyms,stroff,_=symbols
    for i in range(nsyms):
        at=start+symoff+16*i
        strx,kind,_,desc,_=struct.unpack_from("<IBBHQ",original,at)
        name_at=start+stroff+strx
        if original[name_at:original.index(0,name_at)]==NAME:
            assert kind==1 and libraries[(desc>>8)-1].endswith(b"/AudioToolbox")
            result[at+7]=37; result[name_at:name_at+len(NAME)]=ALIAS
            allowed.add(at+7); allowed.update(range(name_at,name_at+len(NAME))); nlisted.append(at)
    assert len(imported)==len(nlisted)==1
    assert len(result)==len(original)
    assert all(a==b or i in allowed for i,(a,b) in enumerate(zip(original,result)))
    return bytes(result)
