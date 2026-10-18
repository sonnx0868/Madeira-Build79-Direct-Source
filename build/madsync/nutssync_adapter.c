/* LGPL-2.1-or-later. Madeira's ntsync descriptor ABI over NutsSync.
 * Included once by madsync_bridge.c. All table state uses NutsSync's lock:
 * no second contended lock, heap allocation, or WineServer call in an operation.
 * Descriptor references outlive server handles, cached handles and active waits.
 */
static void ns_object_created(uint32_t slot);
static void ns_wait_allocated(void *waiter);
static void ns_wait_released(void *waiter);
#define NUTSSYNC_OBJECT_CREATED(slot) ns_object_created(slot)
#define NUTSSYNC_WAIT_ALLOCATED(w) ns_wait_allocated(w)
#define NUTSSYNC_WAIT_RELEASED(w) ns_wait_released(w)
#include "../../third_party/nutssync/nutssync.h"

#define NS_POST_COUNT 16384u
#define NS_POST_BUCKETS 2048u
#define NS_NESTED_WAITS 8u
static uint32_t ns_refs[NUTSSYNC_MAX_OBJECTS];
static struct { unsigned pid, handle, number, next; } ns_posts[NS_POST_COUNT];
static unsigned ns_post_buckets[NS_POST_BUCKETS], ns_posts_used, ns_post_free;

struct ns_wait_frame
{
    struct nutssync_waiter *waiter;
    uint32_t pinned[NUTSSYNC_MAX_WAIT + 1];
    unsigned count;
};
struct ns_thread_state { unsigned depth; struct ns_wait_frame frames[NS_NESTED_WAITS]; };
static _Thread_local struct ns_thread_state ns_thread;
static pthread_key_t ns_thread_key;

/* Lock held. Numbers include a non-wrapping generation, not a bare slot. */
static void ns_object_created(uint32_t slot) { ns_refs[slot] = 1; }
static int ns_pin_locked(uint32_t number)
{
    unsigned slot = (number & NUTSSYNC_SLOT_MASK) - 1;
    if (!nutssync_object_of(number) || ns_refs[slot] == UINT32_MAX) return 0;
    ns_refs[slot]++;
    return 1;
}
static void ns_unref_locked(uint32_t number)
{
    struct nutssync_object *obj = nutssync_object_of(number);
    if (!obj) return;
    unsigned slot = (number & NUTSSYNC_SLOT_MASK) - 1;
    if (!--ns_refs[slot])
    {
        if (obj->waiting) obj->destroyed = 1;
        else nutssync_free_slot(obj);
    }
}
static void ns_wait_allocated(void *waiter)
{
    ns_thread.frames[ns_thread.depth - 1].waiter = waiter;
}
static void ns_wait_released(void *waiter)
{
    (void)waiter;
    ns_thread.frames[ns_thread.depth - 1].waiter = NULL;
}

/* Runs only when the NATIVE thread exits. Server logical termination can
 * precede native exit, so it must not recycle a waiter still used by the latter.
 * Include completed waiters too: a signal can race termination after removing
 * the waiter from active[], before its native thread resumes. */
static void ns_thread_exit(void *value)
{
    struct ns_thread_state *thread = value;
    sigset_t saved;
    nutssync_lock(&saved);
    while (thread->depth)
    {
        struct ns_wait_frame *frame = &thread->frames[--thread->depth];
        struct nutssync_waiter *w = frame->waiter;
        if (w)
        {
            unsigned slot = (unsigned)(w - nutssync->waiters);
            if (!w->state) nutssync_leave(w);
            w->in_use = 0;
            w->next_free = nutssync->waiter_free_head;
            nutssync->waiter_free_head = slot + 1;
        }
        while (frame->count) ns_unref_locked(frame->pinned[--frame->count]);
        frame->waiter = NULL;
    }
    nutssync_unlock(&saved);
}

static int ns_initialize(void)
{
    /* os_sync_* is public from iOS 18 / macOS 15. Do not expose the device
     * unless every resource is ready; falling back after exposure is unsafe. */
    if (__builtin_available(iOS 18.0, macOS 15.0, *))
    {
        if (pthread_key_create(&ns_thread_key, ns_thread_exit)) return 0;
        nutssync = mmap(NULL, sizeof(*nutssync), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANON, -1, 0);
        if (nutssync == MAP_FAILED)
        {
            nutssync = NULL;
            pthread_key_delete(ns_thread_key);
            return 0;
        }
        nutssync->magic = NUTSSYNC_MAGIC;
        nutssync->version = NUTSSYNC_VERSION;
        nutssync->attaches = 1;
        fprintf(stderr, "[nutssync] direct waits enabled, %zu bytes shared, no spin polling\n", sizeof(*nutssync));
        return 1;
    }
    return 0;
}

static int ns_ref(int fd)
{
    sigset_t saved;
    if (!MADSYNC_IS_FD(fd)) { errno = EBADF; return -1; }
    nutssync_lock(&saved);
    int ok = ns_pin_locked((unsigned)fd & 0x0fffffff);
    nutssync_unlock(&saved);
    if (!ok) { errno = EBADF; return -1; }
    return fd;
}
static int ns_close(int fd)
{
    sigset_t saved;
    if (fd == MADSYNC_DEVICE_FD) return 0;
    if (!MADSYNC_IS_FD(fd)) return close(fd);
    unsigned number = (unsigned)fd & 0x0fffffff;
    nutssync_lock(&saved);
    int ok = nutssync_object_of(number) != NULL;
    if (ok) ns_unref_locked(number);
    nutssync_unlock(&saved);
    if (!ok) { errno = EBADF; return -1; }
    return 0;
}

static unsigned ns_post_bucket(unsigned pid, unsigned handle)
{
    return ((pid * 2654435761u) ^ (handle * 2246822519u)) & (NS_POST_BUCKETS - 1);
}
static int ns_post(unsigned pid, unsigned handle, int fd)
{
    sigset_t saved;
    unsigned slot, number = (unsigned)fd & 0x0fffffff, bucket = ns_post_bucket(pid, handle);
    if (!MADSYNC_IS_FD(fd)) { errno = EBADF; return -1; }
    nutssync_lock(&saved);
    if (!nutssync_object_of(number)) { nutssync_unlock(&saved); errno = EBADF; return -1; }
    if (!ns_post_free && ns_posts_used == NS_POST_COUNT)
    { nutssync_unlock(&saved); errno = ENOMEM; return -1; }
    if (!ns_pin_locked(number)) { nutssync_unlock(&saved); errno = EOVERFLOW; return -1; }
    if (ns_post_free) { slot = ns_post_free - 1; ns_post_free = ns_posts[slot].next; }
    else slot = ns_posts_used++;
    ns_posts[slot].pid = pid;
    ns_posts[slot].handle = handle;
    ns_posts[slot].number = number;
    ns_posts[slot].next = ns_post_buckets[bucket];
    ns_post_buckets[bucket] = slot + 1;
    nutssync_unlock(&saved);
    return 0;
}
static int ns_take(unsigned pid, unsigned handle)
{
    sigset_t saved;
    int fd = -1;
    nutssync_lock(&saved);
    unsigned *link = &ns_post_buckets[ns_post_bucket(pid, handle)];
    while (*link)
    {
        unsigned slot = *link - 1;
        if (ns_posts[slot].pid == pid && ns_posts[slot].handle == handle)
        {
            fd = MADSYNC_FD_BASE | ns_posts[slot].number;
            *link = ns_posts[slot].next;
            ns_posts[slot].next = ns_post_free;
            ns_post_free = slot + 1;
            break;
        }
        link = &ns_posts[slot].next;
    }
    nutssync_unlock(&saved);
    return fd;
}
static void ns_discard_posts(unsigned pid)
{
    sigset_t saved;
    nutssync_lock(&saved);
    for (unsigned i = 0; i < NS_POST_BUCKETS; i++)
    {
        unsigned *link = &ns_post_buckets[i];
        while (*link)
        {
            unsigned slot = *link - 1;
            if (ns_posts[slot].pid != pid) { link = &ns_posts[slot].next; continue; }
            ns_unref_locked(ns_posts[slot].number);
            *link = ns_posts[slot].next;
            ns_posts[slot].next = ns_post_free;
            ns_post_free = slot + 1;
        }
    }
    nutssync_unlock(&saved);
}

static int ns_wait(struct ntsync_wait_args *args, int all)
{
    sigset_t saved;
    uint32_t numbers[NUTSSYNC_MAX_WAIT], alert = 0;
    const int *fds = (const int *)(uintptr_t)args->objs;
    if (args->count > NUTSSYNC_MAX_WAIT || args->flags & ~NTSYNC_WAIT_REALTIME || args->pad ||
        (args->count && !fds) || !args->owner) { errno = EINVAL; return -1; }
    if (ns_thread.depth == NS_NESTED_WAITS || pthread_setspecific(ns_thread_key, &ns_thread))
    { errno = ENOMEM; return -1; }
    nutssync_lock(&saved);
    struct ns_wait_frame *frame = &ns_thread.frames[ns_thread.depth++];
    frame->waiter = NULL;
    frame->count = 0;
    for (unsigned i = 0; i < args->count + !!args->alert; i++)
    {
        int fd = i < args->count ? fds[i] : (int)args->alert;
        uint32_t number = (unsigned)fd & 0x0fffffff;
        if (!MADSYNC_IS_FD(fd) || !ns_pin_locked(number))
        {
            while (frame->count) ns_unref_locked(frame->pinned[--frame->count]);
            ns_thread.depth--;
            nutssync_unlock(&saved);
            errno = EINVAL;
            return -1;
        }
        frame->pinned[frame->count++] = number;
        if (i < args->count) numbers[i] = number;
        else alert = number;
    }
    nutssync_unlock(&saved);
    int error = nutssync_wait(numbers, args->count, all, alert, args->owner, args->timeout,
                             !!(args->flags & NTSYNC_WAIT_REALTIME), &args->index);
    nutssync_lock(&saved);
    while (frame->count) ns_unref_locked(frame->pinned[--frame->count]);
    ns_thread.depth--;
    nutssync_unlock(&saved);
    if (error) { errno = error; return -1; }
    return 0;
}

static int ns_ioctl(int fd, unsigned long request, void *arg)
{
    uint32_t number = (unsigned)fd & 0x0fffffff, created = 0;
    int error = 0;
    if ((request & 0xffff0000ul) != 0x4d530000ul) return ioctl(fd, request, arg);
    if (!arg) { errno = EINVAL; return -1; }
    switch (request)
    {
    case NTSYNC_IOC_CREATE_SEM:
    {
        struct ntsync_sem_args *a = arg;
        if (fd != MADSYNC_DEVICE_FD || a->count > a->max) { errno = EINVAL; return -1; }
        created = nutssync_create_semaphore(a->count, a->max);
        break;
    }
    case NTSYNC_IOC_CREATE_MUTEX:
    {
        struct ntsync_mutex_args *a = arg;
        if (fd != MADSYNC_DEVICE_FD || !a->owner != !a->count) { errno = EINVAL; return -1; }
        created = nutssync_create_mutex(a->owner, a->count);
        break;
    }
    case NTSYNC_IOC_CREATE_EVENT:
    {
        struct ntsync_event_args *a = arg;
        if (fd != MADSYNC_DEVICE_FD) { errno = EINVAL; return -1; }
        created = nutssync_create_event(a->manual, a->signaled);
        break;
    }
    case NTSYNC_IOC_WAIT_ANY: case NTSYNC_IOC_WAIT_ALL:
        if (fd != MADSYNC_DEVICE_FD) { errno = EINVAL; return -1; }
        return ns_wait(arg, request == NTSYNC_IOC_WAIT_ALL);
    default:
        if (!MADSYNC_IS_FD(fd)) { errno = EBADF; return -1; }
        switch (request)
        {
        case NTSYNC_IOC_SEM_RELEASE:
            error = nutssync_semaphore_release(number, *(uint32_t *)arg, arg); break;
        case NTSYNC_IOC_SEM_READ:
        {
            struct ntsync_sem_args *a = arg;
            error = nutssync_semaphore_read(number, &a->count, &a->max); break;
        }
        case NTSYNC_IOC_MUTEX_UNLOCK:
        {
            struct ntsync_mutex_args *a = arg;
            error = nutssync_mutex_unlock(number, a->owner, &a->count); break;
        }
        case NTSYNC_IOC_MUTEX_KILL: error = nutssync_mutex_kill(number, *(uint32_t *)arg); break;
        case NTSYNC_IOC_MUTEX_READ:
        {
            struct ntsync_mutex_args *a = arg;
            error = nutssync_mutex_read(number, &a->owner, &a->count); break;
        }
        case NTSYNC_IOC_EVENT_SET: case NTSYNC_IOC_EVENT_RESET: case NTSYNC_IOC_EVENT_PULSE:
            error = nutssync_event(number, request == NTSYNC_IOC_EVENT_SET ? NUTSSYNC_SET :
                                  request == NTSYNC_IOC_EVENT_RESET ? NUTSSYNC_RESET : NUTSSYNC_PULSE, arg); break;
        case NTSYNC_IOC_EVENT_READ:
        {
            struct ntsync_event_args *a = arg;
            error = nutssync_event_read(number, &a->manual, &a->signaled); break;
        }
        default: error = EINVAL; break;
        }
        if (error) { errno = error; return -1; }
        return 0;
    }
    if (!created) { errno = ENOMEM; return -1; }
    return MADSYNC_FD_BASE | created;
}
