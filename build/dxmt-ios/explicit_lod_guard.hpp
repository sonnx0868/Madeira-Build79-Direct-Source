#pragma once
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

namespace dxmt::shaderopt {

// LLVM15 can remove a call-site convergent attribute when the declaration
// lacks it. Therefore keep mixed/unknown uses fully convergent; relax a
// declaration only after proving EVERY call supplies an explicit LOD.
inline unsigned relaxExplicitLODSampling(llvm::Module &module) {
  unsigned relaxed=0;
  for(auto &function:module) {
    if(!function.isDeclaration() || !function.isConvergent() ||
       !function.getName().startswith("air.sample_"))continue;
    bool allExplicit=!function.use_empty();
    for(auto *user:function.users()) {
      auto *call=llvm::dyn_cast<llvm::CallBase>(user);
      if(!call || call->getCalledFunction()!=&function || call->arg_size()<4) {allExplicit=false;break;}
      auto *mode=llvm::dyn_cast<llvm::ConstantInt>(call->getArgOperand(call->arg_size()-4));
      if(!mode || !mode->getType()->isIntegerTy(1) || !mode->isOne()) {allExplicit=false;break;}
    }
    if(!allExplicit)continue;
    function.removeFnAttr(llvm::Attribute::Convergent);
    for(auto *user:function.users()) {
      llvm::cast<llvm::CallBase>(user)->removeFnAttr(llvm::Attribute::Convergent);
      ++relaxed;
    }
  }
  return relaxed;
}
} // namespace dxmt::shaderopt
