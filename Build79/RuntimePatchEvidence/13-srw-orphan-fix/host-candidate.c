#define _GNU_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef int32_t LONG;
typedef uintptr_t ULONG_PTR;
typedef int BOOL;
typedef unsigned char BOOLEAN;
#define TRUE 1
#define FALSE 0
#define WINAPI
typedef struct { uintptr_t Ptr; } RTL_SRWLOCK;
struct srw_lock { short exclusive_waiters; unsigned short owners; };
_Static_assert(sizeof(struct srw_lock) == 4, "Wine SRW layout");
static struct { void *tid; const void *addr; uint64_t since; int inf; }
    ios_alert_waiters[512];
#define IOS_ALERT_WAITER_MAX 512
static unsigned wakes, all_wakes, reap_logs, strike_logs, errors;
static const void *wake_addr;
static LONG InterlockedCompareExchange(LONG *ptr, LONG desired, LONG expected)
{
    __atomic_compare_exchange_n(ptr, &expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static int fixture_dprintf(int fd, const char *format, ...)
{
    (void)fd;
    if (strstr(format, "[lock-reap] SRW")) ++reap_logs;
    if (strstr(format, "no-live-stamp strike=")) ++strike_logs;
    return 0;
}
#define dprintf fixture_dprintf
#define ERR(...) (++errors)
static void ios_srw_note(const void *p, unsigned mode) { (void)p; (void)mode; }
static void ios_shared_futex_wake(const void *p, int all)
{
    ++wakes; all_wakes += !!all; wake_addr = p;
}
static void RtlWakeAddressSingle(const void *p) { ios_shared_futex_wake(p, 0); }
static void RtlWakeAddressAll(const void *p) { ios_shared_futex_wake(p, 1); }

void ios_srw_reap_exclusive( unsigned long long lock_addr, unsigned long long dead_teb )
{
    volatile LONG *word = (volatile LONG *)(ULONG_PTR)lock_addr;
    LONG old, desired;
    unsigned int excl;
    if (!lock_addr || (lock_addr & 3) || lock_addr >= 0x8000000000ULL) return;
    {
        char *page = (char *)((ULONG_PTR)lock_addr & ~0x3fffULL);
        if (msync( page, 0x4000, MS_ASYNC )) return;
    }
    do
    {
        old = *word;
        excl = (unsigned int)old & 0xffff;
        if (!(excl & 1)) return;   /* not exclusively held — nothing to reap */
        desired = (LONG)(excl & ~1u);   /* owners := 0, held bit cleared, waiters kept */
    } while (InterlockedCompareExchange( (LONG *)word, desired, old ) != old);
    dprintf( 2, "[lock-reap] SRW %#llx dead_teb=%#llx released exclusive (word %08x -> %08x) rev=ml446\n",
             lock_addr, dead_teb, (unsigned int)old, (unsigned int)desired );
    if (desired & 0xffff)
        ios_shared_futex_wake( (const void *)(ULONG_PTR)(lock_addr + 2), 0 );
    else
        ios_shared_futex_wake( (const void *)(ULONG_PTR)lock_addr, 1 );
}
void ios_orphan_check( const unsigned long long *live_stamps, int nstamps )
{
    static struct { unsigned long long lock; int strikes; } susp[8];
    int i, j, k;
    for (i = 0; i < IOS_ALERT_WAITER_MAX; i++)
    {
        const void *a = ios_alert_waiters[i].addr;
        unsigned long long lock;
        unsigned int word;
        int nsame = 0, stamped = 0;
        if (!a || ((ULONG_PTR)a & 3) != 2 || (ULONG_PTR)a < 0x10000 || (ULONG_PTR)a >= 0x8000000000ULL) continue;
        lock = (unsigned long long)(ULONG_PTR)a - 2;
        for (j = 0; j < IOS_ALERT_WAITER_MAX; j++)
            if (ios_alert_waiters[j].addr == a) nsame++;
        if (nsame < 3) continue;
        {
            char *page = (char *)((ULONG_PTR)lock & ~0x3fffULL);
            if (msync( page, 0x4000, MS_ASYNC )) continue;
            word = *(volatile unsigned int *)(ULONG_PTR)lock;
        }
        if (!(word & 1))
        {
            /* released legitimately — clear any stale suspicion */
            for (j = 0; j < 8; j++)
                if (susp[j].lock == lock) { susp[j].lock = 0; susp[j].strikes = 0; }
            continue;
        }
        /* ml468: the word must be a COHERENT exclusively-held wine SRW before
         * we may reap it as one.  RtlAcquireSRWLockExclusive sets owners
         * (high 16) to exactly 1 and bit0 of exclusive_waiters; parked
         * exclusive waiters occupy bits 15:1 in steps of 2, so >=3 parked
         * threads imply a non-empty queue.  The ml467 run reaped 0x40010001
         * here — not an SRW at all but a live shared-owned FEX
         * WritePriorityMutex (read-owners=1, write-waiter=1; its read-waiters
         * park at lock+2 too, the lock-ID-rule trap) held by a running sweep
         * thread; zeroing it killed the run within a second. */
        if ((word >> 16) != 1 || ((word & 0xffff) >> 1) == 0)
        {
            static int incoherent_logs;
            if (incoherent_logs < 8)
            {
                incoherent_logs++;
                dprintf( 2, "[lock-orphan] SKIP %#llx word=%08x waiters=%d incoherent-as-SRW (FEX lock?) rev=ml468\n",
                         lock, word, nsame );
            }
            for (j = 0; j < 8; j++)
                if (susp[j].lock == lock) { susp[j].lock = 0; susp[j].strikes = 0; }
            continue;
        }
        for (k = 0; k < nstamps; k++) if (live_stamps[k] == lock) stamped = 1;
        for (j = 0; j < 8; j++) if (susp[j].lock == lock) break;
        if (stamped)
        {
            if (j < 8) { susp[j].lock = 0; susp[j].strikes = 0; }
            continue;
        }
        if (j == 8)   /* new suspect: claim a free slot */
        {
            for (j = 0; j < 8 && susp[j].lock; j++) ;
            if (j == 8) continue;
            susp[j].lock = lock;
            susp[j].strikes = 0;
        }
        susp[j].strikes++;
        dprintf( 2, "[lock-orphan] SRW %#llx word=%08x waiters=%d no-live-stamp strike=%d/3 rev=ml447\n",
                 lock, word, nsame, susp[j].strikes );
        if (susp[j].strikes >= 3)
        {
            /* A missing FEX stamp does not establish the owner of an anonymous
             * SRW is dead. Keep this heuristic diagnostic-only; leave the
             * real owner responsible for release and waiter notification. */
            susp[j].lock = 0;
            susp[j].strikes = 0;
        }
    }
}
void WINAPI RtlReleaseSRWLockExclusive( RTL_SRWLOCK *lock )
{
    union { RTL_SRWLOCK *rtl; struct srw_lock *s; LONG *l; } u = { lock };
    union { struct srw_lock s; LONG l; } old, new;

    do
    {
        old.s = *u.s;
        new = old;

        if (!(old.s.exclusive_waiters & 1)) ERR("Lock %p is not owned exclusive!\n", lock);

        new.s.owners = 0;
        new.s.exclusive_waiters &= ~1;
    } while (InterlockedCompareExchange( u.l, new.l, old.l ) != old.l);

    ios_srw_note( lock, 1 );
    if (new.s.exclusive_waiters)
        RtlWakeAddressSingle( &u.s->owners );
    else
        RtlWakeAddressAll( u.s );
}
void WINAPI RtlReleaseSRWLockShared( RTL_SRWLOCK *lock )
{
    union { RTL_SRWLOCK *rtl; struct srw_lock *s; LONG *l; } u = { lock };
    union { struct srw_lock s; LONG l; } old, new;

    do
    {
        old.s = *u.s;
        new = old;

        if (old.s.exclusive_waiters & 1) ERR("Lock %p is owned exclusive!\n", lock);
        else if (!old.s.owners) ERR("Lock %p is not owned shared!\n", lock);

        --new.s.owners;
    } while (InterlockedCompareExchange( u.l, new.l, old.l ) != old.l);

    ios_srw_note( lock, 3 );
    if (!new.s.owners)
        RtlWakeAddressSingle( &u.s->owners );
}
BOOLEAN WINAPI RtlTryAcquireSRWLockExclusive( RTL_SRWLOCK *lock )
{
    union { RTL_SRWLOCK *rtl; struct srw_lock *s; LONG *l; } u = { lock };
    union { struct srw_lock s; LONG l; } old, new;
    BOOLEAN ret;

    do
    {
        old.s = *u.s;
        new.s = old.s;

        if (!old.s.owners)
        {
            /* Not locked exclusive or shared. We can try to grab it. */
            new.s.owners = 1;
            new.s.exclusive_waiters |= 1;
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    } while (InterlockedCompareExchange( u.l, new.l, old.l ) != old.l);

    if (ret) ios_srw_note( lock, 0 );
    return ret;
}


static RTL_SRWLOCK *lock;
static uint32_t word(void) { return *(volatile uint32_t *)lock; }
static void setup(uint32_t value, unsigned registered)
{
    lock->Ptr = value;
    memset(ios_alert_waiters, 0, sizeof(ios_alert_waiters));
    for (unsigned i = 0; i < registered; ++i) {
        ios_alert_waiters[i].tid = (void *)(uintptr_t)(0x100 + i);
        ios_alert_waiters[i].addr = (const char *)lock + 2;
    }
    wakes = all_wakes = reap_logs = strike_logs = errors = 0;
    wake_addr = NULL;
}

/* An adversarial schedule: the genuine owner is still inside its critical
 * section across the census. A different thread then attempts the actual Wine
 * try-acquire. Baseline permits simultaneous entry; candidate refuses it. */
static void live_owner(unsigned registered, int stamp_kind, const char *name)
{
    uint32_t held = 0x10001u | (registered * 2u);
    unsigned long long stamp = (unsigned long long)(uintptr_t)lock + (stamp_kind == 2 ? 8 : 0);
    setup(held, registered);
    assert(!RtlTryAcquireSRWLockExclusive(lock));
    ios_orphan_check(stamp_kind ? &stamp : NULL, stamp_kind ? 1 : 0);
    bool false_reap = registered >= 3 && stamp_kind != 1;
#ifdef BASELINE
    if (false_reap) {
        assert(word() == registered * 2u);
        assert(reap_logs == 1 && strike_logs == 3 && wakes == 1 && !all_wakes);
        assert(wake_addr == (const char *)lock + 2);
        assert(RtlTryAcquireSRWLockExclusive(lock)); /* live original owner still holds */
        /* Its genuine release now wrongly erases the intervening entrant. */
        RtlReleaseSRWLockExclusive(lock);
        assert(word() == registered * 2u);
        printf("PASS baseline detects live-owner mutual-exclusion violation: %s\n", name);
        return;
    }
#else
    if (false_reap) assert(strike_logs == registered); /* preserved per-waiter diagnostics */
#endif
    assert(word() == held && reap_logs == 0 && wakes == 0 && errors == 0);
    assert(!RtlTryAcquireSRWLockExclusive(lock));
    /* The actual owner's ordinary release is the first permitted unlock. */
    RtlReleaseSRWLockExclusive(lock);
    assert(word() == registered * 2u && errors == 0 && wakes == 1);
    assert(all_wakes == (registered == 0));
    assert(wake_addr == (registered ? (const char *)lock + 2 : (const char *)lock));
    assert(RtlTryAcquireSRWLockExclusive(lock));
    RtlReleaseSRWLockExclusive(lock);
    assert(word() == registered * 2u && errors == 0);
    printf("PASS owner preserved until normal release/wake: %s\n", name);
}
static void ignored_shape(uint32_t value, const char *name)
{
    setup(value, 3);
    ios_orphan_check(NULL, 0);
    assert(word() == value && wakes == 0 && reap_logs == 0);
    printf("PASS non-target state unchanged: %s\n", name);
}
int main(void)
{
    void *mapping = mmap(NULL, 65536, PROT_READ|PROT_WRITE,
                         MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
    assert(mapping != MAP_FAILED);
    lock = (RTL_SRWLOCK *)(((uintptr_t)mapping + 16383) & ~(uintptr_t)16383);
    assert((uintptr_t)lock > 0x10000 && (uintptr_t)lock < 0x8000000000ULL);
    assert(msync(lock, 16384, MS_ASYNC) == 0); /* real mapped-memory check */
    live_owner(3, 0, "three waiters, no stamp, one census");
    live_owner(5, 0, "five waiters, no stamp");
    live_owner(3, 2, "unrelated live FEX stamp");
    live_owner(3, 1, "matching live stamp");
    live_owner(2, 0, "two waiters");
    live_owner(0, 0, "no waiters");
    ignored_shape(0x00020006, "shared owners with queued writers");
    ignored_shape(0x40010007, "incoherent FEX-shaped word");
    ignored_shape(0x00010001, "exclusive with no encoded queued waiters");
    /* Explain the observed terminal word with the exact pinned release code,
     * without pretending this isolated arithmetic case reconstructs a race. */
    setup(0x00000010, 8);
    RtlReleaseSRWLockShared(lock);
    assert(word() == 0xffff0010 && errors == 1 && wakes == 0);
    assert(!RtlTryAcquireSRWLockExclusive(lock));
    printf("PASS observed owners underflow blocks new exclusive acquire\n");
    assert(munmap(mapping, 65536) == 0);
    puts("PASS cases=10");
    return 0;
}
