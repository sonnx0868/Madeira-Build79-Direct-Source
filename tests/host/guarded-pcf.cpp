// Exercise the actual experimental pass. The emitted native fixture replaces
// AIR sampling only AFTER transforming, so it can count the calls that survive.
#include "guarded_pcf.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <functional>
#include <string>
using namespace llvm;

static void require(bool value, const Twine &message) {
  if (!value) { errs() << message << '\n'; std::exit(1); }
}
static std::string dump(Module &module) {
  std::string result; raw_string_ostream stream(result); module.print(stream, nullptr); return result;
}
struct Fixture {
  Module &module;
  Function *function, *sampler, *fma;
  GlobalVariable *memory;
  SelectInst *select;
  SmallVector<CallInst *, 16> calls;
  SmallVector<Instruction *, 16> values;
  Fixture(Module &module, StringRef name, unsigned count = 9, bool bits = false,
          bool reverse = false, bool fakeLoop = false) : module(module) {
    auto &context = module.getContext(); IRBuilder<> ir(context);
    auto *f32 = ir.getFloatTy();
    auto *v2f = FixedVectorType::get(f32, 2), *v2i = FixedVectorType::get(ir.getInt32Ty(), 2);
    auto *p1 = PointerType::get(context, 1), *p2 = PointerType::get(context, 2);
    auto *sampleType = FunctionType::get(StructType::get(f32, ir.getInt8Ty()),
      {p1, p2, ir.getInt32Ty(), v2f, f32, ir.getInt1Ty(), v2i, ir.getInt1Ty(), f32, f32, ir.getInt32Ty()}, false);
    sampler = cast<Function>(module.getOrInsertFunction("air.sample_compare_depth_2d.f32", sampleType).getCallee());
    sampler->addFnAttr(Attribute::ReadOnly); sampler->addFnAttr(Attribute::ArgMemOnly);
    sampler->addFnAttr(Attribute::NoUnwind); sampler->addFnAttr(Attribute::WillReturn);
    fma = cast<Function>(module.getOrInsertFunction("air.fma.f32", FunctionType::get(f32, {f32,f32,f32}, false)).getCallee());
    fma->addFnAttr(Attribute::ReadNone); fma->addFnAttr(Attribute::NoUnwind); fma->addFnAttr(Attribute::WillReturn);
    function = Function::Create(FunctionType::get(f32, {ir.getInt32Ty(), f32}, false), GlobalValue::ExternalLinkage, name, module);
    memory = new GlobalVariable(module, f32, false, GlobalValue::InternalLinkage, ConstantFP::get(f32, 0), name + ".memory");
    ir.SetInsertPoint(BasicBlock::Create(context, "entry", function));
    auto *predicate = ir.CreateICmpNE(function->getArg(0), ir.getInt32(0), "inside");
    Value *sum = ConstantFP::get(f32, 0);
    for (unsigned i = 0; i < count; ++i) {
      auto *x = ir.CreateFAdd(function->getArg(1), ConstantFP::get(f32, double(i) / 16), "coord.x");
      Value *uv = ir.CreateInsertElement(PoisonValue::get(v2f), x, ir.getInt32(0));
      uv = ir.CreateInsertElement(uv, ConstantFP::get(f32, double(i) / 32), ir.getInt32(1));
      auto *call = ir.CreateCall(sampler, {ConstantPointerNull::get(p1), ConstantPointerNull::get(p2), ir.getInt32(1),
        uv, ConstantFP::get(f32, .5), ir.getTrue(), Constant::getNullValue(v2i), ir.getTrue(),
        ConstantFP::get(f32, 0), ConstantFP::get(f32, 0), ir.getInt32(0)}, "tap");
      call->setMetadata(LLVMContext::MD_annotation, MDNode::get(context, MDString::get(context, "preserve-tap")));
      calls.push_back(call);
      auto *value = cast<Instruction>(ir.CreateExtractValue(call, 0)); values.push_back(value);
      auto *weighted = ir.CreateCall(fma, {value, ConstantFP::get(f32, 1.0 / count), sum}, "weighted");
      FastMathFlags flags; flags.setAllowContract(); weighted->setFastMathFlags(flags); sum = weighted;
      if (fakeLoop && i + 2 == count) {
        auto *before = ir.GetInsertBlock();
        auto *loop = BasicBlock::Create(context, "fake.loop", function);
        auto *exit = BasicBlock::Create(context, "after.fake.loop", function);
        ir.CreateBr(loop); ir.SetInsertPoint(loop);
        auto *phi = ir.CreatePHI(f32, 2); phi->addIncoming(sum, before); phi->addIncoming(phi, loop);
        ir.CreateCondBr(ir.getTrue(), exit, loop); ir.SetInsertPoint(exit); sum = phi;
      }
    }
    Value *computed = bits ? ir.CreateBitCast(sum, ir.getInt32Ty()) : sum;
    Value *constant = bits ? static_cast<Value *>(ir.getInt32(0x3f800000)) : ConstantFP::get(f32, 1);
    select = cast<SelectInst>(ir.CreateSelect(predicate, reverse ? constant : computed, reverse ? computed : constant, "shadow"));
    if (!bits) { FastMathFlags flags; flags.setAllowContract(); select->setFastMathFlags(flags); }
    ir.CreateRet(bits ? ir.CreateBitCast(select, f32) : select);
  }
};

static unsigned tests = 0;
static void check(StringRef name, unsigned expected,
                  std::function<void(Fixture &)> edit = {}, unsigned samples = 9,
                  bool bits = false, bool reverse = false) {
  LLVMContext context; Module module(name, context); Fixture fixture(module, "candidate", samples, bits, reverse);
  if (edit) edit(fixture);
  require(!verifyModule(module, &errs()), name + ": invalid input");
  auto before = dump(module);
  auto *condition = fixture.select->getCondition();
  bool floating = isa<FPMathOperator>(fixture.select);
  bool contract = floating && fixture.select->getFastMathFlags().allowContract();
  auto *profile = fixture.select->getMetadata(LLVMContext::MD_prof);
  auto *unpredictable = fixture.select->getMetadata(LLVMContext::MD_unpredictable);
  auto *accuracy = fixture.select->getMetadata(LLVMContext::MD_fpmath);
  auto *annotation = fixture.select->getMetadata(LLVMContext::MD_annotation);
  unsigned count = dxmt::shaderopt::guardConstantPCFSampling(*fixture.function);
  require(count == expected, name + ": unexpected guard count");
  require(!verifyModule(module, &errs()), name + ": invalid output");
  if (!expected) require(dump(module) == before, name + ": rejected candidate mutated");
  else {
    auto *branch = cast<BranchInst>(fixture.function->getEntryBlock().getTerminator());
    auto *frozen = dyn_cast<FreezeInst>(branch->getCondition());
    require(frozen && frozen->getOperand(0) == condition, name + ": predicate was not frozen");
    require(branch->getMetadata(LLVMContext::MD_prof) == profile &&
      branch->getMetadata(LLVMContext::MD_unpredictable) == unpredictable, name + ": branch metadata lost");
    BasicBlock *sample = branch->getSuccessor(reverse ? 1 : 0);
    require(sample->getName().startswith("pcf.sample"), name + ": branch arms changed");
    unsigned index = 0;
    for (auto &instruction : *sample) if (auto *call = dyn_cast<CallInst>(&instruction)) {
      if (call->getCalledFunction() == fixture.sampler) {
        require(call == fixture.calls[index++], name + ": sample order changed");
        require(call->getMetadata(LLVMContext::MD_annotation) != nullptr, name + ": sample metadata lost");
      } else require(call->getFastMathFlags().allowContract(), name + ": math flags lost");
    }
    require(index == samples, name + ": missing taps");
    auto *merge = sample->getTerminator()->getSuccessor(0);
    auto *phi = dyn_cast<PHINode>(&merge->front());
    require(phi && (!floating || phi->getFastMathFlags().allowContract() == contract), name + ": select flags lost");
    require(phi->getMetadata(LLVMContext::MD_fpmath) == accuracy &&
      phi->getMetadata(LLVMContext::MD_annotation) == annotation, name + ": select metadata lost");
  }
  ++tests;
}

static void cleanup(Module &module) {
  LoopAnalysisManager lam; FunctionAnalysisManager fam; CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
  PassBuilder pb; pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
  pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam); pb.crossRegisterProxies(lam, fam, cgam, mam);
  ModulePassManager pm;
  if (auto error = pb.parsePassPipeline(pm, "function(simplifycfg)")) {
    consumeError(std::move(error)); require(false, "cannot parse actual PCF prepass");
  }
  pm.run(module, mam);
}

static void regressions() {
  for (bool bits : {false, true}) for (bool reverse : {false, true}) check("valid-arms", 1, {}, 9, bits, reverse);
  check("four-taps", 1, {}, 4); check("three-taps", 0, {}, 3); check("too-many-taps", 0, {}, 33);
  check("poison-predicate", 1, [](Fixture &f) { f.select->setCondition(PoisonValue::get(Type::getInt1Ty(f.module.getContext()))); });
  check("convergent-declaration", 0, [](Fixture &f) { f.sampler->addFnAttr(Attribute::Convergent); });
  check("convergent-call", 0, [](Fixture &f) { f.calls[0]->addFnAttr(Attribute::Convergent); });
  check("implicit-first-tap", 0, [](Fixture &f) { f.calls[0]->setArgOperand(7, ConstantInt::getFalse(f.module.getContext())); });
  check("implicit-last-tap", 0, [](Fixture &f) { f.calls.back()->setArgOperand(7, ConstantInt::getFalse(f.module.getContext())); });
  check("dynamic-lod", 0, [](Fixture &f) { f.calls[0]->setArgOperand(7, f.select->getCondition()); });
  check("write-capable-sample", 0, [](Fixture &f) { f.sampler->removeFnAttr(Attribute::ReadOnly); });
  check("can-unwind", 0, [](Fixture &f) { f.sampler->removeFnAttr(Attribute::NoUnwind); });
  check("may-not-return", 0, [](Fixture &f) { f.sampler->removeFnAttr(Attribute::WillReturn); });
  check("no-duplicate", 0, [](Fixture &f) { f.sampler->addFnAttr(Attribute::NoDuplicate); });
  check("strict-fp", 0, [](Fixture &f) { f.function->addFnAttr(Attribute::StrictFP); });
  check("shared-sample", 0, [](Fixture &f) { IRBuilder<> ir(f.select->getNextNode()); ir.CreateStore(f.values[0], f.memory); });
  check("used-residency", 0, [](Fixture &f) { IRBuilder<> ir(f.calls[0]->getNextNode()); ir.CreateExtractValue(f.calls[0], 1, "residency"); });
  check("shared-sum", 0, [](Fixture &f) { IRBuilder<> ir(f.select); ir.CreateStore(f.select->getTrueValue(), f.memory); });
  check("sample-dependent-predicate", 0, [](Fixture &f) {
    IRBuilder<> ir(f.select); f.select->setCondition(ir.CreateFCmpOGT(f.values[0], ConstantFP::get(ir.getFloatTy(), 0)));
  });
  check("store-between-taps", 0, [](Fixture &f) { IRBuilder<> ir(f.calls[1]); ir.CreateStore(ConstantFP::get(ir.getFloatTy(), .5), f.memory); });
  check("volatile-load-between-taps", 0, [](Fixture &f) { IRBuilder<> ir(f.calls[1]); ir.CreateLoad(ir.getFloatTy(), f.memory)->setVolatile(true); });
  check("atomic-load-between-taps", 0, [](Fixture &f) {
    IRBuilder<> ir(f.calls[1]); auto *load = ir.CreateLoad(ir.getFloatTy(), f.memory); load->setAtomic(AtomicOrdering::Monotonic);
  });
  for (const char *name : {"air.wg.barrier", "air.dfdx.f32", "unknown.read"})
    check(name, 0, [name](Fixture &f) {
      IRBuilder<> ir(f.calls[1]); auto unknown = f.module.getOrInsertFunction(name, FunctionType::get(ir.getVoidTy(), {}, false));
      auto *call = ir.CreateCall(unknown); call->addFnAttr(Attribute::Convergent);
    });
  check("unknown-select-metadata", 0, [](Fixture &f) {
    auto &c = f.module.getContext(); f.select->setMetadata("fixture.unknown", MDNode::get(c, MDString::get(c, "keep")));
  });
  check("wrong-constant", 0, [](Fixture &f) { f.select->setFalseValue(ConstantFP::get(f.select->getType(), .5)); });
  check("known-select-metadata", 1, [](Fixture &f) {
    auto &c = f.module.getContext(); IRBuilder<> ir(c);
    f.select->setMetadata(LLVMContext::MD_prof, MDNode::get(c, {MDString::get(c, "branch_weights"),
      ConstantAsMetadata::get(ir.getInt32(7)), ConstantAsMetadata::get(ir.getInt32(3))}));
    f.select->setMetadata(LLVMContext::MD_unpredictable, MDNode::get(c, {}));
    f.select->setMetadata(LLVMContext::MD_fpmath, MDNode::get(c, ConstantAsMetadata::get(ConstantFP::get(ir.getFloatTy(), 2.5))));
    f.select->setMetadata(LLVMContext::MD_annotation, MDNode::get(c, MDString::get(c, "preserve-select")));
  });
  LLVMContext context; Module module("fake-loop", context); Fixture fixture(module, "fake", 9, true, false, true);
  require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 0, "cross-block chain was accepted without proof");
  cleanup(module);
  require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 1, "actual CFG prepass did not expose nine-tap chain");
  require(!verifyModule(module, &errs()), "CFG prepass generated invalid IR"); ++tests;
}

static void emitNative(StringRef path) {
  LLVMContext context; Module module("pcf-native-parity", context); Function *sampler = nullptr, *fma = nullptr;
  for (unsigned i = 0; i < 4; ++i) for (bool guard : {false, true}) {
    Fixture fixture(module, (guard ? "guarded_" : "baseline_") + std::to_string(i), 9, i & 1, i & 2);
    if (guard) require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 1, "native fixture was not guarded");
    sampler = fixture.sampler; fma = fixture.fma;
  }
  IRBuilder<> ir(context);
  auto *counter = new GlobalVariable(module, ir.getInt32Ty(), false, GlobalValue::ExternalLinkage, ir.getInt32(0), "sample_calls");
  // These definitions are a test oracle for call count and arithmetic parity,
  // not a GPU texture model. Metal tests separately validate actual sampling.
  sampler->removeFnAttr(Attribute::ReadOnly); sampler->removeFnAttr(Attribute::ArgMemOnly);
  sampler->setLinkage(GlobalValue::InternalLinkage); sampler->addFnAttr(Attribute::NoInline);
  ir.SetInsertPoint(BasicBlock::Create(context, "entry", sampler));
  ir.CreateStore(ir.CreateAdd(ir.CreateLoad(ir.getInt32Ty(), counter), ir.getInt32(1)), counter);
  auto *x = ir.CreateExtractElement(sampler->getArg(3), ir.getInt32(0));
  auto *y = ir.CreateExtractElement(sampler->getArg(3), ir.getInt32(1));
  auto *value = ir.CreateFAdd(ir.CreateFMul(x, ConstantFP::get(ir.getFloatTy(), .125)),
    ir.CreateFAdd(ir.CreateFMul(y, ConstantFP::get(ir.getFloatTy(), .03125)), ConstantFP::get(ir.getFloatTy(), .25)));
  Value *result = ir.CreateInsertValue(UndefValue::get(sampler->getReturnType()), value, 0);
  ir.CreateRet(ir.CreateInsertValue(result, ir.getInt8(1), 1));
  fma->setLinkage(GlobalValue::InternalLinkage);
  ir.SetInsertPoint(BasicBlock::Create(context, "entry", fma));
  auto *intrinsic = Intrinsic::getDeclaration(&module, Intrinsic::fma, {ir.getFloatTy()});
  ir.CreateRet(ir.CreateCall(intrinsic, {fma->getArg(0), fma->getArg(1), fma->getArg(2)}));
  require(!verifyModule(module, &errs()), "invalid native execution fixture");
  std::error_code error; raw_fd_ostream output(path, error); require(!error, "cannot write native execution fixture");
  module.print(output, nullptr);
}
int main(int argc, char **argv) {
  require(argc == 2, "usage: guarded-pcf-check native-fixture.ll"); regressions(); emitNative(argv[1]);
  outs() << "PASS: " << tests << " production PCF transform/CFG/poison/metadata/rejection checks\n";
}
