#!/usr/bin/env python3
"""Run Wine's actual callback wrapper with the CrossRealms 621 bad pointer."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / "wine/dlls/user32/winproc.c").read_text(encoding="utf-8")
start = source.index("static LRESULT call_window_proc(")
end = source.index("/* call a 32-bit dialog procedure */", start)
wrapper = source[start:end]
assert "window-proc-validation-v1" in wrapper
fixture = r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef intptr_t LRESULT;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef uintptr_t ULONG_PTR;
typedef void *HWND;
typedef unsigned UINT;
typedef int LONG;
typedef LRESULT (*WNDPROC)(HWND,UINT,WPARAM,LPARAM);
static int calls, logs, error;
#define ERROR_INVALID_FUNCTION 1
#define TRACE_(channel) NO_TRACE
#define NO_TRACE(...) ((void)0)
#define ERR(...) (++logs)
static void SetLastError(int value) { error=value; }
static LONG InterlockedIncrement(LONG *value) { return ++*value; }
static LRESULT WINPROC_wrapper(WNDPROC proc,HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    return proc(hwnd,msg,wp,lp);
}
static LRESULT valid(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    ++calls; assert(hwnd==(void *)0x40026); assert(msg==0x210);
    assert(wp==1); assert(lp==0x10034); return 123;
}
'''
fixture += wrapper
fixture += r'''
int main(void) {
    uintptr_t bad[] = {0,1,0x3a,0xffff,0xffff0000,0xffff0001,0xffffffff};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);++i) {
        LRESULT result=456; error=0;
        assert(call_window_proc((HWND)0x40026,0x210,1,0x10034,&result,(void *)bad[i])==0);
        assert(result==0 && error==ERROR_INVALID_FUNCTION && calls==0);
    }
    for(int i=0;i<40;++i) {
        LRESULT result=456;
        call_window_proc((HWND)0x40026,0x210,1,0x10034,&result,(void *)0x3a);
    }
    assert(logs==16 && calls==0);
    LRESULT result=0; error=0;
    assert(call_window_proc((HWND)0x40026,0x210,1,0x10034,&result,(void *)valid)==123);
    assert(result==123 && calls==1 && error==0);
    puts("PASS: real Wine callback preserves valid messages, rejects 0x3a and unresolved handles, bounds logs");
}
'''
compiler = os.environ.get("CC") or shutil.which("clang") or shutil.which("cc")
if not compiler:
    raise SystemExit("C compiler required for window procedure regression")
with tempfile.TemporaryDirectory(prefix="madeira-winproc-") as scratch:
    folder=Path(scratch); c=folder/"fixture.c"; exe=folder/("fixture.exe" if os.name=="nt" else "fixture")
    c.write_text(fixture,encoding="utf-8")
    subprocess.run([compiler,"-std=c11","-Werror","-O2",str(c),"-o",str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
