#!/usr/bin/env python3
"""Exercise actual SDL controller profile precedence and game-to-game cleanup."""
from pathlib import Path
import shutil, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'app/Madeira/GamepadInput.swift').read_text(encoding='utf-8')
policy=source[source.index('enum SDLGamepadPolicy {'):source.index('/// iOS 18 otherwise')]
library=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'detectedSDL2: runtime.hasSDL2' in library and 'names.contains("sdl2.dll")' in library
assert 'sdl-xinput-v1 %s=%s' in (root/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
assert source.index('diagnosticTransitions = 0',source.index('func reserveSessionSlot')) < source.index('sample()',source.index('func reserveSessionSlot'))
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:raise SystemExit('Swift is required')
    print('SKIP: production SDL profile fixtures require Swift on Mac CI');raise SystemExit(0)
fixture='''import Foundation
enum MadeiraConfig {
 static var values:[String:String]=[:]
 static func get(_ key:String)->String? { values[key] }
}
func env(_ key:String)->String? { getenv(key).map { String(cString:$0) } }
'''+policy+'''
for (name,_) in SDLGamepadPolicy.defaults { unsetenv(name) }
unsetenv("MADEIRA_NATIVE_XINPUT")
assert(!SDLGamepadPolicy.apply(detectedSDL2:false,hostXInput:true))
assert(SDLGamepadPolicy.defaults.allSatisfy { env($0.0)==nil })
assert(!SDLGamepadPolicy.apply(detectedSDL2:true,hostXInput:false))
assert(SDLGamepadPolicy.apply(detectedSDL2:true,hostXInput:true))
for (name,value) in SDLGamepadPolicy.defaults { assert(env(name)==value) }
assert(!SDLGamepadPolicy.apply(detectedSDL2:false,hostXInput:true))
assert(SDLGamepadPolicy.defaults.allSatisfy { env($0.0)==nil })
setenv("SDL_JOYSTICK_RAWINPUT","1",1)
MadeiraConfig.values=["env.SDL_XINPUT_ENABLED":"0","env.SDL_DIRECTINPUT_ENABLED":"1"]
assert(SDLGamepadPolicy.apply(detectedSDL2:true,hostXInput:true))
assert(env("SDL_XINPUT_ENABLED")=="0" && env("SDL_DIRECTINPUT_ENABLED")=="1" && env("SDL_JOYSTICK_RAWINPUT")=="1")
SDLGamepadPolicy.clearPrevious()
assert(env("SDL_XINPUT_ENABLED")=="0" && env("SDL_DIRECTINPUT_ENABLED")=="1" && env("SDL_JOYSTICK_RAWINPUT")=="1")
for (name,_) in SDLGamepadPolicy.defaults { unsetenv(name) }
MadeiraConfig.values=["env.MADEIRA_NATIVE_XINPUT":"1"]
assert(!SDLGamepadPolicy.apply(detectedSDL2:true,hostXInput:true))
assert(SDLGamepadPolicy.defaults.allSatisfy { env($0.0)==nil })
MadeiraConfig.values=[:]
assert(SDLGamepadPolicy.apply(detectedSDL2:true,hostXInput:true))
setenv("SDL_JOYSTICK_HIDAPI","1",1) // a later explicit override is retained
SDLGamepadPolicy.clearPrevious()
assert(env("SDL_JOYSTICK_HIDAPI")=="1" && env("SDL_XINPUT_ENABLED")==nil)
print("PASS: actual SDL2 host XInput policy, explicit config/environment precedence, native-XInput opt-out and next-game cleanup")
'''
with tempfile.TemporaryDirectory(prefix='madeira-sdl-gamepad-') as scratch:
    p=Path(scratch);(p/'main.swift').write_text(fixture,encoding='utf-8')
    subprocess.run([swift,str(p/'main.swift'),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True)
