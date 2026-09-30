
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

static BOOL alloc_tls_slot( LDR_DATA_TABLE_ENTRY *mod )
{
    const IMAGE_TLS_DIRECTORY *dir;
    ULONG i, size;
    void *new_ptr;
    UINT old_module_count = tls_module_count;
    HANDLE thread = NULL, next;

    if (!(dir = RtlImageDirectoryEntryToData( mod->DllBase, TRUE, IMAGE_DIRECTORY_ENTRY_TLS, &size )))
        return FALSE;

    size = dir->EndAddressOfRawData - dir->StartAddressOfRawData;
    if (!size && !dir->SizeOfZeroFill && !dir->AddressOfCallBacks) return FALSE;

    for (i = 0; i < tls_module_count; i++)
    {
        if (!tls_dirs[i].StartAddressOfRawData && !tls_dirs[i].EndAddressOfRawData &&
            !tls_dirs[i].SizeOfZeroFill && !tls_dirs[i].AddressOfCallBacks)
            break;
    }

    TRACE( "module %p data %p-%p zerofill %lu index %p callback %p flags %lx -> slot %lu\n", mod->DllBase,
           (void *)dir->StartAddressOfRawData, (void *)dir->EndAddressOfRawData, dir->SizeOfZeroFill,
           (void *)dir->AddressOfIndex, (void *)dir->AddressOfCallBacks, dir->Characteristics, i );

    if (i == tls_module_count)
    {
        UINT new_count = tls_module_count * 2;

        new_ptr = RtlReAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY, tls_dirs,
                                     new_count * sizeof(*tls_dirs) );
        if (!new_ptr) return FALSE;
        tls_dirs = new_ptr;
        tls_module_count = new_count;
    }

    /* allocate the data block in all running threads */
    while (!NtGetNextThread( GetCurrentProcess(), thread, THREAD_QUERY_LIMITED_INFORMATION, 0, 0, &next ))
    {
        THREAD_BASIC_INFORMATION tbi;
        TEB *teb;

        if (thread) NtClose( thread );
        thread = next;
        if (NtQueryInformationThread( thread, ThreadBasicInformation, &tbi, sizeof(tbi), NULL ) || !tbi.TebBaseAddress)
        {
            ERR( "NtQueryInformationThread failed.\n" );
            continue;
        }
        teb = tbi.TebBaseAddress;
        if (!teb->ThreadLocalStoragePointer)
        {
            /* Thread is not initialized by loader yet or already teared down. */
            TRACE( "thread %04lx NULL tls block.\n", HandleToULong(tbi.ClientId.UniqueThread) );
            continue;
        }

        if (old_module_count < tls_module_count)
        {
            void **old = teb->ThreadLocalStoragePointer;
            void **new = RtlAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY, tls_module_count * sizeof(*new));

            if (!new) return FALSE;
            if (old) memcpy( new, old, old_module_count * sizeof(*new) );
            teb->ThreadLocalStoragePointer = new;
            TRACE( "thread %04lx tls block %p -> %p\n", HandleToULong(teb->ClientId.UniqueThread), old, new );
            /* FIXME: can't free old block here, should be freed at thread exit */
        }

        if (!(new_ptr = RtlAllocateHeap( GetProcessHeap(), 0, size + dir->SizeOfZeroFill ))) return FALSE;
        memcpy( new_ptr, (void *)dir->StartAddressOfRawData, size );
        memset( (char *)new_ptr + size, 0, dir->SizeOfZeroFill );

        TRACE( "thread %04lx slot %lu: %lu/%lu bytes at %p\n",
               HandleToULong(teb->ClientId.UniqueThread), i, size, dir->SizeOfZeroFill, new_ptr );

        RtlFreeHeap( GetProcessHeap(), 0,
                     InterlockedExchangePointer( (void **)teb->ThreadLocalStoragePointer + i, new_ptr ));
    }
    if (thread) NtClose( thread );

    *(DWORD *)dir->AddressOfIndex = i;
    tls_dirs[i] = *dir;
    {
        /* iOS-Madeira ml704 [tls-life]: which TLS slot each module actually got.
         *
         * Compiler-emitted magic statics in main EXEs commonly hardcode TLS[0]
         * on the Windows convention that the EXE owns slot 0.  If xtajit64 (or
         * anything else) takes slot 0 first, those reads land in the wrong
         * module's TLS block and the guest concludes its registries are already
         * initialised, leaving them zero -- which is exactly what a NULL table
         * pointer in a settings registry looks like.  Print the assignment so
         * the ordering is a measured fact rather than an assumption. */
        static int n_slot;
        if (n_slot < 24)
        {
            n_slot++;
            ERR( "[tls-life] ml704 slot=%lu module=%s base=%p callbacks=%p rawsize=%lu zerofill=%lu\n",
                 i, debugstr_w(mod->BaseDllName.Buffer), mod->DllBase,
                 (void *)dir->AddressOfCallBacks,
                 (ULONG)(dir->EndAddressOfRawData - dir->StartAddressOfRawData),
                 dir->SizeOfZeroFill );
        }
    }
    return TRUE;
}
static NTSTATUS alloc_initial_tls_slot( LDR_DATA_TABLE_ENTRY *mod )
{
    extern void *xlate_ios_jit( void *ptr );
    const IMAGE_TLS_DIRECTORY *dir;
    DWORD *index, *executing_index;
    ULONG size;

    dir = RtlImageDirectoryEntryToData( mod->DllBase, TRUE, IMAGE_DIRECTORY_ENTRY_TLS, &size );
    if (!dir || (dir->StartAddressOfRawData == dir->EndAddressOfRawData &&
                 !dir->SizeOfZeroFill && !dir->AddressOfCallBacks))
        return STATUS_SUCCESS;
    if (!alloc_tls_slot( mod )) return STATUS_NO_MEMORY;
    /* Madeira may execute a process-owned copy of PE .data. The initially
     * loaded ntdll bypasses ordinary import/protection synchronization; update
     * its index through the same existing translation route as delay IATs. */
    index = (DWORD *)dir->AddressOfIndex;
    executing_index = xlate_ios_jit( index );
    if (!executing_index)
    {
        ERR( "[tls-initial] module=%p index_ptr=%p translation failed\n", mod->DllBase, index );
        return STATUS_UNSUCCESSFUL;
    }
    if (executing_index != index) *(volatile DWORD *)executing_index = *index;
    if (*(volatile DWORD *)executing_index != *index)
    {
        ERR( "[tls-initial] module=%p index_ptr=%p executing_index=%p readback failed\n",
             mod->DllBase, index, executing_index );
        return STATUS_UNSUCCESSFUL;
    }
    mod->TlsIndex = -1;
    ERR( "[tls-initial] module=%p index_ptr=%p index=%lu executing_index=%p value=%lu\n",
         mod->DllBase, index, *index, executing_index, *executing_index );
    return STATUS_SUCCESS;
}
static NTSTATUS alloc_thread_tls(void)
{
    void **pointers;
    UINT i, size;

    if (!(pointers = RtlAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY,
                                      tls_module_count * sizeof(*pointers) )))
        return STATUS_NO_MEMORY;

    for (i = 0; i < tls_module_count; i++)
    {
        const IMAGE_TLS_DIRECTORY *dir = &tls_dirs[i];

        if (!dir) continue;
        size = dir->EndAddressOfRawData - dir->StartAddressOfRawData;
        if (!size && !dir->SizeOfZeroFill) continue;

        if (!(pointers[i] = RtlAllocateHeap( GetProcessHeap(), 0, size + dir->SizeOfZeroFill )))
        {
            while (i) RtlFreeHeap( GetProcessHeap(), 0, pointers[--i] );
            RtlFreeHeap( GetProcessHeap(), 0, pointers );
            return STATUS_NO_MEMORY;
        }
        memcpy( pointers[i], (void *)dir->StartAddressOfRawData, size );
        memset( (char *)pointers[i] + size, 0, dir->SizeOfZeroFill );

        TRACE( "slot %u: %u/%lu bytes at %p\n", i, size, dir->SizeOfZeroFill, pointers[i] );
    }
    NtCurrentTeb()->ThreadLocalStoragePointer = pointers;
    ERR( "iOS-Madeira alloc_thread_tls: TEB=%p TEB->TLS=%p TLS[0]=%p TLS[1]=%p TLS[2]=%p TLS[3]=%p (count=%u)\n",
         NtCurrentTeb(), pointers,
         tls_module_count > 0 ? pointers[0] : NULL,
         tls_module_count > 1 ? pointers[1] : NULL,
         tls_module_count > 2 ? pointers[2] : NULL,
         tls_module_count > 3 ? pointers[3] : NULL,
         tls_module_count );
    return STATUS_SUCCESS;
}

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
