#!/usr/bin/env python3
"""Execute the production Zink probe with context-owned renderer strings.

Sanitizers on macOS catch reading glGetString storage after context teardown.
The probe must also reject a software renderer, not silently accept softpipe.
"""
from pathlib import Path
import os, shutil, subprocess, tempfile
root = Path(__file__).resolve().parents[2]
source = (root / "build/win32u-unix/opengl_ios.c").read_text(encoding="utf-8")
start = source.index("static BOOL zink_init(void)")
probe = source[start:source.index("UINT winios_OpenGLInit", start)]
compiler = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc") or shutil.which("clang")
if not compiler: raise SystemExit("A host C compiler is required")
harness = r'''
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef int BOOL, GLint;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef unsigned char GLboolean, GLubyte;
#define TRUE 1
#define FALSE 0
#define OSMESA_FORMAT 1
#define OSMESA_BGRA 2
#define OSMESA_PROFILE 3
#define OSMESA_CORE_PROFILE 4
#define OSMESA_CONTEXT_MAJOR_VERSION 5
#define OSMESA_CONTEXT_MINOR_VERSION 6
#define OSMESA_COMPAT_PROFILE 7
#define GL_UNSIGNED_BYTE 8
#define GL_RENDERER 9
#define GL_VERSION 10
#define RTLD_NOW 1
#define RTLD_LOCAL 2
#define ERR(...) ((void)0)
static void *osmesa_handle;
static void *(*pOSMesaCreateContextAttribs)(const int *,void *);
static void (*pOSMesaDestroyContext)(void *);
static GLboolean (*pOSMesaMakeCurrent)(void *,void *,GLenum,GLsizei,GLsizei);
static void *(*pOSMesaGetCurrentContext)(void);
static void (*pOSMesaPixelStore)(GLint,GLint);
static void *(*pOSMesaGetProcAddress)(const char *);
static void (*pz_glFinish)(void);
static const GLubyte *(*pz_glGetString)(GLenum);
static const char *selected="zink Vulkan Metal";
static char *owned;
static int creations, destructions;
static void *create(const int *a,void *share) {
    (void)a; (void)share; owned=malloc(strlen(selected)+1); strcpy(owned,selected); ++creations; return owned;
}
static void destroy(void *p) { free(p); owned=NULL; ++destructions; }
static GLboolean current(void *c,void *b,GLenum t,GLsizei w,GLsizei h) { return 1; }
static void *get_current(void) { return owned; }
static void pixel_store(GLint p,GLint v) {}
static void finish(void) {}
static const GLubyte *get_string(GLenum n) { return (const GLubyte *)(n==GL_RENDERER ? owned : "4.1"); }
static void *get_proc(const char *n) { return !strcmp(n,"glFinish") ? (void *)finish : (void *)get_string; }
static void *test_dlopen(const char *p,int flags) { return (void *)1; }
static const char *test_dlerror(void) { return "test"; }
static char *test_getenv(const char *n) { return !strcmp(n,"MADEIRA_GL_DIR") ? "/renderer" : NULL; }
static int test_setenv(const char *n,const char *v,int replace) { return 0; }
static void *test_dlsym(void *h,const char *n) {
    if (!strcmp(n,"OSMesaCreateContextAttribs")) return (void *)create;
    if (!strcmp(n,"OSMesaDestroyContext")) return (void *)destroy;
    if (!strcmp(n,"OSMesaMakeCurrent")) return (void *)current;
    if (!strcmp(n,"OSMesaGetCurrentContext")) return (void *)get_current;
    if (!strcmp(n,"OSMesaPixelStore")) return (void *)pixel_store;
    if (!strcmp(n,"OSMesaGetProcAddress")) return (void *)get_proc;
    return NULL;
}
#define dlopen test_dlopen
#define dlerror test_dlerror
#define dlsym test_dlsym
#define getenv test_getenv
#define setenv test_setenv
'''
checks = r'''
int main(void) {
    assert(zink_init() && owned==NULL && creations==1 && destructions==1);
    selected="softpipe";
    assert(!zink_init() && owned==NULL && creations==2 && destructions==2);
    return 0;
}
'''
with tempfile.TemporaryDirectory() as folder:
    p=Path(folder); c=p/"probe.c"; exe=p/("probe.exe" if os.name=="nt" else "probe")
    c.write_text(harness+probe+checks,encoding="utf-8")
    flags=[] if os.name=="nt" else ["-fsanitize=address,undefined"]
    subprocess.run([compiler,"-std=c11","-O1",*flags,str(c),"-o",str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print("PASS: production renderer probe accepts Zink, rejects softpipe and never reads a destroyed context's strings")
