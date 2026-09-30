#import "Platform.h"
#import "ResolutionSettings.h"
#include "ResolutionLaunch.h"
#include <stdint.h>
#import "TouchControls.h"

extern long write(int, const void *, unsigned long);
extern void madeira_resolution_trace(NSString *message);

int madeira_resolution_parse_even(const char *text, int minimum, int maximum) {
    return madeira_resolution_launch_parse_even(text, minimum, maximum);
}

int madeira_resolution_saved(int *width, int *height, int *match) {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    if ([defaults boolForKey:MADEIRA_RESOLUTION_NATIVE_KEY]) {
        if (!madeira_resolution_native(width, height)) return 0;
        if (match) *match = 0;
        return 1;
    }
    int w = (int)[defaults integerForKey:MADEIRA_RESOLUTION_WIDTH_KEY];
    int h = (int)[defaults integerForKey:MADEIRA_RESOLUTION_HEIGHT_KEY];
    if (!madeira_resolution_launch_custom_size_valid(w, h)) return 0;
    if (width) *width = w;
    if (height) *height = h;
    if (match) *match = [defaults boolForKey:MADEIRA_RESOLUTION_MATCH_KEY] ? 1 : 0;
    return 1;
}

int madeira_resolution_native(int *width, int *height) {
    CGSize size = [UIScreen mainScreen].nativeBounds.size;
    int w = (int)(size.width > size.height ? size.width : size.height);
    int h = (int)(size.width < size.height ? size.width : size.height);
    if (!madeira_resolution_launch_native_size_valid(w, h)) return 0;
    if (width) *width = w;
    if (height) *height = h;
    return 1;
}

int madeira_resolution_is_native(void) {
    return [[NSUserDefaults standardUserDefaults] boolForKey:MADEIRA_RESOLUTION_NATIVE_KEY] ? 1 : 0;
}

int madeira_resolution_match_height(int width) {
    CGSize size = [UIScreen mainScreen].nativeBounds.size;
    double wide = size.width > size.height ? size.width : size.height;
    double tall = size.width < size.height ? size.width : size.height;
    if (wide <= 0 || tall <= 0 || width < 640 || width > 4096) return 0;
    int height = (int)((double)width * tall / wide + 0.5);
    height &= ~1;
    return height >= 360 && height <= 4096 ? height : 0;
}

int madeira_resolution_effective(int *width, int *height) {
    int w, h, match;
    if (!madeira_resolution_saved(&w, &h, &match)) return 0;
    if (match && !madeira_resolution_is_native()) {
        h = madeira_resolution_match_height(w);
        if (!h) return 0;
    }
    if (width) *width = w;
    if (height) *height = h;
    return 1;
}

static int same(const char *a, const char *b) {
    return madeira_resolution_launch_same(a, b);
}

// SwiftUI sets MADEIRA_ARGS, then SCREEN_W, then SCREEN_H on the same thread.
// Keep the override scoped to the exact R6 Wine Virtual Desktop launch.
static __thread int activeWidth, activeHeight;

int madeira_resolution_setenv(const char *name, const char *value, int overwrite) {
    if (same(name, "MADEIRA_ARGS")) {
        activeWidth = activeHeight = 0;
        int w = 0, h = 0;
        int effective = madeira_resolution_effective(&w, &h);
        const char *suffix = effective ?
            madeira_resolution_launch_override_suffix(name, value, overwrite, w, h) : 0;
        madeira_resolution_trace([NSString stringWithFormat:
            @"[resolution-setter v61] guest launch wrapper: overwrite=%d match=%d effective=%d native=%d size=%dx%d args=%s",
            overwrite, madeira_resolution_launch_suffix(value) != 0, effective,
            madeira_resolution_is_native(), w, h, value ? value : "(null)"]);
        if (suffix) {
            NSString *tail = [NSString stringWithUTF8String:suffix];
            NSString *replacement = tail ? [[NSString stringWithFormat:@"/desktop=shell,%dx%d", w, h]
                stringByAppendingString:tail] : nil;
            if (replacement) {
                int result = setenv(name, replacement.UTF8String, overwrite);
                if (!result) {
                    activeWidth = w;
                    activeHeight = h;
                    static const char marker[] = "[resolution-setter] R6 desktop override reached\n";
                    (void)write(2, marker, sizeof(marker) - 1);
                    madeira_resolution_trace([NSString stringWithFormat:
                        @"[resolution-setter v61] applied %dx%d environment=%s", w, h, getenv(name)]);
                }
                return result;
            }
        }
        return setenv(name, value, overwrite);
    }
    if (activeWidth && same(name, "MADEIRA_SCREEN_W") && same(value, "1280")) {
        NSString *number = [NSString stringWithFormat:@"%d", activeWidth];
        int result = setenv(name, number.UTF8String, overwrite);
        madeira_resolution_trace([NSString stringWithFormat:
            @"[resolution-setter v61] screen width result=%d environment=%s", result, getenv(name)]);
        return result;
    }
    if (activeHeight && same(name, "MADEIRA_SCREEN_H") && same(value, "720")) {
        NSString *number = [NSString stringWithFormat:@"%d", activeHeight];
        int result = setenv(name, number.UTF8String, overwrite);
        madeira_resolution_trace([NSString stringWithFormat:
            @"[resolution-setter v61] screen height result=%d environment=%s", result, getenv(name)]);
        activeWidth = activeHeight = 0;
        return result;
    }
    return setenv(name, value, overwrite);
}

// Only Madeira's main executable binds to this exported alias. The UI library
// still imports the real libc setenv. Avoid global interposition, which can
// depend on host dyld loading behavior and could also affect LiveContainer.
int mdrenv(const char *name, const char *value, int overwrite) {
    // Six guarded control-only branch sites reuse the existing import stub.
    // Small integer arguments distinguish input from genuine libc strings.
    // Native output calls the untouched Wine functions, not this dispatcher.
    if ((uintptr_t)name<=254 && (uintptr_t)value<=1)
        return MadeiraTouchLegacyInput((unsigned)(uintptr_t)name,
            (unsigned)(uintptr_t)value,(unsigned)overwrite);
    return madeira_resolution_setenv(name, value, overwrite);
}
