#!/usr/bin/env python3
"""Execute the production submit barrier and GPU timestamp/session counters."""
from pathlib import Path
import os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
compiler=os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
fixture=r'''
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include "GPUForegroundGate.h"
#include "GPUFrameDiagnostics.h"
#ifdef _WIN32
#include <windows.h>
static void pause_ms(unsigned ms) { Sleep(ms); }
#else
#include <unistd.h>
static void pause_ms(unsigned ms) { usleep(ms*1000); }
#endif
static MadeiraGPUForegroundGate gate=MADEIRA_GPU_FOREGROUND_INITIALIZER;
static atomic_uint submitted,close_started,close_finished;
static void *submit(void *unused) {
    madeira_gpu_gate_begin(&gate);
    atomic_fetch_add(&submitted,1);
    madeira_gpu_gate_end(&gate);
    return NULL;
}
static void *close_gate(void *unused) {
    atomic_store(&close_started,1);
    madeira_gpu_gate_set(&gate,0);
    atomic_store(&close_finished,1);
    return NULL;
}
int main(void) {
    pthread_t workers[4],closer;
    madeira_gpu_gate_set(&gate,0);
    for(unsigned i=0;i<4;i++) assert(!pthread_create(&workers[i],NULL,submit,NULL));
    pause_ms(40); assert(atomic_load(&submitted)==0);
    madeira_gpu_gate_set(&gate,1);
    for(unsigned i=0;i<4;i++) assert(!pthread_join(workers[i],NULL));
    assert(atomic_load(&submitted)==4);
    madeira_gpu_gate_begin(&gate);
    assert(!pthread_create(&closer,NULL,close_gate,NULL));
    while(!atomic_load(&close_started)) pause_ms(1);
    pause_ms(20); assert(!atomic_load(&close_finished));
    madeira_gpu_gate_end(&gate);
    assert(!pthread_join(closer,NULL)); assert(atomic_load(&close_finished));
    assert(!pthread_create(&workers[0],NULL,submit,NULL));
    pause_ms(20); assert(atomic_load(&submitted)==4);
    madeira_gpu_gate_set(&gate,1);
    assert(!pthread_join(workers[0],NULL)); assert(atomic_load(&submitted)==5);

    MadeiraGPUFrameDiagnostics metrics=MADEIRA_GPU_DIAGNOSTICS_INITIALIZER;
    assert(!madeira_gpu_metrics_begin(&metrics));
    madeira_gpu_metrics_enable(&metrics,1);
    uint64_t first=madeira_gpu_metrics_begin(&metrics),second=madeira_gpu_metrics_begin(&metrics);
    madeira_gpu_metrics_complete(&metrics,first,10,10.001,0);
    MadeiraGPUFrameSample s=madeira_gpu_metrics_take(&metrics);
    assert(s.buffers==1 && s.timed_buffers==1 && s.inflight==1 && s.max_inflight==2);
    assert(s.gpu_ns>990000 && s.gpu_ns<1010000);
    madeira_gpu_metrics_complete(&metrics,second,0,0,1);
    s=madeira_gpu_metrics_take(&metrics);
    assert(s.buffers==1 && s.timed_buffers==0 && s.errors==1 && s.inflight==0);
    uint64_t stale=madeira_gpu_metrics_begin(&metrics);
    madeira_gpu_metrics_enable(&metrics,0);
    madeira_gpu_metrics_enable(&metrics,1);
    madeira_gpu_metrics_complete(&metrics,stale,10,10.1,0);
    s=madeira_gpu_metrics_take(&metrics); assert(!s.buffers && !s.inflight);
    uint64_t invalid=madeira_gpu_metrics_begin(&metrics);
    madeira_gpu_metrics_complete(&metrics,invalid,20,19,0);
    s=madeira_gpu_metrics_take(&metrics); assert(s.buffers==1 && !s.timed_buffers && !s.gpu_ns);
    puts("PASS: inactive GPU work blocks, active work resumes, transitions drain submissions and timing rejects failed/stale buffers");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-gpu-gate-') as temp:
    folder=Path(temp);source=folder/'check.c';binary=folder/('check.exe' if os.name=='nt' else 'check')
    source.write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=c11','-pthread','-I',str(root/'app/Madeira'),str(source),'-o',str(binary)],check=True)
    environment=dict(os.environ)
    environment['PATH']=str(Path(compiler).parent)+os.pathsep+environment.get('PATH','')
    subprocess.run([str(binary)],check=True,env=environment,timeout=15)
