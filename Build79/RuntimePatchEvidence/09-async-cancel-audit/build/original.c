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
struct async_cancel
{
    struct object        obj;                 /* object header */
    struct object       *sync;                /* sync object for wait/signal */
    unsigned int         count;               /* count of the asyncs in the cancel group */
};
struct async
{
    struct object        obj;             /* object header */
    struct thread       *thread;          /* owning thread */
    struct list          queue_entry;     /* entry in async queue list */
    struct list          process_entry;   /* entry in process list */
    struct async_queue  *queue;           /* queue containing this async */
    struct fd           *fd;              /* fd associated with an unqueued async */
    struct timeout_user *timeout;
    unsigned int         timeout_status;  /* status to report upon timeout */
    struct event        *event;
    struct async_data    data;            /* data for async I/O call */
    struct iosb         *iosb;            /* I/O status block */
    obj_handle_t         wait_handle;     /* pre-allocated wait handle */
    unsigned int         initial_status;  /* status returned from initial request */
    unsigned int         signaled :1;
    unsigned int         pending :1;      /* request successfully queued, but pending */
    unsigned int         direct_result :1;/* a flag if we're passing result directly from request instead of APC  */
    unsigned int         alerted :1;      /* fd is signaled, but we are waiting for client-side I/O */
    unsigned int         terminated :1;   /* async has been terminated */
    unsigned int         canceled :1;     /* have we already queued cancellation for this async? */
    unsigned int         unknown_status :1; /* initial status is not known yet */
    unsigned int         blocking :1;     /* async is blocking */
    unsigned int         is_system :1;    /* background system operation not affecting userspace visible state. */
    struct completion   *completion;      /* completion associated with fd */
    apc_param_t          comp_key;        /* completion key associated with fd */
    unsigned int         comp_flags;      /* completion flags */
    async_completion_callback completion_callback; /* callback to be called on completion */
    void                *completion_callback_private; /* argument to completion_callback */
    struct async_cancel *async_cancel;    /* cancel object if async is being canceled */
};
struct thread_apc
{
    struct object       obj;      /* object header */
    struct object      *sync;     /* sync object for wait/signal */
    struct list         entry;    /* queue linked list */
    struct thread      *caller;   /* thread that queued this apc */
    struct object      *owner;    /* object that queued this apc */
    struct reserve     *reserve;  /* reserve object associated with apc object */
    int                 executed; /* has it been executed by the client? */
    union apc_call      call;     /* call arguments */
    union apc_result    result;   /* call results once executed */
};

static void async_cancel_destroy( struct object *obj )
{
    struct async_cancel *cancel = (struct async_cancel *)obj;

    assert( obj->ops == &async_cancel_ops );
    if (cancel->sync) release_object( cancel->sync );
}

static struct async_cancel *create_async_cancel( struct process *process )
{
    struct async_cancel *cancel;

    if (!(cancel = alloc_object( &async_cancel_ops ))) return NULL;
    cancel->sync = NULL;
    cancel->count = 0;

    if (!(cancel->sync = create_internal_sync( 1, 0 )))
    {
        release_object( cancel );
        return NULL;
    }
    return cancel;
}

static inline void async_reselect( struct async *async )
{
    if (async->queue && async->fd) fd_reselect_async( async->fd, async->queue );
}

static void async_destroy( struct object *obj )
{
    struct async *async = (struct async *)obj;
    assert( obj->ops == &async_ops );

    assert( !async->async_cancel );
    list_remove( &async->process_entry );

    if (async->queue)
    {
        list_remove( &async->queue_entry );
        async_reselect( async );
    }
    else if (async->fd) release_object( async->fd );

    if (async->timeout) remove_timeout_user( async->timeout );
    if (async->completion) release_object( async->completion );
    if (async->event) release_object( async->event );
    if (async->iosb) release_object( async->iosb );
    release_object( async->thread );
}

static void async_call_completion_callback( struct async *async )
{
    if (async->completion_callback)
        async->completion_callback( async->completion_callback_private );
    async->completion_callback = NULL;
}

static void async_complete_cancel( struct async *async )
{
    struct async_cancel *cancel;

    if (!(cancel = async->async_cancel)) return;
    async->async_cancel = NULL;

    if (!--cancel->count)
    {
        signal_sync( cancel->sync );
        release_object( cancel );
    }
}

static void async_set_result( struct object *obj, unsigned int status, apc_param_t total )
{
    struct async *async = (struct async *)obj;

    if (obj->ops != &async_ops) return;  /* in case the client messed up the APC results */

    assert( async->terminated );  /* it must have been woken up if we get a result */

    if (async->unknown_status) async_set_initial_status( async, status );

    if (async->alerted && status == STATUS_PENDING)  /* restart it */
    {
        async->terminated = 0;
        async->alerted = 0;
        async_reselect( async );
    }
    else
    {
        if (async->timeout) remove_timeout_user( async->timeout );
        async->timeout = NULL;
        async->terminated = 1;
        if (async->iosb) async->iosb->status = status;

        /* don't signal completion if the async failed synchronously
         * this can happen if the initial status was unknown (i.e. for device files)
         * note that we check the IOSB status here, not the initial status */
        if (async->pending || !NT_ERROR( status ))
        {
            if (async->data.apc)
            {
                union apc_call data;
                memset( &data, 0, sizeof(data) );
                data.type         = APC_USER;
                data.user.flags   = 0;
                data.user.func    = async->data.apc;
                data.user.args[0] = async->data.apc_context;
                data.user.args[1] = async->data.iosb;
                data.user.args[2] = 0;
                thread_queue_apc( NULL, async->thread, NULL, &data );
            }
            else if (async->data.apc_context && (async->pending ||
                     !(async->comp_flags & FILE_SKIP_COMPLETION_PORT_ON_SUCCESS)))
            {
                add_async_completion( async, async->data.apc_context, status, total );
            }

            if (async->event) set_event( async->event );
            else if (async->fd && !async->is_system) set_fd_signaled( async->fd, 1 );
        }

        if (!async->signaled)
        {
            async->signaled = 1;
            wake_up( &async->obj, 0 );
        }

        async_call_completion_callback( async );
        async_complete_cancel( async );

        if (async->queue)
        {
            list_remove( &async->queue_entry );
            async_reselect( async );
            async->fd = NULL;
            async->queue = NULL;
            release_object( async );
        }
    }
}

static void async_terminate( struct async *async, unsigned int status )
{
    struct iosb *iosb = async->iosb;

    if (async->terminated) return;

    async->terminated = 1;
    if (async->iosb && async->iosb->status == STATUS_PENDING) async->iosb->status = status;
    if (status == STATUS_ALERTED)
        async->alerted = 1;

    /* if no APC could be queued (e.g. the process is terminated),
     * thread_queue_apc() may trigger async_set_result(), which may drop the
     * last reference to the async, so grab a temporary reference here */
    grab_object( async );

    if (!async->direct_result)
    {
        union apc_call data;

        memset( &data, 0, sizeof(data) );
        data.type            = APC_ASYNC_IO;
        data.async_io.user   = async->data.user;
        data.async_io.result = iosb ? iosb->result : 0;

        /* this can happen if the initial status was unknown (i.e. for device
         * files). the client should not fill the IOSB in this case; pass it as
         * NULL to communicate that.
         * note that we check the IOSB status and not the initial status */
        if (NT_ERROR( status ) && (!is_fd_overlapped( async->fd ) || !async->pending))
            data.async_io.sb = 0;
        else
            data.async_io.sb = async->data.iosb;

        /* if there is output data, the client needs to make an extra request
         * to retrieve it; use STATUS_ALERTED to signal this case */
        if (iosb && iosb->out_data)
            data.async_io.status = STATUS_ALERTED;
        else
            data.async_io.status = status;

        thread_queue_apc( async->thread->process, async->thread, &async->obj, &data );
    }

    async_reselect( async );

    release_object( async );
}

static void cancel_async( struct async *async )
{
    async->canceled = 1;
    fd_cancel_async( async->fd, async );
}

static void thread_apc_destroy( struct object *obj )
{
    struct thread_apc *apc = (struct thread_apc *)obj;

    if (apc->caller) release_object( apc->caller );
    if (apc->owner)
    {
        if (apc->result.type == APC_ASYNC_IO)
            async_set_result( apc->owner, apc->result.async_io.status, apc->result.async_io.total );
        else if (apc->call.type == APC_ASYNC_IO)
            async_set_result( apc->owner, apc->call.async_io.status, 0 );
        release_object( apc->owner );
    }
    if (apc->sync) release_object( apc->sync );
    reserve_obj_unbind( apc->reserve );
}

static struct thread_apc *create_apc( struct object *owner, const union apc_call *call_data )
{
    struct thread_apc *apc;

    if ((apc = alloc_object( &thread_apc_ops )))
    {
        apc->sync        = NULL;
        if (call_data) apc->call = *call_data;
        else apc->call.type = APC_NONE;
        apc->caller      = NULL;
        apc->owner       = owner;
        apc->reserve     = NULL;
        apc->executed    = 0;
        apc->result.type = APC_NONE;
        if (owner) grab_object( owner );

        if (!(apc->sync = create_internal_sync( 1, 0 )))
        {
            release_object( apc );
            return NULL;
        }
    }
    return apc;
}

static int thread_queue_apc( struct process *process, struct thread *thread, struct object *owner, const union apc_call *call_data )
{
    struct thread_apc *apc;
    int ret = 0;

    if ((apc = create_apc( owner, call_data )))
    {
        ret = queue_apc( process, thread, apc );
        release_object( apc );
    }
    return ret;
}

static int cancel_process_async( struct process *process, struct object *obj, struct thread *thread, client_ptr_t iosb, obj_handle_t *wait_handle )
{
    struct async_cancel *cancel = NULL;
    struct async *async, *next_async;
    struct list tracked;
    int count = 0;

    if (thread && !(cancel = create_async_cancel( process ))) return 0;

    list_init( &tracked );

    /* We can't simply use LIST_FOR_EACH_ENTRY_SAFE here, because currently
     * cancelling an async can cause other asyncs to be removed via
     * async_reselect() */

restart:
    LIST_FOR_EACH_ENTRY( async, &process->asyncs, struct async, process_entry )
    {
        if (async->terminated || async->is_system) continue;
        if ((!obj || (get_fd_user( async->fd ) == obj)) &&
            (!thread || async->thread == thread) &&
            (!iosb || async->data.iosb == iosb))
        {
            if (!async->canceled) cancel_async( async );
            if (cancel)
            {
                assert( !async->async_cancel );
                async->async_cancel = cancel;
                cancel->count++;
            }
            list_remove( &async->process_entry );
            list_add_tail( &tracked, &async->process_entry );
            count++;
            goto restart;
        }
    }
    /* Put the asyncs back into the process list */
    LIST_FOR_EACH_ENTRY_SAFE( async, next_async, &tracked, struct async, process_entry )
    {
        list_remove( &async->process_entry );
        list_add_tail( &process->asyncs, &async->process_entry );
    }
    if (cancel)
    {
        if (!cancel->count) release_object( cancel );
        else *wait_handle = alloc_handle( process, cancel, SYNCHRONIZE, 0 );
    }
    return count;
}

/* Host-only lifecycle fixtures. Production async/APC bodies appear above. */
static const struct object_ops async_ops = {sizeof(struct async), async_destroy, 1};
static const struct object_ops async_cancel_ops = {sizeof(struct async_cancel), async_cancel_destroy, 2};
static const struct object_ops thread_apc_ops = {sizeof(struct thread_apc), thread_apc_destroy, 3};
static const struct object_ops simple_ops = {sizeof(struct object), NULL, 4};
static const struct object_ops thread_ops = {sizeof(struct thread), NULL, 5};
static const struct object_ops fd_ops = {sizeof(struct fd), NULL, 6};
static const struct object_ops iosb_ops = {sizeof(struct iosb), NULL, 7};
static int live_objects, mode[64], cancel_calls, signal_calls;
static int fail_group, fail_sync, fail_handle, trigger_previous;
static struct object *held_handle;
static struct thread_apc *pending[64];
static unsigned pending_count;
static struct process process;
static struct thread *owner;
static struct fd *file;
static struct async_queue queue;

static void *alloc_object(const struct object_ops *ops) {
    if (ops==&async_cancel_ops && fail_group) return NULL;
    struct object *o=calloc(1,ops->size); assert(o);
    o->ops=ops;o->refcount=1;live_objects++;return o;
}
static struct object *grab_object(void *p) {
    struct object *o=p;assert(o && o->refcount>0);o->refcount++;return o;
}
static void release_object(void *p) {
    struct object *o=p;assert(o && o->refcount>0);
    if (--o->refcount) return;
    if (o->ops->destroy) o->ops->destroy(o);
    live_objects--;free(o);
}
static struct object *create_internal_sync(int manual,int signaled) {
    assert(manual==1 && signaled==0);
    if(fail_sync){fail_sync=0;return NULL;}
    return alloc_object(&simple_ops);
}
static void signal_sync(struct object *o) {assert(o && o->refcount>0);signal_calls++;}
static obj_handle_t alloc_handle(struct process *p,void *o,unsigned access,unsigned attr) {
    assert(p==&process && access==SYNCHRONIZE && !attr && !held_handle);
    if(fail_handle)return 0;
    held_handle=grab_object(o);return 0x40;
}
static void async_set_initial_status(struct async *a,unsigned status) {
    a->initial_status=status;a->unknown_status=0;
}
static void add_async_completion(struct async *a,apc_param_t key,unsigned status,apc_param_t total) {
    (void)a;(void)key;(void)status;(void)total;assert(0);
}
static int queue_apc(struct process *p,struct thread *t,struct thread_apc *apc) {
    assert(p==&process && t==owner && apc->call.type==APC_ASYNC_IO);
    struct async *a=(struct async *)apc->owner;
    if(mode[a->data.user]==1){
        assert(pending_count<64);pending[pending_count++]=(struct thread_apc *)grab_object(apc);return 1;
    }
    return 0; /* The real APC destructor completes the dead-thread request now. */
}
static void complete_first(void) {
    assert(pending_count);struct thread_apc *apc=pending[0];
    memmove(pending,pending+1,--pending_count*sizeof(*pending));release_object(apc);
}
static void fd_reselect_async(struct fd *fd,struct async_queue *q) {
    assert(fd==file && q==&queue);
    if(trigger_previous && cancel_calls>1 && pending_count){trigger_previous=0;complete_first();}
}
static void fd_cancel_async(struct fd *fd,struct async *a) {
    assert(fd==file);cancel_calls++;
    async_terminate(a,STATUS_CANCELLED);
}
static void setup(void) {
    assert(!live_objects);memset(&process,0,sizeof(process));list_init(&process.asyncs);
    list_init(&queue.queue);owner=alloc_object(&thread_ops);owner->process=&process;
    file=alloc_object(&fd_ops);file->user=&process.obj;
    memset(mode,0,sizeof(mode));pending_count=0;held_handle=NULL;
    cancel_calls=signal_calls=fail_group=fail_sync=fail_handle=trigger_previous=0;
}
static struct async *make_async(unsigned id,int deferred) {
    struct async *a=alloc_object(&async_ops);assert(id<64);mode[id]=deferred;
    a->thread=(struct thread *)grab_object(owner);a->fd=file;a->queue=&queue;
    a->data.user=id;a->data.iosb=0x1000+id;a->pending=1;
    a->iosb=alloc_object(&iosb_ops);a->iosb->status=STATUS_PENDING;
    list_add_tail(&process.asyncs,&a->process_entry);list_add_tail(&queue.queue,&a->queue_entry);
    return a;
}
static int count_asyncs(void) {
    struct list *p;int n=0;LIST_FOR_EACH(p,&process.asyncs){assert(++n<65);}return n;
}
static void finish(void) {
    trigger_previous=0;
    while(pending_count)complete_first();
    if(held_handle){release_object(held_handle);held_handle=NULL;}
    while(!list_empty(&process.asyncs)) {
        struct async *a=LIST_ENTRY(list_head(&process.asyncs),struct async,process_entry);
        assert(!a->async_cancel);release_object(a);
    }
    assert(list_empty(&queue.queue));release_object(owner);release_object(file);assert(!live_objects);
}
static void dead_case(int group,int n) {
    setup();for(int i=0;i<n;i++)make_async(i,0);
    obj_handle_t h=0;int got=cancel_process_async(&process,file->user,group?owner:NULL,0,&h);
    assert(got==n && !h && !count_asyncs() && cancel_calls==n);
    finish();
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(!strcmp(argv[1],"dead-no-group")){dead_case(0,1);return 0;}
    if(!strcmp(argv[1],"dead-group")){dead_case(1,1);return 0;}
    assert(!strcmp(argv[1],"all"));int cases=0;
    for(int group=0;group<2;group++)for(int n=0;n<=24;n++) {dead_case(group,n);cases++;}
    for(int n=1;n<=24;n++) {
        setup();for(int i=0;i<n;i++)make_async(i,1);
        obj_handle_t h=0;assert(cancel_process_async(&process,file->user,owner,0,&h)==n);
        assert(h==0x40 && pending_count==(unsigned)n && !signal_calls && count_asyncs()==n);
        while(pending_count>1){complete_first();assert(!signal_calls);}
        complete_first();assert(signal_calls==1 && !count_asyncs());finish();cases++;
    }
    /* Reselect may finish an earlier tracked request during a later cancel. */
    setup();make_async(0,1);make_async(1,0);trigger_previous=1;
    obj_handle_t h=0;assert(cancel_process_async(&process,file->user,owner,0,&h)==2);
    assert(!h && !pending_count && !count_asyncs());finish();cases++;
    /* A remaining deferred operation keeps the group alive after sync completion. */
    setup();make_async(0,0);make_async(1,1);
    h=0;assert(cancel_process_async(&process,file->user,owner,0,&h)==2);
    assert(h==0x40 && pending_count==1 && count_asyncs()==1);finish();cases++;
    for(int failure=0;failure<3;failure++) {
        setup();make_async(0,1);fail_group=failure==0;fail_sync=failure==1;fail_handle=failure==2;
        h=0;int count=cancel_process_async(&process,file->user,owner,0,&h);
        assert(!h && count==(failure==2) && cancel_calls==(failure==2));finish();cases++;
    }
    /* Exact iosb matching and already-terminated/system exclusions. */
    setup();make_async(0,0);make_async(1,0);struct async *a=make_async(2,0);a->is_system=1;
    a=make_async(3,0);a->terminated=1;
    h=0;assert(cancel_process_async(&process,file->user,NULL,0x1001,&h)==1);
    assert(count_asyncs()==3 && cancel_calls==1);finish();cases++;
    setup();a=make_async(0,0);a->canceled=1;
    h=0;assert(cancel_process_async(&process,file->user,NULL,0,&h)==1);
    assert(count_asyncs()==1 && !cancel_calls);finish();cases++;
    printf("PASS %d lifecycle cases; all fixture-owned objects released.\n",cases);
}
