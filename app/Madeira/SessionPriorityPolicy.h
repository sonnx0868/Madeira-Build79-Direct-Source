/* SPDX-License-Identifier: GPL-3.0-or-later
 * Session ownership policy. Unknown processes and shared services stay at their
 * existing priority. No thread names, CPU affinity or guest memory probing. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef MSP_PROCESS_CAPACITY
#define MSP_PROCESS_CAPACITY 512
#endif
enum { MSP_NORMAL, MSP_BACKGROUND_UI, MSP_SHARED };
typedef struct { unsigned pid, parent; uintptr_t peb; int kind; } MSPProcess;
typedef struct {
    MSPProcess processes[MSP_PROCESS_CAPACITY];
    unsigned target;
    uintptr_t target_peb;
    int active, foreground, presented, incomplete;
} MSPSession;

static inline int msp_image_kind(const uint16_t *path, size_t length) {
    if (!path || !length || length > 2048) return MSP_NORMAL;
    size_t start = 0;
    for (size_t i = 0; i < length; ++i) if (path[i] == '/' || path[i] == '\\') start = i + 1;
    char name[80]; size_t n = length - start;
    if (!n || n >= sizeof(name)) return MSP_NORMAL;
    for (size_t i = 0; i < n; ++i) {
        uint16_t c = path[start+i];
        if (c < 32 || c > 126) return MSP_NORMAL;
        name[i] = (char)(c >= 'A' && c <= 'Z' ? c + 32 : c);
    }
    name[n] = 0;
    static const char *const ui[] = {"steamwebhelper.exe", "epicwebhelper.exe", "galaxyclient helper.exe",
        "chrome.exe", "chromium.exe", "msedge.exe", "firefox.exe"};
    static const char *const shared[] = {"steam.exe", "steamservice.exe", "gameoverlayui.exe",
        "services.exe", "rpcss.exe", "svchost.exe", "explorer.exe", "winedevice.exe", "madeira-runtime.exe"};
    for (size_t i=0;i<sizeof(ui)/sizeof(ui[0]);++i) if (!strcmp(name,ui[i])) return MSP_BACKGROUND_UI;
    for (size_t i=0;i<sizeof(shared)/sizeof(shared[0]);++i) if (!strcmp(name,shared[i])) return MSP_SHARED;
    return MSP_NORMAL;
}
static inline MSPProcess *msp_find(MSPSession *s, unsigned pid, uintptr_t peb) {
    for (unsigned i=0;i<MSP_PROCESS_CAPACITY;++i)
        if (pid && peb && s->processes[i].pid==pid && s->processes[i].peb==peb) return &s->processes[i];
    return NULL;
}
static inline MSPProcess *msp_unique(MSPSession *s, unsigned pid) {
    MSPProcess *result = NULL;
    for (unsigned i=0;i<MSP_PROCESS_CAPACITY;++i) if (pid && s->processes[i].pid==pid) {
        if (result) return NULL;
        result = &s->processes[i];
    }
    return result;
}
static inline int msp_attach(MSPSession *s, unsigned pid, unsigned parent, uintptr_t peb) {
    if (!pid || !peb) return 0;
    MSPProcess *p = msp_find(s,pid,peb);
    if (p) {
        if (parent && !p->parent) { p->parent=parent; return 1; }
        if (parent && p->parent!=parent) { s->incomplete=1; return 1; }
        return 0;
    }
    for (unsigned i=0;i<MSP_PROCESS_CAPACITY;++i) if (!s->processes[i].pid) {
        s->processes[i]=(MSPProcess){pid,parent,peb,MSP_NORMAL}; return 1;
    }
    s->incomplete=1; return 1;
}
/* 1: selected process or descendant; 0: another known tree; -1: uncertain. */
static inline int msp_family(MSPSession *s, MSPProcess *p) {
    unsigned seen[MSP_PROCESS_CAPACITY], count=0;
    while (p && count<MSP_PROCESS_CAPACITY) {
        if (p->pid==s->target && p->peb==s->target_peb) return 1;
        for (unsigned i=0;i<count;++i) if (seen[i]==p->pid) return -1;
        seen[count++]=p->pid;
        if (!p->parent) return 0;
        p=msp_unique(s,p->parent);
    }
    return -1;
}
static inline int msp_role(MSPSession *s, unsigned pid, uintptr_t peb) {
    if (!s->active || !s->foreground || !s->presented || s->incomplete ||
        !msp_find(s,s->target,s->target_peb) || !msp_unique(s,s->target)) return MSP_NORMAL;
    MSPProcess *p=msp_find(s,pid,peb);
    if (!p || !msp_unique(s,pid) || p->kind!=MSP_BACKGROUND_UI || msp_family(s,p)!=0) return MSP_NORMAL;
    return MSP_BACKGROUND_UI;
}
static inline void msp_retire(MSPSession *s, unsigned pid, uintptr_t peb) {
    MSPProcess *p=msp_find(s,pid,peb);
    if (p) memset(p,0,sizeof(*p));
    if (s->target==pid && s->target_peb==peb) { s->target=0; s->target_peb=0; s->presented=0; }
}

/* QoS transitions are cooperative: run only on the owning pthread. An Eco or
 * game-requested lower class is preserved; only our own changes are restored. */
typedef struct { unsigned saved; int relative, owned; } MSPQoS;
typedef struct {
    int (*get)(unsigned *,int *);
    int (*set)(unsigned,int);
    unsigned utility;
} MSPQoSOps;
static inline int msp_qos_apply(MSPQoS *s, int background, int base_changed, const MSPQoSOps *ops) {
    unsigned current=0; int relative=0;
    if (base_changed) s->owned=0; /* explicit Eco change supersedes our saved value */
    if (!background && !s->owned) return 0;
    if (ops->get(&current,&relative)) return -1;
    if (s->owned && (current!=ops->utility || relative!=0)) s->owned=0; /* preserve an external change */
    if (background) {
        if (s->owned || !current || current<=ops->utility) return 0;
        if (ops->set(ops->utility,0)) return -1;
        s->saved=current; s->relative=relative; s->owned=1; return 1;
    }
    if (!s->owned) return 0;
    if (ops->set(s->saved,s->relative)) return -1;
    s->owned=0; return 2;
}
