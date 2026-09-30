#include "AudioMeter.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    uint8_t pcm[]={0,0,255,127,0,128,192,3,64,252,99};
    MadeiraSampleStats s=MadeiraMeasureSamples(pcm,sizeof(pcm),0);
    assert(s.samples==5&&s.zero==1&&s.near_full_scale==2&&s.peak==32768&&s.nonfinite==0);
    uint8_t fp[]={0,0,0,0,0,0,0,128,0,0,0,63,0,0,128,191,0,0,192,127,0,0,128,127,99};
    s=MadeiraMeasureSamples(fp,sizeof(fp),1);
    assert(s.samples==6&&s.zero==2&&s.near_full_scale==1&&s.nonfinite==2&&s.peak==0x3f800000);
    assert(MadeiraMeasureSamples(NULL,999,1).samples==0);
    puts("Audio meter: PCM16/float32, signed zero, NaN/Inf, trailing bytes PASS");
}
