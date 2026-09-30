"""Test the native scanner's actual extracted pass against real FEX text."""
from pathlib import Path
import subprocess,os,json,hashlib
H=Path(__file__).resolve().parent;original=H.parents[1]/'ipad-ui/audit/virtual_ios.c';old=original.read_text();new=(H/'source/virtual_ios.c').read_text()
a=old.index('static unsigned char *ios_x18_build_data_map(');b=old.index('\n/* Task #25:',a);builder=old[a:b]
def wrapper(s,name):
 a=s.index('    if (ios_teb_tls_slot_offset && text_size >= 12)');b=s.index('\n    for (size_t i = 0; i < text_size; i += 4)',a)
 return 'static void '+name+'(char *text_rw, size_t text_size, unsigned char *data_map) {\nchar *text_rx=text_rw;\n'+s[a:b]+'\n}\n'
pre='''#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
static int ios_teb_tls_slot_offset=0x900;
static int report_log(int fd,const char *format,...){(void)fd;(void)format;return 0;}
#define dprintf report_log
'''
main=r'''
int main(int argc,char **argv){
 assert(argc==3);FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>0);rewind(f);
 char *text=calloc(1,(size_t)n+4),*before=malloc((size_t)n);assert(text&&before);assert(fread(text,1,n,f)==(size_t)n);fclose(f);memcpy(before,text,n);
 unsigned char *map=ios_x18_build_data_map(text,n);assert(map);
 if(!strcmp(argv[2],"negative")){bad_pass(text,n,map);fprintf(stderr,"negative control failed to trigger\n");return 2;}
 good_pass(text,n,map);unsigned changed=0,static_reads=0;
 for(size_t i=0;i+12<=(size_t)n;i+=4){uint32_t *p=(uint32_t*)(before+i);unsigned reg=p[0]&31;
  if((p[0]&0xffffffe0)==0xd53bd060 && p[1]==(0x927df000u|(reg<<5)|reg) && (p[2]&0xffc003ff)==(0xf9400000u|(reg<<5)|reg)){
   static_reads++;
   assert((*(uint32_t*)(text+i+8)&0x003ffc00)==((0x900/8)<<10));
  }
 }
 for(size_t i=0;i<(size_t)n;i++)changed+=text[i]!=before[i];
 printf("triplets=%u changed_bytes=%u\n",static_reads,changed);
 assert(static_reads==6 && changed==6);
 // All three words of a literal-marked sequence are excluded independently.
 for(unsigned word=0;word<3;word++){
  uint32_t probe[]={0xd53bd070,0x927df210,0xf9444e10};unsigned char flags=1u<<word;
  good_pass((char*)probe,sizeof(probe),&flags);assert(probe[2]==0xf9444e10);
 }
 free(map);free(before);free(text);printf("PASS six actual FEX triplets and three literal exclusions; packed bitmap stays in bounds\n");return 0;
}
'''
p=H/'test.c';p.write_text(pre+builder+'\n'+wrapper(old,'bad_pass')+wrapper(new,'good_pass')+main)
exe=H/'test';subprocess.run(['clang','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p),'-o',str(exe)],check=True)
env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
good=subprocess.run([str(exe),str(H/'v2-fex-text.bin'),'fixed'],env=env,capture_output=True,text=True)
(H/'fixed.log').write_text(good.stdout+good.stderr);assert good.returncode==0,good.stdout+good.stderr
bad=subprocess.run([str(exe),str(H/'v2-fex-text.bin'),'negative'],env=env,capture_output=True,text=True)
(H/'negative.log').write_text(bad.stdout+bad.stderr);assert bad.returncode!=0 and 'heap-buffer-overflow' in bad.stderr,bad.stderr
r={'status':'PASS','fixed_result':good.stdout.strip(),'negative_control':'ASan heap-buffer-overflow reproduced in original pass','source_sha256':hashlib.sha256(new.encode()).hexdigest(),'native_built':False,'installed':False,'scope':'Actual native map builder and retarget-pass source executed under host ASan/UBSan on the real v2 FEX .text and literal-exclusion fixtures.'}
(H/'test-report.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
