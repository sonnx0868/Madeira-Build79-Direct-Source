#!/usr/bin/env python3
"""Check counter units, Darwin collection and safe PE data-export resolution."""
from pathlib import Path
import os, platform, shutil, subprocess, sys, tempfile

root=Path(__file__).resolve().parents[2]
compiler=os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
math_test=r'''
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "PerformanceCounters.h"
int main(void) {
    MadeiraPerfCounters z={0}, a={24000000,3000000000ull,6000000000ull,250000000};
    MadeiraPerfDelta d=madeira_perf_delta(z,a,125,3);
    assert(d.valid && d.rate_valid && fabs(d.cpu_ms-1000)<1e-8);
    assert(fabs(d.ghz-3)<1e-8 && fabs(d.ipc-2)<1e-8 && fabs(d.energy_mj-250)<1e-8);
    MadeiraPerfCounters half=a;half.ticks*=2;
    d=madeira_perf_delta(z,half,125,3);assert(d.rate_valid && fabs(d.ghz-1.5)<1e-8);
    d=madeira_perf_delta(a,z,125,3);assert(!d.valid && d.ghz==-1);
    assert(!madeira_perf_delta(z,a,125,0).valid);
    assert(!madeira_perf_delta(z,a,0,3).valid);
    half=a;half.cycles=0;
    d=madeira_perf_delta(z,half,125,3);assert(d.valid && !d.rate_valid && d.ghz==-1);
    half=a;half.instructions=0;
    assert(!madeira_perf_delta(z,half,125,3).rate_valid);
    MadeiraPerfCounters part={12000000,1000000000,2000000000,100000000}, rest;
    assert(madeira_perf_subtract(a,part,&rest) && rest.ticks==12000000 && rest.energy_nj==150000000);
    d=madeira_perf_delta(z,rest,125,3);assert(fabs(d.cpu_ms-500)<1e-8 && fabs(d.ghz-4)<1e-8);
    part.ticks=a.ticks+1;assert(!madeira_perf_subtract(a,part,&rest));
    part=a;part.instructions++;assert(!madeira_perf_subtract(a,part,&rest));
    part=a;part.energy_nj++;assert(!madeira_perf_subtract(a,part,&rest));
    puts("PASS: P/E counter units, rate versus work, absent counters, rollback and safe partitions");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-perf-units-') as tmp:
    p=Path(tmp);(p/'test.c').write_text(math_test,encoding='utf-8')
    subprocess.run([compiler,'-std=c11','-O2','-I'+str(root/'app/Madeira'),str(p/'test.c'),'-o',str(p/'test.exe')],check=True)
    subprocess.run([str(p/'test.exe')],check=True)

if platform.system()!='Darwin':
    if '--require-darwin' in sys.argv:raise SystemExit('Live metrics validation requires Darwin')
    print('SKIP: live Darwin P/E collector and VM-read checks run on Mac CI');raise SystemExit(0)

fixture=r'''
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/mach_time.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <unistd.h>
extern int proc_pid_rusage(int,int,void *);
static dispatch_queue_t stall_queue(void) {
    static dispatch_queue_t queue;static dispatch_once_t once;
    dispatch_once(&once,^{queue=dispatch_queue_create("madeira.metrics.fixture",DISPATCH_QUEUE_SERIAL);});
    return queue;
}
static uint32_t replaced_metrics_target_pid(void) {return 32;}
#include "ReplacedMetrics.h"
static void u32(unsigned char *p,uint32_t x) {memcpy(p,&x,4);}
static void u64(unsigned char *p,uint64_t x) {memcpy(p,&x,8);}
int main(void) { @autoreleasepool {
    unsigned char *image=calloc(1,4096),*peb=calloc(1,256),*ldr=calloc(1,256),*entry=calloc(1,256);
    uint16_t name[9]={'N','t','D','l','L','.','d','L','l'};
    memcpy(image,"MZ",2);u32(image+60,128);memcpy(image+128,"PE\0\0",4);
    image[128+24]=0x0b;image[128+25]=0x02;
    u32(image+128+24+112,512);u32(image+128+24+116,128);
    u32(image+512+20,1);u32(image+512+24,1);
    u32(image+512+28,650);u32(image+512+32,660);u32(image+512+36,670);
    u32(image+650,1024);u32(image+660,700);memcpy(image+700,"ios_xp_nt",10);
    u64(peb+0x18,(uint64_t)ldr);u64(ldr+0x10,(uint64_t)entry);u64(entry,(uint64_t)(ldr+0x10));
    u64(entry+0x30,(uint64_t)image);u32(entry+0x40,4096);entry[0x58]=18;u64(entry+0x60,(uint64_t)name);
    assert(rm_find_sync((uint64_t)peb)==(uint64_t)(image+1024));
    unsigned char byte=0;assert(!rm_read(0,&byte,1));assert(!rm_read(UINT64_MAX,&byte,2));
    assert(!rm_rva((uint64_t)image,4096,4095,&byte,2));
    assert(!rm_rva(UINT64_MAX,4096,2,&byte,1));
    u32(image+512+24,20000);assert(!rm_sync_export((uint64_t)image,4096));u32(image+512+24,1);
    u32(image+650,520);assert(!rm_sync_export((uint64_t)image,4096));u32(image+650,1024);
    u32(image+660,4094);assert(!rm_sync_export((uint64_t)image,4096));u32(image+660,700);
    RMSyncCounters *sync=(RMSyncCounters *)(image+1024);
    sync->magic=UINT64_C(0x31544e5058444d41);sync->frequency=24000000;
    RMRawCounts raw={6000,3000,100,200,500};
    MadeiraPerfCounters converted=rm_thread_counters(raw);
    assert(converted.ticks==300 && converted.cycles==3000 && converted.instructions==6000 && converted.energy_nj==500);
    madeira_replaced_metrics_prepare(1);
    madeira_replaced_metrics_thread(32,0x24,peb);
    madeira_replaced_metrics_thread(32,0x24,peb);
    dispatch_sync(stall_queue(),^{assert(rm_count==1);});
    uint64_t until=mach_absolute_time()+2400000;
    volatile uint64_t sink=0;while(mach_absolute_time()<until) sink=sink*31+7;
    (void)sink;
    dispatch_sync(stall_queue(),^{
        replaced_metrics_snapshot(CACurrentMediaTime(),60);
        assert(rm_previous_sync_valid && rm_sync_address==(uint64_t)sync);
        sync->contended+=8;sync->wait_ticks+=24000;sync->histogram[5]+=8;
        rm_sync_sample(32,(uint64_t)peb,CACurrentMediaTime(),1);
        assert(rm_previous_sync.contended==8 && rm_previous_sync.wait_ticks==24000);
        // Reset/reloaded counters become a new baseline, never a huge unsigned delta.
        sync->contended=0;sync->wait_ticks=0;
        rm_sync_sample(32,(uint64_t)peb,CACurrentMediaTime(),1);
        assert(rm_previous_sync.contended==0);
        rm_sync_sample(32,1,CACurrentMediaTime(),1);assert(!rm_previous_sync_valid);
    });
    madeira_replaced_metrics_prepare(0);
    madeira_replaced_metrics_thread(32,0x24,peb);
    dispatch_sync(stall_queue(),^{assert(rm_count==0 && !rm_active);});
    free(image);free(peb);free(ldr);free(entry);
    puts("PASS: live Darwin collector, exact threadcounts ABI, safe PE data export, malformed pointers, duplicate attach and lifecycle reset");
} }
'''
with tempfile.TemporaryDirectory(prefix='madeira-replaced-live-') as tmp:
    p=Path(tmp);(p/'test.m').write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=c11','-O2','-fblocks','-fobjc-arc','-I'+str(root/'app/Madeira'),str(p/'test.m'),
                    '-framework','Foundation','-framework','QuartzCore','-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
