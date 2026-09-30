#include "ControllerGameDetect.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char image[2048];
static size_t available;
static int memory_read(void *context,uint32_t offset,void *dst,size_t count) {
    (void)context;
    if(offset>available || count>available-offset) return 0;
    memcpy(dst,image+offset,count);return 1;
}
static void put16(unsigned offset,uint16_t n) {image[offset]=(unsigned char)n;image[offset+1]=(unsigned char)(n>>8);}
static void put32(unsigned offset,uint32_t n) {for(unsigned i=0;i<4;i++)image[offset+i]=(unsigned char)(n>>(i*8));}
static void fixture(int delay,const char *name) {
    memset(image,0,sizeof image);available=sizeof image;
    image[0]='M';image[1]='Z';put32(60,128);put32(128,0x4550);
    put16(132,0x8664);put16(134,1);put16(148,240);put16(150,2);
    put16(152,0x20b);put32(212,512);put32(260,16);
    unsigned entry=152+112+(delay?13:1)*8;
    put32(entry,0x1000);put32(entry+4,delay?64:40);
    put32(392+8,1024);put32(392+12,0x1000);put32(392+16,1024);put32(392+20,512);
    if(delay) {put32(512,1);put32(516,0x1080);}
    else put32(524,0x1080);
    memcpy(image+640,name,strlen(name)+1);
}
static int file_read(void *context,uint32_t offset,void *dst,size_t count) {
    FILE *file=context;
    return !fseek(file,(long)offset,SEEK_SET) && fread(dst,1,count,file)==count;
}
int main(int argc,char **argv) {
    fixture(0,"XINPUT1_3.dll");assert(mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_4.dll");assert(mc_pe_game_uses_xinput(memory_read,0));
    fixture(1,"XiNpUt9_1_0.DLL");assert(mc_pe_game_uses_xinput(memory_read,0));
    put32(512,0);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll.fake");assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinputuap.dll");assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"kernel32.dll");assert(!mc_pe_game_uses_xinput(memory_read,0));
    uint16_t unsupported[]={0x14c,0xaa64,0xa641,0xa64e};
    for(unsigned i=0;i<4;i++) {fixture(0,"xinput1_3.dll");put16(132,unsupported[i]);assert(!mc_pe_game_uses_xinput(memory_read,0));}
    fixture(0,"xinput1_3.dll");put16(150,0x2002);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put32(60,UINT32_MAX);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put16(134,97);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put32(272,UINT32_MAX-4);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put32(392+20,UINT32_MAX-4);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");available=650;assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put32(524,0x1000+1020);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put32(260,0);assert(!mc_pe_game_uses_xinput(memory_read,0));
    fixture(0,"xinput1_3.dll");put16(148,16);assert(!mc_pe_game_uses_xinput(memory_read,0));
    // Every truncated prefix is rejected without any out-of-bounds reads.
    fixture(0,"xinput1_3.dll");
    for(available=0;available<653;available++) assert(!mc_pe_game_uses_xinput(memory_read,0));
    for(int i=1;i<argc;i++) {
        FILE *file=fopen(argv[i],"rb");assert(file);
        int found=mc_pe_game_uses_xinput(file_read,file);fclose(file);
        printf("Real EXE: %s => x64 XInput imports=%d\n",argv[i],found);
        assert(found);
    }
    puts("Controller game detection: normal/delay imports, architectures, DLL exclusion and malformed/truncated files passed.");
    return 0;
}
