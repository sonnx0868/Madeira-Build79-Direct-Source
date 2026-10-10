/* LGPL-2.1-or-later. Host-to-server control for a selected Wine guest.
 * The host only queues a Windows PID. Wine's server thread owns process
 * objects and uses its normal termination/signaling path. Never kill the
 * native app, cancel pthreads or terminate Mach threads here.
 */
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/thread_info.h>
#include "object.h"
#include "process.h"
#include "thread.h"

#define STOP_PORTS 512
static _Atomic unsigned int requested_pid;
struct stop_snapshot { unsigned int pid, count; mach_port_t ports[STOP_PORTS]; };
static _Atomic unsigned int watching_pid;
extern void ios_wineserver_wake(void);
extern void wine_session_process_did_exit(unsigned int pid);

void ios_wineserver_request_game_stop(uint32_t pid)
{
    if (!pid) return;
    atomic_store_explicit(&requested_pid, pid, memory_order_release);
    ios_wineserver_wake();
}

static void release_snapshot(struct stop_snapshot *stop)
{
    for (unsigned int i = 0; i < stop->count; ++i)
        mach_port_deallocate(mach_task_self(), stop->ports[i]);
    free(stop);
}

static void *watch_exit(void *arg)
{
    struct stop_snapshot *stop = arg;
    int exited = 0;
    /* Native observations only: no Wine TEB or server object access. Keep
     * watching if the server loop itself stops after the guest exits. */
    for (unsigned int attempt = 0; attempt < 750; ++attempt) {
        unsigned int alive = 0;
        for (unsigned int i = 0; i < stop->count; ++i) {
            thread_basic_info_data_t info;
            mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
            kern_return_t result = thread_info(stop->ports[i], THREAD_BASIC_INFO,
                                               (thread_info_t)&info, &count);
            if (result != KERN_INVALID_ARGUMENT && result != MACH_SEND_INVALID_DEST) ++alive;
        }
        if (!alive) { exited = 1; break; }
        usleep(20000);
    }
    if (exited) {
        wine_session_process_did_exit(stop->pid);
        dprintf(2, "[session-close] server-stop-v1 pid=%04x native threads exited\n", stop->pid);
    } else dprintf(2, "[session-close] server-stop-v1 pid=%04x native exit not confirmed\n", stop->pid);
    release_snapshot(stop);
    atomic_store_explicit(&watching_pid, 0, memory_order_release);
    return NULL;
}

void ios_wineserver_drain_game_stop(void)
{
    unsigned int pid = atomic_exchange_explicit(&requested_pid, 0, memory_order_acq_rel);
    if (!pid || atomic_load_explicit(&watching_pid, memory_order_acquire)) return;
    struct process *process = get_process_from_id(pid);
    if (!process) return;
    struct stop_snapshot *stop = calloc(1, sizeof(*stop));
    if (!stop) { release_object(process); return; }
    stop->pid = pid;
    struct thread *thread;
    int complete = 1;
    LIST_FOR_EACH_ENTRY(thread, &process->thread_list, struct thread, proc_entry) {
        mach_port_t port;
        mach_msg_type_name_t type;
        if (stop->count == STOP_PORTS || thread->unix_tid == (unsigned int)-1 ||
            mach_port_extract_right(mach_task_self(), thread->unix_tid,
                                     MACH_MSG_TYPE_COPY_SEND, &port, &type) != KERN_SUCCESS) {
            complete = 0; break;
        }
        stop->ports[stop->count++] = port;
    }
    if (complete && stop->count) {
        pthread_t watcher;
        atomic_store_explicit(&watching_pid, pid, memory_order_release);
        if (pthread_create(&watcher, NULL, watch_exit, stop) == 0) {
            pthread_detach(watcher);
            dprintf(2, "[session-close] server-stop-v1 pid=%04x termination queued\n", pid);
            kill_process(process, 1);
            stop = NULL; /* watcher owns the port references */
        } else atomic_store_explicit(&watching_pid, 0, memory_order_release);
    }
    if (stop) release_snapshot(stop);
    release_object(process);
}
