// Exercise the PCF transform and compute hook. The emitted native fixture replaces
// AIR sampling only AFTER transforming, so it can count the calls that survive.
#include "guarded_pcf.hpp"
#include "explicit_lod_guard.hpp"
#include "llvm/IR/Dominators.h"
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
static std::string dump(Function &function) {
  std::string result; raw_string_ostream stream(result); function.print(stream); return result;
}
struct Fixture {
  Module &module;
  Function *function, *sampler, *fma;
  GlobalVariable *memory;
  SelectInst *select;
  SmallVector<CallInst *, 16> calls;
  SmallVector<Instruction *, 16> values;
  SmallVector<Instruction *, 32> setup, mustMove, mustStay;
  std::function<void(BasicBlock *)> inspect;
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
      auto *x = cast<Instruction>(ir.CreateFAdd(function->getArg(1), ConstantFP::get(f32, double(i) / 16), "coord.x"));
      setup.push_back(x);
      Value *uv = ir.CreateInsertElement(PoisonValue::get(v2f), x, ir.getInt32(0));
      setup.push_back(cast<Instruction>(uv));
      uv = ir.CreateInsertElement(uv, ConstantFP::get(f32, double(i) / 32), ir.getInt32(1));
      setup.push_back(cast<Instruction>(uv));
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

static SmallVector<Instruction *, 4> diamondSetup(Fixture &fixture) {
  IRBuilder<> ir(fixture.setup[0]); auto *f32 = ir.getFloatTy();
  auto *root = cast<Instruction>(ir.CreateFMul(fixture.function->getArg(1), ConstantFP::get(f32, .25), "diamond.root"));
  auto *left = cast<Instruction>(ir.CreateFAdd(root, ConstantFP::get(f32, .125), "diamond.left"));
  // The deeper right arm makes the first worklist visit to root fail. Adding
  // bridge must reconsider root after both of its users have moved.
  auto *bridge = cast<Instruction>(ir.CreateFAdd(root, ConstantFP::get(f32, .0625), "diamond.bridge"));
  auto *right = cast<Instruction>(ir.CreateFMul(bridge, ConstantFP::get(f32, .5), "diamond.right"));
  auto *joined = cast<Instruction>(ir.CreateFAdd(left, right, "diamond.join"));
  fixture.setup[0]->setOperand(0, joined);
  return {root, left, bridge, right, joined};
}

static SmallVector<Instruction *, 32> cappedSetup(Fixture &fixture) {
  SmallVector<Instruction *, 32> result;
  for (unsigned tap = 0; tap < fixture.calls.size(); ++tap) {
    auto *coordinate = fixture.setup[tap * 3]; IRBuilder<> ir(coordinate);
    Value *value = fixture.function->getArg(1);
    // Nine short chains exceed the optional-move cap without exceeding the
    // mandatory chain visitor's separate depth limit.
    for (unsigned i = 0; i < 30; ++i) {
      value = ir.CreateFAdd(value, ConstantFP::get(ir.getFloatTy(), 1.0 / 4096), "bounded.setup");
      result.push_back(cast<Instruction>(value));
    }
    coordinate->setOperand(0, value);
  }
  return result;
}

static unsigned tests = 0;
static void check(StringRef name, unsigned expected,
                  std::function<void(Fixture &)> edit = {}, unsigned samples = 9,
                  bool bits = false, bool reverse = false) {
  LLVMContext context; Module module(name, context); Fixture fixture(module, "candidate", samples, bits, reverse);
  if (edit) edit(fixture);
  require(!verifyModule(module, &errs()), name + ": invalid input");
  auto before = dump(module);
  auto *originalBlock = fixture.select->getParent();
  SmallVector<std::pair<Instruction *, BasicBlock *>, 16> unchanged;
  for (auto *instruction : fixture.mustStay) unchanged.emplace_back(instruction, instruction->getParent());
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
    auto *branch = cast<BranchInst>(originalBlock->getTerminator());
    auto *frozen = dyn_cast<FreezeInst>(branch->getCondition());
    require(frozen && frozen->getOperand(0) == condition, name + ": predicate was not frozen");
    require(branch->getMetadata(LLVMContext::MD_prof) == profile &&
      branch->getMetadata(LLVMContext::MD_unpredictable) == unpredictable, name + ": branch metadata lost");
    BasicBlock *sample = branch->getSuccessor(reverse ? 1 : 0);
    require(sample->getName().startswith("pcf.sample"), name + ": branch arms changed");
    for (auto *instruction : fixture.mustMove)
      require(instruction->getParent() == sample, name + ": private setup did not move");
    for (const auto &entry : unchanged)
      require(entry.first->getParent() == entry.second, name + ": shared or unsafe setup moved");
    unsigned index = 0;
    for (auto &instruction : *sample) if (auto *call = dyn_cast<CallInst>(&instruction)) {
      if (call->getCalledFunction() == fixture.sampler) {
        require(index < fixture.calls.size(), name + ": unexpected extra tap");
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
    if (fixture.inspect) fixture.inspect(sample);
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

static void markStage(Function &function, StringRef stage = "air.kernel") {
  function.getParent()->getOrInsertNamedMetadata(stage)->addOperand(
    MDNode::get(function.getContext(), ValueAsMetadata::get(&function)));
}

static unsigned runComputeHook(Module &module, bool enableGuard = true) {
  LoopAnalysisManager lam; FunctionAnalysisManager fam; CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
  PassBuilder pb; pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
  pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam); pb.crossRegisterProxies(lam, fam, cgam, mam);
  unsigned guarded = 0;
  FunctionPassManager functions;
  // Populate a CFG analysis before the hook, then verify it afterwards. This
  // exercises invalidation through the nested cleanup/guard pass manager.
  functions.addPass(RequireAnalysisPass<DominatorTreeAnalysis, Function>());
  functions.addPass(dxmt::shaderopt::ComputePCFPass(enableGuard, &guarded));
  functions.addPass(DominatorTreeVerifierPass());
  ModulePassManager pipeline;
  pipeline.addPass(createModuleToFunctionPassAdaptor(std::move(functions)));
  pipeline.run(module, mam);
  require(!verifyModule(module, &errs()), "compute hook generated invalid IR");
  return guarded;
}

static void integrationRegressions() {
  {
    LLVMContext context; Module module("compute-stage-membership", context);
    Fixture compute(module, "compute", 9, false, false, true);
    Fixture fragment(module, "fragment", 9, false, false, true);
    markStage(*compute.function); markStage(*fragment.function, "air.fragment");
    auto fragmentBefore = dump(*fragment.function);
    require(dxmt::shaderopt::guardConstantPCFSampling(*compute.function) == 0,
            "raw guard unexpectedly crossed the eight-plus-one split");
    require(runComputeHook(module) == 1, "shared compute hook did not expose the nine-tap chain");
    require(dump(*fragment.function) == fragmentBefore, "compute hook changed an unlisted fragment function");
    unsigned blocks = 0, taps = 0;
    for (auto &block : *compute.function) if (block.getName().startswith("pcf.sample")) {
      ++blocks;
      for (auto &instruction : block) taps += dxmt::shaderopt::pcf_detail::isExplicitCompare(instruction);
    }
    require(blocks == 1 && taps == 9, "compute hook did not retain all nine samples"); ++tests;
  }
  {
    LLVMContext context; Module module("compute-control", context);
    Fixture fixture(module, "control", 9, false, false, true); markStage(*fixture.function);
    require(runComputeHook(module, false) == 0 && fixture.function->size() == 1,
            "compute control did not perform matching cleanup without a guard");
    require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 1,
            "compute control cleanup did not expose the same nine-tap chain"); ++tests;
  }
  auto unchanged = [](StringRef name, unsigned count, std::function<void(Fixture &)> edit) {
    LLVMContext context; Module module(name, context);
    Fixture fixture(module, "candidate", count, false, false, true); markStage(*fixture.function);
    if (edit) edit(fixture);
    auto before = dump(module);
    require(runComputeHook(module) == 0, name + ": unexpected production guard");
    require(dump(module) == before, name + ": ineligible function received CFG cleanup"); ++tests;
  };
  unchanged("no-kernel-metadata", 9, [](Fixture &f) {
    f.module.eraseNamedMetadata(f.module.getNamedMetadata("air.kernel"));
  });
  unchanged("compute-three-taps", 3, {});
  unchanged("compute-no-unshadowed-select", 9, [](Fixture &f) {
    f.select->setFalseValue(ConstantFP::get(f.select->getType(), .5));
  });
  unchanged("compute-strict-fp", 9, [](Fixture &f) { f.function->addFnAttr(Attribute::StrictFP); });
  unchanged("compute-mixed-implicit-convergence", 9, [](Fixture &f) {
    f.sampler->addFnAttr(Attribute::Convergent);
    f.calls[0]->setArgOperand(7, ConstantInt::getFalse(f.module.getContext()));
    auto before = dump(f.module);
    require(dxmt::shaderopt::relaxExplicitLODSampling(f.module) == 0 && f.sampler->isConvergent(),
            "shipping LOD proof relaxed a mixed explicit/implicit declaration");
    require(dump(f.module) == before, "mixed explicit/implicit proof modified the module");
  });
  {
    LLVMContext context; Module module("compute-prefilter-only", context);
    Fixture fixture(module, "shared", 9, false, false, true); markStage(*fixture.function);
    IRBuilder<> ir(fixture.select->getNextNode()); ir.CreateStore(fixture.select->getTrueValue(), fixture.memory);
    require(fixture.function->size() == 3, "prefilter fixture lost its fake loop");
    require(runComputeHook(module) == 0 && fixture.function->size() == 1,
            "prefilter-positive shared sum should receive only CFG cleanup");
    unsigned taps = 0;
    for (auto &block : *fixture.function) for (auto &instruction : block)
      taps += dxmt::shaderopt::pcf_detail::isExplicitCompare(instruction);
    require(taps == 9, "prefilter-only cleanup lost sample calls"); ++tests;
  }
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
  check("shared-sum", 0, [](Fixture &f) { IRBuilder<> ir(f.select->getNextNode()); ir.CreateStore(f.select->getTrueValue(), f.memory); });
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
  check("private-coordinate-and-weight-setup", 1, [](Fixture &f) {
    f.mustMove.append(f.setup.begin(), f.setup.end());
    IRBuilder<> ir(f.calls[0]);
    auto *index = cast<BinaryOperator>(ir.CreateAdd(f.function->getArg(0), ir.getInt32(4), "private.index"));
    index->setHasNoSignedWrap(true);
    auto *converted = cast<Instruction>(ir.CreateSIToFP(index, ir.getFloatTy(), "private.index.float"));
    auto *weight = cast<Instruction>(ir.CreateFMul(converted, ConstantFP::get(ir.getFloatTy(), .02), "private.weight"));
    FastMathFlags flags; flags.setAllowContract(); weight->setFastMathFlags(flags);
    auto *annotation = MDNode::get(f.module.getContext(), MDString::get(f.module.getContext(), "private-weight"));
    weight->setMetadata(LLVMContext::MD_annotation, annotation);
    cast<CallInst>(f.values[0]->getNextNode())->setArgOperand(1, weight);
    f.mustMove.append({index, converted, weight});
    f.inspect = [index, weight, annotation](BasicBlock *) {
      require(index->hasNoSignedWrap() && weight->getFastMathFlags().allowContract(), "private setup flags lost");
      require(weight->getMetadata(LLVMContext::MD_annotation) == annotation, "private setup metadata lost");
    };
  });
  check("shared-coordinate-after-select", 1, [](Fixture &f) {
    IRBuilder<> ir(f.select->getNextNode()); ir.CreateStore(f.setup[0], f.memory);
    f.mustStay.push_back(f.setup[0]); f.mustMove.append({f.setup[1], f.setup[2]});
  });
  check("coordinate-used-by-predicate", 1, [](Fixture &f) {
    IRBuilder<> ir(f.select);
    auto *predicate = cast<Instruction>(ir.CreateFCmpOGT(f.setup[0], ConstantFP::get(ir.getFloatTy(), 0)));
    f.select->setCondition(predicate); f.mustStay.append({f.setup[0], predicate});
    f.mustMove.append({f.setup[1], f.setup[2]});
  });
  check("normal-load-and-gep-stay", 1, [](Fixture &f) {
    IRBuilder<> ir(f.setup[0]);
    auto *pointer = GetElementPtrInst::Create(ir.getFloatTy(), f.memory, {ir.getInt32(0)}, "coord.pointer", f.setup[0]);
    auto *load = ir.CreateLoad(ir.getFloatTy(), pointer, "coord.load");
    f.setup[0]->setOperand(0, load); f.mustStay.append({pointer, load});
    f.mustMove.append({f.setup[0], f.setup[1], f.setup[2]});
  });
  check("phi-stays", 1, [](Fixture &f) {
    auto *body = &f.function->getEntryBlock();
    auto *before = BasicBlock::Create(f.module.getContext(), "predecessor", f.function, body);
    IRBuilder<> ir(before); ir.CreateBr(body);
    auto *phi = PHINode::Create(ir.getFloatTy(), 1, "coord.phi", &body->front());
    phi->addIncoming(f.function->getArg(1), before); f.setup[0]->setOperand(0, phi);
    f.mustStay.push_back(phi); f.mustMove.append({f.setup[0], f.setup[1], f.setup[2]});
  });
  for (const char *name : {"air.wg.barrier", "unknown.early.read"})
    check(name, 1, [name](Fixture &f) {
      IRBuilder<> ir(f.setup[0]); auto *f32 = ir.getFloatTy();
      auto *before = cast<Instruction>(ir.CreateFAdd(f.function->getArg(1), ConstantFP::get(f32, .125), "before.boundary"));
      auto unknown = f.module.getOrInsertFunction(name, FunctionType::get(ir.getVoidTy(), {}, false));
      auto *boundary = ir.CreateCall(unknown); boundary->addFnAttr(Attribute::ReadOnly);
      boundary->addFnAttr(Attribute::NoUnwind); boundary->addFnAttr(Attribute::WillReturn);
      if (StringRef(name).startswith("air.")) boundary->addFnAttr(Attribute::Convergent);
      auto *after = cast<Instruction>(ir.CreateFMul(before, ConstantFP::get(f32, .5), "after.boundary"));
      f.setup[0]->setOperand(0, after); f.mustStay.append({before, boundary});
      f.mustMove.append({after, f.setup[0], f.setup[1], f.setup[2]});
    });
  check("diamond-setup-reconsidered", 1, [](Fixture &f) {
    auto nodes = diamondSetup(f); f.mustMove.append(nodes.begin(), nodes.end());
    f.mustMove.append(f.setup.begin(), f.setup.end());
  });
  check("optional-setup-cap-retains-guard", 1, [](Fixture &f) {
    auto nodes = cappedSetup(f); f.mustStay.push_back(nodes.front()); f.mustMove.push_back(nodes.back());
    f.inspect = [](BasicBlock *sample) { require(sample->size() == 257, "optional setup did not stop at 256 moved nodes"); };
  });
  LLVMContext context; Module module("fake-loop", context); Fixture fixture(module, "fake", 9, true, false, true);
  require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 0, "single local tap did not meet the four-tap threshold");
  cleanup(module);
  require(dxmt::shaderopt::guardConstantPCFSampling(*fixture.function) == 1, "actual CFG prepass did not expose nine-tap chain");
  require(!verifyModule(module, &errs()), "CFG prepass generated invalid IR"); ++tests;
}

static void emitNative(StringRef path) {
  LLVMContext context; Module module("pcf-native-parity", context); Function *sampler = nullptr, *fma = nullptr;
  for (unsigned i = 0; i < 8; ++i) for (bool guard : {false, true}) {
    Fixture fixture(module, (guard ? "guarded_" : "baseline_") + std::to_string(i), 9, i & 1, i & 2);
    if (i >= 4) { if (i & 1) cappedSetup(fixture); else diamondSetup(fixture); }
    if (guard) markStage(*fixture.function);
    sampler = fixture.sampler; fma = fixture.fma;
  }
  require(runComputeHook(module) == 8, "shared compute hook did not guard every native fixture");
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
  require(argc == 2, "usage: guarded-pcf-check native-fixture.ll"); regressions(); integrationRegressions(); emitNative(argv[1]);
  outs() << "PASS: " << tests << " production PCF transform/stage/pipeline/CFG/poison/metadata/rejection checks\n";
}
