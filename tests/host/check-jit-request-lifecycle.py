#!/usr/bin/env python3
"""Production request identities across retry/relaunch and debugger readiness."""
from pathlib import Path
import shutil, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
app=root/'app/Madeira'
source=(app/'JITBuiltInMessages.swift').read_text(encoding='utf8')
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:raise SystemExit('Swift is required')
    print('SKIP: JIT lifecycle execution needs Swift on Mac CI');raise SystemExit(0)
fixture=r'''
var gate=JITRequestGate()
let first=gate.begin(pid:100,now:0,timeout:60)
assert(gate.accepts(first) && gate.accepts(callback:first.callbackValue))
assert(!gate.accepts(callback:"") && !gate.expired(now:59.9))
assert(gate.expired(now:60))
assert(gate.finish(first) && !gate.finish(first))
let second=gate.begin(pid:100,now:70,timeout:60)
assert(!gate.accepts(first) && !gate.accepts(callback:first.callbackValue))
assert(!gate.finish(first) && gate.accepts(second))
var relaunched=JITRequestGate()
let reusedPID=relaunched.begin(pid:100,now:70,timeout:60)
assert(!relaunched.accepts(first) && !relaunched.accepts(second))
assert(!relaunched.accepts(callback:second.callbackValue))
assert(relaunched.accepts(reusedPID))
let newPID=relaunched.begin(pid:200,now:80,timeout:60)
assert(!relaunched.accepts(reusedPID) && newPID.pid==200)
for flagged in [false,true] {
 for attached in [false,true] {
   assert(JITRequestGate.debuggerReady(flagged:flagged,attached:attached)==(flagged && attached))
 }
}
// A saved pool/CS_DEBUGGED flag alone cannot finish a new enable request.
assert(!JITRequestGate.debuggerReady(flagged:true,attached:false))
print("PASS: JIT request retry/relaunch/PID-reuse isolation, exactly-once completion, deadlines and live debugger readiness")
'''
with tempfile.TemporaryDirectory(prefix='madeira-jit-lifecycle-') as work:
    p=Path(work);f=p/'main.swift';f.write_text(source+'\n'+fixture,encoding='utf8')
    subprocess.run([swift,str(f),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True)
net=(app/'JITNetwork.swift').read_text(encoding='utf8')
setup=(app/'JITSetup.swift').read_text(encoding='utf8')
assert '"?request=" + identity.callbackValue' in net
assert 'callbackGate.accepts(callback: value("request"))' in net
assert 'guard !checkedStartupRecovery else { return }' in net
assert 'afterStartupRecovery' in setup and 'timeout: 180' in setup
assert 'if enableGate.current != nil { enableWaiters.append(completion); return }' in setup
assert 'guard enableGate.finish(identity) else { return }' in setup
print('PASS: shortcut callbacks and coordinator completion are bound to the active process request')
