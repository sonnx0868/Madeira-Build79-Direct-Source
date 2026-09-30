"""Read-only comparison of the CPU fix against the rebuilt and shipped engine."""
from pathlib import Path
import importlib.util,json,subprocess,re,hashlib
H=Path(__file__).resolve().parent
B=H.parent/'fex-windows-baseline'
spec=importlib.util.spec_from_file_location('baseline_inspector',B/'inspect.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
m.ROOT=H.parent
paths={'shipped':B/'shipped-xtajit64.dll','baseline':B/'build/Bin/libarm64ecfex.dll','patched':H/'build/Bin/libarm64ecfex.dll'}
items={};jumps={}
for name,path in paths.items():
 data,base,at,summary=m.pe(path)
 summary.update(m.text_info(path,'fex-cpuid-fix/'+name+'-metadata.txt'))
 meta=(H/(name+'-metadata.txt')).read_text()
 # Validate the final image's CHPE executable ranges and redirects as well.
 code_block=meta.split('  CodeMap [',1)[1].split(']',1)[0]
 ranges=[(int(a,16),int(b,16),kind) for a,b,kind in re.findall(r'(0x[0-9A-F]+) - (0x[0-9A-F]+)\s+(ARM64EC|X64)',code_block)]
 rd=meta.split('  RedirectionMetadata [',1)[1].split(']',1)[0]
 redirects=[(int(a,16),int(b,16)) for a,b in re.findall(r'(0x[0-9A-F]+) -> (0x[0-9A-F]+)',rd)]
 for source,target in redirects:
  assert any(a<=source<b and k=='X64' for a,b,k in ranges)
  assert any(a<=target<b and k=='ARM64EC' for a,b,k in ranges)
 summary.update(redirects=len(redirects),valid_redirect_ranges=True,code_ranges=ranges)
 nm=subprocess.check_output(['llvm-nm','--defined-only',str(path)],text=True)
 hit=re.search(r'^([0-9a-f]+) T _ZN7FEXCore17UncheckedLongJump7SetJumpERNS0_7JumpBufE$',nm,re.M);assert hit
 va=int(hit.group(1),16);jumps[name]=at(va-base,56)
 summary['setjump_sha256']=hashlib.sha256(jumps[name]).hexdigest()
 (H/(name+'-setjump.txt')).write_text(subprocess.check_output(['llvm-objdump','-d',f'--start-address={va}',f'--stop-address={va+56}',str(path)],text=True))
 # Data export and TLS offsets are also recorded for future diagnostics.
 ex={n:int(v,16) for n,v in re.findall(r'Export \{\s+Ordinal: \d+\s+Name: ([^\n]+)\s+RVA: (0x[0-9A-Fa-f]+)',meta)}
 summary['native_hook_exports']={n:ex[n] for n in ex if n.startswith('BTCpu64Ios') or n.startswith('ios_va_log')}
 items[name]=summary
comparisons={}
for a,b in [('shipped','patched'),('baseline','patched')]:
 x,y=items[a],items[b]
 tests={'export_names_ordinals_equal':x['export_ordinals']==y['export_ordinals'],'imports_equal':x['imports']==y['imports'],'setjump_56bytes_equal':jumps[a]==jumps[b], 'tls_contract_equal':(x['tls']['raw_data_size'],x['tls']['zero_fill'],x['tls']['characteristics'],len(x['tls']['callbacks_rva']))==(y['tls']['raw_data_size'],y['tls']['zero_fill'],y['tls']['characteristics'],len(y['tls']['callbacks_rva'])), 'alignment_equal':(x['section_alignment'],x['file_alignment'])==(y['section_alignment'],y['file_alignment']),'machine_and_chpe_equal':(x['raw_coff_machine'],x['llvm_machine_classification'],x['chpe_version'])==(y['raw_coff_machine'],y['llvm_machine_classification'],y['chpe_version'])}
 assert all(tests.values()),tests
 comparisons[a+'->'+b]=tests
report={'images':items,'comparisons':comparisons,'device_tested':False,'reviewer':'root; static artifact comparison, not completed subagent review','limitations':['Static compatibility does not prove execution on the iPad.','A fresh compiler build differs from the shipped artifact beyond the two overlaid CPU source files.','Native hooks are resolved by export names; source ABI is the pinned Madeira integration.']}
(H/'binary-review.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'comparisons':comparisons,'patched_sha256':items['patched']['sha256'],'patched_native_hook_exports':items['patched']['native_hook_exports']},indent=2))
