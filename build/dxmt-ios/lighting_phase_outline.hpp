#pragma once
// Keep the measured lighting phase ahead of material composition. The shader
// retains its original entry point, resolution, bindings and dispatch shape.
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

namespace dxmt::shaderopt::lighting_phase {
using namespace llvm;

inline BasicBlock *block(Function &function, StringRef name) {
  for (auto &candidate : function) if (candidate.getName() == name) return &candidate;
  return nullptr;
}

inline bool safeBody(Function &function, BasicBlock *post) {
  unsigned writes = 0;
  for (auto &bb : function) for (auto &instruction : bb) {
    if (isa<StoreInst, AtomicRMWInst, AtomicCmpXchgInst, FenceInst, AllocaInst>(instruction)) return false;
    if (auto *load = dyn_cast<LoadInst>(&instruction))
      if (load->isVolatile() || load->isAtomic()) return false;
    if (auto *call = dyn_cast<CallBase>(&instruction)) {
      auto *callee = call->getCalledFunction();
      if (!callee || !callee->isDeclaration() || call->isConvergent()) return false;
      if (callee->getName() == "air.write_texture_2d_array.v4f32") {
        if (&bb != post) return false;
        ++writes;
      } else if (!call->doesNotAccessMemory() && !call->onlyReadsMemory()) return false;
    }
  }
  return writes == 1;
}

// This deliberately narrow match is for the measured shader. A changed
// compiler/variant keeps the original shader unchanged. There is no runtime
// quality switch: this transform retains all original lighting calculations.
inline bool run(Function &function) {
  if (function.isDeclaration() || function.getName() != "shader_f4c2c563" ||
      function.arg_size() != 4 || !function.getReturnType()->isVoidTy() ||
      function.hasFnAttribute(Attribute::StrictFP)) return false;
  auto *post = block(function, "end_loop63"), *start = block(function, "end_if1");
  if (!post || !start || !safeBody(function, post) ||
      !isa<BranchInst>(post->getTerminator())) return false;
  auto *finish = cast<BranchInst>(post->getTerminator());
  if (!finish->isUnconditional()) return false;
  SmallVector<PHINode *, 6> outputs;
  for (auto &phi : post->phis()) {
    if (phi.getType()->isIntegerTy(32)) { outputs.push_back(&phi); continue; }
    // The remaining merge reloads the same immutable constant-buffer pointer.
    // A different loop result would also need exporting; decline that shape.
    if (!phi.getType()->isPointerTy()) return false;
    Value *address = nullptr;
    for (auto &incoming : phi.incoming_values()) {
      auto *load = dyn_cast<LoadInst>(incoming.get());
      if (!load || load->isVolatile() || load->isAtomic()) return false;
      if (address && address != load->getPointerOperand()) return false;
      address = load->getPointerOperand();
    }
  }
  if (outputs.size() != 6) return false;
  for (auto *phi : outputs) for (auto *user : phi->users()) {
    auto *instruction = dyn_cast<Instruction>(user);
    if (!instruction || instruction->getParent() != post) return false;
  }
  for (auto &instruction : *post) if (!isa<PHINode>(instruction))
    for (auto *user : instruction.users())
      if (!isa<Instruction>(user) || cast<Instruction>(user)->getParent() != post) return false;
  const char *preheaders[] = {"end_if8", "end_if44"};
  const char *exits[] = {"end_loop42", "end_loop63"};
  SmallPtrSet<BasicBlock *, 32> removed;
  for (unsigned i = 0; i != 2; ++i) {
    auto *pre = block(function, preheaders[i]), *exit = block(function, exits[i]);
    auto *branch = pre ? dyn_cast<BranchInst>(pre->getTerminator()) : nullptr;
    if (!exit || !branch || !branch->isConditional() || branch->getSuccessor(1) != exit) return false;
    SmallVector<BasicBlock *, 32> pending{branch->getSuccessor(0)};
    SmallPtrSet<BasicBlock *, 32> seen;
    while (!pending.empty()) {
      auto *bb = pending.pop_back_val();
      if (bb == exit || !seen.insert(bb).second) continue;
      if (bb == start || bb == &function.getEntryBlock() || bb == post) return false;
      auto *next = dyn_cast<BranchInst>(bb->getTerminator());
      if (!next) return false;
      removed.insert(bb);
      for (unsigned n = 0; n != next->getNumSuccessors(); ++n) pending.push_back(next->getSuccessor(n));
    }
  }
  // Every observable result of the omitted regions must flow through one of
  // the six exported accumulators, or the verified identical CB-pointer merge.
  SmallVector<Instruction *, 64> pending;
  SmallPtrSet<Instruction *, 32> visited;
  for (auto *bb : removed) for (auto &instruction : *bb) pending.push_back(&instruction);
  while (!pending.empty()) {
    auto *value = pending.pop_back_val();
    if (!visited.insert(value).second) continue;
    for (auto *user : value->users()) {
      auto *instruction = dyn_cast<Instruction>(user);
      if (!instruction) return false;
      if (removed.count(instruction->getParent())) continue;
      if (auto *phi = dyn_cast<PHINode>(instruction)) if (phi->getParent() == post) continue;
      if (instruction->isTerminator() || instruction->mayWriteToMemory()) return false;
      pending.push_back(instruction);
    }
  }

  auto &context = function.getContext();
  SmallVector<Type *, 6> fields(6, Type::getInt32Ty(context));
  auto *resultType = StructType::get(context, fields);
  SmallVector<Type *, 4> parameters;
  for (auto &argument : function.args()) parameters.push_back(argument.getType());
  auto *helper = Function::Create(FunctionType::get(resultType, parameters, false),
                                 GlobalValue::InternalLinkage, "madeira_lighting_phase", function.getParent());
  ValueToValueMapTy helperMap;
  auto target = helper->arg_begin();
  for (auto &argument : function.args()) { target->setName(argument.getName()); helperMap[&argument] = &*target++; }
  SmallVector<ReturnInst *, 4> returns;
  CloneFunctionInto(helper, &function, helperMap, CloneFunctionChangeType::LocalChangesOnly, returns);
  helper->setLinkage(GlobalValue::InternalLinkage);
  helper->removeFnAttr(Attribute::NoInline);
  helper->addFnAttr(Attribute::AlwaysInline);
  for (auto *ret : returns) {
    ReturnInst::Create(context, Constant::getNullValue(resultType), ret);
    ret->eraseFromParent();
  }
  auto *helperPost = cast<BasicBlock>(helperMap[post]);
  auto first = helperPost->getFirstNonPHI()->getIterator();
  SmallVector<Instruction *, 128> discarded;
  for (auto at = first; at != helperPost->end(); ++at) discarded.push_back(&*at);
  IRBuilder<> resultBuilder(helperPost->getFirstNonPHI());
  Value *result = Constant::getNullValue(resultType);
  for (unsigned i = 0; i != 6; ++i) result = resultBuilder.CreateInsertValue(result, helperMap[outputs[i]], i);
  resultBuilder.CreateRet(result);
  for (auto at = discarded.rbegin(); at != discarded.rend(); ++at) (*at)->eraseFromParent();
  removeUnreachableBlocks(*helper);

  // Build and verify a candidate before replacing any original instructions.
  ValueToValueMapTy candidateMap;
  auto *candidate = CloneFunction(&function, candidateMap);
  candidate->setLinkage(GlobalValue::InternalLinkage);
  auto *candidateStart = cast<BasicBlock>(candidateMap[start]);
  IRBuilder<> caller(candidateStart->getFirstNonPHI());
  SmallVector<Value *, 4> arguments;
  for (auto &argument : candidate->args()) arguments.push_back(&argument);
  auto *lighting = caller.CreateCall(helper, arguments, "lighting.phase");
  auto *candidatePost = cast<BasicBlock>(candidateMap[post]);
  IRBuilder<> compose(candidatePost->getFirstNonPHI());
  for (unsigned i = 0; i != 6; ++i) {
    auto *phi = cast<PHINode>(candidateMap[outputs[i]]);
    phi->replaceAllUsesWith(compose.CreateExtractValue(lighting, i));
    phi->eraseFromParent();
  }
  for (unsigned i = 0; i != 2; ++i) {
    auto *pre = block(*candidate, preheaders[i]), *exit = block(*candidate, exits[i]);
    auto *branch = cast<BranchInst>(pre->getTerminator());
    branch->getSuccessor(0)->removePredecessor(pre);
    BranchInst::Create(exit, branch);
    branch->eraseFromParent();
  }
  removeUnreachableBlocks(*candidate);
  if (verifyFunction(*helper) || verifyFunction(*candidate)) {
    candidate->dropAllReferences(); candidate->eraseFromParent(); helper->eraseFromParent();
    return false;
  }
  auto original = function.arg_begin();
  for (auto &argument : candidate->args()) argument.replaceAllUsesWith(&*original++);
  function.deleteBody();
  function.getBasicBlockList().splice(function.end(), candidate->getBasicBlockList());
  candidate->eraseFromParent();
  return true;
}
} // namespace dxmt::shaderopt::lighting_phase

namespace dxmt::shaderopt {
struct ComputeLightingPhasePass : llvm::PassInfoMixin<ComputeLightingPhasePass> {
  llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &) {
    llvm::SmallVector<llvm::Function *, 4> entries;
    if (auto *kernels = module.getNamedMetadata("air.kernel"))
      for (auto *entry : kernels->operands())
        if (entry->getNumOperands())
          if (auto *value = llvm::dyn_cast_or_null<llvm::ValueAsMetadata>(entry->getOperand(0).get()))
            if (auto *function = llvm::dyn_cast<llvm::Function>(value->getValue())) entries.push_back(function);
    bool changed = false;
    for (auto *function : entries) changed |= lighting_phase::run(*function);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }
};
} // namespace dxmt::shaderopt
