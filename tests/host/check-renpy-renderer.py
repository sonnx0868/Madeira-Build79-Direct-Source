#!/usr/bin/env python3
"""Compile the production renderer policy; exercise overrides and game changes."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
bridge = (root / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")
start = bridge.index("static void madeira_prepare_renpy_renderer(void)")
end = bridge.index("\n/* Re-applied", start)
policy = bridge[start:end]
assert bridge.count("madeira_prepare_renpy_renderer();") == 2
prefix = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
static int setenv(const char *key,const char *value,int overwrite) {
    return overwrite || !getenv(key) ? _putenv_s(key,value) : 0;
}
static int unsetenv(const char *key) { return _putenv_s(key,""); }
#endif
static const char *configured_renderer;
static int policy_enabled = 1;
static int madeira_cfg_get(const char *key,char *out,size_t length) {
    assert(!strcmp(key,"env.RENPY_RENDERER"));
    if(!configured_renderer) return 0;
    snprintf(out,length,"%s",configured_renderer); return 1;
}
static int madeira_cfg_bool(const char *key,int fallback) {
    (void)fallback;
    assert(!strcmp(key,"env.MADEIRA_RENPY_RENDERER")); return policy_enabled;
}
'''
suffix = r'''
static void expect(const char *key,const char *value) {
    const char *current=getenv(key);
    assert(value ? current && !strcmp(current,value) : !current);
}
int main(void) {
    unsetenv("RENPY_RENDERER"); unsetenv("_MADEIRA_RENPY_AUTO_RENDERER");
    setenv("_MADEIRA_OPENGL_ANGLE_MODE","renpy",1);
    setenv("MADEIRA_GL_BACKEND","auto",1);
    setenv("_MADEIRA_OPENGL_ANGLE_AUTO_ENV","1",1);
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","angle2");
    unsetenv("_MADEIRA_OPENGL_ANGLE_MODE");
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER",NULL);
    setenv("_MADEIRA_OPENGL_ANGLE_MODE","renpy",1);
    setenv("MADEIRA_GL_BACKEND","gles",1);
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","gles2");
    setenv("MADEIRA_GL_BACKEND","zink",1);
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","gl2");
    configured_renderer="angle2";
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","angle2");
    expect("_MADEIRA_RENPY_AUTO_RENDERER",NULL);
    configured_renderer=NULL;
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","angle2");
    unsetenv("RENPY_RENDERER"); unsetenv("_MADEIRA_OPENGL_ANGLE_AUTO_ENV");
    setenv("MADEIRA_GL_BACKEND","auto",1);
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","gles2");
    expect("MADEIRA_GL_BACKEND","gles"); expect("SDL_OPENGL_ES_DRIVER","0");
    policy_enabled=0;
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER",NULL);
    setenv("RENPY_RENDERER","gl2",1);
    madeira_prepare_renpy_renderer(); expect("RENPY_RENDERER","gl2");
    puts("PASS: RenPy renderer selection, overrides, backend fallback and session cleanup");
}
'''
cc = os.environ.get("CC") or shutil.which("cc")
if not cc:
    raise SystemExit("C compiler required")
with tempfile.TemporaryDirectory(prefix="madeira-renpy-renderer-") as scratch:
    folder = Path(scratch)
    source, binary = folder / "policy.c", folder / ("policy.exe" if os.name == "nt" else "policy")
    source.write_text(prefix + policy + suffix, encoding="utf-8")
    flags = ["-std=gnu11", "-Wall", "-Wextra", "-Werror", "-D_DEFAULT_SOURCE"]
    if os.name == "nt": flags += ["-static"]
    subprocess.run([cc,*flags,str(source),"-o",str(binary)],check=True)
    subprocess.run([str(binary)],check=True,timeout=10)
