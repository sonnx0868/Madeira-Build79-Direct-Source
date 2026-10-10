#!/usr/bin/env python3
"""Stage the production Swift tools with receipt and containment fault injection."""
from pathlib import Path
import shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
swift=shutil.which('swiftc')
if not swift: raise SystemExit('swiftc is required')
fixture=r'''
enum MadeiraConfig { static func get(_ name: String) -> String? { nil } }
final class LogStore { static let shared = LogStore(); func log(_ line: String) { print(line) } }
let tmp=URL(fileURLWithPath:CommandLine.arguments[1],isDirectory:true)
let fm=FileManager.default, drive=tmp.appendingPathComponent("drive"), bundle=tmp.appendingPathComponent("resources")
try fm.createDirectory(at:drive,withIntermediateDirectories:true)
try fm.createDirectory(at:bundle,withIntermediateDirectories:true)
var receipt:[String:[String:Any]]=[:]
for name in TranslationTools.files {
    let data=Data(("fixture:"+name).utf8)
    try data.write(to:bundle.appendingPathComponent(name))
    receipt[name]=["sha256":SHA256.hash(data:data).map { String(format:"%02x",$0) }.joined()]
}
let manifest=bundle.appendingPathComponent("receipt.json")
try JSONSerialization.data(withJSONObject:receipt).write(to:manifest)
let path=try TranslationTools.stage(resources:bundle,drive:drive)
assert(path.hasSuffix(".exe")); assert(fm.fileExists(atPath:drive.appendingPathComponent(path).path))
let exe=drive.appendingPathComponent(path)
let before=try Data(contentsOf:exe)
try Data("corrupt".utf8).write(to:bundle.appendingPathComponent(TranslationTools.files[0]))
do { _=try TranslationTools.stage(resources:bundle,drive:drive); fatalError("accepted corrupted tool") } catch {}
let staged=try Data(contentsOf:exe); assert(staged==before)
try before.write(to:bundle.appendingPathComponent(TranslationTools.files[0]))
let outside=tmp.appendingPathComponent("outside"); try fm.createDirectory(at:outside,withIntermediateDirectories:true)
let symlinkDrive=tmp.appendingPathComponent("symlink-drive"); try fm.createDirectory(at:symlinkDrive,withIntermediateDirectories:true)
try fm.createSymbolicLink(at:symlinkDrive.appendingPathComponent("Madeira"),withDestinationURL:outside)
do { _=try TranslationTools.stage(resources:bundle,drive:symlinkDrive); fatalError("escaped drive") } catch {}
assert(!fm.fileExists(atPath:outside.appendingPathComponent("Tools").path))
assert(TranslationTools.latestReport(drive:drive)==nil)
try Data("[translation-lab] checksums=equal fixture=1".utf8).write(to:drive.appendingPathComponent(TranslationTools.reportName))
assert(TranslationTools.latestReport(drive:drive)?.contains("Last result:")==true)
CPUTranslationSettings.apply(mode:"reuse",budget:512,relativePath:path,drive:drive)
assert(String(cString:getenv("MADEIRA_CPU_CACHE"))=="reuse")
assert(String(cString:getenv("FEX_MAXINST"))=="512")
let cacheRelative="Madeira/Cache/cpu-v1/"+CPUTranslationSettings.fileKey(relativePath:path)+".bin"
let cacheFile=try TranslationTools.checkedPath(cacheRelative,drive:drive)
try Data("cache fixture".utf8).write(to:cacheFile)
try Data("disabled fixture".utf8).write(to:cacheFile.appendingPathExtension("disabled"))
try CPUTranslationSettings.clear(relativePath:path,drive:drive)
assert(!fm.fileExists(atPath:cacheFile.path) && !fm.fileExists(atPath:cacheFile.appendingPathExtension("disabled").path))
CPUTranslationSettings.apply(mode:"off",budget:nil,relativePath:path,drive:drive)
assert(String(cString:getenv("MADEIRA_CPU_CACHE"))=="0"); assert(getenv("MADEIRA_CPU_CACHE_PATH")==nil)
assert(getenv("FEX_MAXINST")==nil)
CPUTranslationSettings.apply(mode:"reuse",budget:nil,relativePath:path,drive:symlinkDrive)
assert(String(cString:getenv("MADEIRA_CPU_CACHE"))=="0")
print("PASS: production tool staging, receipt mismatch, path containment, report and CPU settings reset")
'''
with tempfile.TemporaryDirectory(prefix='madeira-translation-tools-') as directory:
    tmp=Path(directory)
    (tmp/'main.swift').write_text((root/'app/Madeira/TranslationTools.swift').read_text(encoding='utf-8')+'\n'+fixture,encoding='utf-8')
    subprocess.run([swift,str(tmp/'main.swift'),'-o',str(tmp/'check')],check=True)
    subprocess.run([str(tmp/'check'),str(tmp)],check=True)
