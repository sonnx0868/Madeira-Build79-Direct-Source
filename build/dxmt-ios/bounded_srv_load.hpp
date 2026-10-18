// SPDX-License-Identifier: MIT
// A raw/structured SRV vector read. Keep the scalar boundary path because
// D3D11 requires zero per out-of-range component, not zero for the whole read.
#pragma once
#include "llvm/IR/IRBuilder.h"

namespace dxmt::dxbc {
inline llvm::Value *loadBoundedSRV4(llvm::IRBuilderBase &ir, llvm::Value *pointer,
                                   llvm::Value *index, llvm::Value *byteLength) {
  auto *i32 = ir.getInt32Ty();
  auto *v4 = llvm::FixedVectorType::get(i32, 4);
  unsigned addressSpace = llvm::cast<llvm::PointerType>(pointer->getType())->getAddressSpace();
  assert(addressSpace == 1 && "Only non-coherent, read-only device SRVs are eligible");
  auto *words = ir.CreateLShr(byteLength, 2);
  // Subtract only under an explicit size guard. Testing index+3 alone would
  // wrap for UINT_MAX and could take an out-of-bounds vector fast path.
  auto *fits = ir.CreateAnd(ir.CreateICmpUGE(words, ir.getInt32(4)),
                            ir.CreateICmpULE(index, ir.CreateSub(words, ir.getInt32(4))));
  auto *function = ir.GetInsertBlock()->getParent();
  auto *fast = llvm::BasicBlock::Create(ir.getContext(), "srv4.full", function);
  auto *edge = llvm::BasicBlock::Create(ir.getContext(), "srv4.edge", function);
  auto *merge = llvm::BasicBlock::Create(ir.getContext(), "srv4.merge", function);
  ir.CreateCondBr(fits, fast, edge);

  ir.SetInsertPoint(fast);
  auto *base = ir.CreateGEP(i32, pointer, index);
  auto *vectorPointer = ir.CreateBitCast(base, llvm::PointerType::get(v4, addressSpace));
  // Structured buffers may have a 60-byte stride. Never invent 16-byte
  // alignment merely because the load has four components.
  auto *full = ir.CreateAlignedLoad(v4, vectorPointer, llvm::Align(4));
  ir.CreateBr(merge);

  ir.SetInsertPoint(edge);
  llvm::Value *partial = llvm::PoisonValue::get(v4);
  for (unsigned component = 0; component < 4; ++component) {
    auto *offset = ir.CreateAdd(index, ir.getInt32(component));
    auto *address = ir.CreateGEP(i32, pointer, offset);
    auto *bounded = ir.CreateSelect(ir.CreateICmpULT(offset, words), address,
                                    llvm::Constant::getNullValue(address->getType()));
    auto *value = ir.CreateAlignedLoad(i32, bounded, llvm::Align(4));
    partial = ir.CreateInsertElement(partial, value, uint64_t(component));
  }
  ir.CreateBr(merge);

  ir.SetInsertPoint(merge);
  auto *result = ir.CreatePHI(v4, 2);
  result->addIncoming(full, fast);
  result->addIncoming(partial, edge);
  return result;
}
} // namespace dxmt::dxbc
