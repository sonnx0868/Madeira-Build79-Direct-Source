"""Assemble/link the bounded single-frame decommit replacement. No device actions."""
from pathlib import Path
import hashlib, importlib.util, json, struct, subprocess
H=Path(__file__).resolve().parent
B=H.parents[1]/'apc-context-fix/Madeira'
BASE_SHA='f3269d77d5e9872c0f3506570de11b9a4b707b5753d356176e1ba2691417c1f4'
START=0x10013bfb0
END=0x10013c514
EPILOGUE=0x10013c4f8
CALLER=0x100134a70
SYMBOLS={
    'alias_lookup':0x10012ae68,
    'anon_mmap_fixed':0x10012c3a0,
    'bzero_stub':0x1009e1644,
    'dprintf_stub':0x1009e17f4,
    'alias_refusal_format':0x100adf907,
    'mach_port_deallocate_stub':0x1009e1cd4,
    'mach_vm_protect_stub':0x1009e1d40,
    'mach_vm_region_stub':0x1009e1d64,
    'mach_task_self_got':0x100bc0e98,
    'host_mask':0x100d5d270,
    'ledger_count':0x100d5d2b8,
    'ledger':0x100d5d2c0,
    'alias_count':0x100d29820,
    'aliases':0x100d29828,
    'pages_vprot':0x100d69320,
    'pool_rw':0x101073fa8,
    'pool_size':0x101073fb8,
}
sha=lambda b:hashlib.sha256(b).hexdigest()
def run(*cmd):return subprocess.check_output(cmd,text=True)
spec=importlib.util.spec_from_file_location('macho_offsets',H.parents[1]/'guest-exit-fix/build.py')
offsets=importlib.util.module_from_spec(spec)
spec.loader.exec_module(offsets)
baseline=B.read_bytes()
assert sha(baseline)==BASE_SHA
link='ENTRY(decommit_pages)\nSECTIONS { . = 0x%x; .text : { *(.text) } /DISCARD/ : { *(.comment) *(.note*) } }\n'%START
link+=''.join('%s = 0x%x;\n'%(k,v) for k,v in SYMBOLS.items())
(H/'link.ld').write_text(link)
run('clang','--target=aarch64-linux-gnu','-c',str(H/'decommit_pages.S'),'-o',str(H/'decommit_pages.o'))
run('ld.lld','-m','aarch64elf','--no-relax','-T',str(H/'link.ld'),str(H/'decommit_pages.o'),'-o',str(H/'decommit_pages.elf'))
run('llvm-objcopy','-O','binary','--only-section=.text',str(H/'decommit_pages.elf'),str(H/'decommit_pages.bin'))
body=(H/'decommit_pages.bin').read_bytes()
assert len(body)==END-START
core_line=next(x for x in run('llvm-nm','-n',str(H/'decommit_pages.elf')).splitlines() if x.endswith(' decommit_core_end'))
core_end=int(core_line.split()[0],16)
assert core_end<=EPILOGUE
at=offsets.file_offset(baseline,START,len(body))
caller_at=offsets.file_offset(baseline,CALLER,4)
original=baseline[at:at+len(body)]
assert body[:32]==original[:32]
assert body[EPILOGUE-START:]==original[EPILOGUE-START:]
assert baseline[caller_at:caller_at+4]==bytes.fromhex('0c000014')
# Route decommit status through existing MOV x24,x0; CBNZ w0,failure_unlock.
caller_new=bytes.fromhex('0a000014')
assert CALLER+(struct.unpack('<I',caller_new)[0]&0x3ffffff)*4==0x100134a98
candidate=bytearray(baseline)
candidate[at:at+len(body)]=body
candidate[caller_at:caller_at+4]=caller_new
assert len(candidate)==len(baseline)
assert all(a==b or at<=i<at+len(body) or caller_at<=i<caller_at+4 for i,(a,b) in enumerate(zip(baseline,candidate)))
(H/'Madeira').write_bytes(candidate)
(H/'decommit_pages.original.bin').write_bytes(original)
(H/'candidate-native.txt').write_text(run('llvm-objdump','--macho','--disassemble','--dis-symname','_decommit_pages',str(H/'Madeira')))
(H/'original-native.txt').write_text(run('llvm-objdump','--macho','--disassemble','--dis-symname','_decommit_pages',str(B)))
(H/'elf-disassembly.txt').write_text(run('llvm-objdump','-d',str(H/'decommit_pages.elf')))
(H/'caller-native.txt').write_text(run('llvm-objdump','--macho','--disassemble','--dis-symname','_NtFreeVirtualMemory',str(H/'Madeira')))
relocs=run('llvm-readelf','--relocations',str(H/'decommit_pages.elf'))
assert 'There are no relocations in this file.' in relocs
(H/'linked-relocations.txt').write_text(relocs)
unwind_before=run('llvm-objdump','--macho','--unwind-info',str(B)).splitlines()[1:]
unwind_after=run('llvm-objdump','--macho','--unwind-info',str(H/'Madeira')).splitlines()[1:]
assert unwind_before==unwind_after
report={
 'baseline':str(B),'baseline_sha256':sha(baseline),'candidate_sha256':sha(candidate),
 'assembly_sha256':sha((H/'decommit_pages.S').read_bytes()),'body_sha256':sha(body),
 'range':{'start':hex(START),'end_exclusive':hex(END),'length':len(body),'file_offset':hex(at)},
 'core_end':hex(core_end),'core_bytes_including_prefix':core_end-START,'padding_bytes':EPILOGUE-core_end,
 'prefix32_unchanged':True,'epilogue28_unchanged':True,'unwind_info_unchanged':True,
 'caller':{'address':hex(CALLER),'file_offset':hex(caller_at),'old':'0c000014','new':caller_new.hex(),'target':'0x100134a98'},
 'only_function_and_one_caller_instruction_changed':True,
 'helper_addresses':{k:hex(v) for k,v in SYMBOLS.items()},
 'status_success':'0x00000000','status_access_denied':'0xc0000022','status_no_memory':'0xc0000017',
 'checks':'Two partial edges; full-page alias and RW/RX-pool overlap rejection; exact Mach current/max/coverage/count validation; opening without COPY/EXEC; both preparations before checked interior mmap; full edge zero readback while RW; reverse restoration and exact requery; commit-bit bookkeeping only success; historical live-ledger alias refusal retained.',
 'stack':'Original0x190 frame. Local records/query/status0x00..0x9c, callee saves0x140..0x18f. No new local callframes. No x18 or x27/x28 or SIMD modification.',
 'darwin_writewatch':'No-op in pinned Darwin source/binary; no new helper required.',
 'native_harness_pending':True,'packaged':False,'installed':False,'beamng_working':False,
}
(H/'build-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
