// Test the production cache with deterministic Metal protocol doubles. No GPU
// is required; real Metal descriptor validation and ARC still execute on macOS.
#import "pipeline_cache_ios.m"
#include <assert.h>

@interface LiveFunction : NSObject <MTLFunction>
@property MTLFunctionType functionType;
@end
@implementation LiveFunction
- (MTLPatchType)patchType { return MTLPatchTypeNone; }
@end
@interface LiveArchive : NSObject <MTLBinaryArchive>
@end
@implementation LiveArchive
- (BOOL)addRenderPipelineFunctionsWithDescriptor:(MTLRenderPipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)addComputePipelineFunctionsWithDescriptor:(MTLComputePipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)serializeToURL:(NSURL *)u error:(NSError **)e { return [@"archive-fixture" writeToURL:u atomically:NO encoding:NSUTF8StringEncoding error:e]; }
@end
@interface LiveDevice : NSObject <MTLDevice> {
@public atomic_uint normal, probes, loads;
}
@property BOOL fail, throws, hit, reenter;
@property unsigned delay;
@end
@implementation LiveDevice
- (NSString *)name { return @"live-fixture-gpu"; }
- (id<MTLBinaryArchive>)newBinaryArchiveWithDescriptor:(MTLBinaryArchiveDescriptor *)d error:(NSError **)e {
    if (d.url) {
        atomic_fetch_add(&loads,1);
        if (![[NSString stringWithContentsOfURL:d.url encoding:NSUTF8StringEncoding error:nil] isEqual:@"archive-fixture"]) return nil;
    }
    return [LiveArchive new];
}
- (id)make:(id)d options:(MTLPipelineOption)o error:(NSError **)e {
    if (o & MTLPipelineOptionFailOnBinaryArchiveMiss) { atomic_fetch_add(&probes,1); return self.hit ? [NSObject new] : nil; }
    atomic_fetch_add(&normal,1);
    if (self.reenter) { self.reenter=NO; assert(madeira_new_render_pipeline(self,d,0,NULL,NULL,YES)); }
    if (self.delay) usleep(self.delay);
    if (self.throws) @throw [NSException exceptionWithName:@"native-pipeline-failure" reason:@"fixture" userInfo:nil];
    if (self.fail) { if(e)*e=[NSError errorWithDomain:@"native-error" code:7 userInfo:nil]; return nil; }
    if(e)*e=nil; return [NSObject new];
}
- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)d options:(MTLPipelineOption)o reflection:(MTLRenderPipelineReflection **)r error:(NSError **)e { return [self make:d options:o error:e]; }
- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)d options:(MTLPipelineOption)o reflection:(MTLComputePipelineReflection **)r error:(NSError **)e { return [self make:d options:o error:e]; }
@end
static MTLRenderPipelineDescriptor *render(void) {
    LiveFunction *v=[LiveFunction new]; v.functionType=MTLFunctionTypeVertex;
    MTLRenderPipelineDescriptor *d=[MTLRenderPipelineDescriptor new]; d.vertexFunction=v;
    d.colorAttachments[0].pixelFormat=MTLPixelFormatBGRA8Unorm;
    madeira_pipeline_allow_warmup(d,YES); return d;
}
static void join(dispatch_group_t group) { assert(!dispatch_group_wait(group,dispatch_time(DISPATCH_TIME_NOW,5*NSEC_PER_SEC))); }
static void drain(LiveDevice *d) { MadeiraPipelineCache *s=cacheForDevice(d); dispatch_sync(s->queue,^{flushCache(s);}); }
int main(void) { @autoreleasepool {
    setenv("MADEIRA_PIPELINE_CACHE","1",1); setenv("MADEIRA_SHADER_WARMUP","0",1);
    NSString *dir=[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString];
    setenv("MADEIRA_PIPELINE_CACHE_DIR",dir.UTF8String,1);
    LiveDevice *d=[LiveDevice new]; d.delay=100000;
    MTLRenderPipelineDescriptor *a=render(); NSString *key=madeira_pipeline_runtime_key(a,NO); assert(key);
    NSMutableArray *answers=[NSMutableArray array]; NSLock *answerLock=[NSLock new];
    dispatch_group_t group=dispatch_group_create();
    for(unsigned i=0;i<12;++i) dispatch_group_async(group,dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0),^{ @autoreleasepool {
        id p=madeira_new_render_pipeline(d,a,0,NULL,NULL,YES); assert(p);
        [answerLock lock]; [answers addObject:p]; [answerLock unlock];
    }});
    join(group); assert(atomic_load(&d->normal)==1 && answers.count==12);
    for(id p in answers) assert(p==answers.firstObject);
    d.delay=0; MadeiraPipelineCache *s=cacheForDevice(d);
    assert(s->sharedHits+s->residentHits==11);
    drain(d); assert(s->lookups.count && [NSFileManager.defaultManager fileExistsAtPath:s->file.path]);
    assert(!a.binaryArchives.count); // caller's descriptor was never modified
    MTLRenderPipelineDescriptor *other=render();
    assert(![key isEqual:madeira_pipeline_runtime_key(other,NO)]);
    assert(madeira_new_render_pipeline(d,other,0,NULL,NULL,YES));
    assert(atomic_load(&d->probes)==1); // same-session flush refreshed immutable lookup
    other.colorAttachments[0].blendingEnabled=YES;
    assert(madeira_new_render_pipeline(d,other,0,NULL,NULL,YES));
    other.colorAttachments[0].pixelFormat=MTLPixelFormatRGBA8Unorm;
    assert(madeira_new_render_pipeline(d,other,0,NULL,NULL,YES));
    unsigned before=atomic_load(&d->normal);
    MTLRenderPipelineReflection *reflection=nil;
    assert(madeira_new_render_pipeline(d,a,MTLPipelineOptionArgumentInfo,&reflection,NULL,YES));
    madeira_pipeline_allow_warmup(a,NO);
    assert(madeira_new_render_pipeline(d,a,0,NULL,NULL,YES));
    madeira_pipeline_allow_warmup(a,YES);
    a.binaryArchives=@[[LiveArchive new]];
    assert(madeira_new_render_pipeline(d,a,0,NULL,NULL,YES)); a.binaryArchives=nil;
    assert(madeira_new_render_pipeline(d,a,0,NULL,NULL,NO));
    assert(atomic_load(&d->normal)==before+4); // reflection/private/explicit/caller bypass
    // Failure and native exceptions release all waiters and are not retained
    // as permanent negative entries. A later request is allowed to retry.
    for(unsigned mode=0;mode<2;++mode) {
        LiveDevice *bad=[LiveDevice new]; bad.fail=mode==0; bad.throws=mode==1; bad.delay=100000;
        MTLRenderPipelineDescriptor *b=render(); group=dispatch_group_create();
        for(unsigned i=0;i<8;++i) dispatch_group_async(group,dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0),^{ @autoreleasepool {
            @try { NSError *e=nil; assert(!madeira_new_render_pipeline(bad,b,0,NULL,&e,YES)); assert(!mode && [e.domain isEqual:@"native-error"]); }
            @catch(NSException *e) { assert(mode && [e.name isEqual:@"native-pipeline-failure"]); }
        }});
        join(group); assert(!cacheForDevice(bad)->building.count);
        bad.fail=bad.throws=NO; bad.delay=0;
        unsigned count=atomic_load(&bad->normal); assert(madeira_new_render_pipeline(bad,b,0,NULL,NULL,YES));
        assert(atomic_load(&bad->normal)==count+1);
    }
    LiveDevice *recursive=[LiveDevice new]; recursive.reenter=YES;
    assert(madeira_new_render_pipeline(recursive,render(),0,NULL,NULL,YES));
    assert(atomic_load(&recursive->normal)==2); // native reentry must not await itself
    // Separate writers never share .tmp or output paths. Old canonical files
    // remain readable, and only owned regular shards participate in pruning.
    LiveDevice *cold=[LiveDevice new]; cold.hit=YES;
    assert(madeira_new_render_pipeline(cold,render(),0,NULL,NULL,YES));
    assert(atomic_load(&cold->probes)==1 && !atomic_load(&cold->normal));
    MadeiraPipelineCache *coldState=cacheForDevice(cold); assert(![s->file isEqual:coldState->file]);
    NSURL *folder=s->identityFile.URLByDeletingLastPathComponent;
    NSURL *unrelated=[folder URLByAppendingPathComponent:@"user.metallib"];
    [@"keep" writeToURL:unrelated atomically:NO encoding:NSUTF8StringEncoding error:nil];
    NSString *stem=s->identityFile.lastPathComponent.stringByDeletingPathExtension;
    NSURL *symlink=[folder URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@.metallib",stem,NSUUID.UUID.UUIDString]];
    assert(!symlink.path || !symlink.path.length || ![NSFileManager.defaultManager fileExistsAtPath:symlink.path]);
    assert([NSFileManager.defaultManager createSymbolicLinkAtPath:symlink.path withDestinationPath:unrelated.path error:nil]);
    for(unsigned i=0;i<12;++i) {
        NSURL *f=[folder URLByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@.metallib",stem,NSUUID.UUID.UUIDString]];
        [@"archive-fixture" writeToURL:f atomically:NO encoding:NSUTF8StringEncoding error:nil];
    }
    [diskLock lock]; pruneArchives(s); [diskLock unlock];
    assert(archiveFiles(s->identityFile).count<=diskArchiveLimit);
    assert(readArchives(d,archiveFiles(s->identityFile),NULL).count<=archiveLimit);
    assert([NSFileManager.defaultManager fileExistsAtPath:unrelated.path]);
    assert([NSFileManager.defaultManager destinationOfSymbolicLinkAtPath:symlink.path error:nil]);
    puts("PASS: live PSO reuse, concurrent builds, descriptor separation, error/exception retry, native reentry, live archive refresh and isolated bounded shards");
} return 0; }
