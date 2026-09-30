/* Read-only guest diagnostics: no registry, DLL, prefix, or game changes.
 * Only creates a NEW report beside the checker; never replaces an existing file.
 * x64 consumer deliberately tests the same loader path as x64 games under FEX.
 */
#include <stdint.h>
#include <stddef.h>
typedef uint32_t DWORD;
typedef int32_t BOOL;
typedef uint16_t WCHAR;
typedef void *HANDLE;
#define IMPORT __declspec(dllimport)
IMPORT HANDLE CreateFileW(const WCHAR *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMPORT BOOL ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMPORT BOOL WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMPORT BOOL CloseHandle(HANDLE);
IMPORT BOOL SetFilePointerEx(HANDLE, int64_t, int64_t *, DWORD);
IMPORT HANDLE LoadLibraryW(const WCHAR *);
IMPORT BOOL FreeLibrary(HANDLE);
IMPORT void *GetProcAddress(HANDLE, const char *);
IMPORT DWORD GetModuleFileNameW(HANDLE, WCHAR *, DWORD);
IMPORT DWORD GetSystemDirectoryW(WCHAR *, DWORD);
IMPORT DWORD GetLastError(void);
IMPORT HANDLE GetStdHandle(DWORD);
IMPORT int WideCharToMultiByte(unsigned, DWORD, const WCHAR *, int, char *, int, const char *, BOOL *);
IMPORT void ExitProcess(DWORD);
typedef struct {
    uint16_t architecture, reserved;
    DWORD page_size;
    void *minimum, *maximum;
    uintptr_t mask;
    DWORD processors, processor_type, granularity;
    uint16_t level, revision;
} SYSTEM_INFO;
_Static_assert(sizeof(SYSTEM_INFO) == 48, "Windows x64 SYSTEM_INFO ABI");
IMPORT void GetNativeSystemInfo(SYSTEM_INFO *);
IMPORT void GetSystemInfo(SYSTEM_INFO *);
IMPORT int32_t RegOpenKeyExW(HANDLE, const WCHAR *, DWORD, DWORD, HANDLE *);
IMPORT int32_t RegQueryValueExW(HANDLE, const WCHAR *, DWORD *, DWORD *, unsigned char *, DWORD *);
IMPORT int32_t RegCloseKey(HANDLE);

static HANDLE report;
static char buffer[4096];
static unsigned used;
static WCHAR path[1024];
static char utf8[4096];
void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *a=dst; const unsigned char *b=src;
    for (size_t i=0;i<n;i++) a[i]=b[i];
    return dst;
}
static void add(const char *s) { while (*s && used<sizeof buffer-2) buffer[used++]=*s++; }
static void number(uint32_t n) {
    char digits[16]; unsigned count=0;
    do { digits[count++]=(char)('0'+n%10); n/=10; } while(n);
    while(count && used<sizeof buffer-2) buffer[used++]=digits[--count];
}
static void hex(uint32_t n) {
    const char *digits="0123456789abcdef"; add("0x");
    for (int i=28;i>=0;i-=4) { char s[2]={digits[(n>>i)&15],0}; add(s); }
}
static void line(void) {
    DWORD written; add("\r\n");
    if (report && report!=(HANDLE)(intptr_t)-1) WriteFile(report,buffer,used,&written,0);
    WriteFile(GetStdHandle((DWORD)-11),buffer,used,&written,0); used=0;
}
static void wide(const WCHAR *s) {
    unsigned count=0; while(s[count] && count<1023) count++;
    int bytes=WideCharToMultiByte(65001,0,s,(int)count,utf8,sizeof utf8-1,0,0);
    if (bytes>0) { utf8[bytes]=0; add(utf8); }
}
static unsigned copywide(WCHAR *dst, const WCHAR *src, unsigned limit) {
    unsigned n=0; while(src[n] && n+1<limit) {dst[n]=src[n];n++;}
    dst[n]=0; return n;
}
static void file_machine(const WCHAR *filename) {
    HANDLE file=CreateFileW(filename,0x80000000u,7,0,3,0x80,0);
    if (file==(HANDLE)(intptr_t)-1) {add(" PE read error=");number(GetLastError());return;}
    unsigned char header[256]; DWORD count=0;
    if (!ReadFile(file,header,sizeof header,&count,0) || count<64 || header[0]!='M' || header[1]!='Z') {
        add(" invalid DOS header");CloseHandle(file);return;
    }
    const char *marker="Wine builtin DLL";
    BOOL builtin=0;
    for (DWORD i=64;i+16<=count;i++) {
        unsigned j=0;while(j<16 && header[i+j]==(unsigned char)marker[j]) j++;
        if (j==16) {builtin=1;break;}
    }
    add(builtin?" provider=Wine-builtin":" provider=unidentified (not vendor-authenticated)");
    DWORD offset=(DWORD)header[60]|((DWORD)header[61]<<8)|((DWORD)header[62]<<16)|((DWORD)header[63]<<24);
    if (offset<64 || offset>16*1024*1024 || !SetFilePointerEx(file,offset,0,0) ||
        !ReadFile(file,header,6,&count,0) || count!=6 || header[0]!='P' || header[1]!='E' || header[2] || header[3]) {
        add(" invalid PE header");CloseHandle(file);return;
    }
    add(" PE-machine=");hex((DWORD)header[4]|((DWORD)header[5]<<8));CloseHandle(file);
}
static void registration(const WCHAR *architecture, DWORD view) {
    WCHAR key[256]; unsigned n=copywide(key,(const WCHAR *)L"Software\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\",256);
    copywide(key+n,architecture,256-n);
    HANDLE handle=0;
    int32_t result=RegOpenKeyExW((HANDLE)(intptr_t)(int32_t)0x80000002u,key,0,0x20019u|view,&handle);
    add("VC14 registry arch=");wide(architecture);add(" view=");number(view==0x100?64:32);
    add(" open-status=");number((DWORD)result);
    if (!result) {
        DWORD value=0,type=0,size=sizeof value;
        result=RegQueryValueExW(handle,(const WCHAR *)L"Installed",0,&type,(unsigned char *)&value,&size);
        add(" Installed-query=");number((DWORD)result);
        if (!result && type==4 && size==4) {add(" Installed=");number(value);}
        WCHAR version[128]={0}; size=sizeof version;
        result=RegQueryValueExW(handle,(const WCHAR *)L"Version",0,&type,(unsigned char *)version,&size);
        if (!result && (type==1 || type==2)) {version[127]=0;add(" Version=");wide(version);}
        RegCloseKey(handle);
    }
    line();
}
static void probe(const WCHAR *name, const char *symbol) {
    unsigned n=GetSystemDirectoryW(path,1024);
    if (!n || n>=1022) {add("GetSystemDirectory failed");line();return;}
    path[n++]='\\';copywide(path+n,name,1024-n);
    add("x64 loader probe ");wide(name);
    HANDLE module=LoadLibraryW(path);
    if (!module) {add(" LOAD FAILED error=");number(GetLastError());line();return;}
    add(" loaded");
    if (!GetProcAddress(module,symbol)) {add(" missing export ");add(symbol);add(" error=");number(GetLastError());}
    else {add(" export-present=");add(symbol);}
    line();
    DWORD length=GetModuleFileNameW(module,path,1024);
    if (length && length<1024) {add(" resolved ");wide(path);file_machine(path);line();}
    FreeLibrary(module);
}
void mainCRTStartup(void) {
    DWORD length=GetModuleFileNameW(0,path,1024);
    if (!length || length>=1024) ExitProcess(2);
    unsigned folder=length;while(folder && path[folder-1]!='\\' && path[folder-1]!='/') folder--;
    if (!folder || folder+64>=1024) ExitProcess(2);
    for (unsigned attempt=1;attempt<=99;attempt++) {
        unsigned n=folder;
        n+=copywide(path+n,(const WCHAR *)L"Madeira-Dependency-Report-",1024-n);
        path[n++]=(WCHAR)('0'+attempt/10);path[n++]=(WCHAR)('0'+attempt%10);
        copywide(path+n,(const WCHAR *)L".txt",1024-n);
        report=CreateFileW(path,0x40000000u,3,0,1,0x80,0); /* CREATE_NEW */
        if (report!=(HANDLE)(intptr_t)-1 || GetLastError()!=80) break;
    }
    if (report==(HANDLE)(intptr_t)-1) ExitProcess(3);
    add("Madeira dependency checker v1 (x64). No installation or registry/DLL edits.");line();
    add("Report path: ");wide(path);line();
    add("Machines: x86=0x14c, x64=0x8664, ARM64=0xaa64, ARM64EC=0xa641, ARM64X=0xa64e.");line();
    SYSTEM_INFO info={0};GetSystemInfo(&info);add("Process system architecture=");number(info.architecture);line();
    GetNativeSystemInfo(&info);add("Native system architecture=");number(info.architecture);line();
    add("System architecture IDs: x86=0, x64=9, ARM64=12. Native ARM64 does NOT establish game architecture.");line();
    for (unsigned view=0;view<2;view++) {
        DWORD flag=view?0x200:0x100;
        registration((const WCHAR *)L"x64",flag);
        registration((const WCHAR *)L"x86",flag);
        registration((const WCHAR *)L"arm64",flag);
    }
    probe((const WCHAR *)L"vcruntime140.dll","_CxxThrowException");
    probe((const WCHAR *)L"vcruntime140_1.dll","__CxxFrameHandler4");
    probe((const WCHAR *)L"msvcp140.dll","?_Xbad_alloc@std@@YAXXZ");
    probe((const WCHAR *)L"ucrtbase.dll","malloc");
    add("Export presence is not a full C++ execution test. This checks x64 loading only, NOT x86/ARM64 game compatibility.");line();
    add("Registry Installed=1 can describe Wine's builtin runtime; it does NOT prove a Microsoft installer was run.");line();
    add("Send this report, game name, launch EXE path, Madeira build, and madeira-log.txt. Do not fake Installed=1.");line();
    CloseHandle(report);ExitProcess(0);
}
