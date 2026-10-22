// Optional offline test: supply a private AIR fixture and compile with the
// isolated dxmt-multikernel-metallib.patch overlay. This adapter is not
// registered in the shipping optimizer; see the saved performance gate.
#include "adaptive_lighting.hpp"
#include "metallib_writer.hpp"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
using namespace llvm;
int main(int argc,char**argv){
 if(argc!=3)return 2;LLVMContext context;auto data=MemoryBuffer::getFile(argv[1]);if(!data)return 3;
 auto parsed=parseBitcodeFile((*data)->getMemBufferRef(),context);if(!parsed){errs()<<toString(parsed.takeError());return 4;}
 auto module=std::move(*parsed);auto*f=module->getFunction("shader_f4c2c563");if(!f)return 5;
 f->setName("shader_other");if(dxmt::shaderopt::adaptive_lighting::run(*module))return 6;f->setName("shader_f4c2c563");
 if(!dxmt::shaderopt::adaptive_lighting::run(*module))return 7;
 if(verifyModule(*module,&errs()))return 8;
 if(module->getNamedMetadata("air.kernel")->getNumOperands()!=4)return 9;
 if(dxmt::shaderopt::adaptive_lighting::run(*module))return 10;
 std::error_code error;raw_fd_ostream out(argv[2],error,sys::fs::OF_None);if(error)return 11;
 dxmt::metallib::MetallibWriter().Write(*module,out);
 outs()<<"PASS: native adaptive entry generation, LLVM verifier, scope, idempotence, original ABI and four Metal exports\n";return 0;
}
