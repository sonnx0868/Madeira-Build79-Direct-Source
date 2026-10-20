// Research gate: exercise the CFG proof, including cases that MUST be refused.
#include "zero-light-guard.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <string>
using namespace llvm;
static void require(bool ok, const Twine &message) {
  if (!ok) { errs() << message << '\n'; std::exit(1); }
}
struct Fixture {
  Function *function, *sample, *maximum;
  BasicBlock *entry, *work, *test, *positive, *exit;
  PHINode *result;
  BranchInst *predicate;
  SmallVector<CallInst *, 4> taps;
  Fixture(Module &module, StringRef name) {
    auto &c = module.getContext(); IRBuilder<> ir(c);
    auto *f = ir.getFloatTy();
    auto *i32 = ir.getInt32Ty();
    auto *v3 = FixedVectorType::get(f, 3), *v2 = FixedVectorType::get(f, 2);
    auto *p1 = PointerType::get(c, 1), *p2 = PointerType::get(c, 2);
    auto *pair = StructType::get(f, ir.getInt8Ty());
    sample = cast<Function>(module.getOrInsertFunction("air.sample_compare_depth_2d.f32",
      FunctionType::get(pair, {p1,p2,i32,v2,f,ir.getInt1Ty(),FixedVectorType::get(i32,2),
                             ir.getInt1Ty(),f,f,i32}, false)).getCallee());
    sample->addFnAttr(Attribute::ReadOnly); sample->addFnAttr(Attribute::ArgMemOnly);
    sample->addFnAttr(Attribute::NoUnwind); sample->addFnAttr(Attribute::WillReturn);
    maximum = cast<Function>(module.getOrInsertFunction("air.fast_fmax.f32",
      FunctionType::get(f,{f,f},false)).getCallee());
    maximum->addFnAttr(Attribute::ReadNone); maximum->addFnAttr(Attribute::NoUnwind);
    maximum->addFnAttr(Attribute::WillReturn);
    function = Function::Create(FunctionType::get(f,{i32,f,f,f,ir.getInt1Ty()},false),
                                GlobalValue::ExternalLinkage,name,module);
    auto *start = BasicBlock::Create(c,"start",function);
    entry = BasicBlock::Create(c,"attenuation",function);
    work = BasicBlock::Create(c,"shadow",function);
    test = BasicBlock::Create(c,"test",function);
    positive = BasicBlock::Create(c,"positive",function);
    exit = BasicBlock::Create(c,"exit",function);
    ir.SetInsertPoint(start); ir.CreateBr(entry);
    ir.SetInsertPoint(entry);
    auto *atten = ir.CreatePHI(i32,1,"atten"); atten->addIncoming(function->getArg(0),start);
    ir.CreateCondBr(function->getArg(4),work,test);
    ir.SetInsertPoint(work);
    Value *shadow = ConstantFP::get(f,0);
    for (unsigned n=0;n<4;n++) {
      auto *call = ir.CreateCall(sample,{ConstantPointerNull::get(p1),ConstantPointerNull::get(p2),ir.getInt32(0),
        Constant::getNullValue(v2),ConstantFP::get(f,.5),ir.getTrue(),
        Constant::getNullValue(FixedVectorType::get(i32,2)),ir.getTrue(),
        ConstantFP::get(f,0),ConstantFP::get(f,0),ir.getInt32(0)});
      taps.push_back(call); shadow = ir.CreateFAdd(shadow,ir.CreateExtractValue(call,0));
    }
    ir.CreateBr(test);
    ir.SetInsertPoint(test);
    auto *weight = ir.CreatePHI(f,2); weight->addIncoming(shadow,work);
    weight->addIncoming(ConstantFP::get(f,1),entry);
    auto *bits = ir.CreateInsertElement(PoisonValue::get(FixedVectorType::get(i32,3)),atten,ir.getInt32(0));
    auto *broadcast = ir.CreateShuffleVector(ir.CreateBitCast(bits,v3),PoisonValue::get(v3),ArrayRef<int>{0,0,0});
    Value *colour = PoisonValue::get(v3);
    for (unsigned n=0;n<3;n++) colour=ir.CreateInsertElement(colour,function->getArg(n+1),ir.getInt32(n));
    auto *value = ir.CreateFMul(ir.CreateFMul(broadcast,colour),ir.CreateVectorSplat(3,weight));
    auto *a = ir.CreateExtractElement(value,ir.getInt32(0));
    auto *b = ir.CreateExtractElement(value,ir.getInt32(1));
    auto *d = ir.CreateExtractElement(value,ir.getInt32(2));
    auto *max = ir.CreateCall(maximum,{d,ir.CreateCall(maximum,{a,b})});
    predicate=ir.CreateCondBr(ir.CreateFCmpOGT(max,ConstantFP::get(f,0)),positive,exit);
    ir.SetInsertPoint(positive); auto *sum=ir.CreateFAdd(ir.CreateFAdd(a,b),d);ir.CreateBr(exit);
    ir.SetInsertPoint(exit); result=ir.CreatePHI(f,2);
    result->addIncoming(sum,positive);result->addIncoming(ConstantFP::get(f,0),test);ir.CreateRet(result);
  }
};

int main(int argc,char **argv) {
  if(argc!=2)return 2;
  LLVMContext c;Module module("zero-light-tests",c);
  auto check=[&](Fixture &fixture,unsigned expected) {
    require(!verifyModule(module,&errs()),"invalid input fixture");
    require(dxmt::shaderopt::zero_light::run(*fixture.function)==expected,fixture.function->getName()+": transform count");
    require(!verifyModule(module,&errs()),"invalid transformed fixture");
  };
  Fixture baseline(module,"baseline"),guarded(module,"guarded");
  check(guarded,1);check(guarded,0); // idempotent
  for(unsigned kind=0;kind<10;kind++) {
    Fixture negative(module,"negative_"+std::to_string(kind));
    IRBuilder<> ir(negative.work->getTerminator());
    auto *memory=new GlobalVariable(module,ir.getInt32Ty(),false,GlobalValue::InternalLinkage,ir.getInt32(0),"memory");
    switch(kind) {
    case 0: ir.CreateStore(ir.getInt32(1),memory);break;
    case 1: ir.CreateLoad(ir.getInt32Ty(),memory)->setVolatile(true);break;
    case 2: negative.taps[0]->addFnAttr(Attribute::Convergent);break;
    case 3: cast<FCmpInst>(negative.predicate->getCondition())->setPredicate(CmpInst::FCMP_OGE);break;
    case 4: negative.result->setIncomingValue(negative.result->getBasicBlockIndex(negative.test),ConstantFP::get(ir.getFloatTy(),1));break;
    case 5: negative.function->addFnAttr(Attribute::StrictFP);break;
    case 6: ir.CreateLoad(ir.getInt32Ty(),memory)->setAtomic(AtomicOrdering::Monotonic);break;
    case 7: {
      auto *call=negative.taps[0];call->setArgOperand(7,ir.getFalse());break;
    }
    case 8: {
      // A value from the bypassed region escapes without a merge PHI.
      IRBuilder<> before(negative.test->getTerminator());
      auto *value=before.CreateFAdd(negative.function->getArg(1),ConstantFP::get(ir.getFloatTy(),1));
      IRBuilder<> atExit(negative.exit->getTerminator());atExit.CreateFAdd(value,negative.result);break;
    }
    case 9: {
      auto unknown=module.getOrInsertFunction("unknown_readnone",FunctionType::get(ir.getVoidTy(),{},false));
      auto *call=ir.CreateCall(unknown);call->setDoesNotAccessMemory();call->setDoesNotThrow();
      call->addFnAttr(Attribute::WillReturn);break;
    }
    }
    check(negative,0);
    negative.function->eraseFromParent();
  }
  // Replace GPU sampling only AFTER proving the transform. Native execution
  // then counts retained/discarded calls instead of accessing textures.
  auto *sample=baseline.sample;
  sample->removeFnAttr(Attribute::ReadOnly);sample->removeFnAttr(Attribute::ArgMemOnly);
  IRBuilder<> ir(BasicBlock::Create(c,"entry",sample));
  auto *counter=new GlobalVariable(module,ir.getInt32Ty(),false,GlobalValue::ExternalLinkage,ir.getInt32(0),"sample_calls");
  ir.CreateStore(ir.CreateAdd(ir.CreateLoad(ir.getInt32Ty(),counter),ir.getInt32(1)),counter);
  auto *pair=cast<StructType>(sample->getReturnType());
  ir.CreateRet(ConstantStruct::get(pair,ConstantFP::get(ir.getFloatTy(),.5),ir.getInt8(0)));
  auto *maximum=baseline.maximum;
  ir.SetInsertPoint(BasicBlock::Create(c,"entry",maximum));
  auto *intrinsic=Intrinsic::getDeclaration(&module,Intrinsic::maxnum,{ir.getFloatTy()});
  ir.CreateRet(ir.CreateCall(intrinsic,{maximum->getArg(0),maximum->getArg(1)}));
  require(!verifyModule(module,&errs()),"invalid emitted native fixture");
  std::error_code error;raw_fd_ostream out(argv[1],error);require(!error,"output file");module.print(out,nullptr);
  outs()<<"PASS: zero-light guard, idempotence, 10 rejected unsafe variants, LLVM verification\n";
}
