#!/usr/bin/env python3
"""Run the integrated thread-name policy with real Darwin QoS calls."""
from pathlib import Path
import subprocess
import tempfile
import sys

if sys.platform != 'darwin':
    raise SystemExit('SKIP: actual pthread QoS checks run in Mac CI')
root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/thread_ios.c').read_text(encoding='utf-8')
start = source.index('#define TQ_MAX_RULES')
end = source.index('/* start_thread, once the TEB is published:', start)
policy = source[start:end]
code = r'''
#include <assert.h>
#include <pthread.h>
#include <pthread/qos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
''' + policy + r'''
static void *run(void *arg) {
  (void)arg;
  assert(tq_enabled() && tq_nrules==2);
  assert(tq_glob("Worker*","Worker3") && !tq_glob("Worker?","Worker33"));
  assert(tq_glob("Gfx?","Gfx1") && !tq_glob("Gfx?","Gfx"));
  tq_self_tid=7;
  assert(tq_store(7,QOS_CLASS_UTILITY,"Worker3"));
  assert(!madeira_guest_qos_self(QOS_CLASS_USER_INTERACTIVE,0));
  qos_class_t actual;int relative;
  assert(!pthread_get_qos_class_np(pthread_self(),&actual,&relative) && actual==QOS_CLASS_UTILITY);
  // ECO/base class takes precedence over name rules.
  assert(!madeira_guest_qos_self(QOS_CLASS_BACKGROUND,0));
  assert(!pthread_get_qos_class_np(pthread_self(),&actual,&relative) && actual==QOS_CLASS_BACKGROUND);
  // Removing a match restores the requested class, never a stale thread's class.
  assert(tq_store(7,QOS_CLASS_UNSPECIFIED,NULL));
  assert(!madeira_guest_qos_self(QOS_CLASS_USER_INTERACTIVE,0));
  assert(!pthread_get_qos_class_np(pthread_self(),&actual,&relative) && actual==QOS_CLASS_USER_INTERACTIVE);
  assert(tq_lookup(7,NULL)==QOS_CLASS_UNSPECIFIED);
  for(unsigned tid=1;tid<=TQ_MAX_THREADS;++tid) tq_store(tid,QOS_CLASS_UTILITY,"Worker");
  assert(!tq_store(TQ_MAX_THREADS+1,QOS_CLASS_BACKGROUND,"Overflow"));
  assert(tq_lookup(TQ_MAX_THREADS+1,NULL)==QOS_CLASS_UNSPECIFIED);
  puts("PASS: real Darwin QoS, name rules, unmatched/ECO precedence, rename, capacity fails open");
  return NULL;
}
int main(void) {
  setenv("MADEIRA_THREAD_QOS","Worker*=utility;Gfx?=user-initiated",1);
  pthread_t thread;
  assert(!pthread_create(&thread,NULL,run,NULL));
  assert(!pthread_join(thread,NULL));
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-upstream-qos-') as work:
    path = Path(work)
    (path / 'check.c').write_text(code, encoding='utf-8')
    subprocess.run(['xcrun', 'clang', '-std=gnu11', '-O2', '-Wall', '-Wextra',
                    '-Werror', '-Wno-unused-function', '-Wno-unused-variable',
                    str(path / 'check.c'), '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=30)
