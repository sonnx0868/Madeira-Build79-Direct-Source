#!/usr/bin/env python3
"""Exercise the actual upstream builtin selection and its native override."""
from pathlib import Path
import os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
compiler=os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
pin=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD:wine'],text=True).strip()
with tempfile.TemporaryDirectory(prefix='madeira-builtin-pad-') as scratch:
    folder=Path(scratch);loader=folder/'dlls/ntdll/loader.c';loader.parent.mkdir(parents=True)
    loader.write_bytes(subprocess.check_output(['git','-C',str(root/'wine'),'show',pin+':dlls/ntdll/loader.c']))
    subprocess.run(['git','-C',str(folder),'apply',str(root/'patches/wine-builtin-xinput.patch')],check=True)
    source=loader.read_text(encoding='utf-8');start=source.index('static BOOL madeira_force_builtin_dll(')
    helper=source[start:source.index('\n/***********************************************************************',start)]
    fixture=r'''
#include <assert.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
typedef wchar_t WCHAR;
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#ifdef _WIN32
#define wcsicmp _wcsicmp
#else
#define wcsicmp wcscasecmp
#endif
typedef struct { WCHAR *Buffer; unsigned short Length,MaximumLength; } UNICODE_STRING;
static void RtlInitUnicodeString(UNICODE_STRING *u,const WCHAR *s) {u->Buffer=(WCHAR*)s;}
static int RtlQueryEnvironmentVariable_U(void *env,UNICODE_STRING *name,UNICODE_STRING *value) {
 (void)env;(void)name;const char *setting=getenv("MADEIRA_NATIVE_XINPUT");
 if(!setting||!*setting) return 1;
 value->Buffer[0]=setting[0];value->Length=sizeof(WCHAR);return 0;
}
'''+helper+r'''
int main(void) {
 const char *e=getenv("MADEIRA_NATIVE_XINPUT");int native=e&&e[0]=='1';
 assert(madeira_force_builtin_dll(L"xinput1_3.dll")==!native);
 assert(madeira_force_builtin_dll(L"XINPUT1_4.DLL")==!native);
 assert(madeira_force_builtin_dll(L"xinputuap.dll")==!native);
 assert(!madeira_force_builtin_dll(L"game.dll"));
 assert(!madeira_force_builtin_dll(L"C:\\game\\xinput1_3.dll"));
 puts("PASS: builtin XInput selection, case folding, explicit native/path choice and unrelated DLLs");
}
'''
    file=folder/'check.c';file.write_text(fixture,encoding='utf-8');exe=folder/'check.exe'
    subprocess.run([compiler,'-std=c11',str(file),'-o',str(exe)],check=True)
    for value in ('0','1'):
        env=dict(os.environ);env['MADEIRA_NATIVE_XINPUT']=value
        subprocess.run([str(exe)],env=env,check=True)
