#ifndef MADEIRA_XINPUT_READER_H
#define MADEIRA_XINPUT_READER_H
#include "../ControllerProtocol.h"
#include "xinput_abi.h"
typedef struct MCReader {
    uint32_t observed, confirmed, epoch, heartbeat_lo, heartbeat_hi;
    uint64_t changed_ms;
    MCPage scratch; /* Caller serializes access; no 4KB guest stack probe. */
} MCReader;
void mc_reader_reset(MCReader *reader);
int mc_reader_state(MCReader *reader, const MCPage *page, uint32_t index,
                    uint64_t now_ms, XINPUT_STATE *state);
#endif
