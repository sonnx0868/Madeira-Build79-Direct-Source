#!/usr/bin/env python3
"""Exercise capture boundaries, stale buffers and the production FPS detector.
The Darwin harness invokes the real Objective-C controller with a fake public
Metal capture destination; no device or measured GPU performance is invented.
"""
from pathlib import Path
import argparse, importlib.util, os, platform, shutil, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--require-darwin',action='store_true');a=p.parse_args()
cc=os.environ.get('CC') or shutil.which('clang')
if not cc: raise SystemExit('clang required')
policy=r'''
#include <assert.h>
#include <stdio.h>
#include "GPUTracePolicy.h"
int main(void) {
 MadeiraGPUTracePolicy p={0};
 assert(!madeira_trace_startable(&p,1));
 madeira_trace_arm(&p,10);
 assert(!madeira_trace_startable(&p,1));
 assert(!madeira_trace_committed(&p,1,0,0));
 assert(!madeira_trace_committed(&p,1,0,1));
 assert(madeira_trace_startable(&p,1) && !madeira_trace_startable(&p,2));
 madeira_trace_started(&p,11);
 assert(!madeira_trace_created(&p,2));
 assert(madeira_trace_created(&p,1)==1);
 assert(!madeira_trace_committed(&p,1,0,1) && p.frames==0);
 assert(!madeira_trace_committed(&p,2,1,1) && p.frames==0);
 assert(!madeira_trace_committed(&p,1,1,0) && p.frames==0);
 assert(!madeira_trace_committed(&p,1,1,1) && p.frames==1);
 assert(madeira_trace_created(&p,1)==1);
 assert(madeira_trace_committed(&p,1,1,1) && p.phase==MGTraceComplete);
 assert(!madeira_trace_expired(&p,100));
 madeira_trace_arm(&p,20);assert(!madeira_trace_expired(&p,35));assert(madeira_trace_expired(&p,35.1));
 madeira_trace_committed(&p,3,0,1);assert(madeira_trace_expired(&p,36));
 madeira_trace_started(&p,25);assert(!madeira_trace_expired(&p,40));assert(madeira_trace_expired(&p,40.1));
 for(int i=0;i<32;i++) assert(madeira_trace_created(&p,3)==1);
 assert(madeira_trace_created(&p,3)==-1 && p.phase==MGTraceFailed);
 puts("PASS: full-frame capture boundaries, selected queue, old-buffer exclusion, timeout and buffer limit");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-gpu-trace-') as temp:
    out=Path(temp);source=out/'check.c';binary=out/('check.exe' if os.name=='nt' else 'check')
    source.write_text(policy,encoding='utf-8')
    subprocess.run([cc,'-std=c11','-I',str(root/'app/Madeira'),str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
    swift=shutil.which('swiftc')
    if swift:
        text=(root/'app/Madeira/GPUTraceComparison.swift').read_text(encoding='utf-8')
        body=text.split('// BEGIN GPU_TRACE_DROP_POLICY',1)[1].split('// END GPU_TRACE_DROP_POLICY',1)[0]
        source=out/'check.swift';binary=out/('swift-check.exe' if os.name=='nt' else 'swift-check')
        source.write_text('import Foundation\n'+body+r'''
var p = GPUTraceDropPolicy(); p.resume(after: 0)
assert(p.sample(fps: 60, at: 1, eligible: true) == nil)
for t in 5...8 { assert(p.sample(fps: 60, at: Double(t), eligible: true) == nil) }
assert(p.sample(fps: 60, at: 9, eligible: true) == "smooth")
assert(p.reference == 60)
p.capturedReference(at: 10)
p.resume(after: 12) // Closing the menu cannot shorten the capture cooldown.
for t in 13...39 { assert(p.sample(fps: 20, at: Double(t), eligible: true) == nil) }
for t in 40...43 { assert(p.sample(fps: 40, at: Double(t), eligible: true) == nil) }
assert(p.sample(fps: 40, at: 44, eligible: true) == "slow")
assert(p.sample(fps: 10, at: 45, eligible: false) == nil)
for t in 46...49 { assert(p.sample(fps: 40, at: Double(t), eligible: true) == nil) }
assert(p.sample(fps: 60, at: 50, eligible: true) == nil) // A brief dip is not sustained.
var q = GPUTraceDropPolicy()
for t in 0...20 { assert(q.sample(fps: 30, at: Double(t), eligible: true) == nil) }
assert(q.reference == nil)
print("PASS: stable gameplay baseline, sustained relative drop, menu exclusion and post-capture cooldown")
assert(!q.setManualReference(.nan) && !q.setManualReference(0))
assert(q.setManualReference(42) && q.reference == 42)
assert(q.setManualReference(11) && q.reference == 11)
''',encoding='utf-8')
        subprocess.run([swift,str(source),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)
    elif a.require_darwin: raise SystemExit('swiftc required on Darwin')
    else: print('SKIP: Swift detector execution requires swiftc; CI must run it')
    if platform.system()=='Darwin':
        text=(root/'app/Madeira/GPUTraceComparison.swift').read_text(encoding='utf-8')
        files=text[text.index('enum GPUTraceFiles {'):text.index('// BEGIN GPU_TRACE_FILE_PRESENTATION')]
        diagnostics=(root/'app/Madeira/RemoteDiagnostics.swift').read_text(encoding='utf-8')
        error=diagnostics[diagnostics.index('enum SupportError:'):diagnostics.index('enum DiagnosticContext')]
        snapshot=diagnostics[diagnostics.index('enum DiagnosticSnapshot {'):diagnostics.index('enum DiagnosticCompanionLogs {')]
        unpack=r'''
func unpackComparison(_ archive: URL, into directory: URL) throws -> URL {
 let process=Process();process.executableURL=URL(fileURLWithPath:"/usr/bin/ditto")
 process.arguments=["-x","-k",archive.path,directory.path]
 try process.run();process.waitUntilExit();assert(process.terminationStatus==0)
 let walker=FileManager.default.enumerator(at:directory,includingPropertiesForKeys:nil)!
 let manifests=walker.compactMap { $0 as? URL }.filter { $0.lastPathComponent=="manifest.json" }
 assert(manifests.count==1);return manifests[0].deletingLastPathComponent()
}
'''
        source=out/'export.swift';binary=out/'export'
        source.write_text('import Foundation\n'+error+snapshot+files+unpack+r'''
enum DiagnosticCompanionLogs { struct Source { let url: URL; let name: String } }
@main struct Check {
 static func reject(_ folder: URL, path: String? = nil) async {
  do { let zip=try await GPUTraceFiles.export(folder:folder);try? FileManager.default.removeItem(at:zip);assertionFailure("Unsafe fixture exported") }
  catch { if let path { assert(error.localizedDescription.contains("symbolic link: "+path), error.localizedDescription) } }
 }
 static func main() async throws {
  let fm=FileManager.default, temporary=fm.temporaryDirectory.resolvingSymlinksInPath()
  // Exercise the public /var spelling even when the runner's TMPDIR uses
  // /private/var. Enumeration returns canonical URLs on Darwin.
  let temporaryPath=temporary.path.hasPrefix("/private/var/") ? String(temporary.path.dropFirst("/private".count)) : temporary.path
  let root=URL(fileURLWithPath:temporaryPath,isDirectory:true).appendingPathComponent(UUID().uuidString)
  try fm.createDirectory(at: root, withIntermediateDirectories: true)
  defer { try? fm.removeItem(at: root) }
  let log=root.appendingPathComponent("input.txt"), folder=root.appendingPathComponent("00000000-0000-0000-0000-000000000001",isDirectory:true)
  try fm.createDirectory(at: folder, withIntermediateDirectories: false)
  try Data("ORIGINAL CAPTURED SESSION".utf8).write(to: log)
  let manifest:[String:Any] = ["format":"madeira-gpu-comparison-v1", "createdAt":"2026-10-17T03:43:00Z", "context":["gameTitle":"REPLACED"],
    "smooth":["file":"smooth.gputrace","complete":true,"frames":2],
    "slow":["file":"slow.gputrace","complete":true,"frames":2]]
  for kind in ["smooth","slow"] {
   let trace=folder.appendingPathComponent(kind+".gputrace")
   try fm.createDirectory(at:trace,withIntermediateDirectories:false)
   try Data((kind+" GPU PAYLOAD").utf8).write(to:trace.appendingPathComponent("capture.bin"))
   // Real Metal archives use these relative sibling aliases for repeated
   // buffer/texture snapshots. Windows ZIP extraction can flatten them.
   try Data((kind+" BUFFER PAYLOAD").utf8).write(to:trace.appendingPathComponent("MTLBuffer-77048-0"))
   try fm.createSymbolicLink(atPath:trace.appendingPathComponent("MTLBuffer-77048-1").path,withDestinationPath:"MTLBuffer-77048-0")
   let nested=trace.appendingPathComponent("nested",isDirectory:true)
   try fm.createDirectory(at:nested,withIntermediateDirectories:false)
   try Data((kind+" TEXTURE PAYLOAD").utf8).write(to:nested.appendingPathComponent("MTLTexture-1-0"))
   try fm.createSymbolicLink(atPath:nested.appendingPathComponent("MTLTexture-1-1").path,withDestinationPath:"MTLTexture-1-0")
  }
  GPUTraceFiles.writeManifest(manifest,folder:folder)
  GPUTraceFiles.snapshot(log:log,folder:folder)
  let zip=try await GPUTraceFiles.export(folder:folder)
  defer { try? fm.removeItem(at:zip) }
  let header=try FileHandle(forReadingFrom:zip);let signature=try header.read(upToCount:4);try header.close()
  assert(signature==Data([0x50,0x4b,0x03,0x04]))
  let extracted=try unpackComparison(zip,into:root.appendingPathComponent("extracted"))
  for kind in ["smooth","slow"] {
   let trace=extracted.appendingPathComponent(kind+".gputrace")
   let buffer=try Data(contentsOf:trace.appendingPathComponent("MTLBuffer-77048-1"))
   let texture=try Data(contentsOf:trace.appendingPathComponent("nested/MTLTexture-1-1"))
   assert(buffer==Data((kind+" BUFFER PAYLOAD").utf8))
   assert(texture==Data((kind+" TEXTURE PAYLOAD").utf8))
  }
  try Data("DIFFERENT GAME SESSION".utf8).write(to:log)
  let zip2=try await GPUTraceFiles.export(folder:folder.resolvingSymlinksInPath());defer {try? fm.removeItem(at:zip2)}
  let capturedLog=try String(contentsOf:folder.appendingPathComponent("session.txt"),encoding:.utf8)
  assert(capturedLog=="ORIGINAL CAPTURED SESSION")
  // Reproduce 727: the latest pointer names a newer unfinished run, while
  // an earlier pair is complete. Discovery must recover the real old pair.
  let unfinished=root.appendingPathComponent("00000000-0000-0000-0000-000000000002",isDirectory:true)
  try fm.createDirectory(at:unfinished,withIntermediateDirectories:false)
  GPUTraceFiles.writeManifest(["format":"madeira-gpu-comparison-v1","createdAt":"2026-10-17T03:45:00Z"],folder:unfinished)
  var pairs=await GPUTraceFiles.savedComparisons(in:root)
  assert(pairs.count==1 && pairs[0].id==folder.lastPathComponent && pairs[0].title=="REPLACED")
  assert(GPUTraceSavedComparison.select(from:pairs,preferredID:unfinished.lastPathComponent)?.id==folder.lastPathComponent)
  assert(GPUTraceSavedComparison.select(from:pairs,preferredID:nil)?.id==folder.lastPathComponent)
  let older=root.appendingPathComponent("00000000-0000-0000-0000-000000000003",isDirectory:true)
  try fm.copyItem(at:folder,to:older)
  var oldManifest=manifest;oldManifest["createdAt"]="2026-10-16T03:43:00Z"
  GPUTraceFiles.writeManifest(oldManifest,folder:older)
  let malformed=root.appendingPathComponent("00000000-0000-0000-0000-000000000004")
  try fm.createDirectory(at:malformed,withIntermediateDirectories:false)
  try Data("invalid json".utf8).write(to:malformed.appendingPathComponent("manifest.json"))
  try fm.createSymbolicLink(at:root.appendingPathComponent("00000000-0000-0000-0000-000000000005"),withDestinationURL:folder)
  pairs=await GPUTraceFiles.savedComparisons(in:root)
  assert(pairs.count==2 && pairs[0].id==folder.lastPathComponent && pairs[1].id==older.lastPathComponent)
  assert(GPUTraceSavedComparison.select(from:pairs,preferredID:older.lastPathComponent)?.id==older.lastPathComponent)
  let restoredZip=try await GPUTraceFiles.export(folder:GPUTraceSavedComparison.select(from:pairs,preferredID:unfinished.lastPathComponent)!.folder)
  defer {try? fm.removeItem(at:restoredZip)}
  print("PASS: saved GPU comparison discovery recovers complete727 pair behind newer unfinished run, preserves selection, sorts dates and excludes malformed/symlink folders")
  let injected=folder.appendingPathComponent("unrelated.txt");try Data("PRIVATE".utf8).write(to:injected)
  await reject(folder)
  try fm.removeItem(at:injected)
  let trace=folder.appendingPathComponent("smooth.gputrace")
  for (name,target) in [
   ("absolute-external",log.path), ("parent-external","../../input.txt"),
   ("absolute-internal",trace.appendingPathComponent("capture.bin").path),
   ("parent-internal","../smooth.gputrace/capture.bin"),
   ("nested-target","nested/MTLTexture-1-0"), ("dangling","missing"),
   ("chain","MTLBuffer-77048-1"), ("cycle","cycle"), ("directory","nested")
  ] {
   let link=trace.appendingPathComponent(name)
   try fm.createSymbolicLink(atPath:link.path,withDestinationPath:target)
   await reject(folder,path:"smooth.gputrace/"+name)
   try fm.removeItem(at:link)
  }
  let cycleA=trace.appendingPathComponent("cycle-a"),cycleB=trace.appendingPathComponent("cycle-b")
  try fm.createSymbolicLink(atPath:cycleA.path,withDestinationPath:"cycle-b")
  try fm.createSymbolicLink(atPath:cycleB.path,withDestinationPath:"cycle-a")
  await reject(folder,path:"smooth.gputrace/cycle-")
  try fm.removeItem(at:cycleA);try fm.removeItem(at:cycleB)
  let sidecar=folder.appendingPathComponent("smooth-stages.json")
  try fm.createSymbolicLink(atPath:sidecar.path,withDestinationPath:"session.txt")
  await reject(folder,path:"smooth-stages.json");try fm.removeItem(at:sidecar)
  for name in ["manifest.json","session.txt"] {
   let file=folder.appendingPathComponent(name),backup=root.appendingPathComponent("backup-"+name)
   try fm.moveItem(at:file,to:backup)
   try fm.createSymbolicLink(at:file,withDestinationURL:backup)
   await reject(folder);try fm.removeItem(at:file);try fm.moveItem(at:backup,to:file)
  }
  let moved=root.appendingPathComponent("moved-trace")
  try fm.moveItem(at:trace,to:moved);try fm.createSymbolicLink(at:trace,withDestinationURL:moved)
  await reject(folder);try fm.removeItem(at:trace);try fm.moveItem(at:moved,to:trace)
  await reject(root.appendingPathComponent("00000000-0000-0000-0000-000000000005"))
  print("PASS: production ZIP extraction reads Metal sibling aliases; absolute/traversal/dangling/chain/cycle/directory/sidecar/metadata/root links remain rejected")
  print("PASS: production background ZIP export, original session persistence and unexpected-file rejection")
 }
}
''',encoding='utf-8')
        subprocess.run([swift,'-parse-as-library',str(source),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=60)
        model=text[text.index('@MainActor final class GPUTraceComparison:'):text.index('/// Enqueue synchronously')]
        drop=text.split('// BEGIN GPU_TRACE_DROP_POLICY',1)[1].split('// END GPU_TRACE_DROP_POLICY',1)[0]
        source=out/'model.swift';binary=out/'model'
        source.write_text('import Foundation\nimport Combine\n'+error+snapshot+files+drop+model+r'''
enum DiagnosticCompanionLogs { struct Source { let url: URL; let name: String } }
enum UIApplication {
 static let willResignActiveNotification=Notification.Name("fixture-inactive")
 static let didBecomeActiveNotification=Notification.Name("fixture-active")
}
func madeira_frame_count() -> UInt64 { UInt64(ProcessInfo.processInfo.systemUptime * 11) }
func madeira_get_vsync_locked() -> Int32 { 1 }
enum GPUTraceNative {
 typealias Status = (phase:Int,buffers:Int,frames:Int,started:Double,error:String,captureComplete:Bool)
 @MainActor static var requests = 0
 @MainActor static var result: Status = (0,0,0,0,"",false)
 @MainActor static var nextResult: Status?
 @MainActor static var delayPoll = false
 @MainActor static var pendingPoll: (@MainActor (Status) -> Void)?
 @MainActor static var delayCancel = false
 @MainActor static var pendingCancellations: [@MainActor () -> Void] = []
 @MainActor static func cancel(_ done: @escaping @MainActor (Status) -> Void) {
  if !result.captureComplete { result.phase=5;result.error="Fixture capture cancelled" }
  let cancelled=result
  if delayCancel {pendingCancellations.append({done(cancelled)})} else {done(cancelled)}
 }
 @MainActor static func request(_ file: URL, done: @escaping @MainActor (Bool,String) -> Void) {
  requests += 1
  try! Data("CONTROLLED GPU CAPTURE FIXTURE; NOT REAL TIMINGS".utf8).write(to:file)
  result=nextResult ?? (4,2,2,ProcessInfo.processInfo.systemUptime,"",true);nextResult=nil
  done(true, "")
 }
 @MainActor static func poll(_ done: @escaping @MainActor (Status) -> Void) {
  if delayPoll {pendingPoll=done} else {done(result)}
 }
}
@MainActor final class LogStore {
 static let shared=LogStore(); var source=URL(fileURLWithPath:"/unused");var title="REPLACED"
 func fileForUpload(previous:Bool)->URL {source}
 func metadataForUpload(previous:Bool)->[String:String] { ["gameTitle":title,"build":"fixture"] }
 func log(_ text:String) {print(text)}
}
// A command-line MainActor has no UIKit run loop. Drive the production tick
// explicitly; the real five-second scheduling Task and export queue still run.
extension GPUTraceComparison {
 @MainActor func fixtureTick() { timer?.invalidate();timer=nil;tick() }
 @MainActor func fixtureCapture() { timer?.invalidate();timer=nil;manualScheduled=false;capture("smooth") }
 @MainActor var fixtureFolder: URL { activeFolder! }
}
@main struct CheckModel {
 @MainActor static func main() async throws {
  let fm=FileManager.default, docs=fm.temporaryDirectory.appendingPathComponent(UUID().uuidString).resolvingSymlinksInPath()
  let root=docs.appendingPathComponent("GPUTraces")
  try fm.createDirectory(at:root,withIntermediateDirectories:true)
  defer {try? fm.removeItem(at:docs)}
  let original=root.appendingPathComponent("480d7618-eb74-452f-b921-3e4fcc2d0d9d",isDirectory:true)
  let unfinished=root.appendingPathComponent("296b2076-ee73-46ac-a2bb-16bf823c3de7",isDirectory:true)
  for folder in [original,unfinished] {try fm.createDirectory(at:folder,withIntermediateDirectories:false)}
  let manifest:[String:Any] = ["format":"madeira-gpu-comparison-v1", "createdAt":"2026-10-17T03:43:00Z",
   "smooth":["file":"smooth.gputrace","frames":2,"complete":true],"slow":["file":"slow.gputrace","frames":2,"complete":true]]
  let originalMetadata=try JSONSerialization.data(withJSONObject:manifest)
  try originalMetadata.write(to:original.appendingPathComponent("manifest.json"))
  for kind in ["smooth","slow"] {try Data("GPU fixture".utf8).write(to:original.appendingPathComponent(kind+".gputrace"))}
  try Data("CAPTURED GAME SESSION".utf8).write(to:original.appendingPathComponent("session.txt"))
  try Data("{\"format\":\"madeira-gpu-comparison-v1\"}".utf8).write(to:unfinished.appendingPathComponent("manifest.json"))
  let input=docs.appendingPathComponent("input.txt");try Data("LATER SESSION".utf8).write(to:input);LogStore.shared.source=input
  let suite="madeira.gpu-trace.fixture."+UUID().uuidString,prefs=UserDefaults(suiteName:suite)!
  defer {prefs.removePersistentDomain(forName:suite)}
  prefs.set(unfinished.lastPathComponent,forKey:"madeira.gpuTrace.latest")
  let model=GPUTraceComparison(documents:docs,preferences:prefs)
  model.restoreSaved()
  for _ in 0..<300 {if !model.findingSaved {break};try await Task.sleep(nanoseconds:10_000_000)}
  assert(!model.findingSaved && model.savedFolder?.lastPathComponent==original.lastPathComponent && model.savedComparisons.count==1)
  assert(prefs.string(forKey:"madeira.gpuTrace.latest")==original.lastPathComponent)
  assert(!model.eligibleSession && !model.canStartRecording && !model.start())
  // Renaming the library title cannot hide recording for the same EXE.
  model.session(title:"Custom library title",executable:"Games/REPLACED Viet Hoa/Replaced.exe")
  assert(model.eligibleSession && !model.canStartRecording && !model.start())
  model.gameDidBecomeVisible();assert(model.canStartRecording && model.recordHint==nil)
  assert(model.start())
  for _ in 0..<300 {if !model.busy {break};try await Task.sleep(nanoseconds:10_000_000)}
  assert(model.recording && model.savedFolder?.lastPathComponent==original.lastPathComponent)
  assert(prefs.string(forKey:"madeira.gpuTrace.latest")==original.lastPathComponent)
  model.interrupt("Fixture stops new incomplete run")
  assert(!model.recording && model.savedFolder?.lastPathComponent==original.lastPathComponent)
  let zip=try await model.export(folder:model.savedFolder!);defer {try? fm.removeItem(at:zip)}
  let cached=try await model.export(folder:model.savedFolder!);assert(zip==cached && !model.exporting)
  let metadataAfter=try Data(contentsOf:original.appendingPathComponent("manifest.json"))
  assert(metadataAfter==originalMetadata)
  let logAfter=try String(contentsOf:original.appendingPathComponent("session.txt"),encoding:.utf8)
  assert(logAfter=="CAPTURED GAME SESSION")
  model.session(title:"Other game",executable:"Games/Other/Game.exe");model.gameDidBecomeVisible()
  assert(!model.eligibleSession && !model.canStartRecording && !model.start() && model.recordHint != nil)
  // The production model accepts both title spellings and an EXE after a
  // user renames the card. Merely opening this session does not start sampling.
  for title in ["Liar's Bar", "Liar’s Bar"] {
   model.session(title:title);assert(model.eligibleSession && !model.canStartRecording)
  }
  model.session(title:"Custom title",executable:"Games/LiarsBar/Liar's Bar.exe")
  model.gameDidBecomeVisible();assert(model.canStartRecording && GPUTraceNative.requests==0)
  LogStore.shared.title="Liar's Bar"
  model.setMenuOpen(true)
  assert(model.start(singleSample:true))
  model.setMenuOpen(false)
  for _ in 0..<300 {if !model.busy {break};try await Task.sleep(nanoseconds:10_000_000)}
  model.fixtureTick()
  for _ in 0..<30 {
   if !model.busy && !model.recording && !model.findingSaved {break}
   try await Task.sleep(nanoseconds:700_000_000);model.fixtureTick()
  }
  assert(!model.recording && !model.busy && !model.canCaptureSlow && GPUTraceNative.requests==1)
  let single=model.savedFolder!;assert(single != original)
  assert(model.savedComparisons.first?.title=="Liar's Bar")
  let singleManifest=try JSONSerialization.jsonObject(with:Data(contentsOf:single.appendingPathComponent("manifest.json"))) as! [String:Any]
  assert(singleManifest["sampleMode"] as? String == "single-scene")
  assert(singleManifest["referenceSelection"] as? String == "manual-current-scene")
  let lowFPS=singleManifest["referenceFPS"] as! Double
  assert(lowFPS > 0 && lowFPS < 20)
  assert(!fm.fileExists(atPath:single.appendingPathComponent("slow.gputrace").path))
  let singleZIP=try await model.export(folder:single);defer {try? fm.removeItem(at:singleZIP)}
  assert(fm.fileExists(atPath:singleZIP.path))
  var invalidMode=singleManifest;invalidMode["sampleMode"]="unknown"
  GPUTraceFiles.writeManifest(invalidMode,folder:single)
  var rejectedMode=false
  do {_=try await GPUTraceFiles.export(folder:single)}catch{rejectedMode=true}
  assert(rejectedMode)
  GPUTraceFiles.writeManifest(singleManifest,folder:single)
  // The user opens the menu after two raw frames, while the stage pass or
  // its write is still pending. A stale poll must not overwrite completion.
  GPUTraceNative.nextResult=(3,4,2,123,"",true)
  assert(model.start(singleSample:true))
  for _ in 0..<300 {if !model.busy {break};try await Task.sleep(nanoseconds:10_000_000)}
  model.fixtureCapture();let completedBeforeMenu=model.fixtureFolder
  GPUTraceNative.delayPoll=true;model.fixtureTick();assert(GPUTraceNative.pendingPoll != nil)
  GPUTraceNative.delayCancel=true
  model.setMenuOpen(true)
  assert(model.busy && !model.canStartRecording && !model.start())
  // A session notification can supersede the first cancellation while its
  // native work is still pending. Only the latest callback may release busy.
  model.session(title:"REPLACED");model.gameDidBecomeVisible()
  assert(model.busy && !model.canStartRecording && !model.start())
  let cancellations=GPUTraceNative.pendingCancellations;GPUTraceNative.pendingCancellations=[]
  assert(cancellations.count==2)
  cancellations[0]();assert(model.busy)
  cancellations[1]();GPUTraceNative.delayCancel=false
  let stalePoll=GPUTraceNative.pendingPoll;GPUTraceNative.pendingPoll=nil;GPUTraceNative.delayPoll=false
  stalePoll?((5,0,0,0,"STALE FAILED POLL",false))
  for _ in 0..<300 {if !model.busy && !model.findingSaved {break};try await Task.sleep(nanoseconds:10_000_000)}
  let recovered=try JSONSerialization.jsonObject(with:Data(contentsOf:completedBeforeMenu.appendingPathComponent("manifest.json"))) as! [String:Any]
  let raw=recovered["smooth"] as! [String:Any]
  assert(raw["complete"] as? Bool == true && raw["frames"] as? Int == 2 && raw["buffers"] as? Int == 4)
  assert(raw["captureUptime"] as? Double == 123 && raw["error"] as? String == "")
  assert((recovered["context"] as! [String:Any])["gameTitle"] as? String == "Liar's Bar")
  assert(model.savedFolder==completedBeforeMenu)
  let recoveredZIP=try await model.export(folder:completedBeforeMenu);defer {try? fm.removeItem(at:recoveredZIP)}
  // A genuinely unfinished raw trace remains incomplete and unexportable.
  GPUTraceNative.nextResult=(3,1,1,124,"",false)
  assert(model.start(singleSample:true))
  model.setMenuOpen(false)
  for _ in 0..<300 {if !model.busy {break};try await Task.sleep(nanoseconds:10_000_000)}
  model.fixtureCapture();let incomplete=model.fixtureFolder;model.setMenuOpen(true)
  for _ in 0..<300 {if !model.busy && !model.findingSaved {break};try await Task.sleep(nanoseconds:10_000_000)}
  let stopped=try JSONSerialization.jsonObject(with:Data(contentsOf:incomplete.appendingPathComponent("manifest.json"))) as! [String:Any]
  assert((stopped["smooth"] as! [String:Any])["complete"] as? Bool == false)
  var rejectedIncomplete=false
  do {_=try await GPUTraceFiles.export(folder:incomplete)}catch{rejectedIncomplete=true}
  assert(rejectedIncomplete && model.savedFolder==completedBeforeMenu)
  print("PASS: menu/session interruption preserves finished raw capture while timings are pending, ignores stale callbacks, blocks overlapping requests and rejects incomplete raw captures")
  print("PASS: Liar's Bar title/EXE eligibility, idle capture off, one low-FPS scene at11, completed single ZIP and unknown-mode rejection")
  print("PASS: actual GPU comparison model restores727 recording, preserves it across a new start/stop and exports/caches its original ZIP and session")
  print("PASS: GPU record readiness rejects home/loading/other games, recognizes a renamed REPLACED by EXE path and enables after its first frame")
 }
}
''',encoding='utf-8')
        subprocess.run([swift,'-swift-version','5','-parse-as-library',str(source),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=60)
        native=text[text.index('enum GPUTraceNative {'):text.index('enum GPUTraceFiles {')]
        source=out/'native-control.swift';binary=out/'native-control'
        source.write_text('import Foundation\n'+native+r'''
struct NativeStatus { var phase:Int32;var buffers,frames:UInt32;var started:Double;var captureComplete:Int32 }
// These entry points model only the native status/write boundary. The next
// harness runs the actual Objective-C capture and stage implementations.
var nativeRequests=0,nativePolls=0,nativeCancels=0
var nativeComplete=true
func madeira_gpu_trace_request(_ path:UnsafePointer<CChar>)->Int32 {nativeRequests+=1;return 1}
func madeira_gpu_trace_error()->String? {nil}
func madeira_gpu_trace_cancel() {assert(nativeRequests==1);nativeCancels+=1}
func madeira_gpu_trace_status()->NativeStatus {
 nativePolls+=1
 return NativeStatus(phase:nativeComplete ? (nativePolls<3 ? 3:4):5,buffers:4,frames:2,started:123,captureComplete:nativeComplete ? 1:0)
}
@main struct NativeControl {
 @MainActor static func main() async {
  var accepted=false
  let completed:GPUTraceNative.Status=await withCheckedContinuation { continuation in
   GPUTraceNative.request(URL(fileURLWithPath:"/fixture.gputrace")) { success,_ in accepted=success }
   GPUTraceNative.cancel { continuation.resume(returning:$0) }
  }
  assert(accepted && nativeRequests==1 && nativeCancels==1 && nativePolls==3)
  assert(completed.phase==4 && completed.captureComplete && completed.frames==2 && completed.buffers==4)
  nativeComplete=false;nativePolls=0
  let incomplete:GPUTraceNative.Status=await withCheckedContinuation { continuation in
   GPUTraceNative.cancel { continuation.resume(returning:$0) }
  }
  assert(nativePolls==1 && incomplete.phase==5 && !incomplete.captureComplete)
  print("PASS: production native control queue preserves request/cancel order and waits for the stage sidecar without treating raw completion as failure")
 }
}
''',encoding='utf-8')
        subprocess.run([swift,'-swift-version','5','-parse-as-library',str(source),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=30)
        source=out/'controller.m';binary=out/'controller'
        source.write_text(r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <assert.h>
#include <unistd.h>
#include "GPUTraceCapture.h"
@interface FakeManager : NSObject
@property BOOL isCapturing;
@property BOOL supported;
@property unsigned starts, stops;
@property BOOL refuse;
@end
@implementation FakeManager
- (BOOL)supportsDestination:(MTLCaptureDestination)d { return self.supported; }
- (BOOL)startCaptureWithDescriptor:(MTLCaptureDescriptor *)d error:(NSError **)error {
 assert(d.captureObject && d.destination==MTLCaptureDestinationGPUTraceDocument);
 if (self.refuse) return NO;
 self.starts++; self.isCapturing=YES;return YES;
}
- (void)stopCapture { assert(self.isCapturing);self.isCapturing=NO;self.stops++; }
@end
@interface FakeBuffer : NSObject
@property id commandQueue;
@end
@implementation FakeBuffer @end
@interface FakeDevice : NSObject @end
@implementation FakeDevice
- (BOOL)supportsCounterSampling:(MTLCounterSamplingPoint)point { return YES; }
@end
@interface FakeQueue : NSObject
@property(nonatomic, strong) id device;
@end
@implementation FakeQueue @end
// Exercise the real pending-stage status, including its asynchronous writer.
int madeira_gpu_stage_hooks_version(void) { return 1; }
static FakeManager *manager;
MTLCaptureManager *madeira_test_capture_manager(void) { return (id)manager; }
static void *ptr(id value) { return (__bridge void *)value; }
static FakeBuffer *buffer(id queue) { FakeBuffer *b=[FakeBuffer new];b.commandQueue=queue;return b; }
int main(void) { @autoreleasepool {
 manager=[FakeManager new];manager.supported=YES;
 FakeQueue *q=[FakeQueue new];q.device=[FakeDevice new];NSObject *other=[NSObject new];
 FakeBuffer *boundary=buffer(q);
 NSString *path=[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".gputrace"]];
 assert(madeira_gpu_trace_request(path.UTF8String));
 assert(!madeira_gpu_trace_request(path.UTF8String));
 assert(!madeira_gpu_trace_will_create(ptr(q)) && manager.starts==0);
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 assert(!madeira_gpu_trace_will_create(ptr(other)) && manager.starts==0);
 uint64_t token=madeira_gpu_trace_will_create(ptr(q));assert(token && manager.starts==1);
 // Already encoded buffers cannot complete a captured frame.
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 assert(madeira_gpu_trace_status().frames==0);
 for(int i=0;i<2;i++) {
   FakeBuffer *draw=buffer(q);madeira_gpu_trace_did_create(ptr(draw),token);madeira_gpu_trace_did_commit(ptr(draw));
   FakeBuffer *present=buffer(q);madeira_gpu_trace_did_create(ptr(present),token);
   madeira_gpu_trace_present(ptr(present));assert(manager.isCapturing);
   madeira_gpu_trace_did_commit(ptr(present));
 }
 MadeiraGPUTraceStatus s=madeira_gpu_trace_status();assert(s.phase==3 && s.captureComplete && s.frames==2 && s.buffers==4);
 assert(manager.stops==1 && !manager.isCapturing);
 madeira_gpu_trace_cancel();assert(manager.stops==1 && madeira_gpu_trace_status().captureComplete);
 for(int i=0;i<300 && madeira_gpu_trace_status().phase!=4;i++) usleep(10000);
 assert(madeira_gpu_trace_status().phase==4 && madeira_gpu_trace_status().captureComplete);
 NSString *stages=[path.stringByDeletingPathExtension stringByAppendingString:@"-stages.json"];
 NSDictionary *timings=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfFile:stages] options:0 error:nil];
 assert(timings && ![timings[@"complete"] boolValue] && [timings[@"error"] isEqualToString:@"Stage sampling cancelled."]);
 [NSFileManager.defaultManager removeItemAtPath:stages error:nil];
 // A foreign/Xcode capture is never stopped by us.
 manager.isCapturing=YES;
 assert(madeira_gpu_trace_request(path.UTF8String));
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 assert(!madeira_gpu_trace_will_create(ptr(q)));
 assert(madeira_gpu_trace_status().phase==5 && manager.stops==1 && manager.isCapturing);
 manager.isCapturing=NO; manager.supported=NO;
 assert(madeira_gpu_trace_request(path.UTF8String));
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 assert(!madeira_gpu_trace_will_create(ptr(q)) && madeira_gpu_trace_status().phase==5);
 manager.supported=YES;manager.refuse=YES;
 assert(madeira_gpu_trace_request(path.UTF8String));
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 assert(!madeira_gpu_trace_will_create(ptr(q)) && madeira_gpu_trace_status().phase==5);
 manager.refuse=NO;
 assert(madeira_gpu_trace_request(path.UTF8String));
 madeira_gpu_trace_present(ptr(boundary));madeira_gpu_trace_did_commit(ptr(boundary));
 uint64_t newToken=madeira_gpu_trace_will_create(ptr(q));assert(newToken && newToken!=token);
 FakeBuffer *late=buffer(q);madeira_gpu_trace_did_create(ptr(late),token);
 madeira_gpu_trace_present(ptr(late));madeira_gpu_trace_did_commit(ptr(late));
 assert(madeira_gpu_trace_status().buffers==0 && madeira_gpu_trace_status().frames==0);
 madeira_gpu_trace_cancel();assert(manager.stops==2 && madeira_gpu_trace_status().phase==5 && !madeira_gpu_trace_status().captureComplete);
 puts("PASS: completed raw status survives cancellation of the real following-frame stage sampler; its sidecar accurately remains incomplete");
 puts("PASS: production Objective-C capture controller, creation-before-encoding, two present commits, stale callbacks, failures and foreign capture ownership");
} }
''',encoding='utf-8')
        subprocess.run(['xcrun','clang','-fobjc-arc','-fblocks','-DMADEIRA_GPU_TRACE_TEST=1','-I',str(root/'app/Madeira'),
                        str(source),str(root/'app/Madeira/GPUTraceCapture.m'),'-framework','Foundation','-framework','Metal','-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=30)
        source=out/'metal-canary.m';binary=out/'metal-canary'
        source.write_text(r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <assert.h>
#include "GPUTraceCapture.h"
static void *ptr(id x) { return (__bridge void *)x; }
int main(void) { @autoreleasepool {
 id<MTLDevice> device=MTLCreateSystemDefaultDevice();
 if(!device || ![MTLCaptureManager.sharedCaptureManager supportsDestination:MTLCaptureDestinationGPUTraceDocument]) {
  puts("SKIP: real Metal GPU trace capture is not available on this macOS runner");return 0;
 }
 id<MTLCommandQueue> queue=[device newCommandQueue];assert(queue);
 MTLTextureDescriptor *td=[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:64 height:64 mipmapped:NO];
 td.usage=MTLTextureUsageRenderTarget;id<MTLTexture> texture=[device newTextureWithDescriptor:td];assert(texture);
 NSString *path=[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".gputrace"]];
 assert(madeira_gpu_trace_request(path.UTF8String));
 // A logical presentation boundary brackets the test workload. This canary
 // has no display layer and is not an on-device frame-rate measurement.
 id<MTLCommandBuffer> boundary=[queue commandBuffer];
 madeira_gpu_trace_present(ptr(boundary));[boundary commit];madeira_gpu_trace_did_commit(ptr(boundary));
 for(int frame=0;frame<2;frame++) {
  uint64_t ticket=madeira_gpu_trace_will_create(ptr(queue));assert(ticket);
  id<MTLCommandBuffer> cb=[queue commandBuffer];madeira_gpu_trace_did_create(ptr(cb),ticket);
  MTLRenderPassDescriptor *rp=[MTLRenderPassDescriptor renderPassDescriptor];
  rp.colorAttachments[0].texture=texture;rp.colorAttachments[0].loadAction=MTLLoadActionClear;
  rp.colorAttachments[0].storeAction=MTLStoreActionStore;
  rp.colorAttachments[0].clearColor=MTLClearColorMake(frame,0,1,1);
  id<MTLRenderCommandEncoder> encoder=[cb renderCommandEncoderWithDescriptor:rp];assert(encoder);[encoder endEncoding];
  madeira_gpu_trace_present(ptr(cb));[cb commit];madeira_gpu_trace_did_commit(ptr(cb));[cb waitUntilCompleted];
  assert(cb.status==MTLCommandBufferStatusCompleted);
 }
 MadeiraGPUTraceStatus status=madeira_gpu_trace_status();assert(status.phase==4 && status.captureComplete && status.frames==2 && status.buffers==2);
 assert([NSFileManager.defaultManager fileExistsAtPath:path]);
 NSDictionary *attrs=[NSFileManager.defaultManager attributesOfItemAtPath:path error:nil];assert(attrs);
 printf("PASS: real public Metal API saves a two-workload GPU trace (display/FPS not tested)\n");
 const char *exportPath=getenv("MADEIRA_GPU_TRACE_CANARY_PATH");
 if(exportPath) assert([NSFileManager.defaultManager moveItemAtPath:path toPath:[NSString stringWithUTF8String:exportPath] error:nil]);
 else [NSFileManager.defaultManager removeItemAtPath:path error:nil];
} }
''',encoding='utf-8')
        subprocess.run(['xcrun','clang','-fobjc-arc','-fblocks','-I',str(root/'app/Madeira'),str(source),
                        str(root/'app/Madeira/GPUTraceCapture.m'),'-framework','Foundation','-framework','Metal','-o',str(binary)],check=True)
        canary=out/'metal-canary.gputrace'
        subprocess.run([str(binary)],check=True,timeout=90,env=dict(os.environ,MTL_CAPTURE_ENABLED='1',MADEIRA_GPU_TRACE_CANARY_PATH=str(canary)))
        if canary.exists():
            source=out/'canary-export.swift';binary=out/'canary-export'
            source.write_text('import Foundation\n'+error+snapshot+files+unpack+r'''
enum DiagnosticCompanionLogs { struct Source { let url: URL; let name: String } }
@main struct CanaryExport {
 static func main() async throws {
  let fm=FileManager.default, original=URL(fileURLWithPath:CommandLine.arguments[1])
  let root=fm.temporaryDirectory.appendingPathComponent(UUID().uuidString).resolvingSymlinksInPath()
  let folder=root.appendingPathComponent(UUID().uuidString)
  try fm.createDirectory(at:folder,withIntermediateDirectories:true)
  defer {try? fm.removeItem(at:root)}
  try fm.copyItem(at:original,to:folder.appendingPathComponent("smooth.gputrace"))
  try Data("PUBLIC METAL CAPTURE CANARY; NO GAME OR TIMING CLAIMS".utf8).write(to:folder.appendingPathComponent("session.txt"))
  GPUTraceFiles.writeManifest(["format":"madeira-gpu-comparison-v1","sampleMode":"single-scene",
   "smooth":["file":"smooth.gputrace","complete":true,"frames":2]],folder:folder)
  let archive=try await GPUTraceFiles.export(folder:folder,directory:root)
  let extracted=try unpackComparison(archive,into:root.appendingPathComponent("extracted"))
  let trace=extracted.appendingPathComponent("smooth.gputrace"), prefix=original.path+"/"
  let walker=fm.enumerator(at:original,includingPropertiesForKeys:[.isDirectoryKey])!
  var files=0
  for case let file as URL in walker {
   if try file.resourceValues(forKeys:[.isDirectoryKey]).isDirectory==true {continue}
   let relative=String(file.path.dropFirst(prefix.count))
   let expected=try Data(contentsOf:file),actual=try Data(contentsOf:trace.appendingPathComponent(relative))
   assert(expected==actual,"Exported Metal resource changed: "+relative);files+=1
  }
  assert(files>0)
  print("PASS: real Metal canary passes production export and every extracted resource retains its readable payload")
 }
}
''',encoding='utf-8')
            subprocess.run([swift,'-parse-as-library',str(source),'-o',str(binary)],check=True)
            subprocess.run([str(binary),str(canary)],check=True,timeout=90)
        probe=subprocess.run(['xcrun','--find','gpudebug'],capture_output=True,text=True)
        if canary.exists() and probe.returncode==0:
            result=subprocess.run([probe.stdout.strip(),'--oneshot','-t',str(canary),'-c','list'],capture_output=True,text=True,timeout=90)
            print('Apple gpudebug canary exit:',result.returncode)
            print(result.stdout[-4000:]);print(result.stderr[-1000:])
            assert result.returncode==0,'real captured workload could not be read by Apple gpudebug'
    elif a.require_darwin: raise SystemExit('Darwin required')
    else: print('SKIP: Objective-C/Metal compile and controller execution require macOS; CI must run them')

spec=importlib.util.spec_from_file_location('pipeline',root/'tests/host/check-dxmt-patches.py')
pipeline=importlib.util.module_from_spec(spec);spec.loader.exec_module(pipeline)
source=pipeline.patched_sources(root)['src/winemetal/unix/winemetal_unix.c']
create=source.split('_MTLCommandQueue_commandBuffer(void *obj)',1)[1].split('static NTSTATUS',1)[0]
assert create.index('madeira_gpu_trace_will_create(')<create.index(' commandBuffer]')<create.index('madeira_gpu_trace_did_create(' ,create.index(' commandBuffer]'))
commit=source.split('_MTLCommandBuffer_commit(void *obj)',1)[1].split('static NTSTATUS',1)[0]
assert commit.index(' commit]')<commit.index('madeira_gpu_trace_did_commit(',commit.index(' commit]'))
print('PASS: complete ordered production DXMT pipeline hooks capture before creation and after commit')
ui=(root/'app/Madeira/GPUTraceComparison.swift').read_text(encoding='utf-8').split('struct GPUTraceComparisonControls:',1)[1]
support=(root/'app/Madeira/SupportViews.swift').read_text(encoding='utf-8')
library=(root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'Button("Record FPS drop"' in ui and 'Record another comparison' not in ui and 'if canStart' not in ui
assert 'GPUTraceComparisonControls { dismiss(); resumeGame() }' in support
assert 'DiagnosticUploadView(gameTitle: model.activeEntry?.title, resumeGame: { model.menu = false })' in library
print('PASS: stable Record FPS drop label in both views; diagnostics start dismisses its sheet and resumes the game menu')
