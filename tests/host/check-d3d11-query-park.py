#!/usr/bin/env python3
"""Execute actual D3D11 GetData/query-state code against controlled GPU events."""
from pathlib import Path
import importlib.util,os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('query_patch_pipeline',root/'tests/host/check-dxmt-patches.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);sources=m.patched_sources(root)
def block(s,start):
    a=s.index(start);b=s.index('{',a);end=b+1;depth=1
    while depth:depth+=(s[end]=='{')-(s[end]=='}');end+=1
    return s[a:end]
header=sources['src/d3d11/d3d11_query.hpp']
enums=block(header,'enum class QueryState')+';\n'+block(header,'enum class EventState')+';\n'
interface=block(header,'struct MTLD3D11EventQuery :')+';\n'
query=block(header,'template <typename DataType>\nclass MTLD3D11EventQueryImpl')+';\n'
getdata=block(sources['src/d3d11/d3d11_context_imm.cpp'],'  HRESULT\n  STDMETHODCALLTYPE\n  GetData(').replace(' override {',' {')
wait=block(sources['src/dxmt/dxmt_command_queue.hpp'],'  void WaitEventSlice(')
cxx=os.environ.get('CXX') or shutil.which('clang++')
if not cxx:raise SystemExit('clang++ required')
code=r'''
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#define DXMT_IOS 1
#define STDMETHODCALLTYPE
#define TRUE 1
#define FALSE 0
#define ERR(...) ((void)0)
using UINT=unsigned;using UINT64=uint64_t;using BOOL=int;using HRESULT=int;
enum {S_OK=0,S_FALSE=1,E_INVALIDARG=-1,DXGI_ERROR_INVALID_CALL=-2,E_FAIL=-3};
enum {D3D11_QUERY_EVENT,D3D11_QUERY_TIMESTAMP,D3D11_QUERY_TIMESTAMP_DISJOINT,D3D11_QUERY_OCCLUSION,D3D11_QUERY_OCCLUSION_PREDICATE,D3D11_QUERY_PIPELINE_STATISTICS};
enum {D3D11_ASYNC_GETDATA_DONOTFLUSH=1};
struct D3D11_QUERY_DESC {unsigned Query;};
struct D3D11_QUERY_DATA_TIMESTAMP_DISJOINT {uint64_t Frequency;BOOL Disjoint;};
struct D3D11_QUERY_DATA_PIPELINE_STATISTICS {uint64_t dummy;};
struct ID3D11Asynchronous {virtual UINT GetDataSize()=0;virtual ~ID3D11Asynchronous()=default;};
struct ID3D11Query : ID3D11Asynchronous {virtual void GetDesc(D3D11_QUERY_DESC*)=0;};
struct MTLD3D11Device {};
template<class Base>struct MTLD3DQueryBase:Base {
    D3D11_QUERY_DESC desc;
    MTLD3DQueryBase(MTLD3D11Device*,const D3D11_QUERY_DESC *d):desc(*d){}
    void GetDesc(D3D11_QUERY_DESC *d) override {*d=desc;}
};
struct IMTLD3DOcclusionQuery:ID3D11Query {virtual HRESULT GetData(void*)=0;};
namespace Logger {static void info(const char*){}}
''' +enums+interface+'constexpr size_t kEventStallThreshold=64;\n'+query+r'''
struct FakeEvent {
    uint64_t value=0,target=0,timeout=0;unsigned waits=0;bool complete=false;
    uint64_t signaledValue() {return value;}
    bool waitUntilSignaledValue(uint64_t v,uint64_t t) {++waits;target=v;timeout=t;if(complete)value=v;return value>=v;}
};
struct Queue {
    FakeEvent event;
    struct Stats {uint64_t event_stall=0;} stats;
    uint64_t SignaledEventSeqId(){return event.signaledValue();}
    Stats& CurrentFrameStatistics(){return stats;}
''' +wait+r'''
};
struct Context {
    Queue cmd_queue;unsigned flushes=0;
    void Flush(){++flushes;}
''' +getdata+r'''
};
int main() {
    Context c;D3D11_QUERY_DESC desc{D3D11_QUERY_EVENT};
    MTLD3D11EventQueryImpl<BOOL> q(nullptr,&desc);BOOL ready=FALSE;
    assert(c.GetData(nullptr,&ready,sizeof(ready),0)==E_INVALIDARG);
    assert(c.GetData(&q,nullptr,sizeof(ready),0)==E_INVALIDARG);
    assert(c.GetData(&q,&ready,999,0)==E_INVALIDARG);
    assert(c.GetData(&q,&ready,sizeof(ready),0)==DXGI_ERROR_INVALID_CALL);
    q.Issue(7);
    for(unsigned i=0;i<64;++i) assert(c.GetData(&q,&ready,sizeof(ready),D3D11_ASYNC_GETDATA_DONOTFLUSH)==S_FALSE);
    assert(!c.flushes && !c.cmd_queue.event.waits); // ordinary polling stays cheap
    c.cmd_queue.event.complete=true;
    assert(c.GetData(&q,&ready,sizeof(ready),D3D11_ASYNC_GETDATA_DONOTFLUSH)==S_FALSE);
    assert(!ready && c.flushes==1 && c.cmd_queue.event.waits==1);
    assert(c.cmd_queue.event.target==7 && c.cmd_queue.event.timeout==1);
    assert(c.GetData(&q,&ready,sizeof(ready),0)==S_OK && ready); // only a real recheck can return ready
    q.Issue(8);ready=FALSE;c.cmd_queue.event.complete=false;
    unsigned waits=c.cmd_queue.event.waits;
    for(unsigned i=0;i<64;++i) assert(c.GetData(&q,&ready,sizeof(ready),0)==S_FALSE);
    assert(c.cmd_queue.event.waits==waits);
    for(unsigned i=0;i<4;++i) assert(c.GetData(&q,&ready,sizeof(ready),0)==S_FALSE && !ready);
    assert(c.cmd_queue.event.waits==waits+4 && c.cmd_queue.event.target==8 && c.cmd_queue.event.timeout==1);
    c.cmd_queue.event.value=8;
    assert(c.GetData(&q,nullptr,0,0)==S_OK); // status-only query
    waits=c.cmd_queue.event.waits;c.cmd_queue.WaitEventSlice(8);assert(c.cmd_queue.event.waits==waits);
    q.Issue(999);assert(q.PendingEventSeqId()==999 && q.CheckEventState(8)==EventState::Pending);
    puts("PASS: production D3D11 GetData preserves pending/ready/errors, short polls, exact GPU event target, one-ms timeout and query reissue");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-d11-query-') as tmp:
    p=Path(tmp);src=p/'check.cpp';exe=p/'check.exe';src.write_text(code,encoding='utf-8')
    subprocess.run([cxx,'-std=c++20','-O2','-Wall','-Wextra','-Werror',*(['-static'] if os.name=='nt' else []),str(src),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
