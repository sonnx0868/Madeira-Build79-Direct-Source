#!/usr/bin/env python3
"""Execute production builtin selection and timed sleeps, including Darwin waits."""
from pathlib import Path
import argparse, os, shutil, subprocess, sys, tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--require-darwin', action='store_true')
args = parser.parse_args()
cc = os.environ.get('CC') or shutil.which('clang')
assert cc, 'clang required'
if args.require_darwin and sys.platform != 'darwin':
    raise SystemExit('Darwin is required for native sleep/QoS verification')

with tempfile.TemporaryDirectory(prefix='madeira-runtime-waits-') as scratch:
    folder = Path(scratch)
    def run(name, source, flags=()):
        file=folder/(name+'.c');binary=folder/(name+'.exe')
        file.write_text(source, encoding='utf-8')
        subprocess.run([cc,'-std=c11','-Werror=implicit-function-declaration',*flags,str(file),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=20)

    loader=(root/'build/ntdll-unix/loader_ios.c').read_text(encoding='utf-8')
    start=loader.index('NTSTATUS load_builtin(')
    body=loader[start:loader.index('\n/********',start)]
    fixture=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
typedef int32_t NTSTATUS;
typedef uint16_t WCHAR,USHORT;
typedef void *HANDLE;
typedef uintptr_t ULONG_PTR,SIZE_T;
#ifndef _WIN32
#include <sys/types.h>
#else
typedef long off_t;
#endif
typedef struct { WCHAR *Buffer; USHORT Length,MaximumLength; } UNICODE_STRING;
typedef struct { int unused; } ANSI_STRING,SECTION_IMAGE_INFORMATION,IO_STATUS_BLOCK;
typedef struct { UNICODE_STRING *ObjectName; } OBJECT_ATTRIBUTES;
struct pe_image_info { int wine_builtin; };
enum loadorder { LO_DISABLED,LO_DEFAULT,LO_NATIVE,LO_NATIVE_BUILTIN,LO_BUILTIN,LO_BUILTIN_NATIVE };
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define IMAGE_FILE_MACHINE_I386 0x14c
#define OBJ_CASE_INSENSITIVE 1
#define FILE_READ_DATA 1
#define SYNCHRONIZE 2
#define FILE_SHARE_READ 4
#define FILE_SHARE_DELETE 8
#define FILE_SYNCHRONOUS_IO_NONALERT 16
#define FILE_NON_DIRECTORY_FILE 32
#define SECTION_ALL_ACCESS 64
#define PAGE_EXECUTE_READ 128
#define SEC_IMAGE 256
#define FALSE 0
#define STATUS_SUCCESS 0
#define STATUS_DLL_NOT_FOUND ((NTSTATUS)0xc0000135)
#define STATUS_NOT_SUPPORTED ((NTSTATUS)0xc00000bb)
#define STATUS_OBJECT_NAME_NOT_FOUND ((NTSTATUS)0xc0000034)
#define STATUS_OBJECT_PATH_NOT_FOUND ((NTSTATUS)0xc000003a)
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xc0000022)
#define STATUS_INVALID_IMAGE_FORMAT ((NTSTATUS)0xc000007b)
#define STATUS_IMAGE_ALREADY_LOADED ((NTSTATUS)0x4000000e)
#define TRACE(...) ((void)0)
#define InitializeObjectAttributes(a,n,f,r,s) ((a)->ObjectName=(n))
static enum loadorder order;
static int ec,wow,opens,sections,maps,closes;
static NTSTATUS open_status,section_status,map_status;
static WCHAR seen[512];static unsigned seen_length;
static enum loadorder get_load_order(const UNICODE_STRING *u){(void)u;return order;}
static int ios_is_arm64ec_cur(void){return ec;}
static void *NtCurrentTeb64(void){return wow?(void*)1:NULL;}
static const char *debugstr_us(const UNICODE_STRING *u){(void)u;return "fixture.dll";}
static NTSTATUS NtOpenFile(HANDLE *f,unsigned access,OBJECT_ATTRIBUTES *a,IO_STATUS_BLOCK *io,unsigned share,unsigned flags){
 (void)access;(void)io;(void)share;(void)flags;++opens;*f=(void*)1;
 seen_length=a->ObjectName->Length;memcpy(seen,a->ObjectName->Buffer,seen_length);return open_status;
}
static NTSTATUS NtCreateSection(HANDLE *m,unsigned a,void*b,void*c,unsigned d,unsigned e,HANDLE f){
 (void)a;(void)b;(void)c;(void)d;(void)e;assert(f==(void*)1);++sections;*m=(void*)2;return section_status;
}
static NTSTATUS NtClose(HANDLE h){assert(h==(void*)1||h==(void*)2);++closes;return 0;}
static NTSTATUS virtual_map_builtin_module(HANDLE m,void **module,SIZE_T *size,SECTION_IMAGE_INFORMATION *info,
 ULONG_PTR low,ULONG_PTR high,USHORT machine,int prefer,off_t offset){
 assert(m==(void*)2 && !prefer && !offset);(void)info;(void)low;(void)high;(void)machine;
 ++maps;if(!map_status){*module=(void*)3;*size=0x10000;}return map_status;
}
'''+(root/'build/ntdll-unix/ios_builtin_pe.h').read_text()+body+r'''
static UNICODE_STRING name(const char *s,WCHAR *buf){
 size_t n=strlen(s);for(size_t i=0;i<n;++i)buf[i]=(unsigned char)s[i];
 return (UNICODE_STRING){buf,(USHORT)(n*2),(USHORT)(n*2)};
}
static void path_is(const char *s){assert(seen_length==strlen(s)*2);for(size_t i=0;i<strlen(s);++i)assert(seen[i]==(unsigned char)s[i]);}
static NTSTATUS call(int builtin){
 WCHAR buf[512];UNICODE_STRING n=name("\\??\\C:\\games\\VCRUNTIME140.dll",buf);ANSI_STRING a={0};
 struct pe_image_info image={builtin};SECTION_IMAGE_INFORMATION info;void *module=NULL;SIZE_T size=0;
 return load_builtin(&image,&n,&a,0,&info,&module,&size,0,UINTPTR_MAX,0);
}
int main(void){
 order=LO_BUILTIN;ec=1;assert(call(0)==0);path_is("\\??\\C:\\windows\\sysx64\\VCRUNTIME140.dll");assert(opens==1&&maps==1&&closes==2);
 ec=0;assert(call(0)==0);path_is("\\??\\C:\\windows\\sysaa64\\VCRUNTIME140.dll");
 wow=1;assert(call(0)==0);path_is("\\??\\C:\\windows\\syswow64\\VCRUNTIME140.dll");wow=0;
 int saved=opens;assert(call(1)==STATUS_IMAGE_ALREADY_LOADED&&opens==saved);
 order=LO_DISABLED;assert(call(0)==STATUS_DLL_NOT_FOUND&&opens==saved);
 for(order=LO_DEFAULT;order<=LO_NATIVE_BUILTIN;++order)assert(call(0)==STATUS_IMAGE_ALREADY_LOADED&&opens==saved);
 order=LO_BUILTIN;open_status=STATUS_OBJECT_NAME_NOT_FOUND;assert(call(0)==STATUS_DLL_NOT_FOUND);
 order=LO_BUILTIN_NATIVE;assert(call(0)==STATUS_IMAGE_ALREADY_LOADED);
 open_status=STATUS_ACCESS_DENIED;assert(call(0)==STATUS_ACCESS_DENIED);
 open_status=0;section_status=STATUS_INVALID_IMAGE_FORMAT;saved=closes;assert(call(0)==section_status&&closes==saved+1);
 section_status=0;map_status=STATUS_DLL_NOT_FOUND;saved=closes;assert(call(0)==STATUS_IMAGE_ALREADY_LOADED&&closes==saved+2);
 order=LO_BUILTIN;assert(call(0)==STATUS_DLL_NOT_FOUND);
 map_status=0;WCHAR buf[512];UNICODE_STRING n=name("vcruntime140.dll",buf);SECTION_IMAGE_INFORMATION info;void *m=NULL;SIZE_T size=0;
 saved=opens;n.Length--;assert(ios_load_builtin_pe(&n,0,&info,&m,&size,0,0,0)==STATUS_DLL_NOT_FOUND&&opens==saved);
 n.Length++;assert(ios_load_builtin_pe(&n,0,&info,&m,&size,0,0,1)==STATUS_DLL_NOT_FOUND&&opens==saved);
 n.Length=sizeof(buf);memset(buf,'a',sizeof(buf));assert(ios_load_builtin_pe(&n,0,&info,&m,&size,0,0,0)==STATUS_DLL_NOT_FOUND&&opens==saved);
 puts("PASS: production builtin preferences select architecture-matched PE farms; native/default/disabled choices, missing files, validation failures and handle cleanup preserved");
}
'''
    run('builtin', fixture, ['-DWINE_IOS=1'])
    pin=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD:wine'],text=True).strip()
    old=subprocess.check_output(['git','-C',str(root/'wine'),'show',pin+':dlls/ntdll/unix/sync.c'])
    file=folder/'dlls/ntdll/unix/sync.c';file.parent.mkdir(parents=True);file.write_bytes(old)
    subprocess.run(['git','-C',str(folder),'apply',str(root/'patches/wine-timed-sleep.patch')],check=True)
    source=file.read_text(encoding='utf-8');start=source.index('NTSTATUS WINAPI NtDelayExecution(')
    delay=source[start:source.index('\n/********',start)]
    start=source.index('NTSTATUS WINAPI NtYieldExecution(')
    yield_body=source[start:source.index('\n/********',start)]
    assert 'return ios_scheduler_yield();' in yield_body
    scheduler=(root/'build/ntdll-unix/ios_scheduler_yield.h').read_text()
    common=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>
typedef struct { int64_t QuadPart; } LARGE_INTEGER;
typedef int64_t timeout_t;
typedef int32_t NTSTATUS;
typedef int BOOLEAN;
#define WINAPI
#define STATUS_SUCCESS 0
#define STATUS_TIMEOUT 258
#define SELECT_INTERRUPTIBLE 1
#define SELECT_ALERTABLE 2
#define TIMEOUT_INFINITE INT64_MAX
#define TRUE 1
#define FALSE 0
#define IOS_ECO_POLL() ((void)0)
static long long ios_xp_delay,ios_xp_delay_hist[6];
static int yields,waits,selects,interrupts;
static unsigned wait_result;
static unsigned server_wait(void*a,int b,int flags,const LARGE_INTEGER*t){(void)a;(void)b;(void)t;assert(flags==3);++waits;return wait_result;}
'''
    fake=common+r'''
#ifdef _WIN32
struct timeval { long tv_sec,tv_usec; };
#else
#include <sys/select.h>
#endif
static int64_t now;
static jmp_buf infinite;
static void NtQuerySystemTime(LARGE_INTEGER *out){out->QuadPart=now;}
static unsigned NtYieldExecution(void){++yields;return 0x40000024;}
static int fake_select(int a,void*b,void*c,void*d,struct timeval*t){
 (void)a;(void)b;(void)c;(void)d;++selects;if(!t)longjmp(infinite,1);
 int64_t delta=((int64_t)t->tv_sec*1000000+t->tv_usec)*10;
 if(interrupts){--interrupts;now+=delta/2;errno=EINTR;return -1;}
 now+=delta;return 0;
}
#define select fake_select
'''+delay+r'''
int main(void){
 LARGE_INTEGER t;now=100000000;t.QuadPart=-10000;assert(!NtDelayExecution(0,&t));assert(now==100010000&&selects==1&&yields==0);
 t.QuadPart=now+23000;assert(!NtDelayExecution(0,&t)&&now==100033000&&yields==0);
 int saved=selects;t.QuadPart=now-1;assert(!NtDelayExecution(0,&t)&&selects==saved&&yields==0);
 t.QuadPart=0;assert(NtDelayExecution(0,&t)==0x40000024&&yields==1);
 t.QuadPart=-10000;interrupts=1;saved=selects;assert(!NtDelayExecution(0,&t)&&selects==saved+2&&now==100043000&&yields==1);
 wait_result=STATUS_TIMEOUT;assert(!NtDelayExecution(1,&t)&&waits==1&&yields==1);
 wait_result=0xc0;assert(NtDelayExecution(1,&t)==0xc0&&waits==2);
 if(!setjmp(infinite))NtDelayExecution(0,NULL);assert(yields==1);
 t.QuadPart=TIMEOUT_INFINITE;if(!setjmp(infinite))NtDelayExecution(0,&t);assert(yields==1);
 puts("PASS: production timed sleeps avoid synthetic yield; zero/expired/absolute/relative waits, interruption, APC and infinite semantics preserved");
}
'''
    run('sleep',fake,['-DWINE_IOS=1'])
    yield_common=common+r'''
#include <time.h>
#include <stdlib.h>
typedef unsigned int useconds_t;
static long long ios_xp_yield,ios_xp_yield_slept;
static long long madeira_cfg_int(const char *key,long long value){(void)key;return value;}
'''
    yield_fake=yield_common+r'''
#define MACH_PORT_NULL 0
#define SWITCH_OPTION_NONE 0
#define KERN_SUCCESS 0
#define STATUS_NO_YIELD_PERFORMED 0x40000024
static int switches,parks,switch_status;
static int thread_switch(int port,int option,int timeout){assert(!port&&!option&&!timeout);++switches;return switch_status;}
static int fake_clock(int clock_id,struct timespec *out){(void)clock_id;out->tv_sec=0;out->tv_nsec=1000;return 0;}
static int fake_park(useconds_t us){assert(us==100);++parks;return 0;}
#undef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#define clock_gettime fake_clock
#define usleep fake_park
'''+scheduler+yield_body+r'''
int main(void){
 assert(NtYieldExecution()==STATUS_SUCCESS&&switches==1&&!parks);
 switch_status=1;assert(NtYieldExecution()==STATUS_NO_YIELD_PERFORMED&&switches==2);
 switch_status=0;for(int i=0;i<300;++i)assert(NtYieldExecution()==STATUS_SUCCESS);
 assert(parks>0&&ios_xp_yield==302&&ios_xp_yield_slept==parks);
 puts("PASS: production Windows yield uses Mach SWITCH_OPTION_NONE; failed handoff status and adaptive anti-spin parking preserved");
}
'''
    run('yield',yield_fake,['-DWINE_IOS=1'])
    if sys.platform=='darwin':
        real=common+r'''
#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <pthread/qos.h>
#include <sched.h>
#include <time.h>
static void NtQuerySystemTime(LARGE_INTEGER *out){struct timeval t;gettimeofday(&t,NULL);out->QuadPart=(int64_t)t.tv_sec*10000000+t.tv_usec*10;}
static unsigned NtYieldExecution(void){++yields;sched_yield();return 0;}
'''+delay+r'''
int main(void){
 assert(!pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0));
 LARGE_INTEGER before,after,t={-10000};struct rusage a,b;getrusage(RUSAGE_SELF,&a);NtQuerySystemTime(&before);
 for(int i=0;i<100;++i)assert(!NtDelayExecution(0,&t));
 NtQuerySystemTime(&after);getrusage(RUSAGE_SELF,&b);
 double elapsed=(after.QuadPart-before.QuadPart)/10000.;
 double cpu=(b.ru_utime.tv_sec-a.ru_utime.tv_sec+b.ru_stime.tv_sec-a.ru_stime.tv_sec)*1000. +
 (b.ru_utime.tv_usec-a.ru_utime.tv_usec+b.ru_stime.tv_usec-a.ru_stime.tv_usec)/1000.;
 assert(yields==0&&elapsed>=90&&elapsed<3000&&cpu<elapsed/2+20);
 qos_class_t qos;int rel;assert(!pthread_get_qos_class_np(pthread_self(),&qos,&rel)&&qos==QOS_CLASS_USER_INTERACTIVE);
 printf("PASS: native Darwin timed sleeps park without spin or scheduler-yield and retain QoS (elapsed=%.2fms CPU=%.2fms)\n",elapsed,cpu);
}
'''
        run('darwin',real,['-DWINE_IOS=1'])
        native_yield=yield_common+r'''
#include <unistd.h>
#include <mach/mach.h>
#include <mach/thread_switch.h>
#include <pthread.h>
#include <pthread/qos.h>
#define STATUS_NO_YIELD_PERFORMED 0x40000024
'''+scheduler+yield_body+r'''
int main(void){
 assert(!pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0));
 for(int i=0;i<128;++i)assert(NtYieldExecution()==STATUS_SUCCESS);
 qos_class_t qos;int rel;assert(!pthread_get_qos_class_np(pthread_self(),&qos,&rel)&&qos==QOS_CLASS_USER_INTERACTIVE);
 assert(ios_xp_yield==128);
 puts("PASS: native Darwin production yield executes 128 non-depressing Mach handoffs and retains QoS");
}
'''
        run('darwin-yield',native_yield,['-DWINE_IOS=1'])
    else:
        print('SKIP: real Darwin sleep/QoS execution requires macOS; CI requires it')
