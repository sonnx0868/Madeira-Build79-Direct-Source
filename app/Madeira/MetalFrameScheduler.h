// Private implementation, included once by IOSDisplayShim.m.
// A Metal display link supplies the drawable and frame deadline together.
// Wine/DXMT continues to encode on its existing worker, never on UIKit's thread.
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#import <objc/message.h>
#import <objc/runtime.h>
#include <stdio.h>

static char madeira_managed_drawable_key;

@interface MadeiraFrameRequest : NSObject
@property(strong) id<CAMetalDrawable> drawable;
@property uint64_t generation;
@property BOOL complete;
@end
@implementation MadeiraFrameRequest
@end

@interface MadeiraMetalFrameScheduler : NSObject <CAMetalDisplayLinkDelegate> {
    NSCondition *_condition;
    NSMutableArray<MadeiraFrameRequest *> *_requests;
    CAMetalLayer *_layer;
    CAMetalDisplayLink *_link;
    NSUInteger _rate;
    uint64_t _generation;
    BOOL _session, _foreground, _ownsLayer, _failed, _stopping;
}
+ (instancetype)shared;
- (void)bindLayer:(CAMetalLayer *)layer;
- (void)setSessionEnabled:(BOOL)enabled;
- (void)setForeground:(BOOL)foreground;
- (void)setRate:(NSUInteger)rate;
- (void)reconfigure;
- (void)offerDrawable:(id<CAMetalDrawable>)drawable deadline:(CFTimeInterval)deadline;
- (void *)nextForLayer:(CAMetalLayer *)layer handled:(int *)handled;
@end

@implementation MadeiraMetalFrameScheduler
+ (instancetype)shared {
    static MadeiraMetalFrameScheduler *value;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ value = [self new]; });
    return value;
}
- (instancetype)init {
    if ((self = [super init])) {
        _condition = [NSCondition new]; _requests = [NSMutableArray new];
        _rate = 60; _generation = 1;
    }
    return self;
}
- (void)cancelRequestsLocked {
    for (MadeiraFrameRequest *request in _requests) request.complete = YES;
    [_requests removeAllObjects]; [_condition broadcast];
}
- (void)bindLayer:(CAMetalLayer *)layer {
    NSAssert(NSThread.isMainThread, @"Configure the display link on main");
    [_condition lock];
    BOOL changed = _layer != layer;
    _layer = layer;
    if (changed) { ++_generation; _failed = NO; [self cancelRequestsLocked]; }
    [_condition unlock];
    if (changed) [self reconfigure];
}
- (void)setSessionEnabled:(BOOL)enabled {
    NSAssert(NSThread.isMainThread, @"Configure the display link on main");
    [_condition lock];
    ++_generation; _session = enabled; _failed = NO;
    [self cancelRequestsLocked]; [_condition unlock];
    [self reconfigure];
}
- (void)setForeground:(BOOL)foreground {
    NSAssert(NSThread.isMainThread, @"Configure the display link on main");
    [_condition lock];
    BOOL changed = _foreground != foreground;
    _foreground = foreground;
    if (changed) { ++_generation; [self cancelRequestsLocked]; }
    [_condition unlock];
    if (changed) [self reconfigure];
}
- (void)setRate:(NSUInteger)rate {
    NSAssert(NSThread.isMainThread, @"Configure the display link on main");
    rate = MIN(MAX(rate, 1), 240);
    [_condition lock];
    BOOL changed = _rate != rate;
    _rate = rate;
    if (changed) { ++_generation; [self cancelRequestsLocked]; }
    [_condition unlock];
    if (changed) [self reconfigure];
}
- (void)reconfigure {
    NSAssert(NSThread.isMainThread, @"Configure the display link on main");
    // Teardown and delegate callbacks share main's run loop. Do not expose the
    // legacy nextDrawable path until the old link has really been invalidated.
    [_link invalidate]; _link = nil;
    [_condition lock];
    _ownsLayer = NO; _stopping = NO;
    BOOL run = _session && _foreground && !_failed && _layer && _layer.superlayer;
    CAMetalLayer *layer = _layer;
    NSUInteger rate = _rate;
    [_condition unlock];
    if (run) {
        CAMetalDisplayLink *link = [[CAMetalDisplayLink alloc] initWithMetalLayer:layer];
        link.delegate = self;
        link.preferredFrameLatency = 2;
        link.preferredFrameRateRange = CAFrameRateRangeMake(rate, rate, rate);
        _link = link;
        [_condition lock]; _ownsLayer = YES; [_condition broadcast]; [_condition unlock];
        [link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
        fprintf(stderr, "[metal-frame-scheduler-v1] active rate=%lu latency=2 drawable-source=display-link\n", (unsigned long)rate);
    } else {
        [_condition lock]; [_condition broadcast]; [_condition unlock];
    }
}
- (void)offerDrawable:(id<CAMetalDrawable>)drawable deadline:(CFTimeInterval)deadline {
    // Never retain a drawable without a waiting renderer. There is no queue of
    // stale frames or repeated blits while the game is loading/paused.
    if (!drawable || deadline <= CACurrentMediaTime()) return;
    [_condition lock];
    if (_ownsLayer && !_stopping && _session && _foreground && _requests.count) {
        MadeiraFrameRequest *request = _requests.firstObject;
        [_requests removeObjectAtIndex:0];
        if (request.generation == _generation) {
            objc_setAssociatedObject(drawable, &madeira_managed_drawable_key, @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            request.drawable = drawable;
        }
        request.complete = YES; [_condition broadcast];
    }
    [_condition unlock];
}
- (void)metalDisplayLink:(CAMetalDisplayLink *)link needsUpdate:(CAMetalDisplayLinkUpdate *)update {
    if (link != _link) return;
    [self offerDrawable:update.drawable deadline:update.targetTimestamp];
}
- (void *)nextForLayer:(CAMetalLayer *)layer handled:(int *)handled {
    *handled = 0;
    [_condition lock];
    if (!_ownsLayer || layer != _layer) { [_condition unlock]; return NULL; }
    *handled = 1;
    // UIKit never waits for its own callback. A second device may request the
    // same surface; keep those requests bounded and deliver them in order.
    if (NSThread.isMainThread || _stopping || !_session || !_foreground || _requests.count >= 4) {
        [_condition unlock]; return NULL;
    }
    MadeiraFrameRequest *request = [MadeiraFrameRequest new];
    request.generation = _generation;
    [_requests addObject:request];
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow:0.5];
    while (!request.complete && request.generation == _generation && _ownsLayer) {
        if (![_condition waitUntilDate:limit]) break;
    }
    id<CAMetalDrawable> drawable = request.generation == _generation ? request.drawable : nil;
    [_requests removeObjectIdenticalTo:request];
    if (!request.complete && !_stopping && request.generation == _generation) {
        _failed = YES; _stopping = YES;
        [self cancelRequestsLocked];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self reconfigure];
            fprintf(stderr, "[metal-frame-scheduler-v1] fallback reason=no-display-link-update\n");
        });
    }
    [_condition unlock];
    // The MRC winemetal boundary consumes this +1 into its autorelease pool.
    // The callback's autorelease pool can drain independently of encoding.
    return drawable ? (void *)CFBridgingRetain(drawable) : NULL;
}
@end

static void madeira_layer_set_bool(CAMetalLayer *layer, NSString *name, BOOL value) {
    SEL selector = NSSelectorFromString(name);
    if ([layer respondsToSelector:selector])
        ((void (*)(id, SEL, BOOL))objc_msgSend)(layer, selector, value);
}

// Fullscreen game only. Native desktop windows keep the existing path.
void *madeira_display_link_next_drawable(void *rawLayer, int *handled) {
    return [[MadeiraMetalFrameScheduler shared] nextForLayer:(__bridge CAMetalLayer *)rawLayer handled:handled];
}
int madeira_display_link_take_drawable(void *rawDrawable) {
    id drawable = (__bridge id)rawDrawable;
    if (!drawable || !objc_getAssociatedObject(drawable, &madeira_managed_drawable_key)) return 0;
    objc_setAssociatedObject(drawable, &madeira_managed_drawable_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    return 1;
}
