#!/usr/bin/env python3
"""Exercise production profile repair and focused game window coordinates."""
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
bridge = (root / 'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
windows = (root / 'app/Madeira/Winios/Winios.m').read_text(encoding='utf-8')
dock = (root / 'app/Madeira/DockStartScreen.swift').read_text(encoding='utf-8')
header = (root / 'app/Madeira/Winios/Winios.h').read_text(encoding='utf-8')


def function(source, name):
    start = source.index(name)
    # Names supplied include the return type and definition signature.
    begin = source.index('{', start)
    depth = 1
    i = begin + 1
    while depth:
        depth += (source[i] == '{') - (source[i] == '}')
        i += 1
    return source[start:i]


profile = function(bridge, 'static void madeira_ensure_profile_folders(NSString *prefix)')
assert 'madeira_ensure_profile_folders( prefix );' in bridge
assert '@"Documents"' in profile and '@"AppData/LocalLow"' in profile
assert 'void winios_set_game_window(unsigned long long hwnd);' in header
show = dock.split('case .showGame:', 1)[1].split('case .reveal:', 1)[0]
assert show.index('winios_set_game_window(window.hwnd)') < show.index('endHold(reason: "game-window")')
assert dock.count('winios_set_game_window(0)') == 2
assert 'if ([g_game_window_key isEqual:key]) g_game_window_key = nil;' in windows
print('PASS: profile repair is wired before Wine; only the selected game window is fitted and session transitions reset it')
if sys.platform != 'darwin':
    if '--require-objc' in sys.argv:
        raise SystemExit('Foundation/QuartzCore tests require macOS')
    print('SKIP: production filesystem and window geometry tests run on macOS CI')
    raise SystemExit(0)

profile_prelude = r'''
#import <Foundation/Foundation.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
void ws_log(const char *format, ...) { (void)format; }
'''
profile_checks = r'''
int main(int argc, char **argv) { @autoreleasepool {
    assert(argc == 2);
    NSString *prefix = [NSString stringWithUTF8String:argv[1]];
    setenv("USER", "mobile", 1);
    unsetenv("MADEIRA_PROFILE_LOCALLOW");
    NSFileManager *fm = NSFileManager.defaultManager;
    NSString *documents = [prefix stringByAppendingPathComponent:@"drive_c/users/mobile/Documents"];
    NSString *low = [prefix stringByAppendingPathComponent:@"drive_c/users/mobile/AppData/LocalLow"];
    NSString *klei = [documents stringByAppendingPathComponent:@"Klei"];
    assert(![fm createDirectoryAtPath:klei withIntermediateDirectories:NO attributes:nil error:nil]);
    madeira_ensure_profile_folders(prefix);
    assert([fm fileExistsAtPath:documents] && [fm fileExistsAtPath:low]);
    assert([fm createDirectoryAtPath:klei withIntermediateDirectories:NO attributes:nil error:nil]);
    NSString *dst = [klei stringByAppendingPathComponent:@"DoNotStarveTogether"];
    assert([fm createDirectoryAtPath:dst withIntermediateDirectories:NO attributes:nil error:nil]);
    NSString *save = [dst stringByAppendingPathComponent:@"settings.ini"];
    assert([@"existing game settings" writeToFile:save atomically:YES encoding:NSUTF8StringEncoding error:nil]);
    madeira_ensure_profile_folders(prefix);
    assert([[NSString stringWithContentsOfFile:save encoding:NSUTF8StringEncoding error:nil] isEqual:@"existing game settings"]);
    assert(![fm fileExistsAtPath:[prefix stringByAppendingPathComponent:@"drive_c/users/mobile/AppData/Roaming"]]);
    setenv("USER", "second", 1); setenv("MADEIRA_PROFILE_LOCALLOW", "0", 1);
    madeira_ensure_profile_folders(prefix);
    assert([fm fileExistsAtPath:[prefix stringByAppendingPathComponent:@"drive_c/users/second/Documents"]]);
    assert(![fm fileExistsAtPath:[prefix stringByAppendingPathComponent:@"drive_c/users/second/AppData/LocalLow"]]);
    setenv("USER", "collision", 1);
    NSString *collision = [prefix stringByAppendingPathComponent:@"drive_c/users/collision"];
    assert([fm createDirectoryAtPath:collision withIntermediateDirectories:YES attributes:nil error:nil]);
    NSString *file = [collision stringByAppendingPathComponent:@"Documents"];
    assert([@"do not replace" writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:nil]);
    madeira_ensure_profile_folders(prefix);
    assert([[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] isEqual:@"do not replace"]);
    puts("PASS: actual Documents repair permits Klei's sequential directory creation and preserves saves, collisions and LocalLow opt-out");
} return 0; }
'''

geometry_prelude = r'''
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
@interface TestView : NSObject
@property CGRect frame;
@property(strong) CALayer *layer;
@end
@implementation TestView
@end
static TestView *g_compositor_view;
static NSNumber *g_fit_key, *g_game_window_key;
static CGRect g_fit_client_px, g_fit_view_pt, g_desk_rect;
static CALayer *g_desk_bg;
static BOOL g_desk_rect_set;
static CGFloat g_px_to_pt, g_px_to_pt_y;
static CGPoint g_desk_origin;
#define WINIOS_PX_TO_PT_Y (g_px_to_pt_y > 0 ? g_px_to_pt_y : g_px_to_pt)
void winios_screen_size(int *w, int *h) { *w = 2560; *h = 1440; }
'''
geometry = '\n'.join(function(windows, name) for name in [
    'static int winios_desktop_fit_enabled(void)',
    'static BOOL winios_desktop_fit(NSNumber *key, CAMetalLayer *ml, CGRect c)',
    'static BOOL winios_desktop_fit_map(CGFloat x, CGFloat y, CGPoint *pt, CGFloat *scale)',
    'int winios_desktop_point_from_window(double wx, double wy, int *px, int *py)',
])
geometry_checks = r'''
int main(void) { @autoreleasepool {
    unsetenv("MADEIRA_DESKTOP_FIT");
    g_compositor_view = [TestView new];
    g_compositor_view.frame = CGRectMake(0, 0, 1590, 1192);
    g_compositor_view.layer = [CALayer layer];
    g_compositor_view.layer.frame = g_compositor_view.frame;
    g_desk_rect_set = YES;
    g_px_to_pt = 1590.0 / 2560;
    g_desk_origin = CGPointMake(0, (1192 - 1440*g_px_to_pt)/2);
    g_desk_rect = CGRectMake(0, g_desk_origin.y, 1590, 1440*g_px_to_pt);
    g_desk_bg = [CALayer layer]; g_desk_bg.frame = g_desk_rect;
    NSNumber *game = @0x2007e;
    CALayer *window = [CALayer layer]; window.frame = CGRectMake(0, g_desk_origin.y, 800, 469);
    [g_compositor_view.layer addSublayer:window];
    CAMetalLayer *metal = [CAMetalLayer layer]; [window addSublayer:metal];
    CGRect client = CGRectMake(4, 30, 1280, 720);
    assert(!winios_desktop_fit(game, metal, client)); // A normal desktop window remains windowed.
    g_game_window_key = game;
    assert(winios_desktop_fit(game, metal, client));
    CGRect shown = [g_compositor_view.layer convertRect:metal.frame fromLayer:window];
    assert(CGRectEqualToRect(shown, g_desk_rect)); // DST fills the monitor instead of half of it.
    for (int stretched = 0; stretched < 2; ++stretched) {
        if (stretched) {
            g_desk_rect = g_compositor_view.frame;
            g_px_to_pt_y = 1192.0 / 1440;
            assert(winios_desktop_fit(game, metal, client));
        }
        for (int n = 1; n < 4; ++n) {
            CGPoint guest = CGPointMake(4 + 1280*n/4.0, 30 + 720*n/4.0), view;
            assert(winios_desktop_fit_map(guest.x, guest.y, &view, NULL));
            int x=0, y=0;
            assert(winios_desktop_point_from_window(view.x, view.y, &x, &y));
            assert(abs(x-(int)guest.x) <= 1 && abs(y-(int)guest.y) <= 1);
        }
    }
    assert(!winios_desktop_fit(@0x20092, metal, client)); // The dummy/other window is not enlarged.
    assert(!winios_desktop_fit(@0x20092, metal, CGRectMake(0, 0, 3840, 2160)) && [g_fit_key isEqual:game]);
    g_game_window_key = nil;
    assert(!winios_desktop_fit(game, metal, client) && !g_fit_key);
    g_game_window_key = game; window.hidden = YES;
    assert(!winios_desktop_fit(game, metal, client));
    puts("PASS: production focused window layout fills the monitor; Fit/Stretch cursor and touch transforms round-trip; desktop, other and hidden windows remain separate");
} return 0; }
'''
with tempfile.TemporaryDirectory(prefix='madeira-profile-display-') as temp:
    temp = Path(temp)
    for name, code, frameworks, args in [
        ('profile', profile_prelude + profile + profile_checks, ['Foundation'], [str(temp / 'prefix')]),
        ('geometry', geometry_prelude + geometry + geometry_checks, ['Foundation', 'QuartzCore'], []),
    ]:
        source, binary = temp / f'{name}.m', temp / name
        source.write_text(code, encoding='utf-8')
        flags = [flag for framework in frameworks for flag in ['-framework', framework]]
        subprocess.run(['xcrun', 'clang', '-fobjc-arc', '-fblocks', str(source), *flags, '-o', str(binary)], check=True)
        subprocess.run([str(binary), *args], check=True)
