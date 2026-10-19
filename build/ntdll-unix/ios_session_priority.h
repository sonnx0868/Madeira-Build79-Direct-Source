/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Native wait boundary, not translated game code. The steady path is an atomic
 * epoch comparison: no Mach thread scan, mutex or scheduling syscall. */
#pragma once
#include <stdint.h>
extern const uint64_t *madeira_session_priority_epoch_address(void) __attribute__((weak));
extern void madeira_session_priority_poll_current(int) __attribute__((weak));
static const uint64_t *ios_priority_epoch;
static _Thread_local uint64_t ios_priority_seen;
static _Thread_local int ios_priority_base_changed;
static inline void ios_session_priority_base_changed(void) {
    ios_priority_seen=0; ios_priority_base_changed=1;
}
static inline void ios_session_priority_poll(void) {
    const uint64_t *epoch=__atomic_load_n(&ios_priority_epoch,__ATOMIC_ACQUIRE);
    if (!epoch && madeira_session_priority_epoch_address) {
        epoch=madeira_session_priority_epoch_address();
        __atomic_store_n(&ios_priority_epoch,epoch,__ATOMIC_RELEASE);
    }
    if (!epoch || !madeira_session_priority_poll_current) return;
    uint64_t value=__atomic_load_n(epoch,__ATOMIC_ACQUIRE);
    if (ios_priority_seen==value && !ios_priority_base_changed) return;
    int changed=ios_priority_base_changed; ios_priority_base_changed=0;
    madeira_session_priority_poll_current(changed);
    ios_priority_seen=value;
}
static inline void ios_session_priority_leave(int *unused) {
    (void)unused; ios_session_priority_poll();
}
#define IOS_SESSION_PRIORITY_SCOPE() \
    int ios_priority_guard __attribute__((cleanup(ios_session_priority_leave),unused))=0; \
    ios_session_priority_poll()
