#import "Platform.h"
#import "TouchPlatform.h"
#import "TouchControls.h"
#include "TouchState.h"
#include "TouchHit.h"
#include "ControllerInput.h"
#include "ControllerProtocol.h"

/* Backend only. The original SwiftUI window still draws every control, runs
 * its gestures/feedback and owns the original editor and saved layout. We
 * observe raw contacts without consuming events or adding any UI views.
 * Only six inspected calls from TouchControlButton to Wine are redirected;
 * physical keyboard, trackpad and all other application calls stay intact. */
extern unsigned long CFGetTypeID(const void *), CFBooleanGetTypeID(void);
typedef enum { MT_KEY=1, MT_LEFT_MOUSE, MT_RIGHT_MOUSE, MT_PAD } MTActionKind;
typedef struct { MTActionKind kind; unsigned value; } MTAction;
typedef _Bool __attribute__((swiftcall)) (*MTBoolGetter)(void *context __attribute__((swift_context)));

@interface MadeiraTouchBinding : NSObject {
@public
    uintptr_t fingers[TOUCH_STATE_CAPACITY];
    NSUInteger fingerCount;
    uint32_t directions[4];
}
@property CGFloat nx, ny, scale;
@property uint32_t mask;
@property TouchStateRole role;
@property BOOL keyStick, supported;
@property CGPoint origin;
- (BOOL)owns:(uintptr_t)finger;
@end
@implementation MadeiraTouchBinding
- (BOOL)owns:(uintptr_t)finger {
    for (NSUInteger i=0;i<fingerCount;i++) if (fingers[i]==finger) return YES;
    return NO;
}
@end

@interface MadeiraNativeTouch : NSObject {
@public
    TouchState state;
    MTAction actions[32];
    unsigned actionCount;
    uint32_t postedButtons;
    NSUInteger begins, ends, cancels, resets, rejected;
}
@property(weak) UIWindow *overlay, *game, *controls;
@property(strong) NSArray<MadeiraTouchBinding *> *bindings;
@property(strong) NSData *layoutData;
@property(strong) NSString *status, *diagnosticText;
@property(strong) id resignObserver;
@property BOOL ready, allowed, gate, editing, visible, attemptedLoad, hasOwnedInput;
@property CGSize geometry;
- (void)publish;
- (void)reset;
- (uint32_t)mask:(MTActionKind)kind value:(unsigned)value;
- (void)reload:(NSData *)data;
- (void)sync;
- (void)event:(UIEvent *)event window:(UIWindow *)window;
@end

static MadeiraNativeTouch *touchManager;
static void (*originalSendEvent)(id,SEL,UIEvent *);
static BOOL hookInstalled;
static long guestSlide;
static BOOL guestFound;

static BOOL dictionary(id value) { return [value isKindOfClass:[NSDictionary class]]; }
static BOOL string(id value) { return [value isKindOfClass:[NSString class]]; }
static BOOL number(id value) {
    return [value isKindOfClass:[NSNumber class]] &&
        CFGetTypeID((__bridge const void *)value)!=CFBooleanGetTypeID();
}
static BOOL finiteNumber(id value) { return number(value) && __builtin_isfinite([value doubleValue]); }
static void locateGuest(void) {
    if (guestFound) return;
    for (unsigned i=0;i<_dyld_image_count();i++) {
        const char *name=_dyld_get_image_name(i), *base=name;
        if (!name) continue;
        for (const char *p=name;*p;p++) if (*p=='/') base=p+1;
        if (__builtin_strcmp(base,"Madeira")) continue;
        guestSlide=_dyld_get_image_vmaddr_slide(i); guestFound=YES; return;
    }
}
/* Read-only Swift getters, using Clang's documented swiftcall/context ABI.
 * The singleton is NEVER constructed here or accessed before initialization.
 * These addresses are guarded by the exact R6 executable hash at packaging. */
static BOOL originalState(BOOL *editing, BOOL *visible) {
    locateGuest();
    if (!guestFound) return NO;
    void *model=*(void **)(uintptr_t)(0x101073cb8ull+guestSlide);
    if (!model) return NO;
    MTBoolGetter getEditing=(MTBoolGetter)(uintptr_t)(0x100026cc4ull+guestSlide);
    MTBoolGetter getVisible=(MTBoolGetter)(uintptr_t)(0x100026cb0ull+guestSlide);
    *editing=getEditing(model); *visible=getVisible(model); return YES;
}
static void observeControls(id window, SEL selector, UIEvent *event) {
    [touchManager event:event window:window];
    // Always forward the unchanged event, retaining original visuals/editor.
    originalSendEvent(window,selector,event);
    [touchManager sync]; // Editor/visibility taps take effect immediately.
}
static void installHook(void) {
    if (hookInstalled) return;
    Class cls=objc_getClass("_TtC7Madeira14ControlsWindow");
    if (!cls) return;
    SEL selector=@selector(sendEvent:);
    void *method=class_getInstanceMethod(cls,selector);
    if (!method) return;
    originalSendEvent=(void (*)(id,SEL,UIEvent *))method_getImplementation(method);
    // Add to this exact app subclass, never replace UIKit or host behavior.
    hookInstalled=class_addMethod(cls,selector,(void *)observeControls,method_getTypeEncoding(method));
}
static unsigned padButton(NSString *name) {
    NSString *names[]={@"A",@"B",@"X",@"Y",@"D↑",@"D↓",@"D←",@"D→",@"LB",@"RB",@"L3",@"R3",@"Menu",@"View"};
    unsigned bits[]={MC_BUTTON_A,MC_BUTTON_B,MC_BUTTON_X,MC_BUTTON_Y,MC_BUTTON_DPAD_UP,
        MC_BUTTON_DPAD_DOWN,MC_BUTTON_DPAD_LEFT,MC_BUTTON_DPAD_RIGHT,MC_BUTTON_LEFT_SHOULDER,
        MC_BUTTON_RIGHT_SHOULDER,MC_BUTTON_LEFT_THUMB,MC_BUTTON_RIGHT_THUMB,MC_BUTTON_START,MC_BUTTON_BACK};
    for (unsigned i=0;i<sizeof(bits)/sizeof(bits[0]);i++) if ([name isEqualToString:names[i]]) return bits[i];
    return 0;
}

@implementation MadeiraNativeTouch
- (instancetype)init {
    self=[super init];
    if (self) {
        touch_state_init(&state,0.12f); self.bindings=@[]; self.status=@"Original touch interface";
        __weak MadeiraNativeTouch *weakSelf=self;
        self.resignObserver=[[NSNotificationCenter defaultCenter]
            addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification *note) { (void)note; weakSelf.allowed=NO; [weakSelf reset]; }];
    }
    return self;
}
- (uint32_t)mask:(MTActionKind)kind value:(unsigned)value {
    for (unsigned i=0;i<actionCount;i++) if (actions[i].kind==kind && actions[i].value==value) return 1u<<i;
    if (actionCount==32) return 0;
    actions[actionCount]=(MTAction){kind,value}; return 1u<<actionCount++;
}
- (void)publish {
    TouchStateOutput output=touch_state_output(&state);
    if (!self.ready || !self.allowed) output=(TouchStateOutput){0};
    uint32_t edges=postedButtons^output.buttons, pad=0;
    for (unsigned i=0;i<actionCount;i++) {
        uint32_t bit=1u<<i;
        if ((output.buttons&bit) && actions[i].kind==MT_PAD) pad|=actions[i].value;
        if (!(edges&bit)) continue;
        int down=!!(output.buttons&bit);
        if (actions[i].kind==MT_KEY) MadeiraTouchPostKey((int)actions[i].value,down);
        else if (actions[i].kind==MT_LEFT_MOUSE) MadeiraTouchPostMouse(down ? 2 : 4);
        else if (actions[i].kind==MT_RIGHT_MOUSE) MadeiraTouchPostMouse(down ? 8 : 16);
    }
    postedButtons=output.buttons;
    MadeiraControllerSetTouchStateWithStick(pad,output.left_trigger,output.right_trigger,
        output.left_x,output.left_y,self.ready && self.allowed);
}
- (void)reset {
    touch_state_reset(&state); [self publish];
    for (MadeiraTouchBinding *binding in self.bindings) binding->fingerCount=0;
    ++resets;
}
- (void)reload:(NSData *)data {
    [self reset]; actionCount=0; self.layoutData=data; self.attemptedLoad=YES;
    NSMutableArray *bindings=[NSMutableArray new]; self.bindings=bindings;
    // The original app's default is an empty layout. No new preset or file.
    if (!data) { self.status=@"Original layout: no saved buttons"; return; }
    id root=data.length<=1024*1024 ? [NSJSONSerialization JSONObjectWithData:data options:0 error:0] : nil;
    id controls=dictionary(root) ? root[@"controls"] : nil;
    if (![controls isKindOfClass:[NSArray class]]) { self.status=@"Unreadable original layout preserved"; return; }
    NSString *ids[64]; unsigned count=0, skipped=0;
    for (id control in controls) {
        if (count==64) { ++skipped; continue; }
        id identifier=dictionary(control) ? control[@"id"] : nil;
        id nx=dictionary(control) ? control[@"nx"] : nil, ny=dictionary(control) ? control[@"ny"] : nil;
        id scale=dictionary(control) ? control[@"scale"] : nil, action=dictionary(control) ? control[@"action"] : nil;
        BOOL duplicate=NO;
        if (string(identifier)) for (unsigned i=0;i<count;i++) if ([identifier isEqualToString:ids[i]]) duplicate=YES;
        if (!string(identifier) || ![[NSUUID alloc] initWithUUIDString:identifier] || duplicate ||
            !finiteNumber(nx) || !finiteNumber(ny) || !finiteNumber(scale) ||
            [nx doubleValue]<0 || [nx doubleValue]>1 || [ny doubleValue]<0 || [ny doubleValue]>1 ||
            [scale doubleValue]<0.5 || [scale doubleValue]>3 || !dictionary(action) || [action count]!=1) {
            ++skipped; continue;
        }
        ids[count++]=identifier;
        MadeiraTouchBinding *binding=[MadeiraTouchBinding new];
        binding.nx=[nx doubleValue]; binding.ny=[ny doubleValue]; binding.scale=[scale doubleValue]; binding.supported=YES;
        id payload=action[@"key"];
        if (dictionary(payload) && number(payload[@"_0"])) {
            double value=[payload[@"_0"] doubleValue];
            if (value>=1 && value<=254 && value==(unsigned)value) binding.mask=[self mask:MT_KEY value:(unsigned)value];
        } else if (dictionary(action[@"mouseLeft"])) binding.mask=[self mask:MT_LEFT_MOUSE value:0];
        else if (dictionary(action[@"mouseRight"])) binding.mask=[self mask:MT_RIGHT_MOUSE value:0];
        else if (dictionary(action[@"joystickWASD"]) || dictionary(action[@"joystickArrows"])) {
            BOOL arrows=dictionary(action[@"joystickArrows"]); binding.keyStick=YES;
            unsigned wasd[]={0x57,0x44,0x53,0x41}, arrow[]={0x26,0x27,0x28,0x25};
            for (unsigned i=0;i<4;i++) {
                binding->directions[i]=[self mask:MT_KEY value:arrows ? arrow[i] : wasd[i]];
                if (!binding->directions[i]) binding.supported=NO;
            }
        } else if (dictionary(action[@"pad"]) && string(action[@"pad"][@"_0"])) {
            NSString *name=action[@"pad"][@"_0"]; unsigned bit=padButton(name);
            if (bit) binding.mask=[self mask:MT_PAD value:bit];
            else if ([name isEqualToString:@"LT"]) binding.role=TOUCH_STATE_ROLE_LEFT_TRIGGER;
            else if ([name isEqualToString:@"RT"]) binding.role=TOUCH_STATE_ROLE_RIGHT_TRIGGER;
            else if ([name isEqualToString:@"LS"]) binding.role=TOUCH_STATE_ROLE_LEFT_STICK;
        }
        // Keyboard toggle keeps its original SwiftUI action; no double toggle.
        if (!binding.mask && !binding.keyStick && !binding.role) binding.supported=NO;
        if (!binding.supported) ++skipped;
        [bindings addObject:binding];
    }
    self.status=[NSString stringWithFormat:@"Original layout: %lu buttons, %u unsupported/invalid/limited",bindings.count,skipped];
}
- (void)sync {
    BOOL editing=NO, visible=NO;
    BOOL ready=hookInstalled && self.controls && !self.controls.hidden &&
        self.controls.windowScene==self.game.windowScene && originalState(&editing,&visible);
    CGSize geometry=self.controls.bounds.size;
    BOOL allowed=ready && self.gate && !editing && visible && geometry.width>geometry.height &&
        [UIApplication sharedApplication].applicationState==0;
    if (ready!=self.ready || editing!=self.editing || visible!=self.visible)
        madeira_resolution_trace([NSString stringWithFormat:@"[touch v65 backend-only] hook=%d ready=%d editing=%d visible=%d",hookInstalled,ready,editing,visible]);
    if (ready!=self.ready || editing!=self.editing || visible!=self.visible ||
        geometry.width!=self.geometry.width || geometry.height!=self.geometry.height || (self.allowed && !allowed)) [self reset];
    self.ready=ready; self.editing=editing; self.visible=visible; self.geometry=geometry; self.allowed=allowed;
    if (ready) self.hasOwnedInput=YES;
    NSString *path=[NSSearchPathForDirectoriesInDomains(9,1,YES)[0] stringByAppendingPathComponent:@"madeira-controls.json"];
    NSData *data=[NSData dataWithContentsOfFile:path];
    if (!self.attemptedLoad || (data && ![data isEqualToData:self.layoutData]) || (!data && self.layoutData)) {
        [self reload:data]; madeira_resolution_trace([NSString stringWithFormat:@"[touch v65 backend-only] %@ ready=%d",self.status,ready]);
    }
}
- (void)event:(UIEvent *)event window:(UIWindow *)window {
    if (window!=self.controls) return;
    [self sync];
    NSSet<UITouch *> *touches=[event touchesForWindow:window];
    for (UITouch *touch in touches) {
        uintptr_t finger=(uintptr_t)(__bridge void *)touch;
        MadeiraTouchBinding *owner=nil;
        for (MadeiraTouchBinding *binding in self.bindings) if ([binding owns:finger]) { owner=binding; break; }
        NSInteger phase=touch.phase;
        if (phase==3 || phase==4) {
            if (!owner) continue;
            (void)touch_state_end(&state,finger);
            for (NSUInteger i=0;i<owner->fingerCount;i++) if (owner->fingers[i]==finger) {
                owner->fingers[i]=owner->fingers[--owner->fingerCount]; break;
            }
            if (phase==4) ++cancels; else ++ends;
            continue;
        }
        if (!self.allowed) continue;
        CGPoint point=[touch locationInView:window];
        if (phase==0 && !owner) {
            if (mtouch_original_toolbar(self.geometry.width,point.x,point.y)) continue;
            // Last-drawn SwiftUI button is the frontmost one when overlapping.
            for (NSUInteger i=self.bindings.count;i>0;i--) {
                MadeiraTouchBinding *binding=self.bindings[i-1];
                if (!mtouch_original_hit(self.geometry.width,self.geometry.height,binding.nx,binding.ny,binding.scale,point.x,point.y)) continue;
                owner=binding; break;
            }
            if (!owner || !owner.supported || ((owner.keyStick || owner.role==TOUCH_STATE_ROLE_LEFT_STICK) && owner->fingerCount)) continue;
            TouchStateResult result=touch_state_begin(&state,finger,owner.mask,owner.role,0,0,
                (owner.role==TOUCH_STATE_ROLE_LEFT_TRIGGER || owner.role==TOUCH_STATE_ROLE_RIGHT_TRIGGER) ? 1 : 0);
            if (result!=TOUCH_STATE_OK) { ++rejected; continue; }
            owner->fingers[owner->fingerCount++]=finger; owner.origin=point; ++begins;
        } else if (phase==1 && owner) {
            CGFloat dx=point.x-owner.origin.x, dy=point.y-owner.origin.y;
            if (owner.keyStick) {
                unsigned directions=mtouch_original_directions(dx,dy,64*owner.scale);
                uint32_t mask=0;
                for (unsigned i=0;i<4;i++) if (directions&(1u<<i)) mask|=owner->directions[i];
                (void)touch_state_update_buttons(&state,finger,mask);
            } else if (owner.role==TOUCH_STATE_ROLE_LEFT_STICK) {
                CGFloat radius=64*owner.scale*0.35;
                (void)touch_state_move(&state,finger,(float)(dx/radius),(float)(-dy/radius),0);
            }
        }
    }
    [self publish];
}
@end

/* Redirected visual-gesture calls are silent when raw ownership is installed,
 * including late onEnded callbacks after a reset. There is no second input
 * sender. If initialization has not finished, retain original behavior. */
int MadeiraTouchLegacyInput(unsigned key, unsigned value, unsigned flags) {
    if (hookInstalled && touchManager.hasOwnedInput) return 0;
    if (key && key<=254 && value<=1) MadeiraTouchPostKey((int)key,(int)value);
    else if (!key && !value && (flags==2 || flags==4 || flags==8 || flags==16)) MadeiraTouchPostMouse(flags);
    return 0;
}
int MadeiraTouchIsEditing(void) {
    BOOL editing=NO, visible=NO;
    return [NSThread isMainThread] && originalState(&editing,&visible) ? editing : 0;
}
void MadeiraTouchRefresh(UIWindow *overlay, UIWindow *game, UIView *metal, int full, int allowed) {
    (void)metal; (void)full;
    if (![NSThread isMainThread]) return;
    if (!touchManager) touchManager=[MadeiraNativeTouch new];
    installHook();
    UIWindow *controls=nil; Class cls=objc_getClass("_TtC7Madeira14ControlsWindow");
    for (UIWindow *window in game.windowScene.windows) if (cls && [window isKindOfClass:cls]) { controls=window; break; }
    if (touchManager.controls!=controls || touchManager.game!=game || (touchManager.gate && !allowed)) [touchManager reset];
    touchManager.overlay=overlay; touchManager.game=game; touchManager.controls=controls;
    touchManager.gate=!!allowed; [touchManager sync];
}
void MadeiraTouchReset(void) { if ([NSThread isMainThread]) [touchManager reset]; }
void MadeiraTouchLayoutChanged(UIWindow *overlay) {
    if (![NSThread isMainThread] || overlay!=touchManager.overlay) return;
    CGSize size=overlay.bounds.size;
    if (size.width!=touchManager.geometry.width || size.height!=touchManager.geometry.height) {
        touchManager.allowed=NO; [touchManager reset];
    }
}
const char *MadeiraTouchDiagnostics(void) {
    if (![NSThread isMainThread] || !touchManager) return "Original touch UI; backend not initialized";
    touchManager.diagnosticText=[NSString stringWithFormat:@"%@; raw hook=%d ready=%d gate=%d visible=%d editing=%d begins=%lu ends=%lu cancels=%lu resets=%lu rejected=%lu",
        touchManager.status,hookInstalled,touchManager.ready,touchManager.allowed,touchManager.visible,touchManager.editing,
        touchManager->begins,touchManager->ends,touchManager->cancels,touchManager->resets,touchManager->rejected];
    return touchManager.diagnosticText.UTF8String;
}
