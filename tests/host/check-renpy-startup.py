#!/usr/bin/env python3
"""Real portable policies for the three failures observed in CrossRealms 612."""
from pathlib import Path
import os
import platform
import shutil
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
folder = root / "build/ntdll-unix"
loader = (folder / "loader_ios.c").read_text(encoding="utf-8")
server = (folder / "server_ios.c").read_text(encoding="utf-8")
thread = (folder / "thread_ios.c").read_text(encoding="utf-8")
virtual = (folder / "virtual_ios.c").read_text(encoding="utf-8")
assert "ios_child_needs_private_ntdll(ios_cur_image_info()->Machine, is_arm64ec())" in loader
assert "get_pe_dir( machine )" in loader and "ios_load_child_private_ntdll" in loader
assert server.count("ios_process_thread_register(") == 3
assert "ios_process_thread_retire(session_pid, runtime_peb)" in server
assert thread.count("ios_depart_current_thread() == 0") == 2
assert "ios_reloc_sync_dir64" in virtual and "while (block < block_end)" not in virtual[virtual.index("NTSTATUS WINAPI NtProtectVirtualMemory") : virtual.index("static struct file_view *get_memory_region_size")]
native = root / "app/Madeira/aarch64-windows/ntdll.dll"
data = native.read_bytes()
pe = struct.unpack_from("<I", data, 60)[0]
assert data[pe:pe+4] == b"PE\0\0" and struct.unpack_from("<H", data, pe+4)[0] == 0xaa64
fixture = r'''
#define IOS_PROCESS_THREAD_CAPACITY 16
#include "ios_process_threads.h"
#include "ios_child_ntdll.h"
#include "ios_pe_relocations.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
static struct ios_process_threads counts = { .lock = PTHREAD_MUTEX_INITIALIZER };
static struct ios_process_threads overflow = { .lock = PTHREAD_MUTEX_INITIALIZER };
static void *worker(void *arg) {
    unsigned tid = (unsigned)(uintptr_t)arg;
    assert(ios_process_threads_add(&counts, 1, tid, (void *)1)); // reserve/attach deduplicates
    assert(ios_process_threads_leave(&counts, 1, tid, (void *)1) >= 1);
    return NULL;
}
static void u32(uint8_t *image, size_t off, uint32_t v) { memcpy(image+off,&v,4); }
static void u16(uint8_t *image, size_t off, uint16_t v) { memcpy(image+off,&v,2); }
static uint64_t u64(uint8_t *image, size_t off) { uint64_t v; memcpy(&v,image+off,8); return v; }
static void fresh(uint8_t *image) {
    memset(image,0,256); u32(image,0x44,12); u16(image,0x48,0xa083);
    uint64_t value = 0x10000083; memcpy(image+0x83,&value,8);
}
int main(void) {
    assert(ios_child_needs_private_ntdll(0xaa64,1));
    assert(ios_child_needs_private_ntdll(0x8664,0));
    assert(!ios_child_needs_private_ntdll(0xaa64,0));
    assert(!ios_child_needs_private_ntdll(0x8664,1));
    assert(!ios_child_needs_private_ntdll(0x14c,0));
    assert(!ios_child_needs_private_ntdll(0x14c,1));
    assert(!ios_child_needs_private_ntdll(0,1));
    assert(ios_process_threads_add(&counts,1,10,(void *)1));
    assert(ios_process_threads_add(&counts,1,11,(void *)1));
    for(unsigned pid=2;pid<8;++pid) {
        assert(ios_process_threads_add(&counts,pid,pid*10,(void *)(uintptr_t)pid));
        assert(ios_process_threads_leave(&counts,pid,pid*10,(void *)(uintptr_t)pid)==0);
    }
    assert(ios_process_threads_leave(&counts,1,11,(void *)1)==1); // helper exits cannot spend parent's count
    assert(ios_process_threads_leave(&counts,1,11,(void *)1)==-1); // duplicate exit cannot kill parent
    assert(ios_process_threads_leave(&counts,1,999,(void *)1)==-1);
    pthread_t workers[8];
    for(unsigned i=0;i<8;++i) {
        assert(ios_process_threads_add(&counts,1,100+i,(void *)1));
        assert(!pthread_create(workers+i,NULL,worker,(void *)(uintptr_t)(100+i)));
    }
    for(unsigned i=0;i<8;++i) assert(!pthread_join(workers[i],NULL));
    assert(ios_process_threads_leave(&counts,1,10,(void *)1)==0);
    assert(ios_process_threads_add(&counts,1,10,(void *)2));
    assert(ios_process_threads_leave(&counts,1,10,(void *)1)==-1); // recycled PID, different generation
    ios_process_threads_remove(&counts,1,(void *)2);
    assert(ios_process_threads_leave(&counts,1,10,(void *)2)==-1);
    for(unsigned i=0;i<16;++i) assert(ios_process_threads_add(&overflow,i+1,i+1,(void *)1));
    assert(!ios_process_threads_add(&overflow,99,99,(void *)1));
    assert(ios_process_threads_leave(&overflow,1,1,(void *)1)==-1); // incomplete tracking is not zero
    uint8_t image[256]; fresh(image);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,0x10000000,0x80,16)==1);
    assert(u64(image,0x83)==0x20000083); // unaligned valid fixup
    fresh(image);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,0x10000000,0x90,16)==0);
    assert(u64(image,0x83)==0x10000083);
    fresh(image); u32(image,0x44,0xaa1503e2); // exact invalid block size in the fault registers
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,1,0,256)==-1);
    fresh(image);
    assert(ios_reloc_sync_dir64(image,256,240,32,0x10000000,1,0,256)==-1);
    assert(ios_reloc_sync_dir64(image,256,UINT32_MAX,32,0x10000000,1,0,256)==-1);
    assert(ios_reloc_sync_dir64(image,256,0x40,7,0x10000000,1,0,256)==-1);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,1,SIZE_MAX,16)==-1);
    fresh(image); u32(image,0x44,11);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,1,0,256)==-1);
    fresh(image); u32(image,0x40,0xfffffff0); u16(image,0x48,0xa020);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,1,0,256)==-1); // no 32-bit RVA wrap
    fresh(image); u16(image,0x48,0xa048);
    assert(ios_reloc_sync_dir64(image,256,0x40,12,0x10000000,1,0,256)==-1); // no metadata overwrite
    fresh(image); u32(image,0x50,0x1000);
    assert(ios_reloc_sync_dir64(image,256,0x40,24,0x10000000,1,0,256)==-1);
    assert(u64(image,0x83)==0x10000083); // later bad block cannot leave partial relocation
    puts("PASS: native/EC child ntdll selection, parent/child thread isolation, concurrency, incomplete counts and bounded relocation replay");
}
'''
cc = os.environ.get("MD_RUNTIME_CC") or os.environ.get("CC") or shutil.which("cc")
if not cc:
    raise SystemExit("C compiler required")
with tempfile.TemporaryDirectory(prefix="madeira-renpy-startup-") as scratch:
    tmp = Path(scratch)
    source = tmp / "check.c"
    binary = tmp / ("check.exe" if os.name == "nt" else "check")
    source.write_text(fixture, encoding="utf-8")
    flags = ["-std=c11", "-O1", "-Wall", "-Wextra", "-Werror", "-pthread"]
    if os.name == "nt": flags += ["-static"]
    elif platform.system() == "Darwin": flags += ["-fsanitize=address,undefined"]
    subprocess.run([cc,*flags,"-I"+str(folder),str(source),"-o",str(binary)],check=True)
    subprocess.run([str(binary)],check=True,timeout=15)
