#ifndef MADEIRA_SYNC_SUSPEND_GUARD_H
#define MADEIRA_SYNC_SUSPEND_GUARD_H
#include <signal.h>
struct madsync_suspend_guard { sigset_t saved; int backend; };
void madsync_suspend_begin(struct madsync_suspend_guard *guard);
void madsync_suspend_end(struct madsync_suspend_guard *guard);
#endif
