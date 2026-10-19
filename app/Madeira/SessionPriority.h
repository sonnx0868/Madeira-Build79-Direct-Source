/* Included once by WineProcessBridge.m. Native hooks call this on their own
 * pthreads, independent of Reusable runtime and diagnostic collection. */
#include "SessionPriorityPolicy.h"
#include <stdatomic.h>
#include <pthread/qos.h>

static MSPSession msp_session;
static pthread_mutex_t msp_lock=PTHREAD_MUTEX_INITIALIZER;
static _Atomic uint64_t msp_epoch=1;
static _Atomic int msp_native_ready;
static _Atomic int msp_waiting_for_surface;
static unsigned msp_surface;
static int msp_frames;
static _Thread_local struct { unsigned pid,tid; uintptr_t peb; MSPQoS qos; } msp_self;

const uint64_t *madeira_session_priority_epoch_address(void) {
    static _Atomic int announced;
    if (!atomic_exchange(&announced,1)) {
        atomic_store(&msp_native_ready,1);
        atomic_fetch_add_explicit(&msp_epoch,1,memory_order_release);
        fprintf(stderr,"[session-priority-v1] native-wait-entry-exit-hook=1\n");
    }
    return (const uint64_t *)&msp_epoch;
}
static void msp_changed(void) { atomic_fetch_add_explicit(&msp_epoch,1,memory_order_release); }
static int msp_get_qos(unsigned *qos,int *relative) {
    qos_class_t value=QOS_CLASS_UNSPECIFIED;
    int rc=pthread_get_qos_class_np(pthread_self(),&value,relative); *qos=value; return rc;
}
static int msp_set_qos(unsigned qos,int relative) { return pthread_set_qos_class_self_np(qos,relative); }
void madeira_session_priority_poll_current(int base_changed) {
    if (!msp_self.pid || !msp_self.peb) return;
    pthread_mutex_lock(&msp_lock);
    int role=atomic_load(&msp_native_ready)?msp_role(&msp_session,msp_self.pid,msp_self.peb):MSP_NORMAL;
    pthread_mutex_unlock(&msp_lock);
    static const MSPQoSOps ops={msp_get_qos,msp_set_qos,QOS_CLASS_UTILITY};
    int result=msp_qos_apply(&msp_self.qos,role==MSP_BACKGROUND_UI,base_changed,&ops);
    if (result) {
        unsigned actual=0; int relative=0; int read=msp_get_qos(&actual,&relative);
        fprintf(stderr,"[session-priority-v1] pid=%04x tid=%04x action=%s qos=%#x relative=%d read_rc=%d own-thread=1\n",
            msp_self.pid,msp_self.tid,result==1?"background-ui":result==2?"restore":"failed",actual,relative,read);
    }
}
void madeira_session_priority_thread(unsigned pid,unsigned tid,unsigned parent,void *peb) {
    if (!pid || !tid || !peb) return;
    if (msp_self.pid && (msp_self.pid!=pid || msp_self.peb!=(uintptr_t)peb)) {
        static const MSPQoSOps ops={msp_get_qos,msp_set_qos,QOS_CLASS_UTILITY};
        if (msp_qos_apply(&msp_self.qos,0,0,&ops)<0) return;
    }
    msp_self.pid=pid; msp_self.tid=tid; msp_self.peb=(uintptr_t)peb;
    pthread_mutex_lock(&msp_lock);
    if (msp_attach(&msp_session,pid,parent,(uintptr_t)peb)) msp_changed();
    pthread_mutex_unlock(&msp_lock);
    madeira_session_priority_poll_current(0);
}
void madeira_session_priority_image(unsigned pid,const uint16_t *image,size_t length) {
    if (msp_self.pid!=pid || !msp_self.peb) return;
    int kind=msp_image_kind(image,length);
    pthread_mutex_lock(&msp_lock);
    MSPProcess *p=msp_find(&msp_session,pid,msp_self.peb);
    if (p && p->kind!=kind) { p->kind=kind; msp_changed(); }
    /* CreateProcess may announce the selected image on its parent before the
     * child's pthread exists. Bind its identity only from that child itself. */
    if (p && msp_session.active && msp_session.target==pid && !msp_session.target_peb) {
        msp_session.target_peb=msp_self.peb; msp_changed();
    }
    pthread_mutex_unlock(&msp_lock);
    madeira_session_priority_poll_current(0);
}
void madeira_session_priority_begin(void) {
    pthread_mutex_lock(&msp_lock);
    msp_session.active=1; msp_session.target=0; msp_session.target_peb=0; msp_session.presented=0;
    msp_surface=0; msp_frames=0; atomic_store(&msp_waiting_for_surface,0);
    msp_changed(); pthread_mutex_unlock(&msp_lock);
    fprintf(stderr,"[session-priority-v1] begin native-wait-poll=required background=known-ui-only shared-services=preserved\n");
}
void madeira_session_priority_select(unsigned pid) {
    if (!pid) return;
    pthread_mutex_lock(&msp_lock);
    if (msp_session.active) {
        msp_session.target=pid;
        msp_session.target_peb=msp_self.pid==pid?msp_self.peb:0;
        msp_session.presented=0; msp_changed();
        atomic_store(&msp_waiting_for_surface,1);
    }
    pthread_mutex_unlock(&msp_lock);
    madeira_session_priority_poll_current(0);
}
void madeira_session_priority_presented(unsigned pid) {
    if (!atomic_load_explicit(&msp_waiting_for_surface,memory_order_relaxed)) return;
    pthread_mutex_lock(&msp_lock);
    MSPProcess *p=msp_unique(&msp_session,pid);
    if (msp_session.active && !msp_session.presented && p && msp_family(&msp_session,p)==1) {
        msp_surface=pid;
        if (msp_frames) { msp_session.presented=1; msp_changed(); atomic_store(&msp_waiting_for_surface,0); }
    }
    pthread_mutex_unlock(&msp_lock);
}
void madeira_session_priority_rendering(void) {
    if (!atomic_load_explicit(&msp_waiting_for_surface,memory_order_relaxed)) return;
    pthread_mutex_lock(&msp_lock);
    msp_frames=1;
    MSPProcess *p=msp_unique(&msp_session,msp_surface);
    unsigned selected=0;
    if (msp_session.active && p && msp_family(&msp_session,p)==1 && !msp_session.presented) {
        msp_session.presented=1; selected=msp_session.target; msp_changed();
        atomic_store(&msp_waiting_for_surface,0);
    }
    pthread_mutex_unlock(&msp_lock);
    if (selected) fprintf(stderr,"[session-priority-v1] foreground=%04x rendered=1 ready=1\n",selected);
}
void madeira_session_priority_foreground(int active) {
    pthread_mutex_lock(&msp_lock);
    if (msp_session.foreground!=!!active) { msp_session.foreground=!!active; msp_changed(); }
    pthread_mutex_unlock(&msp_lock);
}
void madeira_session_priority_end(void) {
    pthread_mutex_lock(&msp_lock);
    msp_session.active=0; msp_session.presented=0; msp_session.target=0; msp_session.target_peb=0;
    atomic_store(&msp_waiting_for_surface,0);
    msp_changed(); pthread_mutex_unlock(&msp_lock);
    fprintf(stderr,"[session-priority-v1] end restore=before-next-guest-return\n");
}
void madeira_session_priority_retired(unsigned pid,void *peb) {
    pthread_mutex_lock(&msp_lock); msp_retire(&msp_session,pid,(uintptr_t)peb); msp_changed(); pthread_mutex_unlock(&msp_lock);
}
