/* App-side, passive REPLACED transition measurements. Included once by
 * WineProcessBridge.m. No thread suspension, affinity, guest writes or native
 * code changes. Existing PE ntdll counters time completed critical-section
 * waits; they do not measure every Windows/Metal wait or time still blocked. */
#include "PerformanceCounters.h"
#include <stddef.h>

extern int proc_pidinfo(int, int, uint64_t, void *, int);
extern void *ios_jit_translate_addr_for_owner(void *, void *);
static uint32_t replaced_metrics_target_pid(void);
enum { RM_MAX_THREADS = 128, RM_MAX_LEVELS = 4 };
typedef struct { uint64_t instructions,cycles,user_ticks,system_ticks,energy_nj; } RMRawCounts;
typedef struct { uint16_t len, reserved; uint32_t reserved2; RMRawCounts counts[RM_MAX_LEVELS]; } RMThreadCounts;
_Static_assert(sizeof(RMRawCounts)==40 && offsetof(RMThreadCounts,counts)==8,"Darwin threadcounts ABI");
static MadeiraPerfCounters rm_thread_counters(RMRawCounts raw) {
    return (MadeiraPerfCounters){raw.user_ticks+raw.system_ticks,raw.cycles,raw.instructions,raw.energy_nj};
}
typedef struct {
    uint64_t magic, frequency, contended, spin0, spin_acquired, wait_ticks, histogram[6], wakes;
    uint64_t address_waits, wake_single, wake_all, ring_index;
} RMSyncCounters;
_Static_assert(sizeof(RMSyncCounters) == 136, "PE ntdll counter ABI");
typedef struct {
    unsigned pid, tid;
    uint64_t native_id, peb;
    mach_port_t port;
    int previous_valid;
    RMThreadCounts previous;
    MadeiraPerfDelta delta[RM_MAX_LEVELS];
    char name[64];
    int priority;
} RMThread;
static RMThread rm_threads[RM_MAX_THREADS];
static unsigned rm_count, rm_dropped, rm_reports;
static unsigned rm_generation;
static int rm_active, rm_mapping;
static char rm_level_names[2][32];
static struct rusage_info_v6 rm_previous_usage;
static int rm_previous_valid;
static double rm_previous_uptime;
static uint64_t rm_session_energy;
static uint64_t rm_sync_address, rm_sync_peb;
static uint32_t rm_sync_span;
static RMSyncCounters rm_previous_sync;
static int rm_previous_sync_valid;

static int rm_read(uint64_t address, void *out, size_t size) {
    _Static_assert(sizeof(vm_address_t)==8 && sizeof(vm_size_t)==8,"64-bit Mach VM addresses required");
    vm_size_t got = 0;
    if (!address || address > UINT64_MAX-size) return 0;
    return vm_read_overwrite(mach_task_self(), (vm_address_t)address, (vm_size_t)size,
        (vm_address_t)out, &got) == KERN_SUCCESS && got == size;
}
static uint32_t rm_u32(const unsigned char *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static int rm_rva(uint64_t base, uint32_t image_size, uint32_t rva, void *out, size_t size) {
    return base<=UINT64_MAX-rva && rva < image_size && size <= image_size-rva && rm_read(base+rva, out, size);
}
/* Bounds apply before every remote read, including export-table lengths.
 * Only one named DATA export is resolved. Never calls an ARM64EC export. */
static uint64_t rm_sync_export(uint64_t base, uint32_t image_size) {
    unsigned char dos[64], pe[264]; uint32_t table[10];
    if (image_size < 512 || !rm_read(base,dos,sizeof(dos)) || memcmp(dos,"MZ",2)) return 0;
    uint32_t off=rm_u32(dos+60);
    if (off>65536 || !rm_rva(base,image_size,off,pe,sizeof(pe)) || memcmp(pe,"PE\0\0",4) ||
        pe[24]!=0x0b || pe[25]!=0x02) return 0;
    uint32_t er=rm_u32(pe+24+112), es=rm_u32(pe+24+116);
    if (es<40 || !rm_rva(base,image_size,er,table,sizeof(table)) || table[6]>16384 || table[5]>16384) return 0;
    for (uint32_t i=0;i<table[6];++i) {
        uint32_t nr=0,fr=0; uint16_t ordinal=0; char name[10];
        if (table[8]>UINT32_MAX-i*4 || !rm_rva(base,image_size,table[8]+i*4,&nr,4) ||
            !rm_rva(base,image_size,nr,name,sizeof(name))) return 0;
        if (memcmp(name,"ios_xp_nt\0",10)) continue;
        if (table[9]>UINT32_MAX-i*2 || !rm_rva(base,image_size,table[9]+i*2,&ordinal,2) || ordinal>=table[5] ||
            table[7]>UINT32_MAX-(uint32_t)ordinal*4 ||
            !rm_rva(base,image_size,table[7]+(uint32_t)ordinal*4,&fr,4)) return 0;
        if (fr>=er && (uint64_t)fr<(uint64_t)er+es) return 0; /* forwarded export */
        if (fr>=image_size || sizeof(RMSyncCounters)>image_size-fr) return 0;
        rm_sync_span=image_size-fr;
        return base+fr;
    }
    return 0;
}
static uint64_t rm_find_sync(uint64_t peb) {
    uint64_t ldr=0,entry=0;
    if (!rm_read(peb+0x18,&ldr,8) || !ldr || !rm_read(ldr+0x10,&entry,8)) return 0;
    for (unsigned i=0;i<160 && entry && entry!=ldr+0x10;++i) {
        uint64_t base=0,buf=0,next=0; uint32_t size=0; uint16_t len=0,name[10]={0};
        if (!rm_read(entry+0x30,&base,8) || !rm_read(entry+0x40,&size,4) ||
            !rm_read(entry+0x58,&len,2) || !rm_read(entry+0x60,&buf,8)) return 0;
        if (len==18 && rm_read(buf,name,18)) {
            const char wanted[]="ntdll.dll"; int match=1;
            for (unsigned j=0;j<9;++j) if ((name[j]|0x20)!=(unsigned char)wanted[j]) match=0;
            if (match) return rm_sync_export(base,size);
        }
        if (!rm_read(entry,&next,8) || next==entry) break;
        entry=next;
    }
    return 0;
}
/* ARM64EC images execute from their JIT-pool copies. PC-relative accesses
 * update that copy's DATA, while the original PE mapping remains stale.
 * Use the existing native owner-aware translator, as server_ios.c does;
 * never execute the PE export or change its counters. */
static uint64_t rm_find_live_sync(uint64_t peb) {
    uint64_t pe=rm_find_sync(peb);
    if (!pe) return 0;
    uint64_t live=(uint64_t)ios_jit_translate_addr_for_owner((void *)(uintptr_t)pe,(void *)(uintptr_t)peb);
    RMSyncCounters counters={0};
    if (!rm_read(live,&counters,sizeof(counters)) ||
        counters.magic!=UINT64_C(0x31544e5058444d41) || !counters.frequency ||
        counters.frequency>UINT64_C(10000000000)) return 0;
    fprintf(stderr,"[replaced-sync-map-v2] peb=%#llx pe=%#llx live=%#llx translated=%d\n",
        (unsigned long long)peb,(unsigned long long)pe,(unsigned long long)live,pe!=live);
    return live;
}
static void rm_clear(void) {
    for (unsigned i=0;i<rm_count;++i) if(rm_threads[i].port) mach_port_deallocate(mach_task_self(),rm_threads[i].port);
    memset(rm_threads,0,sizeof(rm_threads)); rm_count=rm_dropped=rm_reports=0;
    rm_previous_valid=rm_previous_sync_valid=0;rm_sync_address=rm_sync_peb=0;
}
void madeira_replaced_metrics_prepare(int enabled) {
    __atomic_store_n(&rm_active,0,__ATOMIC_RELEASE);
    unsigned generation=__atomic_add_fetch(&rm_generation,1,__ATOMIC_ACQ_REL);
    dispatch_sync(stall_queue(), ^{
        rm_clear(); rm_mapping=0;
        if (!enabled) return;
        size_t size=sizeof(rm_level_names[0]);strcpy(rm_level_names[0],"unknown");strcpy(rm_level_names[1],"unknown");
        int a=sysctlbyname("hw.perflevel0.name",rm_level_names[0],&size,NULL,0);
        size=sizeof(rm_level_names[1]);int b=sysctlbyname("hw.perflevel1.name",rm_level_names[1],&size,NULL,0);
        rm_level_names[0][31]=rm_level_names[1][31]=0;
        rm_mapping=!a && !b && !strcasecmp(rm_level_names[0],"Performance") && !strcasecmp(rm_level_names[1],"Efficiency");
        memset(&rm_previous_usage,0,sizeof(rm_previous_usage));
        rm_previous_valid=proc_pid_rusage(getpid(),6,&rm_previous_usage)==0;
        rm_session_energy=rm_previous_usage.ri_energy_nj;
        rm_previous_uptime=CACurrentMediaTime();
        __atomic_store_n(&rm_active,generation!=0,__ATOMIC_RELEASE);
        fprintf(stderr,"[replaced-metrics-v1] armed generation=%u cadence=5s max-windows=120 max-threads=%d level0=%s level1=%s mapping_valid=%d rusage_v6=%d clock=cycles-per-running-ns sync=completed-critical-section-waits\n",
            generation,RM_MAX_THREADS,rm_level_names[0],rm_level_names[1],rm_mapping,rm_previous_valid);
    });
}
/* The already-linked Wine lifecycle callback calls this for both direct and
 * reusable launches. The registry is independent of the reusable-engine flag. */
void madeira_replaced_metrics_thread(unsigned pid,unsigned tid,void *peb) {
    if (!__atomic_load_n(&rm_active,__ATOMIC_ACQUIRE) || !pid || !tid || !peb) return;
    unsigned generation=__atomic_load_n(&rm_generation,__ATOMIC_ACQUIRE);
    uint64_t native=0;pthread_threadid_np(NULL,&native);
    mach_port_t port=mach_thread_self();
    dispatch_async(stall_queue(), ^{
        if (!__atomic_load_n(&rm_active,__ATOMIC_ACQUIRE) || generation!=__atomic_load_n(&rm_generation,__ATOMIC_ACQUIRE) || !native) {
            mach_port_deallocate(mach_task_self(),port);return;
        }
        for(unsigned i=0;i<rm_count;++i) if(rm_threads[i].native_id==native) {mach_port_deallocate(mach_task_self(),port);return;}
        if(rm_count==RM_MAX_THREADS) {++rm_dropped;mach_port_deallocate(mach_task_self(),port);return;}
        RMThread *t=&rm_threads[rm_count++];
        t->pid=pid;t->tid=tid;t->native_id=native;t->peb=(uint64_t)peb;t->port=port;
        int got=proc_pidinfo(getpid(),34,native,&t->previous,sizeof(t->previous));
        t->previous_valid=got>=8 && t->previous.len>0 && t->previous.len<=RM_MAX_LEVELS && got>=8+(int)t->previous.len*40;
    });
}
static MadeiraPerfCounters rm_usage_total(const struct rusage_info_v6 *u) {
    return (MadeiraPerfCounters){u->ri_user_time+u->ri_system_time,u->ri_cycles,u->ri_instructions,u->ri_energy_nj};
}
static MadeiraPerfCounters rm_usage_p(const struct rusage_info_v6 *u) {
    return (MadeiraPerfCounters){u->ri_user_ptime+u->ri_system_ptime,u->ri_pcycles,u->ri_pinstructions,u->ri_penergy_nj};
}
static void rm_sync_sample(uint32_t pid,uint64_t peb,double uptime,double seconds) {
    if (!peb) {fprintf(stderr,"[replaced-sync-v1] uptime=%.3f pid=%04x available=0 reason=no-game-peb\n",uptime,pid);return;}
    if (peb!=rm_sync_peb) {rm_sync_peb=peb;rm_sync_address=0;rm_sync_span=0;rm_previous_sync_valid=0;}
    if (!rm_sync_address) rm_sync_address=rm_find_live_sync(peb);
    RMSyncCounters now={0};
    if (!rm_sync_address || !rm_read(rm_sync_address,&now,sizeof(now)) ||
        now.magic!=UINT64_C(0x31544e5058444d41) || !now.frequency || now.frequency>UINT64_C(10000000000)) {
        rm_sync_address=0;rm_previous_sync_valid=0;
        fprintf(stderr,"[replaced-sync-v1] uptime=%.3f pid=%04x available=0 reason=counter-export-not-ready\n",uptime,pid);return;
    }
    RMSyncCounters *old=&rm_previous_sync;
    int valid=rm_previous_sync_valid && old->frequency==now.frequency && now.contended>=old->contended && now.wait_ticks>=old->wait_ticks;
    if (valid) {
        fprintf(stderr,"[replaced-sync-v1] uptime=%.3f window_ms=%.0f pid=%04x available=1 scope=game-process completed_cs_wait_ms=%.3f contended=%llu spin_acquired=%llu address_wait_calls=%llu hist_lt2us=%llu lt10us=%llu lt50us=%llu lt200us=%llu lt1ms=%llu ge1ms=%llu\n",
            uptime,seconds*1000,pid,(double)(now.wait_ticks-old->wait_ticks)*1000/now.frequency,
            (unsigned long long)(now.contended-old->contended),(unsigned long long)(now.spin_acquired-old->spin_acquired),
            (unsigned long long)(now.address_waits-old->address_waits),
            (unsigned long long)(now.histogram[0]-old->histogram[0]),(unsigned long long)(now.histogram[1]-old->histogram[1]),
            (unsigned long long)(now.histogram[2]-old->histogram[2]),(unsigned long long)(now.histogram[3]-old->histogram[3]),
            (unsigned long long)(now.histogram[4]-old->histogram[4]),(unsigned long long)(now.histogram[5]-old->histogram[5]));
        uint64_t ring[1024];
        if (now.ring_index>=old->ring_index && rm_sync_span>=sizeof(now)+sizeof(ring) &&
            rm_read(rm_sync_address+sizeof(now),ring,sizeof(ring))) {
            uint64_t available=now.ring_index/4-old->ring_index/4;
            unsigned take=(unsigned)(available<128?available:128),used=0;
            struct {uint64_t address;unsigned count;} top[128]={0};
            for(unsigned j=0;j<take;++j) {
                uint64_t address=ring[(now.ring_index/4-j)&1023];
                if(!address) continue;
                unsigned k;for(k=0;k<used && top[k].address!=address;++k) {}
                if(k==used) {top[used++].address=address;}
                ++top[k].count;
            }
            for(unsigned n=0;n<3;++n) {
                int best=-1;for(unsigned k=0;k<used;++k) if(top[k].count && (best<0 || top[k].count>top[best].count)) best=(int)k;
                if(best<0) break;
                uint64_t owner=0;int32_t lock[2]={0};
                int readable=rm_read(top[best].address+8,lock,sizeof(lock)) && rm_read(top[best].address+16,&owner,8);
                fprintf(stderr,"[replaced-sync-lock-v1] uptime=%.3f pid=%04x address=%#llx sampled_calls=%u samples_considered=%u truncated=%d owner_now=%04llx lock_count_now=%d recursion_now=%d readable=%d\n",
                    uptime,pid,(unsigned long long)top[best].address,top[best].count,take,available>take,
                    (unsigned long long)owner,lock[0],lock[1],readable);
                top[best].count=0;
            }
        }
    } else fprintf(stderr,"[replaced-sync-v1] uptime=%.3f pid=%04x available=1 baseline=1\n",uptime,pid);
    rm_previous_sync=now;rm_previous_sync_valid=1;
}
static void replaced_metrics_snapshot(double uptime,double fps) {
    if (!__atomic_load_n(&rm_active,__ATOMIC_ACQUIRE) || rm_reports++>=120) return;
    double started=CACurrentMediaTime(),seconds=uptime-rm_previous_uptime;
    mach_timebase_info_data_t tb;mach_timebase_info(&tb);
    uint32_t pid=replaced_metrics_target_pid(); uint64_t peb=0;
    struct rusage_info_v6 usage={0};int usageStatus=proc_pid_rusage(getpid(),6,&usage);
    if (!usageStatus && rm_previous_valid && seconds>0) {
        MadeiraPerfCounters p0=rm_usage_p(&rm_previous_usage),p1=rm_usage_p(&usage),e0={0},e1={0};
        MadeiraPerfDelta p=madeira_perf_delta(p0,p1,tb.numer,tb.denom),e={.ghz=-1,.ipc=-1};
        int valid=madeira_perf_subtract(rm_usage_total(&rm_previous_usage),p0,&e0) && madeira_perf_subtract(rm_usage_total(&usage),p1,&e1);
        if(valid) e=madeira_perf_delta(e0,e1,tb.numer,tb.denom);
        MadeiraPerfDelta total=madeira_perf_delta(rm_usage_total(&rm_previous_usage),rm_usage_total(&usage),tb.numer,tb.denom);
        // Some CI/virtual devices accept rusage v6 but expose no hardware
        // counters. Zero P time there is not evidence that all work ran on E.
        int partitionAvailable=valid && p.valid && e.valid && total.rate_valid;
        double energy=usage.ri_energy_nj>=rm_previous_usage.ri_energy_nj ? (double)(usage.ri_energy_nj-rm_previous_usage.ri_energy_nj)/1e6 : -1;
        double runnable=usage.ri_runnable_time>=rm_previous_usage.ri_runnable_time ? (double)(usage.ri_runnable_time-rm_previous_usage.ri_runnable_time)*tb.numer/tb.denom/1e6 : -1;
        fprintf(stderr,"[replaced-cpu-v1] uptime=%.3f window_ms=%.0f fps=%.1f scope=app valid=%d cpu_ms=%.2f partition_available=%d P_ms=%.2f E_ms=%.2f P_GHz=%.3f E_GHz=%.3f P_rate_valid=%d E_rate_valid=%d P_IPC=%.3f E_IPC=%.3f runnable_ms=%.2f energy_est_mJ=%.2f power_est_W=%.3f session_energy_est_J=%.3f energy_available=%d thermal=%ld low_power=%d\n",
            uptime,seconds*1000,fps,total.valid,total.cpu_ms,partitionAvailable,
            partitionAvailable?p.cpu_ms:-1,partitionAvailable?e.cpu_ms:-1,p.ghz,e.ghz,p.rate_valid,e.rate_valid,p.ipc,e.ipc,runnable,
            energy,energy>=0?energy/1000/seconds:-1,usage.ri_energy_nj>=rm_session_energy?(double)(usage.ri_energy_nj-rm_session_energy)/1e9:-1,
            usage.ri_energy_nj>0,(long)NSProcessInfo.processInfo.thermalState,NSProcessInfo.processInfo.lowPowerModeEnabled);
    } else fprintf(stderr,"[replaced-cpu-v1] uptime=%.3f available=0 rusage_rc=%d baseline=%d\n",uptime,usageStatus,rm_previous_valid);
    rm_previous_usage=usage;rm_previous_valid=usageStatus==0;
    unsigned validThreads=0,failed=0;
    for(unsigned i=0;i<rm_count;++i) {
        RMThread *t=&rm_threads[i];RMThreadCounts now={0};
        int got=proc_pidinfo(getpid(),34,t->native_id,&now,sizeof(now));
        int ok=got>=8 && now.len>0 && now.len<=RM_MAX_LEVELS && got>=8+(int)now.len*40;
        memset(t->delta,0,sizeof(t->delta));
        for(unsigned l=0;l<RM_MAX_LEVELS;++l) t->delta[l].ghz=t->delta[l].ipc=-1;
        if (ok && t->previous_valid && now.len==t->previous.len) {
            ++validThreads;
            for(unsigned l=0;l<now.len;++l) t->delta[l]=madeira_perf_delta(rm_thread_counters(t->previous.counts[l]),rm_thread_counters(now.counts[l]),tb.numer,tb.denom);
        } else ++failed;
        t->previous=now;t->previous_valid=ok;
        if(t->pid==pid && !peb) peb=t->peb;
    }
    int shown[RM_MAX_THREADS]={0};
    for(unsigned n=0;n<8;++n) {
        int best=-1;double maximum=0;
        for(unsigned i=0;i<rm_count;++i) if(!shown[i] && rm_threads[i].pid==pid) {
            double ms=0;for(unsigned l=0;l<RM_MAX_LEVELS;++l) ms+=rm_threads[i].delta[l].cpu_ms;
            if(ms>maximum) {maximum=ms;best=(int)i;}
        }
        if(best<0 || maximum<0.1) break;
        shown[best]=1;RMThread *t=&rm_threads[best];
        thread_extended_info_data_t info={0};mach_msg_type_number_t count=THREAD_EXTENDED_INFO_COUNT;
        int rc=thread_info(t->port,THREAD_EXTENDED_INFO,(thread_info_t)&info,&count);
        MadeiraPerfDelta *a=&t->delta[0],*b=&t->delta[1];
        fprintf(stderr,"[replaced-thread-v1] uptime=%.3f pid=%04x tid=%04x native_id=%llu name=%.48s levels=%u mapping_valid=%d level0_ms=%.3f level1_ms=%.3f level0_GHz=%.3f level1_GHz=%.3f level0_rate_valid=%d level1_rate_valid=%d instructions_M=%.2f priority=%d current_priority=%d info_rc=%d\n",
            uptime,t->pid,t->tid,(unsigned long long)t->native_id,info.pth_name,t->previous.len,rm_mapping,
            a->cpu_ms,b->cpu_ms,a->ghz,b->ghz,a->rate_valid,b->rate_valid,a->million_instructions+b->million_instructions,
            info.pth_priority,info.pth_curpri,rc);
    }
    rm_sync_sample(pid,peb,uptime,seconds);
    rm_previous_uptime=uptime;
    fprintf(stderr,"[replaced-metrics-cost-v1] uptime=%.3f elapsed_ms=%.3f registered=%u valid_threads=%u unavailable_threads=%u dropped=%u\n",
        uptime,(CACurrentMediaTime()-started)*1000,rm_count,validThreads,failed,rm_dropped);
}
