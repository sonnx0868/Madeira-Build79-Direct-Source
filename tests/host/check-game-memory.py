#!/usr/bin/env python3
"""Exercise actual Swift launch policy and native profile precedence."""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
library=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
bridge=(root/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
content=(root/'app/Madeira/ContentView.swift').read_text(encoding='utf-8')
assert 'madeira_game_swap_capacity((long)madeira_cfg_int("swap-mb", 0))' in bridge
assert bridge.count('madeira_apply_game_memory_overrides();')==2
assert 'if getenv("_MADEIRA_GAME_REAL_SUSPEND") == nil, let txt' in content
start=library.index('enum GameMemorySettings {'); brace=library.index('{',start); end=brace+1; depth=1
while depth:
    depth+=(library[end]=='{')-(library[end]=='}'); end+=1
policy=library[start:end]
swift=r'''
import Foundation
enum MadeiraConfig {
    static var values:[String:String]=[:]
    static func get(_ key:String)->String? { values[key] }
}
func env(_ key:String)->String? { getenv(key).map { String(cString:$0) } }
'''+policy+r'''
unsetenv("MADEIRA_REAL_SUSPEND")
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:nil,realSuspend:nil)
assert(env("_MADEIRA_GAME_SWAP_MB")==nil && env("_MADEIRA_GAME_REAL_SUSPEND")==nil)
assert(env("MADEIRA_REAL_SUSPEND")==nil)
MadeiraConfig.values=["real-suspend":"0"]
setenv("MADEIRA_REAL_SUSPEND","1",1) // automatic REPLACED policy
GameMemorySettings.apply(swapMB:nil,realSuspend:nil)
assert(env("MADEIRA_REAL_SUSPEND")=="0")
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:4096,realSuspend:true)
assert(env("_MADEIRA_GAME_SWAP_MB")=="4096" && env("MADEIRA_REAL_SUSPEND")=="1")
assert(MadeiraConfig.values==["real-suspend":"0"])
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:nil,realSuspend:nil)
assert(env("_MADEIRA_GAME_SWAP_MB")==nil && env("MADEIRA_REAL_SUSPEND")=="0")
MadeiraConfig.values=["real-suspend":"1"]
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:0,realSuspend:false)
assert(env("_MADEIRA_GAME_SWAP_MB")=="0" && env("MADEIRA_REAL_SUSPEND")=="0")
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:nil,realSuspend:nil)
assert(env("MADEIRA_REAL_SUSPEND")=="1")
MadeiraConfig.values=[:]; unsetenv("MADEIRA_REAL_SUSPEND")
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:1024,realSuspend:true)
GameMemorySettings.clearPrevious()
assert(env("MADEIRA_REAL_SUSPEND")==nil)
GameMemorySettings.apply(swapMB:999999,realSuspend:nil)
assert(env("_MADEIRA_GAME_SWAP_MB")==nil)
MadeiraConfig.values=["env.MADEIRA_REAL_SUSPEND":"0"]
GameMemorySettings.clearPrevious()
GameMemorySettings.apply(swapMB:nil,realSuspend:nil)
assert(env("MADEIRA_REAL_SUSPEND")=="0")
assert(GameMemorySettings.swapChoices==[0,1024,2048,3072,4096])
print("PASS: production per-game Swift policy inherits shared values, restores game-to-game state, keeps explicit Off and preserves global config")
'''
c=r'''
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
static int setenv(const char *key,const char *value,int overwrite) {
    return overwrite || !getenv(key) ? _putenv_s(key,value) : 0;
}
static int unsetenv(const char *key) { return _putenv_s(key,""); }
#endif
#include "GameMemoryProfile.h"
int main(void) {
    unsetenv("_MADEIRA_GAME_SWAP_MB"); unsetenv("_MADEIRA_GAME_REAL_SUSPEND");
    assert(madeira_game_swap_capacity(1536)==1536);
    setenv("MADEIRA_SWAP_MB","1536",1); setenv("MADEIRA_SWAP_FILE","shared.bin",1);
    setenv("MADEIRA_REAL_SUSPEND","0",1); madeira_apply_game_memory_overrides();
    assert(!strcmp(getenv("MADEIRA_SWAP_FILE"),"shared.bin"));
    setenv("_MADEIRA_GAME_SWAP_MB","4096",1); setenv("_MADEIRA_GAME_REAL_SUSPEND","1",1);
    assert(madeira_game_swap_capacity(0)==4096);
    madeira_apply_game_memory_overrides();
    assert(!strcmp(getenv("MADEIRA_SWAP_MB"),"4096") && !strcmp(getenv("MADEIRA_REAL_SUSPEND"),"1"));
    setenv("MADEIRA_REAL_SUSPEND","0",1); setenv("MADEIRA_SWAP_MB","1536",1); // later global env exports
    madeira_apply_game_memory_overrides();
    assert(!strcmp(getenv("MADEIRA_SWAP_MB"),"4096") && !strcmp(getenv("MADEIRA_REAL_SUSPEND"),"1"));
    setenv("_MADEIRA_GAME_SWAP_MB","0",1); setenv("_MADEIRA_GAME_REAL_SUSPEND","0",1);
    madeira_apply_game_memory_overrides();
    assert(madeira_game_swap_capacity(4096)==0 && getenv("MADEIRA_SWAP_FILE")==NULL);
    assert(!strcmp(getenv("MADEIRA_REAL_SUSPEND"),"0"));
    const char *bad[]={"-1","99999999999999999999999999","1024x"," 1024","513",""};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);++i) {
        setenv("_MADEIRA_GAME_SWAP_MB",bad[i],1); assert(madeira_game_swap_capacity(2048)==2048);
    }
    unsetenv("_MADEIRA_GAME_SWAP_MB"); unsetenv("_MADEIRA_GAME_REAL_SUSPEND");
    assert(madeira_game_swap_capacity(1024)==1024);
    puts("PASS: native game memory overrides select actual swap capacity, survive config exports, disable file backing and reject malformed values");
}
'''
cc=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc: raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-game-memory-') as scratch:
    folder=Path(scratch); cf=folder/'test.c'; exe=folder/('test.exe' if os.name=='nt' else 'test')
    cf.write_text(c,encoding='utf-8')
    subprocess.run([cc,'-std=gnu11','-Werror','-O2','-I',str(root/'app/Madeira'),str(cf),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
    compiler=shutil.which('swiftc')
    if compiler:
        sf=folder/'main.swift'; sf.write_text(swift,encoding='utf-8')
        subprocess.run([compiler,str(sf),'-o',str(folder/'swift-test')],check=True,timeout=60)
        subprocess.run([str(folder/'swift-test')],check=True)
    elif '--require-swift' in sys.argv: raise SystemExit('Swift compiler required')
    else: print('SKIP: Swift runtime checks require the macOS CI runner')
