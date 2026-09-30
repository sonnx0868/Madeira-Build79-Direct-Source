#ifndef TOUCH_STATE_H
#define TOUCH_STATE_H

/* Header-only, C99 (Clang/GCC builtins), allocation-free.
 * Caller owns and serializes each state. TouchState state = {0} is valid
 * with no deadzone; init also clears all state and configures the deadzone.
 * IDs are nonzero opaque uintptr_t values, not dereferenced pointers.
 * Buttons are logical action bits; capture persists until end/cancel/reset.
 * Positions are normalized [-1,1], trigger values [0,1]. Nonfinite input
 * becomes zero; finite out-of-range input is clamped. Link with -lm.
 */
#include <stddef.h>
#include <stdint.h>

#define TOUCH_STATE_CAPACITY 32u

typedef enum TouchStateRole {
    TOUCH_STATE_ROLE_NONE = 0,
    TOUCH_STATE_ROLE_LEFT_STICK,
    TOUCH_STATE_ROLE_LEFT_TRIGGER,
    TOUCH_STATE_ROLE_RIGHT_TRIGGER
} TouchStateRole;

typedef enum TouchStateResult {
    TOUCH_STATE_OK = 1,
    TOUCH_STATE_DUPLICATE = 0,
    TOUCH_STATE_UNKNOWN = -1,
    TOUCH_STATE_FULL = -2,
    TOUCH_STATE_INVALID = -3
} TouchStateResult;

typedef struct TouchStateOutput {
    uint32_t buttons;
    float left_x;
    float left_y;
    uint8_t left_trigger;
    uint8_t right_trigger;
} TouchStateOutput;

typedef struct TouchStateContact {
    uintptr_t id;
    uint32_t buttons;
    TouchStateRole role;
    float x;
    float y;
    float trigger;
} TouchStateContact;

typedef struct TouchState {
    TouchStateContact contacts[TOUCH_STATE_CAPACITY];
    size_t count;
    float deadzone;
} TouchState;

static inline float touch_state_clamp(float value, float low, float high)
{
    if (!__builtin_isfinite(value)) return 0.0f;
    return value < low ? low : (value > high ? high : value);
}

/* Initialize or reinitialize; NULL is safely tolerated. */
static inline void touch_state_init(TouchState *state, float deadzone)
{
    if (state != NULL) {
        const TouchState empty = {0};
        *state = empty;
        state->deadzone = touch_state_clamp(deadzone, 0.0f, 1.0f);
    }
}

/* Invoke on mode change, layout change, or input gate loss. */
static inline void touch_state_reset(TouchState *state)
{
    if (state != NULL) touch_state_init(state, state->deadzone);
}

static inline size_t touch_state_find(const TouchState *state, uintptr_t id)
{
    size_t i;
    if (state == NULL) return 0;
    if (id == 0) return state->count;
    for (i = 0; i < state->count; ++i) {
        if (state->contacts[i].id == id) return i;
    }
    return state->count;
}

static inline int touch_state_is_active(const TouchState *state, uintptr_t id)
{
    return state != NULL && id != 0 &&
           touch_state_find(state, id) != state->count;
}

static inline void touch_state_set_values(TouchStateContact *contact,
                                          float x, float y, float trigger)
{
    contact->x = touch_state_clamp(x, -1.0f, 1.0f);
    contact->y = touch_state_clamp(y, -1.0f, 1.0f);
    contact->trigger = touch_state_clamp(trigger, 0.0f, 1.0f);
}

/* A duplicate begin is a complete no-op, including its supplied assignment
 * and values. A new contact can own buttons and one analog role together.
 * Rejected operations never modify state.
 */
static inline TouchStateResult touch_state_begin(
    TouchState *state, uintptr_t id, uint32_t buttons, TouchStateRole role,
    float x, float y, float trigger)
{
    TouchStateContact *contact;
    if (state == NULL || id == 0) return TOUCH_STATE_INVALID;
    if (touch_state_find(state, id) != state->count)
        return TOUCH_STATE_DUPLICATE;
    if (role < TOUCH_STATE_ROLE_NONE || role > TOUCH_STATE_ROLE_RIGHT_TRIGGER)
        return TOUCH_STATE_INVALID;
    if (state->count == TOUCH_STATE_CAPACITY) return TOUCH_STATE_FULL;
    contact = &state->contacts[state->count++];
    contact->id = id;
    contact->buttons = buttons;
    contact->role = role;
    touch_state_set_values(contact, x, y, trigger);
    return TOUCH_STATE_OK;
}

/* No hit testing: moves cannot acquire or reassign buttons or analog roles. */
static inline TouchStateResult touch_state_move(
    TouchState *state, uintptr_t id, float x, float y, float trigger)
{
    size_t index;
    if (state == NULL || id == 0) return TOUCH_STATE_INVALID;
    index = touch_state_find(state, id);
    if (index == state->count) return TOUCH_STATE_UNKNOWN;
    touch_state_set_values(&state->contacts[index], x, y, trigger);
    return TOUCH_STATE_OK;
}

/* The captured owner may update its own logical actions (e.g. keyboard
 * joystick directions). This never changes the analog role or capture ID.
 * UIKit must retain the original view owner and not re-hit-test on moves.
 */
static inline TouchStateResult touch_state_update_buttons(
    TouchState *state, uintptr_t id, uint32_t newmask)
{
    size_t index;
    if (state == NULL || id == 0) return TOUCH_STATE_INVALID;
    index = touch_state_find(state, id);
    if (index == state->count) return TOUCH_STATE_UNKNOWN;
    state->contacts[index].buttons = newmask;
    return TOUCH_STATE_OK;
}

static inline TouchStateResult touch_state_end(TouchState *state, uintptr_t id)
{
    size_t index;
    if (state == NULL || id == 0) return TOUCH_STATE_INVALID;
    index = touch_state_find(state, id);
    if (index == state->count) return TOUCH_STATE_UNKNOWN;
    /* Preserve begin order for deterministic most-recent stick ownership. */
    for (; index + 1u < state->count; ++index)
        state->contacts[index] = state->contacts[index + 1u];
    --state->count;
    {
        const TouchStateContact empty = {0};
        state->contacts[state->count] = empty;
    }
    return TOUCH_STATE_OK;
}

static inline TouchStateResult touch_state_cancel(TouchState *state, uintptr_t id)
{
    return touch_state_end(state, id);
}

/* Radial clamp and radial deadzone, linearly rescaled to full travel.
 * Latest begun active stick wins; releasing it falls back to the previous
 * active stick. Releasing the final stick always returns to center.
 * Recomputing the union avoids stale masks and naturally implements shared
 * button reference-count semantics without per-bit counters.
 */
static inline TouchStateOutput touch_state_output(const TouchState *state)
{
    TouchStateOutput output = {0};
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
    size_t i;
    if (state == NULL) return output;
    for (i = 0; i < state->count; ++i) {
        const TouchStateContact *contact = &state->contacts[i];
        output.buttons |= contact->buttons;
        if (contact->role == TOUCH_STATE_ROLE_LEFT_STICK) {
            float length = __builtin_sqrtf(contact->x * contact->x + contact->y * contact->y);
            output.left_x = 0.0f;
            output.left_y = 0.0f;
            if (length > state->deadzone && state->deadzone < 1.0f) {
                float travel = length > 1.0f ? 1.0f : length;
                float scale = ((travel - state->deadzone) /
                               (1.0f - state->deadzone)) / length;
                output.left_x = contact->x * scale;
                output.left_y = contact->y * scale;
            }
        } else if (contact->role == TOUCH_STATE_ROLE_LEFT_TRIGGER) {
            if (contact->trigger > left_trigger) left_trigger = contact->trigger;
        } else if (contact->role == TOUCH_STATE_ROLE_RIGHT_TRIGGER) {
            if (contact->trigger > right_trigger) right_trigger = contact->trigger;
        }
    }
    output.left_trigger = (uint8_t)(left_trigger * 255.0f + 0.5f);
    output.right_trigger = (uint8_t)(right_trigger * 255.0f + 0.5f);
    return output;
}

#endif /* TOUCH_STATE_H */
