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
        source=out/'export.swift';binary=out/'export'
        source.write_text('import Foundation\n'+error+snapshot+files+r'''
enum DiagnosticCompanionLogs { struct Source { let url: URL; let name: String } }
@main struct Check {
 static func main() async throws {
  let fm=FileManager.default, root=fm.temporaryDirectory.appendingPathComponent(UUID().uuidString)
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
  }
  GPUTraceFiles.writeManifest(manifest,folder:folder)
  GPUTraceFiles.snapshot(log:log,folder:folder)
  let zip=try await GPUTraceFiles.export(folder:folder)
  defer { try? fm.removeItem(at:zip) }
  let header=try FileHandle(forReadingFrom:zip);let signature=try header.read(upToCount:4);try header.close()
  assert(signature==Data([0x50,0x4b,0x03,0x04]))
  try Data("DIFFERENT GAME SESSION".utf8).write(to:log)
  let zip2=try await GPUTraceFiles.export(folder:folder);defer {try? fm.removeItem(at:zip2)}
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
  var rejected=false;do {_=try await GPUTraceFiles.export(folder:folder)}catch{rejected=true};assert(rejected)
  try fm.removeItem(at:injected)
  try fm.createSymbolicLink(at:folder.appendingPathComponent("smooth.gputrace/link"),withDestinationURL:log)
  rejected=false;do {_=try await GPUTraceFiles.export(folder:folder)}catch{rejected=true};assert(rejected)
  print("PASS: production background ZIP export, actual ZIP signature, original session persistence, unexpected-file and symlink rejection")
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
func madeira_frame_count() -> UInt64 { 0 }
func madeira_get_vsync_locked() -> Int32 { 1 }
enum GPUTraceNative {
 @MainActor static func cancel(_ done: @escaping @MainActor () -> Void) { done() }
 @MainActor static func request(_ file: URL, done: @escaping @MainActor (Bool,String) -> Void) { done(true, "") }
 @MainActor static func poll(_ done: @escaping @MainActor ((Int,Int,Int,Double,String)) -> Void) { done((0,0,0,0,"")) }
}
@MainActor final class LogStore {
 static let shared=LogStore(); var source=URL(fileURLWithPath:"/unused")
 func fileForUpload(previous:Bool)->URL {source}
 func metadataForUpload(previous:Bool)->[String:String] { ["gameTitle":"REPLACED","build":"fixture"] }
 func log(_ text:String) {print(text)}
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
  print("PASS: actual GPU comparison model restores727 recording, preserves it across a new start/stop and exports/caches its original ZIP and session")
  print("PASS: GPU record readiness rejects home/loading/other games, recognizes a renamed REPLACED by EXE path and enables after its first frame")
 }
}
''',encoding='utf-8')
        subprocess.run([swift,'-swift-version','5','-parse-as-library',str(source),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=60)
        source=out/'controller.m';binary=out/'controller'
        source.write_text(r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <assert.h>
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
static FakeManager *manager;
MTLCaptureManager *madeira_test_capture_manager(void) { return (id)manager; }
static void *ptr(id value) { return (__bridge void *)value; }
static FakeBuffer *buffer(id queue) { FakeBuffer *b=[FakeBuffer new];b.commandQueue=queue;return b; }
int main(void) { @autoreleasepool {
 manager=[FakeManager new];manager.supported=YES;
 NSObject *q=[NSObject new],*other=[NSObject new];
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
 MadeiraGPUTraceStatus s=madeira_gpu_trace_status();assert(s.phase==4 && s.frames==2 && s.buffers==4);
 assert(manager.stops==1 && !manager.isCapturing);
 madeira_gpu_trace_cancel();assert(manager.stops==1);
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
 madeira_gpu_trace_cancel();assert(manager.stops==2 && madeira_gpu_trace_status().phase==5);
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
 MadeiraGPUTraceStatus status=madeira_gpu_trace_status();assert(status.phase==4 && status.frames==2 && status.buffers==2);
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
