#!/usr/bin/env python3
"""Reproduce the926 empty-pipe QS_DRIVER storm in the actual server callback.

Darwin/Linux use native pipe/poll; Windows uses a real anonymous pipe through
PeekNamedPipe as the poll adapter. This tests readiness semantics, not iPad FPS.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/wineserver/queue_ios.c').read_text(encoding='utf-8')
baseline = subprocess.check_output(['git', '-C', str(root), 'show',
    'd784eea:build/wineserver/queue_ios.c'], text=True)


def callback(text):
    start = text.index('static void msg_queue_poll_event( struct fd *fd, int event )\n{')
    end = text.index('\nstatic void thread_input_dump', start)
    return text[start:end]


fixture = r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#define pipe(p) _pipe(p,4096,_O_BINARY)
#define read _read
#define write _write
#define close _close
#define POLLIN 1
#define POLLERR 8
#define POLLHUP 16
#define POLLNVAL 32
struct pollfd {int fd;short events,revents;};
static int poll(struct pollfd *p,unsigned n,int timeout) {
    assert(n==1 && timeout==0);DWORD bytes=0;
    if(PeekNamedPipe((HANDLE)_get_osfhandle(p->fd),NULL,0,NULL,&bytes,NULL))
        p->revents=bytes ? POLLIN : 0;
    else p->revents=GetLastError()==ERROR_BROKEN_PIPE ? POLLHUP : POLLNVAL;
    return !!p->revents;
}
#else
#include <unistd.h>
#include <poll.h>
#endif
#define WINE_IOS 1
#define QS_DRIVER 0x80000000u
static int msg_queue_ops;
struct fd {int unix_fd,events;void *user;};
struct msg_queue {struct {void *ops;} obj;struct fd *fd;unsigned bits;};
static unsigned wakes;
static void *get_fd_user(struct fd *f) {return f->user;}
static int get_unix_fd(struct fd *f) {return f->unix_fd;}
static void set_fd_events(struct fd *f,int e) {f->events=e;}
static void set_queue_bits(struct msg_queue *q,unsigned bits) {q->bits|=bits;++wakes;}
/*PRODUCTION*/
static void probe(struct fd *f) {if(f->events>0)msg_queue_poll_event(f,POLLIN);}
static void acknowledge(struct msg_queue *q) {q->bits&=~QS_DRIVER;q->fd->events=POLLIN;}
int main(void) {
    int pair[2];assert(!pipe(pair));
    struct fd f={pair[0],POLLIN,NULL};
    struct msg_queue q={{&msg_queue_ops},&f,0};f.user=&q;
    // Reproduce the actual fd_ios.c fallback: synthetic POLLIN even though
    // the producer has not written input, followed by set_queue_mask re-arm.
    for(unsigned i=0;i<10000;++i) {
        probe(&f);
#ifdef BASELINE
        assert(q.bits==QS_DRIVER && f.events==0);
#else
        assert(!q.bits && f.events==POLLIN);
#endif
        acknowledge(&q);
    }
#ifdef BASELINE
    assert(wakes==10000);
    puts("REPRODUCED: production926 callback generates10000 false GUI wakes from an empty pipe");
#else
    assert(!wakes);
    // A notification stays readable until the owning event pump drains it.
    char byte=1,received=0;assert(write(pair[1],&byte,1)==1);
    probe(&f);assert(q.bits==QS_DRIVER && f.events==0 && wakes==1);
    struct pollfd p={pair[0],POLLIN,0};assert(poll(&p,1,0)==1 && (p.revents&POLLIN));
    assert(read(pair[0],&received,1)==1 && received==byte);
    acknowledge(&q);probe(&f);assert(!q.bits && wakes==1 && f.events==POLLIN);
    // A peer draining a notification must not make the stale probe wake us.
    assert(write(pair[1],&byte,1)==1);assert(read(pair[0],&received,1)==1);
    probe(&f);assert(!q.bits && wakes==1 && f.events==POLLIN);
    for(unsigned i=0;i<1000;++i) {
        assert(write(pair[1],&byte,1)==1);probe(&f);assert(q.bits==QS_DRIVER);
        assert(read(pair[0],&received,1)==1);acknowledge(&q);probe(&f);assert(!q.bits);
    }
    assert(wakes==1001);
    close(pair[1]);pair[1]=-1;probe(&f);
    assert(q.bits==QS_DRIVER && f.events==-1 && wakes==1002);
    puts("PASS: production callback rejects10000 empty probes, preserves actual input without consuming bytes, re-arms1000 times, tolerates peer drains and reports real pipe hangup");
#endif
    close(pair[0]);if(pair[1]>=0)close(pair[1]);return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-queue-ready-') as temp:
    work = Path(temp)
    for old in (True, False):
        c = work / ('baseline.c' if old else 'fixed.c')
        exe = c.with_suffix('.exe' if os.name == 'nt' else '')
        c.write_text(fixture.replace('/*PRODUCTION*/', callback(baseline if old else source)), encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        *([] if os.name == 'nt' else ['-fsanitize=address,undefined']),
                        '-Wno-unused-function', *(['-DBASELINE'] if old else []),
                        str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
print('NOT MEASURED: RenPy scene-specific freeze resolution on iPad')
