#!/usr/bin/env python3
"""Run the native WGL initializer with a faulting Mesa probe and RenPy ANGLE."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root/'build/win32u-unix/opengl_ios.c').read_text(encoding='utf-8')
initializer = source[source.index('UINT winios_OpenGLInit('):]
assert 'renpy-wgl-probe-v1' in initializer
fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ios_gl_route.h"
typedef unsigned UINT;
typedef int BOOL;
struct opengl_funcs { int unused; };
struct opengl_driver_funcs { int kind; };
#define WINE_OPENGL_DRIVER_VERSION 42
#define STATUS_INVALID_PARAMETER 1
#define STATUS_NOT_SUPPORTED 2
#define STATUS_SUCCESS 0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define OPENGLES_PATH "OpenGLES"
#define RTLD_NOW 2
#define ERR(...) ((void)0)
#ifdef _WIN32
static int setenv(const char *key,const char *value,int overwrite) {
    return overwrite || !getenv(key) ? _putenv_s(key,value) : 0;
}
static int unsetenv(const char *key) { return _putenv_s(key,""); }
#endif
static const struct opengl_driver_funcs zink_driver_funcs={1}, ios_driver_funcs={2};
static const int ios_formats[6]={0};
static unsigned zink_calls, gles_calls;
static int zink_faults, gles_missing;
static void *gles_handle;
static BOOL zink_init(void) {
    ++zink_calls;
    /* Models the native NULL pipe dereference: the test fails if invoked. */
    assert(!zink_faults);
    return 1;
}
static void *dlopen(const char *path,int flags) {
    assert(!strcmp(path,OPENGLES_PATH) && flags==RTLD_NOW);
    ++gles_calls; return gles_missing ? NULL : (void *)1;
}
static const char *dlerror(void) { return "unavailable"; }
static void *dlsym(void *handle,const char *name) {
    assert(handle==(void *)1 && name && *name); return (void *)2;
}
static void *p_glGenFramebuffers, *p_glDeleteFramebuffers, *p_glBindFramebuffer;
static void *p_glGenRenderbuffers, *p_glDeleteRenderbuffers, *p_glBindRenderbuffer;
static void *p_glRenderbufferStorage, *p_glFramebufferRenderbuffer, *p_glCheckFramebufferStatus;
static void *p_glGetIntegerv, *p_glGetFloatv, *p_glClearDepthf, *p_glDepthRangef, *p_glDrawBuffers;
'''
fixture += initializer
fixture += r'''
static void setup(const char *backend,const char *mode,const char *renderer,int fault) {
    if(backend) setenv("MADEIRA_GL_BACKEND",backend,1); else unsetenv("MADEIRA_GL_BACKEND");
    if(mode) setenv("_MADEIRA_OPENGL_ANGLE_MODE",mode,1); else unsetenv("_MADEIRA_OPENGL_ANGLE_MODE");
    if(renderer) setenv("RENPY_RENDERER",renderer,1); else unsetenv("RENPY_RENDERER");
    zink_calls=gles_calls=0; zink_faults=fault; gles_missing=0;
}
static void expect(int kind,unsigned zink,unsigned gles) {
    const struct opengl_driver_funcs *out=NULL;
    assert(winios_OpenGLInit(42,NULL,&out)==0);
    assert(out && out->kind==kind && zink_calls==zink && gles_calls==gles);
}
int main(void) {
    setup("auto","renpy","angle2",1); expect(2,0,1);
    assert(!strcmp(getenv("MADEIRA_GL_BACKEND"),"auto"));
    assert(!strcmp(getenv("RENPY_RENDERER"),"angle2"));
    setup("software","renpy","angle2",0); expect(1,1,0);
    setup(NULL,"renpy","angle2",1); expect(2,0,1);
    setup("auto","renpy","angle",1); expect(2,0,1);
    setup("angle","renpy","angle2",1); expect(2,0,1);
    setup("zink","renpy","gl2",0); expect(1,1,0);
    setup("zink","renpy","angle2",0); expect(1,1,0);
    setup("auto","renpy","gl2",0); expect(1,1,0);
    setup("auto","renpy",NULL,0); expect(1,1,0);
    setup("auto","love","angle2",0); expect(1,1,0);
    setup("auto",NULL,"angle2",0); expect(1,1,0);
    setup("gles","renpy","gles2",1); expect(2,0,1);
    setup("auto","renpy","angle2",1); gles_missing=1;
    const struct opengl_driver_funcs *out=NULL;
    assert(winios_OpenGLInit(42,NULL,&out)==STATUS_NOT_SUPPORTED);
    assert(!out && !zink_calls && gles_calls==1);
    setup("auto","renpy","angle2",1);
    assert(winios_OpenGLInit(99,NULL,&out)==STATUS_INVALID_PARAMETER);
    assert(!out && !zink_calls && !gles_calls);
    puts("PASS: production WGL initializer avoids faulting Mesa for RenPy ANGLE, preserves pixel formats, explicit GL choices and renderer environment");
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required for RenPy WGL probe regression')
with tempfile.TemporaryDirectory(prefix='madeira-renpy-wgl-') as scratch:
    folder=Path(scratch); c=folder/'fixture.c'; exe=folder/('fixture.exe' if os.name=='nt' else 'fixture')
    c.write_text(fixture,encoding='utf-8')
    subprocess.run([compiler,'-std=gnu11','-Werror','-O2','-I',str(root/'build/win32u-unix'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
