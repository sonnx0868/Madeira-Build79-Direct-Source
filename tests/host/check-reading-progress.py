#!/usr/bin/env python3
"""Exercise the shipped bridge on Python 2/3 and the actual Swift file policy.

No game saves are deserialized. --renpy-python accepts a bundled Ren'Py Python
interpreter for compatibility checks. --require-swift is the Mac CI gate.
"""
import argparse
import ast
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BRIDGE = ROOT / 'app/Madeira/MadeiraReadingProgress.rpy'
args = argparse.ArgumentParser()
args.add_argument('--renpy-python')
args.add_argument('--require-swift', action='store_true')
options = args.parse_args()

# Kept Python 2.7 compatible, including the actual `python hide` scope rules.
fixture = r'''
from __future__ import print_function
import sys, os, json, time, tempfile, shutil, types
try:
    import __builtin__ as builtins
except ImportError:
    import builtins
source = open(sys.argv[1], 'rb').read().decode('utf-8')
block = source.split('init 999 python hide:\n', 1)[1]
code = compile('\n'.join(line[4:] if line.startswith('    ') else line for line in block.splitlines()), 'bridge.rpy', 'exec')
tmp = tempfile.mkdtemp(prefix='madeira-reading-')
original_open, original_time = builtins.open, time.time
clock = [100.0]
time.time = lambda: clock[0]
writes = [0]
def traced_open(path, mode='r', *a, **kw):
    if 'w' in mode and str(path).endswith('.madeira-reading-progress-v1.json'):
        writes[0] += 1
    return original_open(path, mode, *a, **kw)
builtins.open = traced_open
try:
    module = types.ModuleType('renpy')
    config = types.ModuleType('config')
    config.gamedir = tmp
    config.interact_callbacks = []
    module.config = config
    counts, reads = [3, 20], [0, 0]
    def total():
        reads[0] += 1
        return counts[1]
    def seen():
        reads[1] += 1
        return counts[0]
    module.count_dialogue_blocks = total
    module.count_seen_dialogue_blocks = seen
    sys.modules['renpy'] = module
    control = os.path.join(tmp, '.madeira-reading-session-v1.json')
    output = os.path.join(tmp, '.madeira-reading-progress-v1.json')
    token = '00000000-0000-0000-0000-000000000001'
    def settings(enabled=True, session=token):
        with original_open(control, 'wb') as f:
            f.write(json.dumps(dict(schema=1, enabled=enabled, session=session)).encode('ascii'))
    globals_scope = {'__builtins__': builtins.__dict__}
    def install():
        # Ren'Py hide executes with distinct global and temporary local dicts.
        eval(code, globals_scope, {})
    install()
    assert config.interact_callbacks == [], 'missing session must be inert'
    settings(False)
    install()
    assert len(config.interact_callbacks) == 1
    sample = config.interact_callbacks[0]
    sample()
    assert writes[0] == 0 and reads == [0, 0]
    settings(True)
    clock[0] += 2
    sample()
    assert reads == [1, 1] and writes[0] == 1
    assert json.load(original_open(output)) == dict(schema=1, session=token, seen=3, total=20)
    for _ in range(100000): sample()
    assert reads == [1, 1] and writes[0] == 1, 'skip/auto/read repeats must be throttled'
    clock[0] += 2
    sample()
    assert reads == [1, 2] and writes[0] == 1, 'unchanged counts must not write'
    counts[0] = 4
    clock[0] += 2
    sample()
    assert writes[0] == 2 and json.load(original_open(output))['seen'] == 4
    settings(False)
    counts[0] = 8
    clock[0] += 2
    sample()
    assert writes[0] == 2
    settings(True)
    clock[0] += 2
    sample()
    assert writes[0] == 3 and json.load(original_open(output))['seen'] == 8
    # Old script counts stay explicit; the native model rejects the ratio.
    counts[0] = 21
    clock[0] += 2
    sample()
    assert json.load(original_open(output))['seen'] == 21
    settings(True, '00000000-0000-0000-0000-000000000002')
    clock[0] += 2
    sample()
    before = writes[0]
    settings(True)
    clock[0] += 2
    sample()
    assert writes[0] == before, 'old process must not publish into a new session'
    assert list(globals_scope) == ['__builtins__'], 'helper polluted game stores'
    # An engine API throwing must not crash the VN or retry every interaction.
    config.interact_callbacks = []
    def broken(): raise RuntimeError('fixture failure')
    module.count_seen_dialogue_blocks = broken
    install()
    config.interact_callbacks[0]()
    clock[0] += 10
    config.interact_callbacks[0]()
    assert writes[0] == before
    config.interact_callbacks = []
    module.count_seen_dialogue_blocks = None
    install()
    assert config.interact_callbacks == []
    print('PASS Python %s: 100000 callbacks throttled; counts-only writes, enable/resume, session isolation, missing API and error containment' % sys.version.split()[0])
finally:
    builtins.open, time.time = original_open, original_time
    shutil.rmtree(tmp)
'''

swift_fixture = r'''
let fm = FileManager.default
let root = fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try fm.createDirectory(at: root, withIntermediateDirectories: true)
defer { try? fm.removeItem(at: root) }
let gameRoot = root.appendingPathComponent("VN"), game = gameRoot.appendingPathComponent("game")
try fm.createDirectory(at: game, withIntermediateDirectories: true)
try fm.createDirectory(at: gameRoot.appendingPathComponent("renpy"), withIntermediateDirectories: true)
let exe = gameRoot.appendingPathComponent("VN.exe")
assert(ReadingProgressFiles.gameFolder(location: exe, drive: root) == game)
assert(ReadingProgressFiles.gameFolder(location: exe, drive: gameRoot) == nil)
let nested = gameRoot.appendingPathComponent("lib/windows-x86_64/VN.exe")
assert(ReadingProgressFiles.gameFolder(location: nested, drive: root) == game)
let source = game.appendingPathComponent(ReadingProgressFiles.script)
let compiled = game.appendingPathComponent(ReadingProgressFiles.script + "c")
let save = game.appendingPathComponent("persistent")
try Data("preserve-save".utf8).write(to: save)
let token = UUID().uuidString
let bridge = ReadingProgressFiles.marker + Data("\ninit python:\n    pass\n".utf8)
try ReadingProgressFiles.prepare(folder: game, enabled: false, bridge: nil, session: token)
assert(!fm.fileExists(atPath: source.path))
try Data("foreign script".utf8).write(to: source)
do { try ReadingProgressFiles.prepare(folder: game, enabled: true, bridge: bridge, session: token); fatalError("overwrote foreign file") }
catch ReadingProgressFiles.Failure.conflictingFile { }
let foreign = try String(contentsOf: source)
assert(foreign == "foreign script")
try fm.removeItem(at: source)
try ReadingProgressFiles.prepare(folder: game, enabled: true, bridge: bridge, session: token)
try Data("compiled".utf8).write(to: compiled)
try ReadingProgressFiles.prepare(folder: game, enabled: true, bridge: bridge, session: UUID().uuidString)
assert(fm.fileExists(atPath: compiled.path), "unchanged helper must keep its compile cache")
let changed = bridge + Data("# update\n".utf8)
try ReadingProgressFiles.prepare(folder: game, enabled: true, bridge: changed, session: token)
assert(!fm.fileExists(atPath: compiled.path))
func payload(_ seen: Int, _ total: Int, _ session: String = token) -> Data {
    try! JSONSerialization.data(withJSONObject: ["schema": 1, "session": session, "seen": seen, "total": total])
}
let value = ReadingProgressSnapshot.decode(payload(3, 20), session: token)!
assert(value.percentage == 15)
assert(ReadingProgressSnapshot.decode(payload(20, 20), session: token)!.percentage == 100)
assert(ReadingProgressSnapshot.decode(payload(21, 20), session: token)!.percentage == nil)
assert(ReadingProgressSnapshot.decode(payload(0, 0), session: token)!.percentage == nil)
assert(ReadingProgressSnapshot.decode(payload(-1, 20), session: token) == nil)
assert(ReadingProgressSnapshot.decode(payload(3, 20, "stale"), session: token) == nil)
assert(ReadingProgressSnapshot.decode(Data("{\"seen\":".utf8), session: token) == nil)
let output = game.appendingPathComponent(ReadingProgressFiles.output)
try Data(repeating: 0, count: 1025).write(to: output)
assert(ReadingProgressFiles.boundedRead(output) == nil)
try fm.removeItem(at: output)
try fm.createSymbolicLink(at: output, withDestinationURL: save)
do { try ReadingProgressFiles.prepare(folder: game, enabled: true, bridge: changed, session: token); fatalError("followed output symlink") }
catch ReadingProgressFiles.Failure.unsafePath { }
try fm.removeItem(at: output)
try ReadingProgressFiles.prepare(folder: game, enabled: false, bridge: nil, session: token)
assert(!fm.fileExists(atPath: source.path))
let preserved = try String(contentsOf: save)
assert(preserved == "preserve-save")
assert(ReadingProgressPosition(x: .nan, y: 7).bounded == ReadingProgressPosition(x: 0.90, y: 1))
print("PASS Swift: percentage validity, bounded reads, old-session rejection, safe install/update/remove, symlink refusal and save preservation")
'''

with tempfile.TemporaryDirectory(prefix='madeira-reading-check-') as temp:
    temp = Path(temp)
    script = temp / 'bridge_check.py'
    # Ren'Py's bundled Python starts with -OO. Keep checks active there.
    class KeepChecks(ast.NodeTransformer):
        def visit_Assert(self, node):
            return ast.If(test=ast.UnaryOp(op=ast.Not(), operand=node.test),
                          body=[ast.Raise(exc=ast.Call(func=ast.Name(id='AssertionError', ctx=ast.Load()),
                                args=[node.msg or ast.Constant('check failed')], keywords=[]))], orelse=[])
    fixture = ast.unparse(ast.fix_missing_locations(KeepChecks().visit(ast.parse(fixture))))
    script.write_text(fixture, encoding='utf-8')
    subprocess.run([sys.executable, str(script), str(BRIDGE)], check=True, timeout=30)
    if options.renpy_python:
        subprocess.run([options.renpy_python, str(script), str(BRIDGE)], check=True, timeout=30)
    compiler = os.environ.get('SWIFTC') or shutil.which('swiftc')
    if compiler:
        main = temp / 'main.swift'
        main.write_text((ROOT / 'app/Madeira/ReadingProgressData.swift').read_text(encoding='utf-8') + '\n' + swift_fixture, encoding='utf-8')
        binary = temp / ('check.exe' if os.name == 'nt' else 'check')
        subprocess.run([compiler, str(main), '-o', str(binary)], check=True, timeout=90)
        subprocess.run([str(binary)], check=True, timeout=30)
    elif options.require_swift:
        raise SystemExit('swiftc required')
    else:
        print('SKIP: Swift checks require swiftc; run with --require-swift on Mac CI')
