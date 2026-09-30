#ifndef MADEIRA_CONTROLLER_RETRY_H
#define MADEIRA_CONTROLLER_RETRY_H
/* Refresh runs once a second; retry transient failures after 2,4,8,16,30 ticks.
 * Never truncate a bad-size state file (-6) or overwrite another writer. */
typedef struct { unsigned failures, remaining; } MCRetry;
static inline int mc_retry_ready(MCRetry *retry) {
    if (retry->remaining) { --retry->remaining; return 0; }
    return 1;
}
static inline void mc_retry_failed(MCRetry *retry) {
    if (retry->failures < 5) ++retry->failures;
    retry->remaining = retry->failures < 5 ? (1u << retry->failures) : 30u;
}
static inline void mc_retry_reset(MCRetry *retry) { *retry = (MCRetry){0}; }
#endif
