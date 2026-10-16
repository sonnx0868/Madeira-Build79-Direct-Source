/* Yield to another runnable thread without the separate priority depression
 * performed by Darwin's default sched_yield/swtch_pri path. The existing
 * adaptive park in NtYieldExecution still bounds sustained polling. */
static inline NTSTATUS ios_scheduler_yield(void)
{
    return thread_switch( MACH_PORT_NULL, SWITCH_OPTION_NONE, 0 ) == KERN_SUCCESS
        ? STATUS_SUCCESS : STATUS_NO_YIELD_PERFORMED;
}
