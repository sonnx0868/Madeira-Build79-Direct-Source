#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stddef.h>

typedef int32_t NTSTATUS;
typedef unsigned char BYTE;
typedef int vm_prot_t, kern_return_t;
typedef uint32_t mach_port_t,mach_msg_type_number_t;
typedef uint64_t mach_vm_address_t,mach_vm_size_t;
typedef int *vm_region_info_t;
#pragma pack(push,4)
typedef struct {int32_t protection,max_protection,inheritance,shared,reserved;uint64_t offset;int32_t behavior;uint16_t user_wired_count;} vm_region_basic_info_data_64_t;
#pragma pack(pop)
_Static_assert(sizeof(vm_region_basic_info_data_64_t)==36,"Darwin VM_REGION_BASIC_INFO_64 modeled layout");
_Static_assert(offsetof(vm_region_basic_info_data_64_t,max_protection)==4,"current/max fields");
#define VM_REGION_BASIC_INFO_COUNT_64 9u
#define VM_REGION_BASIC_INFO_64 9
#define KERN_SUCCESS 0
#define MACH_PORT_NULL 0
#define FALSE 0
#define VM_PROT_READ PROT_READ
#define VM_PROT_WRITE PROT_WRITE
#define VM_PROT_EXECUTE PROT_EXEC
#define STATUS_SUCCESS ((NTSTATUS)0)
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xc0000022u)
#define STATUS_NO_MEMORY ((NTSTATUS)0xc0000017u)
#define VPROT_COMMITTED 0x20
#define ROUND_ADDR(p,m) ((void *)((uintptr_t)(p)&~(uintptr_t)(m)))
#define HOST 16384u
#define GUEST 4096u
#define PAGES 5u
#define TOTAL (HOST*PAGES)
static const size_t host_page_size=HOST,host_page_mask=HOST-1;
struct file_view {void *base;size_t size;};
static unsigned char *area,*alias_area;
static void *ios_jit_rx_base_global,*ios_jit_rw_base_global;
static size_t ios_jit_pool_size_global;
static struct file_view view;
static int actual[PAGES],maxima[PAGES],mapped[PAGES];
static BYTE vprot[TOTAL/GUEST];
static unsigned query_calls,protect_calls,map_calls,deallocated_ports,issued_ports,metadata_calls,ww_calls;
static unsigned fail_query,fail_protect,fail_after_protect,silent_protect,fail_map;
static unsigned fail_query_count,fail_protect_count;
static int alias_mode,alias_refuse,neighbor_alias;
static unsigned total_tests;
#define CHECK(e) do{if(!(e)){fprintf(stderr,"FAIL line%d %s (errno=%d)\n",__LINE__,#e,errno);exit(1);}}while(0)
static mach_port_t mach_task_self(void){return 1;}
static kern_return_t mach_port_deallocate(mach_port_t task,mach_port_t port){CHECK(task==1&&port!=0);deallocated_ports++;return 0;}
static int page_index(uintptr_t p){CHECK(p>=(uintptr_t)area&&p<(uintptr_t)area+TOTAL);return (int)((p-(uintptr_t)area)/HOST);}
static kern_return_t mach_vm_region(mach_port_t task,mach_vm_address_t *address,mach_vm_size_t *size,int flavor,vm_region_info_t out,mach_msg_type_number_t *count,mach_port_t *object){
 CHECK(task==1&&flavor==VM_REGION_BASIC_INFO_64&&*count==VM_REGION_BASIC_INFO_COUNT_64);query_calls++;
 if(query_calls==fail_query){fail_query_count++;return 5;}
 int i=page_index((uintptr_t)*address);
 if(!mapped[i]){*address=(uintptr_t)area+(i+1)*HOST;*size=HOST;}
 else{*address=(uintptr_t)area+i*HOST;*size=HOST;}
 vm_region_basic_info_data_64_t *info=(void*)out;info->protection=actual[i];info->max_protection=maxima[i];
 *object=100+issued_ports++;return 0;
}
static kern_return_t mach_vm_protect(mach_port_t task,mach_vm_address_t address,mach_vm_size_t size,int setmax,vm_prot_t prot){
 CHECK(task==1&&size==HOST&&setmax==0&&(address%HOST)==0);CHECK((prot&~7)==0);protect_calls++;
 int i=page_index((uintptr_t)address);
 if(protect_calls==fail_protect){fail_protect_count++;return 5;}
 if((prot&maxima[i])!=prot)return 2;
 if(protect_calls==silent_protect)return 0;
 CHECK(mprotect((void*)(uintptr_t)address,HOST,prot)==0);actual[i]=prot;
 if(protect_calls==fail_after_protect){fail_protect_count++;return 5;}
 return 0;
}
static uintptr_t ios_jit_anon_alias_lookup(uintptr_t p){(void)p;return alias_mode?(uintptr_t)alias_area:0;}
int ios_jit_anon_alias_overlaps(void *p,size_t n,uintptr_t *b,uintptr_t *e){
 if(!neighbor_alias)return 0;
 uintptr_t ab=(uintptr_t)area,ae=ab+GUEST;
 if((uintptr_t)p<ae&&ab<(uintptr_t)p+n){*b=ab;*e=ae;return 1;}return 0;
}
static int ios_pool_live_overlap(uintptr_t rw,size_t n,size_t *off,void **peb){CHECK(rw==(uintptr_t)alias_area&&n<=TOTAL);*off=16;*peb=(void*)0x1234;return alias_refuse;}
static void *anon_mmap_fixed(void *base,size_t size,int prot,int flags){
 CHECK(flags==0&&prot==(PROT_READ|PROT_WRITE)&&((uintptr_t)base%HOST)==0&&(size%HOST)==0);map_calls++;
 if(map_calls==fail_map)return MAP_FAILED;
 CHECK((uintptr_t)base>=(uintptr_t)area&&(uintptr_t)base+size<=(uintptr_t)area+TOTAL);
 void *r=mmap(base,size,prot,MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(r==base);
 for(size_t n=0;n<size;n+=HOST){int i=page_index((uintptr_t)base+n);actual[i]=prot;maxima[i]=7;mapped[i]=1;}return r;
}
static void set_page_vprot_bits(void *base,size_t size,BYTE set,BYTE clear){
 CHECK((uintptr_t)base>=(uintptr_t)area&&(uintptr_t)base+size<=(uintptr_t)area+TOTAL);
 metadata_calls++;
 for(size_t n=(uintptr_t)base-(uintptr_t)area;n<(uintptr_t)base-(uintptr_t)area+size;n+=GUEST)vprot[n/GUEST]=(vprot[n/GUEST]&~clear)|set;
}
static void kernel_writewatch_register_range(struct file_view *v,void *base,size_t size){CHECK(v==&view&&size>0&&(uintptr_t)base%HOST==0&&size%HOST==0);ww_calls++;}
#include "decommit_pages.fragment.c"

static void reset(void){
 CHECK(mprotect(area,TOTAL,PROT_READ|PROT_WRITE)==0);memset(area,0xa5,TOTAL);memset(alias_area,0x5a,TOTAL);
 for(unsigned i=0;i<PAGES;i++){actual[i]=PROT_READ|PROT_WRITE;maxima[i]=7;mapped[i]=1;}
 memset(vprot,0x23,sizeof(vprot));query_calls=protect_calls=map_calls=deallocated_ports=issued_ports=metadata_calls=ww_calls=0;
 fail_query=fail_protect=fail_after_protect=silent_protect=fail_map=0;fail_query_count=fail_protect_count=0;
 alias_mode=alias_refuse=neighbor_alias=0;ios_jit_rx_base_global=ios_jit_rw_base_global=0;ios_jit_pool_size_global=0;
}
static void protection(unsigned i,int prot){CHECK(mprotect(area+i*HOST,HOST,prot)==0);actual[i]=prot;}
static sigjmp_buf access_jmp;
static volatile sig_atomic_t probing;
static void segv(int sig){if(probing)siglongjmp(access_jmp,1);signal(sig,SIG_DFL);raise(sig);_exit(121);}
static int can_access(unsigned char *p,int write){
 int ok=0;probing=1;
 if(!sigsetjmp(access_jmp,1)){volatile unsigned char *q=p;unsigned char c=*q;if(write)*q=c;ok=1;}
 probing=0;return ok;
}
static void check_physical(unsigned i,int expected){
 CHECK(actual[i]==expected);CHECK(can_access(area+i*HOST,0)==!!(expected&PROT_READ));CHECK(can_access(area+i*HOST,1)==!!(expected&PROT_WRITE));
}
static void check_content(size_t start,size_t size,int changed){
 for(unsigned i=0;i<PAGES;i++)CHECK(mprotect(area+i*HOST,HOST,PROT_READ|PROT_WRITE)==0);
 for(size_t n=0;n<TOTAL;n++)CHECK(area[n]==((changed&&n>=start&&n<start+size)?0:0xa5));
}
static void finish_test(void){CHECK(deallocated_ports==issued_ports);total_tests++;}
static void positive_geometry(size_t start,size_t len,int edge_prot,int logical_committed){
 reset();unsigned first=start/HOST,last=(start+len-1)/HOST;
 for(unsigned i=first;i<=last;i++)protection(i,edge_prot);
 if(!logical_committed)memset(vprot,0x03,sizeof(vprot));
 CHECK(decommit_pages(&view,(char*)area+start,len)==0);CHECK(metadata_calls==1);
 size_t full_start=(start+HOST-1)&~(size_t)(HOST-1),full_end=(start+len)&~(size_t)(HOST-1);
 for(unsigned i=0;i<PAGES;i++){
  int expected=(i>=first&&i<=last)?edge_prot:(PROT_READ|PROT_WRITE);
  if(i*HOST>=full_start&&(i+1)*HOST<=full_end)expected=PROT_READ|PROT_WRITE;
  check_physical(i,expected);
 }
 CHECK(ww_calls==(full_start<full_end));
 for(size_t n=0;n<TOTAL/GUEST;n++){
  int in=n*GUEST>=start&&n*GUEST<start+len;
  CHECK(!!(vprot[n]&VPROT_COMMITTED)==(logical_committed&&!in));
 }
 check_content(start,len,1);finish_test();
}
static void expect_unchanged_failure(size_t start,size_t size,NTSTATUS expected){
 CHECK(decommit_pages(&view,(char*)area+start,size)==expected);CHECK(metadata_calls==0&&ww_calls==0);
 for(unsigned i=0;i<TOTAL/GUEST;i++)CHECK(vprot[i]==0x23);
 check_content(start,size,0);finish_test();
}
int main(void){
 CHECK(sysconf(_SC_PAGESIZE)==4096);void *raw=mmap(0,TOTAL+HOST,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(raw!=MAP_FAILED);
 area=(void*)(((uintptr_t)raw+HOST-1)&~(uintptr_t)(HOST-1));alias_area=mmap(0,TOTAL,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(alias_area!=MAP_FAILED);view=(struct file_view){area,TOTAL};
 struct sigaction sa={0};sa.sa_handler=segv;sigemptyset(&sa.sa_mask);CHECK(sigaction(SIGSEGV,&sa,0)==0);CHECK(sigaction(SIGBUS,&sa,0)==0);
 /* Negative control: exact observed leading-edge geometry must really fault
  * under Linux's physical PROT_NONE, before using the corrected routine. */
 reset();protection(0,PROT_NONE);pid_t child=fork();CHECK(child>=0);if(!child){signal(SIGSEGV,SIG_DFL);memset(area+GUEST,0,HOST-GUEST);_exit(0);}int ws=0;CHECK(waitpid(child,&ws,0)==child);CHECK(WIFSIGNALED(ws)&&WTERMSIG(ws)==SIGSEGV);finish_test();
 const size_t geometry[][2]={{GUEST,HOST*2},{0,HOST+GUEST},{GUEST,GUEST},{0,GUEST},{GUEST,HOST-GUEST},{GUEST,HOST+2*GUEST},{HOST,2*HOST},{GUEST,2*HOST+2*GUEST},{0,TOTAL}};
 const int protections[]={PROT_NONE,PROT_READ,PROT_READ|PROT_WRITE};
 for(unsigned g=0;g<sizeof(geometry)/sizeof(geometry[0]);g++)for(unsigned p=0;p<3;p++)for(unsigned c=0;c<2;c++)positive_geometry(geometry[g][0],geometry[g][1],protections[p],c);
 /* Mixed guest commitment, guard/write-watch-like metadata, and distinct live
  * neighboring subpages remain intact after requested subpages are cleared. */
 reset();protection(0,PROT_NONE);vprot[0]=0x33;vprot[1]=0x10;vprot[2]=0x63;vprot[3]=0x03;
 CHECK(decommit_pages(&view,(char*)area+GUEST,2*GUEST)==0);CHECK(vprot[0]==0x33&&vprot[1]==0x10&&vprot[2]==0x43&&vprot[3]==0x03);check_physical(0,PROT_NONE);check_content(GUEST,2*GUEST,1);finish_test();
 /* Dirty FEX retained memory after a previous logical decommit must zero on
  * the next call too, without an intervening Windows recommit. */
 reset();CHECK(decommit_pages(&view,(char*)area+GUEST,GUEST)==0);memset(area+GUEST,0x7e,GUEST);CHECK(!(vprot[1]&VPROT_COMMITTED));CHECK(decommit_pages(&view,(char*)area+GUEST,GUEST)==0);CHECK(metadata_calls==2);check_content(GUEST,GUEST,1);finish_test();
 /* Size0 decommits the complete owned view. */
 reset();CHECK(decommit_pages(&view,(char*)area,0)==0);check_content(0,TOTAL,1);finish_test();
 /* Guard against neighboring anon alias even when requested base isn't in it. */
 reset();neighbor_alias=1;expect_unchanged_failure(GUEST,GUEST,STATUS_ACCESS_DENIED);
 for(unsigned which=0;which<2;which++)for(unsigned offset=0;offset<2;offset++){
  reset();ios_jit_pool_size_global=HOST;
  void *pool=offset?(void*)(area+GUEST):(void*)area;
  if(which)ios_jit_rw_base_global=pool;else ios_jit_rx_base_global=pool;
  expect_unchanged_failure(GUEST,GUEST,STATUS_ACCESS_DENIED);
 }
 reset();protection(0,PROT_READ|PROT_EXEC);expect_unchanged_failure(GUEST,GUEST,STATUS_ACCESS_DENIED);
 reset();protection(0,PROT_READ);maxima[0]=PROT_READ;expect_unchanged_failure(GUEST,GUEST,STATUS_ACCESS_DENIED);
 reset();mapped[0]=0;expect_unchanged_failure(GUEST,GUEST,STATUS_ACCESS_DENIED);
 /* Query/protect failures before zero, and silent protection downgrades. */
 for(unsigned mode=0;mode<6;mode++){
  reset();protection(0,PROT_NONE);
  if(mode==0)fail_query=1; if(mode==1)fail_query=2;if(mode==2)fail_protect=1;if(mode==3)fail_after_protect=1;if(mode==4)silent_protect=1;if(mode==5)fail_map=1;
  NTSTATUS want=mode==5?STATUS_NO_MEMORY:STATUS_ACCESS_DENIED;
  size_t size=mode==5?HOST*2:GUEST;
  CHECK(decommit_pages(&view,(char*)area+GUEST,size)==want);CHECK(metadata_calls==0&&ww_calls==0);check_physical(0,PROT_NONE);check_content(GUEST,size,0);finish_test();
 }
 /* Second edge preparation failure restores an already-opened first edge. */
 reset();protection(0,PROT_NONE);protection(2,PROT_NONE);fail_protect=2;
 CHECK(decommit_pages(&view,(char*)area+GUEST,HOST*2)==STATUS_ACCESS_DENIED);CHECK(metadata_calls==0&&map_calls==0);check_physical(0,PROT_NONE);check_physical(2,PROT_NONE);check_content(GUEST,HOST*2,0);finish_test();
 /* Restoration failures cannot be converted to successful logical decommit.
  * Physical restoration after injected kernel refusal cannot be promised. */
 for(unsigned mode=0;mode<3;mode++){
  reset();protection(0,PROT_NONE);if(mode==0)fail_protect=2;if(mode==1)fail_query=3;if(mode==2)silent_protect=2;
  CHECK(decommit_pages(&view,(char*)area+GUEST,GUEST)==STATUS_ACCESS_DENIED);CHECK(metadata_calls==0&&ww_calls==0);
  check_physical(0,mode==1?PROT_NONE:(PROT_READ|PROT_WRITE));check_content(GUEST,GUEST,1);finish_test();
 }
 /* Alias route is not privatized or protected by the edge helper; stale live
  * ledger overlap keeps its historical refusal and logical bookkeeping. */
 for(int refuse=0;refuse<2;refuse++){
  reset();alias_mode=1;alias_refuse=refuse;
  CHECK(decommit_pages(&view,(char*)area+GUEST,GUEST)==0);CHECK(query_calls==0&&protect_calls==0&&map_calls==0&&metadata_calls==1);
  for(unsigned n=0;n<GUEST;n++)CHECK(alias_area[n]==(refuse?0x5a:0));check_content(GUEST,GUEST,0);finish_test();
 }
 printf("PASS %u cases; real Linux mprotect grouped as16KiB host pages; exact baseline write faults; protection/query/remap/restore fault injection checked.\n",total_tests);
 return 0;
}
