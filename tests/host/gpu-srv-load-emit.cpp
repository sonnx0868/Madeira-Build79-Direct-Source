// Offline test: emit actual production LLVM helper through the DXMT writer.
#include "bounded_srv_load.hpp"
#include "metallib_writer.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
int main(int argc, char **argv) {
  if (argc != 3) return 2;
  bool optimized = std::string(argv[1]) == "vector";
  LLVMContext context;
  Module module("srv-load-test", context);
  module.setTargetTriple("air64-apple-macosx14.0.0");
  module.setDataLayout("e-p:64:64:64-i64:64-v64:64-v128:128-n8:16:32");
  IRBuilder<> ir(context);
  auto *i32 = ir.getInt32Ty();
  auto *v3 = FixedVectorType::get(i32, 3);
  auto *v4 = FixedVectorType::get(i32, 4);
  auto *p32 = PointerType::get(i32, 1);
  auto *p4 = PointerType::get(v4, 1);
  auto *function = Function::Create(FunctionType::get(ir.getVoidTy(), {v3,p32,p32,p4}, false),
                                    GlobalValue::ExternalLinkage, "test_srv4", module);
  function->addFnAttr("max-work-group-size", "64");
  ir.SetInsertPoint(BasicBlock::Create(context, "entry", function));
  auto *id = ir.CreateExtractElement(function->getArg(0), ir.getInt32(0));
  auto *pair = ir.CreateMul(id, ir.getInt32(2));
  auto *index = ir.CreateLoad(i32, ir.CreateGEP(i32, function->getArg(2), pair));
  auto *length = ir.CreateLoad(i32, ir.CreateGEP(i32, function->getArg(2), ir.CreateAdd(pair, ir.getInt32(1))));
  Value *result = nullptr;
  if (optimized) result = dxmt::dxbc::loadBoundedSRV4(ir, function->getArg(1), index, length);
  else {
    result = PoisonValue::get(v4);
    auto *words = ir.CreateLShr(length, 2);
    for (unsigned component = 0; component < 4; ++component) {
      auto *offset = ir.CreateAdd(index, ir.getInt32(component));
      auto *address = ir.CreateGEP(i32, function->getArg(1), offset);
      auto *bounded = ir.CreateSelect(ir.CreateICmpULT(offset, words), address,
                                      Constant::getNullValue(address->getType()));
      result = ir.CreateInsertElement(result, ir.CreateAlignedLoad(i32, bounded, Align(4)), uint64_t(component));
    }
  }
  ir.CreateAlignedStore(result, ir.CreateGEP(v4, function->getArg(3), id), Align(4));
  ir.CreateRetVoid();
  auto s = [&](const char *value) -> Metadata * { return MDString::get(context, value); };
  auto n = [&](unsigned value) -> Metadata * { return ConstantAsMetadata::get(ir.getInt32(value)); };
  auto tuple = [&](std::initializer_list<Metadata *> values) { return MDNode::get(context, ArrayRef<Metadata *>(values)); };
  auto buffer = [&](unsigned argument, unsigned location, bool write, bool vec) -> MDNode * {
    return tuple({n(argument),s("air.buffer"),s("air.location_index"),n(location),n(1),
      s(write?"air.write":"air.read"),s("air.address_space"),n(1),s("air.arg_type_size"),n(vec?16:4),
      s("air.arg_type_align_size"),n(4),s("air.arg_type_name"),s(vec?"uint4":"uint"),s("air.arg_name"),s("data")});
  };
  module.getOrInsertNamedMetadata("air.version")->addOperand(tuple({n(2),n(6),n(0)}));
  module.getOrInsertNamedMetadata("air.language_version")->addOperand(tuple({s("Metal"),n(3),n(1),n(0)}));
  auto *inputs = tuple({tuple({n(0),s("air.thread_position_in_grid"),s("air.arg_type_name"),s("uint3"),s("air.arg_name"),s("gid")}),
                       buffer(1,0,false,false),buffer(2,1,false,false),buffer(3,2,true,true)});
  module.getOrInsertNamedMetadata("air.kernel")->addOperand(tuple({ConstantAsMetadata::get(function),tuple({}),inputs,
    tuple({s("air.max_work_group_size"),n(64)})}));
  if (verifyModule(module, &errs())) return 3;
  std::error_code ec;
  raw_fd_ostream irFile(std::string(argv[2])+".ll",ec); if(ec) return 4;
  module.print(irFile,nullptr);
  raw_fd_ostream library(std::string(argv[2])+".metallib",ec); if(ec) return 5;
  dxmt::metallib::MetallibWriter().Write(module,library);
  return 0;
}
