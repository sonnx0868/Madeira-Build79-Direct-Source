#!/usr/bin/env python3
"""Run patched Mesa initialization with failed GPU/software screens and OOM."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--source',type=Path,default=root/'research/mesa-25.0.7/src/gallium/frontends/osmesa/osmesa.c')
args=parser.parse_args()
source=args.source.read_text(encoding='utf-8')
start=source.index('static void\ncreate_st_manager(void)')
end=source.index('\n/**',start)
creation=source[start:end]
start=source.index('   struct pipe_frontend_screen *manager = get_st_manager();')
end=source.index('   osmesa->st = st_api_create_context(manager,',start)
guard=source[start:end]
fixture=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
static int setenv(const char *k,const char *v,int overwrite) { return overwrite || !getenv(k) ? _putenv_s(k,v) : 0; }
#endif
struct pipe_screen { int valid; };
struct pipe_frontend_screen { struct pipe_screen *screen; void *get_param, *get_egl_image; };
static struct pipe_frontend_screen *global_fscreen;
static struct pipe_screen gpu={1},software={2};
static int gpu_ok,software_ok,oom,calls,frees;
static void *allocate(size_t n) { return oom ? NULL : calloc(1,n); }
#define CALLOC_STRUCT(t) allocate(sizeof(struct t))
#define FREE(p) do { ++frees;free(p); } while(0)
static void destroy_st_manager(void) {}
static void *osmesa_st_get_param;
static struct pipe_screen *osmesa_create_screen(void) {
    ++calls;
    return !strcmp(getenv("GALLIUM_DRIVER"),"zink") ? (gpu_ok ? &gpu : NULL) : (software_ok ? &software : NULL);
}
static struct pipe_frontend_screen *get_st_manager(void) { return global_fscreen; }
'''+creation+r'''
static void *context_guard(void) {
    void *osmesa=malloc(1);
'''+guard+r'''
    free(osmesa);return manager->screen;
}
static void setup(int g,int s,int fallback,const char *driver) {
    free(global_fscreen);global_fscreen=NULL;
    gpu_ok=g;software_ok=s;oom=0;calls=frees=0;
    setenv("GALLIUM_DRIVER",driver,1);setenv("MADEIRA_GL_SOFTWARE_FALLBACK",fallback ? "1" : "0",1);
}
int main(void) {
    setup(1,1,1,"zink");create_st_manager();assert(context_guard()==&gpu && calls==1);
    setup(0,1,1,"zink");create_st_manager();assert(context_guard()==&software && calls==2);
    setup(0,1,0,"zink");create_st_manager();assert(context_guard()==NULL && calls==1 && frees==1);
    setup(0,0,1,"zink");create_st_manager();assert(context_guard()==NULL && calls==2 && frees==1);
    setup(0,0,1,"softpipe");create_st_manager();assert(context_guard()==NULL && calls==1 && frees==1);
    setup(1,1,1,"zink");oom=1;create_st_manager();assert(context_guard()==NULL && calls==0 && frees==1);
    free(global_fscreen);global_fscreen=NULL;
    puts("PASS: patched Mesa keeps GPU success, retries fresh software only when permitted, rejects NULL screens and OOM safely");
}
'''
if sys.platform == 'darwin':
    sdk=subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip()
    compiler=['xcrun','--sdk','macosx','clang','-isysroot',sdk]
else:
    executable=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
    if not executable: raise SystemExit('C compiler required')
    compiler=[executable]
with tempfile.TemporaryDirectory(prefix='madeira-osmesa-') as scratch:
    folder=Path(scratch);path=folder/'fixture.c';exe=folder/('fixture.exe' if os.name=='nt' else 'fixture')
    path.write_text(fixture,encoding='utf-8')
    subprocess.run(compiler+['-std=gnu11','-Wall','-Werror',str(path),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
