#ifndef MADEIRA_AUDIO_METER_H
#define MADEIRA_AUDIO_METER_H
#include <stdint.h>
#include <stddef.h>
typedef struct { uint64_t samples, zero, near_full_scale, nonfinite; uint32_t peak; } MadeiraSampleStats;
/* Inspect without changing samples. Float peak is positive IEEE754 bits;
 * PCM16 peak is an integer. Full-scale counts are evidence, not a verdict. */
static inline MadeiraSampleStats MadeiraMeasureSamples(const void *data, size_t bytes, int floating) {
    MadeiraSampleStats s={0};
    const unsigned char *p=data;
    size_t step=floating?4:2;
    if (!p) return s;
    for (size_t i=0;i+step<=bytes;i+=step) {
        uint32_t a;
        if (floating) {
            a=((uint32_t)p[i]|((uint32_t)p[i+1]<<8)|((uint32_t)p[i+2]<<16)|((uint32_t)p[i+3]<<24))&0x7fffffff;
            if (a>=0x7f800000) { s.nonfinite++; s.samples++; continue; }
            if (a>=0x3f800000) s.near_full_scale++;
        } else {
            int32_t v=(int16_t)((uint16_t)p[i]|((uint16_t)p[i+1]<<8));
            a=(uint32_t)(v<0?-v:v);
            if (a>=32760) s.near_full_scale++;
        }
        if (!a) s.zero++;
        if (a>s.peak) s.peak=a;
        s.samples++;
    }
    return s;
}
#endif
