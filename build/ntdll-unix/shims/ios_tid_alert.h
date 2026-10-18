/* SPDX-License-Identifier: LGPL-2.1-or-later
 * One waiter per thread ID; any number of alert senders. A pending alert
 * coalesces just as in NtAlertThreadByThreadId's original binary flag.
 *
 * Publishing PARKED before the compare-and-wait makes posting before the
 * syscall safe: the kernel observes PENDING instead of PARKED and does not
 * sleep. Posting to IDLE needs no kernel wake. Cleanup uses CAS so that an
 * alert posted while the waiter is returning is retained for its next wait.
 */
#ifndef MADEIRA_IOS_TID_ALERT_H
#define MADEIRA_IOS_TID_ALERT_H
#include <stdint.h>

enum { IOS_TID_ALERT_IDLE = 0, IOS_TID_ALERT_PENDING = 1, IOS_TID_ALERT_PARKED = 2 };

static inline int ios_tid_alert_post(volatile int32_t *word)
{
    return __atomic_exchange_n(word, IOS_TID_ALERT_PENDING, __ATOMIC_SEQ_CST) == IOS_TID_ALERT_PARKED;
}

static inline void ios_tid_alert_unpark(volatile int32_t *word)
{
    int32_t expected = IOS_TID_ALERT_PARKED;
    __atomic_compare_exchange_n(word, &expected, IOS_TID_ALERT_IDLE, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline int ios_tid_alert_take_or_park(volatile int32_t *word)
{
    if (__atomic_exchange_n(word, IOS_TID_ALERT_PARKED, __ATOMIC_SEQ_CST) != IOS_TID_ALERT_PENDING)
        return 0;
    ios_tid_alert_unpark(word);
    return 1;
}

static inline int ios_tid_alert_pending(const volatile int32_t *word)
{
    return __atomic_load_n(word, __ATOMIC_SEQ_CST) == IOS_TID_ALERT_PENDING;
}
#endif
