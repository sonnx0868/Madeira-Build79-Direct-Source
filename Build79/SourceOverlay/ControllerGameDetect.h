#ifndef MADEIRA_CONTROLLER_GAME_DETECT_H
#define MADEIRA_CONTROLLER_GAME_DETECT_H
#include <stdint.h>
#include <stddef.h>
/* Bounded, file-backed PE inspection. Only x64 EXEs importing our supported
 * XInput versions qualify. Never identifies x86/ARM64 files by their filename.
 * The callback must return true only when ALL requested bytes were read. */
typedef int (*MCPERead)(void *, uint32_t, void *, size_t);
static uint16_t mc_pe16(const unsigned char *p) {return (uint16_t)(p[0]|((uint16_t)p[1]<<8));}
static uint32_t mc_pe32(const unsigned char *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
typedef struct {MCPERead read;void *context;unsigned char sections[96][40];unsigned count;uint32_t headers;} MCPE;
static int mc_pe_rva(MCPE *pe, uint32_t rva, void *bytes, size_t size) {
    if (size>4096 || size>UINT32_MAX-rva) return 0;
    if (rva<pe->headers && size<=pe->headers-rva) return pe->read(pe->context,rva,bytes,size);
    for (unsigned i=0;i<pe->count;i++) {
        unsigned char *s=pe->sections[i];
        uint32_t va=mc_pe32(s+12),raw=mc_pe32(s+16),offset=mc_pe32(s+20);
        if (rva<va || rva-va>=raw || size>raw-(rva-va)) continue;
        if (offset>UINT32_MAX-(rva-va) || size>UINT32_MAX-offset-(rva-va)) return 0;
        return pe->read(pe->context,offset+(rva-va),bytes,size);
    }
    return 0;
}
static int mc_pe_xinput_name(MCPE *pe,uint32_t rva) {
    char name[32];unsigned count=0;
    for (;count<sizeof name;count++) {
        if (rva>UINT32_MAX-count || !mc_pe_rva(pe,rva+count,name+count,1)) return 0;
        if (!name[count]) break;
        if (name[count]>='A' && name[count]<='Z') name[count]=(char)(name[count]+32);
    }
    if (count==sizeof name) return 0;
    const char *supported[]={"xinput1_3.dll","xinput1_4.dll","xinput9_1_0.dll"};
    for (unsigned i=0;i<3;i++) {
        unsigned j=0;while(j<count && supported[i][j] && name[j]==supported[i][j]) j++;
        if (j==count && !supported[i][j]) return 1;
    }
    return 0;
}
static int mc_pe_imports(MCPE *pe,uint32_t rva,uint32_t size,int delay) {
    unsigned stride=delay?32:20;
    if (!rva || size<stride || size>UINT32_MAX-rva) return 0;
    unsigned limit=size/stride;if(limit>256) limit=256;
    for (unsigned i=0;i<limit;i++) {
        unsigned char descriptor[32];
        if (!mc_pe_rva(pe,rva+i*stride,descriptor,stride)) return 0;
        uint32_t name=mc_pe32(descriptor+(delay?4:12));
        if (!name) break;
        // Modern PE32+ delay descriptors use RVAs; do not interpret legacy VAs.
        if (delay && mc_pe32(descriptor)!=1) continue;
        if (mc_pe_xinput_name(pe,name)) return 1;
    }
    return 0;
}
static int mc_pe_game_uses_xinput(MCPERead read,void *context) {
    MCPE pe={0};pe.read=read;pe.context=context;
    unsigned char dos[64],coff[24],optional[512];
    if (!read(context,0,dos,64) || dos[0]!='M' || dos[1]!='Z') return 0;
    uint32_t offset=mc_pe32(dos+60);
    if (offset<64 || offset>1024*1024 || !read(context,offset,coff,24) ||
        mc_pe32(coff)!=0x4550 || mc_pe16(coff+4)!=0x8664) return 0;
    pe.count=mc_pe16(coff+6);
    uint16_t length=mc_pe16(coff+20),characteristics=mc_pe16(coff+22);
    if (!pe.count || pe.count>96 || length<128 || length>512 ||
        !(characteristics&2) || (characteristics&0x2000) ||
        !read(context,offset+24,optional,length) || mc_pe16(optional)!=0x20b) return 0;
    pe.headers=mc_pe32(optional+60);
    if (pe.headers>4*1024*1024 || !read(context,offset+24+length,pe.sections,pe.count*40)) return 0;
    uint32_t directories=mc_pe32(optional+108);
    if (directories>1 && mc_pe_imports(&pe,mc_pe32(optional+120),mc_pe32(optional+124),0)) return 1;
    return directories>13 && length>=224 &&
        mc_pe_imports(&pe,mc_pe32(optional+216),mc_pe32(optional+220),1);
}
#endif
