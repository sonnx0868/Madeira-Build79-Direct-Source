#ifndef MADEIRA_CONTROLLER_VALUES_H
#define MADEIRA_CONTROLLER_VALUES_H
#include "ControllerProtocol.h"

static inline float mc_clamp(float value, float low, float high) {
    if (!__builtin_isfinite(value)) return 0.0f;
    return value < low ? low : (value > high ? high : value);
}
static inline int16_t mc_axis(float value) {
    value = mc_clamp(value, -1.0f, 1.0f);
    return (int16_t)(value < 0 ? value * 32768.0f - 0.5f : value * 32767.0f + 0.5f);
}
static inline uint32_t mc_stick(float x, float y, float deadzone) {
    x = mc_clamp(x, -1.0f, 1.0f);
    y = mc_clamp(y, -1.0f, 1.0f);
    deadzone = mc_clamp(deadzone, 0.0f, 0.95f);
    if (deadzone > 0.0f) {
        float magnitude = __builtin_sqrtf(x * x + y * y);
        if (magnitude <= deadzone) x = y = 0.0f;
        else {
            float output = (mc_clamp(magnitude, 0.0f, 1.0f) - deadzone) / (1.0f - deadzone);
            x *= output / magnitude;
            y *= output / magnitude;
        }
    }
    return (uint32_t)(uint16_t)mc_axis(x) | ((uint32_t)(uint16_t)mc_axis(y) << 16);
}
static inline uint8_t mc_trigger(float value, float deadzone) {
    value = mc_clamp(value, 0.0f, 1.0f);
    deadzone = mc_clamp(deadzone, 0.0f, 0.95f);
    if (value <= deadzone) return 0;
    return (uint8_t)(((value - deadzone) / (1.0f - deadzone)) * 255.0f + 0.5f);
}
static inline int mc_slot_changed(const MCSlot *a, const MCSlot *b) {
    return a->flags != b->flags || a->buttons != b->buttons ||
           a->triggers != b->triggers || a->left_axes != b->left_axes ||
           a->right_axes != b->right_axes;
}
/* Menus/Off neutralize input without pretending the physical device unplugged.
 * Games that enumerate once must retain their controller across dialogs. */
static inline MCSlot mc_slot_gated(MCSlot slot, int input_enabled) {
    if (!input_enabled) slot = (MCSlot){.flags = slot.flags, .packet = slot.packet};
    return slot;
}
/* Reserve only XInput slot zero as an app-managed virtual pad. Games that
 * enumerate at startup can keep polling it while physical controllers attach
 * or detach later. Never reserve additional pads or bypass inactive/stale input. */
static inline int mc_slot_present(unsigned index, int active, int physical, int touch) {
    return index < MC_SLOT_COUNT && active && (index == 0 || physical || touch);
}
static inline void mc_merge_touch(MCSlot *slot, uint32_t buttons,
                                  uint8_t left_trigger, uint8_t right_trigger) {
    slot->buttons |= buttons & MC_TOUCH_BUTTON_MASK;
    uint8_t left = (uint8_t)slot->triggers;
    uint8_t right = (uint8_t)(slot->triggers >> 8);
    if (left_trigger > left) left = left_trigger;
    if (right_trigger > right) right = right_trigger;
    slot->triggers = (uint32_t)left | ((uint32_t)right << 8);
}
typedef struct MCTouchState {
    uint32_t buttons;
    uint8_t left_trigger, right_trigger;
    float lx, ly;
    int connected;
} MCTouchState;

static inline void mc_touch_reset(MCTouchState *touch) {
    *touch = (MCTouchState){0};
}
static inline void mc_touch_gate(MCTouchState *touch, int enabled) {
    if (!enabled) mc_touch_reset(touch);
}
static inline void mc_touch_set(MCTouchState *touch, uint32_t buttons,
                                uint8_t lt, uint8_t rt, float lx, float ly,
                                int connected, int enabled) {
    *touch = (MCTouchState){.buttons = buttons & MC_TOUCH_BUTTON_MASK,
        .left_trigger = lt, .right_trigger = rt,
        .lx = mc_clamp(lx, -1, 1), .ly = mc_clamp(ly, -1, 1),
        .connected = !!connected};
    mc_touch_gate(touch, enabled && connected);
}
/* Compare in wire units. Promote before negation so -32768 is safe. */
static inline int16_t mc_touch_axis_merge(int16_t physical, int16_t touch) {
    int p = physical, t = touch;
    int pa = p < 0 ? -p : p, ta = t < 0 ? -t : t;
    return ta > pa ? touch : physical;
}
static inline void mc_merge_touch_with_stick(MCSlot *slot, uint32_t buttons,
                                            uint8_t lt, uint8_t rt,
                                            float lx, float ly) {
    mc_merge_touch(slot, buttons, lt, rt);
    int16_t x = mc_axis(lx), y = mc_axis(ly);
    if (!x && !y) return; /* Neutral touch never replaces physical axes. */
    x = mc_touch_axis_merge((int16_t)(uint16_t)slot->left_axes, x);
    y = mc_touch_axis_merge((int16_t)(uint16_t)(slot->left_axes >> 16), y);
    slot->left_axes = (uint32_t)(uint16_t)x | ((uint32_t)(uint16_t)y << 16);
}
#endif
