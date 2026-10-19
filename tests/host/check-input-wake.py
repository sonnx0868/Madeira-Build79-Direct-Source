#!/usr/bin/env python3
"""Reproduce a sleeping GUI queue using the production input producer/pump.

Darwin uses real nonblocking pipes, poll and pthreads under ASan/UBSan. Wine
queue registration calls are controlled adapters; this is not an iPad game-FPS
or scene-specific deadlock measurement.
"""
from pathlib import Path
import argparse
import json
import os
import platform
import shutil
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--require-darwin',action='store_true')
args=parser.parse_args()
app=(root/'app/Madeira/Winios/Winios.m').read_text(encoding='utf-8')
driver=(root/'build/win32u-unix/driver_ios.c').read_text(encoding='utf-8')
message=(root/'build/win32u-unix/message_ios.c').read_text(encoding='utf-8')
def function(text,name):
    start=text.index(name)
    # Find the declaration's line, then its balanced function body.
    start=text.rfind('\n',0,start)+1
    opening=text.index('{',start);end=opening+1;depth=1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
assert 'if (!ios_slice && winios_drv_input_wake_ready())' in message
assert 'winios_user_driver.pThreadDetach = winios_drv_thread_detach' in driver
assert 'winios_input_wake_signal();' in function(app,'void winios_request_session_close(')
assert 'winios_input_wake_signal();' in function(app,'static void winios_q_push_ev(')
assert 'winios_input_wake_ack' not in function(driver,'int winios_drv_input_wake_ready(')
if platform.system()!='Darwin':
    if args.require_darwin:raise SystemExit('Darwin pipe/poll integration test required')
    print('PASS: input wake coupling and fallback contracts; SKIP real pipe/poll test requires Darwin')
    raise SystemExit(0)

wrappers=app[app.index('static struct madeira_input_wake_registry g_input_wakes'):app.index('/* csops syscall')]
ring=app[app.index('#define WINIOS_RING_SIZE'):app.index('/* Public C entry points for Swift / UIKit gesture handlers.')]
registration=driver[driver.index('struct winios_thread_input_wake'):driver.index('/* Direct mode (no virtual desktop)')]
pump=function(app,'BOOL winios_pProcessEvents(')
dispatch=function(message,'static BOOL process_driver_events(')
c=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>
#include <poll.h>
#include <time.h>
#include "InputWakeQueue.h"
typedef void *HWND;
typedef uintptr_t HANDLE,ULONG_PTR;
typedef unsigned DWORD;
typedef int BOOL,NTSTATUS;
#define WINE_IOS 1
#define QS_DRIVER 0x80000000u
#define QS_HARDWARE 0x40000000u
#define QS_ALLINPUT 0x04ffu
typedef unsigned UINT;
typedef long long LONGLONG;
typedef struct {long long QuadPart;} LARGE_INTEGER;
typedef struct {int unused;} MSG;
struct peek_message_filter {int internal;};
static _Thread_local unsigned mock_internal_bits,mask_updates;
static _Thread_local int mock_poll_armed=1,mock_hardware_message;
#define TRUE 1
#define FALSE 0
#define GENERIC_READ 1
#define SYNCHRONIZE 2
#define MOUSEEVENTF_MOVE 1
#define MOUSEEVENTF_LEFTDOWN 2
#define MOUSEEVENTF_LEFTUP 4
#define MOUSEEVENTF_ABSOLUTE 0x8000
static _Thread_local struct {uint64_t driver_data;} thread_info;
static _Thread_local int queue_fd=-1;
static _Thread_local int fail_registration;
static unsigned handles_closed;
#define NtUserGetThreadInfo() (&thread_info)
static unsigned GetCurrentThreadId(void){return 0x24;}
static int wine_server_fd_to_handle(int fd,unsigned access,unsigned attrs,HANDLE *handle) {
    assert(access==(GENERIC_READ|SYNCHRONIZE) && attrs==0);
    if(fail_registration==1)return 0xc0000017;
    *handle=(HANDLE)(fd+1);return 0;
}
enum {OP_set_queue_fd,OP_set_queue_mask};
struct Request {unsigned op;HANDLE handle;unsigned poll_events,wake_mask,changed_mask;};
#define SERVER_START_REQ(name) do {struct Request storage={.op=OP_##name},*req=&storage;
#define SERVER_END_REQ } while(0)
static HANDLE wine_server_obj_handle(HANDLE h){return h;}
int winios_drv_input_wake_ready(void);
static int wine_server_call(struct Request *req){
    if(req->op==OP_set_queue_mask){
        ++mask_updates;
        if(req->poll_events){mock_internal_bits&=~QS_DRIVER;mock_poll_armed=1;}
        return 0;
    }
    assert(queue_fd<0);
    assert(!winios_drv_input_wake_ready()); // creating a pipe is not registration completion
    if(fail_registration==2)return 0xc0000022;
    queue_fd=dup((int)req->handle-1);assert(queue_fd>=0);return 0;
}
static void NtClose(HANDLE h){assert(h);++handles_closed;}
__WRAPPERS__
__REGISTRATION__
static _Atomic unsigned int g_session_close_request;
static _Atomic(uintptr_t) g_restore_foreground;
static HWND winios_drv_session_close_target(int force){(void)force;return NULL;}
static void winios_drv_close_session(HWND h,int force){(void)h;(void)force;}
static int winios_drv_foreground_if_owner(HWND h){(void)h;return 0;}
static double CACurrentMediaTime(void){return 0;}
static void winios_dump_window_tree(void){}
static unsigned delivered,down_count,up_count;
static int expected_down;
static pthread_mutex_t gate_lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_cond=PTHREAD_COND_INITIALIZER;
static int pause_down,entered_down,release_down;
static void winios_drv_post_mouse(int x,int y,unsigned flags,unsigned data,HWND hwnd){
    (void)x;(void)y;(void)data;(void)hwnd;
    if(flags&MOUSEEVENTF_LEFTDOWN){
        pthread_mutex_lock(&gate_lock);
        if(pause_down){entered_down=1;pthread_cond_broadcast(&gate_cond);while(!release_down)pthread_cond_wait(&gate_cond,&gate_lock);}
        pthread_mutex_unlock(&gate_lock);
        assert(!expected_down);expected_down=1;++down_count;
    }
    if(flags&MOUSEEVENTF_LEFTUP){assert(expected_down);expected_down=0;++up_count;}
    ++delivered;
    mock_hardware_message=1;
}
static void winios_drv_post_key(unsigned short key,unsigned flags){(void)key;(void)flags;++delivered;mock_hardware_message=1;}
static void winios_drv_post_key_scan(unsigned short key,unsigned short scan,unsigned flags){(void)scan;winios_drv_post_key(key,flags);}
__RING__
__PUMP__
static int check_internal_bits(unsigned mask){return !!(mock_internal_bits&mask);}
static int check_queue_masks(unsigned wake,unsigned changed){(void)wake;(void)changed;return 1;}
static int is_queue_signaled(void){return mock_hardware_message;}
static void peek_message(MSG *message,struct peek_message_filter *filter){(void)message;(void)filter;}
static void NtQueryPerformanceCounter(LARGE_INTEGER *counter,LARGE_INTEGER *frequency){counter->QuadPart=1;frequency->QuadPart=1;}
static const struct {BOOL (*pProcessEvents)(DWORD);} fixture_driver={winios_pProcessEvents};
#define user_driver (&fixture_driver)
__DISPATCH__
static int readable(int fd,int timeout){struct pollfd p={fd,POLLIN,0};int r;do r=poll(&p,1,timeout);while(r<0&&errno==EINTR);assert(r>=0);return r>0&&(p.revents&POLLIN);}
static void cleanup_thread(void){winios_drv_thread_detach();if(queue_fd>=0){close(queue_fd);queue_fd=-1;}assert(thread_info.driver_data==0);}
static unsigned fd_count(void){unsigned n=0;for(int i=0;i<2048;++i)if(fcntl(i,F_GETFD)>=0)++n;return n;}
static void *delayed_producer(void *ignored){
    (void)ignored;
    winios_q_push_ev(WINIOS_EV_MOUSE,20,30,MOUSEEVENTF_LEFTDOWN,0);
    winios_q_push_ev(WINIOS_EV_MOUSE,20,30,MOUSEEVENTF_LEFTUP,0);
    return NULL;
}
static void *pump_thread(void *ignored){(void)ignored;assert(winios_pProcessEvents(0));cleanup_thread();return NULL;}
int main(void){
    setenv("MADEIRA_QUIET","1",1);unsetenv("MADEIRA_DESKTOP");
    unsigned initial_fds=fd_count();
    assert(!winios_drv_input_wake_ready());
    assert(winios_pProcessEvents(0));assert(queue_fd>=0); // initial fd acknowledgement, no input
    assert(winios_drv_input_wake_ready() && !readable(queue_fd,0));
    unsigned long long token=winios_drv_prepare_input_wake();
    assert(token==winios_drv_prepare_input_wake());
    // Negative control: the old producer can queue a press/release without
    // waking the kernel wait. A later timer/key would have to run the pump.
    winios_q_push_ev_raw(WINIOS_EV_MOUSE,10,10,MOUSEEVENTF_LEFTDOWN,0);
    winios_q_push_ev_raw(WINIOS_EV_MOUSE,10,10,MOUSEEVENTF_LEFTUP,0);
    assert(!readable(queue_fd,20) && delivered==0);
    assert(winios_pProcessEvents(0) && delivered==2);
    // Actual producer: a genuinely sleeping poll wakes without another key,
    // a periodic timer, a graphics present or a foreground transition.
    pthread_t producer;assert(!pthread_create(&producer,NULL,delayed_producer,NULL));
    assert(readable(queue_fd,1000));
    // The pre-wait capability query must not acknowledge this pending input.
    for(unsigned i=0;i<100;++i)assert(winios_drv_input_wake_ready());
    assert(readable(queue_fd,0));
    assert(winios_pProcessEvents(0));assert(!pthread_join(producer,NULL));
    assert(delivered==4 && down_count==2 && up_count==2 && !expected_down);
    assert(!readable(queue_fd,0));
    // Execute Wine's actual event dispatcher. A peer may have consumed the
    // shared ring before this queue wakes. The private fd acknowledgement
    // must still clear QS_DRIVER and re-arm the poll source, without exposing
    // a fabricated application message or forcing every ordinary poll to RPC.
    mock_hardware_message=0;
    winios_input_wake_signal();mock_internal_bits=QS_DRIVER;mock_poll_armed=0;
    assert(!process_driver_events(QS_ALLINPUT,1,1));
    assert(!mock_internal_bits&&mock_poll_armed&&!readable(queue_fd,0));
    // The server may report a poll result captured before a non-waiting pump
    // drained the byte. A latched QS_DRIVER with an empty fd must also re-arm.
    mock_internal_bits=QS_DRIVER;mock_poll_armed=0;
    assert(!process_driver_events(QS_ALLINPUT,1,1));
    assert(!mock_internal_bits&&mock_poll_armed);
    unsigned updates=mask_updates;
    assert(!process_driver_events(QS_ALLINPUT,1,1)&&mask_updates==updates);
    winios_q_push_ev(WINIOS_EV_KEY,0x20,0,0,0);mock_internal_bits=QS_DRIVER;mock_poll_armed=0;
    assert(process_driver_events(QS_ALLINPUT,1,1));
    assert(!mock_internal_bits&&mock_poll_armed&&mask_updates==updates+1);
    mock_hardware_message=0;
    // Each subscriber has its own byte. One ack cannot steal another wake.
    int first,second;uint64_t a=madeira_input_wake_create(&g_input_wakes,&first),b=madeira_input_wake_create(&g_input_wakes,&second);assert(a&&b);
    madeira_input_wake_ack(&g_input_wakes,a);madeira_input_wake_ack(&g_input_wakes,b);
    winios_input_wake_signal();assert(readable(first,0)&&readable(second,0));
    madeira_input_wake_ack(&g_input_wakes,a);assert(!readable(first,0)&&readable(second,0));
    madeira_input_wake_ack(&g_input_wakes,b);madeira_input_wake_remove(&g_input_wakes,a);madeira_input_wake_remove(&g_input_wakes,b);
    winios_pProcessEvents(0);
    // High-rate motion coalesces and notifications remain bounded/nonblocking.
    for(unsigned i=0;i<100000;++i)winios_q_push_ev(WINIOS_EV_MOUSE,i,i,MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE,0);
    assert(readable(queue_fd,0));assert(winios_pProcessEvents(0));assert(!readable(queue_fd,0));
    assert(g_input_q.coalesced_moves==99999 && !g_input_q.dropped_critical);
    // Force two consumers between a ring pop and its Windows injection.
    pause_down=1;entered_down=release_down=0;
    winios_q_push_ev(WINIOS_EV_MOUSE,40,40,MOUSEEVENTF_LEFTDOWN,0);
    winios_q_push_ev(WINIOS_EV_MOUSE,40,40,MOUSEEVENTF_LEFTUP,0);
    pthread_t consumer;assert(!pthread_create(&consumer,NULL,pump_thread,NULL));
    pthread_mutex_lock(&gate_lock);while(!entered_down)pthread_cond_wait(&gate_cond,&gate_lock);pthread_mutex_unlock(&gate_lock);
    unsigned before_competing=delivered;
    assert(winios_pProcessEvents(0)); // acknowledge this subscriber, then re-arm QS_DRIVER
    assert(delivered==before_competing); // the second consumer cannot overtake DOWN
    pthread_mutex_lock(&gate_lock);release_down=1;pthread_cond_broadcast(&gate_cond);pthread_mutex_unlock(&gate_lock);
    assert(!pthread_join(consumer,NULL));pause_down=0;
    assert(down_count==3&&up_count==3&&!expected_down);
    cleanup_thread();
    // Reuse cannot let an old token acknowledge/close the new thread's pipe.
    int reused;uint64_t next=madeira_input_wake_create(&g_input_wakes,&reused);assert(next&&next!=token);
    assert(!madeira_input_wake_ack(&g_input_wakes,token));
    madeira_input_wake_remove(&g_input_wakes,token);assert(readable(reused,0));
    madeira_input_wake_remove(&g_input_wakes,next);
    for(int failure=1;failure<=2;++failure){fail_registration=failure;assert(!winios_drv_prepare_input_wake());assert(!winios_drv_input_wake_ready());cleanup_thread();}fail_registration=0;
    // Descriptor cleanup and bounded subscription capacity.
    uint64_t tokens[MADEIRA_INPUT_WAKE_SLOTS];int descriptors[MADEIRA_INPUT_WAKE_SLOTS];
    for(unsigned i=0;i<MADEIRA_INPUT_WAKE_SLOTS;++i){tokens[i]=madeira_input_wake_create(&g_input_wakes,&descriptors[i]);assert(tokens[i]);}
    int excess;assert(!madeira_input_wake_create(&g_input_wakes,&excess)&&excess==-1);
    for(unsigned i=0;i<MADEIRA_INPUT_WAKE_SLOTS;++i)madeira_input_wake_remove(&g_input_wakes,tokens[i]);
    assert(fd_count()==initial_fds);
    puts("PASS: old queued input does not wake; production producer/pump/dispatcher wakes a sleeping GUI queue, clears and re-arms QS_DRIVER after a peer drain, preserves pending signals, subscriber independence, press/release order, burst coalescing, failure fallback and descriptor cleanup");
}
'''.replace('__WRAPPERS__',wrappers).replace('__REGISTRATION__',registration).replace('__RING__',ring).replace('__PUMP__',pump).replace('__DISPATCH__',dispatch)
with tempfile.TemporaryDirectory(prefix='madeira-input-wake-') as temp:
    folder=Path(temp);source=folder/'check.c';source.write_text(c,encoding='utf-8')
    cc=os.environ.get('CC') or shutil.which('clang');assert cc
    executable=folder/'check'
    subprocess.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-Wno-unused-function',
        '-fsanitize=address,undefined','-pthread','-I'+str(root/'app/Madeira/Winios'),str(source),'-o',str(executable)],check=True)
    subprocess.run([str(executable)],check=True,timeout=30)
    if os.environ.get('BITRISE_DEPLOY_DIR'):
        report={'productionProducerAndEventPumpExecuted':True,'actualPipeAndPoll':True,'sleepingQueueReproduction':True,
                'negativeControlDoesNotWake':True,'candidateWakesWithoutExtraKey':True,'queueSubscribersIndependent':True,
                'concurrentDeliveryOrdered':True,'descriptorCleanup':True,'gameSceneFreezeResolvedOnIPad':False}
        report['productionWineDispatcherExecuted']=True
        report['emptyWakeClearsAndRearmsDriverBit']=True
        (Path(os.environ['BITRISE_DEPLOY_DIR'])/'input-wake-validation.json').write_text(json.dumps(report,indent=2)+'\n')
