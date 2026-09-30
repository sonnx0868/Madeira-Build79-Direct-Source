/* Small private guest-process lifecycle test. Only terminates its own children. */
#include <stdint.h>
#include <stddef.h>
typedef unsigned short WCHAR;
typedef uint32_t DWORD;
typedef int BOOL;
typedef void *HANDLE;
typedef struct {
    DWORD cb; WCHAR *reserved,*desktop,*title;
    DWORD x,y,xsize,ysize,xchars,ychars,fill,flags;
    uint16_t show,reserved2_size; unsigned char *reserved2;
    HANDLE input,output,error;
} StartupInfo;
typedef struct { HANDLE process,thread; DWORD process_id,thread_id; } ProcessInfo;
_Static_assert(sizeof(StartupInfo)==104 && offsetof(StartupInfo,input)==80,"StartupInfo ABI");
_Static_assert(sizeof(ProcessInfo)==24,"ProcessInfo ABI");
#define API __declspec(dllimport)
API WCHAR * __stdcall GetCommandLineW(void);
API DWORD __stdcall GetModuleFileNameW(HANDLE,WCHAR *,DWORD);
API HANDLE __stdcall GetCurrentProcess(void);
API BOOL __stdcall TerminateProcess(HANDLE,DWORD);
API __declspec(noreturn) void __stdcall ExitProcess(DWORD);
API HANDLE __stdcall CreateFileW(const WCHAR *,DWORD,DWORD,void *,DWORD,DWORD,HANDLE);
API BOOL __stdcall WriteFile(HANDLE,const void *,DWORD,DWORD *,void *);
API BOOL __stdcall FlushFileBuffers(HANDLE);
API BOOL __stdcall CloseHandle(HANDLE);
API BOOL __stdcall CreateProcessW(const WCHAR *,WCHAR *,void *,void *,BOOL,DWORD,void *,const WCHAR *,StartupInfo *,ProcessInfo *);
API DWORD __stdcall WaitForSingleObject(HANDLE,DWORD);
API BOOL __stdcall GetExitCodeProcess(HANDLE,DWORD *);
static HANDLE output;
static WCHAR exe[32768], command[32768], log_path[32768];
static void text(const char *s) {
    DWORD count=0,n=0;
    while(s[count]) count++;
    if(!WriteFile(output,s,count,&n,0) || n!=count) ExitProcess(40);
    FlushFileBuffers(output);
}
static void code(DWORD v) {
    char s[12]="0x00000000\n";
    for(unsigned i=0;i<8;i++) s[9-i]="0123456789abcdef"[(v>>(4*i))&15];
    text(s);
}
void mainCRTStartup(void) {
    const WCHAR *arg=GetCommandLineW();
    if(*arg=='"') { arg++; while(*arg && *arg!='"') arg++; if(*arg) arg++; }
    else while(*arg && *arg!=' ') arg++;
    while(*arg==' ') arg++;
    if(arg[0]) {
        if(arg[1]) ExitProcess(41);
        if(arg[0]=='g') ExitProcess(0);
        DWORD status;
        if(arg[0]=='0') status=0;
        else if(arg[0]=='1') status=19;
        else if(arg[0]=='2') status=0xc0000005u;
        else ExitProcess(42);
        TerminateProcess(GetCurrentProcess(),status);
        ExitProcess(43); /* Abrupt self-termination must not return. */
    }
    DWORD n=GetModuleFileNameW(0,exe,32768),base=0;
    if(!n || n>=32000) ExitProcess(44);
    for(DWORD i=0;i<n;i++) {
        if(exe[i]=='"') ExitProcess(45);
        log_path[i]=exe[i];
        if(exe[i]=='\\' || exe[i]=='/') base=i+1;
    }
    if(!base) ExitProcess(46);
    const WCHAR name[]=u"guest-exit-test.log";
    for(DWORD i=0;i<sizeof(name)/sizeof(name[0]);i++) log_path[base+i]=name[i];
    output=CreateFileW(log_path,0x40000000,1,0,2,0x80,0);
    if(output==(HANDLE)(intptr_t)-1) ExitProcess(47);
    text("GuestExit v1: graceful and abrupt self-termination; four private children.\n");
    const WCHAR modes[]={'g','0','1','2'};
    const DWORD expected[]={0,0,19,0xc0000005u};
    for(unsigned test=0;test<4;test++) {
        text("BEGIN case ");code(test);
        command[0]='"';
        for(DWORD i=0;i<n;i++) command[i+1]=exe[i];
        command[n+1]='"';command[n+2]=' ';command[n+3]=modes[test];command[n+4]=0;
        StartupInfo si={0};ProcessInfo pi={0};si.cb=sizeof(si);
        if(!CreateProcessW(exe,command,0,0,0,0,0,0,&si,&pi)) { text("FAIL create child\n");ExitProcess(48); }
        CloseHandle(pi.thread);
        DWORD wait=WaitForSingleObject(pi.process,30000),actual=0;
        if(wait!=0) { text("FAIL child did not finish; left untouched. wait=");code(wait);CloseHandle(pi.process);ExitProcess(49); }
        if(!GetExitCodeProcess(pi.process,&actual)) { text("FAIL query exit code\n");CloseHandle(pi.process);ExitProcess(50); }
        CloseHandle(pi.process);
        text("Child exit=");code(actual);
        if(actual!=expected[test]) { text("FAIL exit code differs\n");ExitProcess(51); }
        text("PASS case ");code(test);
    }
    text("PASS all four cases; parent survived and exit codes matched.\n");
    CloseHandle(output);
    ExitProcess(0);
}
