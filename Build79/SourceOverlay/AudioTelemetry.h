#ifndef MADEIRA_AUDIO_TELEMETRY_H
#define MADEIRA_AUDIO_TELEMETRY_H
#include <stdatomic.h>
/* Diagnostics are session-only and opt-in. No locks on the audio thread. */
static _Atomic int madeira_audio_telemetry;
static inline int MadeiraAudioTelemetryEnabled(void) {
    return atomic_load_explicit(&madeira_audio_telemetry,memory_order_relaxed);
}
static inline void MadeiraAudioTelemetrySet(int enabled) {
    atomic_store_explicit(&madeira_audio_telemetry,enabled!=0,memory_order_relaxed);
}
#endif
