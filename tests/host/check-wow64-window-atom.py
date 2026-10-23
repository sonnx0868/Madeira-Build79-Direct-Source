#!/usr/bin/env python3
"""Run the real WoW64 window thunk with Chrome921's class atom and shifted windows."""
from pathlib import Path
import os, shutil, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
user=(root/'wine/dlls/wow64win/user.c').read_text(encoding='utf-8')
header=(root/'wine/dlls/wow64win/wow64win_private.h').read_text(encoding='utf-8')
start=user.index('NTSTATUS WINAPI wow64_NtUserCreateWindowEx(')
thunk=user[start:user.index('\nNTSTATUS WINAPI ',start+1)]
assert 'const WCHAR *class = get_str_or_atom( &args );' in thunk
start=header.index('static inline void *guest_ptr32(')
helpers=header[start:header.index('static inline void **addr_32to64(',start)]
start=header.index('static inline UNICODE_STRING *unicode_str_32to64(')
unicode=header[start:header.index('static inline SECURITY_DESCRIPTOR *',start)]

fixture=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#define WINAPI
typedef uint32_t UINT,ULONG,DWORD;
typedef uintptr_t ULONG_PTR,NTSTATUS;
typedef uint16_t WCHAR;
typedef int BOOL;
typedef void *HANDLE,*HWND,*HMENU,*HINSTANCE;
typedef struct {uint16_t Length,MaximumLength;uint32_t Buffer;} UNICODE_STRING32;
typedef struct {uint16_t Length,MaximumLength;WCHAR *Buffer;} UNICODE_STRING;
#define LongToHandle(v) ((HANDLE)(intptr_t)(int32_t)(v))
#define HandleToUlong(v) ((ULONG)(uintptr_t)(v))
static ULONG_PTR guest_base;
static uint32_t expected_class;
static unsigned calls;
static ULONG_PTR wow64win_guest_base(void) { return guest_base; }
'''+helpers+unicode+r'''
static HWND NtUserCreateWindowEx(DWORD ex,UNICODE_STRING *name,UNICODE_STRING *ver,
   UNICODE_STRING *title,DWORD style,int x,int y,int w,int h,HWND parent,HMENU menu,
   HINSTANCE inst,void *param,DWORD flags,HINSTANCE client,const WCHAR *class,BOOL ansi) {
    ++calls;
    assert(ex==0x10 && !name && !ver && !title && style==0x80000000);
    assert(x==-10 && y==20 && w==640 && h==480);
    assert(parent==(void*)(intptr_t)-3 && menu==(void*)(uintptr_t)0x1234);
    assert(inst==(void*)(guest_base+0x400000) && param==(void*)(guest_base+0x123456));
    assert(flags==7 && client==(void*)(guest_base+0x500000) && ansi==1);
    // NtUserCreateWindowEx's debug printer and class lookup distinguish an
    // atom from a string with HIWORD. The log's B+0xc021 would take the
    // string path and read a PROT_NONE page; the original atom must survive.
    if(expected_class && expected_class<=0xffff) {
        assert((uintptr_t)class==expected_class && !((uintptr_t)class>>16));
    } else {
        assert(class==(expected_class?(void*)(guest_base+expected_class):NULL));
    }
    return (void*)(uintptr_t)0x30042;
}
'''+thunk+r'''
int main(void) {
    ULONG_PTR bases[]={0,0x7100000000ull,0x7200000000ull,0x7400000000ull};
    uint32_t values[]={0,1,0xbfff,0xc000,0xc021,0xffff,0x10000,0x123456,0x80000000,0xffffffff};
    for(unsigned b=0;b<4;b++) {
        guest_base=bases[b];
        for(unsigned v=0;v<sizeof(values)/sizeof(*values);v++) {
            expected_class=values[v];
            UINT args[]={0x10,0,0,0,0x80000000,(uint32_t)-10,20,640,480,
                         (uint32_t)-3,0x1234,0x400000,0x123456,7,0x500000,
                         expected_class,1,0xaabbccdd};
            assert(wow64_NtUserCreateWindowEx(args)==0x30042);
            assert(args[17]==0xaabbccdd);
        }
    }
    assert(calls==40);
    // Normal text and other pointer parameters still use the generic helper.
    UINT ptr[]={0xc021};UINT *arg=ptr;guest_base=0x7200000000ull;
    assert(get_ptr(&arg)==(void*)0x720000c021ull);
    UNICODE_STRING32 in={4,8,0x123456};UNICODE_STRING out;
    assert(unicode_str_32to64(&out,&in)==&out && out.Length==4 && out.MaximumLength==8);
    assert(out.Buffer==(void*)0x7200123456ull);
    puts("PASS: production WoW64 CreateWindowEx preserves Chrome0xc021, atom boundaries, real pointers, handles, signed coordinates and classic/shifted guest windows");
}
'''
compiler=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler: raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-wow-window-atom-') as scratch:
    work=Path(scratch);(work/'check.c').write_text(fixture,encoding='utf-8')
    exe=work/('check.exe' if os.name=='nt' else 'check')
    subprocess.run([compiler,'-std=c11','-O2','-Werror',str(work/'check.c'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('NOT MEASURED: physical Chrome32 startup or cause of the later chrome.dll breakpoint')
