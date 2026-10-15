#!/usr/bin/env python3
"""Exercise the patched production pacer, including slow frames and interrupts."""
from pathlib import Path
import importlib.util, os, platform, shutil, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('dxmt_patches',root/'tests/host/check-dxmt-patches.py')
patches=importlib.util.module_from_spec(spec);spec.loader.exec_module(patches)
source=patches.patched_sources(root)['src/winemetal/unix/winemetal_unix.c']
def function(start):
    a=source.index(start);b=source.index('{',a);end=b+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[a:end]
wrapper=function('static void madeira_present_limit_surface(')
enabled=function('static int madeira_present_limiter_enabled(')
acquire=function('static NTSTATUS\n_MetalLayer_nextDrawable(')
assert 'presentSkipped' not in acquire and 'since < 0.018' not in acquire
assert acquire.index('madeira_present_limit_surface(')<acquire.index(' nextDrawable]')
assert 'static _Thread_local MadeiraPresentPacer states[8]' in wrapper
cc=os.environ.get('CC') or shutil.which('clang')
if not cc:raise SystemExit('clang required')
fixture=r'''
#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "present_pacing.h"
#define STDERR_FILENO 2
#define TARGET_OS_IOS 1
#ifdef _WIN32
#define dprintf(fd,...) fprintf(stderr,__VA_ARGS__)
#else
#include <unistd.h>
#endif
typedef struct { unsigned numer,denom; } mach_timebase_info_data_t;
typedef int kern_return_t;
enum { KERN_SUCCESS=0, KERN_ABORTED=14 };
static uint64_t now_ticks, slept;
static int waits, interrupted, fail_wait;
static _Atomic int g_madeira_vsync_mode=1,g_madeira_panel_hz=120;
static _Atomic uint64_t g_madeira_pacing_generation=1;
static uint64_t mach_absolute_time(void) {return now_ticks;}
static int mach_timebase_info(mach_timebase_info_data_t *tb) {tb->numer=tb->denom=1;return 0;}
static int mach_wait_until(uint64_t deadline) {
    ++waits;
    if (fail_wait) return 5;
    if (interrupted) {--interrupted;return KERN_ABORTED;}
    now_ticks=deadline;return KERN_SUCCESS;
}
static int madeira_frame_hooks_on(void) {return 1;}
static void ios_frame_limiter(unsigned long long ns) {slept+=ns;}
''' +enabled+'\n'+wrapper+r'''
static void reset(int mode,int panel) {
    ++g_madeira_pacing_generation;g_madeira_vsync_mode=mode;g_madeira_panel_hz=panel;
    now_ticks=slept=0;waits=interrupted=fail_wait=0;
}
int main(int argc,char **argv) {
    (void)argv;
    if (argc>1) {
        assert(!madeira_present_limiter_enabled());
        madeira_present_limit_surface(1);assert(!waits);return 0;
    }
    assert(madeira_present_limiter_enabled());
    reset(1,120);
    for(int i=0;i<120;i++) {
        madeira_present_limit_surface(1);
        uint64_t expected=(uint64_t)i*16666666;
        assert(now_ticks==expected);now_ticks+=4000000;
    }
    assert(waits==119 && slept>1400000000);
    // A 17/20 ms renderer is never rounded to 33 ms by this limiter.
    for(uint64_t work=17000000;work<=20000000;work+=3000000) {
        reset(1,120);
        for(int i=0;i<120;i++) {madeira_present_limit_surface(1);assert(now_ticks==(uint64_t)i*work);now_ticks+=work;}
        assert(waits==0);
    }
    reset(3,120);madeira_present_limit_surface(1);madeira_present_limit_surface(1);assert(now_ticks==33333333);
    reset(0,120);madeira_present_limit_surface(1);madeira_present_limit_surface(1);assert(now_ticks==8333333);
    reset(0,60);madeira_present_limit_surface(1);madeira_present_limit_surface(1);assert(now_ticks==16666666);
    reset(1,120);madeira_present_limit_surface(1);interrupted=2;
    madeira_present_limit_surface(1);assert(waits==3 && now_ticks==16666666);
    now_ticks+=5000000000ull;madeira_present_limit_surface(1);
    uint64_t resumed=now_ticks;madeira_present_limit_surface(1);assert(now_ticks==resumed+16666666);
    reset(1,120);madeira_present_limit_surface(1);fail_wait=1;madeira_present_limit_surface(1);assert(waits==1);
    // Distinct surfaces keep separate clocks on one producer, without halving
    // either window's rate; a live mode change does not inherit old debt.
    reset(1,120);madeira_present_limit_surface(1);madeira_present_limit_surface(2);
    madeira_present_limit_surface(1);assert(now_ticks==16666666);
    madeira_present_limit_surface(2);assert(now_ticks==16666666);
    ++g_madeira_pacing_generation;g_madeira_vsync_mode=3;
    madeira_present_limit_surface(1);assert(now_ticks==16666666);
    madeira_present_limit_surface(1);assert(now_ticks==49999999);
    MadeiraPresentPacer overflow={UINT64_MAX-2,10,1,1};
    assert(madeira_present_deadline(&overflow,UINT64_MAX-5,10,1,1)==UINT64_MAX-5 && !overflow.next);
    puts("PASS: production producer deadlines at 30/60/panel rate, slow frames avoid 30 FPS rounding, interrupted waits, resume, mode changes and independent surfaces");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-present-pacing-') as tmp:
    p=Path(tmp);c=p/'check.c';exe=p/'check.exe';c.write_text(fixture,encoding='utf-8')
    subprocess.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-I'+str(root/'build/dxmt-ios'),str(c),'-o',str(exe)],check=True)
    env=dict(os.environ);env.pop('MADEIRA_PRESENT_LIMITER',None)
    subprocess.run([str(exe)],check=True,env=env)
    subprocess.run([str(exe),'opt-out'],check=True,env=dict(env,MADEIRA_PRESENT_LIMITER='0'))
    if platform.system()=='Darwin':
        live=r'''
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach/mach_time.h>
#include <sys/resource.h>
#include "present_pacing.h"
#define TARGET_OS_IOS 1
static _Atomic int g_madeira_vsync_mode=1,g_madeira_panel_hz=120;
static _Atomic uint64_t g_madeira_pacing_generation=1;
static int madeira_frame_hooks_on(void) {return 0;}
static void ios_frame_limiter(unsigned long long ns) {(void)ns;}
''' +enabled+'\n'+wrapper+r'''
int main(void) {
    struct rusage a,b;getrusage(RUSAGE_SELF,&a);
    mach_timebase_info_data_t tb;mach_timebase_info(&tb);uint64_t started=mach_absolute_time();
    for(unsigned i=0;i<40;++i) madeira_present_limit_surface(1);
    double elapsed=(double)(mach_absolute_time()-started)*tb.numer/tb.denom/1e9;
    getrusage(RUSAGE_SELF,&b);
    double cpu=(b.ru_utime.tv_sec-a.ru_utime.tv_sec)+(b.ru_utime.tv_usec-a.ru_utime.tv_usec)/1e6+
        (b.ru_stime.tv_sec-a.ru_stime.tv_sec)+(b.ru_stime.tv_usec-a.ru_stime.tv_usec)/1e6;
    assert(elapsed>=0.60 && elapsed<3.5 && cpu<0.20);
    printf("PASS: real Darwin producer wait elapsed=%.3fs cpu=%.4fs (no busy spin)\n",elapsed,cpu);
}
'''
        c.write_text(live,encoding='utf-8')
        subprocess.run([cc,'-std=c11','-O2','-I'+str(root/'build/dxmt-ios'),str(c),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True,env=env)
