/* Host-only reduced surrounding types/mocks. These are NOT native ABI headers.
 * Extracted production function bodies follow this file in generated tests. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "list.h"
typedef uint64_t client_ptr_t;
typedef uint64_t apc_param_t;
typedef uint32_t obj_handle_t;
typedef void (*async_completion_callback)(void *);
struct object;
struct object_ops { size_t size; void (*destroy)(struct object *); int kind; };
struct object { int refcount; const struct object_ops *ops; };
struct process { struct object obj; struct list asyncs; };
struct thread { struct object obj; struct process *process; };
struct async;
struct async_queue { struct list queue; };
struct fd { struct object obj; struct object *user; };
struct iosb { struct object obj; unsigned status; apc_param_t result; void *out_data; };
struct event; struct completion; struct timeout_user; struct reserve;
struct async_data { client_ptr_t user, iosb, apc, apc_context; };
enum { APC_NONE, APC_ASYNC_IO, APC_USER };
union apc_call {
    unsigned type;
    struct { unsigned type, status; client_ptr_t user, sb; apc_param_t result; } async_io;
    struct { unsigned type, flags; client_ptr_t func, args[3]; } user;
};
union apc_result {
    unsigned type;
    struct { unsigned type, status; apc_param_t total; } async_io;
};
#define STATUS_PENDING 0x103u
#define STATUS_ALERTED 0x101u
#define STATUS_CANCELLED 0xc0000120u
#define SYNCHRONIZE 0x100000u
#define FILE_SKIP_COMPLETION_PORT_ON_SUCCESS 1
#define NT_ERROR(s) (((unsigned)(s) >> 30) == 3)
static const struct object_ops async_ops, async_cancel_ops, thread_apc_ops;
static struct object *grab_object(void *);
static void release_object(void *);
static void *alloc_object(const struct object_ops *);
static struct object *create_internal_sync(int, int);
static void signal_sync(struct object *);
static int is_fd_overlapped(struct fd *fd) { (void)fd; return 1; }
static void remove_timeout_user(struct timeout_user *p) { assert(!p); }
static void fd_reselect_async(struct fd *, struct async_queue *);
static void async_set_initial_status(struct async *, unsigned);
static void add_async_completion(struct async *, apc_param_t, unsigned, apc_param_t);
static void set_event(struct event *p) { (void)p; assert(0); }
static void set_fd_signaled(struct fd *fd, int n) { (void)fd; (void)n; }
static void wake_up(struct object *p, int n) { (void)p; (void)n; }
static void reserve_obj_unbind(struct reserve *p) { assert(!p); }
static struct object *get_fd_user(struct fd *fd) { return fd->user; }
static obj_handle_t alloc_handle(struct process *, void *, unsigned, unsigned);
static void async_terminate(struct async *, unsigned);
static void async_set_result(struct object *, unsigned, apc_param_t);
static int thread_queue_apc(struct process *, struct thread *, struct object *, const union apc_call *);
struct thread_apc;
static int queue_apc(struct process *, struct thread *, struct thread_apc *);
static void fd_cancel_async(struct fd *, struct async *);
