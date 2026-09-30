"""Compare iOS assembly entry glue in the final DLL against the shipped engine."""
from pathlib import Path
import subprocess,re,struct,json,importlib.util
H=Path(__file__).resolve().parent;B=H.parent/'fex-windows-baseline'
spec=importlib.util.spec_from_file_location('peinspect',B/'inspect.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);m.ROOT=H.parent
paths={'shipped':B/'shipped-xtajit64.dll','bad_asm_flag_negative_control':H.parent/'fex-cpuid-fix/build/Bin/libarm64ecfex.dll','candidate':H/'build/Bin/libarm64ecfex.dll'}
def norm(word):
 # Different final placement affects only PC-relative branches and ADR/ADRP.
 if word&0x7c000000==0x14000000:return word&0xfc000000
 if word&0x9f000000 in (0x10000000,0x90000000):return word&0x9f00001f
 if word&0xff000010==0x54000000 or word&0x7e000000==0x34000000:return word&~(0x7ffff<<5)
 if word&0x7e000000==0x36000000:return word&~(0x3fff<<5)
 return word
items={};sequences={};references={}
for name,path in paths.items():
 data,base,at,meta=m.pe(path);nm=subprocess.check_output(['llvm-nm','--defined-only',str(path)],text=True)
 symbols={n:int(v,16) for v,n in re.findall(r'^([0-9a-f]+) [Tt] (\w+)$',nm,re.M)}
 start=symbols['DispatchJump'];end=symbols['JumpSetStack']+8
 raw=at(start-base,end-start);words=struct.unpack('<'+'I'*(len(raw)//4),raw);sequences[name]=[norm(w) for w in words]
 allsymbols={}
 for va,symbol in re.findall(r'^([0-9a-f]+) \S (.+)$',nm,re.M):allsymbols.setdefault(int(va,16),set()).add(symbol)
 references[name]={}
 for i,w in enumerate(words[1:],1):
  prev=words[i-1]
  if prev&0x9f000000!=0x90000000 or ((w>>5)&31)!=(prev&31):continue
  if w&0xffc00000==0x91000000:offset=(w>>10)&0xfff
  elif w&0xffc00000 in (0xb9400000,0xf9400000):offset=((w>>10)&0xfff)<<(3 if w&0x40000000 else 2)
  else:continue
  imm=(((prev>>5)&0x7ffff)<<2)|((prev>>29)&3)
  if imm&0x100000:imm-=0x200000
  addr=((start+4*(i-1))&~0xfff)+(imm<<12)+offset
  assert addr in allsymbols,(name,i*4,hex(addr))
  references[name][i*4]=sorted(allsymbols[addr])
  sequences[name][i]=w&~(0xfff<<10)
 dis=subprocess.check_output(['llvm-objdump','-d',f'--start-address={start}',f'--stop-address={end}',str(path)],text=True);(H/(name+'-entry-assembly.txt')).write_text(dis)
 # Match the exact TSD triplet native Wine recognizes (same register all three).
 triplets=0
 for i,w in enumerate(words[:-2]):
  reg=w&31
  if w==(0xd53bd060|reg) and words[i+1]==(0x927df000|(reg<<5)|reg) and words[i+2]==(0xf9400000|(0x898//8)<<10|(reg<<5)|reg):triplets+=1
 items[name]={'sha256':meta['sha256'],'bytes':len(raw),'instructions':len(words),'ios_teb_triplets':triplets,'relative_entries':{n:symbols[n]-start for n in ['DispatchJump','RetToEntryThunk','ExitToX64','BeginSimulation','ExitFunctionEC','JumpSetStack']}}
report={'images':items,'same_iOS_entry_layout':items['shipped']['relative_entries']==items['candidate']['relative_entries'],'same_normalized_assembly':sequences['shipped']==sequences['candidate'],'same_data_reference_symbols':references['shipped']==references['candidate'],'data_reference_symbols':references,'negative_control_differs':sequences['shipped']!=sequences['bad_asm_flag_negative_control'],'all_iOS_teb_triplets_present':items['candidate']['ios_teb_triplets']==items['shipped']['ios_teb_triplets'] and items['candidate']['ios_teb_triplets']>0}
# Save mismatches for review without treating normalizing as executable proof.
report['normalization']='PC-relative branch/ADR(P) immediates removed; following ADD/LDR offsets removed only after decoding and matching the referenced symbol in both final images. Remaining bits compare exactly. End excludes the metadata for the next function.'
if len(sequences['shipped'])==len(sequences['candidate']):report['remaining_word_differences']=[{'offset':i*4,'shipped':hex(a),'candidate':hex(b)} for i,(a,b) in enumerate(zip(sequences['shipped'],sequences['candidate'])) if a!=b]
(H/'assembly-review.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
assert report['same_iOS_entry_layout'] and report['same_normalized_assembly'] and report['negative_control_differs'] and report['all_iOS_teb_triplets_present'] and report['same_data_reference_symbols']
