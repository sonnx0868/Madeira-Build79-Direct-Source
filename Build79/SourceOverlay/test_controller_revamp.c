#include "ControllerValues.h"
#include "ControllerRetry.h"
#include "OverrideMerge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    MCRetry retry = {0};
    assert(mc_retry_ready(&retry));
    const unsigned delays[] = {2, 4, 8, 16, 30, 30, 30};
    for (unsigned failure = 0; failure < sizeof(delays) / sizeof(*delays); ++failure) {
        mc_retry_failed(&retry);
        assert(retry.remaining == delays[failure]);
        for (unsigned tick = 0; tick < delays[failure]; ++tick) assert(!mc_retry_ready(&retry));
        assert(mc_retry_ready(&retry));
    }
    mc_retry_reset(&retry);
    assert(!retry.failures && !retry.remaining && mc_retry_ready(&retry));
    MCSlot live = {.flags = MC_CONNECTED, .packet = 42, .buttons = MC_BUTTON_A,
        .triggers = 0xffff, .left_axes = 0x7fff8000, .right_axes = 0x80007fff};
    MCSlot paused = mc_slot_gated(live, 0);
    assert(paused.flags == MC_CONNECTED && paused.packet == 42);
    assert(!paused.buttons && !paused.triggers && !paused.left_axes && !paused.right_axes);
    assert(mc_slot_changed(&live, &paused));
    MCSlot resumed = mc_slot_gated(live, 1);
    assert(!mc_slot_changed(&live, &resumed));
    MCSlot disconnected = mc_slot_gated((MCSlot){0}, 0);
    assert(!disconnected.flags);
    // The primary guest pad exists at startup before any Bluetooth device.
    assert(mc_slot_present(0, 1, 0, 0));
    assert(mc_slot_present(0, 1, 1, 0));
    assert(mc_slot_present(0, 1, 0, 0)); // still present after physical unplug
    assert(!mc_slot_present(1, 1, 0, 0));
    assert(mc_slot_present(1, 1, 1, 0));
    assert(!mc_slot_present(0, 0, 1, 1)); // app inactivity stays safe
    assert(!mc_slot_present(MC_SLOT_COUNT, 1, 1, 1));
    MCSlot virtual_pad = {.flags = MC_CONNECTED};
    MCSlot virtual_neutral = mc_slot_gated(virtual_pad, 0);
    assert(virtual_neutral.flags == MC_CONNECTED && !virtual_neutral.buttons && !virtual_neutral.triggers);
    char out[1024], twice[1024];
    assert(mc_merge_overrides("d3d11=n,b;xinput1_3=b;winhttp=n", out, sizeof(out)));
    assert(!strcmp(out, "d3d11=n,b;winhttp=n;xinput1_3,xinput1_4,xinput9_1_0=n,b"));
    assert(mc_merge_overrides(out, twice, sizeof(twice)) && !strcmp(out, twice));
    assert(!mc_merge_overrides(out, twice, 4));
    puts("PASS: retries, gate/resume, persistent primary virtual pad/hotplug presence, inactivity, Wine overrides");
}
