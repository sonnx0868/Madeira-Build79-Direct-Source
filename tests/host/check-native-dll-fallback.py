#!/usr/bin/env python3
"""Exercise the production PE loader fallback for native ARM64 and ARM64EC."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
loader = (root / "wine/dlls/ntdll/loader.c").read_text(encoding="utf-8")
start = loader.index("#if defined(__arm64ec__) || defined(__aarch64__) || defined(_M_ARM64)")
end = loader.index("\n    RtlFreeHeap( GetProcessHeap(), 0, fullname );", start)
policy = loader[start:end]
fixture = r'''
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
typedef wchar_t WCHAR;
typedef struct { WCHAR *Buffer; } UNICODE_STRING;
#define MAX_PATH 260
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define STATUS_SUCCESS 0
#define STATUS_INVALID_IMAGE_FORMAT 1
#define STATUS_DLL_NOT_FOUND 2
#define STATUS_NO_MEMORY 3
#define ERR(...) ((void)0)
#ifdef TEST_EC
#define __arm64ec__ 1
static const WCHAR expected[] = L"\\??\\C:\\windows\\sysx64\\kernel32.dll";
#else
#define __aarch64__ 1
static const WCHAR expected[] = L"\\??\\C:\\windows\\sysaa64\\kernel32.dll";
#endif
static int opens, open_ok = 1;
static WCHAR opened[MAX_PATH];
static void RtlInitUnicodeString(UNICODE_STRING *out,const WCHAR *text) { out->Buffer=(WCHAR *)text; }
static int open_dll_file(UNICODE_STRING *path,void *wm,void *mapping,void *info,void *id) {
    (void)wm;(void)mapping;(void)info;(void)id;
    ++opens; wcscpy(opened,path->Buffer); return !open_ok;
}
static void RtlFreeUnicodeString(UNICODE_STRING *path) { free(path->Buffer); path->Buffer=NULL; }
static int RtlCreateUnicodeString(UNICODE_STRING *out,const WCHAR *text) {
    out->Buffer=malloc((wcslen(text)+1)*sizeof(WCHAR));
    if(out->Buffer) wcscpy(out->Buffer,text);
    return out->Buffer != NULL;
}
static int run(int status,const WCHAR *libname) {
    UNICODE_STRING name; UNICODE_STRING *nt_name=&name;
    void *pwm=NULL,*mapping=NULL,*image_info=NULL,*id=NULL;
    assert(RtlCreateUnicodeString(nt_name,L"original"));
    POLICY
    if(status==STATUS_SUCCESS && opens) assert(!wcscmp(nt_name->Buffer,expected));
    RtlFreeUnicodeString(nt_name);
    return status;
}
int main(void) {
    assert(run(STATUS_SUCCESS,L"kernel32.dll")==STATUS_SUCCESS && opens==0);
    assert(run(STATUS_INVALID_IMAGE_FORMAT,L"kernel32.dll")==STATUS_SUCCESS);
    assert(!wcscmp(opened,expected));
    opens=0;
    assert(run(STATUS_DLL_NOT_FOUND,L"C:\\windows\\system32\\kernel32")==STATUS_SUCCESS);
    assert(opens==1 && !wcscmp(opened,expected));
    open_ok=0; opens=0;
    assert(run(STATUS_INVALID_IMAGE_FORMAT,L"kernel32.dll")==STATUS_INVALID_IMAGE_FORMAT && opens==1);
    puts("PASS: canonical native/EC DLL-farm fallback, success preservation and failed retry");
}
'''.replace("POLICY", policy)
cc = os.environ.get("CC") or shutil.which("cc")
if not cc:
    raise SystemExit("C compiler required")
with tempfile.TemporaryDirectory(prefix="madeira-native-dll-") as scratch:
    folder = Path(scratch)
    source = folder / "loader.c"
    source.write_text(fixture,encoding="utf-8")
    for mode in ["native", "ec"]:
        binary = folder / (mode + (".exe" if os.name == "nt" else ""))
        flags = ["-std=c11","-Wall","-Wextra","-Werror"]
        if mode == "ec": flags += ["-DTEST_EC"]
        if os.name == "nt": flags += ["-static"]
        subprocess.run([cc,*flags,str(source),"-o",str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=10)
