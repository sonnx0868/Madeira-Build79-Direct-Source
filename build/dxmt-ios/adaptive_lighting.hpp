#pragma once
// Experimental, narrow shader adapter. This builds native entry points from
// the game's translated function; no game shader or asset is embedded here.
// The original full-resolution entry remains the reference/fallback path.
#include "lighting_phase_outline.hpp"

namespace dxmt::shaderopt::adaptive_lighting {
using namespace llvm;
inline constexpr unsigned Cache = 0, Mask = 4147200, List = 6220800;
// The compact list includes padding per 16x16 tile, including partial tiles.
inline constexpr unsigned Tiles = 8309760, Header = 8317920, Words = 8317952;
inline constexpr unsigned Width = 1920, Height = 1080, CoarseWidth = 960, TileWidth = 120;

struct Builder {
  Module &module;
  LLVMContext &context;
  Function &original;
  StructType *resources, *constants, *lighting;
  Type *i32, *f32, *v3, *device;
  Function *reference = nullptr, *phase = nullptr, *coarsePhase = nullptr, *material = nullptr;
  SmallVector<Function *, 4> exports;

  Builder(Module &m, Function &f) : module(m), context(m.getContext()), original(f) {
    i32 = Type::getInt32Ty(context); f32 = Type::getFloatTy(context);
    v3 = FixedVectorType::get(i32, 3); device = PointerType::get(i32, 1);
    resources = dyn_cast<StructType>(f.getArg(2)->getType()->getPointerElementType());
    constants = dyn_cast<StructType>(f.getArg(3)->getType()->getPointerElementType());
    lighting = nullptr;
  }
  Constant *n(unsigned value) { return ConstantInt::get(i32, value); }
  Value *get(IRBuilder<> &b, Value *p, Value *index) {
    return b.CreateLoad(i32, b.CreateGEP(i32, p, index));
  }
  Value *get(IRBuilder<> &b, Value *p, unsigned index) { return get(b, p, n(index)); }
  void put(IRBuilder<> &b, Value *p, Value *index, Value *value) {
    b.CreateStore(value, b.CreateGEP(i32, p, index));
  }
  Value *field(IRBuilder<> &b, StructType *type, Value *p, unsigned index) {
    return b.CreateLoad(type->getElementType(index), b.CreateStructGEP(type, p, index));
  }
  Value *vec(IRBuilder<> &b, Value *x, Value *y, Value *z) {
    Value *v = PoisonValue::get(v3);
    v = b.CreateInsertElement(v, x, n(0)); v = b.CreateInsertElement(v, y, n(1));
    return b.CreateInsertElement(v, z, n(2));
  }
  // prepare_lighting has checked the dispatch bounds and CB/table layout.
  Value *tile(IRBuilder<> &b, Value *group, Value *root, Value *cb) {
    auto *buffer = field(b, constants, cb, 3);
    auto *words = b.CreateBitCast(buffer, PointerType::get(i32, 2));
    auto *offset = get(b, words, 0u);
    auto *list = field(b, resources, root, 71);
    return get(b, list, b.CreateAdd(b.CreateLShr(group, n(2)), offset));
  }
  struct Pixel { Value *x, *y, *layer, *tileIndex; };
  Pixel pixel(IRBuilder<> &b, Value *g, Value *local, Value *root, Value *cb) {
    auto *gx = b.CreateExtractElement(g, n(0));
    auto *packed = tile(b, gx, root, cb);
    auto *tx = b.CreateAnd(packed, n(32767));
    auto *ty = b.CreateAnd(b.CreateLShr(packed, n(15)), n(32767));
    auto *x = b.CreateAdd(b.CreateShl(tx, n(4)), b.CreateAdd(
        b.CreateShl(b.CreateAnd(gx, n(1)), n(3)), b.CreateExtractElement(local, n(0))));
    auto *y = b.CreateAdd(b.CreateShl(ty, n(4)), b.CreateAdd(
        b.CreateShl(b.CreateAnd(b.CreateLShr(gx, n(1)), n(1)), n(3)), b.CreateExtractElement(local, n(1))));
    return {x, y, b.CreateLShr(packed, n(30)), b.CreateAdd(b.CreateMul(ty, n(TileWidth)), tx)};
  }
  Value *inside(IRBuilder<> &b, Pixel p, Value *scratch) {
    return b.CreateAnd(b.CreateICmpULT(p.x, get(b, scratch, Header + 4)),
                       b.CreateICmpULT(p.y, get(b, scratch, Header + 5)));
  }
  Value *cacheIndex(IRBuilder<> &b, Pixel p) {
    return b.CreateMul(b.CreateAdd(b.CreateMul(b.CreateLShr(p.y, n(1)), n(CoarseWidth)),
                                  b.CreateLShr(p.x, n(1))), n(8));
  }
  Function *kernel(StringRef name) {
    SmallVector<Type *, 5> parameters;
    for (auto &arg : original.args()) parameters.push_back(arg.getType());
    parameters.push_back(device);
    auto *f = Function::Create(FunctionType::get(Type::getVoidTy(context), parameters, false),
                               GlobalValue::ExternalLinkage, name, module);
    // Do not copy readonly/readnone from the reference: these kernels write
    // scratch storage as well as the original output texture.
    exports.push_back(f);
    return f;
  }
  void callReference(IRBuilder<> &b, Function *f) {
    b.CreateCall(reference, {f->getArg(0), f->getArg(1), f->getArg(2), f->getArg(3)});
  }
  void coarse() {
    auto *f = kernel("madeira_lighting_coarse_v1");
    auto *entry = BasicBlock::Create(context, "entry", f);
    auto *body = BasicBlock::Create(context, "body", f);
    auto *shade = BasicBlock::Create(context, "shade", f);
    auto *mark = BasicBlock::Create(context, "mark", f);
    auto *ret = BasicBlock::Create(context, "return", f);
    IRBuilder<> b(entry); auto *scratch = f->getArg(4);
    b.CreateCondBr(b.CreateICmpNE(get(b, scratch, Header + 3), n(0)), body, ret);
    b.SetInsertPoint(body);
    auto *lx = b.CreateExtractElement(f->getArg(1), n(0));
    auto *ly = b.CreateExtractElement(f->getArg(1), n(1));
    auto *gx = b.CreateExtractElement(f->getArg(0), n(0));
    auto *quadrant = b.CreateOr(b.CreateLShr(lx, n(2)), b.CreateShl(b.CreateLShr(ly, n(2)), n(1)));
    auto *g = vec(b, b.CreateOr(b.CreateShl(gx, n(2)), quadrant), n(0), n(0));
    auto *local = vec(b, b.CreateShl(b.CreateAnd(lx, n(3)), n(1)),
                         b.CreateShl(b.CreateAnd(ly, n(3)), n(1)), n(0));
    Pixel p = pixel(b, g, local, f->getArg(2), f->getArg(3));
    auto *mono = b.CreateICmpEQ(p.layer, n(0));
    b.CreateCondBr(b.CreateAnd(mono, inside(b, p, scratch)), shade, ret);
    b.SetInsertPoint(shade);
    auto *values = b.CreateCall(coarsePhase, {g, local, f->getArg(2), f->getArg(3)});
    auto *base = cacheIndex(b, p);
    for (unsigned i = 0; i != 6; ++i)
      put(b, scratch, b.CreateAdd(base, n(i)), b.CreateExtractValue(values, i));
    put(b, scratch, b.CreateAdd(base, n(6)), b.CreateExtractValue(values, 6));
    b.CreateCondBr(b.CreateICmpEQ(b.CreateOr(lx, ly), n(0)), mark, ret);
    b.SetInsertPoint(mark);
    put(b, scratch, b.CreateAdd(n(Tiles), p.tileIndex), b.CreateAdd(gx, n(1)));
    b.CreateBr(ret);
    b.SetInsertPoint(ret); b.CreateRetVoid();
  }
  Value *sample(IRBuilder<> &b, Value *scratch, Value *base) {
    Value *value = Constant::getNullValue(lighting);
    for (unsigned i = 0; i != 6; ++i)
      value = b.CreateInsertValue(value, get(b, scratch, b.CreateAdd(base, n(i))), i);
    return value;
  }
  void compose() {
    auto *f = kernel("madeira_lighting_compose_v1");
    auto *entry = BasicBlock::Create(context, "entry", f);
    auto *body = BasicBlock::Create(context, "body", f);
    auto *check = BasicBlock::Create(context, "check", f);
    auto *load = BasicBlock::Create(context, "load", f);
    auto *even = BasicBlock::Create(context, "exact", f);
    auto *interp = BasicBlock::Create(context, "interpolate", f);
    auto *shade = BasicBlock::Create(context, "shade", f);
    auto *full = BasicBlock::Create(context, "reference", f);
    auto *ret = BasicBlock::Create(context, "return", f);
    IRBuilder<> b(entry); auto *scratch = f->getArg(4);
    b.CreateCondBr(b.CreateICmpNE(get(b, scratch, Header + 3), n(0)), body, full);
    b.SetInsertPoint(body); Pixel p = pixel(b, f->getArg(0), f->getArg(1), f->getArg(2), f->getArg(3));
    b.CreateCondBr(b.CreateICmpEQ(p.layer, n(0)), check, full);
    b.SetInsertPoint(check);
    // Out-of-screen pixels must not index fixed-size mask storage.
    auto *mask = BasicBlock::Create(context, "mask", f);
    b.CreateCondBr(inside(b, p, scratch), mask, ret);
    b.SetInsertPoint(mask);
    auto *maskIndex = b.CreateAdd(n(Mask), b.CreateAdd(b.CreateMul(p.y, n(Width)), p.x));
    b.CreateCondBr(b.CreateICmpEQ(get(b, scratch, maskIndex), n(1)), load, ret);
    b.SetInsertPoint(load); auto *base = cacheIndex(b, p); auto *center = sample(b, scratch, base);
    b.CreateCondBr(b.CreateICmpEQ(b.CreateAnd(b.CreateOr(p.x, p.y), n(1)), n(0)), even, interp);
    b.SetInsertPoint(even); b.CreateBr(shade);
    b.SetInsertPoint(interp);
    // The classifier rejects missing neighbours and sharp/discontinuous data.
    auto *right = sample(b, scratch, b.CreateAdd(base, n(8)));
    auto *down = sample(b, scratch, b.CreateAdd(base, n(CoarseWidth * 8)));
    auto *diagonal = sample(b, scratch, b.CreateAdd(base, n(CoarseWidth * 8 + 8)));
    auto *half = ConstantFP::get(f32, 0.5);
    auto *wx = b.CreateFMul(b.CreateUIToFP(b.CreateAnd(p.x, n(1)), f32), half);
    auto *wy = b.CreateFMul(b.CreateUIToFP(b.CreateAnd(p.y, n(1)), f32), half);
    Value *blended = Constant::getNullValue(lighting);
    for (unsigned i = 0; i != 6; ++i) {
      auto asFloat = [&](Value *v) { return b.CreateBitCast(b.CreateExtractValue(v, i), f32); };
      auto *a = asFloat(center), *c = asFloat(down);
      auto *top = b.CreateFAdd(a, b.CreateFMul(b.CreateFSub(asFloat(right), a), wx));
      auto *bottom = b.CreateFAdd(c, b.CreateFMul(b.CreateFSub(asFloat(diagonal), c), wx));
      auto *value = b.CreateFAdd(top, b.CreateFMul(b.CreateFSub(bottom, top), wy));
      blended = b.CreateInsertValue(blended, b.CreateBitCast(value, i32), i);
    }
    b.CreateBr(shade); b.SetInsertPoint(shade);
    auto *values = b.CreatePHI(lighting, 2); values->addIncoming(center, even); values->addIncoming(blended, interp);
    b.CreateCall(material, {f->getArg(0), f->getArg(1), f->getArg(2), f->getArg(3), values});
    b.CreateBr(ret); b.SetInsertPoint(full); callReference(b, f); b.CreateBr(ret);
    b.SetInsertPoint(ret); b.CreateRetVoid();
  }
  void fallback() {
    auto *f = kernel("madeira_lighting_fallback_v1");
    auto *entry = BasicBlock::Create(context, "entry", f);
    auto *body = BasicBlock::Create(context, "body", f);
    auto *ret = BasicBlock::Create(context, "return", f);
    IRBuilder<> b(entry); auto *scratch = f->getArg(4);
    auto *index = b.CreateAdd(b.CreateMul(b.CreateExtractElement(f->getArg(0), n(0)), n(64)),
                               b.CreateExtractElement(f->getArg(1), n(0)));
    auto *packed = get(b, scratch, b.CreateAdd(n(List), index));
    b.CreateCondBr(b.CreateICmpNE(packed, n(~0u)), body, ret);
    b.SetInsertPoint(body);
    auto *x = b.CreateAnd(packed, n(65535)), *y = b.CreateLShr(packed, n(16));
    auto *tileIndex = b.CreateAdd(b.CreateMul(b.CreateLShr(y, n(4)), n(TileWidth)), b.CreateLShr(x, n(4)));
    auto *inverse = get(b, scratch, b.CreateAdd(n(Tiles), tileIndex));
    auto *quadrant = b.CreateOr(b.CreateAnd(b.CreateLShr(x, n(3)), n(1)),
                                b.CreateShl(b.CreateAnd(b.CreateLShr(y, n(3)), n(1)), n(1)));
    auto *g = vec(b, b.CreateOr(b.CreateShl(b.CreateSub(inverse, n(1)), n(2)), quadrant), n(0), n(0));
    auto *local = vec(b, b.CreateAnd(x, n(7)), b.CreateAnd(y, n(7)), n(0));
    b.CreateCall(reference, {g, local, f->getArg(2), f->getArg(3)});
    b.CreateBr(ret); b.SetInsertPoint(ret); b.CreateRetVoid();
  }
  bool run(MDNode *entry) {
    if (!resources || resources->getNumElements() != 75 || !constants || constants->getNumElements() != 5 ||
        module.getDataLayout().getTypeAllocSize(resources) != 600 ||
        module.getDataLayout().getTypeAllocSize(constants) != 40 || entry->getNumOperands() < 3) return false;
    if (resources->getElementType(71) != device ||
        resources->getElementType(72) != Type::getInt64Ty(context)) return false;
    auto *constantWords = PointerType::get(FixedVectorType::get(i32, 4), 2);
    for (auto *type : constants->elements()) if (type != constantWords) return false;
    auto *argMD = dyn_cast<MDNode>(entry->getOperand(2));
    if (!argMD || argMD->getNumOperands() != 4) return false;
    ValueToValueMapTy map;
    if (!lighting_phase::run(original)) return false;
    // The full-rate fallback also gets the proven phase scheduling; it must
    // not silently return to the more expensive old register-liveness order.
    reference = CloneFunction(&original, map);
    reference->setName("madeira_lighting_reference_v1");
    reference->setLinkage(GlobalValue::InternalLinkage);
    reference->addFnAttr(Attribute::AlwaysInline);
    for (auto &bb : original) for (auto &i : bb)
      if (auto *call = dyn_cast<CallInst>(&i)) if (auto *callee = call->getCalledFunction())
        if (callee->getName().startswith("madeira_lighting_phase")) phase = callee;
    if (!phase) { reference->eraseFromParent(); return false; }
    lighting = cast<StructType>(phase->getReturnType());
    // Record whether the original depth/stencil early-outs permitted this
    // coarse sample. Neighbour validation must never consume stale cache data.
    SmallVector<Type *, 7> coarseFields(7, i32);
    auto *coarseResult = StructType::get(context, coarseFields);
    SmallVector<Type *, 4> phaseParameters;
    for (auto &arg : phase->args()) phaseParameters.push_back(arg.getType());
    coarsePhase = Function::Create(FunctionType::get(coarseResult, phaseParameters, false),
        GlobalValue::InternalLinkage, "madeira_lighting_coarse_phase_v1", module);
    map.clear(); auto coarseTarget = coarsePhase->arg_begin();
    for (auto &arg : phase->args()) map[&arg] = &*coarseTarget++;
    SmallVector<ReturnInst *, 4> coarseReturns;
    CloneFunctionInto(coarsePhase, phase, map, CloneFunctionChangeType::LocalChangesOnly, coarseReturns);
    coarsePhase->setLinkage(GlobalValue::InternalLinkage);
    coarsePhase->addFnAttr(Attribute::AlwaysInline);
    for (auto *ret : coarseReturns) {
      IRBuilder<> b(ret); Value *value = Constant::getNullValue(coarseResult);
      for (unsigned i = 0; i != 6; ++i)
        value = b.CreateInsertValue(value, b.CreateExtractValue(ret->getReturnValue(), i), i);
      value = b.CreateInsertValue(value, n(ret->getParent()->getName() == "end_loop63" ? 1 : 0), 6);
      b.CreateRet(value); ret->eraseFromParent();
    }
    SmallVector<Type *, 5> parameters;
    for (auto &arg : original.args()) parameters.push_back(arg.getType());
    parameters.push_back(lighting);
    material = Function::Create(FunctionType::get(original.getReturnType(), parameters, false),
                                 GlobalValue::InternalLinkage, "madeira_lighting_material_v1", module);
    map.clear(); auto target = material->arg_begin();
    for (auto &arg : original.args()) map[&arg] = &*target++;
    SmallVector<ReturnInst *, 4> returns;
    CloneFunctionInto(material, &original, map, CloneFunctionChangeType::LocalChangesOnly, returns);
    material->setLinkage(GlobalValue::InternalLinkage); material->addFnAttr(Attribute::AlwaysInline);
    SmallVector<CallInst *, 2> replacements;
    for (auto &bb : *material) for (auto &i : bb)
      if (auto *call = dyn_cast<CallInst>(&i)) if (call->getCalledFunction() == phase) replacements.push_back(call);
    for (auto *call : replacements) { call->replaceAllUsesWith(material->getArg(4)); call->eraseFromParent(); }
    coarse(); compose(); fallback();
    bool invalid = verifyFunction(*material) || verifyFunction(*reference) || verifyFunction(*coarsePhase);
    for (auto *f : exports) invalid |= verifyFunction(*f);
    if (invalid) {
      for (auto *f : exports) f->dropAllReferences();
      for (auto *f : exports) f->eraseFromParent();
      material->eraseFromParent(); reference->eraseFromParent(); coarsePhase->eraseFromParent(); return false;
    }
    auto mdInt = [&](unsigned v) -> Metadata * { return ConstantAsMetadata::get(n(v)); };
    auto mdText = [&](StringRef v) -> Metadata * { return MDString::get(context, v); };
    auto *scratchMD = MDNode::get(context, {mdInt(4), mdText("air.buffer"), mdText("air.location_index"),
      mdInt(28), mdInt(1), mdText("air.read_write"), mdText("air.address_space"), mdInt(1),
      mdText("air.arg_type_size"), mdInt(4), mdText("air.arg_type_align_size"), mdInt(4),
      mdText("air.arg_type_name"), mdText("uint"), mdText("air.arg_name"), mdText("madeira_lighting_scratch")});
    SmallVector<Metadata *, 5> args;
    for (auto &operand : argMD->operands()) args.push_back(operand.get());
    args.push_back(scratchMD); auto *arguments = MDNode::get(context, args);
    for (auto *f : exports) {
      SmallVector<Metadata *, 8> description{ValueAsMetadata::get(f), entry->getOperand(1).get(), arguments};
      // Keep workgroup-limit and ABI metadata emitted by this AIR version.
      for (unsigned i = 3; i != entry->getNumOperands(); ++i) description.push_back(entry->getOperand(i).get());
      module.getOrInsertNamedMetadata("air.kernel")->addOperand(MDNode::get(context, description));
    }
    return true;
  }
};

inline bool run(Module &module) {
  auto *f = module.getFunction("shader_f4c2c563");
  if (!f || f->isDeclaration() || f->arg_size() != 4 ||
      module.getFunction("madeira_lighting_coarse_v1")) return false;
  auto *vector = FixedVectorType::get(Type::getInt32Ty(module.getContext()), 3);
  if (!f->getReturnType()->isVoidTy() || f->getArg(0)->getType() != vector ||
      f->getArg(1)->getType() != vector) return false;
  for (unsigned index : {2u, 3u}) {
    auto *pointer = dyn_cast<PointerType>(f->getArg(index)->getType());
    if (!pointer || pointer->isOpaque() || pointer->getAddressSpace() != 2 ||
        !isa<StructType>(pointer->getPointerElementType())) return false;
  }
  auto *entries = module.getNamedMetadata("air.kernel");
  if (!entries) return false;
  for (auto *entry : entries->operands()) if (entry->getNumOperands())
    if (auto *v = dyn_cast_or_null<ValueAsMetadata>(entry->getOperand(0).get()))
      if (v->getValue() == f) return Builder(module, *f).run(entry);
  return false;
}
} // namespace dxmt::shaderopt::adaptive_lighting
