/* Included after the real old/new NtYieldExecution implementations. */
#include <stdatomic.h>
#include <pthread.h>
#include <sys/resource.h>

static _Atomic unsigned int turn, ready;
static unsigned int rounds;
static uint64_t work_ns, handshake_sum;
static NTSTATUS (*yield_call)(void);

static uint64_t bench_now(void) { return clock_gettime_nsec_np(CLOCK_UPTIME_RAW); }
static void do_work(uint64_t ns)
{
    if (!ns) return;
    uint64_t end = bench_now() + ns;
    while (bench_now() < end) atomic_signal_fence(memory_order_seq_cst);
}
static void *handoff_worker(void *unused)
{
    (void)unused;
    assert(!pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0));
    atomic_store_explicit(&ready,1,memory_order_release);
    for (unsigned int i=0; i<rounds; ++i)
    {
        while (atomic_load_explicit(&turn,memory_order_acquire)!=1) yield_call();
        do_work(work_ns);
        handshake_sum += i;
        atomic_store_explicit(&turn,0,memory_order_release);
    }
    return NULL;
}
static double cpu_ms(struct rusage *a, struct rusage *b)
{
    return (b->ru_utime.tv_sec-a->ru_utime.tv_sec+b->ru_stime.tv_sec-a->ru_stime.tv_sec)*1000. +
        (b->ru_utime.tv_usec-a->ru_utime.tv_usec+b->ru_stime.tv_usec-a->ru_stime.tv_usec)/1000.;
}
int main(int argc,char **argv)
{
    assert(argc==3);
    assert(!pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0));
    yield_call = !strcmp(argv[1],"legacy") ? legacy_yield_execution : NtYieldExecution;
    yield_call(); /* Warm shared configuration before creating another thread. */
    struct rusage a,b;
    if (!strcmp(argv[2],"poll"))
    {
        getrusage(RUSAGE_SELF,&a);uint64_t start=bench_now();
        while (bench_now()-start<120000000) yield_call();
        uint64_t end=bench_now();getrusage(RUSAGE_SELF,&b);
        assert(ios_xp_yield_slept>0);
        printf("{\"engine\":\"%s\",\"case\":\"poll\",\"wall_ms\":%.3f,\"cpu_ms\":%.3f,\"yields\":%lld,\"parks\":%lld}\n",
               argv[1],(end-start)/1e6,cpu_ms(&a,&b),ios_xp_yield,ios_xp_yield_slept);
        return 0;
    }
    rounds = !strcmp(argv[2],"rapid") ? 10000 : 300;
    work_ns = !strcmp(argv[2],"rapid") ? 0 : 150000;
    pthread_t worker;assert(!pthread_create(&worker,NULL,handoff_worker,NULL));
    while (!atomic_load_explicit(&ready,memory_order_acquire)) sched_yield();
    getrusage(RUSAGE_SELF,&a);uint64_t start=bench_now();
    for (unsigned int i=0;i<rounds;++i)
    {
        do_work(work_ns);
        atomic_store_explicit(&turn,1,memory_order_release);
        while (atomic_load_explicit(&turn,memory_order_acquire)!=0) yield_call();
    }
    assert(!pthread_join(worker,NULL));uint64_t end=bench_now();getrusage(RUSAGE_SELF,&b);
    assert(handshake_sum==(uint64_t)rounds*(rounds-1)/2);
    printf("{\"engine\":\"%s\",\"case\":\"%s\",\"rounds\":%u,\"wall_ms\":%.3f,\"cpu_ms\":%.3f,\"yields\":%lld,\"parks\":%lld,\"checksum\":%llu}\n",
           argv[1],argv[2],rounds,(end-start)/1e6,cpu_ms(&a,&b),ios_xp_yield,ios_xp_yield_slept,(unsigned long long)handshake_sum);
    return 0;
}
