#pragma once
// A separate entry is used only with a zero-extended light-buffer view. The
// original entry and its original bounds checks remain available unchanged.
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"

namespace dxmt::shaderopt::padded_lighting {
using namespace llvm;
inline constexpr uint32_t Bytes = 65536 * 56 * 4 + 16;
inline constexpr const char *Entry = "madeira_lighting_padded_f4c2c563";

inline void simplify(Function &function) {
  // Limit cleanup to this new helper. Do not reorder the guarded-PCF passes
  // or run another global optimization pipeline over unrelated shaders.
  legacy::FunctionPassManager cleanup(function.getParent());
  cleanup.add(createInstructionCombiningPass());
  cleanup.add(createCFGSimplificationPass());
  cleanup.add(createDeadCodeEliminationPass());
  cleanup.add(createInstructionCombiningPass());
  cleanup.add(createCFGSimplificationPass());
  cleanup.doInitialization(); cleanup.run(function); cleanup.doFinalization();
}

inline bool rootField(Value *pointer, Argument *root, unsigned index) {
  auto *gep = dyn_cast<GetElementPtrInst>(pointer);
  if (!gep || gep->getPointerOperand() != root || gep->getNumIndices() != 2) return false;
  auto *zero = dyn_cast<ConstantInt>(gep->getOperand(1));
  auto *field = dyn_cast<ConstantInt>(gep->getOperand(2));
  return zero && zero->isZero() && field && field->equalsInt(index);
}

inline bool run(Module &module) {
  if (module.getFunction(Entry)) return false;
  auto *original = module.getFunction("shader_f4c2c563");
  auto *phase = module.getFunction("madeira_lighting_phase");
  // The preceding phase pass creates this helper only after its exact shader
  // fingerprint, control-flow, purity and observable-output checks have passed.
  if (!original || !phase || original->arg_size() != 4 || phase->arg_size() != 4 ||
      !original->getReturnType()->isVoidTy() || !phase->hasLocalLinkage() ||
      !phase->hasFnAttribute(Attribute::AlwaysInline)) return false;
  auto *pointer = dyn_cast<PointerType>(original->getArg(2)->getType());
  if (!pointer || pointer->isOpaque()) return false;
  auto *layout = dyn_cast<StructType>(pointer->getNonOpaquePointerElementType());
  if (!layout || layout->getNumElements() != 75 ||
      !layout->getElementType(19)->isPointerTy() || !layout->getElementType(20)->isIntegerTy(64)) return false;
  const auto *offsets = module.getDataLayout().getStructLayout(layout);
  if (offsets->getSizeInBytes() != 600 || offsets->getElementOffset(19) != 152 ||
      offsets->getElementOffset(20) != 160 || phase->getArg(2)->getType() != original->getArg(2)->getType()) return false;

  MDNode *kernel = nullptr;
  auto *kernels = module.getNamedMetadata("air.kernel");
  if (!kernels) return false;
  for (auto *candidate : kernels->operands()) {
    if (candidate->getNumOperands() < 3) continue;
    auto *value = dyn_cast_or_null<ValueAsMetadata>(candidate->getOperand(0).get());
    if (value && value->getValue() == original) { kernel = candidate; break; }
  }
  if (!kernel) return false;
  unsigned calls = 0;
  for (auto &block : *original) for (auto &instruction : block)
    if (auto *call = dyn_cast<CallInst>(&instruction)) calls += call->getCalledFunction() == phase;
  if (calls != 1) return false;

  SmallVector<LoadInst *, 16> lengths;
  for (auto &block : *phase) for (auto &instruction : block) {
    auto *load = dyn_cast<LoadInst>(&instruction);
    if (!load || !rootField(load->getPointerOperand(), phase->getArg(2), 20)) continue;
    if (load->isVolatile() || load->isAtomic() || !load->getType()->isIntegerTy(64)) return false;
    // Do not freeze resource-kind/stride bits or any other part of metadata.
    for (auto *user : load->users()) {
      auto *truncation = dyn_cast<TruncInst>(user);
      if (!truncation || !truncation->getType()->isIntegerTy(32)) return false;
    }
    lengths.push_back(load);
  }
  if (lengths.empty()) return false;

  ValueToValueMapTy phaseMap, entryMap;
  auto *fastPhase = CloneFunction(phase, phaseMap);
  fastPhase->setName("madeira_lighting_padded_phase");
  fastPhase->setLinkage(GlobalValue::InternalLinkage);
  for (auto *length : lengths) {
    auto *copy = cast<LoadInst>(phaseMap[length]);
    copy->replaceAllUsesWith(ConstantInt::get(copy->getType(), Bytes));
    copy->eraseFromParent();
  }
  auto *fast = CloneFunction(original, entryMap);
  fast->setName(Entry);
  fast->setLinkage(original->getLinkage());
  for (auto &block : *fast) for (auto &instruction : block)
    if (auto *call = dyn_cast<CallInst>(&instruction))
      if (call->getCalledFunction() == phase) call->setCalledFunction(fastPhase);
  simplify(*fastPhase);
  if (verifyFunction(*fastPhase) || verifyFunction(*fast)) {
    fast->dropAllReferences(); fast->eraseFromParent(); fastPhase->eraseFromParent(); return false;
  }
  SmallVector<Metadata *, 4> metadata;
  for (auto &operand : kernel->operands()) metadata.push_back(operand.get());
  metadata[0] = ValueAsMetadata::get(fast);
  kernels->addOperand(MDNode::get(module.getContext(), metadata));
  return true;
}
} // namespace dxmt::shaderopt::padded_lighting

namespace dxmt::shaderopt {
struct ComputePaddedLightTablePass : llvm::PassInfoMixin<ComputePaddedLightTablePass> {
  llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &) {
    return padded_lighting::run(module) ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }
};
} // namespace dxmt::shaderopt
