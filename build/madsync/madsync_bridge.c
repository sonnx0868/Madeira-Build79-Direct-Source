/* LGPL-2.1-or-later. Madeira synchronization dispatcher, 2026-10-18.
 * Keep the existing ntsync ABI: server and client share the same implementation
 * in libntdll_unix.a. No PE ABI change or proprietary NutsEmu host is needed. */
#define madsync_enabled madsync_legacy_enabled
#define madsync_ioctl madsync_legacy_ioctl
#define madsync_close madsync_legacy_close
#define madsync_ref madsync_legacy_ref
#define madsync_post madsync_legacy_post
#define madsync_take madsync_legacy_take
#include "madsync.c"
#undef madsync_enabled
#undef madsync_ioctl
#undef madsync_close
#undef madsync_ref
#undef madsync_post
#undef madsync_take

#include "nutssync_adapter.c"

static pthread_once_t sync_once = PTHREAD_ONCE_INIT;
static int sync_backend = MADEIRA_SYNC_WINE;

static void sync_initialize(void)
{
    int requested = madeira_cfg_sync_engine();
    const char *session = getenv("MADEIRA_MADSYNC_SESSION");
    /* One-time installers still use the server's RPC-compatible path. */
    if (session && !strcmp(session, "0")) requested = MADEIRA_SYNC_WINE;
    if (requested == MADEIRA_SYNC_NUTSSYNC)
    {
        if (ns_initialize()) sync_backend = MADEIRA_SYNC_NUTSSYNC;
        else fprintf(stderr, "[nutssync] unavailable; using Wine synchronization for this session\n");
    }
    else if (requested == MADEIRA_SYNC_MADSYNC && madsync_legacy_enabled())
        sync_backend = MADEIRA_SYNC_MADSYNC;
    fprintf(stderr, "[sync-engine] nutssync-v1 requested=%d active=%d\n", requested, sync_backend);
}

int madsync_enabled(void)
{
    pthread_once(&sync_once, sync_initialize);
    return sync_backend == MADEIRA_SYNC_NUTSSYNC || sync_backend == MADEIRA_SYNC_MADSYNC;
}

int madsync_ioctl(int fd, unsigned long req, void *arg)
{
    madsync_enabled();
    return sync_backend == MADEIRA_SYNC_NUTSSYNC ? ns_ioctl(fd, req, arg) : madsync_legacy_ioctl(fd, req, arg);
}
int madsync_close(int fd)
{
    madsync_enabled();
    return sync_backend == MADEIRA_SYNC_NUTSSYNC ? ns_close(fd) : madsync_legacy_close(fd);
}
int madsync_ref(int fd)
{
    madsync_enabled();
    return sync_backend == MADEIRA_SYNC_NUTSSYNC ? ns_ref(fd) : madsync_legacy_ref(fd);
}
int madsync_post(unsigned int pid, unsigned int handle, int fd)
{
    madsync_enabled();
    if (sync_backend == MADEIRA_SYNC_NUTSSYNC) return ns_post((unsigned)pid, handle, fd);
    return madsync_legacy_post(pid, handle, fd);
}
int madsync_take(unsigned int pid, unsigned int handle)
{
    madsync_enabled();
    return sync_backend == MADEIRA_SYNC_NUTSSYNC ? ns_take(pid, handle) : madsync_legacy_take(pid, handle);
}

void madsync_discard_posts(unsigned int pid)
{
    sigset_t saved;
    madsync_enabled();
    if (sync_backend == MADEIRA_SYNC_NUTSSYNC) { ns_discard_posts(pid); return; }
    ms_lock(&saved);
    for (unsigned i = 0; i < 256; i++)
        if (g_post[i].fd && g_post[i].pid == pid)
        {
            obj_unref_locked((unsigned)g_post[i].fd & 0x0fffffff);
            g_post[i].fd = 0;
        }
    ms_unlock(&saved);
}

/* Acquire BEFORE thread_suspend(), release AFTER it. No sync critical region
 * makes server requests or waits on a guest: the target can always leave it.
 * Mach suspension ignores signal masks, so masking signals alone is unsafe. */
void madsync_suspend_begin(struct madsync_suspend_guard *guard)
{
    madsync_enabled();
    guard->backend = sync_backend;
    if (guard->backend == MADEIRA_SYNC_NUTSSYNC) nutssync_lock(&guard->saved);
    else if (guard->backend == MADEIRA_SYNC_MADSYNC) ms_lock(&guard->saved);
}
void madsync_suspend_end(struct madsync_suspend_guard *guard)
{
    if (guard->backend == MADEIRA_SYNC_NUTSSYNC) nutssync_unlock(&guard->saved);
    else if (guard->backend == MADEIRA_SYNC_MADSYNC) ms_unlock(&guard->saved);
}
