#!/usr/bin/env python3
"""Run the production PE import parser on valid and corrupt directory tables."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
start = source.index('    static func importNames(')
end = source.index('\n    // MARK: session', start)
swift = shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required for PE import behavior')
    print('SKIP: PE imports require Swift; Mac CI requires it'); raise SystemExit(0)
fixture = 'import Foundation\nenum Parser {\n' + source[start:end] + '\n}\n' + r'''
let folder = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
try FileManager.default.createDirectory(at: folder, withIntermediateDirectories:true)
defer { try? FileManager.default.removeItem(at: folder) }
func put(_ data: inout Data, _ offset: Int, _ value: UInt32, _ count: Int = 4) {
    for i in 0..<count { data[offset+i] = UInt8(truncatingIfNeeded:value >> (i*8)) }
}
func image(_ wide: Bool, _ delayVA: Bool = false) -> Data {
    var data=Data(repeating:0,count:2048)
    data[0]=77;data[1]=90;put(&data,60,128);put(&data,128,0x4550)
    put(&data,134,1,2);put(&data,148,wide ? 240 : 224,2)
    put(&data,152,wide ? 0x20b : 0x10b,2);put(&data,152+28,0x400000)
    put(&data,152+60,512)
    let directory=152+(wide ? 112 : 96)
    put(&data,directory-4,16)
    put(&data,directory+8,0x1000);put(&data,directory+12,40)
    put(&data,directory+13*8,0x1100);put(&data,directory+13*8+4,64)
    let section=152+(wide ? 240 : 224)
    put(&data,section+12,0x1000);put(&data,section+16,1024);put(&data,section+20,512)
    put(&data,512+12,0x1200)
    put(&data,768,delayVA ? 0 : 1)
    put(&data,768+4,delayVA ? 0x401240 : 0x1240)
    data.replaceSubrange(1024..<1037,with:Data("KERNEL32.dll\0".utf8))
    data.replaceSubrange(1088..<1100,with:Data("USER32.dll\0\0".utf8))
    return data
}
func names(_ data: Data) throws -> [String] {
    let file=folder.appendingPathComponent("fixture.exe");try data.write(to:file)
    return Parser.importNames(file)
}
for wide in [false,true] {
    let good=image(wide)
    let valid=try names(good);assert(valid==["kernel32.dll","user32.dll"])
    let file=folder.appendingPathComponent("fixture.exe")
    let imports=Parser.moduleImports(file)
    assert(imports==[Parser.ModuleImport(name:"kernel32.dll",delayed:false),Parser.ModuleImport(name:"user32.dll",delayed:true)])
    var bad=good;let directory=152+(wide ? 112 : 96)
    put(&bad,directory-4,0);let missingCount=try names(bad);assert(missingCount.isEmpty)
    bad=good;put(&bad,directory+12,0);let missingSize=try names(bad);assert(missingSize==["user32.dll"])
    bad=good;bad[1024]=0xff;let binary=try names(bad);assert(binary==["user32.dll"])
    bad=good;bad.replaceSubrange(1024..<1152,with:Data(repeating:65,count:128))
    let unterminated=try names(bad);assert(unterminated.isEmpty)
    bad=good;put(&bad,512+12,0x13fa)
    bad.replaceSubrange(1530..<1541,with:Data("broken.dll\0".utf8))
    let crossing=try names(bad);assert(crossing==["user32.dll"])
    bad=good;put(&bad,directory+12,20);put(&bad,532+12,0x1240)
    put(&bad,directory+13*8+4,0)
    let bounded=try names(bad);assert(bounded==["kernel32.dll"])
}
let oldDelay=try names(image(false,true));assert(oldDelay==["kernel32.dll","user32.dll"])
let invalid64Delay=try names(image(true,true));assert(invalid64Delay==["kernel32.dll"])
let truncated=try names(Data(image(true).prefix(200)));assert(truncated.isEmpty)
print("PASS: production PE32/64 imports retain eager/delay provenance, delay VA/RVA, directory count/size bounds, binary names, missing NUL and section boundaries")
'''
with tempfile.TemporaryDirectory(prefix='madeira-pe-imports-') as scratch:
    folder = Path(scratch)
    path = folder / 'main.swift'; path.write_text(fixture, encoding='utf-8')
    subprocess.run([swift, str(path), '-o', str(folder/'check')], check=True)
    subprocess.run([str(folder/'check')], check=True, timeout=15)
