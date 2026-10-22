#!/usr/bin/env python3
"""Production deferred punch code against a real APFS file and failure path."""
from pathlib import Path
import subprocess
import sys
import tempfile

if sys.platform != 'darwin':
    raise SystemExit('SKIP: APFS F_PUNCHHOLE checks run in Mac CI')
root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
start = source.index('static unsigned long long ios_swap_punch_leaks;')
end = source.index('static int ios_swap_eligible(', start)
production = source[start:end]
code = r'''
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static pthread_mutex_t virtual_mutex=PTHREAD_MUTEX_INITIALIZER;
static int ios_swap_fd;
static unsigned freed;
static uint64_t free_offset,free_length;
static void ios_swap_free_add(uint64_t off,uint64_t len) {
  ++freed;free_offset=off;free_length=len;
}
''' + production + r'''
int main(int argc,char **argv) {
  assert(argc==2);
  int deferred=atoi(argv[1]);
  if(!deferred) setenv("MADEIRA_SWAP_PUNCH_DEFER","0",1);
  char name[]="/tmp/madeira-punch-XXXXXX",data[4096],readback[4096];
  ios_swap_fd=mkstemp(name);assert(ios_swap_fd>=0);unlink(name);
  assert(!ftruncate(ios_swap_fd,2*1024*1024));
  memset(data,0x5a,sizeof(data));assert(pwrite(ios_swap_fd,data,sizeof(data),0)==sizeof(data));
  pthread_mutex_lock(&virtual_mutex);
  ios_swap_give(0,1024*1024);
  // A deferred range cannot be handed out while its completion is pending.
  if(deferred) assert(freed==0);
  pthread_mutex_unlock(&virtual_mutex);
  for(unsigned i=0;i<2000;++i) {
    pthread_mutex_lock(&virtual_mutex);int done=freed!=0;pthread_mutex_unlock(&virtual_mutex);
    if(done) break;
    usleep(1000);
  }
  pthread_mutex_lock(&virtual_mutex);
  assert(freed==1 && free_offset==0 && free_length==1024*1024);
  assert(pread(ios_swap_fd,readback,sizeof(readback),0)==sizeof(readback));
  for(unsigned i=0;i<sizeof(readback);++i) assert(!readback[i]);
  close(ios_swap_fd);ios_swap_fd=open("/dev/null",O_RDWR);assert(ios_swap_fd>=0);
  ios_swap_give(1024*1024,1024*1024);
  pthread_mutex_unlock(&virtual_mutex);
  for(unsigned i=0;i<2000;++i) {
    pthread_mutex_lock(&virtual_mutex);int done=ios_swap_punch_leaks!=0;pthread_mutex_unlock(&virtual_mutex);
    if(done) break;
    usleep(1000);
  }
  pthread_mutex_lock(&virtual_mutex);
  assert(freed==1 && ios_swap_punch_leaks==1);
  pthread_mutex_unlock(&virtual_mutex);
  printf("PASS: %s punch, real zero readback, no reuse before completion, failed punch quarantined\n",
         deferred?"deferred":"synchronous");
  return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-upstream-swap-') as work:
    path = Path(work)
    (path / 'check.c').write_text(code, encoding='utf-8')
    subprocess.run(['xcrun', 'clang', '-std=gnu11', '-O1', '-g', '-Wall', '-Wextra',
                    '-Werror', '-Wno-unused-parameter', '-fsanitize=address,undefined',
                    str(path / 'check.c'), '-o', str(path / 'check')], check=True)
    for mode in ('0', '1'):
        subprocess.run([str(path / 'check'), mode], check=True, timeout=30)
