#ifndef MADEIRA_WINDOW_LAYERS_H
#define MADEIRA_WINDOW_LAYERS_H
typedef struct { double pad, controls, toolbar; } MadeiraWindowLayers;
/* Recompute from the game, never from a previous overlay level: no drift.
 * Preserve the original levels for a normal-level game window. */
static inline int MadeiraWindowLayersForGame(double game, MadeiraWindowLayers *out) {
    if (!out || !__builtin_isfinite(game)) return 0;
    out->pad = game + 1.0 > 100.0 ? game + 1.0 : 100.0;
    out->controls = out->pad + 1.0 > 101.0 ? out->pad + 1.0 : 101.0;
    out->toolbar = out->controls + 1.0 > 110.0 ? out->controls + 1.0 : 110.0;
    return __builtin_isfinite(out->toolbar) && game < out->pad &&
        out->pad < out->controls && out->controls < out->toolbar;
}
#endif
