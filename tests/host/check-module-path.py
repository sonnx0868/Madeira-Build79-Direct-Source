#!/usr/bin/env python3
"""Exercise production DLL recovery, including filesystem/loader refusal paths."""
from pathlib import Path
import os, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
loader = (root / 'wine/dlls/ntdll/loader.c').read_text(encoding='utf-8')
begin = loader.index('static BOOL module_path_ends_with(')
end = loader.index('static NTSTATUS find_dll_file(', begin)
production = loader[begin:end]
assert 'status == STATUS_DLL_NOT_FOUND && !find_loaded && nt_name->Buffer' in loader[end:]
fixture = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
typedef wchar_t WCHAR;
typedef size_t SIZE_T;
typedef int BOOL, NTSTATUS;
typedef void *HANDLE;
typedef struct { unsigned short Length, MaximumLength; WCHAR *Buffer; } UNICODE_STRING;
typedef struct { UNICODE_STRING *ObjectName; } OBJECT_ATTRIBUTES;
typedef struct { unsigned FileAttributes; } FILE_BASIC_INFORMATION;
typedef struct { int dummy; } SECTION_IMAGE_INFORMATION;
struct file_id { int dummy; };
typedef struct { int system; struct { unsigned Flags; } ldr; } WINE_MODREF;
#define TRUE 1
#define FALSE 0
#define STATUS_DLL_NOT_FOUND 1
#define STATUS_OBJECT_NAME_NOT_FOUND 2
#define STATUS_OBJECT_PATH_NOT_FOUND 3
#define STATUS_NOT_A_DIRECTORY 4
#define STATUS_NO_MEMORY 5
#define STATUS_ACCESS_DENIED 6
#define STATUS_INVALID_IMAGE_FORMAT 7
#define FILE_ATTRIBUTE_DIRECTORY 16
#define LDR_WINE_INTERNAL 0x80000000u
#define OBJ_CASE_INSENSITIVE 0
#define GetProcessHeap() NULL
#define InitializeObjectAttributes(a,n,f,r,s) ((a)->ObjectName=(n))
#define ERR(...) ((void)0)
static WCHAR source[1024], target[1024], opened[1024];
static WINE_MODREF owner;
static int enabled, owner_exists, source_exists, target_exists, source_dir, target_dir;
static int original_status, image_status, opens, allocations, live, fail_at;
static void RtlInitUnicodeString(UNICODE_STRING *s, const WCHAR *t) {
    s->Buffer=(WCHAR *)t;s->Length=(unsigned short)(wcslen(t)*sizeof(WCHAR));s->MaximumLength=s->Length+sizeof(WCHAR);
}
static void *RtlAllocateHeap(void *heap,unsigned flags,SIZE_T n) {
    (void)heap;(void)flags;if(++allocations==fail_at)return NULL;
    void *p=malloc(n);if(p)++live;return p;
}
static void RtlFreeHeap(void *heap,unsigned flags,void *p) { (void)heap;(void)flags;if(p){--live;free(p);} }
static BOOL RtlCreateUnicodeString(UNICODE_STRING *s,const WCHAR *t) {
    WCHAR *p=RtlAllocateHeap(NULL,0,(wcslen(t)+1)*sizeof(WCHAR));
    if(!p)return FALSE;wcscpy(p,t);RtlInitUnicodeString(s,p);return TRUE;
}
static void RtlFreeUnicodeString(UNICODE_STRING *s) { RtlFreeHeap(NULL,0,s->Buffer);s->Buffer=NULL; }
static NTSTATUS RtlQueryEnvironmentVariable_U(void *env,UNICODE_STRING *key,UNICODE_STRING *value) {
    (void)env;assert(!wcscmp(key->Buffer,L"MADEIRA_FOLDER_COMPAT"));
    if(!enabled)return 1;value->Buffer[0]=enabled==1?'1':'0';value->Length=sizeof(WCHAR);return 0;
}
static NTSTATUS NtQueryAttributesFile(OBJECT_ATTRIBUTES *attr,FILE_BASIC_INFORMATION *info) {
    const WCHAR *p=attr->ObjectName->Buffer;
    info->FileAttributes=0;
    if(!wcscmp(p,source)){info->FileAttributes=source_dir?FILE_ATTRIBUTE_DIRECTORY:0;return source_exists?0:STATUS_OBJECT_NAME_NOT_FOUND;}
    if(!wcscmp(p,target)){info->FileAttributes=target_dir?FILE_ATTRIBUTE_DIRECTORY:0;return target_exists?0:STATUS_OBJECT_NAME_NOT_FOUND;}
    return original_status;
}
static WINE_MODREF *find_fullname_module(UNICODE_STRING *name) {
    return owner_exists&&!wcscmp(name->Buffer,source)?&owner:NULL;
}
static NTSTATUS open_dll_file(UNICODE_STRING *name,WINE_MODREF **wm,HANDLE *mapping,SECTION_IMAGE_INFORMATION *info,struct file_id *id) {
    (void)wm;(void)mapping;(void)info;(void)id;++opens;wcscpy(opened,name->Buffer);return image_status;
}
PRODUCTION
static void reset(void) {
    enabled=owner_exists=source_exists=target_exists=1;
    owner=(WINE_MODREF){0};source_dir=target_dir=opens=image_status=allocations=fail_at=0;
    original_status=STATUS_NOT_A_DIRECTORY;opened[0]=0;
    wcscpy(source,L"\\??\\C:\\Games\\Folder Name\\plugin.dll");
    wcscpy(target,L"\\??\\C:\\Games\\Folder Name\\client.dll");
}
static void run(const WCHAR *path,int expected,int expected_opens) {
    int failure=fail_at;fail_at=0;
    UNICODE_STRING name;assert(RtlCreateUnicodeString(&name,path));allocations=0;fail_at=failure;
    WCHAR *before=name.Buffer;
    assert(open_module_sibling(&name,NULL,NULL,NULL,NULL)==expected);
    assert(opens==expected_opens);
    if(!expected){assert(name.Buffer!=before&&!wcscmp(name.Buffer,target));assert(!wcscmp(opened,target));}
    else assert(name.Buffer==before&&!wcscmp(name.Buffer,path));
    RtlFreeUnicodeString(&name);assert(live==0);
}
int main(void) {
    const WCHAR *bad=L"\\??\\C:\\Games\\Folder Name\\plugin.dl\\\\client.dll";
    reset();run(bad,0,1);
    reset();run(L"\\??\\C:\\Games\\Folder Name\\plugin.dll\\client.dll",0,1);
    reset();wcscpy(source,L"\\??\\C:\\Games\\Folder Name\\runner.exe");
    run(L"\\??\\C:\\Games\\Folder Name\\runner.ex\\client.dll",0,1);
    reset();enabled=0;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();enabled=2;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();original_status=0;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();original_status=STATUS_ACCESS_DENIED;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();original_status=STATUS_OBJECT_PATH_NOT_FOUND;run(bad,0,1);
    reset();original_status=STATUS_OBJECT_NAME_NOT_FOUND;run(bad,0,1);
    reset();owner_exists=0;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();owner.system=1;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();owner.ldr.Flags=LDR_WINE_INTERNAL;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();source_exists=0;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();target_exists=0;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();source_dir=1;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();target_dir=1;run(bad,STATUS_DLL_NOT_FOUND,0);
    reset();image_status=STATUS_INVALID_IMAGE_FORMAT;run(bad,STATUS_INVALID_IMAGE_FORMAT,1);
    reset();image_status=STATUS_ACCESS_DENIED;run(bad,STATUS_ACCESS_DENIED,1);
    for(int failure=1;failure<=2;failure++){reset();fail_at=failure;run(bad,STATUS_NO_MEMORY,0);}
    const WCHAR *refused[]={
        L"plugin.dl\\client.dll",L"\\??\\C:\\Games\\plugin.txt\\client.dll",
        L"\\??\\C:\\Games\\..\\plugin.dl\\client.dll",L"\\??\\C:\\Games\\.\\plugin.dl\\client.dll",
        L"\\??\\C:\\Games\\plugin.dl\\other\\client.dll",L"\\??\\C:\\Games\\plugin.dl\\client.exe",
        L"\\??\\C:\\Games\\plugin.dl\\client.dll:stream",L"\\??\\UNC\\server\\plugin.dl\\client.dll",
        L"\\??\\C:\\Games/plugin.dl\\client.dll",L"\\??\\C:\\Games\\plugin.dl\\.dll"};
    for(size_t i=0;i<sizeof(refused)/sizeof(refused[0]);i++){reset();run(refused[i],STATUS_DLL_NOT_FOUND,0);}
    WCHAR module[1024],sibling[1024];UNICODE_STRING name;
    RtlInitUnicodeString(&name,L"\\??\\c:\\Jeux\\\u65e5\\PLUGIN.DL\\CLIENT.DLL");
    assert(module_sibling_paths(&name,module,sibling));
    assert(!wcscmp(module,L"\\??\\c:\\Jeux\\\u65e5\\PLUGIN.DLl"));
    assert(!wcscmp(sibling,L"\\??\\c:\\Jeux\\\u65e5\\CLIENT.DLL"));
    puts("PASS: exact module/sibling DLL recovery, opt-in, existing paths, permissions, architecture and allocation failures");
}
'''.replace('PRODUCTION',production)
cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc: raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-module-path-') as scratch:
    folder=Path(scratch); source=folder/'loader.c'; source.write_text(fixture,encoding='utf-8')
    binary=folder/('loader.exe' if os.name=='nt' else 'loader')
    subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Werror',str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True,timeout=10)
