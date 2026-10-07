#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void wine_runtime_enable(void);
int wine_runtime_is_enabled(void);
void wine_runtime_begin_generation(uint64_t generation);
void wine_runtime_set_processors(unsigned count);
unsigned wine_runtime_processors(void);
void wine_runtime_thread_attach(unsigned pid, unsigned tid, unsigned parent_pid, void *peb);
void wine_runtime_thread_reserve(unsigned pid, unsigned tid, void *peb);
void wine_runtime_thread_cancel(unsigned pid, unsigned tid, void *peb);
int wine_runtime_prepare_process_exit(unsigned pid, void *peb);
void wine_runtime_process_retired(unsigned pid, void *peb, int safe);
/* 1: retired + all owned Mach threads dead + GPU completed. 0: draining.
 * -1: missing lifecycle evidence / unsafe retirement; restart required. */
int wine_runtime_reuse_ready(void);
unsigned wine_runtime_live_threads(void);
void wine_runtime_gpu_begin(void);
void wine_runtime_gpu_end(void);
unsigned wine_runtime_gpu_pending(void);
void wine_prepare_game_compatibility(void);
#ifdef __cplusplus
}
#endif
