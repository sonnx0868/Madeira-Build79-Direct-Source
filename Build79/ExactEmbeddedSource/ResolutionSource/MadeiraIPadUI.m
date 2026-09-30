#import "Platform.h"
#import "ResolutionSettings.h"
#import "TouchControls.h"
#include "RuntimeClockPolicy.h"
// Bound to the verified margooey release binary. LiveContainer dlopens the guest
// executable, so its dyld index need not be zero. Never call an offset in the host.
static int madeiraImageIndex(void) {
    unsigned int count=_dyld_image_count();
    for (unsigned int i=0;i<count;i++) {
        const char *name=_dyld_get_image_name(i);
        if (!name) continue;
        const char *base=name;
        for (const char *p=name;*p;p++) if (*p=='/') base=p+1;
        if (base[0]=='M' && base[1]=='a' && base[2]=='d' && base[3]=='e' &&
            base[4]=='i' && base[5]=='r' && base[6]=='a' && base[7]==0)
            return (int)i;
    }
    return -1;
}
static void *madeiraFunction(unsigned long va) {
    int image=madeiraImageIndex();
    if (image<0) return 0;
    return (void *)(va+(unsigned long)_dyld_get_image_vmaddr_slide((unsigned int)image));
}
static void key(int vk, int down) {
    void (*fn)(int,int)=(void (*)(int,int))madeiraFunction(0x100008e7cUL);
    if (fn) fn(vk, down);
}
static int jit_ready(void) {
    int (*fn)(void)=(int (*)(void))madeiraFunction(0x100004358UL);
    return fn ? fn() : 0;
}
static int wine_running(void) {
    int (*fn)(void)=(int (*)(void))madeiraFunction(0x100007714UL);
    return fn ? fn() : 0;
}
void MadeiraTouchPostKey(int vk, int down) { key(vk, down); }
void MadeiraTouchPostMouse(unsigned flags) {
    void (*fn)(int,int,unsigned,int)=(void (*)(int,int,unsigned,int))madeiraFunction(0x10000b244UL);
    if (fn) fn(0,0,flags,0);
}
int MadeiraTouchWineRunning(void) { return wine_running(); }
extern void MadeiraAudioSetDiagnostics(int enabled);
static void diagnostics(int on) {
    MadeiraAudioSetDiagnostics(on);
    void (*fn)(int)=(void (*)(int))madeiraFunction(0x10012a630UL);
    if (fn) fn(on);
}
static void later(double seconds, void (^block)(void)) {
    dispatch_after(dispatch_time(0, (long long)(seconds * 1000000000)), &_dispatch_main_q, block);
}
static NSString *documents(void) {
    return NSSearchPathForDirectoriesInDomains(9, 1, YES)[0];
}
static NSUserDefaults *preferences(void) { return [NSUserDefaults standardUserDefaults]; }
static void appendTrace(NSString *path, NSData *line) {
    int fd=open(path.UTF8String, 0x0001 | 0x0008 | 0x0200 | 0x01000000, 0600);
    if (fd >= 0) {
        const unsigned char *bytes=line.bytes;
        unsigned long remaining=line.length;
        while (remaining) {
            long count=write(fd, bytes, remaining);
            if (count <= 0) break;
            bytes += count; remaining -= (unsigned long)count;
        }
        close(fd);
    }
}
static void trace(NSString *message) {
    NSString *root=documents();
    appendTrace([root stringByAppendingPathComponent:@"madeira-ipad-ui.log"],
        [[message stringByAppendingString:@"\n"] dataUsingEncoding:4]);
    appendTrace([root stringByAppendingPathComponent:@"madeira-log.txt"],
        [[NSString stringWithFormat:@"[iPadUI] %@\n",message] dataUsingEncoding:4]);
}
// The launch wrapper runs on the Wine launch thread. Do not dispatch-sync to
// main here: append-only diagnostics must not hold up launch or change defaults.
void madeira_resolution_trace(NSString *message) { trace(message); }
#include "ControllerIntegration.inc"
extern void MadeiraAudioProvision(void);
static void configureRendererExperiments(void) {
    // These guest settings are cached at process/device construction. They do
    // not modify Wine registry, game files, saves, or rendering quality.
    setenv("DXMT_EXPERIMENT_UPLOAD_WATERMARK",
        [preferences() boolForKey:@"MadeiraDX11.uploadWatermark"] ? "1" : "0", 1);
    setenv("DXMT_DIAGNOSTICS",
        [preferences() boolForKey:@"MadeiraDX11.diagnostics"] ? "1" : "0", 1);
}
static void configureRuntime(void) {
    // Start every session quiet; diagnostic work is an explicit menu opt-in.
    diagnostics(0);
    // Correct the frozen Windows tick clock before wineserver caches its gate.
    // Existing Documents/madeira-usd-time.txt remains the native kill switch.
    NSString *clockPath=[documents() stringByAppendingPathComponent:@"madeira-usd-time.txt"];
    BOOL clockFile=[[NSFileManager defaultManager] fileExistsAtPath:clockPath isDirectory:0];
    unsigned char clockBytes[33];
    long clockSize=0;
    if (clockFile) {
        int fd=open(clockPath.UTF8String,0x0100 | 0x01000000); // RDONLY, NOFOLLOW, CLOEXEC
        if (fd>=0) { clockSize=pread(fd,clockBytes,sizeof clockBytes,0); close(fd); }
    }
    const char *clockEnvironment=getenv("MADEIRA_USD_TIME");
    unsigned long clockEnvironmentSize=0;
    if (clockEnvironment) while (clockEnvironmentSize<33 && clockEnvironment[clockEnvironmentSize]) ++clockEnvironmentSize;
    int clockOn=mrc_choice(clockFile,clockBytes,clockSize>0?(unsigned long)clockSize:0,
        (const unsigned char *)clockEnvironment,clockEnvironmentSize);
    if (setenv("MADEIRA_USD_TIME",clockOn?"1":"0",1)==0)
        trace(clockOn?@"Windows shared-data clock enabled before Wine startup":@"Windows shared-data clock disabled by explicit override");
    else trace(@"Windows shared-data clock environment could not be configured");
    configureRendererExperiments();
    MadeiraAudioProvision();
    controllerConfigureEnvironment();
    // DXMT's default macOS cache-directory lookup fails on iPadOS. Its shipped
    // resolver accepts an absolute Unix directory through this variable. Set it
    // before Wine constructs the Windows environment, preserving any existing
    // user configuration. This enables persistence, not a new rendering mode.
    if (getenv("DXMT_SHADER_CACHE_PATH")) {
        trace(@"Existing DXMT shader cache configuration retained");
        return;
    }
    NSString *path=[documents() stringByAppendingPathComponent:@"MadeiraCaches/DXMT"];
    if (![[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:0]) {
        trace(@"DXMT shader cache directory could not be created");
        return;
    }
    if (setenv("DXMT_SHADER_CACHE_PATH",path.UTF8String,0) == 0)
        trace(@"Persistent DXMT shader cache directory configured");
    else trace(@"DXMT shader cache environment could not be configured");
}

static void appendString(NSMutableData *data, NSString *s) {
    NSData *utf16=[s dataUsingEncoding:0x94000100]; // UTF-16LE, without BOM
    unsigned short count=(unsigned short)(utf16.length / 2);
    [data appendBytes:&count length:2]; [data appendData:utf16];
}
static BOOL writeLaunchLink(NSString *target, NSString *args) {
    // MS-SHLLINK. Include Unicode LinkInfo paths and a working directory so games
    // find their sibling Data files, just as with the verified desktop shortcut.
    NSMutableData *data=[NSMutableData new];
    unsigned int header[19]={0x4c,0x00021401,0,0x000000c0,0x46000000,
        0xb6,0x20,0,0,0,0,0,0,0,0,1,0,0,0};
    [data appendBytes:header length:76];
    NSData *ansi=[target dataUsingEncoding:4];
    NSData *unicode=[target dataUsingEncoding:0x94000100];
    unsigned int base=36+17, suffix=base+(unsigned int)ansi.length+1;
    unsigned int ubase=suffix+1, usuffix=ubase+(unsigned int)unicode.length+2;
    unsigned int info[9]={usuffix+2,36,1,36,base,0,suffix,ubase,usuffix};
    unsigned int volume[4]={17,3,0,16};
    unsigned int zero=0;
    [data appendBytes:info length:36]; [data appendBytes:volume length:16];
    [data appendBytes:&zero length:1]; [data appendData:ansi];
    [data appendBytes:&zero length:2]; [data appendData:unicode];
    [data appendBytes:&zero length:4];
    NSString *working=[[[target stringByReplacingOccurrencesOfString:@"\\" withString:@"/"]
        stringByDeletingLastPathComponent] stringByReplacingOccurrencesOfString:@"/" withString:@"\\"];
    if ([target.lowercaseString containsString:@"\\bin64\\beamng.drive.x64.exe"])
        working=[[[working stringByReplacingOccurrencesOfString:@"\\" withString:@"/"]
            stringByDeletingLastPathComponent] stringByReplacingOccurrencesOfString:@"/" withString:@"\\"];
    appendString(data,@"Madeira game launch"); appendString(data,working); appendString(data,args);
    [data appendBytes:&zero length:4];
    return [data writeToFile:[documents() stringByAppendingPathComponent:@"wine/drive_c/Games/MadeiraLaunch.lnk"] atomically:YES];
}

static BOOL installBeamNGSupport(void) {
    NSFileManager *fm=[NSFileManager defaultManager];
    NSString *bundled=[NSBundle mainBundle].bundlePath;
    NSString *sourceRoot=[bundled stringByAppendingPathComponent:@"BeamNG034Support"];
    NSString *support=[documents() stringByAppendingPathComponent:@"wine/drive_c/MadeiraDiagnostics/BeamNG034"];
    NSString *user=[support stringByAppendingPathComponent:@"User/0.34"];
    if (![fm createDirectoryAtPath:user withIntermediateDirectories:YES attributes:nil error:0]) return NO;
    NSData *launcher=[NSData dataWithContentsOfFile:
        [sourceRoot stringByAppendingPathComponent:@"MadeiraBeamNG034Release.exe"]];
    if (!launcher || ![launcher writeToFile:
        [support stringByAppendingPathComponent:@"MadeiraBeamNG034Release.exe"] atomically:YES]) return NO;
    for (NSString *name in @[@"usp10.dll", @"dxva2.dll",
        @"madeira_content_session.lua", @"madeira_release_startup_v1.lua",
        @"madeira_gridmap_interpreter_startup_v1.lua", @"madeira_gridmap_interpreter_status_v1.lua",
        @"madeira_vehicle_interpreter_startup_v1.lua", @"madeira_vehicle_interpreter_v1.lua",
        @"madeira_material_registration_v1.lua", @"madeira_vehicle_lua_startup_v1.lua"]) {
        NSData *data=[NSData dataWithContentsOfFile:[sourceRoot stringByAppendingPathComponent:name]];
        if (!data || ![data writeToFile:[user stringByAppendingPathComponent:name] atomically:YES]) return NO;
    }
    NSString *bin64=[documents() stringByAppendingPathComponent:@"wine/drive_c/Games/BeamNG034/Bin64"];
    NSString *game=[bin64 stringByAppendingPathComponent:@"BeamNG.drive.x64.exe"];
    if ([fm fileExistsAtPath:game isDirectory:0]) {
        for (NSString *name in @[@"usp10.dll", @"dxva2.dll"]) {
            NSData *data=[NSData dataWithContentsOfFile:[sourceRoot stringByAppendingPathComponent:name]];
            if (!data) return NO;
            NSString *target=[bin64 stringByAppendingPathComponent:name];
            NSData *existing=[NSData dataWithContentsOfFile:target];
            if (existing && ![existing isEqualToData:data]) {
                trace([@"Different existing CEF dependency preserved: " stringByAppendingString:name]);
                return NO;
            }
            if (!existing && ![data writeToFile:target atomically:YES]) return NO;
        }
        trace(@"BeamNG 0.34 CEF dependencies ready");
    }
    trace(@"BeamNG 0.34 content-session support ready");
    return YES;
}

static void ensureBeamNGDesktopShortcut(void) {
    NSFileManager *fm=[NSFileManager defaultManager];
    NSString *games=[documents() stringByAppendingPathComponent:@"wine/drive_c/Games"];
    [fm createDirectoryAtPath:games withIntermediateDirectories:YES attributes:nil error:0];
    NSString *temporary=[games stringByAppendingPathComponent:@"MadeiraLaunch.lnk"];
    if (!writeLaunchLink(@"C:\\MadeiraDiagnostics\\BeamNG034\\MadeiraBeamNG034Release.exe", @"")) return;
    NSData *link=[NSData dataWithContentsOfFile:temporary];
    BOOL wrote=NO;
    for (NSString *profile in @[@"mobile", @"mythic", @"madeira"]) {
        NSString *desktop=[documents() stringByAppendingPathComponent:
            [NSString stringWithFormat:@"wine/drive_c/users/%@/Desktop",profile]];
        [fm createDirectoryAtPath:desktop withIntermediateDirectories:YES attributes:nil error:0];
        NSString *shortcut=[desktop stringByAppendingPathComponent:@"BeamNG.drive 0.34.lnk"];
        if (link && [link writeToFile:shortcut atomically:YES]) wrote=YES;
    }
    if (wrote) trace(@"BeamNG 0.34 desktop shortcut ready in supported Wine profiles");
    [fm removeItemAtPath:temporary error:0];
}

static void provisionBeamNGOffMain(void) {
    static long once;
    dispatch_once(&once, ^{
        dispatch_async(dispatch_get_global_queue(0, 0), ^{
            trace(@"BeamNG 0.34 background setup started");
            if (installBeamNGSupport()) ensureBeamNGDesktopShortcut();
            else trace(@"BeamNG 0.34 background support installation failed");
            trace(@"BeamNG 0.34 background setup finished");
        });
    });
}

@interface MadeiraOverlayWindow : UIWindow @end
@interface UIWindow (MadeiraSceneAssignment)
- (void)setWindowScene:(UIWindowScene *)scene;
@end
#include "WindowLayers.h"
static BOOL sameRect(CGRect a, CGRect b);
static void refreshOwnedWindow(UIWindow *window, CGRect frame, CGFloat level) {
    if (!window) return;
    if (window.windowLevel != level) {
        window.windowLevel=level;
        trace(@"App overlay window ordering repaired");
    }
    if (!sameRect(window.frame,frame)) {
        window.frame=frame;
        // SwiftUI's landscape-only branch and hit testing must see the same
        // geometry as the live game, even if its rotation callback was missed.
        trace(@"App overlay window geometry repaired");
    }
    UIView *root=window.rootViewController.view;
    if (root && !sameRect(root.frame,window.bounds)) root.frame=window.bounds;
    // Do not make this window key or overwrite hidden/alpha state. The
    // keyboard, user-selected touch visibility and toolbar idle timer own it.
}
@implementation MadeiraOverlayWindow
- (UIView *)hitTest:(CGPoint)p withEvent:(UIEvent *)e {
    UIView *hit = [super hitTest:p withEvent:e];
    if (!self.rootViewController.presentedViewController &&
        (hit == self || hit == self.rootViewController.view)) return nil;
    return hit;
}
@end

@interface MadeiraOverlayController : UIViewController
@property BOOL immersive;
@end
@interface UIViewController (TouchLayoutDeclaration)
- (void)viewDidLayoutSubviews;
@end
@implementation MadeiraOverlayController
- (BOOL)prefersStatusBarHidden { return self.immersive; }
- (BOOL)prefersHomeIndicatorAutoHidden { return self.immersive; }
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    MadeiraTouchLayoutChanged(self.view.window);
}
@end

@interface MadeiraIPadUI : NSObject
@property(strong) MadeiraOverlayWindow *overlay;
@property(weak) UIWindow *gameWindow;
@property(weak) UIView *metalView;
@property(weak) UIView *metalHost;
@property(strong) UIButton *fullscreenButton;
@property(strong) UIButton *menuButton;
@property(strong) UIVisualEffectView *fullscreenGlass;
@property(strong) UIVisualEffectView *menuGlass;
@property(strong) UIView *logoMask;
@property(strong) UIView *deviceMask;
@property(strong) NSArray<UILabel *> *rainbowLetters;
@property(strong) NSTimer *timer;
@property(strong) NSTimer *rainbowTimer;
@property BOOL launching;
@property BOOL debugEnabled;
@property BOOL fullscreen;
@property BOOL presentationReady;
@property BOOL toolbarHidden;
@property NSInteger toolbarIdleTicks;
@property NSUInteger rainbowFrame;
@property NSInteger appliedAppearance;
@property(strong) NSArray<UIViewController *> *presentationRoots;
@property(strong) UITapGestureRecognizer *restoreGesture;
@property CGSize lastDrawableSize;
@property NSInteger lastStatusBarHidden;
@property NSUInteger controllerPendingActions;
@property NSInteger controllerLastGate;
@property(weak) UIAlertController *controllerTestAlert;
- (void)attach:(UIView *)view;
- (void)refresh;
- (void)menu;
- (void)setupGuide;
- (void)about;
- (void)more;
- (void)shareLogs;
- (void)toggleFullscreen;
- (void)restoreToolbar;
- (void)applyAppearance;
- (void)toolbarTick;
- (void)rainbowTick;
- (void)controllerUpdateGate;
- (void)controllerCheckGateSoon:(NSUInteger)remaining;
- (void)controllers;
- (void)controllerTest;
- (void)controllerRefreshTest:(UIAlertController *)alert;
- (void)resolutionSettings;
- (void)rendererExperiments;
- (void)customResolutionPrompt;
- (void)message:(NSString *)title text:(NSString *)text;
@end
static MadeiraIPadUI *manager;
void MadeiraTouchToggleKeyboard(void) {
    if (manager.metalView.isFirstResponder) [manager.metalView resignFirstResponder];
    else [manager.metalView becomeFirstResponder];
}
static UIView *findMetal(UIView *root, Class cls, int depth);

// Only the exact classes of Madeira's scene-window roots are modified. Each
// method checks the receiver against those roots and preserves the original
// implementation for every other receiver, including UIKit's modal UI.
static Class presentationHookClasses[32];
static unsigned int presentationHookCount;
static BOOL immersiveRoot(id controller) {
    if (!manager.fullscreen) return NO;
    for (UIViewController *root in manager.presentationRoots)
        if (root == controller) return YES;
    return NO;
}
static void hookPresentationRoot(UIViewController *root) {
    Class cls=object_getClass(root);
    // Never alter the global UIViewController implementation. Our own overlay
    // uses an explicit subclass instead of hooks.
    if (!cls || cls == [UIViewController class] ||
        [root isKindOfClass:[MadeiraOverlayController class]]) return;
    for (unsigned int i=0; i<presentationHookCount; i++)
        if (presentationHookClasses[i] == cls) return;
    if (presentationHookCount == 32) return;
    SEL selectors[4]={@selector(prefersStatusBarHidden), @selector(prefersHomeIndicatorAutoHidden),
        @selector(childViewControllerForStatusBarHidden), @selector(childViewControllerForHomeIndicatorAutoHidden)};
    void *originals[4];
    const char *encodings[4];
    for (unsigned int i=0; i<4; i++) {
        void *method=class_getInstanceMethod(cls,selectors[i]);
        if (!method) return;
        originals[i]=method_getImplementation(method);
        encodings[i]=method_getTypeEncoding(method);
    }
    presentationHookClasses[presentationHookCount++]=cls;
    for (unsigned int i=0; i<4; i++) {
        SEL selector=selectors[i];
        void *replacement;
        // A distinct block captures each class's original IMP. Unlike resolving
        // an IMP using object_getClass in a shared hook, this also preserves
        // correct super calls when two hooked classes share an ancestry.
        if (i < 2) {
            BOOL (*original)(id,SEL)=originals[i];
            replacement=imp_implementationWithBlock(^BOOL(id receiver) {
                return immersiveRoot(receiver) ? YES : original(receiver,selector);
            });
        } else {
            UIViewController *(*original)(id,SEL)=originals[i];
            replacement=imp_implementationWithBlock(^UIViewController *(id receiver) {
                return immersiveRoot(receiver) ? nil : original(receiver,selector);
            });
        }
        if (!class_addMethod(cls,selector,replacement,encodings[i]))
            class_replaceMethod(cls,selector,replacement,encodings[i]);
    }
    trace([NSString stringWithFormat:@"Presentation preferences installed on %s",class_getName(cls)]);
}
static BOOL sameRect(CGRect a, CGRect b) {
    return a.origin.x==b.origin.x && a.origin.y==b.origin.y &&
        a.size.width==b.size.width && a.size.height==b.size.height;
}
static BOOL nativeGlassAvailable(void) {
    Class effect=objc_getClass("UIGlassEffect");
    return effect && [effect respondsToSelector:@selector(effectWithStyle:)];
}
static UIVisualEffectView *nativeGlassBackdrop(void) {
    Class effect=objc_getClass("UIGlassEffect");
    if (!effect || ![effect respondsToSelector:@selector(effectWithStyle:)]) return nil;
    // UIGlassEffectStyleRegular: system-managed contrast over the background.
    id material=[(id)effect effectWithStyle:0];
    if (!material || ![material isKindOfClass:effect]) {
        trace(@"System UIGlassEffect could not be instantiated");
        return nil;
    }
    [material setTintColor:[UIColor colorWithWhite:1 alpha:0.08]];
    UIVisualEffectView *view=[[UIVisualEffectView alloc] initWithEffect:material];
    if (![view.effect isKindOfClass:effect]) {
        trace(@"System glass visual effect view did not retain UIGlassEffect");
        return nil;
    }
    trace([NSString stringWithFormat:@"Native glass material active: %s",class_getName(object_getClass(material))]);
    view.userInteractionEnabled=NO;
    view.clipsToBounds=YES;
    view.layer.cornerRadius=22;
    view.layer.borderWidth=0.8;
    view.layer.borderColor=[UIColor colorWithWhite:1 alpha:0.48].CGColor;
    view.hidden=YES;
    return view;
}
int MadeiraTouchGlassEnabled(void) {
    return [preferences() boolForKey:@"MadeiraIPad.liquidGlass"] && nativeGlassAvailable();
}
UIView *MadeiraTouchCreateGlass(void) { return nativeGlassBackdrop(); }

@implementation MadeiraIPadUI
- (UIButton *)button:(NSString *)title action:(SEL)selector {
    UIButton *button = [UIButton buttonWithType:1];
    [button setTitle:title forState:0];
    [button setTitleColor:[UIColor whiteColor] forState:0];
    button.titleLabel.font = [UIFont systemFontOfSize:14 weight:0.5];
    button.backgroundColor = [UIColor colorWithWhite:0.10 alpha:0.86];
    button.layer.cornerRadius = 12;
    [button addTarget:self action:selector forControlEvents:64];
    return button;
}
- (void)attach:(UIView *)view {
    if (!view.window || [view.window isKindOfClass:[MadeiraOverlayWindow class]]) return;
    if (self.gameWindow != view.window) {
        if (self.restoreGesture) [self.gameWindow removeGestureRecognizer:self.restoreGesture];
        self.gameWindow = view.window;
        self.restoreGesture=[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(restoreToolbar)];
        self.restoreGesture.numberOfTouchesRequired=3;
        self.restoreGesture.numberOfTapsRequired=1;
        self.restoreGesture.cancelsTouchesInView=NO;
        self.restoreGesture.delaysTouchesBegan=NO;
        self.restoreGesture.delaysTouchesEnded=NO;
        self.restoreGesture.delegate=self;
        [self.gameWindow addGestureRecognizer:self.restoreGesture];
    }
    self.metalView = view;
    if (self.overlay && self.overlay.windowScene!=view.window.windowScene) {
        MadeiraTouchReset();
        self.overlay.hidden=YES;
        [self.overlay setWindowScene:view.window.windowScene];
        self.overlay.frame=view.window.bounds;
        self.overlay.hidden=NO;
        self.presentationReady=NO;
        trace(@"Toolbar and native touch overlay moved to the active Madeira scene");
    }
    if (!self.overlay) {
        self.overlay = [[MadeiraOverlayWindow alloc] initWithWindowScene:view.window.windowScene];
        self.overlay.windowLevel = view.window.windowLevel + 110;
        self.overlay.backgroundColor = [UIColor clearColor];
        MadeiraOverlayController *root = [MadeiraOverlayController new];
        root.view.backgroundColor = [UIColor clearColor];
        self.overlay.rootViewController = root;
        self.fullscreenButton = [self button:@"Full screen" action:@selector(toggleFullscreen)];
        self.fullscreenButton.accessibilityLabel = @"Toggle fullscreen game view";
        self.menuButton = [self button:@"Menu" action:@selector(menu)];
        self.menuButton.accessibilityLabel = @"Madeira game menu";
        self.fullscreenGlass=nativeGlassBackdrop();
        self.menuGlass=nativeGlassBackdrop();
        // R6's centered "Madeira" navigation title is baked into the SwiftUI
        // executable. Cover only that title in portrait Controls and present
        // the same word at the left edge; these views never take input.
        self.logoMask=[[UIView alloc] initWithFrame:CGRectMake(0,0,1,1)];
        self.logoMask.backgroundColor=[UIColor colorWithWhite:0 alpha:1];
        self.logoMask.userInteractionEnabled=NO;
        self.deviceMask=[[UIView alloc] initWithFrame:CGRectMake(0,0,1,1)];
        self.deviceMask.backgroundColor=[UIColor colorWithWhite:0 alpha:1];
        self.deviceMask.userInteractionEnabled=NO;
        NSString *glyphs[]={@"M",@"a",@"d",@"e",@"i",@"r",@"a"};
        UIColor *neon[]={
            [UIColor colorWithRed:1.0 green:0.18 blue:0.38 alpha:1],
            [UIColor colorWithRed:1.0 green:0.55 blue:0.13 alpha:1],
            [UIColor colorWithRed:1.0 green:0.92 blue:0.16 alpha:1],
            [UIColor colorWithRed:0.35 green:1.0 blue:0.28 alpha:1],
            [UIColor colorWithRed:0.14 green:1.0 blue:0.96 alpha:1],
            [UIColor colorWithRed:0.29 green:0.55 blue:1.0 alpha:1],
            [UIColor colorWithRed:0.91 green:0.30 blue:1.0 alpha:1]
        };
        NSMutableArray<UILabel *> *letters=[NSMutableArray new];
        for (unsigned i=0;i<7;i++) {
            UILabel *letter=[[UILabel alloc] initWithFrame:CGRectMake(0,0,1,1)];
            letter.text=glyphs[i];
            letter.textColor=neon[i];
            letter.font=[UIFont systemFontOfSize:18 weight:0.6];
            letter.userInteractionEnabled=NO;
            letter.layer.shadowColor=neon[i].CGColor;
            letter.layer.shadowOpacity=0.95f;
            letter.layer.shadowRadius=7;
            letter.layer.shadowOffset=(CGSize){0,0};
            [letters addObject:letter];
        }
        self.rainbowLetters=letters;
        [root.view addSubview:self.logoMask];
        [root.view addSubview:self.deviceMask];
        for (UILabel *letter in letters) [root.view addSubview:letter];
        if (self.fullscreenGlass) [root.view addSubview:self.fullscreenGlass];
        if (self.menuGlass) [root.view addSubview:self.menuGlass];
        [root.view addSubview:self.fullscreenButton];
        [root.view addSubview:self.menuButton];
        self.overlay.hidden = NO;
        self.lastStatusBarHidden=-1;
        self.controllerLastGate=-1;
        __weak MadeiraIPadUI *weakSelf = self;
        self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 repeats:YES block:^(NSTimer *timer) {
            (void)timer;
            [weakSelf refresh];
            [weakSelf toolbarTick];
        }];
        self.rainbowTimer = [NSTimer scheduledTimerWithTimeInterval:0.10 repeats:YES block:^(NSTimer *timer) {
            (void)timer;
            [weakSelf rainbowTick];
        }];
        printf("[iPadUI] fullscreen toolbar and game library ready\n");
        trace(@"Toolbar attached to Madeira window");
        provisionBeamNGOffMain();
    }
    [self refresh];
}
- (void)refresh {
    controllerTryStart();
    MadeiraAudioProvision();
    [self controllerUpdateGate];
    if (!self.gameWindow) return;
    [self applyAppearance];
    CGRect frame = self.gameWindow.bounds;
    if (frame.size.width < 1 || frame.size.height < 1) return;
    if (!sameRect(self.overlay.frame,frame)) self.overlay.frame = frame;
    NSInteger mode = [preferences() integerForKey:@"MadeiraIPad.fullscreenMode"];
    BOOL full = mode == 1 || (mode == 0 && frame.size.width > frame.size.height);
    UIViewController *gameRoot=self.gameWindow.rootViewController;
    id traits = gameRoot.traitOverrides;
    if ([traits respondsToSelector:@selector(setVerticalSizeClass:)] &&
        [gameRoot.traitCollection verticalSizeClass] != (full ? 1 : 2)) {
        [traits setVerticalSizeClass:full ? 1 : 2];
        trace(full ? @"Fullscreen layout selected" : @"Controls layout selected");
        printf("[iPadUI] layout=%s\n", full ? "fullscreen" : "controls");
    }
    NSMutableArray *roots=[NSMutableArray new];
    Class pad=objc_getClass("_TtC7Madeira17PassthroughWindow");
    Class controls=objc_getClass("_TtC7Madeira14ControlsWindow");
    MadeiraWindowLayers levels;
    BOOL validLayers=MadeiraWindowLayersForGame(self.gameWindow.windowLevel,&levels);
    if (validLayers) refreshOwnedWindow(self.overlay,frame,levels.toolbar);
    for (UIWindow *window in self.gameWindow.windowScene.windows) {
        // Repair only the two known Madeira overlay classes in this scene.
        // Never resize or reorder UIKit keyboard/system windows.
        if (validLayers && pad && [window isKindOfClass:pad])
            refreshOwnedWindow(window,frame,levels.pad);
        else if (validLayers && controls && [window isKindOfClass:controls])
            refreshOwnedWindow(window,frame,levels.controls);
        // The idle joystick ring lives in its own click-through SwiftUI
        // window, so hiding the main Controls row does not hide it. Keep the
        // window alive for input/state, but suppress only its drawing view
        // while immersive fullscreen is active.
        if (pad && [window isKindOfClass:pad] && window.rootViewController.view &&
            window.rootViewController.view.hidden != full) {
            window.rootViewController.view.hidden=full;
            trace(full ? @"Fullscreen joystick face hidden" : @"Controls joystick face restored");
        }
        // Scope appearance changes to this app's game and its own touch/UI
        // overlays. Keyboard and other system windows retain their behavior.
        if (window != self.gameWindow && window != self.overlay &&
            !(pad && [window isKindOfClass:pad]) && !(controls && [window isKindOfClass:controls])) continue;
        if (window.rootViewController) [roots addObject:window.rootViewController];
    }
    BOOL ownersChanged=roots.count != self.presentationRoots.count;
    if (!ownersChanged) for (NSUInteger i=0; i<roots.count; i++)
        if (roots[i] != self.presentationRoots[i]) { ownersChanged=YES; break; }
    BOOL modeChanged=!self.presentationReady || self.fullscreen != full;
    if (modeChanged || ownersChanged) {
        NSArray *previousRoots=self.presentationRoots;
        self.fullscreen=full;
        self.presentationReady=YES;
        if (ownersChanged) self.presentationRoots=roots;
        ((MadeiraOverlayController *)self.overlay.rootViewController).immersive=full;
        for (UIViewController *root in roots) {
            hookPresentationRoot(root);
            [root setNeedsStatusBarAppearanceUpdate];
            [root setNeedsUpdateOfHomeIndicatorAutoHidden];
        }
        // Former roots are no longer subject to the override; clear any stale
        // preference if SwiftUI or a scene change replaced its controller.
        if (ownersChanged) for (UIViewController *old in previousRoots) {
            BOOL retained=NO;
            for (UIViewController *root in roots) if (old == root) retained=YES;
            if (!retained) {
                [old setNeedsStatusBarAppearanceUpdate];
                [old setNeedsUpdateOfHomeIndicatorAutoHidden];
            }
        }
        // A mode transition shows the controls long enough to learn the new
        // state; only the fullscreen idle timer hides them afterward.
        [self restoreToolbar];
        [self.fullscreenButton setTitle:full ? @"Controls" : @"Full screen" forState:0];
        trace([NSString stringWithFormat:@"Immersive UI %@ across %lu app window roots",full ? @"requested" : @"off",roots.count]);
    }
    BOOL left = [preferences() boolForKey:@"MadeiraIPad.toolbarLeft"];
    CGFloat x = left ? 14 : frame.size.width - 202;
    BOOL portrait=frame.size.height > frame.size.width;
    BOOL showLogo = !full;
    self.logoMask.hidden=!showLogo;
    self.deviceMask.hidden=!showLogo;
    for (UILabel *letter in self.rainbowLetters) letter.hidden=!showLogo;
    if (showLogo) {
        CGRect maskFrame=CGRectMake(frame.size.width/2-82, portrait ? 66 : 8, 164, portrait ? 32 : 22);
        if (!sameRect(self.logoMask.frame,maskFrame)) self.logoMask.frame=maskFrame;
        CGRect deviceFrame=portrait ? CGRectMake(frame.size.width-100,103,100,41) :
            CGRectMake(frame.size.width-150,24,130,38);
        if (!sameRect(self.deviceMask.frame,deviceFrame)) self.deviceMask.frame=deviceFrame;
        CGFloat glyphX=portrait ? 14 : 46;
        for (UILabel *letter in self.rainbowLetters) {
            CGSize fit=[letter sizeThatFits:(CGSize){120,32}];
            CGFloat glyphW=fit.width>3 ? fit.width+0.5 : 12;
            CGRect glyphFrame=CGRectMake(glyphX,portrait ? 69 : 4,glyphW,portrait ? 32 : 30);
            if (!sameRect(letter.frame,glyphFrame)) letter.frame=glyphFrame;
            glyphX+=glyphW;
        }
    }
    CGRect fullFrame=CGRectMake(x, 63, 112, 44), menuFrame=CGRectMake(x+120, 63, 68, 44);
    if (!sameRect(self.fullscreenButton.frame,fullFrame)) self.fullscreenButton.frame=fullFrame;
    if (!sameRect(self.menuButton.frame,menuFrame)) self.menuButton.frame=menuFrame;
    if (self.fullscreenGlass && !sameRect(self.fullscreenGlass.frame,fullFrame)) self.fullscreenGlass.frame=fullFrame;
    if (self.menuGlass && !sameRect(self.menuGlass.frame,menuFrame)) self.menuGlass.frame=menuFrame;
    // MetalBackedView receives input and follows SwiftUI geometry; the actual
    // CAMetalLayer is owned by a separate window-hosted MetalHostView singleton.
    if (!self.metalHost || self.metalHost.window != self.gameWindow) {
        Class hostClass=objc_getClass("_TtC7Madeira13MetalHostView");
        if (!hostClass) hostClass=objc_getClass("Madeira.MetalHostView");
        if (hostClass) self.metalHost=findMetal(self.gameWindow,hostClass,0);
    }
    CALayer *layer=self.metalHost.layer;
    if ([layer respondsToSelector:@selector(drawableSize)]) {
        CGSize size=[layer drawableSize];
        if (size.width != self.lastDrawableSize.width || size.height != self.lastDrawableSize.height) {
            self.lastDrawableSize=size;
            trace([NSString stringWithFormat:@"Metal drawable %.0f x %.0f pixels",size.width,size.height]);
        }
    }
    UIStatusBarManager *status=self.gameWindow.windowScene.statusBarManager;
    if ([status respondsToSelector:@selector(isStatusBarHidden)]) {
        NSInteger hidden=status.isStatusBarHidden ? 1 : 0;
        if (hidden != self.lastStatusBarHidden) {
            self.lastStatusBarHidden=hidden;
            trace(hidden ? @"System status bar hidden" : @"System status bar visible");
        }
    }
    [self controllerUpdateGate];
}
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gesture shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)other {
    (void)gesture; (void)other;
    return YES;
}
- (void)restoreToolbar {
    self.toolbarHidden=NO;
    self.toolbarIdleTicks=0;
    [self applyAppearance];
}
- (void)applyAppearance {
    id traits=self.gameWindow.rootViewController.traitCollection;
    BOOL light=[traits respondsToSelector:@selector(userInterfaceStyle)] && [traits userInterfaceStyle]==1;
    NSInteger appearance=light ? 1 : 2;
    if (self.appliedAppearance!=appearance) {
        self.appliedAppearance=appearance;
        trace(light ? @"Light appearance: adaptive header and toolbar contrast" : @"Dark appearance: adaptive header and toolbar contrast");
    }
    // These masks cover baked-in title/readout artwork, not the game canvas.
    // Match the SwiftUI header instead of painting black rectangles on white.
    UIColor *surface=[[UIColor systemBackgroundColor] resolvedColorWithTraitCollection:traits];
    self.logoMask.backgroundColor=surface;
    self.deviceMask.backgroundColor=surface;
    BOOL glass=[preferences() boolForKey:@"MadeiraIPad.liquidGlass"] && nativeGlassAvailable();
    BOOL lightToolbar=light && !self.fullscreen;
    UIColor *ink=lightToolbar ? [[UIColor labelColor] resolvedColorWithTraitCollection:traits] : [UIColor whiteColor];
    UIColor *fill=lightToolbar ? [UIColor colorWithWhite:0.93 alpha:0.96] : [UIColor colorWithWhite:0.10 alpha:0.86];
    UIVisualEffectView *backdrops[]={self.fullscreenGlass,self.menuGlass};
    for (unsigned i=0;i<2;i++) {
        UIVisualEffectView *backdrop=backdrops[i];
        if (!backdrop) continue; // UIGlassEffect is unavailable on older iOS.
        backdrop.hidden=self.toolbarHidden || !glass;
        backdrop.layer.borderColor=[UIColor colorWithWhite:lightToolbar ? 0 : 1 alpha:lightToolbar ? 0.18 : 0.48].CGColor;
    }
    NSArray<UIButton *> *buttons=@[self.fullscreenButton,self.menuButton];
    NSArray<NSString *> *titles=@[self.fullscreen ? @"Controls" : @"Full screen", @"Menu"];
    for (NSUInteger i=0;i<buttons.count;i++) {
        UIButton *button=buttons[i];
        button.hidden=NO;
        button.configuration=nil;
        [button setTitle:titles[i] forState:0];
        button.layer.cornerRadius=glass ? 22 : 12;
        button.backgroundColor=self.toolbarHidden || glass ? [UIColor clearColor] : fill;
        UIColor *titleColor=self.toolbarHidden ? [UIColor clearColor] : ink;
        [button setTitleColor:titleColor forState:0];
        [button setTitleColor:titleColor forState:1];
    }
}
- (void)toolbarTick {
    if (!self.fullscreen || self.toolbarHidden || !self.overlay ||
        self.overlay.rootViewController.presentedViewController) {
        self.toolbarIdleTicks=0;
        return;
    }
    if (++self.toolbarIdleTicks >= 6) {
        [self hideToolbar];
        trace(@"Fullscreen toolbar auto-hidden after six idle seconds");
    }
}
- (void)rainbowTick {
    [self controllerRefreshTest:self.controllerTestAlert];
    if (self.logoMask.hidden || self.rainbowLetters.count != 7) return;
    static const CGFloat palette[7][3]={
        {1.0,0.18,0.38}, {1.0,0.55,0.13}, {1.0,0.92,0.16},
        {0.35,1.0,0.28}, {0.14,1.0,0.96}, {0.29,0.55,1.0},
        {0.91,0.30,1.0}
    };
    // Saturated darker rainbow on white: bright yellow/cyan neon is illegible
    // in light mode. Keep the same animation, but with accessible ink colors.
    static const CGFloat lightPalette[7][3]={
        {0.72,0.06,0.22}, {0.65,0.25,0.02}, {0.49,0.37,0.00},
        {0.08,0.43,0.18}, {0.00,0.40,0.46}, {0.16,0.30,0.72},
        {0.53,0.12,0.66}
    };
    BOOL light=self.appliedAppearance==1;
    const CGFloat (*colors)[3]=light ? lightPalette : palette;
    NSUInteger phase=self.rainbowFrame++ % 42;
    for (NSUInteger i=0;i<7;i++) {
        NSUInteger wave=(phase+i*6) % 42;
        NSUInteger from=wave/6, to=(from+1)%7;
        CGFloat blend=(CGFloat)(wave%6)/6.0;
        CGFloat red=colors[from][0]+(colors[to][0]-colors[from][0])*blend;
        CGFloat green=colors[from][1]+(colors[to][1]-colors[from][1])*blend;
        CGFloat blue=colors[from][2]+(colors[to][2]-colors[from][2])*blend;
        UIColor *color=[UIColor colorWithRed:red green:green blue:blue alpha:1];
        UILabel *letter=self.rainbowLetters[i];
        letter.textColor=color;
        letter.layer.shadowColor=color.CGColor;
        letter.layer.shadowOpacity=light ? 0.18f : 0.95f;
        letter.layer.shadowRadius=light ? 2 : 7;
    }
}
- (void)hideToolbar {
    if (!self.fullscreen) return;
    self.toolbarHidden=YES;
    self.toolbarIdleTicks=0;
    // Keep both UIKit buttons hit-testable. A hidden view (or alpha near zero)
    // cannot receive taps, so hide only their artwork and preserve their frames.
    for (UIButton *button in @[self.fullscreenButton,self.menuButton]) {
        button.hidden=NO;
        button.configuration=nil;
        button.backgroundColor=[UIColor clearColor];
        [button setTitleColor:[UIColor clearColor] forState:0];
        [button setTitleColor:[UIColor clearColor] forState:1];
    }
    if (self.fullscreenGlass) self.fullscreenGlass.hidden=YES;
    if (self.menuGlass) self.menuGlass.hidden=YES;
}
- (void)toggleFullscreen {
    BOOL full = self.fullscreen;
    [preferences() setInteger:full ? 2 : 1 forKey:@"MadeiraIPad.fullscreenMode"];
    [self refresh];
}
- (void)option:(UIAlertController *)alert title:(NSString *)title action:(void (^)(void))action {
    [alert addAction:[UIAlertAction actionWithTitle:title style:0 handler:^(UIAlertAction *a) {
        (void)a;
        ++self.controllerPendingActions;
        [self controllerUpdateGate];
        later(0.35, ^{
            if (action) action();
            if (self.controllerPendingActions) --self.controllerPendingActions;
            [self controllerCheckGateSoon:20];
        });
    }]];
}
- (void)present:(UIAlertController *)alert {
    if (self.overlay.rootViewController.presentedViewController) return;
    [alert addAction:[UIAlertAction actionWithTitle:@"Close" style:1 handler:^(UIAlertAction *action) {
        (void)action; [self controllerCheckGateSoon:20];
    }]];
    ++self.controllerPendingActions;
    [self controllerUpdateGate];
    [self.overlay.rootViewController presentViewController:alert animated:YES completion:^{
        if (self.controllerPendingActions) --self.controllerPendingActions;
        [self controllerUpdateGate];
    }];
}
- (void)controllerUpdateGate {
    BOOL modal = NO;
    for (UIWindow *window in self.gameWindow.windowScene.windows)
        if (!window.hidden && window.rootViewController.presentedViewController) modal = YES;
    NSInteger enabled = controllerSessionEnabled && !modal && !self.controllerPendingActions && !self.launching && !MadeiraTouchIsEditing();
    MadeiraTouchRefresh(self.overlay,self.gameWindow,self.metalView,self.fullscreen,
        !modal && !self.controllerPendingActions && !self.launching && wine_running());
    if (controllerNativeStarted && enabled != self.controllerLastGate) {
        MadeiraControllerSetInputEnabled((int)enabled);
        self.controllerLastGate = enabled;
    }
}
- (void)controllerCheckGateSoon:(NSUInteger)remaining {
    [self controllerUpdateGate];
    if (!remaining) return;
    // Each delayed callback evaluates CURRENT ownership; it never blindly
    // enables input using a stale Close/submenu decision.
    __weak MadeiraIPadUI *weakSelf = self;
    later(0.05, ^{ [weakSelf controllerCheckGateSoon:remaining-1]; });
}
- (void)message:(NSString *)title text:(NSString *)text {
    [self present:[UIAlertController alertControllerWithTitle:title message:text preferredStyle:1]];
}
- (void)menu {
    [self restoreToolbar];
    [self refresh];
    unsigned long (*available)(void) = dlsym((void *)-2, "os_proc_available_memory");
    NSString *memory = available ? [NSString stringWithFormat:@"%.1f GB available to app", available() / 1000000000.0] : @"Memory reading unavailable";
    NSProcessInfo *process=[NSProcessInfo processInfo];
    NSInteger thermal=[process respondsToSelector:@selector(thermalState)] ? process.thermalState : -1;
    NSString *thermalLabel=thermal==0 ? @"nominal" : thermal==1 ? @"fair" :
        thermal==2 ? @"serious" : thermal==3 ? @"critical" : @"unavailable";
    NSString *lowPower=[process respondsToSelector:@selector(isLowPowerModeEnabled)] ?
        (process.isLowPowerModeEnabled ? @"on" : @"off") : @"unavailable";
    NSString *output=self.lastDrawableSize.width>0 ? [NSString stringWithFormat:@"Metal output: %.0f×%.0f",
        self.lastDrawableSize.width,self.lastDrawableSize.height] : @"Metal output size unavailable";
    NSString *status = [NSString stringWithFormat:@"JIT %@ · Wine %@\n%@\nThermal: %@ · Low Power Mode %@\n%@",
        jit_ready() ? @"ready" : @"needed", wine_running() ? @"running" : @"not started", memory,
        thermalLabel,lowPower,output];
    trace([NSString stringWithFormat:@"Menu status: thermal %@; Low Power Mode %@; %@",thermalLabel,lowPower,output]);
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Madeira · BeamNG 0.34" message:status preferredStyle:1];
    [self option:alert title:@"How to launch BeamNG.drive 0.34" action:^{ [self setupGuide]; }];
    [self option:alert title:@"Setup guide" action:^{ [self setupGuide]; }];
    [self option:alert title:@"Keyboard" action:^{
        if (self.metalView.isFirstResponder) [self.metalView resignFirstResponder];
        else [self.metalView becomeFirstResponder];
    }];
    [self option:alert title:@"Escape / pause" action:^{ if (wine_running()) { key(0x1b,1); later(0.06, ^{key(0x1b,0);}); } }];
    [self option:alert title:@"Hide toolbar · buttons stay tappable" action:^{ [self hideToolbar]; }];
    [self option:alert title:@"Display & support" action:^{ [self more]; }];
    [self option:alert title:[NSString stringWithFormat:@"Controllers · %@", controllerSummary()]
            action:^{ [self controllers]; }];
    [self option:alert title:@"About this build" action:^{ [self about]; }];
    [self present:alert];
}
- (void)setupGuide {
    [self message:@"BeamNG.drive 0.34 setup" text:@"1. Copy a complete, legally owned Windows installation of BeamNG.drive 0.34 into Files → Madeira → wine → drive_c → Games, and name its folder BeamNG034.\n\n2. Enable JIT for Madeira.\n\n3. Start Wine Virtual Desktop from Controls.\n\n4. Double-tap the BeamNG.drive 0.34 desktop shortcut. Opening Bin64/BeamNG.drive.x64.exe directly skips the compatibility setup.\n\nGame files are not included in this app."];
}
- (void)about {
    [self message:@"Madeira 0.34 Compatibility · Resolution Candidate" text:@"Community build based on Release 6. The Wine desktop defaults to 1280×720; Display & support can save a custom or native pixel size for the next Wine desktop launch. Fullscreen presentation, BeamNG 0.34 support, shader cache, diagnostics, and optional PlayStation controllers remain.\n\nThis build is not affiliated with BeamNG GmbH, Apple, or CodeWeavers. Madeira is free software under GPLv3. Game files are never included."];
}
- (void)more {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Display & support"
        message:@"Fullscreen hides the system status bar and lets the home indicator fade. The Controls and Menu buttons disappear after six seconds but remain tappable in the same spots. A three-finger tap can make them visible again. Heavy diagnostics can reduce performance and remain off until explicitly enabled." preferredStyle:1];
    [self option:alert title:@"Automatic fullscreen in landscape" action:^{
        [preferences() setInteger:0 forKey:@"MadeiraIPad.fullscreenMode"]; [self refresh];
    }];
    int width = 0, height = 0;
    BOOL selected = madeira_resolution_effective(&width, &height);
    NSString *resolution = selected ? [NSString stringWithFormat:@"Wine desktop · %d×%d%@", width, height,
        madeira_resolution_is_native() ? @" · native" : @""] : @"Wine desktop · default 1280×720";
    [self option:alert title:resolution action:^{ [self resolutionSettings]; }];
    [self option:alert title:@"Move toolbar to other side" action:^{
        [preferences() setBool:![preferences() boolForKey:@"MadeiraIPad.toolbarLeft"] forKey:@"MadeiraIPad.toolbarLeft"]; [self refresh];
    }];
    if (nativeGlassAvailable()) {
        BOOL enabled=[preferences() boolForKey:@"MadeiraIPad.liquidGlass"];
        [self option:alert title:enabled ? @"Liquid Glass overlay buttons · On" : @"Liquid Glass overlay buttons · Off" action:^{
            [preferences() setBool:!enabled forKey:@"MadeiraIPad.liquidGlass"];
            [self restoreToolbar];
            trace(!enabled ? @"Native Liquid Glass overlay buttons enabled" : @"Native Liquid Glass overlay buttons disabled");
            [self more];
        }];
    }
    [self option:alert title:@"Enter / confirm" action:^{ if (wine_running()) {key(0x0d,1); later(0.06, ^{key(0x0d,0);});} }];
    [self option:alert title:self.debugEnabled ? @"Runtime & audio diagnostics · On" : @"Runtime & audio diagnostics · Off" action:^{
        self.debugEnabled=!self.debugEnabled; diagnostics(self.debugEnabled);
        [self message:@"Diagnostics" text:self.debugEnabled ? @"Runtime probes and audio sample telemetry enabled for this session. Turn off after capturing a problem." : @"Runtime probes and audio sample telemetry disabled. Audio timing and controller support remain active. Startup/error logs and renderer-internal counters are separate."];
    }];
    [self option:alert title:@"Share startup logs" action:^{ [self shareLogs]; }];
    [self option:alert title:@"DX11 & Metal experiments" action:^{ [self rendererExperiments]; }];
    [self present:alert];
}
- (void)rendererExperiments {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"DX11 & Metal experiments"
        message:@"Experimental settings are off by default. Save your game, close Madeira fully, and reopen it to apply changes to a fresh Wine session. No game settings or files are changed. Performance gains are not guaranteed." preferredStyle:1];
    BOOL upload=[preferences() boolForKey:@"MadeiraDX11.uploadWatermark"];
    [self option:alert title:upload ? @"Completed-upload reuse · On" : @"Completed-upload reuse · Off" action:^{
        [preferences() setBool:!upload forKey:@"MadeiraDX11.uploadWatermark"];
        [self message:@"Saved for next launch" text:@"Completed-upload reuse samples GPU upload completion to reuse eligible memory sooner. It is experimental and may add CPU overhead. Close and reopen Madeira after saving your game. Turn it off if behavior worsens."];
    }];
    BOOL rendererDiagnostics=[preferences() boolForKey:@"MadeiraDX11.diagnostics"];
    [self option:alert title:rendererDiagnostics ? @"Renderer diagnostics · On" : @"Renderer diagnostics · Off" action:^{
        [preferences() setBool:!rendererDiagnostics forKey:@"MadeiraDX11.diagnostics"];
        [self message:@"Saved for next launch" text:@"Renderer memory census and covered allocation statistics are collected only when enabled. This is separate from live runtime/audio telemetry. Close and reopen Madeira to apply; keep off during normal play."];
    }];
    [self present:alert];
}
- (void)resolutionSettings {
    int width = 0, height = 0;
    BOOL selected = madeira_resolution_effective(&width, &height);
    NSString *details = selected ? [NSString stringWithFormat:@"Saved Wine desktop: %d×%d%@. %@", width, height,
        madeira_resolution_is_native() ? @" (native pixels)" : @"",
        wine_running() ? @"The current Wine desktop will not change; launch a new desktop to apply this choice." :
                         @"Start Wine Virtual Desktop to apply it without reopening Madeira."] :
        @"Default desktop: 1280×720. Choose native pixels or enter an even custom width and height.";
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Wine desktop resolution"
        message:details preferredStyle:1];
    [self option:alert title:@"Use device native pixels" action:^{
        int nativeW = 0, nativeH = 0;
        if (!madeira_resolution_native(&nativeW, &nativeH)) {
            [self message:@"Native size unavailable" text:@"This device's native dimensions are outside the supported 640–4096 by 360–4096 range."];
            return;
        }
        [preferences() setBool:YES forKey:MADEIRA_RESOLUTION_NATIVE_KEY];
        [preferences() setInteger:1 forKey:@"MadeiraIPad.fullscreenMode"];
        [self refresh];
        trace([NSString stringWithFormat:@"Native Wine desktop requested: %d×%d", nativeW, nativeH]);
        [self message:@"Native size saved" text:wine_running() ?
            @"Fullscreen presentation is active now. To change actual Wine/game pixels, close the current Wine desktop and start a new one; Madeira itself can stay open." :
            @"Fullscreen presentation is active. Start Wine Virtual Desktop to use the device's native pixel dimensions."];
    }];
    [self option:alert title:@"Set custom width and height" action:^{ [self customResolutionPrompt]; }];
    [self option:alert title:@"Restore default 1280×720" action:^{
        [preferences() setBool:NO forKey:MADEIRA_RESOLUTION_NATIVE_KEY];
        [preferences() setInteger:0 forKey:MADEIRA_RESOLUTION_WIDTH_KEY];
        [preferences() setInteger:0 forKey:MADEIRA_RESOLUTION_HEIGHT_KEY];
        [preferences() setBool:NO forKey:MADEIRA_RESOLUTION_MATCH_KEY];
        trace(@"Wine desktop default restored");
        [self message:@"Default saved" text:wine_running() ?
            @"The current Wine desktop remains unchanged. Close it and start a new one to use 1280×720; Madeira itself can stay open." :
            @"The next Wine Virtual Desktop launch will use 1280×720."];
    }];
    [self present:alert];
}
- (void)customResolutionPrompt {
    int width = 0, height = 0, match = 0;
    BOOL selected = madeira_resolution_saved(&width, &height, &match) && !madeira_resolution_is_native();
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Custom Wine desktop"
        message:@"Enter even pixel dimensions. Width 640–4096; height 360–4096. A size with the game area's aspect ratio fills that area without stretching." preferredStyle:1];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *field) {
        field.placeholder = @"Width in pixels";
        field.keyboardType = 4;
        field.text = [NSString stringWithFormat:@"%d", selected ? width : 1280];
    }];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *field) {
        field.placeholder = @"Height in pixels";
        field.keyboardType = 4;
        field.text = [NSString stringWithFormat:@"%d", selected ? height : 720];
    }];
    __weak UIAlertController *weakAlert = alert;
    [alert addAction:[UIAlertAction actionWithTitle:@"Save" style:0 handler:^(UIAlertAction *action) {
        (void)action;
        NSArray<UITextField *> *fields = weakAlert.textFields;
        [self controllerCheckGateSoon:20];
        if (fields.count != 2) return;
        int chosenW = madeira_resolution_parse_even(fields[0].text.UTF8String, 640, 4096);
        int chosenH = madeira_resolution_parse_even(fields[1].text.UTF8String, 360, 4096);
        if (!chosenW || !chosenH) {
            later(0.35, ^{ [self message:@"Invalid resolution" text:@"Use even numbers: width 640–4096 and height 360–4096."]; });
            return;
        }
        [preferences() setBool:NO forKey:MADEIRA_RESOLUTION_NATIVE_KEY];
        [preferences() setBool:NO forKey:MADEIRA_RESOLUTION_MATCH_KEY];
        [preferences() setInteger:chosenW forKey:MADEIRA_RESOLUTION_WIDTH_KEY];
        [preferences() setInteger:chosenH forKey:MADEIRA_RESOLUTION_HEIGHT_KEY];
        trace([NSString stringWithFormat:@"Custom Wine desktop saved: %d×%d", chosenW, chosenH]);
        later(0.35, ^{ [self message:@"Resolution saved" text:wine_running() ?
            @"The current Wine desktop remains unchanged. Close it and start a new one to apply the new pixel size; Madeira itself can stay open." :
            @"Start Wine Virtual Desktop to apply this size without reopening Madeira."]; });
    }]];
    [self present:alert];
}
- (void)shareLogs {
    NSMutableArray *files=[NSMutableArray new];
    NSFileManager *fm=[NSFileManager defaultManager];
    for (NSString *name in @[@"madeira-log.txt",
        @"wine/drive_c/MadeiraDiagnostics/BeamNG034/beamng034-release-launcher.log",
        @"wine/drive_c/MadeiraDiagnostics/BeamNG034/User/0.34/beamng.log",
        @"wine/drive_c/MadeiraDiagnostics/BeamNG034/User/0.34/cefdev.log",
        @"madeira-ipad-ui.log"]) {
        NSString *path=[documents() stringByAppendingPathComponent:name];
        if ([fm fileExistsAtPath:path isDirectory:0]) [files addObject:[NSURL fileURLWithPath:path]];
    }
    if (!files.count) { [self message:@"No logs yet" text:@"Launch a game first, then return here."]; return; }
    UIActivityViewController *share=[[UIActivityViewController alloc] initWithActivityItems:files applicationActivities:nil];
    [share.popoverPresentationController setSourceView:self.menuButton];
    [share.popoverPresentationController setSourceRect:self.menuButton.bounds];
    if (self.overlay.rootViewController.presentedViewController) return;
    ++self.controllerPendingActions;
    [self controllerUpdateGate];
    share.completionWithItemsHandler = ^(NSString *type, BOOL completed, NSArray *items, NSError *error) {
        (void)type; (void)completed; (void)items; (void)error;
        dispatch_async(&_dispatch_main_q, ^{ [self controllerCheckGateSoon:20]; });
    };
    [self.overlay.rootViewController presentViewController:share animated:YES completion:^{
        if (self.controllerPendingActions) --self.controllerPendingActions;
        [self controllerUpdateGate];
    }];
}
- (void)controllers {
    BOOL enabled = controllerEnabled();
    NSString *details = [NSString stringWithFormat:@"%@\n%@\n%@\n\nPair your controller in iPhone Settings → Bluetooth. Madeira detects supported extended gamepads automatically, including PlayStation and Xbox profiles.\n\nOne neutral virtual gamepad stays available from game startup, so a controller connected later can feed that same gamepad without rediscovery. Disconnecting clears its input.\n\nTest buttons & sticks shows physical input even while this menu pauses game input. Sticks and triggers stay analog. Rumble, motion and adaptive triggers are not included.",
        controllerSummary(), controllerStateLabel, controllerDeploymentLabel];
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Controllers" message:details preferredStyle:1];
    [self option:alert title:enabled ? @"Controller input · On → Off" : @"Controller input · Off → On" action:^{
        controllerSessionEnabled = !enabled;
        [preferences() setBool:controllerSessionEnabled forKey:@"MadeiraIPad.controllersEnabled"];
        mc_retry_reset(&controllerRetry);
        controllerTryStart();
        [self controllerUpdateGate];
        [self controllers];
    }];
    [self option:alert title:@"Test buttons & sticks" action:^{ [self controllerTest]; }];
    [self option:alert title:@"Retry game input setup" action:^{
        if (!controllerEnvironmentReady) controllerConfigureEnvironment();
        mc_retry_reset(&controllerRetry);
        controllerPrepareGames();
        controllerTryStart();
        self.controllerLastGate = -1;
        [self controllerUpdateGate];
        [self message:@"Controller setup" text:[NSString stringWithFormat:
            @"%@\n\nGame folders are being checked in the background. Existing third-party DLLs are preserved. If game input files were just installed, relaunch the game so it loads them; normal On/Off and reconnects do not require restarting Madeira.", controllerStateLabel]];
    }];
    [self option:alert title:@"Refresh status" action:^{ [self controllers]; }];
    [self present:alert];
}
- (void)controllerTest {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Live controller test"
        message:[NSString stringWithUTF8String:MadeiraControllerDiagnostics()] preferredStyle:1];
    self.controllerTestAlert = alert;
    [self present:alert];
}
- (void)controllerRefreshTest:(UIAlertController *)alert {
    if (!alert || !alert.view.window) return;
    alert.message = [NSString stringWithUTF8String:MadeiraControllerDiagnostics()];
}
@end

static void (*oldLayout)(id,SEL);
static UIView *findMetal(UIView *root, Class cls, int depth) {
    if (!root || depth > 24) return nil;
    if ([root isKindOfClass:cls]) return root;
    for (UIView *child in root.subviews) {
        UIView *found=findMetal(child,cls,depth+1);
        if (found) return found;
    }
    return nil;
}
static void discover(Class cls, int attempt) {
    for (UIWindow *window in [UIApplication sharedApplication].windows) {
        if ([window isKindOfClass:[MadeiraOverlayWindow class]]) continue;
        UIView *view=findMetal(window,cls,0);
        if (view) {
            if (!manager) manager=[MadeiraIPadUI new];
            [manager attach:view];
            return;
        }
    }
    if (attempt < 30) later(0.5, ^{discover(cls,attempt+1);});
    else trace(@"Metal view discovery timed out");
}
static void layoutHook(id view, SEL selector) {
    oldLayout(view,selector);
    __weak UIView *weakView=view;
    dispatch_async(&_dispatch_main_q, ^{
        UIView *live=weakView;
        if (!live.window) return;
        if (!manager) manager=[MadeiraIPadUI new];
        [manager attach:live];
    });
}
static void install(void) {
    static int retries;
    if (!retries) {
        trace(@"iPad UI library initialized");
        trace([NSString stringWithFormat:@"Madeira runtime dyld image index: %d of %u",
            madeiraImageIndex(),_dyld_image_count()]);
        // Keep potentially slow file I/O away from UIKit startup and layout.
        provisionBeamNGOffMain();
        trace(@"iPad UI main-queue runtime setup started");
        configureRuntime();
        trace(@"iPad UI main-queue runtime setup finished");
    }
    Class cls=objc_getClass("_TtC7Madeira15MetalBackedView");
    if (!cls) cls=objc_getClass("Madeira.MetalBackedView");
    void *method=cls ? class_getInstanceMethod(cls,@selector(layoutSubviews)) : 0;
    if (!method) {
        if (++retries < 50) later(0.1, ^{install();});
        else trace(@"MetalBackedView class lookup timed out");
        return;
    }
    oldLayout=method_getImplementation(method);
    class_replaceMethod(cls,@selector(layoutSubviews),(void *)layoutHook,method_getTypeEncoding(method));
    trace(@"Layout callback installed");
    discover(cls,0);
}
__attribute__((constructor)) static void initialize(void) {
    dispatch_async(&_dispatch_main_q, ^{install();});
}
