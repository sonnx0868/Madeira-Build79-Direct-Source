#!/usr/bin/env python3
"""The actual library session timer must run during UIKit tracking modes."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
begin = source.index('func begin(_ entry: LibraryEntry')
start = source.index('let value = Timer(timeInterval: 0.5', begin)
end = source.index('timer = value', start) + len('timer = value')
timer = source[start:end]
swift = shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:
        raise SystemExit('swiftc required for library polling behavior')
    print('SKIP: session polling behavior requires Swift; Mac CI requires it')
    raise SystemExit(0)
fixture = '''import Foundation
import CoreFoundation
final class SessionPollFixture {
    var timer: Timer?
    var polls = 0
    func poll() { polls += 1 }
    func begin() {
''' + timer + '''
    }
}
let tracking = "madeira-fixture-tracking"
let mode = CFStringCreateWithCString(nil, tracking, CFStringBuiltInEncodings.UTF8.rawValue)!
CFRunLoopAddCommonMode(CFRunLoopGetMain(), mode)
let session = SessionPollFixture()
session.begin()
let limit = Date().addingTimeInterval(2)
while session.polls == 0 && Date() < limit {
    RunLoop.main.run(mode: RunLoop.Mode(tracking), before: Date().addingTimeInterval(0.1))
}
assert(session.polls > 0, "game exit polling stopped during tracking mode")
session.timer?.invalidate()
print("PASS: production library timer polls game exit in common tracking modes")
'''
with tempfile.TemporaryDirectory(prefix='madeira-session-poll-') as scratch:
    folder = Path(scratch)
    path = folder / 'main.swift'
    path.write_text(fixture, encoding='utf-8')
    subprocess.run([swift, str(path), '-o', str(folder / 'check')], check=True)
    subprocess.run([str(folder / 'check')], check=True, timeout=8)
