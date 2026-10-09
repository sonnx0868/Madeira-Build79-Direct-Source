#!/usr/bin/env python3
"""Exercise production update rules and bounded snapshots using host Swift."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
swift = shutil.which("swiftc")
if not swift:
    if "--require-swift" in sys.argv:
        raise SystemExit("swiftc is required for the remote support behavior tests")
    print("SKIP: Swift behavior tests need swiftc (Codemagic runs with --require-swift)")
    raise SystemExit(0)
update = (root / "app/Madeira/AppUpdate.swift").read_text(encoding="utf-8").split("@MainActor final class AppUpdateModel", 1)[0]
diagnostics = (root / "app/Madeira/RemoteDiagnostics.swift").read_text(encoding="utf-8")
error = diagnostics[diagnostics.index("enum SupportError"):diagnostics.index("enum DiagnosticContext")]
snapshot = diagnostics[diagnostics.index("enum DiagnosticSnapshot"):diagnostics.index("@MainActor final class RemoteDiagnostics")]
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
unity = library[library.index("enum UnityLaunch {"):library.index("enum ExternalGameCompatibility {")]
fps = (root / "app/Madeira/FPSOverlay.swift").read_text(encoding="utf-8")
rate_policy = fps[fps.index("    static func maxHz(for mode:"):fps.index("    /// Arms or releases the link")]
display = "enum TestDisplayPolicy { static var panelMaxFPS = 120\n" + rate_policy + "}\n"
checks = r'''
for panel in [60, 120] {
    TestDisplayPolicy.panelMaxFPS = panel
    assert(TestDisplayPolicy.maxHz(for: 1) == 0 && TestDisplayPolicy.maxHz(for: 3) == 0)
    assert(TestDisplayPolicy.maxHz(for: 0) == panel && TestDisplayPolicy.maxHz(for: 2) == panel)
}
let selected = UnityLaunch.arguments("", resolution: "2560x1440", matchResolution: true, forceD3D11: true)
assert(selected == "-force-d3d11 -screen-width 2560 -screen-height 1440")
assert(UnityLaunch.arguments(selected, resolution: "2560x1440", matchResolution: true, forceD3D11: true) == selected)
let explicit = "-force-vulkan -screen-width 1920 -screen-height 1080"
assert(UnityLaunch.arguments(explicit, resolution: "2560x1440", matchResolution: true, forceD3D11: true) == explicit)
assert(UnityLaunch.arguments("-screen-width=1280", resolution: "2560x1440", matchResolution: true, forceD3D11: false) == "-screen-width=1280 -screen-height 1440")
let quoted = "-profile=\"data x-force-vulkan\"\t-screen-height 720"
assert(UnityLaunch.arguments(quoted, resolution: "2560x1440", matchResolution: true, forceD3D11: true) == quoted + " -force-d3d11 -screen-width 2560")
assert(UnityLaunch.arguments("-custom", resolution: "2560x1440", matchResolution: false, forceD3D11: false) == "-custom")
for bad in ["2560x1440 -other", "99999x1440", "0x0", "2560", "2560x1440x1", "badx2560x1440", "2560xx1440", "2560x1440x"] {
    assert(UnityLaunch.arguments("", resolution: bad, matchResolution: true, forceD3D11: false).isEmpty)
}
assert(ReleaseVersion("0.1.10")! > ReleaseVersion("0.1.9")!)
assert(ReleaseVersion("v0.1.4-build560-abcd")! == ReleaseVersion("0.1.4")!)
assert(ReleaseVersion("bogus") == nil)
assert(UpdateRules.newer(version: "0.1.4", build: 560, commit: "new", installedVersion: "0.1.4", installedBuild: 559, installedCommit: "old"))
assert(!UpdateRules.newer(version: "0.1.4", build: 560, commit: "same", installedVersion: "0.1.4", installedBuild: 559, installedCommit: "same"))
assert(!UpdateRules.newer(version: "0.1.3", build: 900, commit: nil, installedVersion: "0.1.4", installedBuild: 11, installedCommit: ""))
assert(!UpdateRules.newer(version: "0.1.4", build: nil, commit: nil, installedVersion: "0.1.4", installedBuild: 11, installedCommit: ""))
assert(UpdateRules.newer(version: "0.1.5", build: nil, commit: nil, installedVersion: "0.1.4", installedBuild: 560, installedCommit: ""))
assert(UpdateRules.assetURL("https://github.com/sonnx0868/Madeira-Build79-Direct-Source/releases/download/v1/Madeira.ipa") != nil)
for url in ["http://github.com/sonnx0868/Madeira-Build79-Direct-Source/releases/download/v1/Madeira.ipa", "https://evil.example/Madeira.ipa", "https://github.com/other/repo/releases/download/v1/Madeira.ipa", "https://github.com@evil.example/Madeira.ipa"] { assert(UpdateRules.assetURL(url) == nil) }
let directory = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
defer { try? FileManager.default.removeItem(at: directory) }
let file = directory.appendingPathComponent("session.txt")
let small = Data("startup\ncrash details\n".utf8)
try small.write(to: file)
let a = try DiagnosticSnapshot.create(from: file)
assert(!a.truncated && a.originalBytes == UInt64(small.count))
let capturedSmall = try Data(contentsOf: a.url)
assert(capturedSmall == small)
try FileManager.default.removeItem(at: a.url)
var large = Data(repeating: 65, count: DiagnosticSnapshot.maxBytes + 1024)
large.replaceSubrange(large.count-4..<large.count, with: Data("END!".utf8))
try large.write(to: file)
let b = try DiagnosticSnapshot.create(from: file)
let captured = try Data(contentsOf: b.url)
assert(b.truncated && captured.count == DiagnosticSnapshot.maxBytes)
assert(captured.prefix(65536) == large.prefix(65536) && captured.suffix(4) == Data("END!".utf8))
try FileManager.default.removeItem(at: b.url)
// Known companion files from the selected test, excluding symlinks, other
// filenames, stale logs, nested data and files overwritten by a later run.
let drive = directory.appendingPathComponent("drive_c")
let player = drive.appendingPathComponent("users/madeira/AppData/LocalLow/Company/Game/Player.log")
try FileManager.default.createDirectory(at: player.deletingLastPathComponent(), withIntermediateDirectories: true)
let recent = Date(timeIntervalSince1970: 1000)
try Data("game network failure\n".utf8).write(to: player)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: player.path)
let unrelated = player.deletingLastPathComponent().appendingPathComponent("save.dat")
try Data("private save\n".utf8).write(to: unrelated)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: unrelated.path)
let nested = player.deletingLastPathComponent().appendingPathComponent("Nested/Player.log")
try FileManager.default.createDirectory(at: nested.deletingLastPathComponent(), withIntermediateDirectories: true)
try Data("nested private data\n".utf8).write(to: nested)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: nested.path)
let steam = drive.appendingPathComponent("Program Files (x86)/Steam/logs/connection_log.txt")
try FileManager.default.createDirectory(at: steam.deletingLastPathComponent(), withIntermediateDirectories: true)
try Data("Steam connection\n".utf8).write(to: steam)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: steam.path)
let stale = drive.appendingPathComponent("users/madeira/AppData/LocalLow/Other/Old/Player.log")
try FileManager.default.createDirectory(at: stale.deletingLastPathComponent(), withIntermediateDirectories: true)
try Data("old session\n".utf8).write(to: stale)
try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 800)], ofItemAtPath: stale.path)
let linked = drive.appendingPathComponent("users/madeira/AppData/LocalLow/Linked")
try FileManager.default.createSymbolicLink(at: linked, withDestinationURL: directory)
let since = Date(timeIntervalSince1970: 900), before = Date(timeIntervalSince1970: 1100)
let logs = DiagnosticCompanionLogs.find(in: drive, since: since, before: before)
// macOS temporary paths may be spelled /var or /private/var. The collector
// deliberately resolves symlinks before checking containment.
let capturedPaths = Set(logs.map { $0.url.resolvingSymlinksInPath().path })
let expectedPaths = Set([player, steam].map { $0.resolvingSymlinksInPath().path })
assert(logs.count == 2 && capturedPaths == expectedPaths,
       "Unexpected companion logs: \(capturedPaths), expected \(expectedPaths)")
assert(DiagnosticCompanionLogs.find(in: drive, since: since, before: Date(timeIntervalSince1970: 950)).isEmpty)
let bundled = try DiagnosticSnapshot.create(from: file, companions: logs)
let bundleData = try Data(contentsOf: bundled.url)
let bundleText = String(decoding: bundleData, as: UTF8.self)
assert(bundled.truncated && bundled.companionCount == 2 && bundleData.count == DiagnosticSnapshot.maxBytes)
assert(bundleData.prefix(65536) == large.prefix(65536))
assert(bundleText.contains("END!") && bundleText.contains("game network failure") && bundleText.contains("Steam connection"))
assert(!bundleText.contains("private save") && !bundleText.contains("old session") && !bundleText.contains("nested private data"))
try FileManager.default.removeItem(at: bundled.url)
try Data(repeating: 66, count: 2*1024*1024).write(to: player)
let capped = try DiagnosticSnapshot.create(from: file, companions: [DiagnosticCompanionLogs.Source(url: player, name: "Player.log")])
let cappedData = try Data(contentsOf: capped.url)
assert(capped.truncated && cappedData.count == DiagnosticSnapshot.maxBytes)
assert(String(decoding: cappedData, as: UTF8.self).contains("tailOffset=1572864"))
try FileManager.default.removeItem(at: capped.url)
try Data().write(to: file)
do { _ = try DiagnosticSnapshot.create(from: file); fatalError("empty log accepted") } catch {}
print("PASS: Unity arguments, update rules, bounded session and companion snapshots, log age/name selection, symlink exclusion and empty log rejection")
'''
with tempfile.TemporaryDirectory() as folder:
    source = Path(folder) / "main.swift"
    binary = Path(folder) / "check"
    source.write_text(update + error + snapshot + unity + display + checks, encoding="utf-8")
    subprocess.run([swift, str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
