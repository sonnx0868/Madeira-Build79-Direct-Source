#!/usr/bin/env python3
"""Keep manual EXE history across stale metadata saves and protect private games."""
from pathlib import Path
import shutil,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv:raise SystemExit('Swift compiler required')
    print('SKIP: recents policy runs on Mac CI');raise SystemExit(0)
s=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
start=s.index('enum LibraryRecents {');end=s.index('\nenum ExternalGameCompatibility',start)
fixture=r'''
import Foundation
struct LibraryEntry {
    var title:String;var desktop:Bool?;var temporarySession:Bool?;var steamAppID:Int?;var lastPlayed:Date?;var hidden=false
}
let old=Date(timeIntervalSince1970:10),new=Date(timeIntervalSince1970:20)
'''+s[start:end]+r'''
assert(LibraryRecents.latest(new,nil)==new) // a stale pre-launch details sheet
assert(LibraryRecents.latest(new,old)==new) // an asynchronous importer
assert(LibraryRecents.latest(old,new)==new)
assert(LibraryRecents.latest(nil,nil)==nil)
let input=[
    LibraryEntry(title:"Manual EXE",lastPlayed:new),
    LibraryEntry(title:"Epic",lastPlayed:old),
    LibraryEntry(title:"Steam",steamAppID:1,lastPlayed:old),
    LibraryEntry(title:"Uninstalled Steam",steamAppID:2,lastPlayed:new),
    LibraryEntry(title:"Never played"),
    LibraryEntry(title:"Private EXE",lastPlayed:new,hidden:true),
    LibraryEntry(title:"Desktop",desktop:true,lastPlayed:new),
    LibraryEntry(title:"Installer",temporarySession:true,lastPlayed:new)]
let result=LibraryRecents.games(input,installedSteam:[1],isPrivate:{$0.hidden})
assert(result.map(\.title)==["Manual EXE","Epic","Steam"])
let many=(0..<20).map {LibraryEntry(title:"Manual \($0)",lastPlayed:Date(timeIntervalSince1970:Double($0)))}
let recent=LibraryRecents.games(many,installedSteam:[],isPrivate:{_ in false})
assert(recent.count==12 && recent.first?.title=="Manual 19" && recent.last?.title=="Manual 8")
print("PASS: manual/store recents, stale metadata merge, 12-card shelf and hidden/temporary/uninstalled exclusions")
'''
with tempfile.TemporaryDirectory(prefix='madeira-recents-') as tmp:
    p=Path(tmp);(p/'test.swift').write_text(fixture,encoding='utf-8')
    subprocess.run([swift,str(p/'test.swift'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
