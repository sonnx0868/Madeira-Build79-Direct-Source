#include "ControllerValues.h"
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static MCPage *page;
static unsigned done;
static void *writer(void *unused) {
    (void)unused;
    MCPage snapshot = {.magic = MC_MAGIC, .version = MC_VERSION, .size = MC_FILE_SIZE,
                       .slot_count = MC_SLOT_COUNT, .writer_epoch = 42};
    for (uint32_t n = 1; n < 100000; ++n) {
        snapshot.heartbeat_lo = n;
        for (unsigned s = 0; s < MC_SLOT_COUNT; ++s) {
            snapshot.slots[s].packet = n;
            snapshot.slots[s].buttons = n ^ (s + 1);
            snapshot.slots[s].left_axes = ~n;
        }
        mc_publish(page, &snapshot);
        if ((n & 63u) == 0) sched_yield();
    }
    __atomic_store_n(&done, 1, __ATOMIC_RELEASE);
    return 0;
}
int main(void) {
    assert(mc_axis(-1) == -32768 && mc_axis(1) == 32767 && mc_axis(0) == 0);
    assert(mc_axis(-2) == -32768 && mc_axis(2) == 32767);
    assert(mc_axis(__builtin_nanf("")) == 0);
    assert(mc_stick(.02f, -.03f, .1f) == 0);
    assert(mc_stick(-1, 1, 0) == UINT32_C(0x7fff8000));
    assert(mc_stick(0, -1, .1f) == UINT32_C(0x80000000));
    assert(mc_trigger(0, 0) == 0 && mc_trigger(1, 0) == 255);
    assert(mc_trigger(.5f, 0) == 128 && mc_trigger(.04f, .05f) == 0);
    assert(mc_trigger(1, .95f) == 255 && mc_trigger(__builtin_nanf(""), 0) == 0);
    MCSlot disconnected = {0}, connected = {.flags = MC_CONNECTED};
    assert(mc_slot_changed(&disconnected, &connected));
    connected = (MCSlot){.packet = 1};
    assert(!mc_slot_changed(&disconnected, &connected));
    MCSlot merged = {.buttons = MC_BUTTON_A, .triggers = UINT32_C(0x4020)};
    mc_merge_touch(&merged, MC_BUTTON_B | MC_BUTTON_GUIDE | UINT32_C(0x0800), 0x10, 0x80);
    assert(merged.buttons == (MC_BUTTON_A | MC_BUTTON_B | MC_BUTTON_GUIDE));
    assert(merged.triggers == UINT32_C(0x8020));
    page = mmap(0, MC_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(page != MAP_FAILED);
    MCPage snapshot;
    assert(!mc_snapshot(page, &snapshot));
    page->sequence = 1;
    assert(!mc_snapshot(page, &snapshot));
    page->sequence = 0;
    pthread_t thread;
    assert(!pthread_create(&thread, 0, writer, 0));
    unsigned accepted = 0, intermediate = 0;
    do {
        if (mc_snapshot(page, &snapshot)) {
            ++accepted;
            if (snapshot.heartbeat_lo < 99999) ++intermediate;
            for (unsigned s = 0; s < MC_SLOT_COUNT; ++s) {
                assert(snapshot.slots[s].packet == snapshot.heartbeat_lo);
                assert(snapshot.slots[s].buttons == (snapshot.heartbeat_lo ^ (s + 1)));
                assert(snapshot.slots[s].left_axes == ~snapshot.heartbeat_lo);
            }
        }
    } while (!__atomic_load_n(&done, __ATOMIC_ACQUIRE));
    assert(!pthread_join(thread, 0));
    assert(accepted > 0);
    assert(intermediate > 0);
    assert(mc_snapshot(page, &snapshot));
    assert(snapshot.heartbeat_lo == 99999);
    snapshot.flags = 0;
    memset(snapshot.slots, 0, sizeof(snapshot.slots));
    mc_publish(page, &snapshot);
    assert(mc_snapshot(page, &snapshot));
    for (unsigned i = 0; i < MC_SLOT_COUNT; ++i) assert(snapshot.slots[i].flags == 0);
    assert(!munmap(page, MC_FILE_SIZE));
    printf("Controller analog/endpoints/deadzones/touch-merge/neutralization and 99999 concurrent publications passed.\n");
}
