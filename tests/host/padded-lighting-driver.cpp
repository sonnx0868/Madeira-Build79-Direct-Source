// Private saved-AIR adapter: tests the production transformations and writer.
#include "lighting_phase_outline.hpp"
#include "padded_lighting.hpp"
#include "metallib_writer.hpp"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
static std::string body(Function &function) {
  std::string text; raw_string_ostream output(text); function.print(output); return output.str();
}
static bool write(Module &module, StringRef folder, StringRef name) {
  std::error_code error;
  raw_fd_ostream output((folder + "/" + name + ".metallib").str(), error, sys::fs::OF_None);
  if (error) return false;
  dxmt::metallib::MetallibWriter().Write(module, output); return true;
}
int main(int argc, char **argv) {
  if (argc != 3) return 2;
  LLVMContext context;
  auto data = MemoryBuffer::getFile(argv[1]); if (!data) return 3;
  auto parsed = parseBitcodeFile((*data)->getMemBufferRef(), context);
  if (!parsed) { errs() << toString(parsed.takeError()); return 4; }
  auto module = std::move(*parsed);
  if (dxmt::shaderopt::padded_lighting::run(*module)) return 5;
  ModuleAnalysisManager analyses;
  if (dxmt::shaderopt::ComputeLightingPhasePass().run(*module, analyses).areAllPreserved()) return 6;
  auto *original = module->getFunction("shader_f4c2c563");
  auto *phase = module->getFunction("madeira_lighting_phase");
  if (!original || !phase || verifyModule(*module, &errs())) return 7;
  if (!write(*module, argv[2], "cpp-phase")) return 8;
  auto cleaned = CloneModule(*module);
  dxmt::shaderopt::padded_lighting::simplify(*cleaned->getFunction("madeira_lighting_phase"));
  if (verifyModule(*cleaned, &errs()) || !write(*cleaned, argv[2], "cpp-cleaned")) return 9;
  original->setName("unrelated_shader");
  if (dxmt::shaderopt::padded_lighting::run(*module)) return 10;
  original->setName("shader_f4c2c563");
  std::string before = body(*original), helperBefore = body(*phase);
  if (!dxmt::shaderopt::padded_lighting::run(*module) || verifyModule(*module, &errs())) return 11;
  if (body(*original) != before || body(*phase) != helperBefore) return 12;
  if (module->getNamedMetadata("air.kernel")->getNumOperands() != 2 ||
      !module->getFunction(dxmt::shaderopt::padded_lighting::Entry)) return 13;
  if (dxmt::shaderopt::padded_lighting::run(*module)) return 14;
  if (!write(*module, argv[2], "native-padded")) return 15;
  if (!write(*module, argv[2], "native-original")) return 16;
  outs() << "PASS: exact guarded phase input, optional padded entry, original entry/helper unchanged, LLVM verification, two exports, scope and idempotence\n";
  return 0;
}
