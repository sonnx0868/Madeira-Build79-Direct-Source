#pragma once
#include <math.h>
#include <stdint.h>

typedef struct {
    double started;
    uint64_t frames;
    unsigned reports;
} MadeiraSlowFrameWindow;

/* 0: still collecting, 1: completed window, 2: completed slow window.
 * Explicit 30 FPS and no-present stalls are handled by their own policies. */
static inline int madeira_slow_frame_window(MadeiraSlowFrameWindow *window,
                                            double now, uint64_t frames, int mode,
                                            double *seconds, double *fps) {
    if (!isfinite(now) || now < window->started || frames < window->frames) {
        window->started = isfinite(now) ? now : 0;
        window->frames = frames;
        return 0;
    }
    const double elapsed = now - window->started;
    if (elapsed < 5) return 0;
    *seconds = elapsed;
    *fps = (double)(frames - window->frames) / elapsed;
    window->started = now;
    window->frames = frames;
    if (mode == 3 || mode < 0 || mode > 3 || *fps < 1 || *fps >= 50 || window->reports >= 12)
        return 1;
    ++window->reports;
    return 2;
}
