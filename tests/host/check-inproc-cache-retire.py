#!/usr/bin/env python3
"""Run Wine's actual cache-retirement function with live adapter descriptors."""
from pathlib import Path
import os, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
text=(root/'wine/dlls/ntdll/unix/sync.c').read_text()
def block(header):
    start=text.index(header); pos=text.index('{',start)+1; depth=1
    while depth:
        depth+=(text[pos]=='{')-(text[pos]=='}'); pos+=1
    return text[start:pos]

template=r'''
#include "madsync_bridge.c"
#include <assert.h>
typedef int32_t LONG;
static LONG InterlockedCompareExchange(LONG *p,LONG value,LONG expected)
{ __atomic_compare_exchange_n(p,&expected,value,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST); return expected; }
static LONG InterlockedDecrement(LONG *p) { return __atomic_sub_fetch(p,1,__ATOMIC_SEQ_CST); }
static pthread_mutex_t fd_cache_mutex=PTHREAD_MUTEX_INITIALIZER;
static void server_enter_uninterrupted_section(pthread_mutex_t *m,sigset_t *saved)
{ sigset_t all; sigfillset(&all); pthread_sigmask(SIG_BLOCK,&all,saved); pthread_mutex_lock(m); }
static void server_leave_uninterrupted_section(pthread_mutex_t *m,sigset_t *saved)
{ pthread_mutex_unlock(m); pthread_sigmask(SIG_SETMASK,saved,NULL); }
@STRUCTS@
static struct ios_inproc_cache *ios_inproc_caches[IOS_MAX_INPROC_CACHES];
static pthread_mutex_t ios_inproc_cache_lock=PTHREAD_MUTEX_INITIALIZER;
#define close(fd) madsync_close(fd)
@RELEASE@
@RETIRE@
#undef close
int main(void)
{
    assert(madsync_enabled());
    struct ios_inproc_cache *cache=calloc(1,sizeof(*cache)); assert(cache);
    cache->peb=(void*)123; cache->blocks[0]=cache->initial; ios_inproc_caches[0]=cache;
    int fds[3]; struct ntsync_event_args args={0,0};
    for(unsigned i=0;i<3;i++)
    {
        fds[i]=madsync_ioctl(MADSYNC_DEVICE_FD,NTSYNC_IOC_CREATE_EVENT,&args); assert(fds[i]>=0);
        assert(madsync_ref(fds[i])==fds[i]);
        cache->initial[i].fd=fds[i];
    }
    cache->initial[0].refcount=2; /* cache + one in-flight Wine API user */
    cache->initial[1].refcount=1; /* cache only */
    cache->initial[2].refcount=1; cache->initial[2].closed=1; /* already closed; one user */
    ios_inproc_cache_release((void*)123);
    assert(!ios_inproc_caches[0] && cache->initial[0].closed);
    assert(cache->initial[0].refcount==1 && cache->initial[1].refcount==0 && cache->initial[2].refcount==1);
    ios_inproc_cache_release((void*)123); /* no double release */
    for(unsigned i=0;i<3;i++) assert(madsync_close(fds[i])==0); /* server ownership ends */
    release_inproc_sync(&cache->initial[0]);
    release_inproc_sync(&cache->initial[2]);
    for(unsigned i=0;i<3;i++) assert(madsync_ref(fds[i])==-1);
    free(cache);
    puts("PASS actual Wine cache retirement: in-flight users, already closed handles, no leaked descriptor references");
}
'''
structures=block('struct inproc_sync\n')+';\n'
structures+='#define INPROC_SYNC_CACHE_BLOCK_SIZE (65536 / sizeof(struct inproc_sync))\n#define INPROC_SYNC_CACHE_ENTRIES 128\n'
structures+=block('struct ios_inproc_cache\n')+';\n#define IOS_MAX_INPROC_CACHES 64\n'
source=template.replace('@STRUCTS@',structures).replace('@RELEASE@',block('static void release_inproc_sync( struct inproc_sync *sync )\n{'))
source=source.replace('@RETIRE@',block('void ios_inproc_cache_release( void *peb )'))
with tempfile.TemporaryDirectory(prefix='madeira-sync-retire-') as tmp:
    folder=Path(tmp); (folder/'test.c').write_text(source); (folder/'madeira.cfg').write_text('sync-engine=nutssync\n')
    binary=folder/'test'
    subprocess.run(['clang','-std=gnu11','-O1','-g','-fsanitize=address,undefined',
                    '-Wno-incompatible-pointer-types-discards-qualifiers','-mmacosx-version-min=15.0',
                    '-I',str(root/'build/madsync'),str(folder/'test.c'),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],env=dict(os.environ,MADEIRA_DOCS_DIR=str(folder),
                    ASAN_OPTIONS='abort_on_error=1:detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'),check=True)
