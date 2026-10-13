#!/usr/bin/env python3
"""Exercise Wine's production lookups when a shared client slot is recycled."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'wine/dlls/win32u/window.c').read_text(encoding='utf-8')

def function(signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

records = source[source.index('struct client_user_object'):source.index('#define SWP_AGG_NOGEOMETRYCHANGE')]
assert 'user-object-generation-v1' in records
assert 'publish_client_user_object( index, handle, type, ptr );' in function('HANDLE alloc_user_handle(')
assert 'user_lock();' in function('HANDLE alloc_user_handle(')
assert 'set_user_handle_ptr( win->handle, NULL );' in function('static void free_window_handle(')
fixture = r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef void *HANDLE;
typedef unsigned UINT, DWORD;
typedef unsigned short WORD;
typedef int BOOL;
#define MAX_USER_HANDLES 8
#define NTUSER_OBJ_WINDOW 1
#define OBJ_OTHER_PROCESS ((void *)1)
#define USER_HANDLE_TO_INDEX(h) (((uintptr_t)(h) & 0xffff) >> 1)
#define ERR(...) (++logs)
static unsigned pid=0x20, logs, locks;
static unsigned GetCurrentProcessId(void) { return pid; }
static void user_lock(void) { ++locks; }
static void user_unlock(void) { assert(locks); --locks; }
struct user_entry { unsigned pid, tid, type; HANDLE handle; };
static struct user_entry server[MAX_USER_HANDLES];
static BOOL get_user_entry(HANDLE h, unsigned short type, struct user_entry *e, HANDLE *full) {
    unsigned i=USER_HANDLE_TO_INDEX(h); uintptr_t generation=(uintptr_t)h>>16;
    if (i>=MAX_USER_HANDLES || !server[i].handle || server[i].type!=type) return 0;
    if (generation && generation!=0xffff && h!=server[i].handle) return 0;
    *e=server[i]; *full=e->handle; return 1;
}
static BOOL get_user_entry_at(WORD i, unsigned short type, struct user_entry *e, HANDLE *full) {
    return get_user_entry((HANDLE)(uintptr_t)(i*2),type,e,full);
}
'''
fixture += records + '\n' + function('void *get_user_handle_ptr(')
fixture += '\n' + function('void *next_thread_user_object(')
fixture += '\n' + function('static void set_user_handle_ptr(')
fixture += r'''
int main(void) {
    HANDLE old=(HANDLE)0x30002, current=(HANDLE)0x40002, next=(HANDLE)0x10004;
    server[1]=(struct user_entry){0x20,0x24,1,old};
    /* Deliberately invalid pointer: a stale lookup must never dereference it. */
    user_lock(); publish_client_user_object(1,old,1,(void *)0x5555); user_unlock();
    assert(get_user_handle_ptr(old,1)==(void *)0x5555); user_unlock();
    server[1].handle=current;
    assert(get_user_handle_ptr(current,1)==OBJ_OTHER_PROCESS && !locks);
    assert(get_user_handle_ptr((HANDLE)2,1)==OBJ_OTHER_PROCESS && !locks);
    assert(get_user_handle_ptr(old,1)==NULL && !locks);
    server[2]=(struct user_entry){0x20,0x24,1,next};
    user_lock(); publish_client_user_object(2,next,1,(void *)0x8888);
    HANDLE iter=NULL;
    assert(next_thread_user_object(0x24,&iter,1)==(void *)0x8888 && iter==next);
    assert(next_thread_user_object(0x24,&iter,1)==NULL);
    set_user_handle_ptr(current,(void *)0x9999);
    set_user_handle_ptr(old,NULL); /* delayed cleanup cannot clear a new generation */
    user_unlock();
    assert(get_user_handle_ptr(current,1)==(void *)0x9999); user_unlock();
    assert(get_user_handle_ptr((HANDLE)0xffff0002,1)==(void *)0x9999); user_unlock();
    pid=0x30;
    assert(get_user_handle_ptr(current,1)==OBJ_OTHER_PROCESS && !locks);
    server[1].pid=0x30; /* even PID ownership changes without a generation change */
    assert(get_user_handle_ptr(current,1)==OBJ_OTHER_PROCESS && !locks);
    server[1].type=2;
    assert(get_user_handle_ptr(current,2)==NULL && !locks);
    pid=0x20; server[1].pid=0x20; server[1].type=1;
    user_lock(); set_user_handle_ptr(current,NULL); user_unlock();
    assert(get_user_handle_ptr(current,1)==NULL && !locks);
    assert(logs==5);
    user_lock(); publish_client_user_object(1,old,1,(void *)0x5555); user_unlock();
    for(unsigned i=0;i<100;++i) assert(get_user_handle_ptr(current,1)==OBJ_OTHER_PROCESS && !locks);
    assert(logs==32);
    puts("PASS: production user object lookup rejects stale generations/owners/types, preserves valid and truncated handles, skips stale cleanup, bounds logs");
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required for user object generation regression')
with tempfile.TemporaryDirectory(prefix='madeira-user-generation-') as scratch:
    folder=Path(scratch); c=folder/'fixture.c'; exe=folder/('fixture.exe' if os.name=='nt' else 'fixture')
    c.write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=c11','-Werror','-O2',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
