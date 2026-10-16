#!/usr/bin/env python3
"""Real SDL2 x86 reads the production transport, using a test-only XInput API.

Windows host test. Supply the official SDL2 2.32.10 x86 runtime DLL from
https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10 . Nothing is
installed in the app, game folder or Windows system directories.
"""
from pathlib import Path
import argparse, hashlib, json, os, re, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler');parser.add_argument('--sdl',type=Path);parser.add_argument('--receipt',type=Path)
args=parser.parse_args()
if os.name!='nt':
    print('SKIP: real SDL2 x86 integration requires a Windows host; Swift policy remains mandatory on Mac CI')
    raise SystemExit(0)
assert args.compiler and args.sdl,'Supply i686 llvm-mingw compiler and pinned official SDL2.dll'
blob=args.sdl.read_bytes()
assert hashlib.sha256(blob).hexdigest()=='68c78590d1997122c30c992ec79857d32e5e976a0f17c0a69a2e29eab0101d70','SDL canary DLL must match the pinned official runtime'
source=(root/'app/Madeira/GamepadInput.swift').read_text(encoding='utf-8')
policy=source.split('enum SDLGamepadPolicy {')[1].split('private static var applied:')[0]
defaults=dict(re.findall(r'\("(SDL_\w+)", "([01])"\)',policy));assert len(defaults)==7
driver=(root/'build/win32u-unix/driver_ios.c').read_text(encoding='utf-8')
query=driver[driver.index('/* ml1920: same-task controller snapshots'):]
query=query.replace('../../app/Madeira/Winios/WiniosGamepad.h','WiniosGamepad.h')
header='#include <windows.h>\n#include <xinput.h>\n#include <stdio.h>\n#define dprintf(fd,...) fprintf(stderr,__VA_ARGS__)\n'
definition='''LIBRARY XInput1_4
EXPORTS
 XInputGetState=XInputGetState@8 @2
 XInputSetState=XInputSetState@8 @3
 XInputGetCapabilities=XInputGetCapabilities@12 @4
 XInputGetStateEx=XInputGetStateEx@8 @100
 XInputGetCapabilitiesEx=XInputGetCapabilitiesEx@16 @108
 XInputGetBatteryInformation=XInputGetBatteryInformation@12 @7
 ProbeState
 ProbeCount
'''
results=[]
with tempfile.TemporaryDirectory(prefix='madeira-sdl-xinput-') as scratch:
    p=Path(scratch);(p/'SDL2.dll').write_bytes(blob)
    (p/'bridge.c').write_text(header+query+(root/'tests/host/sdl-xinput-fixture.c').read_text(encoding='utf-8'),encoding='utf-8')
    (p/'bridge.def').write_text(definition,encoding='utf-8')
    subprocess.run([args.compiler,'-O1','-shared','-static','-pthread','-I'+str(root/'app/Madeira/Winios'),
        str(p/'bridge.c'),str(root/'app/Madeira/Winios/WiniosGamepad.c'),str(p/'bridge.def'),'-o',str(p/'XInput1_4.dll')],check=True)
    subprocess.run([args.compiler,'-O1',str(root/'tests/host/sdl-xinput-probe.c'),'-o',str(p/'probe.exe')],check=True)
    for mode in (1,2,0):
        env=dict(os.environ,**defaults)
        if mode==1:env['SDL_JOYSTICK_RAWINPUT']='1'
        if mode==2:env['SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS']='0'
        result=subprocess.run([str(p/'probe.exe'),str(mode)],cwd=p,env=env,capture_output=True,text=True,timeout=15)
        if result.returncode:raise AssertionError((mode,result.returncode,result.stdout,result.stderr))
        print(result.stdout.strip());results.append({'mode':mode,'result':result.stdout.strip()})
if args.receipt:
    inputs=['app/Madeira/GamepadInput.swift','app/Madeira/Winios/WiniosGamepad.c',
            'build/win32u-unix/driver_ios.c','tests/host/sdl-xinput-fixture.c','tests/host/sdl-xinput-probe.c']
    record={'sdl_sha256':hashlib.sha256(blob).hexdigest(),'cases':results,
            'inputs':{name:hashlib.sha256((root/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for name in inputs},
            'scope':'Windows x86 SDL and production host snapshot/query only; no iOS GameController, Mach/FEX or Celeste execution.'}
    args.receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
