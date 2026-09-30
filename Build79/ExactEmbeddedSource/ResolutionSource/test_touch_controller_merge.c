#include "ControllerInput.h"
#include "ControllerValues.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void assert_neutral(MCTouchState t) {
    assert(!t.buttons && !t.left_trigger && !t.right_trigger);
    assert(t.lx == 0 && t.ly == 0 && !t.connected);
}

int main(void) {
    const MCSlot physical = {.flags = MC_CONNECTED, .packet = 7,
        .buttons = MC_BUTTON_A, .triggers = 200u | (30u << 8),
        .left_axes = 0, .right_axes = 0x80007fffu};
    MCSlot slot = physical;
    slot.left_axes = mc_stick(.8f, -.2f, 0);
    const MCSlot baseline = slot;
    mc_merge_touch_with_stick(&slot, 0, 0, 0, 0, 0);
    assert(!memcmp(&slot, &baseline, sizeof(slot)));
    mc_merge_touch_with_stick(&slot, MC_BUTTON_B | 0x0800u, 100, 240, -.4f, .9f);
    assert(slot.buttons == (MC_BUTTON_A | MC_BUTTON_B));
    assert(slot.triggers == (200u | (240u << 8)));
    assert((int16_t)slot.left_axes == mc_axis(.8f));
    assert((int16_t)(slot.left_axes >> 16) == mc_axis(.9f));
    assert(slot.right_axes == physical.right_axes);
    assert(slot.flags == physical.flags && slot.packet == physical.packet);

    slot = baseline;
    mc_merge_touch_with_stick(&slot, 0, 0, 0, NAN, INFINITY);
    assert(!memcmp(&slot, &baseline, sizeof(slot)));
    slot = physical;
    mc_merge_touch_with_stick(&slot, 0, 0, 0, -INFINITY, .5f);
    assert(slot.left_axes == mc_stick(0, .5f, 0));
    slot = physical;
    mc_merge_touch_with_stick(&slot, 0, 0, 0, 4, -4);
    assert(slot.left_axes == mc_stick(1, -1, 0));
    assert(mc_touch_axis_merge(100, -100) == 100);
    assert(mc_touch_axis_merge(-100, 100) == -100);
    assert(mc_touch_axis_merge(-32768, 32767) == -32768);
    assert(mc_touch_axis_merge(32767, -32768) == -32768);

    /* The old merge keeps its original buttons/triggers-only behavior. */
    slot = baseline;
    mc_merge_touch(&slot, MC_BUTTON_X, 255, 0);
    assert(slot.left_axes == baseline.left_axes);
    assert(slot.buttons == (MC_BUTTON_A | MC_BUTTON_X));
    assert(slot.triggers == (255u | (30u << 8)));

    MCTouchState touch = {0};
    mc_touch_set(&touch, UINT32_MAX, 10, 20, NAN, -2, 1, 1);
    assert(touch.buttons == MC_TOUCH_BUTTON_MASK);
    assert(touch.lx == 0 && touch.ly == -1 && touch.connected);
    mc_touch_gate(&touch, 1);
    assert(touch.connected && touch.ly == -1);
    mc_touch_gate(&touch, 0); /* InputEnabled(0) / app resign. */
    assert_neutral(touch);
    mc_touch_gate(&touch, 1); /* Resume cannot resurrect touch. */
    assert_neutral(touch);
    mc_touch_set(&touch, MC_BUTTON_Y, 255, 255, 1, 1, 1, 0);
    assert_neutral(touch); /* Updates while gated are discarded. */
    mc_touch_set(&touch, MC_BUTTON_Y, 255, 255, 1, 1, 0, 1);
    assert_neutral(touch); /* Disconnect clears every contribution. */
    mc_touch_set(&touch, MC_BUTTON_Y, 255, 255, 1, 1, 1, 1);
    slot = baseline;
    mc_merge_touch_with_stick(&slot, touch.buttons, touch.left_trigger,
        touch.right_trigger, touch.lx, touch.ly);
    assert(slot.buttons == (MC_BUTTON_A | MC_BUTTON_Y));
    assert(slot.left_axes == mc_stick(1, 1, 0));
    mc_touch_gate(&touch, 0);
    mc_touch_gate(&touch, 1);
    /* Publish reconstructs physical input, not the formerly merged snapshot. */
    slot = baseline;
    if (touch.connected)
        mc_merge_touch_with_stick(&slot, touch.buttons, touch.left_trigger,
            touch.right_trigger, touch.lx, touch.ly);
    assert(!memcmp(&slot, &baseline, sizeof(slot)));
    mc_touch_set(&touch, MC_BUTTON_Y, 255, 255, 1, 1, 1, 1);
    mc_touch_set(&touch, MC_BUTTON_B, 0, 0, 0, 0, 1, 1);
    assert(!touch.lx && !touch.ly && touch.buttons == MC_BUTTON_B);
    mc_touch_reset(&touch); /* Stop, including before mapping exists. */
    assert_neutral(touch);
    MCSlot gated = mc_slot_gated(baseline, 0);
    assert(gated.flags == baseline.flags && gated.packet == baseline.packet);
    assert(!gated.buttons && !gated.triggers && !gated.left_axes && !gated.right_axes);
    puts("PASS: touch union/max, neutral/nonfinite/clamped left stick, physical ties, gate/reset");
    return 0;
}
