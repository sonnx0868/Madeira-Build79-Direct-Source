#!/usr/bin/env python3
"""Test verbatim upstream feature selection, ABI choices and IR marker storage."""
from pathlib import Path
import os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
bridge=(root/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
startup=bridge[bridge.index('ios_main_image_i386 = is_i386_target ? 1 : 0;'):]
startup=startup[:startup.index('__wine_main(argc, argv);')]
assert '\n        madeira_publish_host_probe();' in startup
assert 'if (has_i386_set) madeira_publish_host_probe()' not in startup
names=['Source/Windows/Common/CPUFeatures.cpp','FEXCore/Source/Interface/IR/PassManager.cpp']
compiler=os.environ.get('CXX') or shutil.which('clang++')
if not compiler: raise SystemExit('clang++ required')
pin=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD:FEX'],text=True).strip()
with tempfile.TemporaryDirectory(prefix='madeira-upstream-fex-') as scratch:
    source=Path(scratch)
    for name in names:
        file=source/name;file.parent.mkdir(parents=True,exist_ok=True)
        file.write_bytes(subprocess.check_output(['git','-C',str(root/'FEX'),'show',pin+':'+name]))
    for _ in range(2):
        subprocess.run(['python3' if os.name!='nt' else 'python',str(root/'scripts/apply-fex-host-features.py'),'--source',str(source)],check=True)
    cpu=(source/names[0]).read_text(encoding='utf-8')
    at=cpu.index('FEXCore::HostFeatures CPUFeatures::FetchHostFeatures(')
    end=cpu.index('\n  HKEY Key =',at)
    function=cpu[at:end]+'\n  return {};\n#endif\n}\n'
    ir=(source/names[1]).read_text(encoding='utf-8')
    begin=ir.index('#if defined(FEX_IOS_HOST) && defined(_WIN32)',ir.index('namespace {'))
    marker=ir[begin:ir.index('\nstd::atomic<uint32_t> IRCapTaken',begin)]
    fixture=r'''
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <type_traits>
static uint64_t overrides;
#define FEX_CONFIG_OPT(name,key) auto name = [] { return overrides; }
#ifdef MADEIRA_HOST_WINDOWS
#define setenv(name,value,overwrite) _putenv_s(name,value)
#endif
namespace FEXCore {
struct HostFeatures {
 enum class HostTypeEnum { Arm64ec, Wow64 };
 unsigned DCacheLineSize{},ICacheLineSize{};
 bool SupportsCacheMaintenanceOps{},SupportsAES{},SupportsCRC{},SupportsAtomics{},SupportsRCPC{},SupportsSHA{};
 bool SupportsPMULL_128Bit{},SupportsFCMA{},SupportsFlagM{},SupportsFlagM2{},SupportsAFP{},SupportsTSOImm9{};
 bool SupportsAVX{},SupportsAES256{};
 std::vector<unsigned> CPUMIDRs;
 HostTypeEnum HostType{};
};
namespace Config { struct HostFeatures { static constexpr uint64_t DISABLELRCPC2=1, ENABLEAFP=2; }; }
}
namespace LogMan::Msg { template<class... Args> void IFmt(const char*,Args...) {} }
namespace FEX::Windows {
struct CPUFeatures { static FEXCore::HostFeatures FetchHostFeatures(bool,FEXCore::HostFeatures::HostTypeEnum); };
'''+function+'\n}\n#ifndef _WIN32\n#define _WIN32 1\n#endif\n'+marker+r'''
int main(void) {
 using Type=FEXCore::HostFeatures::HostTypeEnum;
 setenv("MADEIRA_FEX_AVX","0",1);
 setenv("FEX_MADEIRA_HOSTPROBE","LRCPC2=1,AFP=0,FLAGM=1,FLAGM2=1",1);
#ifdef ARCHITECTURE_arm64ec
 auto features=FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec);
 assert(features.SupportsTSOImm9 && !features.SupportsAFP && !features.SupportsAVX);
 for(const char* probe: {"LRCPC2=?","LRCPC2=0","XLRCPC2=1","AFP=0"}) {
   setenv("FEX_MADEIRA_HOSTPROBE",probe,1);
   assert(!FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec).SupportsTSOImm9);
 }
 setenv("FEX_MADEIRA_HOSTPROBE","LRCPC2=1,AFP=1",1);overrides=1;
 assert(!FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec).SupportsTSOImm9);
 overrides=2;assert(FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec).SupportsAFP);
 setenv("FEX_MADEIRA_HOSTPROBE","LRCPC2=1,AFP=0,FLAGM=0,FLAGM2=0",1);
 features=FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec);
 assert(!features.SupportsAFP && !features.SupportsFlagM && !features.SupportsFlagM2);
 auto wow=FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Wow64);
 assert(!wow.SupportsAFP);
 overrides=0;setenv("MADEIRA_FEX_AVX","1",1);
 assert(FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Arm64ec).SupportsAVX);
 assert(!FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Wow64).SupportsAVX);
#else
 auto wow=FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Wow64);
 assert(!wow.SupportsTSOImm9 && !wow.SupportsAFP);
 setenv("MADEIRA_FEX_AVX","1",1);
 assert(!FEX::Windows::CPUFeatures::FetchHostFeatures(true,Type::Wow64).SupportsAVX);
#endif
 static_assert(std::is_same_v<decltype(IRCapRIP),std::atomic<uint64_t>>);
 uint64_t gameTLS[2]={0x12345678,0xabcdef};IRCapRIP=42;IRCapRIP=0;
 assert(gameTLS[0]==0x12345678 && gameTLS[1]==0xabcdef);
 puts("PASS: upstream CPU positive/unknown/negative probes, overrides, WOW64/AVX choices and non-TLS IR marker");
}
'''
    file=source/'check.cpp';file.write_text(fixture,encoding='utf-8');exe=source/'check.exe'
    env=dict(os.environ);env['PATH']=str(Path(compiler).parent)+os.pathsep+env.get('PATH','')
    for arch in ([],['-DARCHITECTURE_arm64ec=1']):
        host=['-DMADEIRA_HOST_WINDOWS=1'] if os.name=='nt' else []
        subprocess.run([compiler,'-std=c++20','-DFEX_IOS_HOST=1',*host,*arch,str(file),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],env=env,check=True)
