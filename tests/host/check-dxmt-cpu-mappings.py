#!/usr/bin/env python3
"""Exercise the production cache policy across allocation flags and racing callers."""
from pathlib import Path
import importlib.util, os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('patches',root/'tests/host/check-dxmt-patches.py')
patches=importlib.util.module_from_spec(spec); spec.loader.exec_module(patches)
source=patches.patched_sources(root)['src/winemetal/unix/winemetal_unix.c']
start=source.index('static MTLResourceOptions\nmadeira_cpu_cache_mode')
helper=source[start:source.index('\nstatic NTSTATUS',start)]
compiler=os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
fixture=r'''
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#ifdef _WIN32
#define dprintf(fd, ...) fprintf(stderr, __VA_ARGS__)
#endif
typedef uint64_t MTLResourceOptions;
#define MTLResourceCPUCacheModeMask 15ull
'''+helper+r'''
static void *worker(void *unused) {
  (void)unused;
  for (unsigned i=0;i<100000;i++) {
    uint64_t flags=0xdeadbeef8000ull | (i&15);
    uint64_t expected=flags;
#if TARGET_OS_IOS
    if (!getenv("MADEIRA_WRITE_COMBINED") || getenv("MADEIRA_WRITE_COMBINED")[0]!='1')
      expected &= ~MTLResourceCPUCacheModeMask;
#endif
    assert(madeira_cpu_cache_mode(flags)==expected);
  }
  return NULL;
}
int main(void) {
  pthread_t threads[8];
  for(unsigned i=0;i<8;i++) assert(!pthread_create(threads+i,NULL,worker,NULL));
  for(unsigned i=0;i<8;i++) assert(!pthread_join(threads[i],NULL));
  return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-cpu-mappings-') as scratch:
    folder=Path(scratch); c=folder/'test.c'; c.write_text(fixture,encoding='utf-8')
    env=dict(os.environ); env['PATH']=str(Path(compiler).parent)+os.pathsep+env.get('PATH','')
    for ios in (0,1):
        binary=folder/('test'+str(ios)+('.exe' if os.name=='nt' else ''))
        subprocess.run([compiler,'-std=c11','-O2','-pthread','-DTARGET_OS_IOS='+str(ios),str(c),'-o',str(binary)],check=True)
        for setting in (None,'0','1'):
            runenv=dict(env); runenv.pop('MADEIRA_WRITE_COMBINED',None)
            if setting is not None: runenv['MADEIRA_WRITE_COMBINED']=setting
            subprocess.run([str(binary)],env=runenv,check=True)
print('PASS: production CPU cache policy, storage/hazard flag preservation, opt-out, macOS and concurrent callers')
