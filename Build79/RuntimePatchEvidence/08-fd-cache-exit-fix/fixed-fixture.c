
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
union fd_cache_entry
{
    LONG64 data;
    struct
    {
        int fd;
        enum server_fd_type type : 5;
        unsigned int        access : 3;
        unsigned int        options : 24;
    } s;
};

C_ASSERT( sizeof(union fd_cache_entry) == sizeof(LONG64) );

#define FD_CACHE_BLOCK_SIZE  (65536 / sizeof(union fd_cache_entry))
#define FD_CACHE_ENTRIES     128

struct ios_fd_cache {
    union fd_cache_entry *blocks[FD_CACHE_ENTRIES];
    union fd_cache_entry initial_block[FD_CACHE_BLOCK_SIZE];
};

#define IOS_MAX_FD_CACHES 64
static struct ios_fd_cache_slot
{
    void *peb;                    /* pseudo-process identity */
    struct ios_fd_cache *cache;
    int in_use;                   /* separate flag: peb==NULL is a VALID key
                                   * (the initial process), so NULL cannot
                                   * double as "free slot" the way it does in
                                   * ios_proc_sockets. */
} ios_fd_caches[IOS_MAX_FD_CACHES];
static volatile int ios_fd_cache_count = 0;
static pthread_mutex_t ios_fd_cache_alloc_lock = PTHREAD_MUTEX_INITIALIZER;
static struct ios_fd_cache ios_fd_cache_fallback;   /* last resort, see below */

void ios_fd_cache_release( void *peb )
{
    int i, j, n, closed = 0;
    struct ios_fd_cache *c = NULL;

    pthread_mutex_lock( &ios_fd_cache_alloc_lock );
    n = ios_fd_cache_count;
    for (i = 0; i < n && i < IOS_MAX_FD_CACHES; i++)
        if (ios_fd_caches[i].in_use && ios_fd_caches[i].peb == peb)
        {
            c = ios_fd_caches[i].cache;
            ios_fd_caches[i].in_use = 0;
            ios_fd_caches[i].cache = NULL;
            break;
        }
    pthread_mutex_unlock( &ios_fd_cache_alloc_lock );
    if (!c) return;

    for (i = 0; i < FD_CACHE_ENTRIES; i++)
    {
        union fd_cache_entry *block = c->blocks[i];
        if (!block) continue;
        for (j = 0; j < FD_CACHE_BLOCK_SIZE; j++)
            if (block[j].s.type != FD_TYPE_INVALID && block[j].s.fd > 0)
            {
                /* Match add/get/remove_fd_from_cache: zero is unset, valid
                 * descriptors are encoded as fd+1, INVALID holds an error. */
                int fd = block[j].s.fd - 1;
                ios_fdt_note_close( fd, "fd-cache-release", peb );
                close( fd );
                closed++;
            }
        if (block != c->initial_block) free( block );
    }
    free( c );
    dprintf( 2, "[fd-cache] rev=ml571 released peb=%p, closed %d cached fd(s)\n", peb, closed );
}


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
