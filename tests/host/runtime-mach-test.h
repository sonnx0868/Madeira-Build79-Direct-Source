/* Dependency substitutes for the production GameRuntimeBridge.m, not a
 * second implementation of its lifecycle policy. No real host thread is killed. */
#include <stdint.h>
typedef unsigned mach_port_t;
typedef unsigned mach_msg_type_number_t;
typedef int kern_return_t;
typedef int thread_basic_info_data_t;
typedef int *thread_info_t;
typedef int pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER 0
#define THREAD_BASIC_INFO_COUNT 1
#define THREAD_BASIC_INFO 1
#define MACH_PORT_RIGHT_SEND 1
#define KERN_SUCCESS 0
#define KERN_INVALID_ARGUMENT 4
#define MACH_SEND_INVALID_DEST 0x10000003
extern unsigned runtime_test_self;
extern int runtime_test_alive[8192], runtime_test_refs[8192];
static inline unsigned pthread_self(void) { return runtime_test_self; }
static inline unsigned pthread_mach_thread_np(unsigned thread) { return thread; }
static inline unsigned mach_task_self(void) { return 1; }
static inline void pthread_mutex_lock(pthread_mutex_t *lock) { (void)lock; }
static inline void pthread_mutex_unlock(pthread_mutex_t *lock) { (void)lock; }
static inline void usleep(unsigned delay) { (void)delay; }
static inline int mach_port_mod_refs(unsigned task, unsigned port, unsigned right, int delta) {
    (void)task; (void)right; runtime_test_refs[port] += delta; return KERN_SUCCESS;
}
static inline int mach_port_deallocate(unsigned task, unsigned port) {
    (void)task; --runtime_test_refs[port]; return KERN_SUCCESS;
}
static inline int thread_info(unsigned port, unsigned flavor, thread_info_t info, unsigned *count) {
    (void)flavor; (void)info; (void)count;
    return runtime_test_alive[port] > 0 ? 0 : runtime_test_alive[port] < 0 ? 5 : KERN_INVALID_ARGUMENT;
}
