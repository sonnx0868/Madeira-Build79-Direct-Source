#include "AudioPeriod.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    assert(MadeiraAudioPeriodTicks(1024.0/48000.0)==213334);
    assert(MadeiraAudioPeriodTicks(512.0/48000.0)==106667);
    assert(MadeiraAudioPeriodTicks(256.0/48000.0)==53334);
    assert(MadeiraAudioPeriodTicks(0.010)==100000);
    assert(MadeiraAudioPeriodTicks(1024.0/44100.0)==232200);
    assert(MadeiraAudioPeriodTicks(0)==0&&MadeiraAudioPeriodTicks(-1)==0);
    assert(MadeiraAudioPeriodTicks(NAN)==0&&MadeiraAudioPeriodTicks(INFINITY)==0);
    assert(MadeiraAudioPeriodTicks(10)==0);
    puts("Audio period: actual hardware durations, upward rounding, invalid-value fallback PASS");
}
