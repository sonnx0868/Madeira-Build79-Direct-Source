#!/usr/bin/env python3
"""Exercise production Steam-folder/direct-EXE resolution before JIT."""
from pathlib import Path
import shutil, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2];app=root/'app/Madeira'
def block(source,signature):
    a=source.index(signature);b=source.index('{',a);n=1;e=b+1
    while n:n+=(source[e]=='{')-(source[e]=='}');e+=1
    return source[a:e]
ui=(app/'RuntimePreparationUI.swift').read_text(encoding='utf8')
library=(app/'Library.swift').read_text(encoding='utf8')
content=(app/'ContentView.swift').read_text(encoding='utf8')
steam=(app/'SteamOwnedLibrary.swift').read_text(encoding='utf8')
assert 'Section("Game check & repair")' in ui and 'repairContent: true' in ui
assert library.index('GameComponentPreparationSection(entry: entry)')<library.index('Section("Library details")')
assert '(try? LibraryModel.executable(entry.launchRelativePath)) != nil' not in block(content,'private func launchPreparedLibraryEntry(')
verify=block(steam,'    func verifyInstalledGame(')
assert 'repair(appID)' in verify and 'while let item = downloads[appID]' in verify
assert 'case .failed' in verify and 'case .paused' in verify and '!inSession' in verify
print('PASS: existing-game repair is visible before library details, waits for actual Steam verification, and Steam folders no longer bypass preflight',flush=True)
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:raise SystemExit('Swift is required')
    print('SKIP: resolver fixtures need Swift on Mac CI');raise SystemExit(0)
rules=block((app/'SteamGames.swift').read_text(encoding='utf8'),'enum SteamDirectStart {')
option=block((app/'SwiftSteam/Library/SteamAppInfo.swift').read_text(encoding='utf8'),'struct SteamLaunchOption:')
# Only the fields are needed; the network metadata parser is covered separately.
option=option[:option.index('    /// Entries')]+ '}\n'
resolver=block(ui,'    private static func executable(').replace('private static','static',1)
inside=block((app/'RuntimePreparation.swift').read_text(encoding='utf8'),'    static func inside(')
stubs=r'''
import Foundation
enum RuntimePreparationError: Error { case message(String) }
enum LibraryError: Error { case message(String) }
struct LibraryEntry {
 var steamAppID: Int?=nil;var relativePath:String;var steamProgram:String?=nil
 var launchRelativePath:String { relativePath }
}
enum LibraryModel {
 static var drive=FileManager.default.temporaryDirectory
 static func executable(_ relative:String)throws->URL {
   let file=drive.appendingPathComponent(relative)
   guard RuntimeFiles.inside(file,drive),FileManager.default.fileExists(atPath:file.path),file.pathExtension=="exe" else {throw LibraryError.message("missing")}
   return file
 }
}
@MainActor final class SteamOwnedLibrary {
 static let shared=SteamOwnedLibrary()
 var options:[SteamLaunchOption]?=nil
 func launchOptions(appID:Int)async->[SteamLaunchOption]? {options}
}
'''
test=r'''
@main struct Check {
 @MainActor static func main() async throws {
  let fm=FileManager.default,base=fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
  try fm.createDirectory(at:base,withIntermediateDirectories:true)
  defer {try? fm.removeItem(at:base)}
  LibraryModel.drive=base
  func file(_ path:String)throws {
    let url=base.appendingPathComponent(path);try fm.createDirectory(at:url.deletingLastPathComponent(),withIntermediateDirectories:true);try Data([77,90]).write(to:url)
  }
  try file("Games/Manual/game.exe")
  let direct=try await Resolver.executable(LibraryEntry(relativePath:"Games/Manual/game.exe"),drive:base)
  assert(direct?.lastPathComponent=="game.exe")
  try file("Steam/common/Game/Bin/Game.exe");try file("Steam/common/Game/setup.exe")
  SteamOwnedLibrary.shared.options=[SteamLaunchOption(executable:"Bin/Game.exe",oslist:"windows")]
  var entry=LibraryEntry(steamAppID:100,relativePath:"Steam/common/Game")
  let selected=try await Resolver.executable(entry,drive:base)
  assert(selected?.path.hasSuffix("/Bin/Game.exe")==true)
  entry.steamProgram="setup.exe"
  let picked=try await Resolver.executable(entry,drive:base);assert(picked?.lastPathComponent=="setup.exe")
  entry.steamProgram=nil;SteamOwnedLibrary.shared.options=nil
  let ambiguous=try await Resolver.executable(entry,drive:base);assert(ambiguous==nil)
  entry.relativePath="../outside"
  do {_ = try await Resolver.executable(entry,drive:base);fatalError("outside folder accepted")}catch{}
  print("PASS: production installed-game resolver handles direct EXEs, Steam metadata, picked programs, ambiguity and out-of-drive rejection")
 }
}
'''
source=stubs+option+rules+'\nenum RuntimeFiles {\n'+inside+'\n}\n@MainActor enum Resolver {\n'+resolver+'\n}\n'+test
with tempfile.TemporaryDirectory(prefix='madeira-repair-') as work:
    p=Path(work);(p/'Check.swift').write_text(source,encoding='utf8')
    subprocess.run([swift,'-parse-as-library','-swift-version','5',str(p/'Check.swift'),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True,timeout=20)
