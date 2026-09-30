"""Compile the extracted source routine and verify real descriptor ownership."""
from pathlib import Path
import json, os, subprocess
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
source=ROOT/'work/beamng/legacy/fd-limit-audit/evidence/build__ntdll-unix__server_ios.c'
header=r'''
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
typedef int64_t LONG64;
#define C_ASSERT(x) _Static_assert(x, #x)
enum server_fd_type { FD_TYPE_INVALID, FD_TYPE_FILE, FD_TYPE_DIR, FD_TYPE_SOCKET,
    FD_TYPE_SERIAL, FD_TYPE_CHAR, FD_TYPE_DEVICE, FD_TYPE_NB_TYPES };
static int noted[8], note_count;
static void ios_fdt_note_close(int fd,const char *why,void *peb) { noted[note_count++]=fd; }
'''
main=r'''
int main(int argc,char **argv) {
    int p[2]; assert(pipe(p)==0); assert(p[1]==p[0]+1);
    struct ios_fd_cache *c=calloc(1,sizeof(*c)); assert(c);
    void *peb=(void *)(uintptr_t)0x12340000;
    ios_fd_cache_count=1;
    ios_fd_caches[0].peb=peb; ios_fd_caches[0].cache=c; ios_fd_caches[0].in_use=1;
    c->blocks[0]=c->initial_block;
    c->initial_block[8191].s.fd=p[0]+1;
    c->initial_block[8191].s.type=FD_TYPE_FILE;
    ios_fd_cache_release(peb);
    int original=argc>1;
    assert(note_count==1 && noted[0]==p[0]+original);
    assert((fcntl(p[0],F_GETFD)>=0)==original);
    assert((fcntl(p[1],F_GETFD)>=0)==!original);
    assert(!ios_fd_caches[0].in_use && !ios_fd_caches[0].cache);
    /* A second release is a no-op, so it cannot close a newly reused FD. */
    int reused=open("/dev/null",O_RDONLY); assert(reused>=0);
    ios_fd_cache_release(peb);
    assert(fcntl(reused,F_GETFD)>=0 && note_count==1);
    close(reused); close(p[original?0:1]);
    puts("PASS real descriptor ownership and repeated release");
}
'''
results=[]
for name,path in [('original',source),('fixed',HERE/'server_ios.fixed.c')]:
    text=path.read_text()
    union=text[text.index('union fd_cache_entry\n'):text.index('#ifdef WINE_IOS\n/* On iOS, Wine "processes"')]
    cache=text[text.index('struct ios_fd_cache {'):text.index('static struct ios_fd_cache *ios_get_fd_cache(void)')]
    routine=text[text.index('void ios_fd_cache_release( void *peb )'):text.index('#define fd_cache           (ios_get_fd_cache()->blocks)')]
    c=HERE/(name+'-fixture.c')
    c.write_text(header+union+cache+routine+main)
    exe=HERE/(name+'-fixture')
    subprocess.run(['clang','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-pthread',str(c),'-o',str(exe)],check=True)
    # LeakSanitizer cannot enumerate threads under this host's ptrace sandbox.
    # Keep AddressSanitizer/UBSan active; describe that limitation in the report.
    env=os.environ.copy(); env['ASAN_OPTIONS']='detect_leaks=0'
    result=subprocess.run([str(exe)]+(['original'] if name=='original' else []),env=env,capture_output=True,text=True,check=True)
    results.append({'variant':name,'result':'PASS','stdout':result.stdout,'stderr':result.stderr})
report={'result':'PASS','scope':'Extracted original/fixed iOS source with real Linux pipe descriptors; ASan and UBSan enabled; LeakSanitizer disabled because the ptrace sandbox prevents its thread scan',
        'original_closes_neighbor_instead_of_owned_fd':True,'fixed_closes_owned_fd_and_preserves_neighbor':True,
        'second_release_preserves_reused_descriptor':True,'cases':results}
(HERE/'real-fd-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
