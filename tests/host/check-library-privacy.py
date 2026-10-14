#!/usr/bin/env python3
"""Exercise actual library visibility/cache policies and PBKDF2 PIN verifier."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
swift = shutil.which('swiftc')
library = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
grouping = (root / 'app/Madeira/LibraryGrouping.swift').read_text(encoding='utf-8')
pin = (root / 'app/Madeira/LibraryPIN.swift').read_text(encoding='utf-8')
access = (root / 'app/Madeira/LibraryAccess.swift').read_text(encoding='utf-8')
assert 'SecItemAdd' in access and 'kSecAttrAccessibleWhenUnlockedThisDeviceOnly' in access
assert 'token == epoch' in access and 'private(set) var unlocked = false' in access
assert 'willResignActiveNotification' in access and 'window.windowLevel = .alert + 5' in access
assert 'LibraryPrivacyShield.shared.cover(); access.background()' in access
assert 'LibraryModel.shared.current != nil || (unlocked && LibraryModel.shared.startingJIT != nil)' in access
assert 'LibraryAccess.shared.unlocked || LibraryModel.shared.current != nil' in access
assert 'access.guarded && (access.privateSession || access.privateLibraryVisible)' in access
assert 'asyncAfter(deadline: .now() + 0.5)' not in access
assert 'access.privateLibraryVisible = privatePlaylist && tab == 0' in library
assert 'reportsHeaderAlignment = false' in library
assert 'LibraryScenicBackground(sidebar: true)' in library and 'LibraryScenicBackground().ignoresSafeArea()' in access
assert 'LibraryInlineSearch(text:' in library and 'sidebar.frame(width: 224)' in library

assert '.onLongPressGesture(minimumDuration: 2)' in library
assert 'onTapGesture(count: 3)' not in access
assert 'var genreID: UUID?' in library and 'var privateGame: Bool?' in library
assert 'access.request { browser = true }' not in library
assert 'access.request { settingsSheet' not in library
assert 'privatePlaylist = true; recentOnly = false' in library
assert 'SecureField("6-digit PIN"' not in access and 'ForEach(0..<6' in access
assert 'var resolution = "1920x1080"' in library and 'display.flatMap(DisplayMode.init(rawValue:)) ?? .fit' in library
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required for privacy behavior')
    print('SKIP: privacy behavior requires Mac Swift/CommonCrypto'); raise SystemExit(0)


def function(signature):
    start = library.index(signature)
    pos = library.index('{', start) + 1
    depth = 1
    while depth:
        depth += (library[pos] == '{') - (library[pos] == '}')
        pos += 1
    return library[start:pos]


fixture = grouping + '\n' + pin + r'''
struct LibraryEntry: Codable {
    var id: UUID
    var title: String
    var relativePath: String
    var bits: Int
    var desktop: Bool?
    var steamAppID: Int?
    var genreID: UUID?
    var privateGame: Bool?
    var lastPlayed: Date?
    var folderBytes: Int64?
}
final class LibraryAccess { static let shared = LibraryAccess(); var unlocked = false }
final class Model {
    var entries: [LibraryEntry] = [] { didSet { displayedEntriesCache = nil } }
    var genres: [LibraryGenre] = [] { didSet { displayedEntriesCache = nil } }
    private var displayedEntriesCache: (search: String, sort: String, genre: UUID?, unlocked: Bool, items: [LibraryEntry])?
    func persist(_ next: [LibraryEntry]) { entries = next }
'''
for signature in ('func isPrivate(', 'func isVisible(', 'func steamVisible(', 'func displayedEntries(', 'func removeSteam('):
    fixture += function(signature) + '\n'
fixture += '\n}\n'
# Execute the exact background policy with UIKit-free model/access stubs.
bg_start = access.index('    func background() {')
bg_end = access.index('    private func query()', bg_start)
background = access[bg_start:bg_end].replace('LibraryModel.shared', 'BackgroundModel.shared')
fixture += r'''
final class BackgroundModel {
    static let shared = BackgroundModel()
    var current: String?
    var startingJIT: String?
}
final class BackgroundAccess {
    var unlocked = false
    var epoch = 0
    var requested = false
    var afterAccess: (() -> Void)?
    func lock() { epoch += 1; unlocked = false; requested = false; afterAccess = nil }
'''
fixture += background + r'''
}
let backgroundAccess = BackgroundAccess()
backgroundAccess.unlocked = true
BackgroundModel.shared.current = "private game"
backgroundAccess.background()
assert(backgroundAccess.unlocked && backgroundAccess.epoch == 1)
BackgroundModel.shared.current = nil
BackgroundModel.shared.startingJIT = "authorized private game"
backgroundAccess.background()
assert(backgroundAccess.unlocked && backgroundAccess.epoch == 2)
backgroundAccess.unlocked = false
backgroundAccess.background()
assert(!backgroundAccess.unlocked) // pending JIT cannot grant private access
BackgroundModel.shared.startingJIT = nil
backgroundAccess.unlocked = true
backgroundAccess.background()
assert(!backgroundAccess.unlocked) // ordinary library leave still locks
struct Document: Codable { var version: Int; var entries: [LibraryEntry]; var genres: [LibraryGenre]? }
let old = Data(#"{"version":1,"entries":[{"id":"10000000-0000-0000-0000-000000000001","title":"Public","relativePath":"Games/P/game.exe","bits":64}]}"#.utf8)
let doc = try JSONDecoder().decode(Document.self, from: old)
assert(doc.genres == nil && doc.entries[0].privateGame == nil && doc.entries[0].genreID == nil)
let model = Model()
let publicGroup = LibraryGenre(title:"Action")
let privateGroup = LibraryGenre(title:"Novels",isPrivate:true)
model.genres = [publicGroup,privateGroup]
var one=doc.entries[0], two=one, three=one, four=one
one.genreID=publicGroup.id
two.id=UUID();two.title="Private title";two.privateGame=true
three.id=UUID();three.title="Group private title";three.genreID=privateGroup.id
four.id=UUID();four.title="Steam private title";four.steamAppID=123;four.privateGame=true
model.entries=[one,two,three,four]
assert(model.displayedEntries(search:"",sort:"played").map(\.title)==["Public"])
assert(model.displayedEntries(search:"private",sort:"played").isEmpty)
assert(!model.steamVisible(123) && model.steamVisible(999))
assert(LibraryVisibility.shownGenres(model.genres,unlocked:false)==[publicGroup])
LibraryAccess.shared.unlocked=true
assert(model.displayedEntries(search:"private",sort:"played").count==2 && !model.steamVisible(123))
assert(model.steamVisible(123,privatePlaylist:true))
LibraryAccess.shared.unlocked=false // same cache key must not retain private results
assert(model.displayedEntries(search:"private",sort:"played").isEmpty && !model.steamVisible(123))
assert(model.displayedEntries(search:"",sort:"added",genre:publicGroup.id).count==1)
assert(model.displayedEntries(search:"",sort:"added",genre:privateGroup.id).isEmpty)
assert(LibraryVisibility.isPrivate(flag:nil,genreID:UUID(),genres:model.genres))
let encoded=try JSONEncoder().encode(Document(version:1,entries:model.entries,genres:model.genres))
let restored=try JSONDecoder().decode(Document.self,from:encoded)
assert(restored.genres==model.genres && restored.entries[1].privateGame==true)
assert(restored.entries[2].genreID==privateGroup.id)
assert(!LibraryVisibility.shelfAllows(isPrivate:true,unlocked:true,privatePlaylist:false))
assert(LibraryVisibility.shelfAllows(isPrivate:true,unlocked:true,privatePlaylist:true))
assert(!LibraryVisibility.shelfAllows(isPrivate:true,unlocked:false,privatePlaylist:true))
assert(LibraryVisibility.shelfAllows(isPrivate:false,unlocked:false,privatePlaylist:false))
model.removeSteam(appID:123)
assert(!model.steamVisible(123) && model.entries.first { $0.steamAppID==123 }?.relativePath==".steam-profiles/123")
LibraryAccess.shared.unlocked=true
assert(model.steamVisible(123,privatePlaylist:true))
LibraryAccess.shared.unlocked=false
assert(LibraryPIN.valid("123456") && !LibraryPIN.valid("12345") && !LibraryPIN.valid("１２３４５６"))
let vector=try LibraryPIN.derive("password",salt:Data("salt".utf8),rounds:1)
assert(vector.map { String(format:"%02x",$0) }.joined()=="120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b")
let salt=Data((0..<16).map { UInt8($0) })
let digest=try LibraryPIN.derive("123456",salt:salt,rounds:120_000)
let record=LibraryPIN.Record(salt:salt,digest:digest,rounds:120_000)
let correct=try LibraryPIN.verify("123456",record:record)
let incorrect=try LibraryPIN.verify("123457",record:record)
assert(correct && !incorrect)
print("PASS: legacy library decoding, persistent genres, hidden search/counts/Steam, relock cache invalidation and PBKDF2 PIN verification")
'''
with tempfile.TemporaryDirectory(prefix='madeira-privacy-') as scratch:
    folder = Path(scratch)
    path = folder / 'main.swift'
    path.write_text(fixture, encoding='utf-8')
    subprocess.run([swift, str(path), '-o', str(folder / 'check')], check=True)
    subprocess.run([str(folder / 'check')], check=True, timeout=30)
