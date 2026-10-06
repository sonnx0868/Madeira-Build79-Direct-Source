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
checks = r'''
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
try Data().write(to: file)
do { _ = try DiagnosticSnapshot.create(from: file); fatalError("empty log accepted") } catch {}
print("PASS: production version/build comparison, repository URL validation, snapshot contents, size bound and empty log rejection")
'''
with tempfile.TemporaryDirectory() as folder:
    source = Path(folder) / "main.swift"
    binary = Path(folder) / "check"
    source.write_text(update + error + snapshot + checks, encoding="utf-8")
    subprocess.run([swift, str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
