/* Exercise the actual dispatcher and object implementation on Apple Silicon.
 * No replacement lock, fake wait API, or mirrored synchronization algorithm. */
#include "../../build/madsync/madsync_bridge.c"
#include <assert.h>
#include <stdatomic.h>
#include <sys/resource.h>
#include <mach/thread_act.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s errno=%d\n", __LINE__, #x, errno); abort(); } } while (0)
static uint64_t test_now_ns(void) { return nutssync_now(0); }
static int event(int manual, int signaled)
{
    struct ntsync_event_args a = {manual, signaled};
    int fd = madsync_ioctl(MADSYNC_DEVICE_FD, NTSYNC_IOC_CREATE_EVENT, &a);
    CHECK(fd >= 0); return fd;
}
static int semaphore(unsigned count, unsigned max)
{
    struct ntsync_sem_args a = {count, max};
    int fd = madsync_ioctl(MADSYNC_DEVICE_FD, NTSYNC_IOC_CREATE_SEM, &a);
    CHECK(fd >= 0); return fd;
}
static int op(int fd, unsigned long operation)
{
    unsigned prev = 0;
    CHECK(madsync_ioctl(fd, operation, &prev) == 0); return prev;
}
static int wait_objects(int *fds, unsigned count, int all, int alert, uint64_t timeout, unsigned owner, unsigned *index)
{
    struct ntsync_wait_args a = {.objs=(uintptr_t)fds, .count=count, .alert=(unsigned)alert,
        .owner=owner, .timeout=timeout, .index=UINT32_MAX};
    int ret = madsync_ioctl(MADSYNC_DEVICE_FD, all ? NTSYNC_IOC_WAIT_ALL : NTSYNC_IOC_WAIT_ANY, &a);
    if (index) *index = a.index;
    return ret;
}
static void poll_timeout(int *fds, unsigned count, int all)
{
    CHECK(wait_objects(fds, count, all, 0, 0, 42, NULL) == -1 && errno == ETIMEDOUT);
}
static unsigned active_on(int fd)
{
    sigset_t saved; unsigned count = 0;
    if (sync_backend == MADEIRA_SYNC_NUTSSYNC)
    {
        unsigned number = (unsigned)fd & 0x0fffffff;
        nutssync_lock(&saved);
        for (unsigned i = 0; i < nutssync->active_count; i++)
        {
            struct nutssync_waiter *w = &nutssync->waiters[nutssync->active[i]];
            for (unsigned j = 0; j < w->count; j++) if (w->objs[j] == number) { count++; break; }
        }
        nutssync_unlock(&saved);
    }
    else
    {
        ms_lock(&saved);
        struct ms_obj *o = obj_of(fd);
        for (struct ms_link *l = o->head.next; l != &o->head; l = l->next) count++;
        ms_unlock(&saved);
    }
    return count;
}
static void wait_registered(int fd, unsigned count)
{
    uint64_t end = test_now_ns() + 2000000000ull;
    while (active_on(fd) != count && test_now_ns() < end) usleep(100);
    CHECK(active_on(fd) == count);
}
struct waiting { int fds[2], all, alert; unsigned count, index, owner; int result, error; };
static void *wait_thread(void *arg)
{
    struct waiting *w = arg;
    w->result = wait_objects(w->fds, w->count, w->all, w->alert, UINT64_MAX, w->owner, &w->index);
    w->error = errno;
    return NULL;
}
static void semantics(void)
{
    puts("CHECK immediate waits and event state");
    int e = event(0, 0), manual = event(1, 1); unsigned index;
    poll_timeout(&e, 1, 0);
    CHECK(op(e, NTSYNC_IOC_EVENT_SET) == 0);
    CHECK(wait_objects(&e, 1, 0, 0, 0, 1, &index) == 0 && index == 0);
    poll_timeout(&e, 1, 0);
    CHECK(wait_objects(&manual, 1, 0, 0, 0, 1, NULL) == 0);
    CHECK(wait_objects(&manual, 1, 0, 0, 0, 1, NULL) == 0);
    int pair[] = {manual, e};
    op(e, NTSYNC_IOC_EVENT_SET);
    CHECK(wait_objects(pair, 2, 0, 0, 0, 1, &index) == 0 && index == 0);
    CHECK(wait_objects(&e, 1, 0, 0, 0, 1, NULL) == 0); /* lowest index won */
    int sem = semaphore(1, 2), all[] = {sem, e};
    poll_timeout(all, 2, 1);
    struct ntsync_sem_args s;
    CHECK(madsync_ioctl(sem, NTSYNC_IOC_SEM_READ, &s) == 0 && s.count == 1);
    op(e, NTSYNC_IOC_EVENT_SET);
    CHECK(wait_objects(all, 2, 1, 0, 0, 1, NULL) == 0);
    poll_timeout(&sem, 1, 0); poll_timeout(&e, 1, 0);
    unsigned release = 3;
    CHECK(madsync_ioctl(sem, NTSYNC_IOC_SEM_RELEASE, &release) == -1 && errno == EOVERFLOW);
    int duplicate[] = {manual, manual};
    CHECK(wait_objects(duplicate, 2, 1, 0, 0, 1, NULL) == -1 && errno == EINVAL);
    int alert = event(1, 1);
    CHECK(wait_objects(&e, 1, 0, alert, 0, 1, &index) == 0 && index == 1);
    /* Manual alert is observed repeatedly until Wine dispatches/reset it. */
    CHECK(wait_objects(NULL, 0, 0, alert, 0, 1, &index) == 0 && index == 0);
    op(alert, NTSYNC_IOC_EVENT_RESET);
    poll_timeout(&e, 1, 0);
    puts("CHECK monotonic/realtime timeout");
    uint64_t start = test_now_ns();
    CHECK(wait_objects(&e, 1, 0, 0, start + 10000000, 1, NULL) == -1 && errno == ETIMEDOUT);
    CHECK(test_now_ns() - start >= 9000000 && test_now_ns() - start < 1000000000);
    struct ntsync_wait_args rt = {.objs=(uintptr_t)&e, .count=1, .owner=1,
        .timeout=nutssync_now(1) + 10000000, .flags=NTSYNC_WAIT_REALTIME};
    CHECK(madsync_ioctl(MADSYNC_DEVICE_FD, NTSYNC_IOC_WAIT_ANY, &rt) == -1 && errno == ETIMEDOUT);
    puts("CHECK mutex recursion and abandonment");
    struct ntsync_mutex_args m = {7, 1};
    int mutex = madsync_ioctl(MADSYNC_DEVICE_FD, NTSYNC_IOC_CREATE_MUTEX, &m); CHECK(mutex >= 0);
    CHECK(wait_objects(&mutex, 1, 0, 0, 0, 7, NULL) == 0);
    m.owner = 9;
    CHECK(madsync_ioctl(mutex, NTSYNC_IOC_MUTEX_UNLOCK, &m) == -1 && errno == EPERM);
    m.owner = 7; CHECK(madsync_ioctl(mutex, NTSYNC_IOC_MUTEX_UNLOCK, &m) == 0 && m.count == 2);
    unsigned owner = 7;
    CHECK(madsync_ioctl(mutex, NTSYNC_IOC_MUTEX_KILL, &owner) == 0);
    CHECK(wait_objects(&mutex, 1, 0, 0, 0, 9, &index) == -1 && errno == EOWNERDEAD && index == 0);
    m.owner = 9; CHECK(madsync_ioctl(mutex, NTSYNC_IOC_MUTEX_UNLOCK, &m) == 0);
    CHECK(wait_objects(&mutex, 1, 0, 0, 0, 10, NULL) == 0); /* abandoned only once */
    m.owner = 10; CHECK(madsync_ioctl(mutex, NTSYNC_IOC_MUTEX_UNLOCK, &m) == 0);
    /* Pulse reaches current waiters, never a future waiter. */
    puts("CHECK pulse and blocked wait-all");
    struct waiting w = {.fds={e}, .count=1, .owner=100}; pthread_t thread;
    CHECK(pthread_create(&thread, NULL, wait_thread, &w) == 0); wait_registered(e, 1);
    op(e, NTSYNC_IOC_EVENT_PULSE); CHECK(pthread_join(thread, NULL) == 0 && w.result == 0);
    poll_timeout(&e, 1, 0);
    w = (struct waiting){.fds={e, sem}, .count=2, .all=1, .owner=100};
    CHECK(pthread_create(&thread, NULL, wait_thread, &w) == 0); wait_registered(e, 1);
    op(e, NTSYNC_IOC_EVENT_SET); CHECK(active_on(e) == 1);
    release = 1; CHECK(madsync_ioctl(sem, NTSYNC_IOC_SEM_RELEASE, &release) == 0);
    CHECK(pthread_join(thread, NULL) == 0 && w.result == 0);
    puts("CHECK descriptor handoff");
    unsigned posts = sync_backend == MADEIRA_SYNC_NUTSSYNC ? 1000 : 128;
    for (unsigned i = 0; i < posts; i++) CHECK(madsync_post(10, i, e) == 0);
    CHECK(madsync_take(11, 1) == -1);
    for (unsigned i = 0; i < posts; i++) CHECK(madsync_close(madsync_take(10, i)) == 0);
    CHECK(madsync_post(10, 1, e) == 0); madsync_discard_posts(10); CHECK(madsync_take(10, 1) == -1);
    int fds[] = {e,manual,sem,alert,mutex};
    for (unsigned i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) CHECK(madsync_close(fds[i]) == 0);
    puts("PASS events, pulse, wait-any/all, APC alerts, mutex abandonment, semaphore overflow, timeouts, handoff");
}
static void terminate_handler(int sig) { (void)sig; pthread_exit(NULL); }
static void termination(void)
{
    struct sigaction action = {.sa_handler=terminate_handler}; sigemptyset(&action.sa_mask);
    CHECK(sigaction(SIGUSR1, &action, NULL) == 0);
    int e = event(0, 0);
    for (unsigned i = 0; i < 64; i++)
    {
        struct waiting w = {.fds={e}, .count=1, .owner=123}; pthread_t thread;
        CHECK(pthread_create(&thread, NULL, wait_thread, &w) == 0); wait_registered(e, 1);
        CHECK(pthread_kill(thread, SIGUSR1) == 0); CHECK(pthread_join(thread, NULL) == 0);
        CHECK(active_on(e) == 0);
        op(e, NTSYNC_IOC_EVENT_SET);
        CHECK(wait_objects(&e, 1, 0, 0, 0, 124, NULL) == 0); /* dead waiter cannot steal signal */
    }
    CHECK(madsync_ref(e) == e);
    struct waiting w = {.fds={e}, .count=1, .owner=123}; pthread_t thread;
    CHECK(pthread_create(&thread, NULL, wait_thread, &w) == 0); wait_registered(e, 1);
    CHECK(madsync_close(e) == 0);
    op(e, NTSYNC_IOC_EVENT_SET); CHECK(pthread_join(thread, NULL) == 0 && w.result == 0);
    CHECK(madsync_close(e) == 0);
    if (sync_backend == MADEIRA_SYNC_NUTSSYNC)
    {
        CHECK(madsync_ref(e) == -1);
        for (unsigned i = 0; i < 2200; i++)
        {
            int other = event(0, 0); CHECK(other != e); CHECK(madsync_ref(e) == -1); CHECK(madsync_close(other) == 0);
        }
        CHECK(nutssync->active_count == 0);
        for (unsigned i = 0; i < nutssync->waiters_used; i++) CHECK(!nutssync->waiters[i].in_use);
    }
    puts("PASS native thread termination, close during wait, no stale descriptor reuse");
}

static _Atomic int suspend_running, suspend_entered;
static void *suspend_worker(void *arg)
{
    (void)arg;
    atomic_store(&suspend_entered, 1);
    while (atomic_load(&suspend_running))
    {
        sigset_t saved;
        nutssync_lock(&saved);
        /* Make the former lock-holder suspension problem reproducible. */
        for (volatile unsigned i = 0; i < 1000; i++) {}
        nutssync_unlock(&saved);
    }
    return NULL;
}
static void suspension(void)
{
    if (sync_backend != MADEIRA_SYNC_NUTSSYNC) return;
    int e = event(0, 0); pthread_t thread;
    atomic_store(&suspend_running, 1);
    CHECK(pthread_create(&thread, NULL, suspend_worker, NULL) == 0);
    while (!atomic_load(&suspend_entered)) usleep(100);
    mach_port_t port = pthread_mach_thread_np(thread);
    for (unsigned i = 0; i < 300; i++)
    {
        struct madsync_suspend_guard guard;
        madsync_suspend_begin(&guard);
        CHECK(thread_suspend(port) == KERN_SUCCESS);
        madsync_suspend_end(&guard);
        op(e, NTSYNC_IOC_EVENT_SET);
        CHECK(wait_objects(&e, 1, 0, 0, 0, 100, NULL) == 0);
        CHECK(thread_resume(port) == KERN_SUCCESS);
    }
    atomic_store(&suspend_running, 0); CHECK(pthread_join(thread, NULL) == 0); CHECK(madsync_close(e) == 0);
    puts("PASS 300 persistent Mach suspensions during sync contention; other threads still signal/wait");
}

struct job { int ready, done; unsigned rounds, owner; };
static void *job_worker(void *arg)
{
    struct job *job = arg;
    for (unsigned i = 0; i < job->rounds; i++)
    {
        CHECK(wait_objects(&job->ready, 1, 0, 0, UINT64_MAX, job->owner, NULL) == 0);
        op(job->done, NTSYNC_IOC_EVENT_SET);
    }
    return NULL;
}
static void jobs(void)
{
    enum { WORKERS=8, ROUNDS=2000 }; struct job jobs[WORKERS]; pthread_t threads[WORKERS]; int done[WORKERS];
    for (unsigned i = 0; i < WORKERS; i++)
    {
        jobs[i]=(struct job){event(0,0),event(0,0),ROUNDS,1000+i}; done[i]=jobs[i].done;
        CHECK(pthread_create(&threads[i],NULL,job_worker,&jobs[i]) == 0);
    }
    struct rusage before, after; getrusage(RUSAGE_SELF, &before); uint64_t start=test_now_ns();
    for (unsigned r = 0; r < ROUNDS; r++)
    {
        for (unsigned i = 0; i < WORKERS; i++) op(jobs[i].ready,NTSYNC_IOC_EVENT_SET);
        CHECK(wait_objects(done,WORKERS,1,0,UINT64_MAX,999,NULL) == 0);
    }
    double wall=(test_now_ns()-start)/1e6; getrusage(RUSAGE_SELF,&after);
    double cpu=(after.ru_utime.tv_sec-before.ru_utime.tv_sec+after.ru_stime.tv_sec-before.ru_stime.tv_sec)*1000.0+
        (after.ru_utime.tv_usec-before.ru_utime.tv_usec+after.ru_stime.tv_usec-before.ru_stime.tv_usec)/1000.0;
    for (unsigned i=0;i<WORKERS;i++)
    { CHECK(pthread_join(threads[i],NULL)==0); CHECK(madsync_close(jobs[i].ready)==0); CHECK(madsync_close(jobs[i].done)==0); }
    printf("BENCH backend=%d rounds=%d workers=%d wall_ms=%.3f cpu_ms=%.3f\n",sync_backend,ROUNDS,WORKERS,wall,cpu);
    puts("PASS 16000 concurrent jobs with atomic wait-all; no lost wakes");
}
static void capacity(void)
{
    if (sync_backend != MADEIRA_SYNC_NUTSSYNC) return;
    int *fds=malloc(sizeof(int)*NUTSSYNC_MAX_OBJECTS); CHECK(fds);
    unsigned n=0; struct ntsync_event_args a={0,0};
    while(n<NUTSSYNC_MAX_OBJECTS)
    { int fd=madsync_ioctl(MADSYNC_DEVICE_FD,NTSYNC_IOC_CREATE_EVENT,&a); if(fd<0) break; fds[n++]=fd; }
    CHECK(n>130000 && n<=NUTSSYNC_MAX_OBJECTS);
    CHECK(madsync_ref(fds[n-1])==fds[n-1]); CHECK(madsync_close(fds[n-1])==0);
    CHECK(madsync_ioctl(MADSYNC_DEVICE_FD,NTSYNC_IOC_CREATE_EVENT,&a)==-1 && errno==ENOMEM);
    for(unsigned i=0;i<n;i++) CHECK(madsync_close(fds[i])==0); free(fds);
    for(unsigned i=0;i<nutssync->used;i++) CHECK(ns_refs[i]==0 && nutssync->objects[i].type==NUTSSYNC_FREE);
    puts("PASS complete object capacity, valid final slot, references fully reclaimed");
}
int main(int argc,char **argv)
{
    (void)argc;(void)argv; alarm(50); setvbuf(stdout,NULL,_IONBF,0);
    CHECK(madsync_enabled());
    semantics(); puts("CHECK native termination"); termination();
    puts("CHECK Mach suspend"); suspension(); puts("CHECK concurrent jobs"); jobs(); capacity();
    puts("PASS production synchronization suite"); return 0;
}
