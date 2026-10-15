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
engines = (root / "app/Madeira/GameEngineDetection.swift").read_text(encoding="utf-8")
error = diagnostics[diagnostics.index("enum SupportError"):diagnostics.index("enum DiagnosticContext")]
snapshot = diagnostics[diagnostics.index("enum DiagnosticSnapshot"):diagnostics.index("@MainActor final class RemoteDiagnostics")]
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
start = library.index("enum UnityLaunch {")
end = library.index("{", start) + 1
depth = 1
while depth:
    depth += (library[end] == "{") - (library[end] == "}")
    end += 1
unity = library[start:end] + "\n"
godot = library[library.index('enum GodotLaunch {'):library.index('enum UnityLaunch {')]
start = library.index('    var launchArguments: String {')
end = library.index('{', start) + 1
depth = 1
while depth:
    depth += (library[end] == '{') - (library[end] == '}')
    end += 1
arguments = library[start:end]
assert library.count('MadeiraConfig.flag("MADEIRA_UNITY_D3D11", fallback: false)') == 2
renderer_profile = r'''
enum MadeiraConfig {
    static var rendererChoice: String?
    static func flag(_ name: String, fallback: Bool = true) -> Bool {
        rendererChoice.map { $0 != "0" } ?? fallback
    }
}
enum ExternalGameCompatibility { static func isUnity(_ entry: RendererEntry) -> Bool { entry.unity } }
enum LibraryModel { static let drive = URL(fileURLWithPath: NSTemporaryDirectory()) }
struct RendererEntry {
    var desktop: Bool? = nil
    var desktopCommand: String? = nil
    var startsSteamGameDirectly = false
    var steamProgramArguments: String? = nil
    var steamAppID: Int? = nil
    var gogID: Int? = nil
    var gogLaunchCommand: String? = nil
    var arguments = ""
    var resolution = "1408x648"
    var unity = true
    var glBackend: String? = nil
    var launchRelativePath = "fixture-renderer.exe"
''' + arguments + '\n}\n' + godot

# The production argument property must recognize a real legacy layout:
# the mocked modern-player performance predicate intentionally returns false.
legacy_profile = r'''
let legacyDrive = LibraryModel.drive.appendingPathComponent(UUID().uuidString)
try FileManager.default.createDirectory(at: legacyDrive, withIntermediateDirectories: true)
defer { try? FileManager.default.removeItem(at: legacyDrive) }
let legacyPath = legacyDrive.lastPathComponent + "/Sundered.exe"
let legacyPlayer = legacyDrive.appendingPathComponent("UnityPlayer.dll")
try Data().write(to: legacyPlayer)
assert(RendererEntry(unity: false, launchRelativePath: legacyDrive.lastPathComponent + "/Modern.exe").launchArguments == "-screen-width 1408 -screen-height 648")
try FileManager.default.removeItem(at: legacyPlayer)
assert(RendererEntry(unity: false, launchRelativePath: legacyPath).launchArguments.isEmpty)
let legacyData = legacyDrive.appendingPathComponent("SuNdErEd_DaTa")
try FileManager.default.createDirectory(at: legacyData.appendingPathComponent("Managed"), withIntermediateDirectories: true)
try FileManager.default.createDirectory(at: legacyData.appendingPathComponent("Mono"), withIntermediateDirectories: true)
try Data().write(to: legacyData.appendingPathComponent("Managed/UnityEngine.dll"))
assert(RendererEntry(unity: false, launchRelativePath: legacyPath).launchArguments.isEmpty)
try Data().write(to: legacyData.appendingPathComponent("Mono/mono.dll"))
let windowed = "-screen-width 1408 -screen-height 648 -screen-fullscreen 0"
assert(RendererEntry(unity: false, launchRelativePath: legacyPath).launchArguments == windowed)
assert(RendererEntry(arguments: windowed, unity: false, launchRelativePath: legacyPath).launchArguments == windowed)
for explicitMode in ["-screen-fullscreen 1", "-screen-fullscreen=1", "-popupwindow", "-window-mode borderless"] {
    assert(RendererEntry(arguments: explicitMode, unity: false, launchRelativePath: legacyPath).launchArguments == explicitMode + " -screen-width 1408 -screen-height 648")
}
assert(RendererEntry(unity: false, launchRelativePath: legacyDrive.lastPathComponent + "/Other.exe").launchArguments.isEmpty)
assert(RendererEntry(steamAppID: 356400, unity: false, launchRelativePath: legacyPath).launchArguments.isEmpty)
print("PASS: production legacy Unity launch applies the selected size and Sundered windowed default, preserving explicit modes, native games and Steam policy")
'''
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
// Compatibility default: no implicit graphics API, while existing resolution
// arguments and deliberate renderer overrides remain supported.
assert(UnityLaunch.arguments("", resolution: "1408x648", matchResolution: true, forceD3D11: false) == "-screen-width 1408 -screen-height 648")
assert(UnityLaunch.arguments("-force-d3d12", resolution: "1408x648", matchResolution: true, forceD3D11: false) == "-force-d3d12 -screen-width 1408 -screen-height 648")
MadeiraConfig.rendererChoice=nil
assert(RendererEntry().launchArguments == "-screen-width 1408 -screen-height 648")
MadeiraConfig.rendererChoice="1"
assert(RendererEntry().launchArguments == "-force-d3d11 -screen-width 1408 -screen-height 648")
assert(RendererEntry(arguments:"-force-d3d12").launchArguments == "-force-d3d12 -screen-width 1408 -screen-height 648")
MadeiraConfig.rendererChoice="0"
assert(!RendererEntry().launchArguments.contains("-force-d3d11"))
assert(RendererEntry(unity:false).launchArguments.isEmpty)
assert(RendererEntry(startsSteamGameDirectly:true,steamProgramArguments:"-force-d3d12").launchArguments == "-force-d3d12")
assert(RendererEntry(gogID:42,gogLaunchCommand:"-store-default",arguments:"-user-choice").launchArguments == "-store-default -user-choice")
print("PASS: production Unity launch preserves the game's renderer by default and explicit DX11/DX12 choices")
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
assert(UpdateRules.newer(version: "0.1.6", build: nil, commit: nil, installedVersion: "0.1.5", installedBuild: 597, installedCommit: ""))
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
assert(captured.prefix(4 * 1024 * 1024) == large.prefix(4 * 1024 * 1024))
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
// Include only recent log/traceback files from the selected Ren'Py installation.
let vn = drive.appendingPathComponent("Games/VN")
let vnLib = vn.appendingPathComponent("lib/py3-windows-x86_64")
try FileManager.default.createDirectory(at: vnLib, withIntermediateDirectories: true)
try Data().write(to: vnLib.appendingPathComponent("librenpython.dll"))
let traceback = vn.appendingPathComponent("traceback.txt")
let renpyLog = vn.appendingPathComponent("log.txt")
for path in [traceback, renpyLog] {
    try Data("RenPy Python startup exception\n".utf8).write(to: path)
    try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: path.path)
}
let selectedRenpyLogs = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                         executableRelativePath: "Games/VN/VN.exe")
assert(Set(selectedRenpyLogs.map(\.name)).isSuperset(of: ["Games/VN/traceback.txt", "Games/VN/log.txt"]))
let nestedSelected = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                               executableRelativePath: "Games/VN/lib/py3-windows-x86_64/pythonw.exe")
assert(Set(nestedSelected.map(\.name)) == Set(selectedRenpyLogs.map(\.name)))
let outsideSelection = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                                 executableRelativePath: "../outside.exe")
assert(Set(outsideSelection.map(\.name)) == Set(logs.map(\.name)))
try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 800)], ofItemAtPath: traceback.path)
let staleTraceback = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                               executableRelativePath: "Games/VN/VN.exe")
assert(!staleTraceback.contains(where: { $0.name == "Games/VN/traceback.txt" }))
let otherGame = drive.appendingPathComponent("Games/Other")
try FileManager.default.createDirectory(at: otherGame, withIntermediateDirectories: true)
try Data("unrelated log\n".utf8).write(to: otherGame.appendingPathComponent("log.txt"))
let otherSelected = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                              executableRelativePath: "Games/Other/game.exe")
assert(Set(otherSelected.map(\.name)) == Set(logs.map(\.name)))
let managedError = otherGame.appendingPathComponent("error_log.txt")
try Data("Engine startup failure\nSteam not found!\n".utf8).write(to: managedError)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: managedError.path)
assert(DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
    executableRelativePath: "Games/Other/game.exe").contains { $0.name == "Games/Other/error_log.txt" })
assert(DiagnosticCompanionLogs.startupError(in: drive, since: since, before: before,
    executableRelativePath: "Games/Other/game.exe")?.contains("Steam not found!") == true)
assert(DiagnosticCompanionLogs.startupError(in: drive, since: since, before: before,
    executableRelativePath: "../Other/game.exe") == nil)
assert(!DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
    executableRelativePath: "Games/VN/VN.exe").contains { $0.name.hasSuffix("error_log.txt") })
try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 800)], ofItemAtPath: managedError.path)
assert(!DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
    executableRelativePath: "Games/Other/game.exe").contains { $0.name.hasSuffix("error_log.txt") })
assert(DiagnosticCompanionLogs.startupError(in: drive, since: since, before: before,
    executableRelativePath: "Games/Other/game.exe") == nil)
let legacyUnity = drive.appendingPathComponent("Games/Sundered/Sundered_Data/output_log.txt")
try FileManager.default.createDirectory(at: legacyUnity.deletingLastPathComponent(), withIntermediateDirectories: true)
try Data("legacy Unity startup failure\n".utf8).write(to: legacyUnity)
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: legacyUnity.path)
let unitySelected = DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                               executableRelativePath: "Games/Sundered/Sundered.exe")
assert(unitySelected.contains { $0.name == "Games/Sundered/Sundered_Data/output_log.txt" })
assert(!DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                    executableRelativePath: "Games/Other/game.exe").contains { $0.name.hasSuffix("output_log.txt") })
try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 800)], ofItemAtPath: legacyUnity.path)
assert(!DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                    executableRelativePath: "Games/Sundered/Sundered.exe").contains { $0.name.hasSuffix("output_log.txt") })
let unityLink = drive.appendingPathComponent("Games/Other/game_Data")
try FileManager.default.createSymbolicLink(at: unityLink, withDestinationURL: legacyUnity.deletingLastPathComponent())
try FileManager.default.setAttributes([.modificationDate: recent], ofItemAtPath: legacyUnity.path)
assert(!DiagnosticCompanionLogs.find(in: drive, since: since, before: before,
                                    executableRelativePath: "Games/Other/game.exe").contains { $0.name.hasSuffix("output_log.txt") })
let klei=drive.appendingPathComponent("users/mobile/Documents/Klei/DoNotStarveTogether/123456789")
try FileManager.default.createDirectory(at:klei,withIntermediateDirectories:true)
let client=klei.appendingPathComponent("client_log.txt")
try Data("asset assertion\n".utf8).write(to:client)
try FileManager.default.setAttributes([.modificationDate:recent],ofItemAtPath:client.path)
try Data("private game save".utf8).write(to:klei.appendingPathComponent("save.txt"))
let kleiLogs=DiagnosticCompanionLogs.find(in:drive,since:since,before:before)
assert(kleiLogs.contains { $0.name.hasSuffix("123456789/client_log.txt") })
assert(!kleiLogs.contains { $0.name.hasSuffix("save.txt") })
try FileManager.default.setAttributes([.modificationDate:Date(timeIntervalSince1970:800)],ofItemAtPath:client.path)
assert(!DiagnosticCompanionLogs.find(in:drive,since:since,before:before).contains { $0.name.hasSuffix("client_log.txt") })
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
    source.write_text(update + engines + error + snapshot + unity + renderer_profile + legacy_profile + display + checks, encoding="utf-8")
    subprocess.run([swift, str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
