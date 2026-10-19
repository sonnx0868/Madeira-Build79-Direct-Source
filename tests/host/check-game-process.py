#!/usr/bin/env python3
"""Exercise actual selected-process tracking before rendering and after child exit."""
from pathlib import Path
import os, shutil, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
bridge=(root/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
state=bridge[bridge.index('static pthread_mutex_t g_game_path_lock'):bridge.index('void wine_presented_process_track(')]
at=bridge.index('void wine_session_process_did_exit(')
retire=bridge[at:bridge.index('\nint wine_session_close_has_exited',at)]
source=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'wine_game_process_has_exited() != 0' in source
assert 'wine_game_process_exit_status(&value)' in source
assert 'LogStore.shared.finishGameLog()' in source
assert 'wine_game_process_started(reply->pid' in (root/'build/ntdll-unix/process_ios.c').read_text(encoding='utf-8')
if sys.platform!='darwin':
    if '--require-objc' in sys.argv: raise SystemExit('Mac Objective-C required')
    print('SKIP: process tracking behavior requires Mac Foundation');raise SystemExit(0)
fixture=r'''
#import <Foundation/Foundation.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "SessionPriorityAPI.h"
#include "SessionPriority.h"
static uint32_t g_presented_pid;
static uint64_t g_presented_exit;
static int g_session_close_exited;
static int wine_session_close_targets_process(uint32_t pid) {(void)pid;return 0;}
'''+state+retire+r'''
static void started(uint32_t pid,NSString *path) {
    unichar text[1024];[path getCharacters:text range:NSMakeRange(0,path.length)];
    wine_game_process_started(pid,text,path.length);
}
int main(void) { @autoreleasepool {
    wine_game_process_expect("C:\\Steam\\steamapps\\common\\Party Animals\\PartyAnimals.exe",0);
    started(32,@"C:\\Steam\\steam.exe");assert(!g_game_pid);
    started(44,@"C:\\Steam\\steamapps\\common\\Party Animals\\crashhandler.exe");assert(!g_game_pid);
    started(52,@"\\??\\C:\\STEAM\\steamapps\\common\\Party Animals\\PartyAnimals.EXE");assert(g_game_pid==52);
    assert(msp_session.target==52 && !msp_session.target_peb); // parent announcement
    madeira_session_priority_thread(52,52,0,(void *)0x1000);
    started(52,@"C:\\Steam\\steamapps\\common\\Party Animals\\PartyAnimals.exe");
    assert(msp_session.target_peb==0x1000); // child binds itself even after the first CAS
    wine_session_process_did_exit(32,(int)0xc000001du);assert(!wine_game_process_has_exited());
    wine_session_process_did_exit(52,(int)0xc0000005u);uint32_t status;
    assert(wine_game_process_has_exited() && wine_game_process_exit_status(&status) && status==0xc0000005u);
    g_game_pid=0;g_game_exit=0;
    wine_game_process_expect("C:/games/Primal Planet/",1);
    started(60,@"C:\\games\\Primal Planet Other\\game.exe");assert(!g_game_pid);
    started(64,@"C:\\games\\Primal Planet\\shader.dll");assert(!g_game_pid);
    started(68,@"C:\\games\\Primal Planet\\PrimalPlanet.exe");assert(g_game_pid==68);
    wine_session_process_did_exit(68,0);assert(wine_game_process_has_exited());
    assert(wine_game_process_exit_status(&status) && status==0);
    puts("PASS: selected program identity, pre-frame tracking, helpers ignored, directory boundaries and normal/crash exits");
} }
'''
with tempfile.TemporaryDirectory(prefix='madeira-game-process-') as tmp:
    p=Path(tmp);(p/'check.m').write_text(fixture,encoding='utf-8')
    subprocess.run(['clang','-fobjc-arc','-fblocks','-I'+str(root/'app/Madeira'),str(p/'check.m'),'-framework','Foundation','-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True)
