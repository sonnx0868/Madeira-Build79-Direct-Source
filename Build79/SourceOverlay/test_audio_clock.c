#include "AudioClock.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    MadeiraHardwareClock a={0},b={0}; uint64_t queue_consumed=0;
    assert(MadeiraClockRead(&a)==0);
    for(unsigned i=0;i<234;i++) { MadeiraClockAdvance(&a,1024); queue_consumed+=960; }
    assert(MadeiraClockRead(&a)==239616&&queue_consumed==224640);
    assert(MadeiraClockRead(&a)-queue_consumed==14976);
    /* Stop freezes because AudioOutputUnitStop prevents callbacks; reads alone
     * cannot advance it. Resume continues, reset affects only this stream. */
    uint64_t stopped=MadeiraClockRead(&a);
    for(unsigned i=0;i<100;i++)assert(MadeiraClockRead(&a)==stopped);
    MadeiraClockAdvance(&a,512); assert(MadeiraClockRead(&a)==stopped+512);
    MadeiraClockAdvance(&b,1024); MadeiraClockReset(&a);
    assert(MadeiraClockRead(&a)==0&&MadeiraClockRead(&b)==1024);
    assert(queue_consumed==224640);
    puts("Hardware clock: underrun progression, queue separation, stop/read, resume, independent reset PASS");
}
