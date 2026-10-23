#!/usr/bin/env python3
"""Exercise the production Chrome package/profile/command policy, without a browser session.

Requires Swift for execution. This is not proof that Chrome runs through Wine on iOS.
Optionally inspect a real, clean Chrome folder with --chrome /path/to/chrome.exe.
"""
from pathlib import Path
import argparse
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--require-swift', action='store_true')
parser.add_argument('--chrome', type=Path)
args = parser.parse_args()
library = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
runtime = (root / 'app/Madeira/GameRuntime.swift').read_text(encoding='utf-8')
frontend = (root / 'app/Madeira/ContentView.swift').read_text(encoding='utf-8')
assert 'if isWindowsBrowser {\n            return WindowsBrowser.arguments' in library
assert 'desktop == true || isWindowsBrowser { setenv("MADEIRA_DESKTOP", "1", 1)' in library
assert 'if entry.isWindowsBrowser { return true }' in runtime
assert 'if epicAppName != nil || isWindowsBrowser { setenv("_MADEIRA_PRIVATE_ARGS", "1", 1)' in library
assert frontend.index('try WindowsBrowser.prepare(') < frontend.index('entry.configureLaunch()', frontend.index('try WindowsBrowser.prepare('))
print('PASS: Chrome-only desktop/direct routing and private URL argument logging are wired')

swift = shutil.which('swiftc')
if not swift:
    if args.require_swift:
        raise SystemExit('Swift compiler required; no Chrome execution result claimed')
    print('SKIP: compiled production browser policy requires Swift/Mac CI')
    raise SystemExit(0)

fixture = r'''
import Foundation
let fm = FileManager.default
let scratch = fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try fm.createDirectory(at: scratch, withIntermediateDirectories: true)
defer { try? fm.removeItem(at: scratch) }
let drive = scratch.appendingPathComponent("drive_c")
try fm.createDirectory(at: drive, withIntermediateDirectories: true)
func image(_ machine: UInt16 = 0x8664) -> Data {
    var bytes = [UInt8](repeating: 0, count: 134)
    bytes[0] = 0x4d; bytes[1] = 0x5a; bytes[60] = 128
    bytes[128] = 0x50; bytes[129] = 0x45
    bytes[132] = UInt8(truncatingIfNeeded: machine); bytes[133] = UInt8(machine >> 8)
    return Data(bytes)
}
func install(_ root: URL, version: String? = nil, machine: UInt16 = 0x8664) throws -> URL {
    try fm.createDirectory(at: root, withIntermediateDirectories: true)
    let exe = root.appendingPathComponent("chrome.exe")
    try image(machine).write(to: exe)
    let dlls = version.map { root.appendingPathComponent($0) } ?? root
    try fm.createDirectory(at: dlls.appendingPathComponent("locales"), withIntermediateDirectories: true)
    for name in ["chrome.dll", "chrome_elf.dll"] { try image(machine).write(to: dlls.appendingPathComponent(name)) }
    for name in ["icudtl.dat", "resources.pak", "v8_context_snapshot.bin", "locales/en-US.pak"] {
        try Data([1]).write(to: dlls.appendingPathComponent(name))
    }
    return exe
}
let flat = try install(drive.appendingPathComponent("Apps/Chrome flat"))
assert(WindowsBrowser.inspect(flat)?.machine == 0x8664)
let versioned = try install(drive.appendingPathComponent("Apps/Chrome versioned"), version: "156.0.8078.47")
assert(WindowsBrowser.inspect(versioned)?.binaryDirectory.lastPathComponent == "156.0.8078.47")
let x86 = try install(drive.appendingPathComponent("Apps/x86"), machine: 0x14c)
assert(WindowsBrowser.inspect(x86)?.machine == 0x14c)
let fake = drive.appendingPathComponent("just chrome.exe")
try image().write(to: fake); assert(WindowsBrowser.inspect(fake) == nil)
let malformed = drive.appendingPathComponent("Malformed/chrome.exe")
try fm.createDirectory(at: malformed.deletingLastPathComponent(), withIntermediateDirectories: true)
try Data("not PE".utf8).write(to: malformed); assert(WindowsBrowser.inspect(malformed) == nil)
let dll = flat.deletingLastPathComponent().appendingPathComponent("chrome.dll")
try image(0x14c).write(to: dll); assert(WindowsBrowser.inspect(flat) == nil)
try image().write(to: dll)
let pak = flat.deletingLastPathComponent().appendingPathComponent("resources.pak")
try Data().write(to: pak); assert(WindowsBrowser.inspect(flat) == nil)
try Data([1]).write(to: pak)
let old = UUID(), other = UUID()
try WindowsBrowser.prepare(executable: flat, drive: drive, id: old)
let profile = drive.appendingPathComponent(WindowsBrowser.profileRelativePath(old)).appendingPathComponent("Profile")
try fm.createDirectory(at: profile, withIntermediateDirectories: true)
let preferences = profile.appendingPathComponent("Preferences"), lock = profile.appendingPathComponent("SingletonLock")
try Data("existing preferences".utf8).write(to: preferences)
try Data("existing lock".utf8).write(to: lock)
try WindowsBrowser.prepare(executable: flat, drive: drive, id: old)
let savedPreferences = try Data(contentsOf: preferences), savedLock = try Data(contentsOf: lock)
assert(savedPreferences == Data("existing preferences".utf8))
assert(savedLock == Data("existing lock".utf8))
try WindowsBrowser.prepare(executable: flat, drive: drive, id: other)
assert(WindowsBrowser.profileRelativePath(old) != WindowsBrowser.profileRelativePath(other))
let outside = try install(scratch.appendingPathComponent("Outside"))
do { try WindowsBrowser.prepare(executable: outside, drive: drive, id: old); fatalError("outside executable accepted") } catch {}
let escapeID = UUID(), escaped = drive.appendingPathComponent(WindowsBrowser.profileRelativePath(escapeID))
try fm.createSymbolicLink(at: escaped, withDestinationURL: scratch)
do { try WindowsBrowser.prepare(executable: flat, drive: drive, id: escapeID); fatalError("outside profile accepted") } catch {}
assert(!fm.fileExists(atPath: scratch.appendingPathComponent("Profile").path))
let nestedID = UUID(), nested = drive.appendingPathComponent(WindowsBrowser.profileRelativePath(nestedID))
try fm.createDirectory(at: nested, withIntermediateDirectories: true)
try fm.createSymbolicLink(at: nested.appendingPathComponent("Profile"), withDestinationURL: scratch)
do { try WindowsBrowser.prepare(executable: flat, drive: drive, id: nestedID); fatalError("nested outside profile accepted") } catch {}
try fm.removeItem(at: dll); try fm.createSymbolicLink(at: dll, withDestinationURL: outside)
assert(WindowsBrowser.inspect(flat) == nil)
let text = "https://example.com/?q=tiếng-Việt&x=1"
let normal = WindowsBrowser.arguments(WindowsBrowser.quote(text), id: old, resolution: "1920x1080", software: false, disableSandbox: false)
let tokens = WindowsBrowser.tokens(normal)
assert(tokens.contains(text) && tokens.contains("--window-size=1920,1080"))
assert(tokens.filter { $0.hasPrefix("--user-data-dir=") }.count == 1)
assert(tokens.contains("--enable-logging=stderr") && tokens.contains("--log-level=3"))
for flag in ["--no-sandbox", "--disable-gpu", "--single-process", "--in-process-gpu", "--remote-debugging-port", "--enable-automation"] {
    assert(!tokens.contains { $0.hasPrefix(flag) })
}
let custom = #"--user-data-dir="C:\My Profile" --window-size=800,600 "https://example.com/a b""#
let adjusted = WindowsBrowser.tokens(WindowsBrowser.arguments(custom, id: old, resolution: "1920x1080", software: true, disableSandbox: true))
assert(adjusted.filter { $0.hasPrefix("--user-data-dir=") } == [#"--user-data-dir=C:\My Profile"#])
assert(adjusted.filter { $0.hasPrefix("--window-size=") } == ["--window-size=800,600"])
assert(adjusted.contains("--no-sandbox") && adjusted.contains("--disable-gpu"))
let customLogging = WindowsBrowser.tokens(WindowsBrowser.arguments("--enable-logging=file --log-level=2", id: old, resolution: "1920x1080", software: false, disableSandbox: false))
assert(customLogging.filter { $0.hasPrefix("--enable-logging=") } == ["--enable-logging=file"])
assert(customLogging.filter { $0.hasPrefix("--log-level=") } == ["--log-level=2"])
assert(WindowsBrowser.tokens(#"one\ two"#) == [#"one\"#, "two"])
for value in ["", "plain", "a b", "\t", "quotes\"inside", "C:\\ends in slash\\", "tiếng Việt 日本語 🙂", "&|%$()"] {
    assert(WindowsBrowser.tokens(WindowsBrowser.quote(value)) == [value], value)
}
if CommandLine.arguments.count > 1 {
    let actual = URL(fileURLWithPath: CommandLine.arguments[1])
    guard let result = WindowsBrowser.inspect(actual) else { fatalError("real Chrome package was not identified") }
    print("PASS: actual Chrome package arch=\(String(result.machine, radix: 16)) directory=\(result.binaryDirectory.lastPathComponent)")
}
print("PASS: production Chrome flat/versioned/x86 packages, malformed/mismatched/missing resources, symlink containment, persistent profiles and quoted launch switches")
print("NOT MEASURED: Chrome/Wine iOS rendering, networking, input, sandbox compatibility or device memory")
'''
with tempfile.TemporaryDirectory(prefix='madeira-browser-') as temp:
    work = Path(temp)
    (work / 'main.swift').write_text(fixture, encoding='utf-8')
    target = work / 'browser-check'
    subprocess.run([swift, str(root / 'app/Madeira/WindowsBrowser.swift'), str(work / 'main.swift'), '-o', str(target)], check=True)
    subprocess.run([str(target), *([str(args.chrome)] if args.chrome else [])], check=True)
