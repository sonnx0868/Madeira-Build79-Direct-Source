#include "reader.h"

void mc_reader_reset(MCReader *reader)
{
    reader->observed = reader->confirmed = 0;
}

int mc_reader_state(MCReader *reader, const MCPage *page, uint32_t index,
                    uint64_t now_ms, XINPUT_STATE *state)
{
    MCSlot *slot;
    *state = (XINPUT_STATE){0};
    if (!page || index >= MC_SLOT_COUNT || !mc_snapshot(page, &reader->scratch))
        return 0;
    if (!reader->observed || reader->epoch != reader->scratch.writer_epoch) {
        reader->observed = 1;
        reader->confirmed = 0;
        reader->epoch = reader->scratch.writer_epoch;
        reader->heartbeat_lo = reader->scratch.heartbeat_lo;
        reader->heartbeat_hi = reader->scratch.heartbeat_hi;
        reader->changed_ms = now_ms;
        return 0; /* An old file alone must never look like a live controller. */
    }
    if (reader->heartbeat_lo != reader->scratch.heartbeat_lo ||
        reader->heartbeat_hi != reader->scratch.heartbeat_hi) {
        reader->heartbeat_lo = reader->scratch.heartbeat_lo;
        reader->heartbeat_hi = reader->scratch.heartbeat_hi;
        reader->changed_ms = now_ms;
        reader->confirmed = 1;
    }
    if (!reader->confirmed || now_ms < reader->changed_ms ||
        now_ms - reader->changed_ms > 500 ||
        !(reader->scratch.flags & MC_FLAG_ACTIVE)) return 0;
    slot = &reader->scratch.slots[index];
    if (!(slot->flags & MC_CONNECTED)) return 0;
    state->dwPacketNumber = slot->packet;
    /* Preserve Guide for ordinal-100 XInputGetStateEx. The public
     * XInputGetState entry point removes it from its documented contract. */
    state->Gamepad.wButtons = (WORD)(slot->buttons & 0xf7ffu);
    state->Gamepad.bLeftTrigger = (BYTE)slot->triggers;
    state->Gamepad.bRightTrigger = (BYTE)(slot->triggers >> 8);
    state->Gamepad.sThumbLX = (SHORT)(WORD)slot->left_axes;
    state->Gamepad.sThumbLY = (SHORT)(WORD)(slot->left_axes >> 16);
    state->Gamepad.sThumbRX = (SHORT)(WORD)slot->right_axes;
    state->Gamepad.sThumbRY = (SHORT)(WORD)(slot->right_axes >> 16);
    return 1;
}
