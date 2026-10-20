#pragma once
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"

namespace dxmt::shaderopt {
namespace pcf_detail {

inline bool isUnshadowed(llvm::Value *value) {
  if (auto *number = llvm::dyn_cast<llvm::ConstantFP>(value))
    return number->getType()->isFloatTy() && number->isExactlyValue(1.0);
  if (auto *bits = llvm::dyn_cast<llvm::ConstantInt>(value))
    return bits->getType()->isIntegerTy(32) && bits->getZExtValue() == 0x3f800000;
  return false;
}

inline bool safeCall(const llvm::CallInst &call) {
  return !call.isConvergent() && !call.hasFnAttr(llvm::Attribute::NoDuplicate) &&
         !call.hasFnAttr(llvm::Attribute::ReturnsTwice) && call.doesNotThrow() &&
         call.hasFnAttr(llvm::Attribute::WillReturn) && !call.hasOperandBundles();
}

inline bool isExplicitCompare(const llvm::Instruction &instruction) {
  auto *call = llvm::dyn_cast<llvm::CallInst>(&instruction);
  if (!call || !safeCall(*call) || !call->onlyReadsMemory()) return false;
  auto *function = call->getCalledFunction();
  if (!function || !function->isDeclaration() ||
      function->getName() != "air.sample_compare_depth_2d.f32" || call->arg_size() != 11) return false;
  auto *mode = llvm::dyn_cast<llvm::ConstantInt>(call->getArgOperand(7));
  auto *result = llvm::dyn_cast<llvm::StructType>(call->getType());
  return mode && mode->getType()->isIntegerTy(1) && mode->isOne() && result &&
         result->getNumElements() == 2 && result->getElementType(0)->isFloatTy() &&
         result->getElementType(1)->isIntegerTy(8);
}

inline bool isPureMath(const llvm::Instruction &instruction) {
  if (auto *call = llvm::dyn_cast<llvm::CallInst>(&instruction)) {
    auto *function = call->getCalledFunction();
    return function && safeCall(*call) && call->doesNotAccessMemory() &&
           (function->getName() == "air.fma.f32" || function->getIntrinsicID() == llvm::Intrinsic::fma);
  }
  return llvm::isa<llvm::BinaryOperator, llvm::CastInst, llvm::ExtractValueInst,
                   llvm::InsertValueInst, llvm::ExtractElementInst, llvm::InsertElementInst,
                   llvm::ShuffleVectorInst, llvm::CmpInst, llvm::SelectInst>(instruction);
}

inline bool isSafeSpanInstruction(const llvm::Instruction &instruction) {
  if (auto *load = llvm::dyn_cast<llvm::LoadInst>(&instruction))
    return !load->isVolatile() && !load->isAtomic();
  return isExplicitCompare(instruction) || isPureMath(instruction) ||
         llvm::isa<llvm::GetElementPtrInst>(instruction);
}

struct Chain {
  llvm::BasicBlock *block;
  llvm::DenseMap<llvm::Instruction *, bool> dependent;
  llvm::SmallPtrSet<llvm::Instruction *, 32> moved;
  unsigned samples = 0;
  bool valid = true;
  explicit Chain(llvm::BasicBlock *block) : block(block) {}

  // First find the mandatory sample-dependent operations. Private pure setup
  // can join this set after the span and exclusive-use checks below.
  bool visit(llvm::Value *value, unsigned depth = 0) {
    auto *instruction = llvm::dyn_cast<llvm::Instruction>(value);
    if (!instruction || instruction->getParent() != block || llvm::isa<llvm::PHINode>(instruction)) return false;
    auto previous = dependent.find(instruction);
    if (previous != dependent.end()) return previous->second;
    if (depth > 128 || dependent.size() >= 1024) { valid = false; return false; }
    dependent[instruction] = false;
    bool usesSample = isExplicitCompare(*instruction);
    if (auto *call = llvm::dyn_cast<llvm::CallInst>(instruction))
      if (auto *function = call->getCalledFunction())
        if (function->getName().startswith("air.sample_") && !usesSample) valid = false;
    if (usesSample && ++samples > 32) valid = false;
    for (auto &operand : instruction->operands()) usesSample |= visit(operand.get(), depth + 1);
    dependent[instruction] = usesSample;
    if (usesSample) {
      if (!isExplicitCompare(*instruction) && !isPureMath(*instruction)) valid = false;
      moved.insert(instruction);
      if (moved.size() > 256) valid = false;
    }
    return usesSample;
  }
};

inline bool guardSelect(llvm::SelectInst &select) {
  if (!select.getCondition()->getType()->isIntegerTy(1)) return false;
  const bool trueConstant = isUnshadowed(select.getTrueValue());
  const bool falseConstant = isUnshadowed(select.getFalseValue());
  if (trueConstant == falseConstant) return false;
  auto *computed = trueConstant ? select.getFalseValue() : select.getTrueValue();
  auto *constant = trueConstant ? select.getTrueValue() : select.getFalseValue();
  auto *block = select.getParent();
  Chain chain{block};
  if (!chain.visit(computed) || !chain.valid || chain.samples < 4) return false;

  // A status/residency use, shared sum or predicate dependent on the samples
  // prevents guarding the calls. No use on the skipped path may be lost.
  for (auto *instruction : chain.moved) for (auto &use : instruction->uses()) {
    if (use.getUser() == &select) {
      if (use.getOperandNo() != (trueConstant ? 2u : 1u)) return false;
    } else {
      auto *user = llvm::dyn_cast<llvm::Instruction>(use.getUser());
      if (!user || !chain.moved.count(user)) return false;
    }
  }

  // Work only after the last write, atomic, barrier, derivative or unknown
  // operation before the select. Every mandatory node must be in this window;
  // an optional producer before it stays where it was. Loads and GEPs may be
  // crossed under the existing span proof, but are never setup candidates.
  llvm::SmallVector<llvm::Instruction *, 64> window;
  llvm::SmallPtrSet<llvm::Instruction *, 32> inWindow;
  for (auto &instruction : *block) {
    if (&instruction == &select) break;
    if (!isSafeSpanInstruction(instruction)) { window.clear(); inWindow.clear(); }
    else { window.push_back(&instruction); inWindow.insert(&instruction); }
  }
  for (auto *instruction : chain.moved) if (!inWindow.count(instruction)) return false;

  // Sink only pure producers whose complete user set is already moving. This
  // keeps shared coordinates, predicates, memory reads and GEPs in
  // place. Seed in source order so the bounded subset is repeatable. Requeue
  // operands when adding a user: an earlier failed diamond ancestor can then
  // become exclusive. Reaching the cap leaves the remaining setup in place.
  llvm::SmallVector<llvm::Instruction *, 64> pending;
  auto enqueueOperands = [&](llvm::Instruction *instruction) {
    for (auto &operand : instruction->operands())
      if (auto *producer = llvm::dyn_cast<llvm::Instruction>(operand.get()))
        if (inWindow.count(producer) && !chain.moved.count(producer)) pending.push_back(producer);
  };
  for (auto *instruction : window) if (chain.moved.count(instruction)) enqueueOperands(instruction);
  for (unsigned index = 0; index < pending.size() && chain.moved.size() < 256; ++index) {
    auto *producer = pending[index];
    if (chain.moved.count(producer) || !isPureMath(*producer)) continue;
    bool exclusive = true;
    for (auto *user : producer->users()) {
      auto *instruction = llvm::dyn_cast<llvm::Instruction>(user);
      if (!instruction || !chain.moved.count(instruction)) { exclusive = false; break; }
    }
    if (!exclusive) continue;
    chain.moved.insert(producer);
    enqueueOperands(producer);
  }

  llvm::SmallVector<llvm::Instruction *, 64> ordered;
  for (auto *instruction : window) if (chain.moved.count(instruction)) ordered.push_back(instruction);
  if (ordered.size() != chain.moved.size()) return false;

  llvm::SmallVector<std::pair<unsigned, llvm::MDNode *>, 4> metadata;
  select.getAllMetadataOtherThanDebugLoc(metadata);
  for (const auto &entry : metadata)
    if (entry.first != llvm::LLVMContext::MD_prof && entry.first != llvm::LLVMContext::MD_unpredictable &&
        entry.first != llvm::LLVMContext::MD_fpmath && entry.first != llvm::LLVMContext::MD_annotation) return false;

  auto *merge = block->splitBasicBlock(select.getIterator(), "pcf.merge");
  auto *sample = llvm::BasicBlock::Create(block->getContext(), "pcf.sample", block->getParent(), merge);
  auto *done = llvm::BranchInst::Create(merge, sample);
  done->setDebugLoc(select.getDebugLoc());
  for (auto *instruction : ordered) instruction->moveBefore(done);
  block->getTerminator()->eraseFromParent();
  // select can yield poison without immediate UB; a branch cannot consume
  // poison. Freeze refines that case while retaining every defined predicate.
  auto *condition = new llvm::FreezeInst(select.getCondition(), "pcf.condition", block);
  condition->setDebugLoc(select.getDebugLoc());
  auto *branch = llvm::BranchInst::Create(trueConstant ? merge : sample, trueConstant ? sample : merge,
                                         condition, block);
  branch->setDebugLoc(select.getDebugLoc());
  auto *result = llvm::PHINode::Create(select.getType(), 2, "", &select);
  result->takeName(&select);
  result->setDebugLoc(select.getDebugLoc());
  if (llvm::isa<llvm::FPMathOperator>(&select)) result->copyFastMathFlags(&select);
  for (const auto &entry : metadata) {
    if (entry.first == llvm::LLVMContext::MD_prof || entry.first == llvm::LLVMContext::MD_unpredictable)
      branch->setMetadata(entry.first, entry.second);
    else result->setMetadata(entry.first, entry.second);
  }
  result->addIncoming(computed, sample);
  result->addIncoming(constant, block);
  select.replaceAllUsesWith(result);
  select.eraseFromParent();
  return true;
}
} // namespace pcf_detail

// Experimental: not in the production pipeline until captured-kernel compiler
// checks and nonzero Metal fixtures demonstrate parity and a useful benefit.
// Run after CFG cleanup and the existing explicit-LOD convergence proof.
inline unsigned guardConstantPCFSampling(llvm::Function &function) {
  if (function.hasFnAttribute(llvm::Attribute::StrictFP)) return 0;
  llvm::SmallVector<llvm::SelectInst *, 16> candidates;
  for (auto &block : function) for (auto &instruction : block)
    if (auto *select = llvm::dyn_cast<llvm::SelectInst>(&instruction)) candidates.push_back(select);
  unsigned guarded = 0;
  for (auto *select : candidates) guarded += pcf_detail::guardSelect(*select);
  return guarded;
}

struct GuardedPCFPass : llvm::PassInfoMixin<GuardedPCFPass> {
  llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &) {
    return guardConstantPCFSampling(function) ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }
};
} // namespace dxmt::shaderopt
