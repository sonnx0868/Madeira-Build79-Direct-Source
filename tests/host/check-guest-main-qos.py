#!/usr/bin/env python3
"""Run the production guest thread attributes on Darwin and reproduce the old failure."""
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile

if platform.system() != 'Darwin':
    if '--require-darwin' in sys.argv:
        raise SystemExit('Darwin pthread/Mach APIs required')
    print('SKIP: guest main QoS behavior requires Darwin; enforced by Mac CI')
    raise SystemExit(0)

root = Path(__file__).resolve().parents[2]
source = (root / 'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')

def function(signature):
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

start = function('int wine_process_start(const char *prefix_path)')
attributes = start[start.index('    pthread_attr_t attr;'):start.index('    if (ret != 0)')]
fixture = r'''
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <pthread/qos.h>
#include <mach/mach.h>
#include <stdio.h>
#include <unistd.h>
'''+function('static void madeira_guest_main_qos(void)')+r'''
static pthread_t g_wine_thread;
static int legacy, legacyPriority;
static void *wine_process_thread(void *unused) {
    (void)unused;
    qos_class_t before = QOS_CLASS_UNSPECIFIED, after = QOS_CLASS_UNSPECIFIED;
    int relative = 0;
    assert(pthread_get_qos_class_np(pthread_self(), &before, &relative) == 0);
    int setStatus = pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    madeira_guest_main_qos();
    assert(pthread_get_qos_class_np(pthread_self(), &after, &relative) == 0);
    thread_extended_info_data_t ext = {0};
    mach_msg_type_number_t count = THREAD_EXTENDED_INFO_COUNT;
    assert(thread_info(pthread_mach_thread_np(pthread_self()), THREAD_EXTENDED_INFO,
                       (thread_info_t)&ext, &count) == KERN_SUCCESS);
    printf("guest-main-qos: %s before=%#x set_rc=%d after=%#x relative=%d priority=%d\n",
           legacy ? "legacy" : "production", before, setStatus, after, relative, ext.pth_priority);
    if (legacy) {
        assert(before == QOS_CLASS_UNSPECIFIED && after == QOS_CLASS_UNSPECIFIED);
        assert(setStatus == EPERM);
        // macOS may inherit priority 31 despite the attribute requesting 20;
        // iPad build 685 actually reports 20. Both lose the QoS class.
        legacyPriority = ext.pth_priority;
    } else {
        // The new pthread must have QoS even before its first self-promotion.
        assert(before == QOS_CLASS_USER_INTERACTIVE && after == QOS_CLASS_USER_INTERACTIVE);
        assert(setStatus == 0 && relative == 0 && ext.pth_priority > legacyPriority);
    }
    return NULL;
}
static int create_production_thread(void) {
'''+attributes+r'''
    return ret;
}
int main(void) {
    // Reproduce build 685's actual attributes on the same Darwin host.
    legacy = 1;
    pthread_attr_t old;
    struct sched_param fixed = {.sched_priority = 20};
    assert(pthread_attr_init(&old) == 0);
    assert(pthread_attr_setschedparam(&old, &fixed) == 0);
    assert(pthread_create(&g_wine_thread, &old, wine_process_thread, NULL) == 0);
    assert(pthread_attr_destroy(&old) == 0);
    assert(pthread_join(g_wine_thread, NULL) == 0);
    legacy = 0;
    assert(create_production_thread() == 0);
    assert(pthread_join(g_wine_thread, NULL) == 0);
    puts("PASS: legacy fixed priority rejects QoS; production thread starts interactive and accepts QoS");
}
'''
compiler = shutil.which('clang')
if not compiler:
    raise SystemExit('clang required')
with tempfile.TemporaryDirectory(prefix='madeira-main-qos-') as scratch:
    folder = Path(scratch)
    test, executable = folder / 'check.c', folder / 'check'
    test.write_text(fixture, encoding='utf-8')
    subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', str(test), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
