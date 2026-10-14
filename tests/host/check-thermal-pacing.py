#!/usr/bin/env python3
"""Run production thermal caps and immediate restoration at nominal."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'app/Madeira/FPSOverlay.swift').read_text(encoding='utf-8')
policy=source[source.index('struct ThermalPacingPolicy {'):source.index('/// Receives user intent')]
assert 'ProcessInfo.thermalStateDidChangeNotification' in source
assert 'ProMotionIntent.shared.setActive(false)' in source
library=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'ThermalPacingController.request(effectiveFPSMode)' in library
assert 'ThermalPacingController.request(entry.effectiveFPSMode)' in library
assert 'ThermalPacingController.request(applied)' in library
gamepad=(root/'app/Madeira/GamepadInput.swift').read_text(encoding='utf-8')
assert 'profile.valueChangedHandler =' in gamepad and '.milliseconds(pollingMilliseconds)' in gamepad
assert 'level >= 2 ? 16 : level == 1 ? 8 : 4' in gamepad
fixture=r'''
var policy=ThermalPacingPolicy()
func mode(_ requested:Int32=1,_ thermal:Int=0,_ now:Double=0,
          lowPower:Bool=false,enabled:Bool=true,supported:Bool=true)->Int32 {
    policy.mode(requested:requested,thermal:thermal,lowPower:lowPower,enabled:enabled,has30Cap:supported,now:now)
}
assert(mode(0)==0 && mode(1)==1 && mode(2)==2 && mode(3)==3)
assert(mode(0,1)==1 && mode(2,1)==1 && mode(3,1)==3)
assert(mode(1,2,10)==1) // cooling preserves the chosen 60 FPS cap
assert(mode(2,0,11)==2) // nominal restores RAW immediately
assert(mode(2,0,70)==2)
assert(mode(2,0,71)==2)
assert(mode(0,3,100)==1)
assert(mode(0,0,101)==0)
assert(mode(0,1,140)==1) // fair caps MAX at 60, not the previous hot 30
assert(mode(0,0,141)==0 && mode(0,0,200)==0)
assert(mode(0,0,201)==0)
assert(mode(3,2,220)==3 && mode(3,0,230)==3 && mode(3,0,290)==3)
assert(mode(2,2,300,enabled:false)==2) // opt-out is immediate
assert(mode(2,0,301)==2)
assert(mode(0,0,310,lowPower:true)==1)
assert(mode(1,0,310,lowPower:true)==1)
assert(mode(3,0,310,lowPower:true)==3) // explicit user 30 is preserved
assert(mode(0,0,311)==0)
policy=ThermalPacingPolicy()
assert(mode(0,2,0,supported:false)==1) // mode 3 must never become an uncapped fallback
assert(mode(3,0,60,enabled:false,supported:false)==1)
assert(mode(1,2,400)==1 && mode(1,0,401)==1)
print("PASS: thermal FPS caps, immediate nominal restoration, low power preserves 60 FPS, explicit user 30, opt-out and unsupported backend")
'''
compiler=shutil.which('swiftc')
if not compiler:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required')
    print('PASS: thermal pacing wiring; Swift behavior runs on macOS CI')
    raise SystemExit(0)
with tempfile.TemporaryDirectory(prefix='madeira-thermal-') as scratch:
    folder=Path(scratch); swift=folder/'main.swift'; binary=folder/'check'
    swift.write_text('import Foundation\n'+policy+fixture,encoding='utf-8')
    subprocess.run([compiler,str(swift),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
