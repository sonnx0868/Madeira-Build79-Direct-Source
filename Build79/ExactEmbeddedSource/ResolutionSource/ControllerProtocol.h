#ifndef MADEIRA_CONTROLLER_PROTOCOL_H
#define MADEIRA_CONTROLLER_PROTOCOL_H
#include <stdint.h>
#include <stddef.h>

/* Little-endian, naturally aligned 32-bit words; no pointers or native BOOL. */
#define MC_MAGIC UINT32_C(0x4d434750) /* file bytes: P G C M */
#define MC_VERSION 1u
#define MC_FILE_SIZE 4096u
#define MC_SLOT_COUNT 4u
#define MC_FLAG_ACTIVE 1u
#define MC_CONNECTED 1u
#define MC_BUTTON_DPAD_UP 0x0001u
#define MC_BUTTON_DPAD_DOWN 0x0002u
#define MC_BUTTON_DPAD_LEFT 0x0004u
#define MC_BUTTON_DPAD_RIGHT 0x0008u
#define MC_BUTTON_START 0x0010u
#define MC_BUTTON_BACK 0x0020u
#define MC_BUTTON_LEFT_THUMB 0x0040u
#define MC_BUTTON_RIGHT_THUMB 0x0080u
#define MC_BUTTON_LEFT_SHOULDER 0x0100u
#define MC_BUTTON_RIGHT_SHOULDER 0x0200u
#define MC_BUTTON_GUIDE 0x0400u
#define MC_BUTTON_A 0x1000u
#define MC_BUTTON_B 0x2000u
#define MC_BUTTON_X 0x4000u
#define MC_BUTTON_Y 0x8000u
#define MC_TOUCH_BUTTON_MASK 0xf7ffu

typedef struct MCSlot {
    uint32_t flags;             /* bit 0 connected; disconnected state is zero */
    uint32_t packet;            /* advances only when the slot state changes */
    uint32_t buttons;           /* standard XINPUT_GAMEPAD wButtons bits */
    uint32_t triggers;          /* LT bits 0..7, RT bits 8..15; each 0..255 */
    uint32_t left_axes;         /* signed 16-bit X low, signed 16-bit Y high */
    uint32_t right_axes;
    uint32_t reserved[2];
} MCSlot;

typedef struct MCPage {
    uint32_t magic, version, size, slot_count;
    uint32_t sequence;          /* odd = publishing; acquire-load before/after */
    uint32_t writer_epoch;      /* randomized on writer start */
    uint32_t heartbeat_lo;      /* monotonic update counter, NOT a wall clock */
    uint32_t heartbeat_hi;
    uint32_t flags;             /* MC_FLAG_ACTIVE; zero forces disconnected */
    uint32_t reserved[7];
    MCSlot slots[MC_SLOT_COUNT];
    uint32_t padding[(MC_FILE_SIZE - 64 - MC_SLOT_COUNT * 32) / 4];
} MCPage;

_Static_assert(sizeof(MCSlot) == 32, "MCSlot wire layout");
_Static_assert(offsetof(MCPage, sequence) == 16, "sequence alignment");
_Static_assert(offsetof(MCPage, slots) == 64, "slot wire offset");
_Static_assert(sizeof(MCPage) == MC_FILE_SIZE, "one fixed wire page");

/* Single writer. All payload words are atomic too: no torn C data races. */
static inline void mc_publish(MCPage *mapping, const MCPage *snapshot) {
    uint32_t sequence = __atomic_load_n(&mapping->sequence, __ATOMIC_RELAXED);
    sequence = (sequence + 1u) | 1u;
    __atomic_exchange_n(&mapping->sequence, sequence, __ATOMIC_ACQ_REL);
    uint32_t *destination = (uint32_t *)mapping;
    const uint32_t *source = (const uint32_t *)snapshot;
    for (unsigned i = 0; i < sizeof(MCPage) / 4; ++i)
        if (i != 4) __atomic_store_n(destination + i, source[i], __ATOMIC_RELAXED);
    __atomic_store_n(&mapping->sequence, sequence + 1u, __ATOMIC_RELEASE);
}

/* Read-only mapping. Bounded retry: never spin indefinitely if writer dies.
 * Reader policy: require heartbeat advancement after open/epoch change; treat
 * 500 ms without (epoch, heartbeat) advancement as disconnected. The heartbeat
 * has no wall-clock epoch: measure elapsed time using the reader's own clock. */
static inline int mc_snapshot(const MCPage *mapping, MCPage *snapshot) {
    for (unsigned retry = 0; retry < 8; ++retry) {
        uint32_t before = __atomic_load_n(&mapping->sequence, __ATOMIC_ACQUIRE);
        if (before & 1u) continue;
        const uint32_t *source = (const uint32_t *)mapping;
        uint32_t *destination = (uint32_t *)snapshot;
        for (unsigned i = 0; i < sizeof(MCPage) / 4; ++i)
            destination[i] = __atomic_load_n(source + i, __ATOMIC_RELAXED);
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        uint32_t after = __atomic_load_n(&mapping->sequence, __ATOMIC_ACQUIRE);
        if (before == after && !(after & 1u))
            return snapshot->magic == MC_MAGIC && snapshot->version == MC_VERSION &&
                   snapshot->size == MC_FILE_SIZE && snapshot->slot_count == MC_SLOT_COUNT;
    }
    return 0;
}
#endif
