#!/usr/bin/env python3
"""Test the real descriptor policy and Metal view/pixel compatibility.

These are correctness checks. The virtual Mac GPU cannot establish thermal
or FPS improvement on the user's iPad.
"""
from pathlib import Path
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('dxmt_patches',root/'tests/host/check-dxmt-patches.py')
module = importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
sources = module.patched_sources(root)
text = sources['src/d3d11/d3d11_resource_helper.cpp']
start = text.index('  if (metal_format.Flag & MTL_DXGI_FORMAT_TYPELESS) {')
depth=1;end=text.index('{',start)+1
while depth:
    depth += (text[end]=='{')-(text[end]=='}');end+=1
policy = text[start:end]
cxx = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
assert cxx, 'C++ compiler required'
source = r'''
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string>
enum { MTL_DXGI_FORMAT_TYPELESS=1,MTL_DXGI_FORMAT_DEPTH_PLANER=2,MTL_DXGI_FORMAT_STENCIL_PLANER=4,MTL_DXGI_FORMAT_BC=8 };
enum { D3D11_BIND_DEPTH_STENCIL=64,D3D11_BIND_UNORDERED_ACCESS=128,WMTTextureUsagePixelFormatView=16 };
namespace env { static std::string getEnvVar(const char *s) {const char *v=getenv(s);return v?v:"";} }
static unsigned usage(unsigned format_flags,unsigned BindFlags) {
    struct { unsigned Flag; } metal_format={format_flags};
    unsigned metal_usage=5;
__POLICY__
    return metal_usage;
}
int main(int argc,char **argv) {
    bool restore=argc>1;
    assert(usage(0,0)==5);
    assert(usage(MTL_DXGI_FORMAT_TYPELESS,0)==(restore?21:5));
    assert(usage(MTL_DXGI_FORMAT_TYPELESS,D3D11_BIND_DEPTH_STENCIL)==21);
    assert(usage(MTL_DXGI_FORMAT_TYPELESS,D3D11_BIND_UNORDERED_ACCESS)==21);
    for(unsigned mask=2;mask<16;mask+=2) assert(usage(MTL_DXGI_FORMAT_TYPELESS|mask,0)==21);
    puts("PASS: production descriptor policy preserves typed, depth-bound, UAV, plane/BC resources and explicit restore");
}
'''.replace('__POLICY__',policy)
with tempfile.TemporaryDirectory(prefix='madeira-lossless-views-') as scratch:
    folder=Path(scratch);path=folder/'policy.cpp';path.write_text(source,encoding='utf8')
    binary=folder/('policy.exe' if os.name=='nt' else 'policy')
    subprocess.run([cxx,'-std=c++20','-O2',*(['-static'] if os.name=='nt' else []),str(path),'-o',str(binary)],check=True)
    clean={k:v for k,v in os.environ.items() if k!='DXMT_TYPELESS_PFV'}
    subprocess.run([str(binary)],check=True,env=clean)
    subprocess.run([str(binary),'restore'],check=True,env={**clean,'DXMT_TYPELESS_PFV':'1'})
    if sys.platform!='darwin':
        if '--require-darwin' in sys.argv:raise SystemExit('Darwin/Metal required')
        print('SKIP: real Metal view and pixel checks require Darwin');raise SystemExit(0)
    native=root/'tests/host/lossless-color-views.m'
    sdk=subprocess.check_output(['xcrun','--sdk','iphoneos','--show-sdk-path'],text=True).strip()
    subprocess.run(['xcrun','--sdk','iphoneos','clang','-target','arm64-apple-ios18.0','-isysroot',sdk,
        '-fobjc-arc','-Werror','-fsyntax-only',str(native)],check=True)
    executable=folder/'native'
    subprocess.run(['xcrun','clang','-fobjc-arc','-O2',str(native),'-framework','Foundation','-framework','Metal','-o',str(executable)],check=True)
    output=folder/'lossless-color-view-validation.json'
    subprocess.run([str(executable),str(output)],check=True,timeout=90,env={**os.environ,'MTL_DEBUG_LAYER':'1'})
    report=json.loads(output.read_text(encoding='utf8'));print(json.dumps(report,sort_keys=True))
    if os.environ.get('BITRISE_DEPLOY_DIR'):
        target=Path(os.environ['BITRISE_DEPLOY_DIR']);target.mkdir(parents=True,exist_ok=True);shutil.copy2(output,target/output.name)
