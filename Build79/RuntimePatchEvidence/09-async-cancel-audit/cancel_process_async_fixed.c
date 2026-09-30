/* Proposed isolated replacement, not installed. Source basis and tests in README.md. */
static int cancel_process_async( struct process *process, struct object *obj, struct thread *thread, client_ptr_t iosb, obj_handle_t *wait_handle )
{
    struct async_cancel *cancel = NULL;
    struct async *async;
    struct list tracked;
    int count = 0;

    if (thread && !(cancel = create_async_cancel( process ))) return 0;
    /* The construction sentinel keeps synchronous completions from destroying
     * this group while the remaining operations are being added. */
    if (cancel) cancel->count = 1;

    list_init( &tracked );

restart:
    LIST_FOR_EACH_ENTRY( async, &process->asyncs, struct async, process_entry )
    {
        if (async->terminated || async->is_system) continue;
        if ((!obj || (get_fd_user( async->fd ) == obj)) &&
            (!thread || async->thread == thread) &&
            (!iosb || async->data.iosb == iosb))
        {
            /* Cancellation and reselect may complete this or earlier tracked
             * operations. Retain every tracked entry until it is restored. */
            grab_object( async );
            if (cancel)
            {
                assert( !async->async_cancel );
                async->async_cancel = cancel;
                cancel->count++;
            }
            /* Publish group membership before a callback can complete it. */
            if (!async->canceled) cancel_async( async );
            list_remove( &async->process_entry );
            list_add_tail( &tracked, &async->process_entry );
            count++;
            goto restart;
        }
    }
    while (!list_empty( &tracked ))
    {
        async = LIST_ENTRY( list_head( &tracked ), struct async, process_entry );
        list_remove( &async->process_entry );
        list_add_tail( &process->asyncs, &async->process_entry );
        release_object( async );
    }
    if (cancel)
    {
        if (!--cancel->count) release_object( cancel );
        else *wait_handle = alloc_handle( process, cancel, SYNCHRONIZE, 0 );
    }
    return count;
}
