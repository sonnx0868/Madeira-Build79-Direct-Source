"""Build fixed native request body against the exact GuestExit-v2 executable."""
from pathlib import Path
import hashlib,json,subprocess,sys
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parents[1]/'guest-exit-fix'))
from build import file_offset
START,END=0x100054dec,0x10005506c
BASE=HERE.parents[1]/'fd-cache-exit-fix/Madeira'
BASE_SHA='c3125a1266952a6a1fbc14928755c917654da309fec09f9642287524d49126cd'
SYMS={'get_handle_obj':0x1000698fc,'alloc_object':0x1000753d8,'create_internal_sync':0x100061dbc,
      'get_fd_user':0x100064c18,'fd_cancel_async':0x100065044,'grab_object':0x1000756bc,
      'release_object':0x100074bbc,'alloc_handle':0x1000692b0,'req_cancel_assert':0x100998ea4,
      'current':0x101073d08,'global_error':0x101073d10,'async_cancel_ops':0x100bc5270}
sha=lambda x:hashlib.sha256(x).hexdigest()
def main():
    data=BASE.read_bytes(); assert sha(data)==BASE_SHA
    subprocess.run(['clang','--target=aarch64-linux-gnu','-c',str(HERE/'cancel.S'),'-o',str(HERE/'cancel.o')],check=True)
    subprocess.run(['ld.lld','-m','aarch64elf','-Ttext='+hex(START),'--entry=req_cancel_fixed',
                    *['--defsym='+k+'='+hex(v) for k,v in SYMS.items()],str(HERE/'cancel.o'),'-o',str(HERE/'cancel.elf')],check=True)
    subprocess.run(['llvm-objcopy','-O','binary','--only-section=.text',str(HERE/'cancel.elf'),str(HERE/'cancel.bin')],check=True)
    code=(HERE/'cancel.bin').read_bytes(); assert len(code)==END-START==640
    off=file_offset(data,START,len(code)); orig=data[off:off+len(code)]
    assert code[:32]==orig[:32]                 # original frame creation
    assert code[0x25c:]==orig[0x25c:]           # epilogue and cold assert call
    fixed=data[:off]+code+data[off+len(code):]
    assert len(fixed)==len(data)
    assert fixed[:off]==data[:off] and fixed[off+len(code):]==data[off+len(code):]
    (HERE/'Madeira-original').write_bytes(data); (HERE/'Madeira').write_bytes(fixed)
    dis=subprocess.check_output(['llvm-objdump','-d',str(HERE/'cancel.elf')],text=True)
    (HERE/'cancel.asm').write_text(dis)
    report={'baseline_sha256':BASE_SHA,'native_sha256':sha(fixed),'code_sha256':sha(code),
            'source_sha256':sha((HERE/'cancel.S').read_bytes()),'range':[hex(START),hex(END)],
            'bytes':len(code),'differing_bytes':sum(a!=b for a,b in zip(orig,code)),
            'same_prologue_epilogue_and_unwind_metadata':True,'outside_function_identical':True,
            'symbols':{k:hex(v) for k,v in SYMS.items()},'installed':False,'device_validated':False}
    (HERE/'build-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
