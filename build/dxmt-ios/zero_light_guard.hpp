#pragma once
// Skip explicit shadow work whose contribution is provably zero.
// The compute-only wrapper is the production entry; retain the original FP math.
#include "guarded_pcf.hpp"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/SmallSet.h"

namespace dxmt::shaderopt::zero_light {
using namespace llvm;

inline bool maxTree(Value *value, Value *&vector, unsigned &lanes, unsigned depth = 0) {
  if (depth > 4) return false;
  if (auto *extract = dyn_cast<ExtractElementInst>(value)) {
    auto *index = dyn_cast<ConstantInt>(extract->getIndexOperand());
    auto *type = dyn_cast<FixedVectorType>(extract->getVectorOperandType());
    if (!index || !type || type->getNumElements() != 3 || !type->getElementType()->isFloatTy() ||
        index->getZExtValue() >= 3) return false;
    if (vector && vector != extract->getVectorOperand()) return false;
    vector = extract->getVectorOperand(); lanes |= 1u << index->getZExtValue();
    return true;
  }
  auto *call = dyn_cast<CallInst>(value);
  auto *function = call ? call->getCalledFunction() : nullptr;
  return function && function->isDeclaration() && function->getName() == "air.fast_fmax.f32" &&
         call->arg_size() == 2 && call->doesNotAccessMemory() && pcf_detail::safeCall(*call) &&
         maxTree(call->getArgOperand(0), vector, lanes, depth + 1) &&
         maxTree(call->getArgOperand(1), vector, lanes, depth + 1);
}

// A bitcast and all-zero shuffle select only lane 0 of the insertion; poison
// in the other lanes is never observed. Do not accept arbitrary shuffles.
inline Value *splatScalar(Value *value) {
  auto *shuffle = dyn_cast<ShuffleVectorInst>(value);
  if (!shuffle || shuffle->getShuffleMask().size() != 3) return nullptr;
  for (int lane : shuffle->getShuffleMask()) if (lane != 0) return nullptr;
  Value *inserted = shuffle->getOperand(0);
  if (auto *cast = dyn_cast<BitCastInst>(inserted)) inserted = cast->getOperand(0);
  auto *insert = dyn_cast<InsertElementInst>(inserted);
  auto *index = insert ? dyn_cast<ConstantInt>(insert->getOperand(2)) : nullptr;
  if (!index || !index->isZero()) return nullptr;
  auto *scalar = insert->getOperand(1);
  if (!scalar->getType()->isIntegerTy(32) && !scalar->getType()->isFloatTy()) return nullptr;
  return scalar;
}

inline void factors(Value *value, SmallVectorImpl<Value *> &result, unsigned depth = 0) {
  if (depth > 4) return;
  if (auto *multiply = dyn_cast<BinaryOperator>(value)) {
    if (multiply->getOpcode() == Instruction::FMul) {
      factors(multiply->getOperand(0), result, depth + 1);
      factors(multiply->getOperand(1), result, depth + 1);
      return;
    }
  }
  if (auto *scalar = splatScalar(value)) result.push_back(scalar);
}

inline bool pureInstruction(Instruction &instruction, unsigned &samples) {
  if (isa<PHINode, BranchInst>(instruction)) return true;
  if (auto *load = dyn_cast<LoadInst>(&instruction)) return !load->isVolatile() && !load->isAtomic();
  if (auto *call = dyn_cast<CallInst>(&instruction)) {
    if (pcf_detail::isExplicitCompare(*call)) { ++samples; return true; }
    auto *function = call->getCalledFunction();
    if (!function || !function->isDeclaration() || !pcf_detail::safeCall(*call) ||
        !call->doesNotAccessMemory()) return false;
    // These calls have no cross-lane or derivative semantics. An unknown
    // readnone intrinsic can still be convergent or otherwise unsafe to skip.
    const StringRef name = function->getName();
    return name.startswith("air.fma.") || name.startswith("air.fast_fmin.") ||
           name.startswith("air.fast_fmax.") || name.startswith("air.fast_fabs.") ||
           name.startswith("air.fast_saturate.") || name.startswith("air.fast_sqrt.") ||
           name.startswith("air.fast_rsqrt.") || name.startswith("air.fast_pow.") ||
           name.startswith("air.fast_floor.") ||
           name.startswith("air.dot.") || name.startswith("air.convert.");
  }
  return isa<BinaryOperator, UnaryOperator, CastInst, ExtractValueInst, InsertValueInst,
             ExtractElementInst, InsertElementInst, ShuffleVectorInst, CmpInst,
             SelectInst, GetElementPtrInst, FreezeInst>(instruction);
}

struct Region {
  BasicBlock *entry, *exit;
  DominatorTree &dominators;
  SmallPtrSet<BasicBlock *, 32> blocks, visiting;
  unsigned instructions = 0, samples = 0;

  bool collect(BasicBlock *block) {
    if (block == exit) return true;
    if (block == entry || visiting.count(block)) return false; // no loops
    if (blocks.count(block)) return true;
    if (blocks.size() >= 256 || !dominators.dominates(entry, block)) return false;
    auto *branch = dyn_cast<BranchInst>(block->getTerminator());
    if (!branch) return false;
    blocks.insert(block); visiting.insert(block);
    for (auto &instruction : *block) {
      if (++instructions > 4096 || !pureInstruction(instruction, samples)) return false;
    }
    for (unsigned i = 0; i < branch->getNumSuccessors(); ++i)
      if (!collect(branch->getSuccessor(i))) return false;
    visiting.erase(block);
    return true;
  }

  bool closedUses() {
    for (auto *block : blocks) for (auto &instruction : *block) for (auto &use : instruction.uses()) {
      auto *user = dyn_cast<Instruction>(use.getUser());
      if (!user) return false;
      if (blocks.count(user->getParent())) continue;
      auto *phi = dyn_cast<PHINode>(user);
      if (!phi || phi->getParent() != exit || !blocks.count(phi->getIncomingBlock(use))) return false;
    }
    return true;
  }
};

inline bool guard(BranchInst &test, Value *scalar, DominatorTree &dominators) {
  auto *producer = dyn_cast<Instruction>(scalar);
  if (!producer || !isa<PHINode>(producer)) return false;
  auto *entry = producer->getParent(), *exit = test.getSuccessor(1);
  if (entry == test.getParent() || entry == exit || !dominators.dominates(entry, test.getParent())) return false;
  auto *branch = dyn_cast<BranchInst>(entry->getTerminator());
  if (!branch || !branch->isConditional()) return false;
  if (branch->getSuccessor(0) == exit || branch->getSuccessor(1) == exit) return false;
  Region region{entry, exit, dominators};
  for (unsigned i = 0; i < branch->getNumSuccessors(); ++i)
    if (!region.collect(branch->getSuccessor(i))) return false;
  if (!region.blocks.count(test.getParent()) || region.samples < 4 || !region.closedUses()) return false;

  SmallVector<std::pair<PHINode *, Constant *>, 8> merges;
  for (auto &phi : exit->phis()) {
    const int index = phi.getBasicBlockIndex(test.getParent());
    if (index < 0) return false;
    auto *value = dyn_cast<Constant>(phi.getIncomingValue(index));
    if (!value || !value->isNullValue()) return false;
    // A path can leave the region before reaching the colour test. Such a
    // path must also contribute zero; otherwise the new edge would discard
    // its result. Nonzero contributions are safe only behind the test.
    for (unsigned i = 0; i < phi.getNumIncomingValues(); ++i) {
      auto *predecessor = phi.getIncomingBlock(i);
      if (!region.blocks.count(predecessor) || dominators.dominates(test.getParent(), predecessor)) continue;
      auto *other = dyn_cast<Constant>(phi.getIncomingValue(i));
      if (!other || !other->isNullValue()) return false;
    }
    merges.push_back({&phi, value});
  }
  if (merges.empty()) return false;

  // For +/-0 attenuation, multiplication produces only zero or NaN, even
  // for infinite colours/shadows. max of those cannot pass ordered > 0.
  // Preserve the ORIGINAL false-path PHIs, rather than substituting results
  // in the middle of floating-point expressions (0*Inf is not zero).
  auto *work = entry->splitBasicBlock(branch->getIterator(), "lighting.nonzero");
  auto *old = entry->getTerminator();
  IRBuilder<> ir(old);
  Value *bits = scalar;
  if (bits->getType()->isFloatTy()) bits = ir.CreateBitCast(bits, ir.getInt32Ty());
  bits = ir.CreateFreeze(bits, "lighting.attenuation.bits");
  auto *zero = ir.CreateICmpEQ(ir.CreateAnd(bits, ir.getInt32(0x7fffffff)), ir.getInt32(0), "lighting.zero");
  ir.CreateCondBr(zero, exit, work); old->eraseFromParent();
  for (auto [phi, value] : merges) phi->addIncoming(value, entry);
  dominators.recalculate(*entry->getParent());
  return true;
}

inline unsigned run(Function &function) {
  if (function.isDeclaration() || function.hasFnAttribute(Attribute::StrictFP)) return 0;
  SmallVector<BranchInst *, 16> tests;
  for (auto &block : function) {
    auto *branch = dyn_cast<BranchInst>(block.getTerminator());
    if (branch && branch->isConditional()) tests.push_back(branch);
  }
  DominatorTree dominators(function);
  unsigned changed = 0;
  for (auto *branch : tests) {
    auto *compare = dyn_cast<FCmpInst>(branch->getCondition());
    auto *zero = compare ? dyn_cast<ConstantFP>(compare->getOperand(1)) : nullptr;
    if (!compare || compare->getPredicate() != CmpInst::FCMP_OGT || !zero || !zero->isZero()) continue;
    Value *vector = nullptr; unsigned lanes = 0;
    if (!maxTree(compare->getOperand(0), vector, lanes) || lanes != 7) continue;
    auto *multiply = dyn_cast<BinaryOperator>(vector);
    if (!multiply || multiply->getOpcode() != Instruction::FMul) continue;
    SmallVector<Value *, 4> candidates;
    factors(vector, candidates);
    for (auto *scalar : candidates) if (guard(*branch, scalar, dominators)) { ++changed; break; }
  }
  return changed;
}
} // namespace dxmt::shaderopt::zero_light

namespace dxmt::shaderopt {
struct ComputeZeroLightPass : llvm::PassInfoMixin<ComputeZeroLightPass> {
  unsigned *guardedCount;
  explicit ComputeZeroLightPass(unsigned *count = nullptr) : guardedCount(count) {}
  llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &) {
    if (!function.getParent()) return llvm::PreservedAnalyses::all();
    bool compute = false;
    if (auto *kernels = function.getParent()->getNamedMetadata("air.kernel"))
      for (auto *entry : kernels->operands())
        if (entry->getNumOperands())
          if (auto *value = llvm::dyn_cast_or_null<llvm::ValueAsMetadata>(entry->getOperand(0).get()))
            compute |= value->getValue() == &function;
    if (!compute) return llvm::PreservedAnalyses::all();
    unsigned count = zero_light::run(function);
    if (guardedCount) *guardedCount += count;
    return count ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }
};
} // namespace dxmt::shaderopt

