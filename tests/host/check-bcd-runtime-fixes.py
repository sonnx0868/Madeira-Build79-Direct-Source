#!/usr/bin/env python3
"""Exercise the integrated production cancel walker and counter-copy method.

Real freed-page/null faults reproduce both previous failures. Server callbacks
and Metal encoding are controlled boundaries; no device FPS claim is made.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
def function(text,signature):
    start=text.index(signature);brace=text.index('{',start);depth=0
    for end in range(brace,len(text)):
        if text[end]=='{': depth+=1
        elif text[end]=='}':
            depth-=1
            if not depth:return text[start:end+1]
    raise AssertionError(signature)
fixed=(root/'wine/server/async.c').read_text(encoding='utf-8')
old=subprocess.check_output(['git','-C',str(root/'wine'),'show','HEAD:server/async.c']).decode('utf-8')
assert 'madeira-bcd async-cancel-keepalive' in fixed
c=r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wine/list.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#endif
typedef uintptr_t client_ptr_t;
typedef unsigned obj_handle_t;
#ifndef SYNCHRONIZE
#define SYNCHRONIZE 1
#endif
struct object { unsigned refs,kind; };
struct thread { int unused; };
struct fd { struct object *user; };
struct process { struct list asyncs; };
struct async_cancel { struct object obj; unsigned count; };
struct async {
 struct object obj;struct list process_entry;struct fd *fd;struct thread *thread;
 struct { client_ptr_t iosb; } data;
 unsigned canceled,terminated,is_system,completed,immediate;
 struct async_cancel *async_cancel;
};
static unsigned destroyed,cancel_freed;
static struct async_cancel *group;
static void release_object(void *ptr) {
 struct object *obj=ptr;assert(obj->refs);if(--obj->refs)return;
 if(obj->kind==2) { ++cancel_freed;free(obj);return; }
 struct async *a=ptr;list_remove(&a->process_entry);++destroyed;
#ifdef _WIN32
 DWORD old;assert(VirtualProtect(a,0x4000,PAGE_NOACCESS,&old));
#else
 assert(!mprotect(a,0x4000,PROT_NONE));
#endif
}
void *grab_object(void *ptr) { struct object *o=ptr;assert(o->refs);++o->refs;return ptr; }
static struct async_cancel *create_async_cancel(struct process *p) {
 (void)p;group=calloc(1,sizeof(*group));assert(group);group->obj=(struct object){1,2};return group;
}
static struct object *get_fd_user(struct fd *f) { return f->user; }
static obj_handle_t alloc_handle(struct process *p,void *o,unsigned r,unsigned f) {
 (void)p;(void)o;(void)r;(void)f;return 77;
}
static void cancel_async(struct async *a) {
 a->canceled=1;a->terminated=1;
 if(a->immediate) { a->completed=1;release_object(a); }
}
FUNCTION
static struct async *make(struct process *p,struct thread *t,int immediate) {
#ifdef _WIN32
 struct async *a=VirtualAlloc(NULL,0x4000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(a);
#else
 struct async *a=mmap(NULL,0x4000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);assert(a!=MAP_FAILED);
#endif
 memset(a,0,sizeof(*a));a->obj=(struct object){1,1};a->thread=t;a->immediate=immediate;
 list_add_tail(&p->asyncs,&a->process_entry);return a;
}
int main(void) {
#ifdef _WIN32
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
#else
 struct rlimit limit={0,0};setrlimit(RLIMIT_CORE,&limit);
#endif
 struct process p;struct thread t={0};obj_handle_t wait=0;list_init(&p.asyncs);
 make(&p,&t,1);
 assert(cancel_process_async(&p,NULL,&t,0,&wait)==1);
 assert(destroyed==1 && cancel_freed==1 && !wait && list_empty(&p.asyncs));
 struct async *a=make(&p,&t,0);wait=0;
 assert(cancel_process_async(&p,NULL,&t,0,&wait)==1);
 assert(wait==77 && group->count==1 && a->obj.refs==1 && !a->completed);
 a->completed=1;assert(--group->count==0);release_object(group);release_object(a);
 assert(destroyed==2 && cancel_freed==2 && list_empty(&p.asyncs));
 make(&p,&t,1);make(&p,&t,1);wait=0;
 assert(cancel_process_async(&p,NULL,NULL,0,&wait)==2);
 assert(destroyed==4 && cancel_freed==2 && list_empty(&p.asyncs));
 puts("PASS: immediate completion stays alive; completed groups do not hang; queued and multi-async cancellation preserved");
 return 0;
}
'''

dx=(root/'dxmt/src/d3d11/d3d11_context_impl.cpp').read_text(encoding='utf-8')
assert 'madeira-bcd: CopyStructureCount without a buffer' in dx
signature='  void\n  STDMETHODCALLTYPE\n  CopyStructureCount('
dx_fixed=function(dx,signature)
# Reconstruct the unchanged previous method around the small imported hunk.
dx_old=dx_fixed
start=dx_old.index('        /* madeira-bcd: CopyStructureCount')
end=dx_old.index('        SwitchToBlitEncoder',start)
dx_old=dx_old[:start]+dx_old[end:]
dx_old=dx_old.replace('dst = std::move(dst_rc), counter = std::move(counter_rc)',
                      'dst = dst_bind->buffer(), counter = uav->counter()')
cpp=r'''
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#endif
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif
#define ERR(...) ((void)0)
using UINT=unsigned;using mutex_t=std::mutex;
enum class CommandBufferState { BlitEncoderActive };
enum { DXMT_ENCODER_RESOURCE_ACESS_WRITE,DXMT_ENCODER_RESOURCE_ACESS_READ,WMTBlitCommandCopyFromBufferToBuffer=7 };
struct Buffer { uintptr_t value; __attribute__((noinline)) void *buffer() const { return (void *)*(volatile const uintptr_t *)&value; } };
struct ID3D11Buffer {};
struct D3D11ResourceCommon:ID3D11Buffer { std::shared_ptr<Buffer> ptr;auto buffer(){return ptr;} };
struct ID3D11UnorderedAccessView {};
struct D3D11UnorderedAccessView:ID3D11UnorderedAccessView { std::shared_ptr<Buffer> ptr;auto counter(){return ptr;} };
struct wmtcmd_blit_copy_from_buffer_to_buffer { int type=0;UINT copy_length=0;void *src=nullptr,*dst=nullptr;uintptr_t src_offset=0,dst_offset=0; };
struct ArgumentEncodingContext {
 wmtcmd_blit_copy_from_buffer_to_buffer command;
 auto access(std::shared_ptr<Buffer> p,UINT offset,UINT length,int mode) {
  assert(length==4);if(mode==DXMT_ENCODER_RESOURCE_ACESS_WRITE)assert(offset==16);else assert(!offset);
  return std::make_pair(p.get(),mode==DXMT_ENCODER_RESOURCE_ACESS_WRITE?128u:32u);
 }
 template<class T> T &encodeBlitCommand(){return command;}
};
struct Interface { virtual void CopyStructureCount(ID3D11Buffer *,UINT,ID3D11UnorderedAccessView *)=0; };
struct Context:Interface {
 mutex_t mutex;ArgumentEncodingContext enc;unsigned switches=0,emits=0;
 void SwitchToBlitEncoder(CommandBufferState){++switches;}
 template<class F> void EmitOP(F &&f){++emits;f(enc);}
FUNCTION
};
int main(int argc,char **argv) {
#ifdef _WIN32
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
#else
 struct rlimit limit={0,0};setrlimit(RLIMIT_CORE,&limit);
#endif
 assert(argc==2);Context c;D3D11ResourceCommon dst;D3D11UnorderedAccessView uav;
 dst.ptr=std::make_shared<Buffer>(Buffer{111});uav.ptr=std::make_shared<Buffer>(Buffer{222});
 if(!strcmp(argv[1],"missing"))uav.ptr.reset();
 c.CopyStructureCount(&dst,16,&uav);
 if(!strcmp(argv[1],"missing")) {
  assert(!c.switches && !c.emits);dst.ptr.reset();c.CopyStructureCount(&dst,16,&uav);
  uav.ptr=std::make_shared<Buffer>(Buffer{222});c.CopyStructureCount(&dst,16,&uav);
  assert(!c.switches && !c.emits);puts("PASS: absent destination/counter does not enqueue a GPU command");
 } else {
  auto &r=c.enc.command;assert(c.switches==1 && c.emits==1 && r.type==7 && r.copy_length==4);
  assert(r.src==(void *)222 && r.dst==(void *)111 && r.src_offset==32 && r.dst_offset==144);
  puts("PASS: valid counter copy preserves command, resource access and offsets");
 }
 return 0;
}
'''
cc=os.environ.get('MD_RUNTIME_CC') or os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc:raise SystemExit('C compiler required')
cxx=os.environ.get('CXX') or str(Path(cc).with_name('clang++.exe' if os.name=='nt' else 'clang++'))
expected={0xc0000005} if os.name=='nt' else {-11,-10}
def memory_fault(code):return code&0xffffffff in {n&0xffffffff for n in expected}
with tempfile.TemporaryDirectory(prefix='madeira-bcd-runtime-') as tmp:
    tmp=Path(tmp)
    (tmp/'wine').mkdir()
    (tmp/'wine/list.h').write_bytes((root/'wine/include/wine/list.h').read_bytes())
    def build(name,text,cpp=False):
        source=tmp/(name+('.cpp' if cpp else '.c'));binary=tmp/(name+('.exe' if os.name=='nt' else ''))
        source.write_text(text,encoding='utf-8')
        flags=['-std=c++17' if cpp else '-std=c11','-O1','-Wall','-Wextra','-Werror','-I'+str(tmp)]
        if os.name=='nt':flags+=['-static']
        subprocess.run([cxx if cpp else cc,*flags,str(source),'-o',str(binary)],check=True)
        return binary
    for name,source in [('old',old),('fixed',fixed)]:
        exe=build('cancel-'+name,c.replace('FUNCTION',function(source,'static int cancel_process_async(')))
        result=subprocess.run([str(exe)],capture_output=True,timeout=10)
        if name=='old':assert memory_fault(result.returncode),result.returncode
        else:
            assert result.returncode==0,(result.returncode,result.stderr)
            print(result.stdout.decode(),end='',flush=True)
    for name,method in [('old',dx_old),('fixed',dx_fixed)]:
        exe=build('counter-'+name,cpp.replace('FUNCTION',method),cpp=True)
        for mode in ['valid','missing']:
            result=subprocess.run([str(exe),mode],capture_output=True,timeout=10)
            if name=='old' and mode=='missing':assert memory_fault(result.returncode),result.returncode
            else:
                assert result.returncode==0,(result.returncode,result.stderr)
                print(result.stdout.decode(),end='',flush=True)
print('PASS: both baseline faults reproduced using production code; both integrated fixes pass',flush=True)
