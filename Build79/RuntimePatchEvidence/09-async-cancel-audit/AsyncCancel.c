/* Private overlapped-pipe diagnostic. No game files or unrelated processes. */
#include <stdint.h>
#include <stddef.h>
typedef uint32_t DWORD; typedef int BOOL; typedef unsigned short WCHAR; typedef void *HANDLE;
typedef struct { uintptr_t internal,high; DWORD offset,offset_high; HANDLE event; } OVERLAPPED;
_Static_assert(sizeof(OVERLAPPED)==32 && offsetof(OVERLAPPED,event)==24,"OVERLAPPED ABI");
#define API __declspec(dllimport)
#define BAD ((HANDLE)(intptr_t)-1)
API DWORD __stdcall GetModuleFileNameW(HANDLE,WCHAR *,DWORD);
API DWORD __stdcall GetCurrentProcessId(void);
API DWORD __stdcall GetLastError(void);
API HANDLE __stdcall CreateNamedPipeW(const WCHAR *,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,void *);
API BOOL __stdcall ConnectNamedPipe(HANDLE,OVERLAPPED *);
API HANDLE __stdcall CreateFileW(const WCHAR *,DWORD,DWORD,void *,DWORD,DWORD,HANDLE);
API HANDLE __stdcall CreateEventW(void *,BOOL,BOOL,const WCHAR *);
API BOOL __stdcall ResetEvent(HANDLE);
API BOOL __stdcall ReadFile(HANDLE,void *,DWORD,DWORD *,OVERLAPPED *);
API BOOL __stdcall WriteFile(HANDLE,const void *,DWORD,DWORD *,OVERLAPPED *);
API BOOL __stdcall CancelIo(HANDLE);
API BOOL __stdcall CancelIoEx(HANDLE,OVERLAPPED *);
API BOOL __stdcall GetOverlappedResult(HANDLE,OVERLAPPED *,DWORD *,BOOL);
API DWORD __stdcall WaitForSingleObject(HANDLE,DWORD);
API BOOL __stdcall FlushFileBuffers(HANDLE);
API BOOL __stdcall CloseHandle(HANDLE);
API __declspec(noreturn) void __stdcall ExitProcess(DWORD);
static HANDLE output;
static WCHAR path[32768];
static OVERLAPPED ops[4],connect_op;
static unsigned char buffers[4];
static void text(const char *s) { DWORD n=0,w=0;while(s[n])n++;if(!WriteFile(output,s,n,&w,0)||w!=n)ExitProcess(90);FlushFileBuffers(output); }
static void number(DWORD v) { char s[]="0x00000000\n";for(unsigned i=0;i<8;i++)s[9-i]="0123456789abcdef"[(v>>(4*i))&15];text(s); }
static void fail(const char *s,DWORD error) {text("FAIL ");text(s);text(" error=");number(error);CloseHandle(output);ExitProcess(1);}
void mainCRTStartup(void) {
 DWORD n=GetModuleFileNameW(0,path,32768),base=0;
 if(!n||n>32000)ExitProcess(91);
 for(DWORD i=0;i<n;i++)if(path[i]=='\\'||path[i]=='/')base=i+1;
 if(!base)ExitProcess(92);
 const WCHAR name[]=u"async-cancel-test.log";
 for(unsigned i=0;i<sizeof(name)/2;i++)path[base+i]=name[i];
 output=CreateFileW(path,0x40000000,1,0,2,0x80,0);if(output==BAD)ExitProcess(93);
 text("AsyncCancel v1: 48 rounds, four overlapped pipe reads per round.\n");
 WCHAR pipe[]=u"\\\\.\\pipe\\MadeiraAsyncCancel-00000000";
 DWORD pid=GetCurrentProcessId();unsigned len=sizeof(pipe)/2-1;
 for(unsigned i=0;i<8;i++)pipe[len-1-i]=(WCHAR)"0123456789abcdef"[(pid>>(4*i))&15];
 HANDLE server=CreateNamedPipeW(pipe,0x40000003,0,1,4096,4096,0,0);if(server==BAD)fail("create pipe",GetLastError());
 connect_op.event=CreateEventW(0,1,0,0);if(!connect_op.event)fail("connect event",GetLastError());
 if(ConnectNamedPipe(server,&connect_op)||GetLastError()!=997)fail("pending connect",GetLastError());
 HANDLE client=CreateFileW(pipe,0xc0000000,0,0,3,0,0);if(client==BAD)fail("open client",GetLastError());
 DWORD transferred=0;
 if(WaitForSingleObject(connect_op.event,3000)!=0||!GetOverlappedResult(server,&connect_op,&transferred,0))fail("connect completion",GetLastError());
 CloseHandle(connect_op.event);
 for(unsigned i=0;i<4;i++){ops[i].event=CreateEventW(0,1,0,0);if(!ops[i].event)fail("read event",GetLastError());}
 for(unsigned round=0;round<48;round++) {
  text("BEGIN round ");number(round);
  for(unsigned i=0;i<4;i++){
   ops[i].internal=ops[i].high=0;ops[i].offset=ops[i].offset_high=0;ResetEvent(ops[i].event);
   if(ReadFile(server,&buffers[i],1,&transferred,&ops[i])||GetLastError()!=997)fail("read was not pending",GetLastError());
  }
  unsigned mode=round%3;
  if(mode==0) {if(!CancelIo(server))fail("CancelIo",GetLastError());}
  else if(mode==1) {if(!CancelIoEx(server,0))fail("CancelIoEx all",GetLastError());}
  else for(unsigned i=0;i<4;i++)if(!CancelIoEx(server,&ops[i]))fail("CancelIoEx one",GetLastError());
  for(unsigned i=0;i<4;i++){
   if(WaitForSingleObject(ops[i].event,3000)!=0)fail("cancel signal timeout",GetLastError());
   if(GetOverlappedResult(server,&ops[i],&transferred,0)||GetLastError()!=995)fail("expected OPERATION_ABORTED",GetLastError());
  }
  // The pipe remains usable after cancellation: byte transport must still work.
  unsigned char sent=(unsigned char)round;
  if(!WriteFile(client,&sent,1,&transferred,0)||transferred!=1)fail("write after cancel",GetLastError());
  ops[0].internal=ops[0].high=0;ResetEvent(ops[0].event);
  BOOL read=ReadFile(server,&buffers[0],1,&transferred,&ops[0]);
  if(!read&&GetLastError()!=997)fail("read after cancel",GetLastError());
  if(!read&&(WaitForSingleObject(ops[0].event,3000)!=0||!GetOverlappedResult(server,&ops[0],&transferred,0)))fail("read completion after cancel",GetLastError());
  if(transferred!=1||buffers[0]!=sent)fail("pipe data differs",transferred);
  text("PASS round ");number(round);
 }
 for(unsigned i=0;i<4;i++)CloseHandle(ops[i].event);
 CloseHandle(client);CloseHandle(server);
 text("PASS all 48 rounds: 192 cancellations and 48 subsequent byte transfers.\n");
 CloseHandle(output);ExitProcess(0);
}
