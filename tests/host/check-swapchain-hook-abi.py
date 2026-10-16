#!/usr/bin/env python3
"""Test the production ABI annotation using genuine ARM64EC compiler output."""
from pathlib import Path
import argparse, importlib.util, os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('swapchain_contract',root/'tools/dxmt-swapchain-bundle.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--compiler');parser.add_argument('--require-tools',action='store_true')
a=parser.parse_args();compiler=a.compiler or os.environ.get('MD_RUNTIME_ARM64EC_CXX') or shutil.which('arm64ec-w64-mingw32-clang++')
if not compiler:
    if a.require_tools:raise SystemExit('ARM64EC compiler required')
    print('SKIP: ARM64EC compiler not available');raise SystemExit(0)
source=r'''
#include "swapchain_hook_abi.h"
struct Base {
 virtual long Present(unsigned,unsigned)=0;
 virtual long ResizeBuffers(unsigned,unsigned,unsigned,unsigned,unsigned)=0;
 virtual long Present1(unsigned,unsigned,const void*)=0;
 virtual long ResizeBuffers1(unsigned,unsigned,unsigned,unsigned,unsigned,const unsigned*,void*const*)=0;
};
struct Chain final:Base {
 long MADEIRA_SWAPCHAIN_ENTRY Present(unsigned a,unsigned b) final { return Present1(a,b,nullptr); }
 long MADEIRA_SWAPCHAIN_ENTRY ResizeBuffers(unsigned a,unsigned b,unsigned c,unsigned d,unsigned e) final { return a+b+c+d+e; }
 long MADEIRA_SWAPCHAIN_ENTRY Present1(unsigned a,unsigned b,const void *p) final { return a+b+(p!=nullptr); }
 long MADEIRA_SWAPCHAIN_ENTRY ResizeBuffers1(unsigned a,unsigned b,unsigned c,unsigned d,unsigned e,const unsigned*,void*const*) final { return ResizeBuffers(a,b,c,d,e); }
};
extern "C" __declspec(dllexport) Base *Make(){ static Chain chain;return &chain; }
'''
prefixes=[('_ZN5Chain','_ZTV5Chain')];layout={'Present':0,'ResizeBuffers':1,'Present1':2,'ResizeBuffers1':3}
with tempfile.TemporaryDirectory(prefix='madeira-swapchain-abi-') as temp:
    p=Path(temp);(p/'fixture.cpp').write_text(source,encoding='utf-8')
    for enabled in (False,True):
        dll=p/('enabled.dll' if enabled else 'before.dll')
        subprocess.run([compiler,'-std=c++20','-O2','-fno-rtti','-fno-exceptions','-shared',
            '-Wl,--no-insert-timestamp','-DDXMT_IOS='+('1' if enabled else '0'),
            *([] if enabled else ['-UDXMT_IOS']),'-I'+str(root/'build/dxmt-ios'),str(p/'fixture.cpp'),'-o',str(dll)],check=True)
        pe=m.PE(dll.read_bytes())
        if enabled:
            entries=m.verify_entries(pe,prefixes,layout);assert len(entries)==4
            # A vtable referring straight to the native implementation, as in
            # the failing production image, must fail this gate even if a FFS
            # happens to exist elsewhere in the image.
            syms=m.symbols(pe);base,_,_=m.hybrid(pe);bad=bytearray(pe.data)
            import struct
            struct.pack_into('<Q',bad,pe.offset(syms['_ZTV5Chain']+16,8),base+entries[0]['native_body_rva'])
            try:m.verify_entries(m.PE(bytes(bad)),prefixes,layout)
            except AssertionError:pass
            else:raise AssertionError('raw ARM64 vtable target accepted')
        else:
            try:m.verify_entries(pe,prefixes,layout)
            except (AssertionError,KeyError):pass
            else:raise AssertionError('pre-fix native entries accepted as hookable')
print('PASS: genuine EC COM vtables expose all four x64 hook entries; native implementations/dispatch remain; pre-fix and raw-body targets rejected')
