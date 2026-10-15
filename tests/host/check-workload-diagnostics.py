#!/usr/bin/env python3
"""Exercise production counter units and distinguish work/rate/IPC changes."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
compiler = os.environ.get('CC') or shutil.which('clang')
if not compiler:
    raise SystemExit('clang required')
fixture = r'''
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "WorkloadDiagnostics.h"
static int near(double x,double y) {return fabs(x-y)<0.000001;}
int main(void) {
    MadeiraWorkCounters zero={0},fast={240000000,30000000000ull,60000000000ull};
    MadeiraWorkSample a=madeira_work_sample(zero,fast,125,3,5,300);
    assert(a.valid && a.counter_valid && near(a.cpu_ms,10000));
    assert(near(a.cpu_cores,2) && near(a.cycles_per_cpu_ns,3) && near(a.instructions_per_cycle,2));
    assert(near(a.instructions_per_frame,200000000));
    // Twice the CPU time for the same instructions and frame count: lower
    // counter rate, rather than increased work. This is not a device benchmark.
    MadeiraWorkCounters lower=fast;lower.cpu_ticks*=2;
    MadeiraWorkSample b=madeira_work_sample(zero,lower,125,3,5,300);
    assert(near(b.instructions_per_frame,a.instructions_per_frame));
    assert(near(b.cycles_per_cpu_ns,1.5) && near(b.cpu_ms,20000));
    // More instructions at the same rate is a different signature.
    lower=fast;lower.instructions*=2;
    b=madeira_work_sample(zero,lower,125,3,5,300);
    assert(near(b.cycles_per_cpu_ns,3) && near(b.instructions_per_frame,400000000));
    b=madeira_work_sample(zero,fast,125,3,5,0);
    assert(b.valid && b.cpu_ms_per_frame==-1 && b.instructions_per_frame==-1);
    for(unsigned i=0;i<3;i++) {
        MadeiraWorkCounters unavailable=fast;
        if(i!=1) unavailable.cycles=0;
        if(i!=0) unavailable.instructions=0;
        b=madeira_work_sample(zero,unavailable,125,3,5,300);
        assert(b.valid && !b.counter_valid && b.instructions_per_frame==-1);
    }
    lower=fast;lower.cycles=0;
    b=madeira_work_sample(fast,lower,125,3,5,300);
    assert(b.valid && !b.counter_valid);
    lower=fast;lower.cpu_ticks--;
    assert(!madeira_work_sample(fast,lower,125,3,5,300).valid);
    assert(!madeira_work_sample(zero,fast,0,3,5,300).valid);
    assert(!madeira_work_sample(zero,fast,125,0,5,300).valid);
    assert(!madeira_work_sample(zero,fast,125,3,NAN,300).valid);
    assert(!madeira_work_sample(zero,fast,125,3,0,300).valid);
    assert(!madeira_work_sample(zero,fast,125,3,-5,300).valid);
    puts("PASS: counter units, workload/rate/IPC signatures, unsupported counters, rollback and loading windows");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-workload-') as scratch:
    folder=Path(scratch);source=folder/'check.c';exe=folder/'check.exe'
    source.write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=c11','-O2','-I'+str(root/'app/Madeira'),str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
