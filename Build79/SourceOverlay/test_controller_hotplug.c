#include "ControllerValues.h"
#include "xinput/reader.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    MCPage mapping = {0}, snapshot = {.magic = MC_MAGIC, .version = MC_VERSION,
        .size = MC_FILE_SIZE, .slot_count = MC_SLOT_COUNT, .writer_epoch = 123,
        .flags = MC_FLAG_ACTIVE, .heartbeat_lo = 1};
    MCReader reader = {0};
    XINPUT_STATE state;
    snapshot.slots[0].flags = mc_slot_present(0, 1, 0, 0) ? MC_CONNECTED : 0;
    mc_publish(&mapping, &snapshot);
    assert(!mc_reader_state(&reader, &mapping, 0, 0, &state));
    ++snapshot.heartbeat_lo;
    mc_publish(&mapping, &snapshot);
    assert(mc_reader_state(&reader, &mapping, 0, 16, &state));
    assert(!state.Gamepad.wButtons && !state.Gamepad.bRightTrigger);
    // This exact reader/game slot now sees a controller connected mid-session.
    snapshot.slots[0].buttons = MC_BUTTON_A;
    snapshot.slots[0].triggers = 255u << 8;
    snapshot.slots[0].left_axes = mc_stick(1, 0, 0);
    ++snapshot.slots[0].packet;
    ++snapshot.heartbeat_lo;
    mc_publish(&mapping, &snapshot);
    assert(mc_reader_state(&reader, &mapping, 0, 32, &state));
    assert(state.Gamepad.wButtons == MC_BUTTON_A && state.Gamepad.bRightTrigger == 255);
    assert(state.Gamepad.sThumbLX == 32767);
    // Physical unplug releases all input without removing the virtual device.
    snapshot.slots[0] = mc_slot_gated(snapshot.slots[0], 0);
    ++snapshot.slots[0].packet;
    ++snapshot.heartbeat_lo;
    mc_publish(&mapping, &snapshot);
    assert(mc_reader_state(&reader, &mapping, 0, 48, &state));
    assert(!state.Gamepad.wButtons && !state.Gamepad.bRightTrigger && !state.Gamepad.sThumbLX);
    snapshot.slots[0].buttons = MC_BUTTON_B;
    ++snapshot.slots[0].packet;
    ++snapshot.heartbeat_lo;
    mc_publish(&mapping, &snapshot);
    assert(mc_reader_state(&reader, &mapping, 0, 64, &state));
    assert(state.Gamepad.wButtons == MC_BUTTON_B);
    assert(!mc_reader_state(&reader, &mapping, 1, 64, &state));
    assert(!mc_reader_state(&reader, &mapping, 0, 565, &state)); // stale virtual pad is not live
    snapshot.flags = 0;
    ++snapshot.heartbeat_lo;
    mc_publish(&mapping, &snapshot);
    assert(!mc_reader_state(&reader, &mapping, 0, 566, &state));
    puts("PASS: guest reader enumerates virtual pad before pairing; mid-game attach/unplug/reconnect; unused slots, stale/background safety");
}
