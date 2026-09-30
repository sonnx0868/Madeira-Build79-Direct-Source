#import "Platform.h"
#include "AudioMeter.h"
#include "AudioClock.h"
#include "AudioPeriod.h"
#include "AudioTelemetry.h"
#include <stdatomic.h>
#ifndef MADEIRA_AUDIO_DEVICE_CLOCK
#define MADEIRA_AUDIO_DEVICE_CLOCK 0
#endif
#ifndef MADEIRA_AUDIO_PERIOD_TEST
#define MADEIRA_AUDIO_PERIOD_TEST 0
#endif
typedef int32_t OSStatus;
typedef OSStatus (*RenderFn)(void *,uint32_t *,const void *,uint32_t,uint32_t,void *);
typedef struct { RenderFn fn; void *context; } RenderCallback;
typedef struct { uint32_t channels,bytes; void *data; } AudioBuffer;
typedef struct { uint32_t count; AudioBuffer buffers[1]; } BufferList;
typedef struct { double rate; uint32_t id,flags,packet_bytes,packet_frames,frame_bytes,channels,bits,reserved; } StreamFormat;
extern OSStatus AudioUnitSetProperty(void *,uint32_t,uint32_t,uint32_t,const void *,uint32_t);
/* Fixed storage; no allocation, locks, formatting, or Wine calls on RT thread.
 * Slots are not reclaimed, avoiding use-after-free while an old callback exits. */
typedef struct {
    void *unit; RenderCallback original; StreamFormat format;
    _Atomic uint64_t calls,frames,missing,underruns,samples,zeros,full,nonfinite,errors;
    _Atomic uint32_t peak;
    _Atomic(void *) stream;
    MadeiraHardwareClock clock;
    _Atomic uint64_t requests,requested,submissions,submitted,submit960,submit1024,submitOther,producerErrors;
} AudioProbe;
static AudioProbe probes[64];
static unsigned used;
static atomic_flag setup_lock=ATOMIC_FLAG_INIT;
static RenderFn expected_callback;
void MadeiraAudioSetDiagnostics(int enabled) { MadeiraAudioTelemetrySet(enabled); }
static void *resolveAddress(unsigned long address) {
    for(unsigned i=0;i<_dyld_image_count();i++) {
        const char *p=_dyld_get_image_name(i); if(!p)continue;
        const char *base=p; for(;*p;p++)if(*p=='/')base=p+1;
        const char *name="Madeira"; unsigned n=0; while(base[n]&&base[n]==name[n])n++;
        if(n==7&&!base[n])return (void *)(address+(unsigned long)_dyld_get_image_vmaddr_slide(i));
    }
    return 0;
}
static OSStatus measuredRender(void *context,uint32_t *flags,const void *stamp,uint32_t bus,uint32_t frames,void *buffers) {
    AudioProbe *p=context;
    if(!MadeiraAudioTelemetryEnabled()) {
        OSStatus result=p->original.fn(p->original.context,flags,stamp,bus,frames,buffers);
        /* Preserve optional hardware-clock semantics even without telemetry. */
#if MADEIRA_AUDIO_DEVICE_CLOCK
        if(!result)MadeiraClockAdvance(&p->clock,frames);
#endif
        return result;
    }
    /* Exact build-66 callback disassembly confirms play_pos at +0x60.
     * Packaging rejects any other executable. Before/after consumption
     * measures real missing frames, rather than guessing from silent music. */
    _Atomic uint64_t *position=(_Atomic uint64_t *)((unsigned char *)p->original.context+0x60);
    uint64_t before=atomic_load_explicit(position,memory_order_relaxed);
    OSStatus result=p->original.fn(p->original.context,flags,stamp,bus,frames,buffers);
    uint64_t after=atomic_load_explicit(position,memory_order_relaxed);
    if(!result)MadeiraClockAdvance(&p->clock,frames);
    atomic_fetch_add_explicit(&p->calls,1,memory_order_relaxed);
    atomic_fetch_add_explicit(&p->frames,frames,memory_order_relaxed);
    if(result)atomic_fetch_add_explicit(&p->errors,1,memory_order_relaxed);
    if(after>=before&&after-before<frames) {
        atomic_fetch_add_explicit(&p->underruns,1,memory_order_relaxed);
        atomic_fetch_add_explicit(&p->missing,frames-(after-before),memory_order_relaxed);
    }
    BufferList *list=buffers;
    if(!result&&list&&list->count==1&&list->buffers[0].data) {
        size_t bytes=(size_t)frames*p->format.frame_bytes;
        if(bytes>list->buffers[0].bytes)bytes=list->buffers[0].bytes;
        MadeiraSampleStats s=MadeiraMeasureSamples(list->buffers[0].data,bytes,(p->format.flags&1)!=0);
        atomic_fetch_add_explicit(&p->samples,s.samples,memory_order_relaxed);
        atomic_fetch_add_explicit(&p->zeros,s.zero,memory_order_relaxed);
        atomic_fetch_add_explicit(&p->full,s.near_full_scale,memory_order_relaxed);
        atomic_fetch_add_explicit(&p->nonfinite,s.nonfinite,memory_order_relaxed);
        uint32_t previous=atomic_load_explicit(&p->peak,memory_order_relaxed);
        while(previous<s.peak&&!atomic_compare_exchange_weak_explicit(&p->peak,&previous,s.peak,memory_order_relaxed,memory_order_relaxed)){}
    }
    return result;
}
#if MADEIRA_AUDIO_DEVICE_CLOCK || MADEIRA_AUDIO_PERIOD_TEST
typedef int32_t (*UnixFn)(void *);
typedef struct { uint64_t stream; int32_t device,result; uint64_t *position,*qpc; } PositionParams;
typedef struct { uint64_t stream; int32_t result; } HandleParams;
typedef struct { uint64_t stream; void *timer; int32_t result; } ReleaseParams;
_Static_assert(sizeof(PositionParams)==32&&offsetof(PositionParams,position)==16,"Wine position ABI");
static UnixFn old_release;
#if MADEIRA_AUDIO_DEVICE_CLOCK
static UnixFn old_position,old_reset;
#endif
static AudioProbe *probeForStream(uint64_t stream) {
    /* Reverse scan handles malloc-address reuse without retaining a freed
     * stream: original calls validate the handle before we use the clock. */
    for(unsigned i=64;i>0;i--)if(atomic_load_explicit(&probes[i-1].stream,memory_order_acquire)==(void *)(uintptr_t)stream)return &probes[i-1];
    return 0;
}
#if MADEIRA_AUDIO_DEVICE_CLOCK
static int32_t hardwarePosition(void *args) {
    int32_t status=old_position(args); PositionParams *p=args;
    if(!status&&p->result>=0&&p->position) {
        AudioProbe *probe=probeForStream(p->stream);
        if(probe)*p->position=MadeiraClockRead(&probe->clock);
    }
    return status;
}
static int32_t hardwareReset(void *args) {
    int32_t status=old_reset(args); HandleParams *p=args;
    if(!status&&p->result>=0) {
        AudioProbe *probe=probeForStream(p->stream);
        if(probe)MadeiraClockReset(&probe->clock);
    }
    return status;
}
#endif
static int32_t hardwareRelease(void *args) {
    ReleaseParams *p=args; AudioProbe *probe=probeForStream(p->stream);
    int32_t status=old_release(args);
    /* The original joins the timer and disposes its AudioUnit first. Retire
     * the identity, not callback storage; uninstrumented pointer reuse must
     * never inherit the previous stream's clock. */
    if(!status&&p->result>=0&&probe)atomic_store_explicit(&probe->stream,0,memory_order_release);
    return status;
}
#if MADEIRA_AUDIO_DEVICE_CLOCK
static void installHardwareClock(void) {
    if(old_position)return;
    /* Exact SHA guard plus nm/section inspection: this table is in writable
     * __DATA,__data. No code patch, mprotect, global hook or host lookup. */
    UnixFn *table=resolveAddress(0x100c1b1c0UL);
    UnixFn position=resolveAddress(0x1000bdb68UL),reset=resolveAddress(0x1000bd3c8UL);
    UnixFn release=resolveAddress(0x1000bce74UL);
    if(!table||table[23]!=position||table[8]!=reset||table[5]!=release) { printf("[audio-clock v1] table guard failed; unchanged\n"); return; }
    old_position=position; old_reset=reset; old_release=release;
    table[23]=hardwarePosition; table[8]=hardwareReset; table[5]=hardwareRelease;
    printf("[audio-clock v1] hardware position enabled; ring/padding unchanged\n");
}
#endif
#if MADEIRA_AUDIO_PERIOD_TEST
@interface AVAudioSession : NSObject
+ (instancetype)sharedInstance;
@property(readonly) double IOBufferDuration;
@end
typedef struct { const char *device; int32_t flow,result; int64_t *normal,*minimum; } PeriodParams;
typedef struct { uint64_t stream; uint32_t frames; int32_t result; void **data; } RequestParams;
typedef struct { uint64_t stream; uint32_t frames,flags; int32_t result; } SubmitParams;
typedef struct { uint64_t stream; int32_t result; uint32_t *frames; } SizeParams;
_Static_assert(sizeof(PeriodParams)==32&&offsetof(PeriodParams,normal)==16,"Wine period ABI");
_Static_assert(sizeof(RequestParams)==24&&offsetof(RequestParams,data)==16,"Wine render request ABI");
_Static_assert(offsetof(SubmitParams,result)==16&&offsetof(SizeParams,frames)==16,"Wine render submission/size ABI");
static UnixFn old_period,old_request,old_submit,old_size;
static int32_t actualDevicePeriod(void *args) {
    int32_t status=old_period(args); PeriodParams *p=args;
    if(!status&&p->result>=0&&p->flow==0) {
        AVAudioSession *session=[AVAudioSession sharedInstance];
        double seconds=0;
        if(session&&[session respondsToSelector:@selector(IOBufferDuration)])
            seconds=session.IOBufferDuration;
        int64_t ticks=MadeiraAudioPeriodTicks(seconds);
        if(ticks) {
            if(p->normal)*p->normal=ticks;
            if(p->minimum)*p->minimum=ticks;
            printf("[audio-period v1] actual=%.6f seconds ticks=%lld\n",seconds,(long long)ticks);
        } else printf("[audio-period v1] unavailable; original period retained\n");
    }
    return status;
}
static int32_t producerRequest(void *args) {
    int32_t status=old_request(args); RequestParams *p=args;
    if(!MadeiraAudioTelemetryEnabled())return status;
    AudioProbe *probe=probeForStream(p->stream);
    if(probe) {
        if(status||p->result<0)atomic_fetch_add_explicit(&probe->producerErrors,1,memory_order_relaxed);
        else { atomic_fetch_add_explicit(&probe->requests,1,memory_order_relaxed); atomic_fetch_add_explicit(&probe->requested,p->frames,memory_order_relaxed); }
    }
    return status;
}
static int32_t producerSubmit(void *args) {
    int32_t status=old_submit(args); SubmitParams *p=args;
    if(!MadeiraAudioTelemetryEnabled())return status;
    AudioProbe *probe=probeForStream(p->stream);
    if(probe) {
        if(status||p->result<0)atomic_fetch_add_explicit(&probe->producerErrors,1,memory_order_relaxed);
        else if(p->frames) {
            atomic_fetch_add_explicit(&probe->submissions,1,memory_order_relaxed);
            atomic_fetch_add_explicit(&probe->submitted,p->frames,memory_order_relaxed);
            _Atomic uint64_t *hist=p->frames==960?&probe->submit960:p->frames==1024?&probe->submit1024:&probe->submitOther;
            atomic_fetch_add_explicit(hist,1,memory_order_relaxed);
        }
    }
    return status;
}
static int32_t producerSize(void *args) {
    int32_t status=old_size(args); SizeParams *p=args;
    if(!status&&p->result>=0&&p->frames)printf("[audio-period v1] stream=0x%llx buffer_frames=%u\n",(unsigned long long)p->stream,*p->frames);
    return status;
}
static void installActualPeriod(void) {
    if(old_period)return;
    UnixFn *table=resolveAddress(0x100c1b1c0UL);
    UnixFn period=resolveAddress(0x1000bd934UL),request=resolveAddress(0x1000bd4f4UL),submit=resolveAddress(0x1000bd668UL),size=resolveAddress(0x1000bd964UL),release=resolveAddress(0x1000bce74UL);
    if(!table||table[17]!=period||table[10]!=request||table[11]!=submit||table[18]!=size||table[5]!=release) { printf("[audio-period v1] table guard failed; unchanged\n"); return; }
    old_period=period; old_request=request; old_submit=submit; old_size=size; old_release=release;
    table[17]=actualDevicePeriod; table[10]=producerRequest; table[11]=producerSubmit; table[18]=producerSize; table[5]=hardwareRelease;
    printf("[audio-period v1] actual period enabled; original clock/ring retained\n");
}
#endif
#endif
/* Only Madeira's main image imports this alias; not a global interpose. */
OSStatus MadeiraAudioProperty(void *unit,uint32_t property,uint32_t scope,uint32_t element,const void *data,uint32_t size) {
    if(scope!=1||element!=0||!data||(property!=8&&property!=23))return AudioUnitSetProperty(unit,property,scope,element,data,size);
    while(atomic_flag_test_and_set_explicit(&setup_lock,memory_order_acquire)){}
    if(!expected_callback)expected_callback=(RenderFn)resolveAddress(0x1000bdf84UL);
#if MADEIRA_AUDIO_DEVICE_CLOCK
    installHardwareClock();
#endif
#if MADEIRA_AUDIO_PERIOD_TEST
    installActualPeriod();
#endif
    AudioProbe *p=0;
    for(unsigned i=used;i>0;i--)if(probes[i-1].unit==unit){p=&probes[i-1];break;}
    OSStatus result;
    if(property==8) {
        result=AudioUnitSetProperty(unit,property,scope,element,data,size);
        if(!result&&size==sizeof(StreamFormat)&&used<64) {
            p=&probes[used++]; p->unit=unit; p->format=*(const StreamFormat *)data;
        }
    } else {
        const RenderCallback *cb=data;
        int supported=p&&((p->format.flags&1)?p->format.bits==32:p->format.bits==16)&&p->format.frame_bytes==p->format.channels*(p->format.bits/8);
        if(size==sizeof(*cb)&&supported&&expected_callback&&cb->fn==expected_callback&&cb->context&&!p->original.fn) {
            p->original=*cb; RenderCallback replacement={measuredRender,p};
            result=AudioUnitSetProperty(unit,property,scope,element,&replacement,sizeof(replacement));
            if(result)p->original=(RenderCallback){0};
            else atomic_store_explicit(&p->stream,cb->context,memory_order_release);
        } else result=AudioUnitSetProperty(unit,property,scope,element,data,size);
    }
    atomic_flag_clear_explicit(&setup_lock,memory_order_release);
    return result;
}
__attribute__((constructor)) static void startAudioReports(void) {
#if MADEIRA_AUDIO_PERIOD_TEST
    /* Install before Wine's initial GetDevicePeriod, not after first stream. */
    installActualPeriod();
#endif
    dispatch_async(&_dispatch_main_q, ^{
        [NSTimer scheduledTimerWithTimeInterval:5 repeats:YES block:^(NSTimer *timer){
            (void)timer;
            if(!MadeiraAudioTelemetryEnabled())return;
            for(unsigned i=0;i<64;i++) {
                AudioProbe *p=&probes[i];
                uint64_t calls=atomic_exchange_explicit(&p->calls,0,memory_order_relaxed);
                if(!calls)continue;
#define TAKE(field) ((unsigned long long)atomic_exchange_explicit(&p->field,0,memory_order_relaxed))
                printf("[audio-probe v1] slot=%u calls=%llu frames=%llu underruns=%llu missing=%llu samples=%llu zero=%llu full=%llu nonfinite=%llu peak=%u errors=%llu\n",i,(unsigned long long)calls,TAKE(frames),TAKE(underruns),TAKE(missing),TAKE(samples),TAKE(zeros),TAKE(full),TAKE(nonfinite),atomic_exchange_explicit(&p->peak,0,memory_order_relaxed),TAKE(errors));
#if MADEIRA_AUDIO_PERIOD_TEST
                printf("[audio-producer v1] slot=%u requests=%llu requested=%llu submissions=%llu submitted=%llu write960=%llu write1024=%llu other=%llu errors=%llu\n",i,TAKE(requests),TAKE(requested),TAKE(submissions),TAKE(submitted),TAKE(submit960),TAKE(submit1024),TAKE(submitOther),TAKE(producerErrors));
#endif
#undef TAKE
            }
        }];
    });
}
