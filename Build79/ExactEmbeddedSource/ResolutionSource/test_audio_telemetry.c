#include "AudioTelemetry.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(!MadeiraAudioTelemetryEnabled());
    MadeiraAudioTelemetrySet(1); assert(MadeiraAudioTelemetryEnabled());
    MadeiraAudioTelemetrySet(0); assert(!MadeiraAudioTelemetryEnabled());
    MadeiraAudioTelemetrySet(-3); assert(MadeiraAudioTelemetryEnabled());
    MadeiraAudioTelemetrySet(0); assert(!MadeiraAudioTelemetryEnabled());
    puts("Audio telemetry: default off and reversible session toggle PASS");
}
