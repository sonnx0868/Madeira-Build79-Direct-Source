// Exercise the real AIR emitter, including mixed call order and loop guards.
#include "nt/air_builder.hpp"
#include "explicit_lod_guard.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
using namespace llvm;
static void require(bool condition,const char *message) {
  if(!condition){errs()<<message<<'\n';std::exit(1);}
}
static void mixedCalls(bool explicitFirst,bool compare) {
  LLVMContext context;Module module("sampling",context);IRBuilder<> ir(context);air::AIRBuilder air(ir,errs());
  air::Texture texture{compare?air::Texture::depth2d:air::Texture::texture2d,air::Texture::sample_float,air::Texture::access_sample};
  auto *function=Function::Create(FunctionType::get(ir.getVoidTy(),{air.getTextureHandleType(texture),air.getSamplerHandleType()},false),GlobalValue::ExternalLinkage,"mixed",module);
  ir.SetInsertPoint(BasicBlock::Create(context,"entry",function));
  int32_t offset[3]={};auto *coord=Constant::getNullValue(FixedVectorType::get(ir.getFloatTy(),2));
  for(bool explicitLOD:{explicitFirst,!explicitFirst}) {
    if(compare) {
      if(explicitLOD)air.CreateSampleCmp(texture,function->getArg(0),function->getArg(1),coord,nullptr,air.getFloat(0.5),offset,air::sample_level{air.getFloat(0)});
      else air.CreateSampleCmp(texture,function->getArg(0),function->getArg(1),coord,nullptr,air.getFloat(0.5),offset,air::sample_bias{air.getFloat(0)},air::sample_min_lod_clamp{air.getFloat(0)});
    } else {
      if(explicitLOD)air.CreateSample(texture,function->getArg(0),function->getArg(1),coord,nullptr,offset,air::sample_level{air.getFloat(0)});
      else air.CreateSample(texture,function->getArg(0),function->getArg(1),coord,nullptr,offset);
    }
  }
  auto *barrier=air.CreateBarrier(air::MemFlags::Threadgroup);ir.CreateRetVoid();
  require(dxmt::air::relaxExplicitLODSampling(module)==0,"mixed LOD declaration was relaxed");
  unsigned count=0;
  for(auto &block:*function)for(auto &instruction:block)if(auto *call=dyn_cast<CallInst>(&instruction)) {
    if(call==barrier){require(call->isConvergent(),"barrier lost convergence");continue;}
    require(call->getCalledFunction()->isConvergent(),"mixed declaration lost convergence");
    bool explicitLOD=count++==0?explicitFirst:!explicitFirst;
    require(call->isConvergent(),"mixed call lost sampling convergence");
  }
  require(count==2,"missing sampling calls");require(!verifyModule(module,&errs()),"invalid emitted module");
}
static void loopGuard(bool explicitLOD) {
  LLVMContext context;Module module("guard",context);IRBuilder<> ir(context);air::AIRBuilder air(ir,errs());
  air::Texture texture{air::Texture::depth2d,air::Texture::sample_float,air::Texture::access_sample};
  auto *function=Function::Create(FunctionType::get(ir.getVoidTy(),{air.getTextureHandleType(texture),air.getSamplerHandleType(),ir.getInt32Ty(),PointerType::get(ir.getFloatTy(),1)},false),GlobalValue::ExternalLinkage,"guard",module);
  auto *entry=BasicBlock::Create(context,"entry",function),*header=BasicBlock::Create(context,"header",function),*body=BasicBlock::Create(context,"body",function),*exit=BasicBlock::Create(context,"exit",function);
  ir.SetInsertPoint(entry);int32_t offset[3]={};auto *coord=Constant::getNullValue(FixedVectorType::get(ir.getFloatTy(),2));
  Value *sample;
  if(explicitLOD)sample=air.CreateSampleCmp(texture,function->getArg(0),function->getArg(1),coord,nullptr,air.getFloat(0.5),offset,air::sample_level{air.getFloat(0)}).first;
  else sample=air.CreateSampleCmp(texture,function->getArg(0),function->getArg(1),coord,nullptr,air.getFloat(0.5),offset,air::sample_bias{air.getFloat(0)},air::sample_min_lod_clamp{air.getFloat(0)}).first;
  auto *call=cast<CallInst>(cast<ExtractValueInst>(sample)->getAggregateOperand());
  ir.CreateBr(header);ir.SetInsertPoint(header);auto *index=ir.CreatePHI(ir.getInt32Ty(),2);index->addIncoming(ir.getInt32(0),entry);
  ir.CreateCondBr(ir.CreateICmpULT(index,function->getArg(2)),body,exit);
  ir.SetInsertPoint(body);ir.CreateStore(sample,ir.CreateGEP(ir.getFloatTy(),function->getArg(3),index));
  auto *next=ir.CreateAdd(index,ir.getInt32(1));index->addIncoming(next,body);ir.CreateBr(header);
  ir.SetInsertPoint(exit);ir.CreateRetVoid();
  require(dxmt::air::relaxExplicitLODSampling(module)==unsigned(explicitLOD),"wrong explicit sampling eligibility");
  LoopAnalysisManager lam;FunctionAnalysisManager fam;CGSCCAnalysisManager cgam;ModuleAnalysisManager mam;PassBuilder pb;
  pb.registerModuleAnalyses(mam);pb.registerCGSCCAnalyses(cgam);pb.registerFunctionAnalyses(fam);pb.registerLoopAnalyses(lam);pb.crossRegisterProxies(lam,fam,cgam,mam);
  ModulePassManager pm;
  // Production already runs InstCombine before rotation/sinking. In
  // particular, remove the unused sample-status extract emitted by AIRBuilder.
  if(auto error=pb.parsePassPipeline(pm,"function(instcombine,loop-simplify,lcssa,loop(loop-rotate),sink)")){consumeError(std::move(error));std::exit(2);}
  pm.run(module,mam);
  if((call->getParent()!=entry)!=explicitLOD) {
    errs()<<"explicit LOD="<<explicitLOD<<'\n';module.print(errs(),nullptr);
    require(false,"texture sampling did not respect zero-trip / derivative guard");
  }
  require(!verifyModule(module,&errs()),"invalid optimized module");
}
int main() {
  for(bool first:{false,true})for(bool compare:{false,true})mixedCalls(first,compare);
  loopGuard(false);loopGuard(true);
  outs()<<"PASS: explicit sampling can skip empty loops; implicit derivatives and barriers retain convergence in both call orders\n";
}
