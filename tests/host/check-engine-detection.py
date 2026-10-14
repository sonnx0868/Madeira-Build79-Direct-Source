#!/usr/bin/env python3
"""Identify real runtime layout markers without reading or modifying stories."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'app/Madeira/GameEngineDetection.swift').read_text(encoding='utf-8')
library = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
source += '\n' + library[library.index('enum GodotLaunch {'):library.index('enum UnityLaunch {')]
source += '\n' + library[library.index('enum FolderSteamIdentity {'):library.index('enum GodotLaunch {')]
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
assert(GodotLaunch.angleArguments("") == "--rendering-method gl_compatibility --rendering-driver opengl3_angle")
assert(GodotLaunch.angleArguments("--fullscreen").hasPrefix("--fullscreen --rendering-method"))
for args in ["--rendering-driver opengl3", "--rendering-driver=vulkan", "--rendering-method gl_compatibility"] {
    assert(GodotLaunch.angleArguments(args) == args)
}
assert(GodotLaunch.angleArguments("--path \"C:\\some game\\\"").hasPrefix("--path \"C:\\some game\\\" --rendering-method"))
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
    assert(GameEngineDetection.detect(folder) == expected, "wrong Steam folder engine: \(name)")
    assert(try! Data(contentsOf: exe) == original)
}
let empty = Data()
let steamFolder=base.appendingPathComponent("folder-steam")
try fm.createDirectory(at:steamFolder.appendingPathComponent("steam_settings"),withIntermediateDirectories:true)
let steamExecutable=steamFolder.appendingPathComponent("game.exe")
let identity=steamFolder.appendingPathComponent("steam_settings/steam_appid.txt")
try Data("1687000\n".utf8).write(to:identity)
assert(FolderSteamIdentity.appID(executable:steamExecutable)==1687000)
for bad in ["0","-1","4294967296","x1687000",String(repeating:"9",count:65)] {
    try Data(bad.utf8).write(to:identity)
    assert(FolderSteamIdentity.appID(executable:steamExecutable)==nil)
}
try fixture("gamemaker", files:["data.win":Data("FORM1234GEN8".utf8)], expected:"GameMaker")
try fixture("false-gamemaker", files:["data.win":Data("FORM1234OTHER".utf8)], expected:nil)
try fixture("renpy", files:["LIB/PY3-WINDOWS-X86_64/LibRenPython.DLL":empty], expected:"Ren'Py")
try fixture("unity", files:["UnityPlayer.dll":empty], expected:"Unity")
try fixture("tyrano", files:["resources/app/tyrano/tyrano.js":empty], expected:"TyranoScript")
try fixture("mv", files:["www/index.html":empty,"www/js/rpg_core.js":empty,"www/data/System.json":empty], expected:"RPG Maker MV")
try fixture("mz", files:["index.html":empty,"js/rmmz_core.js":empty,"data/System.json":empty], expected:"RPG Maker MZ")
try fixture("rgss", files:["Game.ini":empty,"RGSS301.dll":empty], expected:"RPG Maker XP/VX/VX Ace (RGSS)")
try fixture("ons", files:["nscript.dat":empty,"arc.nsa":empty], expected:"NScripter/ONScripter")
try fixture("kirikiri", files:["data.xp3":Data([0x58,0x50,0x33,0x0d,0x0a,0x20,0x0a,0x1a,0x8b,0x67,0x01])], expected:"Kirikiri/KAG")
try fixture("godot", files:["data.pck":Data("GDPC".utf8)], expected:"Godot")
try fixture("godot-steam", files:["libgodotsteam.windows.template_release.x86_64.dll":empty], expected:"Godot")
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
// Primal Planet replaces the version resource but retains the template's
// name in the PE export directory. Exercise RVA translation, PE32/PE32+,
// renamed files, truncated tables and section boundaries using real headers.
func templatePE(_ name: String, wide: Bool = true) -> Data {
    var data=Data(repeating:0,count:1024)
    func u16(_ at:Int,_ value:UInt16) { for n in 0..<2 { data[at+n]=UInt8(truncatingIfNeeded:value >> (n*8)) } }
    func u32(_ at:Int,_ value:UInt32) { for n in 0..<4 { data[at+n]=UInt8(truncatingIfNeeded:value >> (n*8)) } }
    data[0]=0x4d; data[1]=0x5a; u32(60,128)
    data[128]=0x50;data[129]=0x45;u16(134,1)
    let optional=wide ? 240 : 224, directory=wide ? 112 : 96
    u16(148,UInt16(optional));u16(152,wide ? 0x20b : 0x10b)
    u32(152+directory-4,1);u32(152+directory,0x1000);u32(152+directory+4,128)
    let section=152+optional
    data.replaceSubrange(section..<section+6,with:Data(".edata".utf8))
    u32(section+12,0x1000);u32(section+16,512);u32(section+20,512)
    u32(524,0x1040)
    data.replaceSubrange(576..<576+name.utf8.count,with:Data(name.utf8))
    return data
}
let template=base.appendingPathComponent("renamed-template.exe")
for wide in [false,true] {
    try templatePE("Godot.Windows.template_release.x86_64.exe",wide:wide).write(to:template)
    assert(GameEngineDetection.detect(template)=="Godot")
}
for name in ["another.engine.exe","godot.windows.template_release.x86_64.dll","godot.fake.exe"] {
    try templatePE(name).write(to:template); assert(GameEngineDetection.detect(template)==nil)
}
var broken=templatePE("godot.windows.template_release.x86_64.exe")
broken[268]=39;try broken.write(to:template);assert(GameEngineDetection.detect(template)==nil)
broken=templatePE("godot.windows.template_release.x86_64.exe");broken[134]=255
try broken.write(to:template);assert(GameEngineDetection.detect(template)==nil)
broken=templatePE("godot.windows.template_release.x86_64.exe");broken[524]=255;broken[525]=17;broken[1023]=103
try broken.write(to:template);assert(GameEngineDetection.detect(template)==nil) // unterminated section-end name
print("PASS: renamed executables, case folding, 12 runtime families, false markers, bounded PE resources/exports and unchanged files")
'''
with tempfile.TemporaryDirectory(prefix='madeira-engine-check-') as temp:
    folder = Path(temp)
    (folder/'main.swift').write_text(source+'\n'+checks, encoding='utf-8')
    subprocess.run([swift,str(folder/'main.swift'),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True)
