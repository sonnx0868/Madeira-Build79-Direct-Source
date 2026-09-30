#!/usr/bin/env python3
"""Execute the patched real allocation routines with bounded host service mocks."""
from pathlib import Path
import subprocess,os,json,hashlib
H=Path(__file__).resolve().parent;B=H/'build/tests';B.mkdir(parents=True,exist_ok=True)
s=(H/'source/loader.c').read_text()
def routine(start,end):
 a=s.index(start);b=s.index(end,a);return s[a:b].rstrip()
alloc=routine('static BOOL alloc_tls_slot(', '\n\n#ifdef __arm64ec__\n/* Initially')
initial=routine('static NTSTATUS alloc_initial_tls_slot(', '\n#endif')
thread=routine('static NTSTATUS alloc_thread_tls(', '\n\n/*************************************************************************')
# These checks test that the actual caller applies the required-allocation result,
# and that ntdll registration is after main but before emulator/module init.
main=s.index('if ((status = alloc_initial_tls_slot( &wm->ldr )))')
nd=s.index('if ((status = alloc_initial_tls_slot( ntdll )))',main)
assert s.index('build_ntdll_module();',s.index('void loader_init(')) < main < nd < s.index('load_arm64ec_module();',nd)
assert 'NtTerminateProcess( GetCurrentProcess(), status );' in s[main:main+150]
assert 'NtTerminateProcess( GetCurrentProcess(), status );' in s[nd:nd+145]
pre=r'''
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
typedef unsigned int ULONG,UINT,DWORD; typedef int BOOL,NTSTATUS; typedef void *HANDLE;
#define TRUE 1
#define FALSE 0
#define STATUS_SUCCESS 0
#define STATUS_UNSUCCESSFUL ((int)0xc0000001)
#define STATUS_NO_MEMORY ((int)0xc0000017)
#define HEAP_ZERO_MEMORY 8
#define THREAD_QUERY_LIMITED_INFORMATION 0x800
#define ThreadBasicInformation 0
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define TRACE(...) ((void)0)
#define ERR(...) ((void)0)
typedef struct { uintptr_t StartAddressOfRawData,EndAddressOfRawData,AddressOfIndex,AddressOfCallBacks; ULONG SizeOfZeroFill,Characteristics; } IMAGE_TLS_DIRECTORY;
typedef struct { void *DllBase; short TlsIndex; } LDR_DATA_TABLE_ENTRY;
typedef struct { void **ThreadLocalStoragePointer; } TEB;
typedef struct {TEB *TebBaseAddress;} THREAD_BASIC_INFORMATION;
static IMAGE_TLS_DIRECTORY *tls_dirs; static UINT tls_module_count;
static void *translate_from,*translate_to;
void *xlate_ios_jit(void *p){return p==translate_from?translate_to:p;}
static TEB teb[8],*current; static int threads,fail_after=-1,allocations,checks;
static void *GetProcessHeap(void){return NULL;}
static void *RtlAllocateHeap(void *h,unsigned flags,size_t n){(void)h;if(fail_after>=0 && allocations++==fail_after)return NULL;return flags&HEAP_ZERO_MEMORY?calloc(1,n?n:1):malloc(n?n:1);}
static void *RtlReAllocateHeap(void *h,unsigned flags,void *p,size_t n){(void)h;(void)flags;if(fail_after>=0 && allocations++==fail_after)return NULL;size_t old=tls_module_count*sizeof(*tls_dirs);void *q=realloc(p,n);if(q && n>old)memset((char*)q+old,0,n-old);return q;}
static void RtlFreeHeap(void*h,unsigned f,void*p){(void)h;(void)f;free(p);}
static HANDLE GetCurrentProcess(void){return NULL;}
static int NtGetNextThread(HANDLE p,HANDLE old,unsigned access,unsigned a,unsigned b,HANDLE*out){(void)p;(void)access;(void)a;(void)b;uintptr_t i=(uintptr_t)old;if(i>=(uintptr_t)threads)return 1;*out=(void*)(i+1);return 0;}
static int NtQueryInformationThread(HANDLE h,unsigned cl,THREAD_BASIC_INFORMATION *out,size_t n,void *ret){(void)cl;(void)n;(void)ret;out->TebBaseAddress=&teb[(uintptr_t)h-1];return 0;}
static void NtClose(HANDLE h){(void)h;}
static IMAGE_TLS_DIRECTORY *RtlImageDirectoryEntryToData(void*p,int mapped,int dir,ULONG*sz){(void)mapped;assert(dir==9);*sz=sizeof(IMAGE_TLS_DIRECTORY);return p;}
static void *InterlockedExchangePointer(void **p,void*v){void*old=*p;*p=v;return old;}
static TEB *NtCurrentTeb(void){return current;}
#define ck(x) do{++checks;if(!(x)){fprintf(stderr,"check failed line%d: %s\n",__LINE__,#x);abort();}}while(0)
'''
post=r'''
static void reset(void){for(int t=0;t<8;t++){if(teb[t].ThreadLocalStoragePointer){for(UINT i=0;i<tls_module_count;i++)free(teb[t].ThreadLocalStoragePointer[i]);free(teb[t].ThreadLocalStoragePointer);}}free(tls_dirs);memset(teb,0,sizeof(teb));tls_module_count=4;tls_dirs=calloc(4,sizeof(*tls_dirs));current=&teb[0];threads=1;fail_after=-1;allocations=0;translate_from=translate_to=NULL;}
static LDR_DATA_TABLE_ENTRY mod(IMAGE_TLS_DIRECTORY*d){LDR_DATA_TABLE_ENTRY m={d,0};return m;}
static IMAGE_TLS_DIRECTORY desc(void*p,size_t size,DWORD*index,size_t zero){IMAGE_TLS_DIRECTORY d={(uintptr_t)p,(uintptr_t)p+size,(uintptr_t)index,0,(ULONG)zero,0};return d;}
int main(void){
 unsigned char a[512]={0},n[96]={0},f[40]={0},data[12];memset(data,0x3d,sizeof(data));
 DWORD ai=0,ni=0,fi=0,di=0;IMAGE_TLS_DIRECTORY ad=desc(a,sizeof(a),&ai,0),nd=desc(n,sizeof(n),&ni,0),fd=desc(f,sizeof(f),&fi,0),dd=desc(data,sizeof(data),&di,13);
 LDR_DATA_TABLE_ENTRY am=mod(&ad),nm=mod(&nd),fm=mod(&fd),dm=mod(&dd);
 reset();ck(alloc_initial_tls_slot(&am)==0);ck(ai==0);ck(am.TlsIndex==-1);
 DWORD mirror=0xbad;translate_from=&ni;translate_to=&mirror;
 ck(alloc_initial_tls_slot(&nm)==0);ck(ni==1);ck(mirror==1);translate_from=translate_to=NULL;ck(nm.TlsIndex==-1);ck(alloc_tls_slot(&fm));ck(fi==2);
 ck(teb[0].ThreadLocalStoragePointer==NULL);ck(alloc_thread_tls()==0);
 for(int i=0;i<512;i++)ck(((unsigned char*)teb[0].ThreadLocalStoragePointer[ai])[i]==0);
 ck(teb[0].ThreadLocalStoragePointer[ai]!=teb[0].ThreadLocalStoragePointer[ni]);
 memset(teb[0].ThreadLocalStoragePointer[ai],0xa5,512);
 memset((char*)teb[0].ThreadLocalStoragePointer[ni]+0x20,0x4c,64);
 for(int i=0;i<512;i++)ck(((unsigned char*)teb[0].ThreadLocalStoragePointer[ai])[i]==0xa5);
 threads=2;current=&teb[1];ck(alloc_thread_tls()==0);
 for(int i=0;i<96;i++)ck(((unsigned char*)teb[1].ThreadLocalStoragePointer[ni])[i]==0);
 ck(teb[0].ThreadLocalStoragePointer[ni]!=teb[1].ThreadLocalStoragePointer[ni]);
 ck(alloc_initial_tls_slot(&dm)==0);ck(di==3);
 for(int t=0;t<2;t++){for(int i=0;i<12;i++)ck(((unsigned char*)teb[t].ThreadLocalStoragePointer[di])[i]==0x3d);for(int i=12;i<25;i++)ck(((unsigned char*)teb[t].ThreadLocalStoragePointer[di])[i]==0);}
 // Fresh process with no EXE TLS: no phantom reservation, ntdll owns first real slot.
 reset();LDR_DATA_TABLE_ENTRY empty=mod(NULL);nm.TlsIndex=0;ni=99;
 ck(alloc_initial_tls_slot(&empty)==0);ck(empty.TlsIndex==0);ck(alloc_initial_tls_slot(&nm)==0);ck(ni==0);
 ck(alloc_thread_tls()==0);ck(current->ThreadLocalStoragePointer[0]!=NULL);
 // Existing current thread + required allocation failure: don't mark registered,
 // don't overwrite the existing EXE's block or assign the next module's index.
 reset();am.TlsIndex=0;nm.TlsIndex=0;ni=99;ck(alloc_initial_tls_slot(&am)==0);ck(alloc_thread_tls()==0);
 void *owned=current->ThreadLocalStoragePointer[0];memset(owned,0xa5,512);
 fail_after=0;allocations=0;ck(alloc_initial_tls_slot(&nm)==STATUS_NO_MEMORY);ck(nm.TlsIndex==0);ck(ni==99);ck(tls_dirs[1].StartAddressOfRawData==0);ck(current->ThreadLocalStoragePointer[0]==owned);
 for(int i=0;i<512;i++)ck(((unsigned char*)owned)[i]==0xa5);
 // Required EXE allocation failure similarly reports failure before next registration.
 reset();ck(alloc_thread_tls()==0);am.TlsIndex=0;ai=99;fail_after=0;allocations=0;
 ck(alloc_initial_tls_slot(&am)==STATUS_NO_MEMORY);ck(am.TlsIndex==0);ck(ai==99);ck(tls_dirs[0].StartAddressOfRawData==0);
 // An unexpected NULL executing-copy translation fails closed before marking ready.
 reset();nm.TlsIndex=0;ni=99;translate_from=&ni;translate_to=NULL;
 ck(alloc_initial_tls_slot(&nm)==STATUS_UNSUCCESSFUL);ck(nm.TlsIndex==0);
 // A directory with zero raw bytes and zero-fill is still registered and copied.
 reset();dd=desc(data,0,&di,13);dm=mod(&dd);ck(alloc_initial_tls_slot(&dm)==0);ck(alloc_thread_tls()==0);
 for(int i=0;i<13;i++)ck(((unsigned char*)current->ThreadLocalStoragePointer[di])[i]==0);
 // Thread setup failure releases its partial allocations and never publishes them.
 reset();ck(alloc_initial_tls_slot(&am)==0);ck(alloc_initial_tls_slot(&nm)==0);fail_after=2;allocations=0;
 ck(alloc_thread_tls()==STATUS_NO_MEMORY);ck(current->ThreadLocalStoragePointer==NULL);
 reset();free(tls_dirs);tls_dirs=NULL;printf("PASS %d checks; actual alloc_tls_slot/alloc_initial_tls_slot/alloc_thread_tls\n",checks);return 0;
}
'''
f=B/'loader-test.c';f.write_text(pre+'\n'+alloc+'\n'+initial+'\n'+thread+'\n'+post)
c=['clang','-g','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',str(f),'-o',str(B/'loader-test')]
subprocess.run(c,check=True)
env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
r=subprocess.run([str(B/'loader-test')],env=env,text=True,capture_output=True,check=True)
assert not r.stderr,r.stderr
report={'result':r.stdout.strip(),'source_sha256':hashlib.sha256(s.encode()).hexdigest(),'sanitizer_stderr':r.stderr,'caller_order_checked':True,'scope':'Actual extracted allocation functions; mock thread/heap services, not ARM64EC runtime execution'}
(H/'loader-test-report.json').write_text(json.dumps(report,indent=2)+'\n');print(r.stdout)
