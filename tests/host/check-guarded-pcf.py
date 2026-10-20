#!/usr/bin/env python3
"""Compile the experimental LLVM15 pass tests and execute emitted native IR.

Requires an existing LLVM15 SDK; never downloads or modifies the app pipeline.
This is arithmetic/control-flow validation, not a GPU performance measurement.
"""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--llvm-config', default=os.environ.get('LLVM_CONFIG') or str(root / 'toolchains/llvm15-macos/bin/llvm-config'))
parser.add_argument('--cxx', default=os.environ.get('CXX') or shutil.which('clang++'))
parser.add_argument('--cc', default=os.environ.get('CC') or shutil.which('clang'))
args = parser.parse_args()
if not args.cxx or not args.cc:
    raise SystemExit('Existing clang/clang++ toolchain required')

def config(*flags):
    return subprocess.check_output([args.llvm_config, *flags], text=True).strip()

if not config('--version').startswith('15.'):
    raise SystemExit('This regression requires LLVM15, matching the DXMT compiler')
sdk_flags = []
if sys.platform == 'darwin':
    # xcrun normally supplies SDKROOT. The workflow passes absolute compiler
    # paths, so carry the selected SDK explicitly into both compiler calls.
    sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'], text=True).strip()
    sdk_flags = ['-isysroot', sdk]

harness = r'''
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern unsigned sample_calls;
#define DECLARE(i) extern float baseline_##i(int,float),guarded_##i(int,float)
DECLARE(0);DECLARE(1);DECLARE(2);DECLARE(3);DECLARE(4);DECLARE(5);DECLARE(6);DECLARE(7);
typedef float (*kernel)(int,float);
static uint32_t bits(float value) {uint32_t result;memcpy(&result,&value,4);return result;}
int main(void) {
  kernel baseline[]={baseline_0,baseline_1,baseline_2,baseline_3,baseline_4,baseline_5,baseline_6,baseline_7};
  kernel guarded[]={guarded_0,guarded_1,guarded_2,guarded_3,guarded_4,guarded_5,guarded_6,guarded_7};
  unsigned inside=0,outside=0;
  for(unsigned k=0;k<sizeof(baseline)/sizeof(baseline[0]);k++)for(unsigned i=0;i<132;i++) {
    // Alternating predicates model both paths and mixed lanes. This CPU test
    // does not claim SIMD convergence or Metal timing coverage.
    int predicate=i&1,constant=(k&2)?predicate:!predicate;
    float x=(i%64)/64.0f;
    if(i>=128)x=(i&2)?INFINITY:NAN;
    sample_calls=0;float expected=baseline[k](predicate,x);assert(sample_calls==9);
    sample_calls=0;float actual=guarded[k](predicate,x);
    assert(bits(expected)==bits(actual));
    assert(sample_calls==(constant?0:9));
    if(constant){assert(bits(actual)==0x3f800000);outside++;}
    else {if(i<128)assert(actual>0 && actual<1);inside++;}
  }
  assert(inside && outside);
  printf("PASS: %u inside and %u outside native cases; bitwise parity, nonzero sums, nine retained/zero discarded samples\n",inside,outside);
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-pcf-guard-') as temporary:
    out = Path(temporary)
    check = out / ('check.exe' if os.name == 'nt' else 'check')
    ir = out / 'native.ll'
    cflags = shlex.split(config('--cxxflags'))
    libs = shlex.split(config('--ldflags', '--system-libs', '--libs', 'core', 'passes'))
    rpath = ['-Wl,-rpath,' + config('--libdir')] if sys.platform != 'win32' else []
    subprocess.run([args.cxx, *cflags, *sdk_flags, '-std=c++17', '-O2', '-I' + str(root / 'build/dxmt-ios'),
                    str(root / 'tests/host/guarded-pcf.cpp'), *libs, *rpath, '-o', str(check)], check=True)
    subprocess.run([str(check), str(ir)], check=True, timeout=60)
    source = out / 'native.c'
    source.write_text(harness, encoding='utf-8')
    native = out / ('native.exe' if os.name == 'nt' else 'native')
    subprocess.run([args.cc, *sdk_flags, '-O0', str(ir), str(source), '-lm', '-o', str(native)], check=True)
    subprocess.run([str(native)], check=True, timeout=30)
