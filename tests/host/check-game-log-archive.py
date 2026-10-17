#!/usr/bin/env python3
"""Execute per-game byte capture, session boundaries and crash recovery."""
from pathlib import Path
import shutil
import subprocess
import tempfile
root=Path(__file__).resolve().parents[2]
swift=shutil.which('swiftc')
if not swift:
    print('SKIP: game log archive behavior requires host Swift'); raise SystemExit(0)
source=(root/'app/Madeira/GameLogArchive.swift').read_text(encoding='utf-8')
checks=r'''
// Await the real serial IO queue. A fixed sleep can expire before fsync/journal
// removal on a busy runner (for example immediately after Simulator startup).
extension GameLogArchive {
    func waitForPendingWrites() { queue.sync {} }
}
let fm=FileManager.default
let dir=fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try fm.createDirectory(at:dir,withIntermediateDirectories:true)
defer {try? fm.removeItem(at:dir)}
let log=dir.appendingPathComponent("madeira-log.txt")
try Data("before\n".utf8).write(to:log)
let archive=GameLogArchive()
archive.begin(title:"Game / one",sourceURL:log,metadata:["build":"fixture"])
archive.waitForPendingWrites()
let writer=try FileHandle(forWritingTo:log)
try writer.seekToEnd();try writer.write(contentsOf:Data("Wine output\nFEX crash line\n".utf8));try writer.close()
archive.finish(sourceURL:log)
let after=try FileHandle(forWritingTo:log);try after.seekToEnd()
try after.write(contentsOf:Data("AFTER GAME EXIT\n".utf8));try after.close()
archive.waitForPendingWrites()
let files=try fm.contentsOfDirectory(at:dir.appendingPathComponent("Logs"),includingPropertiesForKeys:nil)
let first=files.first(where:{$0.pathExtension=="txt"})!
let text=try String(contentsOf:first,encoding:.utf8)
assert(first.lastPathComponent.hasPrefix("Game _ one_"))
assert(text.contains("Wine output\nFEX crash line\n") && !text.contains("before\n"))
assert(!text.contains("AFTER GAME EXIT"))
assert(!fm.fileExists(atPath:dir.appendingPathComponent("Logs/.active-session.json").path))
let unicode=GameLogArchive.filename(title:String(repeating:"🎮",count:1000),date:Date())
assert(unicode.utf8.count<255 && !unicode.contains("/"))
// A crash can happen after bytes land in the archive but before a progress update.
let folder=dir.appendingPathComponent("Logs")
let target=folder.appendingPathComponent("crash.txt")
try Data("HDR\nWine ".utf8).write(to:target)
try Data("prefix\nWine crash remains\n".utf8).write(to:log)
let journal:[String:Any]=["filename":"crash.txt","startOffset":7,"headerBytes":4]
try JSONSerialization.data(withJSONObject:journal).write(to:folder.appendingPathComponent(".active-session.json"))
GameLogArchive.recover(sourceURL:log)
let recovered=try String(contentsOf:target,encoding:.utf8)
assert(recovered=="HDR\nWine crash remains\n")
// An interrupted finish retains its recorded boundary, even when unrelated
// app output already reached the old file before the finish worker ran.
let bounded=folder.appendingPathComponent("bounded.txt")
try Data("HDR\nGAME\nEXTRA APP OUTPUT\n".utf8).write(to:bounded)
try Data("prefix\nGAME\nAFTER EXIT\n".utf8).write(to:log)
let bound:[String:Any]=["filename":"bounded.txt","startOffset":7,"headerBytes":4,"endOffset":12]
try JSONSerialization.data(withJSONObject:bound).write(to:folder.appendingPathComponent(".active-session.json"))
GameLogArchive.recover(sourceURL:log)
let boundedText=try String(contentsOf:bounded,encoding:.utf8)
assert(boundedText=="HDR\nGAME\n")
// Traversal journals are rejected and cannot create an outside file.
let bad:[String:Any]=["filename":"../outside.txt","startOffset":0,"headerBytes":0]
try JSONSerialization.data(withJSONObject:bad).write(to:folder.appendingPathComponent(".active-session.json"))
GameLogArchive.recover(sourceURL:log)
assert(!fm.fileExists(atPath:dir.appendingPathComponent("outside.txt").path))
print("PASS: raw Wine/FEX session bytes, Unicode filenames, game boundaries and crash recovery")
'''
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp);(p/'check.swift').write_text(source+checks,encoding='utf-8')
    subprocess.run([swift,str(p/'check.swift'),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True)
