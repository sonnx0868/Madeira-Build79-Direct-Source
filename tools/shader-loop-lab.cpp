// Offline shader compiler experiment. This tool is not linked into the app.
#include "metallib_writer.hpp"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>
#include <cstdlib>

using namespace llvm;
// Offline partial evaluation only. Values come from a saved dispatch; these
// constants are never used as a game-wide or device-wide runtime override.
static unsigned specialize(Module &m) {
  const char *text=std::getenv("MADEIRA_SHADER_LAB_CONSTANTS");
  if(!text)return 0;
  auto document=json::parse(text);
  if(!document){consumeError(document.takeError());return 0;}
  auto *config=document->getAsObject();if(!config)return 0;
  auto name=config->getString("function");
  auto argument=config->getInteger("table_argument");
  auto *values=config->getArray("values");
  if(!name || !argument || !values || values->size()>32)return 0;
  auto *f=m.getFunction(*name);if(!f || *argument<0 || uint64_t(*argument)>=f->arg_size())return 0;
  SmallVector<Instruction *,16> erased;
  for(auto &block:*f) for(auto &inst:block) {
    auto *load=dyn_cast<LoadInst>(&inst);
    if(!load || !load->getType()->isIntegerTy(32) || load->isVolatile() || load->isAtomic())continue;
    int64_t offset=0,tableOffset=0;
    auto *base=GetPointerBaseWithConstantOffset(load->getPointerOperand(),offset,m.getDataLayout());
    auto *cb=dyn_cast<LoadInst>(base);
    if(!cb || !cb->getType()->isPointerTy() || cb->getType()->getPointerAddressSpace()!=2)continue;
    auto *table=GetPointerBaseWithConstantOffset(cb->getPointerOperand(),tableOffset,m.getDataLayout());
    if(table!=f->getArg(*argument) || tableOffset<0 || tableOffset%8)continue;
    for(const auto &entry:*values) {
      auto *value=entry.getAsObject();if(!value)continue;
      auto slot=value->getInteger("slot"),byteOffset=value->getInteger("byte_offset"),word=value->getInteger("value");
      if(!slot || !byteOffset || !word || *word<0 || uint64_t(*word)>UINT32_MAX)continue;
      if(tableOffset/8==*slot && offset==*byteOffset) {
        load->replaceAllUsesWith(ConstantInt::get(load->getType(),*word));erased.push_back(load);break;
      }
    }
  }
  for(auto *instruction:erased)instruction->eraseFromParent();
  return erased.size();
}
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
  unsigned folded=mode=="specialize"?specialize(*m):0;
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
    } else if(mode=="o2" || mode=="specialize") pm=pb.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
    else return 8;
    pm.run(*m,mam);
  }
  if(verifyModule(*m,&errs())) return 9;
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  std::error_code error;
  raw_fd_ostream output(argv[3],error); if(error) return 10;
  dxmt::metallib::MetallibWriter().Write(*m,output);
  outs()<<formatv("{0}\n",json::Value(json::Object{{"mode",mode.str()},{"before",std::move(before)},
    {"after",stats(*m)},{"optimizer_ms",ms},{"folded_loads",int64_t(folded)}}));
  return 0;
}
