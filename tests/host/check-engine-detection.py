#!/usr/bin/env python3
"""Identify real runtime layout markers without reading or modifying stories."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'app/Madeira/GameEngineDetection.swift').read_text(encoding='utf-8')
project = (root / 'app/Madeira.xcodeproj/project.pbxproj').read_text(encoding='utf-8')
assert 'GameEngineDetection.swift in Sources' in project
assert 'GameEngineDetection.detect' in (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'GameEngineDetection.detect' in (root / 'app/Madeira/GameRuntime.swift').read_text(encoding='utf-8')
swift = shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required')
    print('SKIP: runtime layout behavior tests require Swift; macOS CI requires them')
    raise SystemExit(0)
checks = r'''
let fm = FileManager.default
let base = fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try fm.createDirectory(at: base, withIntermediateDirectories: true)
defer { try? fm.removeItem(at: base) }
func fixture(_ name: String, files: [String: Data], expected: String?) throws {
    let folder = base.appendingPathComponent(name)
    try fm.createDirectory(at: folder, withIntermediateDirectories: true)
    let exe = folder.appendingPathComponent("renamed game.exe")
    try Data("MZnot a valid PE".utf8).write(to: exe)
    for (name, data) in files {
        let file = folder.appendingPathComponent(name)
        try fm.createDirectory(at: file.deletingLastPathComponent(), withIntermediateDirectories: true)
        try data.write(to: file)
    }
    let original = try Data(contentsOf: exe)
    assert(GameEngineDetection.detect(exe) == expected, "wrong engine: \(name)")
    assert(try! Data(contentsOf: exe) == original)
}
let empty = Data()
try fixture("renpy", files:["LIB/PY3-WINDOWS-X86_64/LibRenPython.DLL":empty], expected:"Ren'Py")
try fixture("unity", files:["UnityPlayer.dll":empty], expected:"Unity")
try fixture("tyrano", files:["resources/app/tyrano/tyrano.js":empty], expected:"TyranoScript")
try fixture("mv", files:["www/index.html":empty,"www/js/rpg_core.js":empty,"www/data/System.json":empty], expected:"RPG Maker MV")
try fixture("mz", files:["index.html":empty,"js/rmmz_core.js":empty,"data/System.json":empty], expected:"RPG Maker MZ")
try fixture("rgss", files:["Game.ini":empty,"RGSS301.dll":empty], expected:"RPG Maker XP/VX/VX Ace (RGSS)")
try fixture("ons", files:["nscript.dat":empty,"arc.nsa":empty], expected:"NScripter/ONScripter")
try fixture("kirikiri", files:["data.xp3":Data([0x58,0x50,0x33,0x0d,0x0a,0x20,0x0a,0x1a,0x8b,0x67,0x01])], expected:"Kirikiri/KAG")
try fixture("godot", files:["data.pck":Data("GDPC".utf8)], expected:"Godot")
try fixture("nw", files:["nw.dll":empty,"package.json":empty], expected:"NW.js")
try fixture("electron", files:["chrome_elf.dll":empty,"resources/app.asar":empty], expected:"Electron")
try fixture("false-markers", files:["data.xp3":empty,"fake.pck":empty,"nscript.dat":empty,"js/rpg_core.js":empty], expected:nil)
try fixture("python-only", files:["lib/windows-x86_64/libpython3.9.dll":empty], expected:nil)
try fixture("empty", files:[:], expected:nil)
// Exercise the actual bounded PE resource reader, including UTF-16 identity.
var pe = Data(repeating:0,count:1024)
pe[0]=0x4d; pe[1]=0x5a; pe[60]=128
pe[128]=0x50; pe[129]=0x45; pe[134]=1
pe.replaceSubrange(152..<157,with:Data(".rsrc".utf8))
pe[168]=128; pe[173]=2 // raw length 128, raw offset 512
let marker = Data("LiveMaker".utf16.flatMap { [UInt8($0&255),UInt8($0>>8)] })
pe.replaceSubrange(512..<512+marker.count,with:marker)
let resource = base.appendingPathComponent("resource.exe")
try pe.write(to:resource)
assert(GameEngineDetection.detect(resource)=="LiveMaker")
pe[134]=255; try pe.write(to:resource)
assert(GameEngineDetection.detect(resource)==nil) // malformed section count
print("PASS: renamed executables, case folding, 12 runtime families, false markers, bounded PE resources and unchanged files")
'''
with tempfile.TemporaryDirectory(prefix='madeira-engine-check-') as temp:
    folder = Path(temp)
    (folder/'main.swift').write_text(source+'\n'+checks, encoding='utf-8')
    subprocess.run([swift,str(folder/'main.swift'),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True)
