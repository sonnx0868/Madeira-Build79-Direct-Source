/* SPDX-License-Identifier: MIT */
#pragma once
#include <pthread.h>
#include <string.h>
#ifndef IOS_PROCESS_THREAD_CAPACITY
#define IOS_PROCESS_THREAD_CAPACITY 8192
#endif
struct ios_process_thread { unsigned pid, tid; void *peb; };
struct ios_process_threads {
    pthread_mutex_t lock;
    struct ios_process_thread entries[IOS_PROCESS_THREAD_CAPACITY];
    int incomplete;
};
static inline int ios_process_threads_add(struct ios_process_threads *s, unsigned pid, unsigned tid, void *peb) {
    pthread_mutex_lock(&s->lock);
    int empty = -1, result = 0;
    for (unsigned i = 0; i < IOS_PROCESS_THREAD_CAPACITY; ++i) {
        struct ios_process_thread *t = s->entries + i;
        if (pid && tid && peb && t->pid == pid && t->tid == tid && t->peb == peb) { result = 1; break; }
        if (!t->pid && empty < 0) empty = (int)i;
    }
    if (!result && pid && tid && peb && empty >= 0) {
        s->entries[empty] = (struct ios_process_thread){pid, tid, peb}; result = 1;
    }
    if (!result) s->incomplete = 1; // uncertainty never means last thread
    pthread_mutex_unlock(&s->lock); return result;
}
static inline int ios_process_threads_leave(struct ios_process_threads *s, unsigned pid, unsigned tid, void *peb) {
    pthread_mutex_lock(&s->lock);
    int found = 0, remaining = 0;
    for (unsigned i = 0; i < IOS_PROCESS_THREAD_CAPACITY; ++i) {
        struct ios_process_thread *t = s->entries + i;
        if (t->pid != pid || t->peb != peb || !pid || !peb) continue;
        if (t->tid == tid) { memset(t, 0, sizeof(*t)); found = 1; }
        else ++remaining;
    }
    if (!found || s->incomplete) remaining = -1;
    pthread_mutex_unlock(&s->lock); return remaining;
}
static inline void ios_process_threads_remove(struct ios_process_threads *s, unsigned pid, void *peb) {
    pthread_mutex_lock(&s->lock);
    for (unsigned i = 0; i < IOS_PROCESS_THREAD_CAPACITY; ++i)
        if (s->entries[i].pid == pid && s->entries[i].peb == peb) memset(s->entries + i, 0, sizeof(s->entries[i]));
    pthread_mutex_unlock(&s->lock);
}
int ios_process_thread_register(unsigned pid, unsigned tid, void *peb);
void ios_process_thread_retire(unsigned pid, void *peb);
