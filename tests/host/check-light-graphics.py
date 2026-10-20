#!/usr/bin/env python3
"""Execute production sampler creation and texture-clamp configuration.

Verifies the limited visual change and full-mip policy, not a physical game
FPS gain. Swift checks cover per-game choice and exact automatic identities.
"""
from pathlib import Path
import importlib.util,os,shutil,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('pipeline',root/'tests/host/check-dxmt-patches.py')
pipeline=importlib.util.module_from_spec(spec);spec.loader.exec_module(pipeline)
sources=pipeline.patched_sources(root)
def block(text,needle):
    start=text.index(needle);brace=text.index('{',start);end=brace+1;depth=1
    while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
state=sources['src/d3d11/d3d11_state_object.cpp']
creation=block(state,'template <>\nHRESULT\nStateObjectCache<D3D11_SAMPLER_DESC, D3D11SamplerState>::CreateStateObject(')
address=block(state,'constexpr WMTSamplerAddressMode kAddressModeMap[] = {')+';'
compare=block(state,'constexpr WMTCompareFunction kCompareFunctionMap[] = {')+';'
auto_config=block(sources['src/d3d11/d3d11_texture_device.cpp'],'GetMipClampAutoConfig() {')
fixture=r'''
#define DXMT_NATIVE 1
#include "winemetal.h"
#include "light_graphics.h"
#include "d3d11_mip_clamp_policy.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
using HRESULT=int;constexpr HRESULT S_OK=0,S_FALSE=1,E_FAIL=-1,E_INVALIDARG=-2;
constexpr unsigned D3D11_TEXTURE_ADDRESS_BORDER=4;
#ifdef __uuidof
#undef __uuidof
#endif
#define __uuidof(...) 0
#define WARN(...) ((void)0)
#define ERR(...) ((void)0)
#define D3D11_DECODE_MIN_FILTER(x) (((x)>>4)&3)
#define D3D11_DECODE_MAG_FILTER(x) (((x)>>2)&3)
#define D3D11_DECODE_MIP_FILTER(x) ((x)&3)
#define D3D11_DECODE_IS_ANISOTROPIC_FILTER(x) ((x)&0x40)
#define D3D11_DECODE_IS_COMPARISON_FILTER(x) ((x)&0x80)
namespace dxmt{using mutex=std::mutex;}
namespace mip_clamp=dxmt::mip_clamp;
static constexpr bool kMadeira32BitModule=false;
namespace env{static std::string getEnvVar(const char*k){const char*v=getenv(k);return v?v:"";}}
struct Config{static Config&getInstance(){static Config c;return c;}template<class T>T getOption(const char*,T value){return value;} };
struct D3D11_SAMPLER_DESC {
 unsigned Filter,AddressU,AddressV,AddressW;float MipLODBias;unsigned MaxAnisotropy,ComparisonFunc;
 float BorderColor[4],MinLOD,MaxLOD;
 bool operator<(const D3D11_SAMPLER_DESC &b)const{return std::memcmp(this,&b,sizeof b)<0;}
};
template<class T>void InitReturnPtr(T **p){if(p)*p=nullptr;}
template<class T>using Rc=std::shared_ptr<T>;
struct MTLD3D11Device {int GetMTLDevice(){return 0;}};
struct Sampler{
 WMTSamplerInfo info;float bias;static inline unsigned creations=0;
 static Rc<Sampler> createSampler(int,WMTSamplerInfo info,float bias){++creations;return std::make_shared<Sampler>(Sampler{info,bias});}
};
struct D3D11SamplerState{
 virtual ~D3D11SamplerState()=default;
 virtual Rc<Sampler>sampler()=0;virtual void GetDesc(D3D11_SAMPLER_DESC*)=0;
 HRESULT QueryInterface(int,void **out){*out=this;return 0;}
};
struct MTLD3D11SamplerState:D3D11SamplerState{
 Rc<Sampler>value;D3D11_SAMPLER_DESC description;
 MTLD3D11SamplerState(MTLD3D11Device*,Rc<Sampler>&&s,D3D11_SAMPLER_DESC d):value(std::move(s)),description(d){}
 Rc<Sampler>sampler()override{return value;}void GetDesc(D3D11_SAMPLER_DESC*d)override{*d=description;}
};
template<class D,class O>struct StateObjectCache{
 std::mutex mutex_cache;MTLD3D11Device*device;std::map<D,std::unique_ptr<MTLD3D11SamplerState>> cache;
 explicit StateObjectCache(MTLD3D11Device*d):device(d){}
 HRESULT CreateStateObject(const D*,O**);
};
__ADDRESS__
__COMPARE__
__CREATION__
struct MipClampAutoConfig{bool enabled;uint32_t threshold_mb;};
static const MipClampAutoConfig &
__AUTO__
int main(int argc,char **argv){
 bool light=argc>1;assert(env::getEnvVar("MADEIRA_LIGHT_EFFECTS")== (light?"1":"0"));
 MTLD3D11Device d;StateObjectCache<D3D11_SAMPLER_DESC,D3D11SamplerState>cache(&d);
 D3D11_SAMPLER_DESC input={};input.Filter=0x55;input.MaxAnisotropy=16;
 input.AddressU=input.AddressV=input.AddressW=1;input.MinLOD=-16;input.MaxLOD=32;input.MipLODBias=-0.5;
 D3D11SamplerState*a=nullptr,*b=nullptr;
 assert(cache.CreateStateObject(&input,&a)==0 && a);
 auto sampler=a->sampler();assert(sampler->info.max_anisotroy==(light?8u:16u));
 assert(sampler->info.lod_min_clamp==input.MinLOD && sampler->info.lod_max_clamp==input.MaxLOD && sampler->bias==input.MipLODBias);
 D3D11_SAMPLER_DESC logical={};a->GetDesc(&logical);assert(std::memcmp(&input,&logical,sizeof input)==0);
 assert(cache.CreateStateObject(&input,&b)==0 && a==b && Sampler::creations==1);
 input.Filter=0xd5;input.ComparisonFunc=4;assert(cache.CreateStateObject(&input,&b)==0);
 assert(b->sampler()->info.max_anisotroy==16 && b->sampler()->info.compare_function==WMTCompareFunctionLessEqual);
 input.Filter=0;assert(cache.CreateStateObject(&input,&b)==0 && b->sampler()->info.max_anisotroy==1);
 for(unsigned n:{1u,2u,4u,8u}){
  input.Filter=0x55;input.MaxAnisotropy=n;assert(cache.CreateStateObject(&input,&b)==0 && b->sampler()->info.max_anisotroy==n);
 }
 assert(GetMipClampAutoConfig().enabled); // Existing pressure protection survives either profile.
 printf("PASS: light=%d ordinary16x=%u, shadow16x=16, point=1, LOD/descriptor/cache and texture safeguards retained\n",light,sampler->info.max_anisotroy);
}
'''.replace('__ADDRESS__',address).replace('__COMPARE__',compare).replace('__CREATION__',creation).replace('__AUTO__',auto_config)
library=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
swift='import Foundation\n'+block(library,'enum LightEffectsPolicy {')+r'''
assert(LightEffectsPolicy.enabled(choice:nil,appID:3097560,executable:"renamed"))
assert(LightEffectsPolicy.enabled(choice:nil,appID:nil,executable:"C:\\Games\\REPLACED\\Replaced.exe"))
assert(LightEffectsPolicy.enabled(choice:nil,appID:nil,executable:"Games/Liar's Bar/Liar's Bar.exe"))
assert(!LightEffectsPolicy.enabled(choice:false,appID:3097560,executable:"Liar's Bar.exe"))
assert(!LightEffectsPolicy.enabled(choice:nil,appID:nil,executable:"Games/Replaced Mod/Other.exe"))
assert(!LightEffectsPolicy.enabled(choice:nil,appID:nil,executable:"Games/Hollow Knight/Hollow_Knight.exe"))
assert(LightEffectsPolicy.enabled(choice:true,appID:nil,executable:"Game.exe"))
print("PASS: exact automatic identities, explicit per-game On/Off and unrelated games preserved")
'''
compiler=os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
assert compiler,'C++ compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-light-graphics-') as work:
 folder=Path(work);source=folder/'test.cpp';source.write_text(fixture,encoding='utf-8');binary=folder/('check.exe' if os.name=='nt' else 'check')
 subprocess.run([compiler,'-std=c++20','-O1','-pthread',*(['-static']if os.name=='nt'else[]),
  '-I'+str(root/'dxmt/src/winemetal'),'-I'+str(root/'dxmt/src/d3d11'),'-I'+str(root/'build/dxmt-ios'),str(source),'-o',str(binary)],check=True)
 for value in ('0','1'):
  subprocess.run([str(binary),*(['light']if value=='1'else[])],check=True,env={**os.environ,'MADEIRA_LIGHT_EFFECTS':value,'MADEIRA_MIP_CLAMP_AUTO':'1'})
 swiftc=shutil.which('swiftc')
 if swiftc:
  source=folder/'main.swift';source.write_text(swift,encoding='utf-8');binary=folder/'swift-check'
  subprocess.run([swiftc,str(source),'-o',str(binary)],check=True,timeout=60);subprocess.run([str(binary)],check=True)
 elif '--require-swift' in sys.argv:raise SystemExit('Swift compiler required')
 else:print('SKIP: Swift launch policy requires the Mac runner')
 if sys.platform=='darwin':
  binary=folder/'metal-check'
  subprocess.run(['xcrun','clang','-fobjc-arc','-O2','-I'+str(root/'build/dxmt-ios'),
   str(root/'tests/host/light-filtering-metal.m'),'-framework','Foundation','-framework','Metal',
   '-framework','CoreGraphics','-framework','ImageIO','-o',str(binary)],check=True)
  output=Path(os.environ.get('BITRISE_DEPLOY_DIR',str(folder)));output.mkdir(parents=True,exist_ok=True)
  subprocess.run([str(binary),str(output)],check=True,timeout=90)
