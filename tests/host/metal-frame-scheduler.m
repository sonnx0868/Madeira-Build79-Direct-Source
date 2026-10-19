#import <AppKit/AppKit.h>
#include <assert.h>
#include <unistd.h>
#include "MetalFrameScheduler.h"

static NSUInteger pending(MadeiraMetalFrameScheduler *scheduler) {
    NSCondition *condition = [scheduler valueForKey:@"condition"];
    [condition lock];
    NSUInteger count = [[scheduler valueForKey:@"requests"] count];
    [condition unlock]; return count;
}
static void awaitPending(MadeiraMetalFrameScheduler *scheduler, NSUInteger count) {
    CFTimeInterval limit = CACurrentMediaTime() + 0.3;
    while (pending(scheduler) != count && CACurrentMediaTime() < limit) usleep(100);
    assert(pending(scheduler) == count);
}
static void pumpUntil(BOOL (^done)(void), double seconds) {
    CFTimeInterval limit = CACurrentMediaTime() + seconds;
    while (!done() && CACurrentMediaTime() < limit)
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];
    assert(done());
}
static void queueTests(void) {
    MadeiraMetalFrameScheduler *scheduler = [MadeiraMetalFrameScheduler new];
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = MTLCreateSystemDefaultDevice();
    CALayer *parent = [CALayer layer]; [parent addSublayer:layer];
    layer.drawableSize = CGSizeMake(32, 32);
    [scheduler bindLayer:layer]; [scheduler setForeground:YES]; [scheduler setSessionEnabled:YES];
    int handled = -1;
    assert([scheduler nextForLayer:[CAMetalLayer layer] handled:&handled] == NULL && !handled);
    assert([scheduler nextForLayer:layer handled:&handled] == NULL && handled); // main must never wait
    dispatch_queue_t worker = dispatch_queue_create("frame-test.worker", DISPATCH_QUEUE_SERIAL);
    dispatch_semaphore_t done = dispatch_semaphore_create(0);
    __block void *result;
    __block int status;
    dispatch_async(worker, ^{
        @autoreleasepool { result = [scheduler nextForLayer:layer handled:&status]; dispatch_semaphore_signal(done); }
    });
    awaitPending(scheduler, 1);
    NSObject *frame = [NSObject new];
    [scheduler offerDrawable:(id)frame deadline:CACurrentMediaTime() - 1];
    assert(pending(scheduler) == 1);
    [scheduler offerDrawable:(id)frame deadline:CACurrentMediaTime() + 1];
    assert(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC)) == 0);
    assert(status && result == (__bridge void *)frame);
    id owned = CFBridgingRelease(result); result = NULL;
    assert(owned == frame);
    assert(madeira_display_link_take_drawable((__bridge void *)owned) == 1);
    assert(madeira_display_link_take_drawable((__bridge void *)owned) == 0);
    [scheduler offerDrawable:(id)frame deadline:CACurrentMediaTime() + 1];
    assert(pending(scheduler) == 0); // unused frames are never queued
    dispatch_async(worker, ^{
        @autoreleasepool { result = [scheduler nextForLayer:layer handled:&status]; dispatch_semaphore_signal(done); }
    });
    awaitPending(scheduler, 1);
    [scheduler setSessionEnabled:NO];
    assert(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC)) == 0);
    assert(status && !result && pending(scheduler) == 0);
    [scheduler setSessionEnabled:YES];
    dispatch_async(worker, ^{
        @autoreleasepool { result = [scheduler nextForLayer:layer handled:&status]; dispatch_semaphore_signal(done); }
    });
    awaitPending(scheduler, 1);
    [scheduler setRate:30]; // cancels the old epoch, no old-rate drawable escapes
    assert(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC)) == 0);
    assert(status && !result);
    CAMetalDisplayLink *link = [scheduler valueForKey:@"link"];
    assert(link.preferredFrameRateRange.preferred == 30 && link.preferredFrameRateRange.maximum == 30);
    assert(link.preferredFrameLatency == 2);
    [scheduler setForeground:NO];
    assert(![scheduler valueForKey:@"link"]);
    [scheduler setForeground:YES];
    // No event-loop pumping: a missing display source must time out and then
    // fully invalidate on main before permitting native nextDrawable fallback.
    dispatch_async(worker, ^{
        @autoreleasepool { result = [scheduler nextForLayer:layer handled:&status]; dispatch_semaphore_signal(done); }
    });
    awaitPending(scheduler, 1);
    assert(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC)) == 0);
    assert(status && !result);
    pumpUntil(^BOOL { return [scheduler valueForKey:@"link"] == nil; }, 1);
    assert([scheduler nextForLayer:layer handled:&handled] == NULL && !handled);
    [scheduler setSessionEnabled:NO];
    printf("PASS: frame mailbox deadlines, ownership, bounded main-thread behavior, stop/resume, rate epoch and timeout fallback\n");
}

int main(void) { @autoreleasepool {
    queueTests();
    NSApplication *application = NSApplication.sharedApplication;
    [application setActivationPolicy:NSApplicationActivationPolicyRegular];
    [application finishLaunching];
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 128, 128)
        styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    window.releasedWhenClosed = NO;
    window.contentView.wantsLayer = YES;
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = MTLCreateSystemDefaultDevice();
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = NO;
    layer.drawableSize = CGSizeMake(64, 64);
    layer.frame = CGRectMake(0, 0, 128, 128);
    [window.contentView.layer addSublayer:layer];
    [window makeKeyAndOrderFront:nil]; [application activateIgnoringOtherApps:YES];
    MadeiraMetalFrameScheduler *scheduler = [MadeiraMetalFrameScheduler new];
    [scheduler bindLayer:layer]; [scheduler setForeground:YES]; [scheduler setSessionEnabled:YES];
    id<MTLCommandQueue> queue = [layer.device newCommandQueue];
    id<MTLBuffer> pixels = [layer.device newBufferWithLength:64 * 64 * 4 options:MTLResourceStorageModeShared];
    NSLock *stateLock = [NSLock new];
    __block BOOL finished = NO;
    __block NSUInteger frames = 0;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
        for (unsigned n = 0; n < 8; ++n) { @autoreleasepool {
            int handled = 0;
            void *raw = [scheduler nextForLayer:layer handled:&handled];
            assert(handled && raw);
            id<CAMetalDrawable> drawable = CFBridgingRelease(raw);
            assert(madeira_display_link_take_drawable((__bridge void *)drawable));
            MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
            pass.colorAttachments[0].texture = drawable.texture;
            pass.colorAttachments[0].loadAction = MTLLoadActionClear;
            pass.colorAttachments[0].storeAction = MTLStoreActionStore;
            pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 1, 0, 1);
            id<MTLCommandBuffer> buffer = [queue commandBuffer];
            [[buffer renderCommandEncoderWithDescriptor:pass] endEncoding];
            id<MTLBlitCommandEncoder> blit = [buffer blitCommandEncoder];
            [blit copyFromTexture:drawable.texture sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0)
                 sourceSize:MTLSizeMake(64, 64, 1) toBuffer:pixels destinationOffset:0 destinationBytesPerRow:256 destinationBytesPerImage:16384];
            [blit endEncoding];
            [buffer presentDrawable:drawable]; [buffer commit]; [buffer waitUntilCompleted];
            assert(buffer.status == MTLCommandBufferStatusCompleted);
            unsigned char *bytes = pixels.contents;
            for (unsigned i = 0; i < 64 * 64; ++i)
                assert(bytes[i*4] == 0 && bytes[i*4+1] == 255 && bytes[i*4+2] == 0 && bytes[i*4+3] == 255);
            [stateLock lock]; ++frames; [stateLock unlock];
        }}
        [stateLock lock]; finished = YES; [stateLock unlock];
    });
    pumpUntil(^BOOL { [stateLock lock]; BOOL value = finished; [stateLock unlock]; return value; }, 10);
    [scheduler setSessionEnabled:NO];
    [window close];
    assert(frames == 8);
    printf("PASS: real CAMetalDisplayLink supplied eight drawables/deadlines; Metal commands completed and all output pixels matched\n");
    printf("NOTE: functional host validation, not REPLACED FPS, iPad clock or thermal validation\n");
    return 0;
}}
