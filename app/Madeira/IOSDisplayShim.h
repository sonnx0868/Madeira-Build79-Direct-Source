// IOSDisplayShim.h — bridges Wine/DXMT's expected macdrv_* driver API to
// an iOS CAMetalLayer handed in from Swift.
//
// DXMT's winemetal unix side calls dlsym(RTLD_DEFAULT, "macdrv_functions"),
// and if that's present, uses get_win_data → client_cocoa_view →
// macdrv_view_create_metal_view → macdrv_view_get_metal_layer to obtain a
// CAMetalLayer from an HWND. On iOS there's one window (the device screen),
// so the shim resolves every HWND to the single Swift-owned CAMetalLayer.

#ifndef IOS_DISPLAY_SHIM_H
#define IOS_DISPLAY_SHIM_H

#ifdef __OBJC__
#import <QuartzCore/CAMetalLayer.h>
// Register the CAMetalLayer that DXMT-rendered content should go into.
// Must be called before the first D3D11 swapchain is created.
void madeira_display_set_layer(CAMetalLayer *layer);

// Posted (on the main queue) when the guest's virtual monitor changes size.
extern NSString * const MadeiraDisplayModeChangedNotification;
CAMetalLayer *madeira_display_layer_for_hwnd(void *hwnd);
#endif

// The guest's virtual-monitor size in guest pixels, for the front end's
// layout and touch mapping. Seeded from MADEIRA_SCREEN_W/H (the session
// default win32u uses, 1024x768 when unset); winios_display_mode_changed()
// lets a win32u that supports mode changes publish a new size.
void winios_screen_size(int *w, int *h);
void winios_display_mode_changed(int w, int h);

// The client that is actually expanded into the direct game's Metal layer.
// Snapshot only: UIKit never calls Wine window APIs. Coordinates are screen pixels.
void madeira_input_surface_reset(void);
int madeira_input_surface_rect(int *x, int *y, int *width, int *height);
void madeira_input_surface_window_destroyed(unsigned long long hwnd);
void madeira_input_surface_update(unsigned long long hwnd, int x, int y, int width, int height);
int madeira_input_surface_tracks(unsigned long long hwnd);

#endif
