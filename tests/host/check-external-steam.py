#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 125hz
# Madeira Converter Exception: see LICENSE-EXCEPTION.md
"""Test only the external process adapter with our own fixture EXEs.

No Steam login, client DLL, game or credential is loaded. Authentication and
subscription checks remain in the existing session path, before this adapter.
"""
from pathlib import Path
import os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
session=(root/'madeira-dock/src/session.c').read_text(encoding='utf-8')
launch=(root/'madeira-dock/src/launch.c').read_text(encoding='utf-8')
assert session.count('result = sh_launch(')==1
assert session.index('result = listed ? 0 : 35;') < session.index('result = sh_launch(')
assert session.index('session-native-handoff-app-mismatch') < session.index('result = sh_launch(')
assert launch.index('launch-client-discovery-published') < launch.index('result = sh_external_launch(')
assert 'if (!external && wide_flag(L"MADEIRA_STEAM_HOST_CEG"' in launch
assert 'sh_external_launch' not in session
print('PASS: external adapter follows existing authentication, App ID binding, subscription and discovery gates')
if os.name!='nt':
    print('SKIP: real Windows process/job adapter needs a Windows host');raise SystemExit(0)
cc=os.environ.get('CC') or shutil.which('clang')
if not cc: raise SystemExit('C compiler required')
child=r'''
#include <windows.h>
#include <assert.h>
#include <wchar.h>
int wmain(int argc,wchar_t **argv) {
    wchar_t value[32768],dir[32768];
    assert(GetEnvironmentVariableW(L"SteamAppId",value,32768)&&!wcscmp(value,L"10"));
    assert(GetEnvironmentVariableW(L"SteamGameId",value,32768)&&!wcscmp(value,L"10"));
    assert(GetCurrentDirectoryW(32768,dir));
    assert(GetEnvironmentVariableW(L"SteamAppPath",value,32768)&&!wcscmp(value,dir));
    assert(!GetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_ARGUMENTS",value,32768));
    assert(!GetEnvironmentVariableW(L"MADEIRA_DOCK_AUTH_FILE",value,32768));
    if(argc==2&&!wcscmp(argv[1],L"fail"))return 13;
    if(argc==2&&!wcscmp(argv[1],L"descendant")){Sleep(250);return 0;}
    assert(argc==4&&!wcscmp(argv[1],L"two words")&&!wcscmp(argv[2],L"quote\"value")&&!wcscmp(argv[3],L"\u65e5"));
    wchar_t exe[32768],cmd[32768];assert(GetModuleFileNameW(NULL,exe,32768));
    _snwprintf(cmd,32768,L"\"%ls\" descendant",exe);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    assert(CreateProcessW(exe,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi));
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return 0;
}
'''
adapter=r'''
#include <assert.h>
#include <stdio.h>
#include "external.c"
static int started,exit_status;
static bool next(int32_t pipe,struct sh_callback *cb){(void)pipe;(void)cb;return false;}
static void release(int32_t pipe){(void)pipe;}
static uint64_t now(void){return GetTickCount64();}
static void sleep_ms(uint32_t ms){Sleep(ms);}
static void event(const char *key,int32_t value){if(!strcmp(key,"external-process-started"))started=value;if(!strcmp(key,"external-process-exit"))exit_status=value;}
int wmain(int argc,wchar_t **argv) {
    assert(argc==2);assert(SetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_PROGRAM",argv[1]));
    assert(SetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_EXTERNAL",L"1"));
    assert(SetEnvironmentVariableW(L"MADEIRA_DOCK_AUTH_FILE",L"synthetic-consumed-file"));
    assert(SetEnvironmentVariableW(L"SteamAppId",L"999"));
    struct sh_api api={.get_callback=next,.free_callback=release};
    struct sh_observer observer={now,sleep_ms,event};
    assert(sh_external_requested());
    assert(SetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_ARGUMENTS",L"\"two words\" \"quote\\\"value\" \u65e5"));
    assert(sh_external_validate());
    uint64_t begin=now();assert(sh_external_launch(&api,&observer,1,10)==0);
    assert(started&&exit_status==0&&now()-begin>=200);
    wchar_t original[32];assert(GetEnvironmentVariableW(L"SteamAppId",original,32)&&!wcscmp(original,L"999"));
    assert(SetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_ARGUMENTS",L"fail"));
    assert(sh_external_launch(&api,&observer,1,10)==53&&exit_status==13);
    assert(SetEnvironmentVariableW(L"MADEIRA_STEAM_HOST_PROGRAM",L"C:\\missing.exe"));
    assert(!sh_external_validate()&&sh_external_launch(&api,&observer,1,10)==50);
    puts("PASS: real Windows EXE, Unicode/quoted arguments, exact working directory, child-only Steam identity, descendant job lifetime and exit failures");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-steam-process-', dir=Path(os.environ['LOCALAPPDATA'])/'Temp') as temporary:
    folder=Path(temporary)/'Imported Title';folder.mkdir()
    assert str(folder).lower().startswith('c:'),'Fixture needs C: to match the app drive contract'
    for name,code in [('child',child),('adapter',adapter)]:
        c=folder/(name+'.c');binary=folder/(name+'.exe');c.write_text(code,encoding='utf-8')
        subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Werror','-municode','-static','-I',str(root/'madeira-dock/src'),str(c),'-o',str(binary)],check=True)
    subprocess.run([str(folder/'adapter.exe'),str(folder/'child.exe')],check=True,timeout=20)
