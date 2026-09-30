#ifndef MADEIRA_AUDIO_PERIOD_H
#define MADEIRA_AUDIO_PERIOD_H
#include <stdint.h>
/* Round upwards so integer frame conversion cannot shorten a device cycle.
 * Zero means invalid/unavailable: retain the original driver result. */
static inline int64_t MadeiraAudioPeriodTicks(double seconds) {
    if(!(seconds>=0.0001&&seconds<=0.2))return 0;
    double ticks=seconds*10000000.0;
    int64_t value=(int64_t)ticks;
    return value+((double)value<ticks);
}
#endif
