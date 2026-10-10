#!/usr/bin/env python3
"""Exercise the production server wake gate against real Mach semaphores."""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/wineserver/fd_ios.c').read_text(encoding='utf-8')
gate = source[source.index('semaphore_t ios_srv_wake_sem ='):source.index('/* __WINESRC__')]
assert source.count('ios_wineserver_wake_consumed( wkr )') == 1
assert 'ios_wineserver_wake_consumed( KERN_SUCCESS )' in source
if '--static-only' in sys.argv:
    print('PASS: both server wait paths consume the production wake gate')
    raise SystemExit(0)

checks = r'''
#include <assert.h>
#include <mach/mach.h>
#include <mach/semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
''' + gate + r'''
static void *burst(void *unused)
{
    for (int i = 0; i < 20000; i++) ios_wineserver_wake();
    return NULL;
}
static unsigned queued, finished;
static void *writer(void *unused)
{
    for (int i = 0; i < 10000; i++)
    {
        __atomic_fetch_add(&queued, 1, __ATOMIC_RELEASE);
        ios_wineserver_wake();
        if (!(i % 127)) sched_yield();
    }
    __atomic_fetch_add(&finished, 1, __ATOMIC_RELEASE);
    ios_wineserver_wake();
    return NULL;
}
int main(void)
{
    pthread_t threads[8];
    mach_timespec_t zero = {0, 0}, tick = {0, 1000000};
    assert(semaphore_create(mach_task_self(), &ios_srv_wake_sem, SYNC_POLICY_FIFO, 0) == KERN_SUCCESS);
    for (int i = 0; i < 8; i++) assert(!pthread_create(&threads[i], NULL, burst, NULL));
    for (int i = 0; i < 8; i++) assert(!pthread_join(threads[i], NULL));
    assert(semaphore_timedwait(ios_srv_wake_sem, zero) == KERN_SUCCESS);
    // Requests after consumption and before resetting belong to the current scan.
    ios_wineserver_wake();
    assert(semaphore_timedwait(ios_srv_wake_sem, zero) == KERN_OPERATION_TIMED_OUT);
    ios_wineserver_wake_consumed(KERN_SUCCESS);
    assert(!ios_srv_wake_pending);
    // A timed-out/aborted wait must not discard a pending wake.
    ios_wineserver_wake();
    ios_wineserver_wake_consumed(KERN_OPERATION_TIMED_OUT);
    ios_wineserver_wake_consumed(KERN_ABORTED);
    assert(ios_srv_wake_pending);
    assert(semaphore_timedwait(ios_srv_wake_sem, zero) == KERN_SUCCESS);
    ios_wineserver_wake_consumed(KERN_SUCCESS);
    unsigned handled = 0, rounds = 0;
    for (int i = 0; i < 8; i++) assert(!pthread_create(&threads[i], NULL, writer, NULL));
    while (__atomic_load_n(&finished, __ATOMIC_ACQUIRE) < 8 || __atomic_load_n(&queued, __ATOMIC_ACQUIRE))
    {
        kern_return_t result = semaphore_timedwait(ios_srv_wake_sem, tick);
        assert(result == KERN_SUCCESS || result == KERN_OPERATION_TIMED_OUT);
        ios_wineserver_wake_consumed(result);
        handled += __atomic_exchange_n(&queued, 0, __ATOMIC_ACQ_REL); // production fd scan
        assert(++rounds < 20000);
    }
    for (int i = 0; i < 8; i++) assert(!pthread_join(threads[i], NULL));
    assert(handled == 80000);
    // At most one unconsumed wake remains after the concurrent requests.
    kern_return_t last = semaphore_timedwait(ios_srv_wake_sem, zero);
    ios_wineserver_wake_consumed(last);
    assert(semaphore_timedwait(ios_srv_wake_sem, zero) == KERN_OPERATION_TIMED_OUT);
    assert(semaphore_destroy(mach_task_self(), ios_srv_wake_sem) == KERN_SUCCESS);
    puts("PASS: concurrent bursts coalesce, new requests wake, timeout keeps pending work");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-server-wake-') as tmp:
    path = Path(tmp)
    (path / 'check.c').write_text(checks, encoding='utf-8')
    cc = os.environ.get('CC') or shutil.which('clang')
    subprocess.run([cc, '-std=gnu11', '-Wall', '-Werror', '-Wno-unused-parameter',
                    '-fsanitize=address,undefined', str(path / 'check.c'), '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=40)
