#!/usr/bin/env python3
"""Exercise actual nullable-stats binary guard on bounded synthetic PE inputs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
root=Path(__file__).resolve().parents[2]
swift=shutil.which('swiftc')
if not swift: raise SystemExit('swiftc required for GameMaker compatibility guard')
fixture=r'''
func put(_ data:inout Data,_ at:Int,_ value:UInt32,_ count:Int=4) {
    for n in 0..<count { data[at+n]=UInt8(truncatingIfNeeded:value >> (n*8)) }
}
var image=Data(repeating:0,count:2048)
image[0]=77;image[1]=90;put(&image,60,128);put(&image,128,0x4550)
put(&image,132,0x8664,2);put(&image,134,3,2);put(&image,148,240,2)
put(&image,152,0x20b,2);put(&image,260,16)
put(&image,264,0x2000);put(&image,268,128)
put(&image,288,0x3000);put(&image,292,12)
for n in 0..<3 {
    let section=392+n*40
    put(&image,section+12,UInt32(0x1000*(n+1)))
    put(&image,section+16,512);put(&image,section+20,UInt32(512*(n+1)))
}
put(&image,1024+20,1);put(&image,1024+24,1)
put(&image,1024+28,0x2040);put(&image,1024+32,0x2048);put(&image,1024+36,0x2050)
put(&image,1088,0x1000);put(&image,1096,0x2080);put(&image,1104,0,2)
let name=Data("?steam_get_achievement@@fixture\0".utf8)
image.replaceSubrange(1152..<1152+name.count,with:name)
let code=512
image.replaceSubrange(code+109..<code+119,with:[0x48,0x8b,0x01,0xff,0x50,0x30,0x3c,0x01,0x75,0x11])
image.replaceSubrange(code+136..<code+139,with:[0x0f,0x57,0xc0])
image.replaceSubrange(code+167..<code+176,with:Data(repeating:0xcc,count:9))
put(&image,1536,0x1000);put(&image,1540,0x1000+167);put(&image,1544,0x20e0)
guard let guarded=GameMakerCompatibility.statsGuard(image) else { fatalError("valid guard rejected") }
assert(guarded.count==image.count && guarded.prefix(code+109)==image.prefix(code+109))
assert(Array(guarded[code+109..<code+112])==[0xeb,0x38,0x90])
assert(Array(guarded[code+167..<code+174])==[0xe3,0xdf,0x48,0x8b,0x01,0xeb,0xc2])
// Null RCX branches to the existing false result. Non-null returns to the
// original virtual CALL, preserving its ABI, caller address and behavior.
assert(109+2+Int(guarded[code+110])==167)
assert(167+2+Int(Int8(bitPattern:guarded[code+168]))==136)
assert(172+2+Int(Int8(bitPattern:guarded[code+173]))==112)
assert(Array(guarded[code+112..<code+115])==[0xff,0x50,0x30])
assert(guarded[1540]==UInt8(truncatingIfNeeded:0x1000+174))
assert(GameMakerCompatibility.statsGuard(guarded)==nil)
var drift=image;drift[code+167]=0x90;assert(GameMakerCompatibility.statsGuard(drift)==nil)
drift=image;drift[code+111]=0x02;assert(GameMakerCompatibility.statsGuard(drift)==nil)
drift=image;put(&drift,1540,0x1000+160);assert(GameMakerCompatibility.statsGuard(drift)==nil)
assert(GameMakerCompatibility.statsGuard(Data(image.prefix(200)))==nil)
print("PASS: production GameMaker stats guard preserves valid calls, branches null interfaces to existing false result, covers alignment padding in unwind metadata, rejects drift and is idempotent")
'''
with tempfile.TemporaryDirectory(prefix='madeira-gm-stats-') as scratch:
    folder=Path(scratch);path=folder/'main.swift'
    path.write_text((root/'app/Madeira/GameMakerCompatibility.swift').read_text(encoding='utf-8')+'\n'+fixture,encoding='utf-8')
    subprocess.run([swift,str(path),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True)
