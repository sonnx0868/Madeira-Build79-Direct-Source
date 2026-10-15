#!/usr/bin/env python3
"""Exercise actual detach code with an observable trap and attachment probe."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
swift = shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:
        raise SystemExit('Swift compiler required')
    print('SKIP: production Swift detach fixture requires Mac CI')
    raise SystemExit(0)

def function(source, signature):
    start=source.index(signature);brace=source.index('{',start);end=brace+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

ent=(root/'app/Madeira/EntitlementChecker.swift').read_text(encoding='utf-8')
helper=(root/'app/Madeira/StikJITHelper.swift').read_text(encoding='utf-8')
probe=function(ent,'func debuggerAttachment()')
probe=probe.replace('sysctl(', 'test_sysctl(').replace('getpid()', 'test_getpid()')
enum=ent[ent.index('enum DebuggerAttachment {'):ent.index('\n}',ent.index('enum DebuggerAttachment {'))+2]
body=function(helper,'static func detachDebugger()')
fixture=r'''
import Foundation
import Dispatch
struct Proc { var p_flag: Int32 = 0 }
struct kinfo_proc { var kp_proc = Proc() }
let CTL_KERN: Int32 = 1, KERN_PROC: Int32 = 2, KERN_PROC_PID: Int32 = 3, P_TRACED: Int32 = 0x800
var probeFails=false, probeShort=false, live=false
var trapCount=0, trapDetaches=true, probeFailsAfterTrap=false
func test_getpid() -> Int32 { 42 }
func test_sysctl(_ mib: inout [Int32],_ count: UInt32,_ info: inout kinfo_proc,
                _ size: inout Int,_ new: UnsafeRawPointer?,_ n: Int) -> Int32 {
    assert(mib == [CTL_KERN,KERN_PROC,KERN_PROC_PID,42] && count==4)
    info.kp_proc.p_flag = live ? P_TRACED : 0
    if probeShort {size=0}
    return probeFails ? -1 : 0
}
func jit26_detach() {
    assert(live, "attempted an unhandled breakpoint without a debugger")
    trapCount+=1
    Thread.sleep(forTimeInterval:0.002)
    if trapDetaches {live=false}
    if probeFailsAfterTrap {probeFails=true}
}
enum Level {case info,error,success}
final class LogStore {
    static let shared=LogStore()
    var messages:[String]=[]
    func log(_ text:String,level:Level = .info) {messages.append(text)}
}
'''+enum+'\n'+probe+'\n'+function(ent,'func isDebuggerAttached()')+r'''
enum StikJITHelper {
    private static let detachLock=NSLock()
'''+body+r'''
}
func flag() -> String? {getenv("MADEIRA_DETACHED").map {String(cString:$0)}}
unsetenv("MADEIRA_DETACHED")
// Sticky CS_DEBUGGED must not matter: there is no live debugger here.
assert(debuggerAttachment() == .detached && !isDebuggerAttached())
StikJITHelper.detachDebugger();assert(trapCount==0 && flag()=="1")
live=true
StikJITHelper.detachDebugger();assert(trapCount==1 && !live && flag()=="1")
StikJITHelper.detachDebugger();assert(trapCount==1) // late call after early detach
// A stale environment flag must not hide a genuine reattachment.
live=true
StikJITHelper.detachDebugger();assert(trapCount==2 && !live)
// Unknown/truncated attachment state cannot authorize a breakpoint.
unsetenv("MADEIRA_DETACHED");probeFails=true;live=true
StikJITHelper.detachDebugger();assert(trapCount==2 && flag()==nil)
assert(debuggerAttachment() == .unavailable && !isDebuggerAttached())
probeFails=false;probeShort=true
StikJITHelper.detachDebugger();assert(trapCount==2 && flag()==nil)
probeShort=false;trapDetaches=false
StikJITHelper.detachDebugger();assert(trapCount==3 && live && flag()==nil)
// Concurrent late callers must serialize the probe and request as one action.
trapDetaches=true;live=true
DispatchQueue.concurrentPerform(iterations:64) {_ in StikJITHelper.detachDebugger()}
assert(trapCount==4 && !live && flag()=="1")
// An unconfirmed request never publishes a detached state to native waiters.
live=true;probeFailsAfterTrap=true
StikJITHelper.detachDebugger();assert(trapCount==5 && flag()==nil)
print("PASS: actual detach/probe skips absent, unknown and duplicate breakpoints; reattachment, confirmation and 64 concurrent callers")
'''
with tempfile.TemporaryDirectory(prefix='madeira-jit-detach-') as scratch:
    folder=Path(scratch);source=folder/'main.swift';exe=folder/'check'
    source.write_text(fixture,encoding='utf-8')
    subprocess.run([swift,'-swift-version','5',str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True,timeout=10)
