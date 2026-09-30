#include "reader.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void neutral(const XINPUT_STATE *state)
{
    const XINPUT_STATE zero = {0};
    assert(!memcmp(state, &zero, sizeof zero));
}

int main(void)
{
    MCPage mapping = {0}, sample = { .magic = MC_MAGIC, .version = MC_VERSION,
        .size = MC_FILE_SIZE, .slot_count = MC_SLOT_COUNT, .writer_epoch = 7,
        .heartbeat_lo = 1, .flags = MC_FLAG_ACTIVE };
    MCReader reader = {0};
    XINPUT_STATE state;
    sample.slots[0] = (MCSlot){ .flags = MC_CONNECTED, .packet = 42,
        .buttons = 0xffff, .triggers = 0xff00,
        .left_axes = 0x7fff8000, .right_axes = 0x80007fff };
    mc_publish(&mapping, &sample);
    assert(!mc_reader_state(&reader, &mapping, 0, 100, &state)); neutral(&state);
    assert(!mc_reader_state(&reader, &mapping, 0, 5000, &state)); neutral(&state);
    sample.heartbeat_lo++;
    mc_publish(&mapping, &sample);
    assert(mc_reader_state(&reader, &mapping, 0, 5001, &state));
    assert(state.dwPacketNumber == 42 && state.Gamepad.wButtons == 0xf7ff);
    assert(state.Gamepad.bLeftTrigger == 0 && state.Gamepad.bRightTrigger == 255);
    assert(state.Gamepad.sThumbLX == -32768 && state.Gamepad.sThumbLY == 32767);
    assert(state.Gamepad.sThumbRX == 32767 && state.Gamepad.sThumbRY == -32768);
    assert(mc_reader_state(&reader, &mapping, 0, 5501, &state));
    assert(!mc_reader_state(&reader, &mapping, 0, 5502, &state)); neutral(&state);
    sample.heartbeat_hi++; /* Both counter words participate in freshness. */
    mc_publish(&mapping, &sample);
    assert(mc_reader_state(&reader, &mapping, 0, 5503, &state));
    assert(!mc_reader_state(&reader, &mapping, 0, 5502, &state)); neutral(&state);
    sample.flags = 0; sample.heartbeat_lo++;
    mc_publish(&mapping, &sample);
    assert(!mc_reader_state(&reader, &mapping, 0, 5504, &state)); neutral(&state);
    sample.flags = MC_FLAG_ACTIVE; sample.slots[0].flags = 0;
    mc_publish(&mapping, &sample);
    assert(!mc_reader_state(&reader, &mapping, 0, 5505, &state)); neutral(&state);
    sample.slots[0].flags = MC_CONNECTED; sample.writer_epoch++;
    mc_publish(&mapping, &sample);
    assert(!mc_reader_state(&reader, &mapping, 0, 5506, &state)); neutral(&state);
    sample.heartbeat_lo++;
    mc_publish(&mapping, &sample);
    assert(mc_reader_state(&reader, &mapping, 0, 5507, &state));
    assert(!mc_reader_state(&reader, &mapping, 4, 5508, &state)); neutral(&state);
    assert(!mc_reader_state(&reader, 0, 0, 5508, &state)); neutral(&state);
    uint32_t *invalid_fields[] = { &sample.magic, &sample.version,
                                  &sample.size, &sample.slot_count };
    for (unsigned i = 0; i < sizeof invalid_fields / sizeof *invalid_fields; ++i) {
        uint32_t saved = *invalid_fields[i];
        *invalid_fields[i] = 0;
        mc_publish(&mapping, &sample);
        assert(!mc_reader_state(&reader, &mapping, 0, 5508, &state)); neutral(&state);
        *invalid_fields[i] = saved;
    }
    mc_publish(&mapping, &sample);
    __atomic_store_n(&mapping.sequence, 1, __ATOMIC_RELEASE);
    assert(!mc_reader_state(&reader, &mapping, 0, 5508, &state)); neutral(&state);
    mc_reader_reset(&reader);
    mc_publish(&mapping, &sample);
    assert(!mc_reader_state(&reader, &mapping, 0, 5509, &state)); neutral(&state);
    puts("Reader ABI, analog endpoints, 500ms stale boundary, initial/epoch handshake, malformed/torn input, and neutralization passed.");
}
