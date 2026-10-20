#!/usr/bin/env python3
"""Exercise the research-only transform with LLVM15 and native IEEE inputs."""
import argparse, os, pathlib, shlex, subprocess, sys, tempfile
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--llvm-config',required=True)
p.add_argument('--cxx',required=True)
p.add_argument('--cc',required=True)
a=p.parse_args(); root=pathlib.Path(__file__).resolve().parents[2]
def config(*flags):return subprocess.check_output([a.llvm_config,*flags],text=True).strip()
if not config('--version').startswith('15.'):raise SystemExit('LLVM15 required')
sdk=['-isysroot',subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip()] if sys.platform=='darwin' else []
harness=r'''
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
extern unsigned sample_calls;
extern float baseline(uint32_t,float,float,float,bool),guarded(uint32_t,float,float,float,bool);
static uint32_t bits(float x){uint32_t v;memcpy(&v,&x,4);return v;}
static float real(uint32_t x){float v;memcpy(&v,&x,4);return v;}
int main(void){
 uint32_t values[]={0,0x80000000,1,0x80000001,0x3f800000,0xbf800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000};
 unsigned cases=0,skipped=0;uint32_t random=1234567;
 for(unsigned i=0;i<100000;i++)for(unsigned active=0;active<2;active++){
  random=random*1664525u+1013904223u;
  uint32_t attenuation=values[i%11];
  float r=real(values[(i/11)%11]),g=real(values[(i/121)%11]),b=real(random);
  sample_calls=0;float x=baseline(attenuation,r,g,b,active);unsigned before=sample_calls;
  sample_calls=0;float y=guarded(attenuation,r,g,b,active);
  assert(bits(x)==bits(y)||(isnan(x)&&isnan(y)));
  unsigned zero=(attenuation&0x7fffffff)==0;
  assert(before==(active?4:0));assert(sample_calls==(active&&!zero?4:0));
  if(active&&zero)skipped++;cases++;
 }
 printf("PASS: %u native cases (signed zero, subnormals, finite, infinities, NaNs); %u zero-contribution shadow regions skipped\n",cases,skipped);
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-zero-light-') as temporary:
 out=pathlib.Path(temporary);suffix='.exe' if os.name=='nt' else ''
 check=out/('check'+suffix);ir=out/'native.ll'
 flags=shlex.split(config('--cxxflags'));libs=shlex.split(config('--ldflags','--system-libs','--libs','core','passes'))
 rpath=['-Wl,-rpath,'+config('--libdir')] if os.name!='nt' else []
 subprocess.run([a.cxx,*flags,*sdk,'-std=c++17','-O2','-I'+str(root/'tools'),'-I'+str(root/'build/dxmt-ios'),str(root/'tests/host/zero-light-guard.cpp'),*libs,*rpath,'-o',str(check)],check=True)
 subprocess.run([str(check),str(ir)],check=True,timeout=60)
 (out/'native.c').write_text(harness)
 subprocess.run([a.cc,*sdk,'-O0',str(ir),str(out/'native.c'),'-lm','-o',str(out/('native'+suffix))],check=True)
 subprocess.run([str(out/('native'+suffix))],check=True,timeout=30)
