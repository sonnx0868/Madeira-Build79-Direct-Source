// Public Metal capture only. No register access, guest writes or frame skipping.
#pragma once
#include <stdint.h>
// phase waits for the following-frame timing file; captureComplete describes
// the raw Metal document independently, including while timings are cancelled.
typedef struct { int phase; unsigned buffers, frames; double started; int captureComplete; } MadeiraGPUTraceStatus;
int madeira_gpu_trace_request(const char *path);
void madeira_gpu_trace_cancel(void);
MadeiraGPUTraceStatus madeira_gpu_trace_status(void);
uint64_t madeira_gpu_trace_will_create(void *queue);
void madeira_gpu_trace_did_create(void *buffer, uint64_t ticket);
void madeira_gpu_trace_present(void *buffer);
void madeira_gpu_trace_did_commit(void *buffer);
#ifdef __OBJC__
#import <Foundation/Foundation.h>
NSString *madeira_gpu_trace_error(void);
#endif
