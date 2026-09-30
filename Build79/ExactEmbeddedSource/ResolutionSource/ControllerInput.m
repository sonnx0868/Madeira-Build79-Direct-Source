#import "Platform.h"
#include "ControllerInput.h"
#include "ControllerValues.h"

/* Public API declarations only; no SDK implementation is copied here. */
@class GCController, GCControllerElement, GCExtendedGamepad;
typedef void (^GCChanged)(GCExtendedGamepad *, GCControllerElement *);
@interface GCControllerAxisInput : NSObject
@property(readonly) float value;
@end
@interface GCControllerButtonInput : NSObject
@property(readonly) float value;
@property(readonly, getter=isPressed) BOOL pressed;
@end
@interface GCControllerDirectionPad : NSObject
@property(readonly) GCControllerAxisInput *xAxis, *yAxis;
@property(readonly) GCControllerButtonInput *up, *down, *left, *right;
@end
@interface GCExtendedGamepad : NSObject
@property(copy) GCChanged valueChangedHandler;
@property(readonly) GCControllerButtonInput *buttonA, *buttonB, *buttonX, *buttonY;
@property(readonly) GCControllerButtonInput *buttonMenu, *buttonOptions;
@property(readonly) GCControllerButtonInput *leftShoulder, *rightShoulder;
@property(readonly) GCControllerButtonInput *leftTrigger, *rightTrigger;
@property(readonly) GCControllerButtonInput *leftThumbstickButton, *rightThumbstickButton;
@property(readonly) GCControllerDirectionPad *leftThumbstick, *rightThumbstick, *dpad;
@end
@interface GCController : NSObject
+ (NSArray<GCController *> *)controllers;
@property(readonly) GCExtendedGamepad *extendedGamepad;
@property(retain) id handlerQueue; /* dispatch_queue_t is an Objective-C object on iOS. */
@property(readonly) NSString *vendorName;
@end
@interface NSNotification : NSObject
@property(readonly) id object;
@end
@interface NSOperationQueue : NSObject
+ (instancetype)mainQueue;
@end
@interface NSNotificationCenter : NSObject
+ (instancetype)defaultCenter;
- (id)addObserverForName:(NSString *)name object:(id)object queue:(NSOperationQueue *)queue usingBlock:(void (^)(NSNotification *))block;
- (void)removeObserver:(id)observer;
@end
@interface NSThread : NSObject
+ (BOOL)isMainThread;
@end
@interface NSRunLoop : NSObject
+ (instancetype)mainRunLoop;
- (void)addTimer:(NSTimer *)timer forMode:(NSString *)mode;
@end
@interface NSTimer (MadeiraControllerTimer)
+ (instancetype)timerWithTimeInterval:(double)interval repeats:(BOOL)repeat block:(void (^)(NSTimer *))block;
- (void)invalidate;
@end
@interface UIApplication (MadeiraControllerState)
@property(readonly) NSInteger applicationState;
@end
extern NSString * const GCControllerDidConnectNotification;
extern NSString * const GCControllerDidDisconnectNotification;
extern NSString * const UIApplicationWillResignActiveNotification;
extern NSString * const UIApplicationDidBecomeActiveNotification;
extern NSString * const NSRunLoopCommonModes;
extern int open(const char *, int, ...), close(int), flock(int, int);
extern int ftruncate(int, long long), munmap(void *, unsigned long);
extern long long lseek(int, long long, int);
extern void *mmap(void *, unsigned long, int, int, int, long long);
extern uint32_t arc4random(void);

static MCPage *mapped;
static MCPage state;
static int descriptor = -1;
static GCController *controllers[MC_SLOT_COUNT];
static GCChanged previousHandlers[MC_SLOT_COUNT], installedHandlers[MC_SLOT_COUNT];
static id previousQueues[MC_SLOT_COUNT];
static id observers[4];
static NSTimer *heartbeatTimer;
static float leftDeadzone, rightDeadzone, triggerDeadzone;
static int applicationActive, inputEnabled = 1;
static MCTouchState touchState;
static unsigned reconcileTicks;
static NSString *diagnosticText;
static unsigned diagnosticSamples, controllerInputEvents;

static void publish(void) {
    if (!mapped) return;
    if (++state.heartbeat_lo == 0) ++state.heartbeat_hi;
    state.flags = applicationActive ? MC_FLAG_ACTIVE : 0;
    for (unsigned i = 0; i < MC_SLOT_COUNT; ++i) {
        GCExtendedGamepad *pad = controllers[i].extendedGamepad;
        MCSlot next = {0};
        int hasTouch = i == 0 && touchState.connected;
        if (mc_slot_present(i, !!state.flags, pad != nil, hasTouch)) {
            next.flags = MC_CONNECTED;
            if (pad) {
#define BUTTON(element, bit) if (pad.element.isPressed) next.buttons |= bit
            BUTTON(dpad.up, MC_BUTTON_DPAD_UP); BUTTON(dpad.down, MC_BUTTON_DPAD_DOWN);
            BUTTON(dpad.left, MC_BUTTON_DPAD_LEFT); BUTTON(dpad.right, MC_BUTTON_DPAD_RIGHT);
            BUTTON(buttonMenu, MC_BUTTON_START); BUTTON(buttonOptions, MC_BUTTON_BACK);
            BUTTON(leftThumbstickButton, MC_BUTTON_LEFT_THUMB);
            BUTTON(rightThumbstickButton, MC_BUTTON_RIGHT_THUMB);
            BUTTON(leftShoulder, MC_BUTTON_LEFT_SHOULDER);
            BUTTON(rightShoulder, MC_BUTTON_RIGHT_SHOULDER);
            BUTTON(buttonA, MC_BUTTON_A); BUTTON(buttonB, MC_BUTTON_B);
            BUTTON(buttonX, MC_BUTTON_X); BUTTON(buttonY, MC_BUTTON_Y);
#undef BUTTON
            next.triggers = mc_trigger(pad.leftTrigger.value, triggerDeadzone) |
                            ((uint32_t)mc_trigger(pad.rightTrigger.value, triggerDeadzone) << 8);
            next.left_axes = mc_stick(pad.leftThumbstick.xAxis.value, pad.leftThumbstick.yAxis.value, leftDeadzone);
            next.right_axes = mc_stick(pad.rightThumbstick.xAxis.value, pad.rightThumbstick.yAxis.value, rightDeadzone);
            }
            if (hasTouch)
                mc_merge_touch_with_stick(&next, touchState.buttons,
                    touchState.left_trigger, touchState.right_trigger,
                    touchState.lx, touchState.ly);
        }
        next = mc_slot_gated(next, inputEnabled);
        next.packet = state.slots[i].packet + mc_slot_changed(&next, &state.slots[i]);
        state.slots[i] = next;
    }
    mc_publish(mapped, &state);
}

static void detach(unsigned slot) {
    GCController *controller = controllers[slot];
    GCExtendedGamepad *pad = controller.extendedGamepad;
    if (pad.valueChangedHandler == installedHandlers[slot]) {
        pad.valueChangedHandler = previousHandlers[slot];
        controller.handlerQueue = previousQueues[slot];
    }
    controllers[slot] = nil;
    previousHandlers[slot] = installedHandlers[slot] = nil;
    previousQueues[slot] = nil;
}

static void reconcileExcluding(GCController *excluded) {
    if (!mapped) return;
    NSArray<GCController *> *connected = [GCController controllers];
    for (unsigned slot = 0; slot < MC_SLOT_COUNT; ++slot) {
        BOOL found = NO;
        for (GCController *controller in connected)
            if (controller != excluded && controller == controllers[slot] && controller.extendedGamepad) found = YES;
        if (!found && controllers[slot]) detach(slot);
    }
    for (GCController *controller in connected) {
        if (controller == excluded || !controller.extendedGamepad) continue;
        BOOL assigned = NO;
        for (unsigned slot = 0; slot < MC_SLOT_COUNT; ++slot)
            if (controllers[slot] == controller) assigned = YES;
        if (assigned) continue;
        for (unsigned slot = 0; slot < MC_SLOT_COUNT; ++slot) {
            if (controllers[slot]) continue;
            controllers[slot] = controller;
            previousQueues[slot] = controller.handlerQueue;
            previousHandlers[slot] = controller.extendedGamepad.valueChangedHandler;
            controller.handlerQueue = (__bridge id)(void *)&_dispatch_main_q;
            installedHandlers[slot] = ^(GCExtendedGamepad *pad, GCControllerElement *element) {
                if (controllers[slot].extendedGamepad != pad) return;
                ++controllerInputEvents;
                publish();
                GCChanged old = previousHandlers[slot];
                id oldQueue = previousQueues[slot];
                if (old) {
                    if (oldQueue && (__bridge void *)oldQueue != (void *)&_dispatch_main_q) {
                        // Capture retained values rather than reading slot globals
                        // later: the slot may disconnect or be reused meanwhile.
                        dispatch_async((__bridge void *)oldQueue, ^{ old(pad, element); });
                    } else old(pad, element);
                }
            };
            controller.extendedGamepad.valueChangedHandler = installedHandlers[slot];
            break;
        }
    }
    publish();
}
static void reconcile(void) { reconcileExcluding(nil); }

int MadeiraControllerStart(const char *path) {
    if (![NSThread isMainThread]) return -1;
    if (mapped) return -2;
    if (!path || path[0] != '/') return -3;
    NSString *name = [NSString stringWithUTF8String:path];
    if (!name || ![[NSFileManager defaultManager] createDirectoryAtPath:name.stringByDeletingLastPathComponent
          withIntermediateDirectories:YES attributes:nil error:0]) return -4;
    /* Darwin O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC; LOCK_EX | LOCK_NB. */
    descriptor = open(path, 0x0002 | 0x0200 | 0x0100 | 0x01000000, 0600);
    if (descriptor < 0) return -5;
    long long length;
    if (flock(descriptor, 2 | 4) || (length = lseek(descriptor, 0, 2)) < 0 ||
        (length != 0 && length != MC_FILE_SIZE) ||
        (length == 0 && ftruncate(descriptor, MC_FILE_SIZE))) {
        close(descriptor); descriptor = -1; return -6;
    }
    /* Darwin PROT_READ | PROT_WRITE, MAP_SHARED. */
    void *memory = mmap(0, MC_FILE_SIZE, 1 | 2, 1, descriptor, 0);
    if (memory == (void *)-1) { close(descriptor); descriptor = -1; return -7; }
    mapped = memory;
    state = (MCPage){ .magic = MC_MAGIC, .version = MC_VERSION, .size = MC_FILE_SIZE,
                      .slot_count = MC_SLOT_COUNT, .writer_epoch = arc4random() };
    applicationActive = [UIApplication sharedApplication].applicationState == 0;
    mc_touch_gate(&touchState, applicationActive);
    inputEnabled = 1;
    reconcileTicks = 0;
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *queue = [NSOperationQueue mainQueue];
    observers[0] = [center addObserverForName:GCControllerDidConnectNotification object:nil queue:queue
                                   usingBlock:^(NSNotification *note) { (void)note; reconcile(); }];
    observers[1] = [center addObserverForName:GCControllerDidDisconnectNotification object:nil queue:queue
                                   usingBlock:^(NSNotification *note) { reconcileExcluding(note.object); }];
    observers[2] = [center addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:queue
                                   usingBlock:^(NSNotification *note) {
        (void)note; applicationActive = 0; mc_touch_reset(&touchState); publish();
    }];
    observers[3] = [center addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:queue
                                   usingBlock:^(NSNotification *note) { (void)note; applicationActive = 1; reconcile(); }];
    reconcile();
    heartbeatTimer = [NSTimer timerWithTimeInterval:1.0 / 60.0 repeats:YES
                          block:^(NSTimer *timer) {
        (void)timer;
        // Repair missed connect/disconnect notifications without replacing
        // handlers for controllers already assigned to a stable slot.
        if (++reconcileTicks >= 60) { reconcileTicks = 0; reconcile(); }
        else publish();
    }];
    [[NSRunLoop mainRunLoop] addTimer:heartbeatTimer forMode:NSRunLoopCommonModes];
    return 0;
}

void MadeiraControllerStop(void) {
    if (![NSThread isMainThread]) return;
    mc_touch_reset(&touchState);
    if (!mapped) return;
    [heartbeatTimer invalidate]; heartbeatTimer = nil;
    for (unsigned i = 0; i < 4; ++i) {
        if (observers[i]) [[NSNotificationCenter defaultCenter] removeObserver:observers[i]];
        observers[i] = nil;
    }
    for (unsigned i = 0; i < MC_SLOT_COUNT; ++i) detach(i);
    applicationActive = 0;
    publish(); /* Final disconnected frame is visible before unmapping. */
    munmap(mapped, MC_FILE_SIZE); mapped = 0;
    close(descriptor); descriptor = -1;
}

void MadeiraControllerSetDeadzones(float left, float right, float triggers) {
    if (![NSThread isMainThread]) return;
    leftDeadzone = mc_clamp(left, 0, .95f); rightDeadzone = mc_clamp(right, 0, .95f);
    triggerDeadzone = mc_clamp(triggers, 0, .95f);
    publish();
}
void MadeiraControllerSetInputEnabled(int enabled) {
    if (![NSThread isMainThread]) return;
    inputEnabled = !!enabled;
    mc_touch_gate(&touchState, inputEnabled);
    publish();
}
void MadeiraControllerSetTouchState(uint32_t buttons, uint8_t left_trigger,
                                    uint8_t right_trigger, int connected) {
    MadeiraControllerSetTouchStateWithStick(buttons, left_trigger, right_trigger,
                                            0, 0, connected);
}
void MadeiraControllerSetTouchStateWithStick(uint32_t buttons, uint8_t left_trigger,
                                             uint8_t right_trigger, float lx,
                                             float ly, int connected) {
    if (![NSThread isMainThread]) return;
    mc_touch_set(&touchState, buttons, left_trigger, right_trigger, lx, ly,
                 connected, inputEnabled && (!mapped || applicationActive));
    publish();
}
unsigned MadeiraControllerConnectedCount(void) {
    if (![NSThread isMainThread]) return 0;
    unsigned count = 0;
    for (unsigned slot = 0; slot < MC_SLOT_COUNT; ++slot) if (controllers[slot]) ++count;
    return count;
}

unsigned MadeiraControllerDetectedCount(void) {
    if (![NSThread isMainThread]) return 0;
    unsigned count = 0;
    for (GCController *controller in [GCController controllers])
        if (controller.extendedGamepad) ++count;
    return count;
}

const char *MadeiraControllerDiagnostics(void) {
    if (![NSThread isMainThread]) return "Diagnostics require the main thread";
    diagnosticText = [NSString stringWithFormat:@"Refresh %u · input events %u\n\n",
        ++diagnosticSamples, controllerInputEvents];
    unsigned index = 0;
    for (GCController *controller in [GCController controllers]) {
        GCExtendedGamepad *pad = controller.extendedGamepad;
        if (!pad) continue;
        NSString *pressed = @"";
#define SHOW(element, label) if (pad.element.isPressed) pressed = [pressed stringByAppendingString:label @" "]
        SHOW(buttonA, @"A"); SHOW(buttonB, @"B"); SHOW(buttonX, @"X"); SHOW(buttonY, @"Y");
        SHOW(leftShoulder, @"LB"); SHOW(rightShoulder, @"RB");
        SHOW(leftThumbstickButton, @"L3"); SHOW(rightThumbstickButton, @"R3");
        SHOW(buttonMenu, @"Start"); SHOW(buttonOptions, @"Back");
        SHOW(dpad.up, @"↑"); SHOW(dpad.down, @"↓"); SHOW(dpad.left, @"←"); SHOW(dpad.right, @"→");
#undef SHOW
        diagnosticText = [diagnosticText stringByAppendingString:[NSString stringWithFormat:
            @"%@%u · %@\nButtons: %@\nLeft: %+.2f, %+.2f\nRight: %+.2f, %+.2f\nTriggers: %.0f%% / %.0f%%\n",
            index ? @"\n" : @"", index + 1, controller.vendorName ?: @"Game controller",
            pressed.length ? pressed : @"—", pad.leftThumbstick.xAxis.value, pad.leftThumbstick.yAxis.value,
            pad.rightThumbstick.xAxis.value, pad.rightThumbstick.yAxis.value,
            pad.leftTrigger.value * 100, pad.rightTrigger.value * 100]];
        if (++index == MC_SLOT_COUNT) break;
    }
    if (!index) diagnosticText = [GCController controllers].count ?
        @"A controller is connected, but it has no supported extended-gamepad profile." :
        @"No controller detected. Pair it in iPhone Settings → Bluetooth, then return here.\n\nThis screen updates automatically.";
    return diagnosticText.UTF8String;
}
