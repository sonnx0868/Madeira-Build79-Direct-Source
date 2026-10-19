/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Each Wine GUI queue gets a private nonblocking pipe. A producer signals all
 * subscribers once per unacknowledged burst; one consumer cannot erase another
 * queue's wake. This state contains no Wine/TEB or UIKit objects.
 */
#ifndef MADEIRA_INPUT_WAKE_QUEUE_H
#define MADEIRA_INPUT_WAKE_QUEUE_H
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define MADEIRA_INPUT_WAKE_SLOTS 64
struct madeira_input_wake_slot {
    uint64_t token;
    int read_fd, write_fd, pending, broken;
};
struct madeira_input_wake_registry {
    pthread_mutex_t lock;
    uint64_t generation;
    struct madeira_input_wake_slot slots[MADEIRA_INPUT_WAKE_SLOTS];
};
#define MADEIRA_INPUT_WAKE_INITIALIZER { .lock = PTHREAD_MUTEX_INITIALIZER }

static inline int madeira_input_wake_write(struct madeira_input_wake_slot *slot) {
    if (slot->pending) return 1;
    char byte = 1;
    ssize_t count;
    do count = write(slot->write_fd, &byte, 1); while (count < 0 && errno == EINTR);
    if (count == 1 || (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
        slot->pending = 1;
        return 1;
    }
    slot->broken = 1;
    return 0;
}

static inline uint64_t madeira_input_wake_create(struct madeira_input_wake_registry *registry, int *read_fd) {
    *read_fd = -1;
    pthread_mutex_lock(&registry->lock);
    unsigned index;
    for (index = 0; index < MADEIRA_INPUT_WAKE_SLOTS; ++index)
        if (!registry->slots[index].token) break;
    if (index == MADEIRA_INPUT_WAKE_SLOTS) {
        pthread_mutex_unlock(&registry->lock);
        errno = EMFILE;
        return 0;
    }
    int pair[2];
    if (pipe(pair)) { pthread_mutex_unlock(&registry->lock); return 0; }
    for (unsigned i = 0; i < 2; ++i) {
        int flags = fcntl(pair[i], F_GETFL);
        if (flags < 0 || fcntl(pair[i], F_SETFL, flags | O_NONBLOCK) < 0 ||
            fcntl(pair[i], F_SETFD, FD_CLOEXEC) < 0) {
            int error = errno;
            close(pair[0]); close(pair[1]);
            pthread_mutex_unlock(&registry->lock);
            errno = error;
            return 0;
        }
    }
    uint64_t token;
    int collision;
    do {
        token = ++registry->generation;
        collision = !token;
        for (unsigned i = 0; !collision && i < MADEIRA_INPUT_WAKE_SLOTS; ++i)
            collision = registry->slots[i].token == token;
    } while (collision);
    struct madeira_input_wake_slot slot = {token, pair[0], pair[1], 0, 0};
    // Cover input enqueued before the subscriber existed. The first event-pump
    // pass acknowledges this byte before draining the existing input ring.
    if (!madeira_input_wake_write(&slot)) {
        int error = errno;
        close(pair[0]); close(pair[1]);
        pthread_mutex_unlock(&registry->lock);
        errno = error;
        return 0;
    }
    registry->slots[index] = slot;
    *read_fd = pair[0];
    pthread_mutex_unlock(&registry->lock);
    return token;
}

static inline void madeira_input_wake_signal(struct madeira_input_wake_registry *registry) {
    pthread_mutex_lock(&registry->lock);
    for (unsigned i = 0; i < MADEIRA_INPUT_WAKE_SLOTS; ++i)
        if (registry->slots[i].token && !registry->slots[i].broken)
            madeira_input_wake_write(&registry->slots[i]);
    pthread_mutex_unlock(&registry->lock);
}

static inline int madeira_input_wake_ack(struct madeira_input_wake_registry *registry, uint64_t token) {
    int acknowledged = 0;
    pthread_mutex_lock(&registry->lock);
    for (unsigned i = 0; token && i < MADEIRA_INPUT_WAKE_SLOTS; ++i) {
        struct madeira_input_wake_slot *slot = &registry->slots[i];
        if (slot->token != token) continue;
        if (!slot->pending) break; // ordinary input polls need no read syscall
        char bytes[64];
        ssize_t count;
        do count = read(slot->read_fd, bytes, sizeof bytes); while (count > 0 || (count < 0 && errno == EINTR));
        if (count == 0 || (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) slot->broken = 1;
        slot->pending = 0;
        acknowledged = !slot->broken;
        break;
    }
    pthread_mutex_unlock(&registry->lock);
    return acknowledged;
}

/* A capability check must not consume readiness between an event-pump pass
 * and the subsequent kernel wait. Only the event pump may acknowledge it. */
static inline int madeira_input_wake_ready(struct madeira_input_wake_registry *registry, uint64_t token) {
    int ready = 0;
    pthread_mutex_lock(&registry->lock);
    for (unsigned i = 0; token && i < MADEIRA_INPUT_WAKE_SLOTS; ++i)
        if (registry->slots[i].token == token) { ready = !registry->slots[i].broken; break; }
    pthread_mutex_unlock(&registry->lock);
    return ready;
}

static inline void madeira_input_wake_remove(struct madeira_input_wake_registry *registry, uint64_t token) {
    pthread_mutex_lock(&registry->lock);
    for (unsigned i = 0; token && i < MADEIRA_INPUT_WAKE_SLOTS; ++i) {
        struct madeira_input_wake_slot *slot = &registry->slots[i];
        if (slot->token != token) continue;
        int read_fd = slot->read_fd, write_fd = slot->write_fd;
        *slot = (struct madeira_input_wake_slot){0};
        close(read_fd); close(write_fd);
        break;
    }
    pthread_mutex_unlock(&registry->lock);
}
#endif
