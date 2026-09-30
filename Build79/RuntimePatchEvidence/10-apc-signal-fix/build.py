"""Correct only iOS guest signaling's task port; do not enable remote memory paths."""
from pathlib import Path
import difflib,hashlib,json,sys
H=Path(__file__).resolve().parent
sys.path.insert(0,str(H.parent/'guest-exit-fix'))
from build import file_offset
sha=lambda b:hashlib.sha256(b).hexdigest()
BASE=H.parent/'async-cancel-audit/native/Madeira'
BASE_SHA='f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984'
START,END=0x10006c32c,0x10006c340
old=bytes.fromhex('084440f9009941b968fa41b91f0500310418407a')
new=bytes.fromhex('a85a0090084d47f9000140b968fa41b91f050031')
data=BASE.read_bytes();assert sha(data)==BASE_SHA
at=file_offset(data,START,END-START);assert at==0x7032c and data[at:at+20]==old
# Reuse the exact same in-function mach_task_self_ GOT load sequence already
# used for right deallocation. Both ADRP instructions reside in the same page.
assert data[file_offset(data,0x10006c3ac,12):file_offset(data,0x10006c3ac,12)+12]==new[:12]
fixed=data[:at]+new+data[at+20:]
assert len(fixed)==len(data)and fixed[:at]==data[:at]and fixed[at+20:]==data[at+20:]
(H/'Madeira-original').write_bytes(data);(H/'Madeira').write_bytes(fixed)
src=(H/'mach_ios.original.c').read_bytes();assert sha(src)=='d5a1ba1d9e5c33addc00a411bea600d5ce179ad9039ced68b926b7cde6b5c0ef'
before=src.decode();start=before.index('int send_thread_signal(');end=before.index('\n#ifdef WINE_IOS',start)
function=before[start:end]
old_src='''    mach_port_t process_port = get_process_port( thread->process );

    if (thread->unix_pid != -1 && process_port)'''
new_src='''    /* Wine guests are pthreads in this iOS app's Darwin task. The normal
     * cross-process task-port handoff is disabled here. Use our task only
     * for real thread-right extraction and signal delivery; leave
     * get_process_port() disabled for remote-memory operations. */
    mach_port_t process_port = mach_task_self();

    if (thread->unix_pid != -1)'''
assert function.count(old_src)==1
after=before[:start]+function.replace(old_src,new_src)+before[end:]
(H/'mach_ios.fixed.c').write_text(after)
patch=''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='a/build/wineserver/mach_ios.c',tofile='b/build/wineserver/mach_ios.c'))
(H/'source.patch').write_text(patch)
result={'native_baseline_sha256':BASE_SHA,'native_candidate_sha256':sha(fixed),'source_baseline_sha256':sha(src),
 'source_patch_sha256':sha(patch.encode()),'range':[hex(START),hex(END)],'bytes':20,
 'differing_bytes':sum(a!=b for a,b in zip(old,new)),'only_send_thread_signal_changed':True,
 'same_prologue_epilogue_unwind_and_call_error_paths':True,
 'no_get_process_port_or_remote_memory_change':True,'real_mach_right_extraction_and_pthread_signal_retained':True,
 'scope':'Signals sent through this helper include SIGUSR1 system APC and SIGQUIT termination; task_self is guaranteed valid in the live app. This is not fake signal success.',
 'limitation':'Native/host fixtures cannot prove actual iOS signal-handler entry; pinned source records earlier nondelivery despite successful __pthread_kill. Physical status publication and outside-wait delivery tests required.',
 'packaged':False,'installed':False,'device_validated':False,'cef_fixed':False}
(H/'build-report.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
