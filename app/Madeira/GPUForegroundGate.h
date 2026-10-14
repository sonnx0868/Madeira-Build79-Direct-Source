#ifndef MADEIRA_GPU_FOREGROUND_GATE_H
#define MADEIRA_GPU_FOREGROUND_GATE_H
#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t resumed;
    int active;
} MadeiraGPUForegroundGate;
#define MADEIRA_GPU_FOREGROUND_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 1}

static inline void madeira_gpu_gate_set(MadeiraGPUForegroundGate *gate, int active) {
    pthread_mutex_lock(&gate->lock);
    gate->active = active;
    if (active) pthread_cond_broadcast(&gate->resumed);
    pthread_mutex_unlock(&gate->lock);
}
// Hold through commit so an inactive notification cannot overtake submission.
static inline void madeira_gpu_gate_begin(MadeiraGPUForegroundGate *gate) {
    pthread_mutex_lock(&gate->lock);
    while (!gate->active) pthread_cond_wait(&gate->resumed, &gate->lock);
}
static inline void madeira_gpu_gate_end(MadeiraGPUForegroundGate *gate) {
    pthread_mutex_unlock(&gate->lock);
}
#endif
