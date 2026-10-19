// IOSDisplayShim.m — iOS stand-in for Wine's mac driver, used by DXMT.
//
// DXMT (src/winemetal/unix/winemetal_unix.c) looks up this API via
// dlsym(RTLD_DEFAULT, "macdrv_functions") to obtain a CAMetalLayer for a
// given HWND. We export those symbols from the main binary so DXMT finds
// them in the same process.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <pthread.h>

#include "IOSDisplayShim.h"
#include "InputSurfaceState.h"
#include "GPUForegroundGate.h"
#include "GPUFrameDiagnostics.h"
#include "MetalLayerConfiguration.h"
#include <stdio.h>

void madeira_metal_layer_configure(CAMetalLayer *layer, int panel_hz) {
    int sync = madeira_layer_set_sync(layer, NO);
    int rate = madeira_layer_set_nominal_rate(layer, panel_hz);
    fprintf(stderr, "[metal-layer-scalars-v1] display-sync-off=%d nominal-rate-set=%d panel-max=%d argument-abi=scalar\n",
            sync, rate, panel_hz);
}

static pthread_mutex_t g_input_surface_lock = PTHREAD_MUTEX_INITIALIZER;
static struct madeira_input_surface g_input_surface;
extern void winios_drv_publish_input_rect(void *hwnd);

void madeira_input_surface_reset(void) {
    pthread_mutex_lock(&g_input_surface_lock);
    g_input_surface = (struct madeira_input_surface){0};
    pthread_mutex_unlock(&g_input_surface_lock);
}
void madeira_input_surface_window_destroyed(unsigned long long hwnd) {
    pthread_mutex_lock(&g_input_surface_lock);
    if (g_input_surface.window == (uintptr_t)hwnd) g_input_surface = (struct madeira_input_surface){0};
    pthread_mutex_unlock(&g_input_surface_lock);
}
int madeira_input_surface_tracks(unsigned long long hwnd) {
    pthread_mutex_lock(&g_input_surface_lock);
    int matches = hwnd && g_input_surface.window == (uintptr_t)hwnd;
    pthread_mutex_unlock(&g_input_surface_lock);
    return matches;
}
int madeira_input_surface_rect(int *x, int *y, int *width, int *height) {
    pthread_mutex_lock(&g_input_surface_lock);
    struct madeira_input_surface s = g_input_surface;
    pthread_mutex_unlock(&g_input_surface_lock);
    if (!s.valid) return 0;
    if (x) *x=s.x; if (y) *y=s.y; if (width) *width=s.width; if (height) *height=s.height;
    return 1;
}
void madeira_input_surface_update(unsigned long long hwnd, int x, int y, int width, int height) {
    static unsigned reported;
    pthread_mutex_lock(&g_input_surface_lock);
    int changed = madeira_input_surface_store(&g_input_surface, (uintptr_t)hwnd, x, y, width, height);
    int report = changed && reported++ < 24;
    pthread_mutex_unlock(&g_input_surface_lock);
    if (report) fprintf(stderr, "[presented-input-v1] hwnd=0x%llx client={%d,%d %dx%d} coordinates=screen\n", hwnd,x,y,width,height);
}

// --- Types mirroring DXMT's expectations (see winemetal_unix.c lines ~1524) ---

typedef struct macdrv_opaque_metal_device *macdrv_metal_device;
typedef struct macdrv_opaque_metal_view   *macdrv_metal_view;
typedef struct macdrv_opaque_metal_layer  *macdrv_metal_layer;
typedef struct macdrv_opaque_view         *macdrv_view;
typedef struct macdrv_opaque_window       *macdrv_window;
typedef struct opaque_HWND                *HWND;

struct macdrv_win_data {
    HWND         hwnd;
    macdrv_window cocoa_window;
    macdrv_view   cocoa_view;
    macdrv_view   client_cocoa_view;
};

struct macdrv_functions_t {
    void (*macdrv_init_display_devices)(BOOL);
    struct macdrv_win_data *(*get_win_data)(HWND hwnd);
    void (*release_win_data)(struct macdrv_win_data *data);
    macdrv_window (*macdrv_get_cocoa_window)(HWND hwnd, BOOL require_on_screen);
    macdrv_metal_device (*macdrv_create_metal_device)(void);
    void (*macdrv_release_metal_device)(macdrv_metal_device d);
    macdrv_metal_view (*macdrv_view_create_metal_view)(macdrv_view v, macdrv_metal_device d);
    macdrv_metal_layer (*macdrv_view_get_metal_layer)(macdrv_metal_view v);
    void (*macdrv_view_release_metal_view)(macdrv_metal_view v);
    void (*on_main_thread)(dispatch_block_t b);
};

// --- iOS-side state: one layer shared by the whole process ---

static CAMetalLayer *g_layer = nil;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static MadeiraGPUForegroundGate gpu_gate = MADEIRA_GPU_FOREGROUND_INITIALIZER;
static MadeiraGPUFrameDiagnostics gpu_metrics = MADEIRA_GPU_DIAGNOSTICS_INITIALIZER;

static void madeira_gpu_install_lifecycle(void) {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
        [center addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:nil usingBlock:^(NSNotification *note) {
            madeira_gpu_gate_set(&gpu_gate, 0);
        }];
        [center addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:nil usingBlock:^(NSNotification *note) {
            madeira_gpu_gate_set(&gpu_gate, 1);
        }];
        // MetalHostView registers its layer on main, before starting Wine.
        if ([NSThread isMainThread]) {
            madeira_gpu_gate_set(&gpu_gate, UIApplication.sharedApplication.applicationState == UIApplicationStateActive);
        }
        fprintf(stderr, "[gpu-foreground-v1] native DXMT submission gate registered\n");
    });
}
void madeira_gpu_submit_begin(void) { madeira_gpu_gate_begin(&gpu_gate); }
void madeira_gpu_submit_end(void) { madeira_gpu_gate_end(&gpu_gate); }
void madeira_gpu_diagnostics_enable(int enabled) { madeira_gpu_metrics_enable(&gpu_metrics, enabled); }
unsigned long long madeira_gpu_diagnostics_begin(void) { return madeira_gpu_metrics_begin(&gpu_metrics); }
void madeira_gpu_diagnostics_complete(unsigned long long ticket, double start, double end, int error) {
    madeira_gpu_metrics_complete(&gpu_metrics, ticket, start, end, error);
}
MadeiraGPUStats madeira_gpu_diagnostics_sample(void) {
    MadeiraGPUFrameSample sample = madeira_gpu_metrics_take(&gpu_metrics);
    return (MadeiraGPUStats){sample.buffers, sample.timed_buffers, sample.gpu_ns, sample.max_gpu_ns,
                            sample.inflight, sample.max_inflight, sample.errors,
                            sample.union_gpu_ns, sample.span_gpu_ns, sample.timing_dropped};
}

void madeira_display_set_layer(CAMetalLayer *layer) {
    madeira_gpu_install_lifecycle();
    pthread_mutex_lock(&g_lock);
    g_layer = layer;
    pthread_mutex_unlock(&g_lock);
}

// --- The guest's virtual monitor ---------------------------------------
//
// The front end lays out the presented layer and maps touches in guest
// pixels, so it needs the monitor size win32u uses. win32u reads
// MADEIRA_SCREEN_W/H once per session (sysparams_ios.c, ios_screen_size), so
// that is the seed; sysparams_ios.c calls winios_display_mode_changed() when
// a program changes the display mode (ios_publish_screen_size), and the
// front end calls it when it chooses a session's size.

static int g_screen_w, g_screen_h;   // 0 until something publishes a size
static pthread_mutex_t g_screen_lock = PTHREAD_MUTEX_INITIALIZER;

NSString * const MadeiraDisplayModeChangedNotification = @"MadeiraDisplayModeChanged";

void winios_screen_size(int *w, int *h) {
    pthread_mutex_lock(&g_screen_lock);
    int sw = g_screen_w, sh = g_screen_h;
    pthread_mutex_unlock(&g_screen_lock);
    if (sw <= 0 || sh <= 0) {
        // Nothing published: the session default, read each time because the
        // app sets it per launch.
        const char *we = getenv("MADEIRA_SCREEN_W"), *he = getenv("MADEIRA_SCREEN_H");
        sw = (we && atoi(we) > 0) ? atoi(we) : 1024;
        sh = (he && atoi(he) > 0) ? atoi(he) : 768;
    }
    if (w) *w = sw;
    if (h) *h = sh;
}

void winios_display_mode_changed(int w, int h) {
    if (w <= 0 || h <= 0) return;
    pthread_mutex_lock(&g_screen_lock);
    int changed = (g_screen_w != w || g_screen_h != h);
    g_screen_w = w;
    g_screen_h = h;
    pthread_mutex_unlock(&g_screen_lock);
    if (!changed) return;
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:MadeiraDisplayModeChangedNotification object:nil];
    });
}

// --- DXMT display-rate hook -----------------------------------------------
//
// DXMT from willfaust/dxmt#1 on defines madeira_set_display_max_fps() (the
// panel and intent rates for its [frame] line) together with the 30 FPS cap
// (vsync mode 3). main's DXMT has neither. This weak no-op definition keeps
// the app linking against either: when DXMT's strong definition is linked it
// replaces this one, and this one never runs. When it does run, DXMT lacks the
// 30 FPS cap too (mode 3 would present uncapped), so the front end hides it.

static volatile int g_dxmt_display_pacing_missing;

__attribute__((weak)) void madeira_set_display_max_fps(int panel_hz, int intent_hz) {
    (void)panel_hz;
    (void)intent_hz;
    g_dxmt_display_pacing_missing = 1;
}

int madeira_dxmt_has_display_pacing(void) {
    return !g_dxmt_display_pacing_missing;
}

// --- macdrv_* implementations ---

// DXMT only dereferences client_cocoa_view (passing it straight back to
// macdrv_view_create_metal_view), so we use that field to carry the HWND
// through: desktop mode needs it to pick the right window's layer. One
// static struct suffices — DXMT's get/create/release sequence is not
// concurrent per-process, and the value is consumed before release.
static struct macdrv_win_data g_fake_win_data = {
    .hwnd              = NULL,
    .cocoa_window      = NULL,
    .cocoa_view        = (macdrv_view)(uintptr_t)0x1,
    .client_cocoa_view = (macdrv_view)(uintptr_t)0x1,
};

static struct macdrv_win_data *my_get_win_data(HWND hwnd) {
    // Called from DXMT's Wine thread, which owns a valid TEB. Query here,
    // never on UIKit's main thread. Desktop input keeps the compositor path.
    const char *desk = getenv("MADEIRA_DESKTOP");
    if (!(desk && *desk == '1')) {
        pthread_mutex_lock(&g_input_surface_lock);
        madeira_input_surface_select(&g_input_surface, (uintptr_t)hwnd);
        pthread_mutex_unlock(&g_input_surface_lock);
        winios_drv_publish_input_rect(hwnd);
    }
    g_fake_win_data.hwnd = hwnd;
    g_fake_win_data.client_cocoa_view = (macdrv_view)hwnd;
    return &g_fake_win_data;
}

static void my_release_win_data(struct macdrv_win_data *data) {
    (void)data;
}

static int madeira_desktop_mode(void) {
    static int desk = -1;
    if (desk < 0) {
        const char *d = getenv("MADEIRA_DESKTOP");
        desk = d && *d == '1';
    }
    return desk;
}

// Winios.m compositor: per-window CAMetalLayer inside the window's
// compositor layer (desktop mode only).
extern CAMetalLayer *winios_metal_layer_for_hwnd(void *hwnd);

static macdrv_metal_device my_create_metal_device(void) {
    // DXMT also has a separate code path that creates its own MTLDevice;
    // this is only called by a Wine-flavoured API we don't hit. Return a
    // non-null sentinel so the caller doesn't think it's a failure.
    return (macdrv_metal_device)(uintptr_t)0x1;
}

static void my_release_metal_device(macdrv_metal_device d) {
    (void)d;
}

// The critical two: return a "view" handle that maps to the CAMetalLayer.
CAMetalLayer *madeira_display_layer_for_hwnd(void *hwnd) {
    if (madeira_desktop_mode()) return winios_metal_layer_for_hwnd(hwnd);
    pthread_mutex_lock(&g_lock);
    CAMetalLayer *layer = g_layer;
    pthread_mutex_unlock(&g_lock);
    return layer;
}

// We pack the layer pointer directly. `v` carries the swapchain's HWND
// (see my_get_win_data). Desktop mode: per-window layer in the desktop
// compositor. Game mode: the fullscreen singleton, exactly as before.
static macdrv_metal_view my_view_create_metal_view(macdrv_view v, macdrv_metal_device d) {
    (void)d;
    if (madeira_desktop_mode()) {
        CAMetalLayer *layer = winios_metal_layer_for_hwnd((void *)v);
        if (!layer) {
            NSLog(@"[madeira-display] desktop metal layer creation failed for hwnd=%p", (void *)v);
            return NULL;
        }
        fprintf(stderr, "[madeira-display] desktop metal view for hwnd=%p layer=%p\n", (void *)v, layer);
        fflush(stderr);
        return (macdrv_metal_view)CFBridgingRetain(layer);
    }
    pthread_mutex_lock(&g_lock);
    CAMetalLayer *layer = g_layer;
    pthread_mutex_unlock(&g_lock);
    if (!layer) {
        NSLog(@"[madeira-display] view_create_metal_view called before layer registered!");
        return NULL;
    }
    return (macdrv_metal_view)CFBridgingRetain(layer);
}

static macdrv_metal_layer my_view_get_metal_layer(macdrv_metal_view v) {
    return (macdrv_metal_layer)v;
}

static void my_view_release_metal_view(macdrv_metal_view v) {
    if (v) CFBridgingRelease((CFTypeRef)v);
}

static void my_on_main_thread(dispatch_block_t b) {
    if ([NSThread isMainThread]) b();
    else dispatch_async(dispatch_get_main_queue(), b);
}

// --- Exported symbols (dlsym RTLD_DEFAULT finds these in the main binary) ---
//
// `used` is LOAD-BEARING, not decoration. Nothing in this program references
// these by name: DXMT's winemetal_unix.c reaches them only through
// dlsym(RTLD_DEFAULT, "macdrv_functions"), which the linker cannot see. Under a
// Release link (-dead_strip) they are therefore unreferenced and get removed,
// visibility("default") notwithstanding -- visibility governs whether a symbol
// that SURVIVES is exported, not whether it survives. `used` emits .no_dead_strip
// so the linker keeps them.
//
// That is exactly how Thumper broke on 2026-08-08: the host app was rebuilt
// Release instead of Debug, these six symbols vanished from the binary, DXMT's
// lookup returned NULL and d3d11_swapchain aborted with "your Wine has no
// exported symbols needed by DXMT" -> exit code 3 -> white screen.
//
// `used` alone proved sufficient: after adding it, all six land in the export
// trie of a Release build, so no -u roots or -export_dynamic are needed. Do not
// assume that stays true -- VERIFY BY CONTENT after any build or link change:
//   xcrun dyld_info -exports Madeira.app/Madeira | grep macdrv_functions
// If that prints nothing, Thumper will exit 3 with a white screen.

__attribute__((used, visibility("default")))
struct macdrv_functions_t macdrv_functions = {
    .macdrv_init_display_devices    = NULL,
    .get_win_data                   = my_get_win_data,
    .release_win_data               = my_release_win_data,
    .macdrv_get_cocoa_window        = NULL,
    .macdrv_create_metal_device     = my_create_metal_device,
    .macdrv_release_metal_device    = my_release_metal_device,
    .macdrv_view_create_metal_view  = my_view_create_metal_view,
    .macdrv_view_get_metal_layer    = my_view_get_metal_layer,
    .macdrv_view_release_metal_view = my_view_release_metal_view,
    .on_main_thread                 = my_on_main_thread,
};

// Also export individual symbols as a fallback (DXMT checks both paths).
__attribute__((used, visibility("default")))
struct macdrv_win_data *get_win_data(HWND hwnd) { return my_get_win_data(hwnd); }

__attribute__((used, visibility("default")))
void release_win_data(struct macdrv_win_data *data) { my_release_win_data(data); }

__attribute__((used, visibility("default")))
macdrv_metal_view macdrv_view_create_metal_view(macdrv_view v, macdrv_metal_device d) {
    return my_view_create_metal_view(v, d);
}

__attribute__((used, visibility("default")))
macdrv_metal_layer macdrv_view_get_metal_layer(macdrv_metal_view v) {
    return my_view_get_metal_layer(v);
}

__attribute__((used, visibility("default")))
void macdrv_view_release_metal_view(macdrv_metal_view v) {
    my_view_release_metal_view(v);
}
