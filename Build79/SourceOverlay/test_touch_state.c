/* Host: cc -std=c99 -Wall -Wextra -Wpedantic -Werror
 *       -fsanitize=address,undefined -fno-omit-frame-pointer
 *       test_touch_state.c -lm -o /tmp/test_touch_state
 */
#include "TouchState.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#define ACCEL UINT32_C(1)
#define STEER UINT32_C(2)
#define BRAKE UINT32_C(0x80000000)

static void near_value(float actual, float expected)
{
    assert(isfinite(actual));
    assert(fabsf(actual - expected) < 0.00001f);
}

static void neutral(const TouchState *state)
{
    TouchStateOutput out = touch_state_output(state);
    assert(out.buttons == 0);
    near_value(out.left_x, 0.0f);
    near_value(out.left_y, 0.0f);
    assert(out.left_trigger == 0 && out.right_trigger == 0);
}

static void simultaneous_and_capture(void)
{
    TouchState state;
    TouchStateOutput out;
    touch_state_init(&state, 0.2f);
    assert(touch_state_begin(&state, 1, ACCEL, TOUCH_STATE_ROLE_RIGHT_TRIGGER,
                             0, 0, 0.8f) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 2, STEER, TOUCH_STATE_ROLE_LEFT_STICK,
                             0.6f, 0, 0) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    assert(out.buttons == (ACCEL | STEER));
    near_value(out.left_x, 0.5f);
    assert(out.right_trigger == 204);
    /* Outside original button bounds: assignment remains captured. */
    assert(touch_state_move(&state, 1, -10, 10, 1) == TOUCH_STATE_OK);
    assert(touch_state_move(&state, 2, -4, 0, 1) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    assert(out.buttons == (ACCEL | STEER));
    near_value(out.left_x, -1);
    assert(out.left_trigger == 0 && out.right_trigger == 255);
    assert(touch_state_end(&state, 2) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    assert(out.buttons == ACCEL && out.right_trigger == 255);
    near_value(out.left_x, 0);
    near_value(out.left_y, 0);
    assert(touch_state_cancel(&state, 1) == TOUCH_STATE_OK);
    neutral(&state);
}

static void duplicates_shared_buttons_and_cancel(void)
{
    TouchState state;
    TouchStateOutput out;
    touch_state_init(&state, 0);
    assert(touch_state_begin(&state, UINTPTR_MAX, ACCEL | BRAKE,
                             TOUCH_STATE_ROLE_NONE, 0, 0, 0) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, UINTPTR_MAX, STEER,
                             TOUCH_STATE_ROLE_LEFT_STICK, 1, 1, 1) == TOUCH_STATE_DUPLICATE);
    assert(state.count == 1);
    out = touch_state_output(&state);
    assert(out.buttons == (ACCEL | BRAKE));
    near_value(out.left_x, 0);
    assert(touch_state_begin(&state, 2, ACCEL, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 3, STEER, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == TOUCH_STATE_OK);
    assert(touch_state_cancel(&state, UINTPTR_MAX) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == (ACCEL | STEER));
    assert(touch_state_end(&state, UINTPTR_MAX) == TOUCH_STATE_UNKNOWN);
    assert(touch_state_cancel(&state, UINTPTR_MAX) == TOUCH_STATE_UNKNOWN);
    assert(touch_state_output(&state).buttons == (ACCEL | STEER));
    assert(touch_state_end(&state, 2) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == STEER);
    assert(touch_state_cancel(&state, 3) == TOUCH_STATE_OK);
    neutral(&state);
}

static void analog_arbitration(void)
{
    TouchState state;
    TouchStateOutput out;
    touch_state_init(&state, 0);
    assert(touch_state_begin(&state, 1, 0, TOUCH_STATE_ROLE_LEFT_TRIGGER,
                             0, 0, 0.25f) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 2, 0, TOUCH_STATE_ROLE_LEFT_TRIGGER,
                             0, 0, 0.75f) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 3, 0, TOUCH_STATE_ROLE_RIGHT_TRIGGER,
                             0, 0, 0.5f) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    assert(out.left_trigger == 191 && out.right_trigger == 128);
    assert(touch_state_begin(&state, 6, 0, TOUCH_STATE_ROLE_RIGHT_TRIGGER,
                             0, 0, 1) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).right_trigger == 255);
    assert(touch_state_move(&state, 6, 0, 0, 0.25f) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).right_trigger == 128);
    assert(touch_state_cancel(&state, 6) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).right_trigger == 128);
    assert(touch_state_move(&state, 1, 0, 0, 1) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).left_trigger == 255);
    assert(touch_state_cancel(&state, 1) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).left_trigger == 191);
    assert(touch_state_move(&state, 2, 0, 0, 0.1f) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).left_trigger == 26);
    assert(touch_state_begin(&state, 4, 0, TOUCH_STATE_ROLE_LEFT_STICK,
                             1, 0, 0) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 5, 0, TOUCH_STATE_ROLE_LEFT_STICK,
                             0, -1, 0) == TOUCH_STATE_OK);
    assert(touch_state_move(&state, 4, -1, 0, 0) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    near_value(out.left_x, 0);
    near_value(out.left_y, -1);
    assert(touch_state_cancel(&state, 5) == TOUCH_STATE_OK);
    near_value(touch_state_output(&state).left_x, -1);
    assert(touch_state_end(&state, 4) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    near_value(out.left_x, 0);
    near_value(out.left_y, 0);
    assert(out.left_trigger == 26 && out.right_trigger == 128);
    assert(touch_state_end(&state, 2) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).left_trigger == 0);
    assert(touch_state_end(&state, 3) == TOUCH_STATE_OK);
    neutral(&state);
}

static void dynamic_masks_and_union_collisions(void)
{
    TouchState state;
    TouchStateOutput out;
    touch_state_init(&state, 0);
    assert(!touch_state_is_active(&state, 1));
    assert(touch_state_begin(&state, 1, ACCEL | STEER,
                             TOUCH_STATE_ROLE_LEFT_STICK, 0.5f, 0, 0) > 0);
    assert(touch_state_begin(&state, 2, STEER, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) > 0);
    assert(touch_state_is_active(&state, 1));
    assert(touch_state_is_active(&state, 2));
    assert(touch_state_update_buttons(&state, 1, BRAKE) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    /* Old ACCEL released; old STEER still held by a different contact. */
    assert(out.buttons == (BRAKE | STEER));
    near_value(out.left_x, 0.5f);
    assert(state.contacts[0].id == 1);
    assert(state.contacts[0].role == TOUCH_STATE_ROLE_LEFT_STICK);
    assert(touch_state_update_buttons(&state, 2, BRAKE) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == BRAKE);
    assert(touch_state_update_buttons(&state, 1, 0) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == BRAKE);
    assert(touch_state_is_active(&state, 1));
    near_value(touch_state_output(&state).left_x, 0.5f);
    assert(touch_state_end(&state, 2) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == 0);
    assert(!touch_state_is_active(&state, 2));
    assert(touch_state_update_buttons(&state, 2, UINT32_MAX) == TOUCH_STATE_UNKNOWN);
    assert(touch_state_update_buttons(&state, 0, UINT32_MAX) == TOUCH_STATE_INVALID);
    assert(touch_state_update_buttons(NULL, 1, UINT32_MAX) == TOUCH_STATE_INVALID);
    assert(touch_state_update_buttons(&state, 1, UINT32_MAX) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons == UINT32_MAX);
    assert(touch_state_begin(&state, 1, ACCEL, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == 0);
    assert(touch_state_output(&state).buttons == UINT32_MAX);
    assert(touch_state_cancel(&state, 1) == TOUCH_STATE_OK);
    neutral(&state);
    assert(!touch_state_is_active(&state, 1));
    assert(!touch_state_is_active(&state, 0));
    assert(!touch_state_is_active(NULL, 1));
}

static void bounds_deadzone_and_nonfinite(void)
{
    TouchState state;
    TouchStateOutput out;
    touch_state_init(&state, 0.25f);
    assert(touch_state_begin(&state, 1, 0, TOUCH_STATE_ROLE_LEFT_STICK,
                             0.1f, 0.1f, NAN) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    near_value(out.left_x, 0);
    near_value(out.left_y, 0);
    assert(touch_state_move(&state, 1, 0.25f, 0, 0) == TOUCH_STATE_OK);
    near_value(touch_state_output(&state).left_x, 0);
    assert(touch_state_move(&state, 1, 0.625f, 0, 0) == TOUCH_STATE_OK);
    near_value(touch_state_output(&state).left_x, 0.5f);
    assert(touch_state_move(&state, 1, FLT_MAX, -FLT_MAX, 0) == TOUCH_STATE_OK);
    out = touch_state_output(&state);
    near_value(out.left_x, sqrtf(0.5f));
    near_value(out.left_y, -sqrtf(0.5f));
    assert(touch_state_move(&state, 1, NAN, INFINITY, 0) == TOUCH_STATE_OK);
    near_value(touch_state_output(&state).left_x, 0);
    near_value(touch_state_output(&state).left_y, 0);
    assert(touch_state_move(&state, 1, -INFINITY, 0.625f, 0) == TOUCH_STATE_OK);
    near_value(touch_state_output(&state).left_y, 0.5f);
    assert(touch_state_begin(&state, 2, 0, TOUCH_STATE_ROLE_RIGHT_TRIGGER,
                             NAN, INFINITY, FLT_MAX) == TOUCH_STATE_OK);
    assert(touch_state_output(&state).right_trigger == 255);
    {
        const float invalid[] = {NAN, INFINITY, -INFINITY, -FLT_MAX, -0.1f};
        size_t i;
        for (i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
            assert(touch_state_move(&state, 2, 0, 0, invalid[i]) == TOUCH_STATE_OK);
            assert(touch_state_output(&state).right_trigger == 0);
        }
    }
    touch_state_reset(&state);
    neutral(&state);
    near_value(state.deadzone, 0.25f);
    touch_state_init(&state, 1);
    assert(touch_state_begin(&state, 1, 0, TOUCH_STATE_ROLE_LEFT_STICK,
                             1, 1, 0) == TOUCH_STATE_OK);
    neutral(&state);
    touch_state_init(&state, NAN);
    near_value(state.deadzone, 0);
    touch_state_init(&state, -1);
    near_value(state.deadzone, 0);
    touch_state_init(&state, 2);
    near_value(state.deadzone, 1);
}

static void rejection_overflow_reset_and_isolation(void)
{
    TouchState state;
    TouchState other;
    size_t i;
    touch_state_init(&state, 0.1f);
    touch_state_init(&other, 0.3f);
    assert(touch_state_find(NULL, 1) == 0);
    assert(touch_state_find(&state, 0) == state.count);
    assert(TOUCH_STATE_CAPACITY >= 16);
    assert(touch_state_begin(&state, 0, ACCEL, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_begin(&state, 1, ACCEL, (TouchStateRole)99,
                             0, 0, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_move(&state, 0, 0, 0, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_end(&state, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_move(&state, 123, 1, 1, 1) == TOUCH_STATE_UNKNOWN);
    assert(touch_state_end(&state, 123) == TOUCH_STATE_UNKNOWN);
    neutral(&state);
    for (i = 0; i < TOUCH_STATE_CAPACITY; ++i) {
        assert(touch_state_begin(&state, (uintptr_t)(i + 1u), ACCEL,
                                 TOUCH_STATE_ROLE_NONE, 0, 0, 0) == TOUCH_STATE_OK);
    }
    assert(touch_state_begin(&state, 1, BRAKE, TOUCH_STATE_ROLE_LEFT_STICK,
                             1, 1, 1) == TOUCH_STATE_DUPLICATE);
    assert(touch_state_begin(&state, UINTPTR_MAX, BRAKE,
                             TOUCH_STATE_ROLE_LEFT_STICK, 1, 1, 1) == TOUCH_STATE_FULL);
    assert(touch_state_move(&state, UINTPTR_MAX, 1, 1, 1) == TOUCH_STATE_UNKNOWN);
    assert(touch_state_cancel(&state, UINTPTR_MAX) == TOUCH_STATE_UNKNOWN);
    assert(state.count == TOUCH_STATE_CAPACITY);
    assert(touch_state_output(&state).buttons == ACCEL);
    for (i = 0; i < TOUCH_STATE_CAPACITY; ++i) {
        assert(touch_state_end(&state, (uintptr_t)(i + 1u)) == TOUCH_STATE_OK);
        assert(touch_state_output(&state).buttons ==
               (i + 1u < TOUCH_STATE_CAPACITY ? ACCEL : 0));
    }
    /* Slot reuse after overflow, then reset all roles/buttons. */
    assert(touch_state_begin(&state, 1, ACCEL, TOUCH_STATE_ROLE_LEFT_STICK,
                             1, -1, 0) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 2, STEER, TOUCH_STATE_ROLE_LEFT_TRIGGER,
                             0, 0, 1) == TOUCH_STATE_OK);
    assert(touch_state_begin(&state, 3, BRAKE, TOUCH_STATE_ROLE_RIGHT_TRIGGER,
                             0, 0, 1) == TOUCH_STATE_OK);
    assert(touch_state_begin(&other, 1, BRAKE, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == TOUCH_STATE_OK);
    /* Same reset entry point for mode/layout/gate loss, repeatably. */
    for (i = 0; i < 3; ++i) {
        touch_state_reset(&state);
        neutral(&state);
        assert(state.count == 0);
        near_value(state.deadzone, 0.1f);
        assert(touch_state_move(&state, 1, 1, 1, 1) == TOUCH_STATE_UNKNOWN);
    }
    assert(touch_state_output(&other).buttons == BRAKE);
    near_value(other.deadzone, 0.3f);
    touch_state_init(NULL, 0);
    touch_state_reset(NULL);
    neutral(NULL);
    assert(touch_state_begin(NULL, 1, 0, TOUCH_STATE_ROLE_NONE,
                             0, 0, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_move(NULL, 1, 0, 0, 0) == TOUCH_STATE_INVALID);
    assert(touch_state_end(NULL, 1) == TOUCH_STATE_INVALID);
    assert(touch_state_cancel(NULL, 1) == TOUCH_STATE_INVALID);
}

int main(void)
{
    simultaneous_and_capture();
    duplicates_shared_buttons_and_cancel();
    analog_arbitration();
    dynamic_masks_and_union_collisions();
    bounds_deadzone_and_nonfinite();
    rejection_overflow_reset_and_isolation();
    puts("touch_state: all regression tests passed");
    return 0;
}
