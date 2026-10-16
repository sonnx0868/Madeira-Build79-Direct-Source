#!/usr/bin/env python3
"""Exercise the production full-clear coverage predicate, including unsafe cases."""
from pathlib import Path
import os, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
fixture = r'''
#include "color_store_policy.hpp"
#include <cassert>
struct Render { unsigned render_target_width=1920, render_target_height=1080, render_target_array_length=1; };
struct Clear { bool clear_dsv=false; void *attachment; unsigned width=1920, height=1080, array_length=1; };
struct Color { void *attachment; unsigned level=0, slice=0, depth_plane=0; void *resolve_attachment=nullptr; int store_action=1; };
int main() {
 int a,b; Render r; Clear c{false,&a}; Color v{&a};
 assert(madeiraCanDiscardClearedColor(&r,&c,v));
 auto reject=[&]{assert(!madeiraCanDiscardClearedColor(&r,&c,v));};
 c.width--; reject(); c.width++;
 c.height--; reject(); c.height++;
 c.array_length++; reject(); c.array_length--;
 c.clear_dsv=true; reject(); c.clear_dsv=false;
 c.attachment=nullptr; reject(); c.attachment=&b; reject(); c.attachment=&a;
 v.level=1; reject(); v.level=0; v.slice=1; reject(); v.slice=0;
 v.depth_plane=1; reject(); v.depth_plane=0;
 v.resolve_attachment=&b; reject(); v.resolve_attachment=nullptr;
 v.store_action=2; reject(); v.store_action=1;
 r.render_target_width=c.width=0; reject(); r.render_target_width=c.width=1920;
 r.render_target_height=c.height=0; reject(); r.render_target_height=c.height=1080;
 r.render_target_array_length=c.array_length=0; reject();
}
'''
# The constant belongs to winemetal's enum in production; isolate the pure
# decision from that platform header so every guard is tested on host CI too.
fixture = fixture.replace('#include "color_store_policy.hpp"',
                          'enum { WMTStoreActionStore=1 };\n#include "color_store_policy.hpp"')
compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('g++')
assert compiler, 'C++ compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-color-store-') as scratch:
    p = Path(scratch)
    (p/'check.cpp').write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=c++17','-I',str(root/'build/dxmt-ios'),str(p/'check.cpp'),'-o',str(p/'check.exe')],check=True)
    subprocess.run([str(p/'check.exe')],check=True)
print('PASS: exact full color overwrite accepted; partial, depth, resolve, other view/subresource and empty extents rejected')
