#!/usr/bin/env python3
"""Exercise the production GOG library/installer parser and path validation."""
from pathlib import Path
import shutil
import subprocess
import tempfile
root=Path(__file__).resolve().parents[2]
swift=shutil.which('swiftc')
if not swift:
    print('SKIP: GOG rules require host Swift');raise SystemExit(0)
source=(root/'app/Madeira/GOG/GOGStore.swift').read_text(encoding='utf-8')
source='import Foundation\n'+source[source.index('enum GOGError:'):source.index('@MainActor final class GOGAuth')]
checks=r'''
let products=Data(#"{"totalPages":2,"products":[{"id":1,"title":"Windows game","image":"//images.gog.com/art","worksOn":{"Windows":true}},{"id":2,"title":"Mac game","worksOn":{"Windows":false}}]}"#.utf8)
let (games,pages)=try GOGRules.products(products)
assert(games.count==1 && games[0].image?.scheme=="https" && pages==2)
let metadata=Data(#"{"downloads":{"installers":[{"os":"windows","language":"en","files":[{"id":"a","size":42,"downlink":"https://api.gog.com/file"}]}]}}"#.utf8)
let installers=try GOGRules.installers(metadata)
assert(installers.count==1)
assert(!GOGRules.official(URL(string:"https://api.gog.com.attacker.test/file")!))
let root=FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try FileManager.default.createDirectory(at:root,withIntermediateDirectories:true)
defer {try? FileManager.default.removeItem(at:root)}
for path in ["../escape.exe","/absolute.exe","C:/escape.exe","a//b","a/../b"] {
    do {_=try GOGRules.safe(path,under:root);fatalError("accepted traversal")}catch {}
}
try FileManager.default.createSymbolicLink(at:root.appendingPathComponent("link"),withDestinationURL:root.deletingLastPathComponent())
do {_=try GOGRules.safe("link/outside.exe",under:root);fatalError("accepted symlink")}catch {}
print("PASS: GOG Windows library, installer metadata, official API hosts and path containment")
'''
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp);(p/'check.swift').write_text(source+checks,encoding='utf-8')
    subprocess.run([swift,str(p/'check.swift'),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True)
