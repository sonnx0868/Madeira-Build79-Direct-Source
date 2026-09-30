from pathlib import Path
import subprocess,os,json
H=Path(__file__).resolve().parent;B=H/'tests';B.mkdir(exist_ok=True);BASE=H.parent/'fex-windows-baseline/FEX/FEXCore/Source/Interface/Core'
def function(source,needle):
 a=source.index(needle);b=source.index('{',a);depth=1;e=b+1
 while depth:
  depth+=(source[e]=='{')-(source[e]=='}');e+=1
 return source[a:e]
def make(root,baseline=False):
 cpp=(root/'CPUID.cpp').read_text();header=(root/'CPUID.h').read_text();methods=[]
 for name in ['Function_1Ah','Function_8000_0002h','Function_8000_0003h','Function_8000_0004h']:
  methods.append(function(cpp,'FEXCore::CPUID::FunctionResults CPUIDEmu::'+name+'(uint32_t Leaf) const'))
  if name!='Function_1Ah':methods.append(function(cpp,'FEXCore::CPUID::FunctionResults CPUIDEmu::'+name+'(uint32_t Leaf, uint32_t CPU) const'))
 inline=function(header,'FEXCore::CPUID::FunctionResults RunFunctionName(')
 declarations='\n'.join(x.split('{',1)[0].replace('CPUIDEmu::','')+';' for x in methods)
 prefix='''#include <cstdint>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cstdio>
#include <sys/types.h>
namespace FEXCore {namespace CPUID {struct FunctionResults {uint32_t eax{},ebx{},ecx{},edx{};};}
class CPUIDEmu {public:
struct CPUData {const char *ProductName;uint32_t MIDR{};bool IsBig{};};
std::vector<CPUData> PerCPUData;bool Hybrid{};uint32_t cpu{};
uint32_t GetCPUID()const{return cpu;}
'''+declarations+inline+'};\n'+'\n'.join(methods)+'\n}\n'
 test=r'''
static unsigned checks;
#define CHECK(x) do{++checks;assert(x);}while(0)
static void expected(FEXCore::CPUID::FunctionResults r,const char*name,unsigned chunk){
 unsigned char e[16]={0};if(name){size_t n=strlen(name),off=chunk*16;if(n>off)memcpy(e,name+off,std::min(n-off,(size_t)16));}
 CHECK(!memcmp(&r,e,16));}
int main(){using FEXCore::CPUIDEmu;CPUIDEmu e;
#ifdef PROVE_BASELINE_FAULT
 e.PerCPUData={{"one",0,true}};e.cpu=8;auto r=e.Function_8000_0002h(0);printf("unexpected success %u\n",r.eax);return 0;
#else
 for(unsigned count=1;count<=3;++count){
  e.PerCPUData={{"Generic ARM processor metadata",0,true},{"Second CPU name longer than thirty-two characters",0,false},{"third",0,true}};e.PerCPUData.resize(count);e.Hybrid=true;
  const uint32_t ids[]={0,1,2,3,4,5,6,7,8,31,255,UINT32_MAX};
  for(uint32_t cpu:ids){
#ifdef BASELINE_IN_RANGE
   if(cpu>=count)continue;
#endif
   e.cpu=cpu;const auto&d=e.PerCPUData[cpu%count];
   for(unsigned chunk=0;chunk<3;++chunk){auto r=chunk==0?e.Function_8000_0002h(0):chunk==1?e.Function_8000_0003h(0):e.Function_8000_0004h(0);expected(r,d.ProductName,chunk);expected(e.RunFunctionName(0x80000002+chunk,0,cpu),d.ProductName,chunk);}
   CHECK(e.Function_1Ah(0).eax==((d.IsBig?0x40u:0x20u)<<24));
  }
 }
#ifndef BASELINE_IN_RANGE
 char short_name[66];
 for(unsigned length: {0u,1u,15u,16u,17u,31u,32u,33u,47u,48u,49u,65u}){
  memset(short_name,'a',length);short_name[length]=0;e.PerCPUData={{short_name,0,true}};e.cpu=UINT32_MAX;
  for(unsigned chunk=0;chunk<3;++chunk)expected(e.RunFunctionName(0x80000002+chunk,0,UINT32_MAX),short_name,chunk);
 }
 e.PerCPUData.clear();e.cpu=UINT32_MAX;
 for(unsigned chunk=0;chunk<3;++chunk){expected(e.RunFunctionName(0x80000002+chunk,0,UINT32_MAX),nullptr,chunk);auto r=chunk==0?e.Function_8000_0002h(0):chunk==1?e.Function_8000_0003h(0):e.Function_8000_0004h(0);expected(r,nullptr,chunk);}
 CHECK(e.Function_1Ah(0).eax==0);
 e.PerCPUData={{nullptr,0,true}};
 for(unsigned chunk=0;chunk<3;++chunk)expected(e.RunFunctionName(0x80000002+chunk,0,8),nullptr,chunk);
#endif
 printf("PASS %u extracted CPUID metadata checks\n",checks);return 0;
#endif
}
'''
 return prefix+test
original=B/'original.cpp';original.write_text(make(BASE));patched=B/'patched.cpp';patched.write_text(make(H/'source'))
env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'};results=[]
for name,src,define in [('baseline-valid',original,'BASELINE_IN_RANGE'),('patched',patched,''),('baseline-oob',original,'PROVE_BASELINE_FAULT')]:
 exe=B/name;cmd=['clang++','-std=c++20','-O1','-g','-Wall','-Wextra','-Wno-unused-parameter','-Wno-unused-function','-Wno-unused-variable','-Werror','-fsanitize=address,undefined']
 if define:cmd+=['-D'+define]
 subprocess.run(cmd+[str(src),'-o',str(exe)],check=True);r=subprocess.run([str(exe)],env=env,capture_output=True,text=True);(B/(name+'.log')).write_text(r.stdout+r.stderr)
 if name=='baseline-oob':assert r.returncode!=0 and 'heap-buffer-overflow' in r.stderr,(r.returncode,r.stdout,r.stderr)
 else:assert r.returncode==0 and not r.stderr,(r.returncode,r.stdout,r.stderr)
 results.append({'test':name,'exit':r.returncode,'result':'Expected heap-buffer-overflow confirmed' if name=='baseline-oob' else r.stdout.strip()});print(results[-1])
(H/'test-report.json').write_text(json.dumps({'status':'PASS','tests':results,'source':'Actual source-extracted brand/hybrid handlers and inline RunFunctionName','sanitizers':['ASan','UBSan'],'device_tested':False},indent=2)+'\n')
