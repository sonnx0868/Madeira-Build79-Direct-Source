/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef MADEIRA_INPUT_SURFACE_STATE_H
#define MADEIRA_INPUT_SURFACE_STATE_H
#include <stdint.h>

struct madeira_input_surface {
    uintptr_t window;
    int x, y, width, height;
    int valid;
};
static inline void madeira_input_surface_select(struct madeira_input_surface *s, uintptr_t window) {
    if (s->window != window) *s = (struct madeira_input_surface){ .window = window };
}
static inline int madeira_input_surface_store(struct madeira_input_surface *s, uintptr_t window,
                                               int x, int y, int width, int height) {
    if (!window || window != s->window) return 0;
    int valid = width > 0 && height > 0;
    int changed = s->valid != valid || s->x != x || s->y != y || s->width != width || s->height != height;
    s->x=x; s->y=y; s->width=width; s->height=height; s->valid=valid;
    return changed;
}
#endif
