#include <cstdint>
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
FEXCore::CPUID::FunctionResults Function_1Ah(uint32_t Leaf) const ;
FEXCore::CPUID::FunctionResults Function_8000_0002h(uint32_t Leaf) const ;
FEXCore::CPUID::FunctionResults Function_8000_0002h(uint32_t Leaf, uint32_t CPU) const ;
FEXCore::CPUID::FunctionResults Function_8000_0003h(uint32_t Leaf) const ;
FEXCore::CPUID::FunctionResults Function_8000_0003h(uint32_t Leaf, uint32_t CPU) const ;
FEXCore::CPUID::FunctionResults Function_8000_0004h(uint32_t Leaf) const ;
FEXCore::CPUID::FunctionResults Function_8000_0004h(uint32_t Leaf, uint32_t CPU) const ;FEXCore::CPUID::FunctionResults RunFunctionName(uint32_t Function, uint32_t Leaf, uint32_t CPU) const {
    if (PerCPUData.empty()) return {};
    if (Function == 0x8000'0002U) {
      return Function_8000_0002h(Leaf, CPU % PerCPUData.size());
    } else if (Function == 0x8000'0003U) {
      return Function_8000_0003h(Leaf, CPU % PerCPUData.size());
    } else {
      return Function_8000_0004h(Leaf, CPU % PerCPUData.size());
    }
  }};
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_1Ah(uint32_t Leaf) const {
  FEXCore::CPUID::FunctionResults Res {};
  if (Hybrid && !PerCPUData.empty()) {
    uint32_t CPU = GetCPUID();
    auto& Data = PerCPUData[CPU % PerCPUData.size()];
    // 0x40 is a big CPU
    // 0x20 is a little CPU
    Res.eax |= (Data.IsBig ? 0x40 : 0x20) << 24;
  }
  return Res;
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0002h(uint32_t Leaf) const {
  return Function_8000_0002h(Leaf, GetCPUID());
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0002h(uint32_t Leaf, uint32_t CPU) const {
  FEXCore::CPUID::FunctionResults Res {};
  // Personal Madeira integration: host processor IDs may outnumber the
  // metadata records (the iOS host currently provides one generic MIDR).
  // Match RunFunctionName's existing indexing convention without changing
  // processor counts or scheduler affinity.
  if (PerCPUData.empty()) return Res;
  auto& Data = PerCPUData[CPU % PerCPUData.size()];
  if (!Data.ProductName) return Res;
  memcpy(&Res, Data.ProductName, std::min(strlen(Data.ProductName), sizeof(FEXCore::CPUID::FunctionResults)));
  return Res;
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0003h(uint32_t Leaf) const {
  return Function_8000_0003h(Leaf, GetCPUID());
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0003h(uint32_t Leaf, uint32_t CPU) const {
  FEXCore::CPUID::FunctionResults Res {};
  if (PerCPUData.empty()) return Res;
  auto& Data = PerCPUData[CPU % PerCPUData.size()];
  if (!Data.ProductName) return Res;
  const size_t Length = strlen(Data.ProductName);
  if (Length <= 16) return Res;
  memcpy(&Res, Data.ProductName + 16, std::min(Length - 16, sizeof(FEXCore::CPUID::FunctionResults)));
  return Res;
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0004h(uint32_t Leaf) const {
  return Function_8000_0004h(Leaf, GetCPUID());
}
FEXCore::CPUID::FunctionResults CPUIDEmu::Function_8000_0004h(uint32_t Leaf, uint32_t CPU) const {
  FEXCore::CPUID::FunctionResults Res {};
  if (PerCPUData.empty()) return Res;
  auto& Data = PerCPUData[CPU % PerCPUData.size()];
  if (!Data.ProductName) return Res;
  const size_t Length = strlen(Data.ProductName);
  if (Length <= 32) return Res;
  memcpy(&Res, Data.ProductName + 32, std::min(Length - 32, sizeof(FEXCore::CPUID::FunctionResults)));
  return Res;
}
}

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
