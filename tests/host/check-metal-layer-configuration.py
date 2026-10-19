#!/usr/bin/env python3
"""Reproduce the old Swift scalar ABI bug and execute the fix on UIKit/Metal."""
from pathlib import Path
import json
import os
import plistlib
import shutil
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parents[2]
if sys.platform != 'darwin':
    if '--require-darwin' in sys.argv:
        raise SystemExit('Xcode and iPad Simulator required')
    print('SKIP: Objective-C scalar ABI verification requires Apple runtime')
    raise SystemExit(0)

bridge = r'''
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
NSObject *make_scalar_probe(void);
NSDictionary *scalar_snapshot(NSObject *object);
int configure_scalars(NSObject *object, int hz);
void exercise_signature_guards(void);
'''
objc = r'''
#import "Bridge.h"
#import <Metal/Metal.h>
#include <assert.h>
#include "MetalLayerConfiguration.h"
@interface ScalarProbe : NSObject
@property BOOL displaySyncEnabled;
@property NSInteger nominalFramesPerSecond;
@end
@implementation ScalarProbe
@end
@interface IntProbe : NSObject
@property BOOL displaySyncEnabled;
@property int nominalFramesPerSecond;
@end
@implementation IntProbe
@end
@interface ObjectProbe : NSObject
@property(strong) id displaySyncEnabled;
@property(strong) id nominalFramesPerSecond;
@end
@implementation ObjectProbe
@end
NSObject *make_scalar_probe(void) { return [ScalarProbe new]; }
NSDictionary *scalar_snapshot(NSObject *object) {
    NSMutableDictionary *result = [NSMutableDictionary new];
    for (NSString *name in @[@"displaySyncEnabled", @"nominalFramesPerSecond"]) {
        SEL selector = NSSelectorFromString(name);
        if ([object respondsToSelector:selector]) result[name] = [object valueForKey:name];
    }
    return result;
}
int configure_scalars(NSObject *object, int hz) {
    return madeira_layer_set_sync(object, NO) | (madeira_layer_set_nominal_rate(object, hz) << 1);
}
void exercise_signature_guards(void) {
    ObjectProbe *object = [ObjectProbe new];
    assert(!configure_scalars(object, 60));
    assert(!object.displaySyncEnabled && !object.nominalFramesPerSecond);
    assert(!configure_scalars([NSObject new], 60));
    assert(!configure_scalars(nil, 60));
    ScalarProbe *wide = [ScalarProbe new];
    assert(madeira_layer_set_sync(wide, YES) && wide.displaySyncEnabled);
    assert(madeira_layer_set_sync(wide, NO) && !wide.displaySyncEnabled);
    for (NSNumber *hz in @[@30, @60, @120]) {
        assert(madeira_layer_set_nominal_rate(wide, hz.integerValue));
        assert(wide.nominalFramesPerSecond == hz.integerValue);
    }
    assert(!madeira_layer_set_nominal_rate(wide, -1));
    assert(!madeira_layer_set_nominal_rate(wide, 0));
    assert(!madeira_layer_set_nominal_rate(wide, 1001));
    IntProbe *narrow = [IntProbe new];
    assert(configure_scalars(narrow, 120) == 3 && narrow.nominalFramesPerSecond == 120);
}
'''
swift = r'''
import Foundation
import UIKit
import Metal
import QuartzCore
@MainActor final class LiquidMetalSetting {
    static let shared = LiquidMetalSetting()
    var on = true
}
@MainActor final class LogStore {
    static let shared = LogStore()
    var messages: [String] = []
    func log(_ text: String) { messages.append(text); print(text) }
}
@main @MainActor final class Fixture: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    func application(_ app: UIApplication, didFinishLaunchingWithOptions options: [UIApplication.LaunchOptionsKey: Any]? = nil) -> Bool {
        window = UIWindow(frame: UIScreen.main.bounds)
        let root = UIViewController()
        let search = UISearchBar(frame: CGRect(x: 0, y: 0, width: 400, height: 44))
        search.placeholder = "Library"
        root.navigationItem.titleView = search
        window!.rootViewController = UINavigationController(rootViewController: root)
        window!.makeKeyAndVisible()
        DispatchQueue.main.async { self.exercise() }
        return true
    }
    func exercise() {
        let probe = make_scalar_probe()!
        // Execute the exact Swift calling convention used in MetalHostView.
        probe.perform(NSSelectorFromString("setDisplaySyncEnabled:"), with: NSNumber(value: false))
        probe.perform(NSSelectorFromString("setNominalFramesPerSecond:"), with: 120 as NSNumber)
        let old = scalar_snapshot(probe)!
        precondition((old["nominalFramesPerSecond"] as! NSNumber).int64Value != 120,
                     "Old ABI bug did not reproduce; inspect before claiming a fix")
        precondition(configure_scalars(probe, 120) == 3)
        let fixed = scalar_snapshot(probe)!
        precondition((fixed["nominalFramesPerSecond"] as! NSNumber).intValue == 120)
        precondition(!(fixed["displaySyncEnabled"] as! NSNumber).boolValue)
        exercise_signature_guards()
        // Real iPad Simulator layer, including the actual private signatures.
        let layer = CAMetalLayer()
        layer.device = MTLCreateSystemDefaultDevice()
        let applied = configure_scalars(layer, 120)
        let actual = scalar_snapshot(layer)!
        if (applied & 1) != 0, let value = actual["displaySyncEnabled"] as? NSNumber {
            precondition(!value.boolValue)
        }
        if (applied & 2) != 0, let value = actual["nominalFramesPerSecond"] as? NSNumber {
            precondition(value.intValue == 120)
        }
        var result: [String: Any] = ["oldSwiftArguments": old, "fixedArguments": fixed,
            "realLayerAppliedMask": applied, "realLayerReadback": actual,
            "integer32And64Verified": true, "changedOrMissingSignaturesSkipped": true,
            "physicalIPadGameFpsMeasured": false]
        GlassSkin.shared.start()
        Task { @MainActor in
            try! await Task.sleep(nanoseconds: 600_000_000)
            let baseline = GlassSkin.shared.renderedFrames
            guard self.require(baseline > 0, "Production search skin did not submit Metal frames: " + LogStore.shared.messages.joined(separator: "; ")) else { return }
            // Occlusion leaves the library's UIKit views attached. While a
            // game owns the window, even a late onAppear/start must stay idle.
            GlassSkin.sessionChanged(active: true)
            let paused = GlassSkin.shared.renderedFrames
            GlassSkin.shared.start(fadeIn: true)
            try! await Task.sleep(nanoseconds: 400_000_000)
            guard self.require(GlassSkin.shared.renderedFrames == paused, "Hidden library still submits GPU work"),
                  self.require(LiquidMetalSetting.shared.on, "Game changed the saved appearance setting") else { return }
            GlassSkin.sessionChanged(active: false)
            try! await Task.sleep(nanoseconds: 400_000_000)
            guard self.require(GlassSkin.shared.renderedFrames > paused, "Returning to Library did not resume its renderer") else { return }
            GlassSkin.shared.stop()
            result["hiddenLibraryMetalSubmissionsStopped"] = true
            result["libraryAppearanceResumesAfterGame"] = true
            result["actualSkinFramesBeforePause"] = baseline
            let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
            try! JSONSerialization.data(withJSONObject: result, options: [.prettyPrinted, .sortedKeys])
                .write(to: docs.appendingPathComponent("result.json"))
        }
    }
    func require(_ value: Bool, _ message: String) -> Bool {
        if !value {
            let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
            try! Data(message.utf8).write(to: docs.appendingPathComponent("failure.txt"))
        }
        return value
    }
}
'''


def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


with tempfile.TemporaryDirectory(prefix='madeira-layer-scalars-') as scratch:
    folder = Path(scratch)
    app = folder / 'LayerScalars.app'
    app.mkdir()
    (folder / 'Bridge.h').write_text(bridge)
    (folder / 'Bridge.m').write_text(objc)
    (folder / 'Fixture.swift').write_text(swift)
    sdk = subprocess.check_output(['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-path'], text=True).strip()
    target = 'arm64-apple-ios18.0-simulator'
    run('xcrun', '--sdk', 'iphonesimulator', 'clang', '-fobjc-arc', '-target', target, '-isysroot', sdk,
        '-I' + str(root / 'app/Madeira'), '-c', str(folder / 'Bridge.m'), '-o', str(folder / 'Bridge.o'))
    run('xcrun', '--sdk', 'iphonesimulator', 'swiftc', '-swift-version', '5', '-parse-as-library',
        '-target', target, '-sdk', sdk, '-import-objc-header', str(folder / 'Bridge.h'),
        str(folder / 'Fixture.swift'), str(root / 'app/Madeira/GlassSkin.swift'),
        str(folder / 'Bridge.o'), '-o', str(app / 'LayerScalars'))
    run('xcrun', '--sdk', 'iphonesimulator', 'metal', '-c', str(root / 'app/Madeira/LiquidMetalSkin.metal'),
        '-target', 'air64-apple-ios18.0-simulator', '-isysroot', sdk, '-o', str(folder / 'Skin.air'))
    run('xcrun', '--sdk', 'iphonesimulator', 'metallib', str(folder / 'Skin.air'), '-o', str(app / 'default.metallib'))
    bundle = 'org.madeira.layer-scalars-fixture'
    with (app / 'Info.plist').open('wb') as output:
        plistlib.dump(dict(CFBundleIdentifier=bundle, CFBundleExecutable='LayerScalars', CFBundleName='Layer Scalars',
                          CFBundleVersion='1', CFBundleShortVersionString='1.0', CFBundlePackageType='APPL',
                          UIDeviceFamily=[2], LSRequiresIPhoneOS=True, UILaunchScreen={}), output)
    run('codesign', '--force', '--sign', '-', str(app))
    devices = json.loads(subprocess.check_output(['xcrun', 'simctl', 'list', 'devices', 'available', '--json']))
    ipads = [d for group in devices['devices'].values() for d in group if d.get('isAvailable') and 'iPad' in d['name']]
    assert ipads, 'iPad Simulator runtime required'
    ipads.sort(key=lambda d: ('13-inch' not in d['name'], 'Pro' not in d['name']))
    device = ipads[0]['udid']
    was_booted = ipads[0].get('state') == 'Booted'
    subprocess.run(['xcrun', 'simctl', 'boot', device], check=False, capture_output=True)
    run('xcrun', 'simctl', 'bootstatus', device, '-b', timeout=180)
    try:
        run('xcrun', 'simctl', 'install', device, str(app))
        documents = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, bundle, 'data'], text=True).strip()) / 'Documents'
        result_file = documents / 'result.json'
        result_file.unlink(missing_ok=True)
        failure_file = documents / 'failure.txt'
        failure_file.unlink(missing_ok=True)
        console = subprocess.Popen(['xcrun', 'simctl', 'launch', '--console', device, bundle], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for _ in range(30):
            if result_file.exists() or failure_file.exists() or console.poll() is not None: break
            time.sleep(1)
        if failure_file.exists(): raise AssertionError(failure_file.read_text())
        if not result_file.exists():
            subprocess.run(['xcrun', 'simctl', 'terminate', device, bundle], capture_output=True)
            try: print(console.communicate(timeout=10)[0][-6000:])
            except subprocess.TimeoutExpired: console.kill()
        assert result_file.exists(), 'Scalar fixture failed or timed out; inspect Simulator crash report'
        result = json.loads(result_file.read_text())
        output = Path(os.environ.get('BITRISE_DEPLOY_DIR', folder))
        output.mkdir(exist_ok=True, parents=True)
        shutil.copy2(result_file, output / 'metal-layer-scalars-validation.json')
        print('PASS: old Swift NSNumber-to-scalar ABI bug reproduced; typed production setters, 30/60/120, guards and real UIKit layer verified')
        print('PASS: actual Liquid Metal search draws on UIKit; game stops its GPU submissions despite attached views and late start; Library resumes without changing appearance preference')
        print(json.dumps(result, sort_keys=True))
    finally:
        subprocess.run(['xcrun', 'simctl', 'terminate', device, bundle], check=False, capture_output=True)
        if 'console' in locals():
            try: console.communicate(timeout=10)
            except subprocess.TimeoutExpired: console.kill(); console.communicate()
        if not was_booted: run('xcrun', 'simctl', 'shutdown', device)
