// Offline shader compiler experiment. This tool is not linked into the app.
#include "metallib_writer.hpp"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>

using namespace llvm;
static json::Object stats(Module &m) {
  int64_t instructions=0, loads=0, stores=0, calls=0;
  unsigned totalThreads=0;
  std::string name;
  if(auto *kernels=m.getNamedMetadata("air.kernel"))
    if(kernels->getNumOperands())
      if(auto *function=mdconst::dyn_extract<Function>(kernels->getOperand(0)->getOperand(0))) {
        name=function->getName().str();
        auto attr=function->getFnAttribute("max-work-group-size");
        if(attr.isStringAttribute())attr.getValueAsString().getAsInteger(10,totalThreads);
      }
  for(auto &f:m) for(auto &b:f) for(auto &i:b) {
    ++instructions;
    loads+=isa<LoadInst>(i); stores+=isa<StoreInst>(i); calls+=isa<CallInst>(i);
  }
  return json::Object{{"instructions",instructions},{"loads",loads},{"stores",stores},{"calls",calls},{"function",name},{"total_threads",int64_t(totalThreads)}};
}
int main(int argc,char **argv) {
  if(argc!=4) return 2;
  auto input=MemoryBuffer::getFile(argv[1]); if(!input) return 3;
  auto bytes=(*input)->getBuffer();
  const char magic[]={'B','C',char(0xc0),char(0xde)};
  auto offset=bytes.find(StringRef(magic,4)); if(offset==StringRef::npos) return 4;
  LLVMContext context;
  auto parsed=parseBitcodeFile(MemoryBufferRef(bytes.substr(offset),"saved-air"),context);
  if(!parsed) { consumeError(parsed.takeError());return 5; }
  auto m=std::move(*parsed);
  if(!m->getNamedMetadata("air.kernel")) return 6;
  auto before=stats(*m);
  const auto started=std::chrono::steady_clock::now();
  StringRef mode(argv[2]);
  if(mode!="baseline") {
    LoopAnalysisManager lam; FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
    PipelineTuningOptions tuning;
    tuning.LoopVectorization=false; tuning.SLPVectorization=false;
    tuning.LoopUnrolling=false; tuning.LoopInterleaving=false;
    PassBuilder pb(nullptr,tuning);
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam,fam,cgam,mam);
    ModulePassManager pm;
    if(mode=="loops") {
      if(auto err=pb.parsePassPipeline(pm,"function(loop-simplify,lcssa,loop-mssa(licm),gvn,instcombine,simplifycfg,adce)")) {
        consumeError(std::move(err));return 7;
      }
    } else if(mode=="cfg") {
      FunctionPassManager fpm;
      const auto options=SimplifyCFGOptions().convertSwitchRangeToICmp(true).hoistCommonInsts(true).sinkCommonInsts(true);
      fpm.addPass(SimplifyCFGPass(options));
      fpm.addPass(InstCombinePass());
      fpm.addPass(SimplifyCFGPass(options));
      pm.addPass(createModuleToFunctionPassAdaptor(std::move(fpm)));
    } else if(mode=="o2") pm=pb.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
    else return 8;
    pm.run(*m,mam);
  }
  if(verifyModule(*m,&errs())) return 9;
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  std::error_code error;
  raw_fd_ostream output(argv[3],error); if(error) return 10;
  dxmt::metallib::MetallibWriter().Write(*m,output);
  outs()<<formatv("{0}\n",json::Value(json::Object{{"mode",mode.str()},{"before",std::move(before)},
    {"after",stats(*m)},{"optimizer_ms",ms}}));
  return 0;
}
