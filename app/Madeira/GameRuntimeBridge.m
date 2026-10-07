/* SPDX-License-Identifier: GPL-3.0-or-later
 * Madeira Converter Exception: see LICENSE-EXCEPTION.md.
 * Read-only Mach liveness checks. Never suspends, cancels or kills host threads.
 * A Windows process handle signaled by wineserver alone is NOT enough to
 * recycle the FEX/JIT address space shared by all pseudo-processes. */
#include "GameRuntime.h"
#ifdef MADEIRA_RUNTIME_UNIT_TEST
#include "runtime-mach-test.h"
#else
#include <mach/mach.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include <stdatomic.h>
#include <string.h>
#include <time.h>

enum { MAX_RUNTIME_PROCESSES = 512, MAX_RUNTIME_THREADS = 8192 };
struct runtime_process {
    unsigned pid, parent;
    void *peb;
    uint64_t generation;
    int retired, safe;
};
struct runtime_thread { unsigned pid, tid; void *peb; mach_port_t port; int reserved; };
static struct runtime_process runtime_processes[MAX_RUNTIME_PROCESSES];
static struct runtime_thread runtime_threads[MAX_RUNTIME_THREADS];
static pthread_mutex_t runtime_lock = PTHREAD_MUTEX_INITIALIZER;
static _Atomic int runtime_enabled, runtime_unsafe;
static _Atomic unsigned runtime_gpu;
static _Atomic unsigned runtime_processors;
static unsigned runtime_root;
static uint64_t runtime_generation;

static int port_alive(mach_port_t port) {
    thread_basic_info_data_t info;
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    if (!port) return 1; /* pending thread: no proof of exit */
    kern_return_t result = thread_info(port, THREAD_BASIC_INFO, (thread_info_t)&info, &count);
    if (result == KERN_SUCCESS) return 1;
    /* An arbitrary query failure is not permission to reuse memory. */
    return result != KERN_INVALID_ARGUMENT && result != MACH_SEND_INVALID_DEST;
}
static struct runtime_process *process_find(unsigned pid, void *peb, int create, unsigned parent) {
    struct runtime_process *empty = NULL;
    for (unsigned i = 0; i < MAX_RUNTIME_PROCESSES; ++i) {
        struct runtime_process *p = &runtime_processes[i];
        if (p->pid == pid && p->peb == peb) return p;
        if (!p->pid && !empty) empty = p;
    }
    if (!create) return NULL;
    if (!empty || !pid || !peb) { atomic_store(&runtime_unsafe, 1); return NULL; }
    uint64_t epoch = 0;
    if (!runtime_root && !parent) runtime_root = pid;
    if (pid != runtime_root) {
        if (parent == runtime_root) epoch = runtime_generation;
        for (unsigned i = 0; !epoch && i < MAX_RUNTIME_PROCESSES; ++i)
            if (runtime_processes[i].pid == parent && runtime_processes[i].generation == runtime_generation)
                epoch = runtime_generation;
        if (!epoch) { atomic_store(&runtime_unsafe, 1); epoch = runtime_generation; }
    }
    *empty = (struct runtime_process){ .pid = pid, .parent = parent, .peb = peb, .generation = epoch };
    return empty;
}
static void thread_record(unsigned pid, unsigned tid, unsigned parent, void *peb, int reserved) {
    if (!wine_runtime_is_enabled() || !pid || !tid || !peb) return;
    mach_port_t port = reserved ? 0 : pthread_mach_thread_np(pthread_self());
    pthread_mutex_lock(&runtime_lock);
    if (!reserved) process_find(pid, peb, 1, parent);
    struct runtime_thread *slot = NULL;
    for (unsigned i = 0; i < MAX_RUNTIME_THREADS; ++i) {
        struct runtime_thread *t = &runtime_threads[i];
        if (t->pid == pid && t->tid == tid && t->peb == peb) { slot = t; break; }
        if (!t->pid && !slot) slot = t;
    }
    if (!slot) atomic_store(&runtime_unsafe, 1);
    else {
        if (port && slot->port != port) {
            if (mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_SEND, 1) != KERN_SUCCESS)
                atomic_store(&runtime_unsafe, 1);
            else {
                if (slot->port) mach_port_deallocate(mach_task_self(), slot->port);
                slot->port = port;
            }
        }
        slot->pid = pid; slot->tid = tid; slot->peb = peb;
        if (!reserved || !slot->port) slot->reserved = reserved;
    }
    pthread_mutex_unlock(&runtime_lock);
}
void wine_runtime_enable(void) { atomic_store(&runtime_enabled, 1); }
void wine_runtime_set_processors(unsigned count) { atomic_store(&runtime_processors, count > 0 && count < 64 ? count : 0); }
unsigned wine_runtime_processors(void) { return atomic_load(&runtime_processors); }
int wine_runtime_is_enabled(void) { return atomic_load(&runtime_enabled); }
void wine_runtime_thread_attach(unsigned pid, unsigned tid, unsigned parent, void *peb) { thread_record(pid, tid, parent, peb, 0); }
void wine_runtime_thread_reserve(unsigned pid, unsigned tid, void *peb) { thread_record(pid, tid, 0, peb, 1); }
void wine_runtime_thread_cancel(unsigned pid, unsigned tid, void *peb) {
    if (!wine_runtime_is_enabled()) return;
    pthread_mutex_lock(&runtime_lock);
    for (unsigned i = 0; i < MAX_RUNTIME_THREADS; ++i) {
        struct runtime_thread *t = &runtime_threads[i];
        if (t->pid == pid && t->tid == tid && t->peb == peb && t->reserved && !t->port) memset(t, 0, sizeof(*t));
    }
    pthread_mutex_unlock(&runtime_lock);
}
void wine_runtime_begin_generation(uint64_t generation) {
    pthread_mutex_lock(&runtime_lock);
    /* Only the state machine calls this after reuse_ready == 1. Do not drop
     * live references as a way to make the next readiness check pass. */
    for (unsigned i = 0; i < MAX_RUNTIME_THREADS; ++i) {
        struct runtime_thread *t = &runtime_threads[i];
        if (t->pid && t->pid != runtime_root && !t->reserved && !port_alive(t->port)) {
            if (t->port) mach_port_deallocate(mach_task_self(), t->port);
            memset(t, 0, sizeof(*t));
        }
    }
    for (unsigned i = 0; i < MAX_RUNTIME_PROCESSES; ++i) {
        struct runtime_process *p = &runtime_processes[i];
        if (p->pid && p->pid != runtime_root && p->retired && p->safe) memset(p, 0, sizeof(*p));
    }
    runtime_generation = generation;
    pthread_mutex_unlock(&runtime_lock);
}
int wine_runtime_prepare_process_exit(unsigned pid, void *peb) {
    if (!wine_runtime_is_enabled()) return 1;
    unsigned self_tid = 0;
    mach_port_t self_port = pthread_mach_thread_np(pthread_self());
    /* Ordinary orderly quits normally already joined the game's workers.
     * Give their native destructors 250 ms, but never unmap code beneath an
     * executing peer just because wineserver considers its thread terminated. */
    for (unsigned attempt = 0; attempt < 26; ++attempt) {
        unsigned peers = 0;
        pthread_mutex_lock(&runtime_lock);
        for (unsigned i = 0; i < MAX_RUNTIME_THREADS; ++i) {
            struct runtime_thread *t = &runtime_threads[i];
            if (t->pid == pid && t->peb == peb) {
                if (t->port == self_port) self_tid = t->tid;
                else if (t->reserved || port_alive(t->port)) ++peers;
            }
        }
        pthread_mutex_unlock(&runtime_lock);
        if (!peers && self_tid && !atomic_load(&runtime_unsafe)) return 1;
        if (attempt < 25) usleep(10000);
    }
    atomic_store(&runtime_unsafe, 1);
    return 0;
}
void wine_runtime_process_retired(unsigned pid, void *peb, int safe) {
    if (!wine_runtime_is_enabled()) return;
    pthread_mutex_lock(&runtime_lock);
    struct runtime_process *p = process_find(pid, peb, 0, 0);
    if (p) { p->retired = 1; p->safe = safe; }
    else atomic_store(&runtime_unsafe, 1);
    if (!safe) atomic_store(&runtime_unsafe, 1);
    pthread_mutex_unlock(&runtime_lock);
}
static unsigned live_locked(void) {
    unsigned live = 0;
    for (unsigned i = 0; i < MAX_RUNTIME_THREADS; ++i) {
        struct runtime_thread *t = &runtime_threads[i];
        struct runtime_process *p = t->pid ? process_find(t->pid, t->peb, 0, 0) : NULL;
        if (p && p->generation == runtime_generation && (t->reserved || port_alive(t->port))) ++live;
    }
    return live;
}
unsigned wine_runtime_live_threads(void) {
    pthread_mutex_lock(&runtime_lock); unsigned live = live_locked(); pthread_mutex_unlock(&runtime_lock); return live;
}
int wine_runtime_reuse_ready(void) {
    if (!wine_runtime_is_enabled() || atomic_load(&runtime_unsafe)) return -1;
    unsigned found = 0, pending = 0;
    pthread_mutex_lock(&runtime_lock);
    for (unsigned i = 0; i < MAX_RUNTIME_PROCESSES; ++i) {
        struct runtime_process *p = &runtime_processes[i];
        if (p->pid && p->generation == runtime_generation) { ++found; if (!p->retired || !p->safe) ++pending; }
    }
    unsigned live = live_locked();
    pthread_mutex_unlock(&runtime_lock);
    if (!found) return -1; /* an old cached native bundle must never look clean */
    return !pending && !live && !wine_runtime_gpu_pending() ? 1 : 0;
}
void wine_runtime_gpu_begin(void) { atomic_fetch_add(&runtime_gpu, 1); }
void wine_runtime_gpu_end(void) {
    unsigned before = atomic_fetch_sub(&runtime_gpu, 1);
    if (!before) { atomic_store(&runtime_gpu, 0); atomic_store(&runtime_unsafe, 1); }
}
unsigned wine_runtime_gpu_pending(void) { return atomic_load(&runtime_gpu); }
