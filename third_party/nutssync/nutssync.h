/*
 * NutsSync: Windows synchronization objects in shared memory (NutsEmu)
 *
 * Semaphores, mutexes and events with Windows semantics (wait for any or all of several,
 * mutex ownership, recursion and abandonment, auto- and manual-reset events, pulse, an
 * alert object for APCs), for Wine's in-process synchronization: it does what Linux's
 * ntsync driver does for Wine there, so waits and signals never go to the server. All state
 * is in one shared memory region the server makes and hands to every process (the games in
 * NutsEmu's process and Steam's in other iOS processes), so it lives in no process; threads
 * sleep on a word of their own with os_sync_wait_on_address (shared) and the thread that
 * signals wakes them. One short lock guards every change.
 *
 * Header-only: the server and ntdll include it. Objects are numbers (slot and generation),
 * never file descriptors. Results are errno values as ntsync's ioctls give them, which
 * Wine's code already turns into NTSTATUS: 0, EOVERFLOW, EPERM, EOWNERDEAD, ETIMEDOUT,
 * EINVAL, ENOMEM.
 *
 * Written for NutsEmu from Wine's use of ntsync (dlls/ntdll/unix/sync.c,
 * server/inproc_sync.c).
 *
 * Copyright 2026 Evan Bolton
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef NUTSSYNC_H
#define NUTSSYNC_H

#include <errno.h>
#include <fcntl.h>
#include <os/os_sync_wait_on_address.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define NUTSSYNC_MAGIC        0x4e555453u  /* "NUTS" */
#define NUTSSYNC_VERSION      1
/* Madeira 2026-10-18: slot+1 must fit in 17 bits; reserve zero. The local
 * ntsync descriptor adapter has 11 generation bits and retires exhausted
 * slots, rather than letting an old descriptor alias a new object. */
#define NUTSSYNC_MAX_OBJECTS  131071
#define NUTSSYNC_GEN_MASK    0x7ffu
#ifndef NUTSSYNC_OBJECT_CREATED
#define NUTSSYNC_OBJECT_CREATED(slot) ((void)0)
#endif
#ifndef NUTSSYNC_WAIT_ALLOCATED
#define NUTSSYNC_WAIT_ALLOCATED(waiter) ((void)0)
#define NUTSSYNC_WAIT_RELEASED(waiter) ((void)0)
#endif
#define NUTSSYNC_MAX_WAITERS  8192
#define NUTSSYNC_MAX_WAIT     64           /* objects in one wait (MAXIMUM_WAIT_OBJECTS) */

enum nutssync_type { NUTSSYNC_FREE = 0, NUTSSYNC_SEM, NUTSSYNC_MUTEX, NUTSSYNC_EVENT };

struct nutssync_object
{
    uint32_t type;        /* enum nutssync_type */
    uint32_t gen;         /* bumped when the slot is freed: stale numbers fail */
    uint32_t waiting;     /* waiters that name it (a destroyed object waits for them) */
    uint32_t destroyed;   /* destroyed while waited on: freed when the last waiter leaves */
    uint32_t a, b;        /* sem: count, max; mutex: owner, count; event: manual, signaled */
    uint32_t abandoned;   /* mutex */
    uint32_t next_free;   /* free list (slot + 1, 0 = end) */
};

struct nutssync_waiter
{
    uint32_t state;       /* the word it sleeps on: 0 waiting, 1 done */
    uint32_t in_use;
    uint32_t all;         /* wait for all (else any) */
    uint32_t count;       /* objects */
    uint32_t owner;       /* Windows thread id (mutexes) */
    uint32_t alert;       /* object number of the alert, or 0 */
    uint32_t result;      /* when done: index (count = the alert) */
    int32_t  error;       /* when done: 0 or EOWNERDEAD */
    uint64_t seq;         /* registration order: the first waiter gets an object first */
    uint32_t objs[NUTSSYNC_MAX_WAIT];
    uint32_t next_free;
};

struct nutssync_region
{
    uint32_t magic, version;
    uint32_t lock;            /* 0 free, 1 held, 2 held with sleepers */
    int32_t  lock_pid;        /* holder (a holder that died gives the lock up) */
    uint32_t free_head;       /* object free list */
    uint32_t used;            /* slots ever handed out (the rest are past the free list) */
    uint32_t waiter_free_head;
    uint32_t waiters_used;
    uint64_t seq;
    uint32_t attaches;        /* processes that mapped it (the server first): shows it's shared */
    uint32_t missed_wakes;    /* waits that found themselves satisfied but not woken */
    uint32_t active_count;    /* waiters waiting, listed in active[] */
    uint32_t active[NUTSSYNC_MAX_WAITERS];
    struct nutssync_waiter waiters[NUTSSYNC_MAX_WAITERS];
    struct nutssync_object objects[NUTSSYNC_MAX_OBJECTS];
};

static struct nutssync_region *nutssync;  /* this module's mapping */

/* ---------------------------------------------------------------------------------- */
/* the region */

/* The server: a new region, unlinked at once; returns the descriptor the processes map
 * (it goes to them as the in-process sync "device"), or -1. */
static inline int nutssync_create_region( void )
{
    char path[1024];
    const char *tmp = getenv( "TMPDIR" );
    size_t size = sizeof(struct nutssync_region);
    int fd;

    snprintf( path, sizeof(path), "%s/nutssync-%d-XXXXXX", tmp ? tmp : "/tmp", getpid() );
    if ((fd = mkstemp( path )) < 0) return -1;
    unlink( path );
    if (ftruncate( fd, size ) < 0) goto failed;
    nutssync = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (nutssync == MAP_FAILED) goto failed;
    nutssync->magic = NUTSSYNC_MAGIC;
    nutssync->version = NUTSSYNC_VERSION;
    nutssync->attaches = 1;
    return fd;
failed:
    nutssync = NULL;
    close( fd );
    return -1;
}

/* A process: maps the region the server handed it. 0 or an errno; *number: how many
 * processes have mapped it, this one included (the server is 1). */
static inline int nutssync_attach( int fd, uint32_t *number )
{
    struct nutssync_region *r = mmap( NULL, sizeof(*r), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (r == MAP_FAILED) return errno;
    if (r->magic != NUTSSYNC_MAGIC || r->version != NUTSSYNC_VERSION)
    {
        munmap( r, sizeof(*r) );
        return EINVAL;
    }
    nutssync = r;
    if (number) *number = __atomic_add_fetch( &r->attaches, 1, __ATOMIC_ACQ_REL );
    return 0;
}

static inline int nutssync_wait_word( uint32_t *word, uint32_t value, uint64_t timeout_ns )
{
    int ret = timeout_ns ? os_sync_wait_on_address_with_timeout( word, value, 4, OS_SYNC_WAIT_ON_ADDRESS_SHARED,
                                                                   OS_CLOCK_MACH_ABSOLUTE_TIME, timeout_ns )
                         : os_sync_wait_on_address( word, value, 4, OS_SYNC_WAIT_ON_ADDRESS_SHARED );
    return ret < 0 ? errno : 0;
}

static inline void nutssync_wake_word( uint32_t *word, int all )
{
    if (all) os_sync_wake_by_address_all( word, 4, OS_SYNC_WAKE_BY_ADDRESS_SHARED );
    else os_sync_wake_by_address_any( word, 4, OS_SYNC_WAKE_BY_ADDRESS_SHARED );
}

/* The lock. Signals are blocked while it's held: Wine suspends a thread with a signal and
 * keeps it in the handler until it's resumed, and one suspended holding the lock stopped
 * every thread of every program, the server's too (compatibility suite, test ns1). A holder
 * that died (another process, killed) would block everything too: a waiter that waited a
 * second checks it and takes the lock over. */
static inline void nutssync_lock( sigset_t *saved )
{
    sigset_t all;
    sigfillset( &all );
    pthread_sigmask( SIG_BLOCK, &all, saved );
    uint32_t expected = 0;
    if (__atomic_compare_exchange_n( &nutssync->lock, &expected, 1, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED ))
        goto locked;
    for (;;)
    {
        if (__atomic_exchange_n( &nutssync->lock, 2, __ATOMIC_ACQUIRE ) == 0) goto locked;
        if (nutssync_wait_word( &nutssync->lock, 2, 1000000000ull ) == ETIMEDOUT)
        {
            int32_t pid = __atomic_load_n( &nutssync->lock_pid, __ATOMIC_RELAXED );
            if (pid > 0 && kill( pid, 0 ) < 0 && errno == ESRCH)
            {
                /* the holder is gone: its change may be half done, which is better than
                 * every thread of every program stopping */
                __atomic_store_n( &nutssync->lock, 2, __ATOMIC_RELEASE );
                goto locked;
            }
        }
    }
locked:
    nutssync->lock_pid = getpid();
}

static inline void nutssync_unlock( const sigset_t *saved )
{
    nutssync->lock_pid = 0;
    if (__atomic_exchange_n( &nutssync->lock, 0, __ATOMIC_RELEASE ) == 2) nutssync_wake_word( &nutssync->lock, 0 );
    pthread_sigmask( SIG_SETMASK, saved, NULL );
}

/* ---------------------------------------------------------------------------------- */
/* objects: number = generation << 17 | (slot + 1), positive, never 0 */

#define NUTSSYNC_SLOT_BITS 17
#define NUTSSYNC_SLOT_MASK ((1u << NUTSSYNC_SLOT_BITS) - 1)

static inline uint32_t nutssync_number( uint32_t slot, uint32_t gen )
{
    return ((gen & NUTSSYNC_GEN_MASK) << NUTSSYNC_SLOT_BITS) | (slot + 1);
}

/* the object a number names (lock held), or NULL */
static inline struct nutssync_object *nutssync_object_of( uint32_t number )
{
    uint32_t slot = (number & NUTSSYNC_SLOT_MASK) - 1;
    struct nutssync_object *obj;
    if (!number || slot >= NUTSSYNC_MAX_OBJECTS) return NULL;
    obj = &nutssync->objects[slot];
    if (obj->type == NUTSSYNC_FREE || obj->destroyed || obj->gen != number >> NUTSSYNC_SLOT_BITS) return NULL;
    return obj;
}

/* What a number names, for logs (read without the lock: a snapshot, may be torn) */
static inline int nutssync_describe( uint32_t number, char *buf, size_t size )
{
    uint32_t slot = (number & NUTSSYNC_SLOT_MASK) - 1;
    const struct nutssync_object *obj;
    if (!number || slot >= NUTSSYNC_MAX_OBJECTS) return snprintf( buf, size, "%#x(bad slot)", number );
    obj = &nutssync->objects[slot];
    return snprintf( buf, size, "%#x(slot %u gen %u/%u type %u%s a %u b %u waiting %u)", number, slot,
                     number >> NUTSSYNC_SLOT_BITS, obj->gen, obj->type,
                     obj->destroyed ? " destroyed" : "", obj->a, obj->b, obj->waiting );
}

static inline uint32_t nutssync_create( enum nutssync_type type, uint32_t a, uint32_t b )
{
    sigset_t saved;
    struct nutssync_object *obj;
    uint32_t slot;

    nutssync_lock( &saved );
    if (nutssync->free_head)
    {
        slot = nutssync->free_head - 1;
        nutssync->free_head = nutssync->objects[slot].next_free;
    }
    else if (nutssync->used < NUTSSYNC_MAX_OBJECTS) slot = nutssync->used++;
    else
    {
        nutssync_unlock( &saved );
        return 0;
    }
    obj = &nutssync->objects[slot];
    obj->type = type;
    obj->a = a;
    obj->b = b;
    obj->abandoned = 0;
    obj->waiting = 0;
    obj->destroyed = 0;
    uint32_t number = nutssync_number( slot, obj->gen );
    NUTSSYNC_OBJECT_CREATED( slot );
    nutssync_unlock( &saved );
    return number;
}

static inline uint32_t nutssync_create_semaphore( uint32_t count, uint32_t max )
{
    if (count > max) return 0;
    return nutssync_create( NUTSSYNC_SEM, count, max );
}

static inline uint32_t nutssync_create_mutex( uint32_t owner, uint32_t count )
{
    if (!owner != !count) return 0;
    return nutssync_create( NUTSSYNC_MUTEX, owner, count );
}

static inline uint32_t nutssync_create_event( int manual, int signaled )
{
    return nutssync_create( NUTSSYNC_EVENT, !!manual, !!signaled );
}

static inline void nutssync_free_slot( struct nutssync_object *obj )
{
    uint32_t slot = obj - nutssync->objects;
    obj->type = NUTSSYNC_FREE;
    obj->gen++;
    if (obj->gen <= NUTSSYNC_GEN_MASK)
    {
        obj->next_free = nutssync->free_head;
        nutssync->free_head = slot + 1;
    }
}

/* The server, when the object's last handle anywhere is closed. A thread still waiting on
 * it keeps it until it leaves (it can't be signaled any more, as on Windows). */
static inline void nutssync_destroy( uint32_t number )
{
    sigset_t saved;
    struct nutssync_object *obj;
    nutssync_lock( &saved );
    if ((obj = nutssync_object_of( number )))
    {
        if (obj->waiting) obj->destroyed = 1;
        else nutssync_free_slot( obj );
    }
    nutssync_unlock( &saved );
}

/* ---------------------------------------------------------------------------------- */
/* waking */

static inline int nutssync_can_take( const struct nutssync_object *obj, uint32_t owner )
{
    switch (obj->type)
    {
    case NUTSSYNC_SEM: return obj->a > 0;
    case NUTSSYNC_MUTEX: return !obj->a || (obj->a == owner && obj->b < UINT32_MAX);
    case NUTSSYNC_EVENT: return obj->b;
    }
    return 0;
}

/* takes it (can_take was true): EOWNERDEAD for an abandoned mutex, else 0 */
static inline int nutssync_take( struct nutssync_object *obj, uint32_t owner )
{
    int ret = 0;
    switch (obj->type)
    {
    case NUTSSYNC_SEM:
        obj->a--;
        break;
    case NUTSSYNC_MUTEX:
        if (obj->abandoned)
        {
            obj->abandoned = 0;
            ret = EOWNERDEAD;
        }
        obj->a = owner;
        obj->b++;
        break;
    case NUTSSYNC_EVENT:
        if (!obj->a) obj->b = 0;  /* auto-reset */
        break;
    }
    return ret;
}

/* Whether the waiter's wait is satisfied now, and if so takes what it waited for and
 * records the result (lock held). */
static inline int nutssync_try_satisfy( struct nutssync_waiter *w )
{
    struct nutssync_object *obj;
    uint32_t i;

    if (w->all)
    {
        for (i = 0; i < w->count; i++)
            if (!(obj = &nutssync->objects[(w->objs[i] & NUTSSYNC_SLOT_MASK) - 1]) || obj->destroyed ||
                !nutssync_can_take( obj, w->owner ))
                goto alert;
        w->error = 0;
        for (i = 0; i < w->count; i++)
            if (nutssync_take( &nutssync->objects[(w->objs[i] & NUTSSYNC_SLOT_MASK) - 1], w->owner ) == EOWNERDEAD)
                w->error = EOWNERDEAD;
        w->result = 0;
        return 1;
    }
    for (i = 0; i < w->count; i++)
    {
        obj = &nutssync->objects[(w->objs[i] & NUTSSYNC_SLOT_MASK) - 1];
        if (obj->destroyed || !nutssync_can_take( obj, w->owner )) continue;
        w->error = nutssync_take( obj, w->owner );
        w->result = i;
        return 1;
    }
alert:
    if (w->alert && (obj = nutssync_object_of( w->alert )) && nutssync_can_take( obj, w->owner ))
    {
        nutssync_take( obj, w->owner );
        w->error = 0;
        w->result = w->count;
        return 1;
    }
    return 0;
}

static inline void nutssync_leave( struct nutssync_waiter *w );

/* After a change: every waiter whose wait can now be satisfied, first registered first
 * (active[] is in registration order; lock held). */
static inline void nutssync_wake_waiters( void )
{
    uint32_t i = 0;
    while (i < nutssync->active_count)
    {
        struct nutssync_waiter *w = &nutssync->waiters[nutssync->active[i]];
        if (!nutssync_try_satisfy( w ))
        {
            i++;
            continue;
        }
        nutssync_leave( w );  /* (takes it out of active[]: the next one is at i now) */
        __atomic_store_n( &w->state, 1, __ATOMIC_RELEASE );
        nutssync_wake_word( &w->state, 1 );
    }
}

/* ---------------------------------------------------------------------------------- */
/* operations (each 0 or an errno, as ntsync's) */

static inline int nutssync_semaphore_release( uint32_t number, uint32_t count, uint32_t *prev )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_SEM) ret = EINVAL;
    else if ((uint64_t)obj->a + count > obj->b) ret = EOVERFLOW;
    else
    {
        if (prev) *prev = obj->a;
        obj->a += count;
        nutssync_wake_waiters();
    }
    nutssync_unlock( &saved );
    return ret;
}

static inline int nutssync_semaphore_read( uint32_t number, uint32_t *count, uint32_t *max )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_SEM) ret = EINVAL;
    else
    {
        *count = obj->a;
        *max = obj->b;
    }
    nutssync_unlock( &saved );
    return ret;
}

/* *prev_count: the count before */
static inline int nutssync_mutex_unlock( uint32_t number, uint32_t owner, uint32_t *prev_count )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_MUTEX) ret = EINVAL;
    else if (!owner || obj->a != owner) ret = EPERM;
    else
    {
        if (prev_count) *prev_count = obj->b;
        if (!--obj->b)
        {
            obj->a = 0;
            nutssync_wake_waiters();
        }
    }
    nutssync_unlock( &saved );
    return ret;
}

static inline int nutssync_mutex_read( uint32_t number, uint32_t *owner, uint32_t *count )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_MUTEX) ret = EINVAL;
    else
    {
        *owner = obj->a;
        *count = obj->b;
        if (obj->abandoned) ret = EOWNERDEAD;
    }
    nutssync_unlock( &saved );
    return ret;
}

/* A thread ended: a mutex it owned is abandoned and free. */
static inline int nutssync_mutex_kill( uint32_t number, uint32_t owner )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_MUTEX) ret = EINVAL;
    else if (!owner || obj->a != owner) ret = EPERM;
    else
    {
        obj->a = 0;
        obj->b = 0;
        obj->abandoned = 1;
        nutssync_wake_waiters();
    }
    nutssync_unlock( &saved );
    return ret;
}

enum nutssync_event_op { NUTSSYNC_SET, NUTSSYNC_RESET, NUTSSYNC_PULSE };

static inline int nutssync_event( uint32_t number, enum nutssync_event_op op, uint32_t *prev )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_EVENT) ret = EINVAL;
    else
    {
        if (prev) *prev = obj->b;
        obj->b = op != NUTSSYNC_RESET;
        if (op != NUTSSYNC_RESET) nutssync_wake_waiters();
        if (op == NUTSSYNC_PULSE) obj->b = 0;
    }
    nutssync_unlock( &saved );
    return ret;
}

static inline int nutssync_event_read( uint32_t number, uint32_t *manual, uint32_t *signaled )
{
    sigset_t saved;
    struct nutssync_object *obj;
    int ret = 0;
    nutssync_lock( &saved );
    if (!(obj = nutssync_object_of( number )) || obj->type != NUTSSYNC_EVENT) ret = EINVAL;
    else
    {
        *manual = obj->a;
        *signaled = obj->b;
    }
    nutssync_unlock( &saved );
    return ret;
}

/* ---------------------------------------------------------------------------------- */
/* A thread ended (the server, with the mutexes it abandons): its waits go. A thread that
 * died waiting (its process crashed, or Wine ended it) left a waiter behind, which took the
 * next signal for itself and the living waiter never woke (Steam stopped at "Sign in" after
 * its first web helper crashed, test ns3). */
static inline void nutssync_thread_ended( uint32_t tid )
{
    uint32_t i = 0;
    sigset_t saved;

    if (!tid) return;
    nutssync_lock( &saved );
    while (i < nutssync->active_count)
    {
        uint32_t slot = nutssync->active[i];
        struct nutssync_waiter *w = &nutssync->waiters[slot];
        if (w->owner != tid)
        {
            i++;
            continue;
        }
        nutssync_leave( w );  /* (takes it out of active[]) */
        w->in_use = 0;
        w->next_free = nutssync->waiter_free_head;
        nutssync->waiter_free_head = slot + 1;
    }
    nutssync_unlock( &saved );
}

/* ---------------------------------------------------------------------------------- */
/* waiting */

static inline void nutssync_leave( struct nutssync_waiter *w )
{
    uint32_t i, me = w - nutssync->waiters;
    for (i = 0; i < nutssync->active_count; i++)
        if (nutssync->active[i] == me)
        {
            memmove( &nutssync->active[i], &nutssync->active[i + 1],
                     (--nutssync->active_count - i) * sizeof(nutssync->active[0]) );  /* (keeps the order) */
            break;
        }
    for (i = 0; i < w->count; i++)
    {
        struct nutssync_object *obj = &nutssync->objects[(w->objs[i] & NUTSSYNC_SLOT_MASK) - 1];
        if (!--obj->waiting && obj->destroyed) nutssync_free_slot( obj );
    }
}

static inline uint64_t nutssync_now( int realtime )
{
    struct timespec ts;
    clock_gettime( realtime ? CLOCK_REALTIME : CLOCK_MONOTONIC, &ts );
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

/* Waits for any (all = 0) or all of `count` objects, and the alert (0 for none), until the
 * deadline (ns on CLOCK_MONOTONIC, or CLOCK_REALTIME if realtime; UINT64_MAX: forever).
 * *index: what was taken (count = the alert). 0, EOWNERDEAD, ETIMEDOUT, EINVAL or ENOMEM. */
static inline int nutssync_wait( const uint32_t *objs, uint32_t count, int all, uint32_t alert, uint32_t owner,
                          uint64_t deadline, int realtime, uint32_t *index )
{
    sigset_t saved;
    struct nutssync_waiter *w;
    uint32_t i, j, slot;
    int ret;

    if (count > NUTSSYNC_MAX_WAIT) return EINVAL;
    nutssync_lock( &saved );
    for (i = 0; i < count; i++)
    {
        if (!nutssync_object_of( objs[i] )) goto invalid;
        if (all)  /* the same object twice can't be waited for all at once */
            for (j = 0; j < i; j++)
                if (objs[j] == objs[i]) goto invalid;
    }
    if (alert && !nutssync_object_of( alert )) goto invalid;

    if (nutssync->waiter_free_head)
    {
        slot = nutssync->waiter_free_head - 1;
        nutssync->waiter_free_head = nutssync->waiters[slot].next_free;
    }
    else if (nutssync->waiters_used < NUTSSYNC_MAX_WAITERS) slot = nutssync->waiters_used++;
    else
    {
        nutssync_unlock( &saved );
        return ENOMEM;
    }
    w = &nutssync->waiters[slot];
    w->in_use = 1;
    w->all = all && count > 1;
    w->count = count;
    w->owner = owner;
    w->alert = alert;
    w->state = 0;
    memcpy( w->objs, objs, count * sizeof(*objs) );
    NUTSSYNC_WAIT_ALLOCATED( w );

    if (nutssync_try_satisfy( w ))  /* at once */
    {
        *index = w->result;
        ret = w->error;
        goto release;
    }
    if (deadline != UINT64_MAX && nutssync_now( realtime ) >= deadline)
    {
        ret = ETIMEDOUT;
        goto release;
    }
    for (i = 0; i < count; i++) nutssync->objects[(objs[i] & NUTSSYNC_SLOT_MASK) - 1].waiting++;
    w->seq = ++nutssync->seq;
    nutssync->active[nutssync->active_count++] = slot;
    nutssync_unlock( &saved );

    for (;;)
    {
        uint64_t left = 1000000000ull;  /* at most a second at a time: see below */
        if (__atomic_load_n( &w->state, __ATOMIC_ACQUIRE )) break;
        if (deadline != UINT64_MAX)
        {
            uint64_t now = nutssync_now( realtime );
            if (now >= deadline) break;
            if (deadline - now < left) left = deadline - now;
        }
        if (nutssync_wait_word( &w->state, 0, left ) == ETIMEDOUT && !__atomic_load_n( &w->state, __ATOMIC_ACQUIRE ))
        {
            /* a wake-up that never came: what it waits for may be there anyway (the safety
             * net; counted, so it shows) */
            nutssync_lock( &saved );
            if (!w->state && nutssync_try_satisfy( w ))
            {
                nutssync_leave( w );
                w->state = 1;
                nutssync->missed_wakes++;
            }
            nutssync_unlock( &saved );
        }
    }

    nutssync_lock( &saved );
    if (w->state)  /* satisfied by a waker (which took it out of the active list) */
    {
        *index = w->result;
        ret = w->error;
    }
    else
    {
        nutssync_leave( w );
        ret = ETIMEDOUT;
    }
release:
    NUTSSYNC_WAIT_RELEASED( w );
    w->in_use = 0;
    w->next_free = nutssync->waiter_free_head;
    nutssync->waiter_free_head = slot + 1;
    nutssync_unlock( &saved );
    return ret;
invalid:
    nutssync_unlock( &saved );
    return EINVAL;
}

#endif /* NUTSSYNC_H */
