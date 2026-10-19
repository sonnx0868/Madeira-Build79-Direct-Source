#!/usr/bin/env python3
"""Ownership/restore cases and real Darwin pthread QoS using shipping code."""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
import shutil

root=Path(__file__).resolve().parents[2]
cc=os.environ.get('CC') or shutil.which('clang')
assert cc, 'clang required'
if '--require-darwin' in sys.argv and sys.platform!='darwin': raise SystemExit('Darwin required')

model=r'''
#include <assert.h>
#include <stdio.h>
#define MSP_PROCESS_CAPACITY 8
#include "SessionPriorityPolicy.h"
static unsigned current=0x21; static int relative=-3, failget, failset, sets;
static int get(unsigned *q,int *r) { *q=current;*r=relative;return failget; }
static int set(unsigned q,int r) { ++sets;if(failset)return failset;current=q;relative=r;return 0; }
static void image(MSPSession *s,unsigned pid,uintptr_t peb,const char *name) {
    uint16_t text[256];size_t n=strlen(name);assert(n<256);
    for(size_t i=0;i<n;++i)text[i]=(unsigned char)name[i];
    msp_find(s,pid,peb)->kind=msp_image_kind(text,n);
}
int main(void) {
    MSPSession s={0};
    msp_attach(&s,10,0,10);image(&s,10,10,"C:\\Steam\\Steam.exe");
    msp_attach(&s,20,10,20);image(&s,20,20,"C:/Steam/STEAMWEBHELPER.EXE");
    msp_attach(&s,100,0,100);image(&s,100,100,"REPLACED.exe");
    msp_attach(&s,101,100,101);image(&s,101,101,"chrome.exe");
    msp_attach(&s,102,999,102);image(&s,102,102,"steamwebhelper.exe");
    s.active=s.foreground=1;s.target=100;s.target_peb=100;
    assert(msp_role(&s,20,20)==MSP_NORMAL); // no background change during launch
    s.presented=1;
    assert(msp_role(&s,20,20)==MSP_BACKGROUND_UI);
    assert(msp_role(&s,10,10)==MSP_NORMAL); // Steam IPC stays unchanged
    assert(msp_role(&s,100,100)==MSP_NORMAL && msp_role(&s,101,101)==MSP_NORMAL);
    assert(msp_role(&s,102,102)==MSP_NORMAL); // missing ancestry is not evidence
    s.foreground=0;assert(msp_role(&s,20,20)==MSP_NORMAL);s.foreground=1;
    s.target=20;s.target_peb=20;assert(msp_role(&s,20,20)==MSP_NORMAL); // UI selected as game
    s.target=100;s.target_peb=100;
    msp_attach(&s,20,10,2000);assert(msp_role(&s,20,20)==MSP_NORMAL); // PID reuse ambiguity
    msp_retire(&s,20,20);image(&s,20,2000,"unknown.exe");
    assert(msp_find(&s,20,2000) && msp_role(&s,20,2000)==MSP_NORMAL);
    msp_attach(&s,30,31,30);image(&s,30,30,"chrome.exe");msp_attach(&s,31,30,31);
    assert(msp_role(&s,30,30)==MSP_NORMAL); // ancestry cycle
    msp_attach(&s,40,0,40);msp_attach(&s,41,0,41);assert(s.incomplete);
    assert(msp_role(&s,30,30)==MSP_NORMAL); // saturation fails open
    msp_retire(&s,100,100);assert(!s.target && !s.presented);
    uint16_t bad[]={0x100,'x'};assert(msp_image_kind(bad,2)==MSP_NORMAL);
    assert(msp_image_kind(NULL,1)==MSP_NORMAL);

    const MSPQoSOps ops={get,set,0x11};MSPQoS q={0};
    assert(msp_qos_apply(&q,0,0,&ops)==0 && !sets && current==0x21);
    assert(msp_qos_apply(&q,1,0,&ops)==1 && q.owned && current==0x11);
    for(int i=0;i<100;++i) assert(msp_qos_apply(&q,1,0,&ops)==0);
    assert(sets==1 && msp_qos_apply(&q,0,0,&ops)==2 && current==0x21 && relative==-3);
    current=0x11;assert(!msp_qos_apply(&q,1,0,&ops) && !q.owned); // Eco stays low
    current=0;assert(!msp_qos_apply(&q,1,0,&ops)); // unknown class stays unchanged
    current=0x21;failset=1;assert(msp_qos_apply(&q,1,0,&ops)<0 && !q.owned && current==0x21);
    failset=0;assert(msp_qos_apply(&q,1,0,&ops)==1);
    failget=1;assert(msp_qos_apply(&q,0,0,&ops)<0 && q.owned);failget=0;
    failset=1;assert(msp_qos_apply(&q,0,0,&ops)<0 && q.owned);failset=0;
    assert(msp_qos_apply(&q,0,0,&ops)==2 && current==0x21);
    assert(msp_qos_apply(&q,1,0,&ops)==1);current=0x09;
    assert(!msp_qos_apply(&q,0,0,&ops) && current==0x09 && !q.owned); // external choice wins
    current=0x21;assert(msp_qos_apply(&q,1,0,&ops)==1);current=0x11;
    assert(!msp_qos_apply(&q,1,1,&ops) && !q.owned); // live Eco toggle supersedes saved class
    puts("PASS: session ownership, selected descendants, shared services, unknown/PID reuse/cycles/saturation, exact restoration, Eco and QoS failures");
}
'''

darwin=r'''
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <pthread/qos.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <mach/mach.h>
#include "SessionPriorityAPI.h"
#include "ios_session_priority.h"
#include "SessionPriority.h"
static atomic_int stage=-1,ack,registered;
static unsigned observed[7][3];static int priorities[7][3];
static void image(unsigned pid,const char *name) {
    uint16_t text[80];size_t n=strlen(name);for(size_t i=0;i<n;++i)text[i]=name[i];
    madeira_session_priority_image(pid,text,n);
}
static void *worker(void *arg) {
    unsigned id=(unsigned)(uintptr_t)arg,pid=id==0?100:id==1?20:10;
    madeira_session_priority_thread(pid,pid,id==1?10:0,(void *)(uintptr_t)pid);
    image(pid,id==0?"REPLACED.exe":id==1?"steamwebhelper.exe":"steam.exe");
    if(id==0)madeira_session_priority_select(pid);
    atomic_fetch_add(&registered,1);
    for(int next=0;next<7;++next) {
        while(atomic_load(&stage)!=next)usleep(500);
        if(next==4 && id==1)madeira_session_priority_select(pid);
        { IOS_SESSION_PRIORITY_SCOPE(); /* production entry and exit path */ }
        qos_class_t q=0;int rel=0;assert(!pthread_get_qos_class_np(pthread_self(),&q,&rel));
        observed[next][id]=q;
        thread_extended_info_data_t info={0};mach_msg_type_number_t n=THREAD_EXTENDED_INFO_COUNT;
        assert(thread_info(pthread_mach_thread_np(pthread_self()),THREAD_EXTENDED_INFO,(thread_info_t)&info,&n)==KERN_SUCCESS);
        priorities[next][id]=info.pth_priority;
        atomic_fetch_add(&ack,1);
    }
    return NULL;
}
static void *late(void *arg) {
    unsigned pid=arg?100:21;
    madeira_session_priority_thread(pid,500,arg?0:10,(void *)(uintptr_t)pid);
    if(!arg)image(pid,"steamwebhelper.exe");
    { IOS_SESSION_PRIORITY_SCOPE(); }
    qos_class_t q=0;int r=0;assert(!pthread_get_qos_class_np(pthread_self(),&q,&r));
    assert(q==(arg?QOS_CLASS_USER_INTERACTIVE:QOS_CLASS_UTILITY));
    return NULL;
}
int main(void) {
    madeira_session_priority_begin();madeira_session_priority_foreground(1);
    pthread_attr_t attr;assert(!pthread_attr_init(&attr));
    assert(!pthread_attr_set_qos_class_np(&attr,QOS_CLASS_USER_INTERACTIVE,0));
    pthread_t threads[3];for(unsigned i=0;i<3;++i)assert(!pthread_create(&threads[i],&attr,worker,(void *)(uintptr_t)i));
    while(atomic_load(&registered)!=3)usleep(500);
    for(int phase=0;phase<7;++phase) {
        if(phase==1){madeira_session_priority_presented(100);madeira_session_priority_rendering();}
        if(phase==2)madeira_session_priority_foreground(0);
        if(phase==3)madeira_session_priority_foreground(1);
        if(phase==4){madeira_session_priority_end();madeira_session_priority_begin();}
        if(phase==5){madeira_session_priority_presented(20);madeira_session_priority_rendering();}
        if(phase==6)madeira_session_priority_end();
        atomic_store(&ack,0);atomic_store(&stage,phase);
        while(atomic_load(&ack)!=3)usleep(500);
        for(int i=0;i<3;++i) {
            unsigned expected=i==1 && (phase==1 || phase==3)?QOS_CLASS_UTILITY:QOS_CLASS_USER_INTERACTIVE;
            assert(observed[phase][i]==expected);
        }
        printf("DARWIN phase=%d game_qos=%#x ui_qos=%#x shared_qos=%#x priorities=%d/%d/%d\n",phase,
            observed[phase][0],observed[phase][1],observed[phase][2],priorities[phase][0],priorities[phase][1],priorities[phase][2]);
        if(phase==1) {
            assert(priorities[phase][1]<priorities[phase][0] && priorities[phase][2]==priorities[phase][0]);
            pthread_t t;assert(!pthread_create(&t,&attr,late,NULL));assert(!pthread_join(t,NULL));
            assert(!pthread_create(&t,&attr,late,(void *)1));assert(!pthread_join(t,NULL));
        }
    }
    for(unsigned i=0;i<3;++i)assert(!pthread_join(threads[i],NULL));
    pthread_attr_destroy(&attr);
    puts("PASS: real Darwin lowers only the background UI pthread; game/shared priorities stay high; resume, switching sessions, late threads and quit restore actual QoS");
    // Epoch changes while a native wait is blocked are applied before return.
    {IOS_SESSION_PRIORITY_SCOPE();madeira_session_priority_foreground(0);}
    assert(ios_priority_seen==atomic_load(&msp_epoch));
    unsigned long long before=clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
    for(unsigned i=0;i<2000000;++i)ios_session_priority_poll();
    unsigned long long elapsed=clock_gettime_nsec_np(CLOCK_UPTIME_RAW)-before;
    printf("NATIVE_HOOK_BENCH iterations=2000000 steady_poll_ns=%.2f (not game FPS)\n",(double)elapsed/2000000);
}
'''

with tempfile.TemporaryDirectory(prefix='madeira-session-priority-') as scratch:
    folder=Path(scratch)
    def run(name,code,extra=()):
        source=folder/(name+'.c');binary=folder/(name+'.exe');source.write_text(code,encoding='utf-8')
        subprocess.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror',
            '-I'+str(root/'app/Madeira'),'-I'+str(root/'build/ntdll-unix'),*extra,str(source),'-o',str(binary)],check=True)
        return subprocess.check_output([str(binary)],text=True,timeout=25)
    print(run('policy',model),end='')
    if sys.platform=='darwin':
        output=run('darwin',darwin)
        print(output,end='')
        receipt={'ownershipAndErrorCases':'passed','realDarwinPthreadQoS':'passed',
                 'nativeWaitEpochAndCleanup':'passed','physicalIPadFpsMeasured':False,
                 'observations':output.splitlines()}
        deploy=Path(os.environ.get('BITRISE_DEPLOY_DIR',folder));deploy.mkdir(parents=True,exist_ok=True)
        (deploy/'session-priority-validation.json').write_text(json.dumps(receipt,indent=2)+'\n')
    else: print('SKIP: real pthread QoS validation requires Darwin; required by Mac CI')
