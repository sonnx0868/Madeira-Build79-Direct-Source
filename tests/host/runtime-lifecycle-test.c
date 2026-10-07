#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../app/Madeira/GameRuntime.h"
unsigned runtime_test_self;
int runtime_test_alive[8192], runtime_test_refs[8192];
static void attach(unsigned port, unsigned pid, unsigned tid, unsigned parent, uintptr_t peb) {
    runtime_test_self = port; runtime_test_alive[port] = 1;
    wine_runtime_thread_attach(pid, tid, parent, (void *)peb);
}
int main(int argc, char **argv) {
    wine_runtime_enable(); wine_runtime_begin_generation(1);
    attach(1, 1, 1, 0, 0x1000);
    assert(wine_runtime_reuse_ready() == -1); /* no child lifecycle evidence */
    attach(2, 2, 2, 1, 0x2000);
    assert(wine_runtime_live_threads() == 1 && runtime_test_refs[2] == 1);
    if (argc > 1 && !strcmp(argv[1], "unsafe")) {
        attach(3, 2, 3, 0, 0x2000); runtime_test_self = 2;
        assert(!wine_runtime_prepare_process_exit(2, (void *)0x2000));
        wine_runtime_process_retired(2, (void *)0x2000, 0);
        runtime_test_alive[2] = runtime_test_alive[3] = 0;
        assert(wine_runtime_reuse_ready() == -1);
        puts("PASS: live native peers quarantine retirement and forbid another game"); return 0;
    }
    wine_runtime_thread_reserve(2, 3, (void *)0x2000);
    assert(wine_runtime_live_threads() == 2);
    wine_runtime_thread_cancel(2, 3, (void *)0x2000);
    assert(wine_runtime_live_threads() == 1);
    assert(wine_runtime_prepare_process_exit(2, (void *)0x2000));
    wine_runtime_process_retired(2, (void *)0x2000, 1);
    assert(!wine_runtime_reuse_ready()); /* signaled Windows process is not enough */
    runtime_test_alive[2] = -1;
    assert(!wine_runtime_reuse_ready()); /* query failure is not death */
    runtime_test_alive[2] = 0;
    wine_runtime_gpu_begin(); assert(!wine_runtime_reuse_ready());
    wine_runtime_gpu_end(); assert(wine_runtime_reuse_ready() == 1);
    wine_runtime_begin_generation(2); assert(runtime_test_refs[2] == 0);
    attach(4, 2, 4, 1, 0x2000); /* PID and PEB reused only after the old generation drained */
    attach(5, 5, 5, 2, 0x5000); /* grandchild belongs to this generation */
    runtime_test_self = 4;
    assert(wine_runtime_prepare_process_exit(2, (void *)0x2000));
    wine_runtime_process_retired(2, (void *)0x2000, 1); runtime_test_alive[4] = 0;
    assert(!wine_runtime_reuse_ready());
    runtime_test_self = 5;
    assert(wine_runtime_prepare_process_exit(5, (void *)0x5000));
    wine_runtime_process_retired(5, (void *)0x5000, 1); runtime_test_alive[5] = 0;
    assert(wine_runtime_reuse_ready() == 1);
    wine_runtime_set_processors(4); assert(wine_runtime_processors() == 4);
    wine_runtime_set_processors(64); assert(!wine_runtime_processors());
    puts("PASS: actual production lifecycle policy: pending threads, ports, GPU, descendants, two generations and CPU reporting");
    return 0;
}
