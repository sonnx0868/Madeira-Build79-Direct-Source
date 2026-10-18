// Offline shader compiler experiment. This tool is not linked into the app.
#include "metallib_writer.hpp"
#include "explicit_lod_guard.hpp"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>
#include <cstdlib>
#include <functional>
#include <vector>

using namespace llvm;
// Preserve the SRV's per-component boundary checks while moving a captured,
// read-only resource into constant memory. This is an offline experiment.
static unsigned embedSRV(Module &m,bool captureValues) {
  const char *text=std::getenv("MADEIRA_SHADER_LAB_EMBEDDED_SRV");
  if(!text)return 0;
  auto document=json::parse(text);if(!document){consumeError(document.takeError());return 0;}
  auto *config=document->getAsObject();if(!config)return 0;
  auto name=config->getString("function");
  auto argument=config->getInteger("table_argument");
  auto field=config->getInteger("field_byte_offset");auto *words=config->getArray("words");
  if(!name || !argument || !field || !words || words->empty() || words->size()>4096)return 0;
  auto *f=m.getFunction(*name);if(!f || *argument<0 || uint64_t(*argument)>=f->arg_size())return 0;
  std::vector<uint32_t> values;
  for(auto &entry:*words) {
    auto word=entry.getAsInteger();if(!word || *word<0 || uint64_t(*word)>UINT32_MAX)return 0;
    values.push_back(*word);
  }
  SmallVector<LoadInst *,8> resources;
  SmallVector<LoadInst *,16> lengths;
  for(auto &block:*f)for(auto &inst:block)if(auto *load=dyn_cast<LoadInst>(&inst)) {
    int64_t offset=0;auto *base=GetPointerBaseWithConstantOffset(load->getPointerOperand(),offset,m.getDataLayout());
    if(base!=f->getArg(*argument))continue;
    if(offset==*field && load->getType()->isPointerTy() && load->getType()->getPointerAddressSpace()==1)resources.push_back(load);
    if(offset==*field+8 && load->getType()->isIntegerTy(64))lengths.push_back(load);
  }
  if(resources.empty())return 0;
  auto *data=ConstantDataArray::get(m.getContext(),values);
  GlobalVariable *global=nullptr;
  if(captureValues) {
    global=new GlobalVariable(m,data->getType(),true,GlobalValue::InternalLinkage,data,"captured_srv",nullptr,
                              GlobalVariable::NotThreadLocal,2);
    global->setAlignment(Align(16));
  }
  auto *zeroData=ConstantAggregateZero::get(ArrayType::get(Type::getInt32Ty(m.getContext()),4));
  auto *zeroGlobal=new GlobalVariable(m,zeroData->getType(),true,GlobalValue::InternalLinkage,zeroData,"captured_srv_zero",nullptr,
                                      GlobalVariable::NotThreadLocal,2);
  zeroGlobal->setAlignment(Align(16));
  std::function<bool(Value *,unsigned)> depends=[&](Value *v,unsigned depth) {
    if(depth>16)return false;if(std::find(resources.begin(),resources.end(),v)!=resources.end())return true;
    if(auto *gep=dyn_cast<GetElementPtrInst>(v))return depends(gep->getPointerOperand(),depth+1);
    if(auto *cast=dyn_cast<BitCastInst>(v))return depends(cast->getOperand(0),depth+1);
    if(auto *select=dyn_cast<SelectInst>(v))return depends(select->getTrueValue(),depth+1)||depends(select->getFalseValue(),depth+1);
    return false;
  };
  SmallVector<LoadInst *,64> candidates;
  for(auto &block:*f)for(auto &inst:block)if(auto *load=dyn_cast<LoadInst>(&inst))
    if(!load->isVolatile() && !load->isAtomic() && depends(load->getPointerOperand(),0))candidates.push_back(load);
  for(auto *load:candidates) {
    IRBuilder<> ir(load);
    auto *base=captureValues?ir.CreateGEP(data->getType(),global,{ir.getInt32(0),ir.getInt32(0)}):nullptr;
    auto *zero=ir.CreateGEP(zeroData->getType(),zeroGlobal,{ir.getInt32(0),ir.getInt32(0)});
    auto constantType=[&](Value *old)->PointerType * {
      auto *p=cast<PointerType>(old->getType());
      return p->isOpaque()?PointerType::get(m.getContext(),2):PointerType::get(p->getNonOpaquePointerElementType(),2);
    };
    std::function<Value *(Value *,unsigned)> rebase=[&](Value *v,unsigned depth)->Value * {
      if(depth>16)return nullptr;
      if(std::find(resources.begin(),resources.end(),v)!=resources.end()) {
        if(captureValues)return ir.CreateBitCast(base,constantType(v));
        auto *original=cast<LoadInst>(v);
        auto *slotType=PointerType::get(constantType(v),original->getPointerAddressSpace());
        auto *slot=ir.CreateBitCast(original->getPointerOperand(),slotType);
        return ir.CreateAlignedLoad(constantType(v),slot,original->getAlign());
      }
      if(isa<ConstantPointerNull>(v))return ir.CreateBitCast(zero,constantType(v));
      if(auto *gep=dyn_cast<GetElementPtrInst>(v)) {
        auto *p=rebase(gep->getPointerOperand(),depth+1);if(!p)return nullptr;
        SmallVector<Value *,4> indices(gep->idx_begin(),gep->idx_end());
        return ir.CreateGEP(gep->getSourceElementType(),p,indices);
      }
      if(auto *cast=dyn_cast<BitCastInst>(v)) {
        auto *p=rebase(cast->getOperand(0),depth+1);return p?ir.CreateBitCast(p,constantType(v)):nullptr;
      }
      if(auto *select=dyn_cast<SelectInst>(v)) {
        auto *a=rebase(select->getTrueValue(),depth+1),*b=rebase(select->getFalseValue(),depth+1);
        return a&&b?ir.CreateSelect(select->getCondition(),a,b):nullptr;
      }
      return nullptr;
    };
    if(auto *p=rebase(load->getPointerOperand(),0)) {
      auto *value=ir.CreateAlignedLoad(load->getType(),p,load->getAlign());
      load->replaceAllUsesWith(value);
    }
  }
  if(captureValues)for(auto *length:lengths)length->replaceAllUsesWith(ConstantInt::get(length->getType(),values.size()*4));
  // Old loads have no users now. Leave any unsupported chain on the original
  // resource, and let DCE remove only unused loads and pointer calculations.
  unsigned rewritten=0;
  for(auto *load:candidates)if(load->use_empty()){load->eraseFromParent();++rewritten;}
  return rewritten;
}
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
  unsigned relaxed=mode=="guarded"?dxmt::air::relaxExplicitLODSampling(*m):0;
  unsigned folded=(mode=="specialize" || mode=="embed")?specialize(*m):0;
  unsigned embedded=(mode=="embed" || mode=="constant")?embedSRV(*m,mode=="embed"):0;
  if(mode!="baseline") {
    LoopAnalysisManager lam; FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
    PipelineTuningOptions tuning;
    tuning.LoopVectorization=false; tuning.SLPVectorization=false;
    tuning.LoopUnrolling=mode=="embed"; tuning.LoopInterleaving=false;
    PassBuilder pb(nullptr,tuning);
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam,fam,cgam,mam);
    ModulePassManager pm;
    if(mode=="guarded") {
      if(auto err=pb.parsePassPipeline(pm,"function(loop-simplify,lcssa,loop(loop-rotate),sink)")) {
        consumeError(std::move(err));return 7;
      }
    } else if(mode=="loops") {
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
    } else if(mode=="o2" || mode=="specialize" || mode=="embed" || mode=="constant") pm=pb.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
    else return 8;
    pm.run(*m,mam);
    if(mode=="embed") {
      for(auto &function:*m) {
        if(function.isDeclaration())continue;
        auto &loops=fam.getResult<LoopAnalysis>(function);
        auto &evolution=fam.getResult<ScalarEvolutionAnalysis>(function);
        for(auto *loop:loops.getLoopsInPreorder()) {
          auto trips=evolution.getSmallConstantTripCount(loop);
          if(!trips || trips>4)continue;
          Metadata *hint[]={MDString::get(context,"llvm.loop.unroll.full")};
          Metadata *members[]={nullptr,MDNode::get(context,hint)};
          auto *id=MDNode::getDistinct(context,members);id->replaceOperandWith(0,id);loop->setLoopID(id);
        }
      }
      FunctionPassManager finish;
      finish.addPass(LoopUnrollPass(LoopUnrollOptions(2,true).setPartial(false).setRuntime(false).setFullUnrollMaxCount(4)));
      finish.addPass(InstCombinePass());
      finish.addPass(SimplifyCFGPass());
      ModulePassManager cleanup;cleanup.addPass(createModuleToFunctionPassAdaptor(std::move(finish)));cleanup.run(*m,mam);
    }
  }
  if(verifyModule(*m,&errs())) return 9;
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  std::error_code error;
  raw_fd_ostream output(argv[3],error); if(error) return 10;
  dxmt::metallib::MetallibWriter().Write(*m,output);
  outs()<<formatv("{0}\n",json::Value(json::Object{{"mode",mode.str()},{"before",std::move(before)},
    {"after",stats(*m)},{"optimizer_ms",ms},{"folded_loads",int64_t(folded)},{"embedded_loads",int64_t(embedded)},
    {"relaxed_explicit_samples",int64_t(relaxed)}}));
  return 0;
}
