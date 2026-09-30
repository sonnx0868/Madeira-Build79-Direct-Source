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
