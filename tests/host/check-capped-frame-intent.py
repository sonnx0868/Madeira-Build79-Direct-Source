#!/usr/bin/env python3
"""Execute the production ProMotion controller with real UIKit on iPad Simulator."""
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
source = (root / 'app/Madeira/FPSOverlay.swift').read_text(encoding='utf-8')
controller = source[source.index('final class ProMotionIntent {'):source.index('/// Small overlay shown')]
if sys.platform != 'darwin':
    print('SKIP: native CADisplayLink validation requires Xcode and iPad Simulator')
    raise SystemExit(0)

fixture = r'''
var publishedRates: [(Int32, Int32)] = []
func madeira_set_display_max_fps(_ panel: Int32, _ intent: Int32) { publishedRates.append((panel, intent)) }
func madeira_dxmt_has_display_pacing() -> Int32 { 1 }
@main @MainActor final class Fixture: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    func application(_ app: UIApplication, didFinishLaunchingWithOptions options: [UIApplication.LaunchOptionsKey: Any]? = nil) -> Bool {
        window = UIWindow(frame: UIScreen.main.bounds)
        window!.rootViewController = UIViewController()
        window!.makeKeyAndVisible()
        DispatchQueue.main.async {
            do { try self.exercise() }
            catch { try? Data(String(describing: error).utf8).write(to: self.docs.appendingPathComponent("failure.txt")) }
        }
        return true
    }
    func check(_ ok: @autoclosure () -> Bool, _ message: String) throws {
        if !ok() { throw NSError(domain: "FrameIntentFixture", code: 1, userInfo: [NSLocalizedDescriptionKey: message]) }
    }
    func activeLink() -> CADisplayLink? {
        guard let value = Mirror(reflecting: ProMotionIntent.shared).children.first(where: { $0.label == "link" })?.value else { return nil }
        return Mirror(reflecting: value).children.first?.value as? CADisplayLink
    }
    func range(_ link: CADisplayLink?) -> [Float] {
        guard let value = link?.preferredFrameRateRange, let preferred = value.preferred else { return [] }
        return [value.minimum, value.maximum, preferred]
    }
    func exercise() throws {
        let panel = ProMotionIntent.panelMaxFPS
        try check(panel > 0, "Missing UIKit panel rate")
        ProMotionIntent.configureSession(cappedIntent: false, mode: 1)
        try check(activeLink() == nil && publishedRates.last!.1 == 0, "Changed ordinary capped games")
        ProMotionIntent.configureSession(cappedIntent: true, mode: 1)
        let sixty = range(activeLink()), first = activeLink()
        let want60 = Float(min(60, panel))
        try check(sixty == [want60, want60, want60], "60 FPS mode requested a different display rate")
        try check(publishedRates.last!.1 == Int32(want60), "Native display-intent publication differs from UIKit")
        for _ in 0..<100 { ProMotionIntent.apply(mode: 1) }
        try check(activeLink() === first, "Periodic sampling recreated the display link")
        ProMotionIntent.apply(mode: 3)
        let thirty = range(activeLink()), want30 = Float(min(30, panel))
        try check(thirty == [want30, want30, want30], "Explicit 30 FPS mode lost its selected rate")
        ProMotionIntent.apply(mode: 0)
        let maximum = range(activeLink())
        try check(maximum == [Float(min(60, panel)), Float(panel), Float(panel)], "MAX mode changed")
        ProMotionIntent.configureSession(cappedIntent: false, mode: 1)
        try check(activeLink() == nil, "Next ordinary game inherited the REPLACED hint")
        ProMotionIntent.configureSession(cappedIntent: true, mode: 1)
        ProMotionIntent.resetSession()
        ProMotionIntent.shared.setActive(false)
        try check(activeLink() == nil, "Quit retained a display link")
        ProMotionIntent.apply(mode: 1)
        try check(activeLink() == nil, "Quit retained capped-session state")
        ProMotionIntent.shared.setActive(true, maxHz: panel * 2)
        try check(range(activeLink())[1] == Float(panel), "A request exceeded the physical panel maximum")
        ProMotionIntent.shared.setActive(false)
        let result: [String: Any] = ["panelMaximum": panel, "capped60": sixty, "capped30": thirty,
            "maximumMode": maximum, "repeatedApplyReusesLink": true, "quitReleasesLink": true,
            "otherGamesPreserved": true, "nativePublicationMatchesUIKit": true,
            "physicalIPadFpsOrTemperatureMeasured": false]
        try JSONSerialization.data(withJSONObject: result, options: [.prettyPrinted, .sortedKeys])
            .write(to: docs.appendingPathComponent("result.json"))
    }
}
'''


def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


with tempfile.TemporaryDirectory(prefix='madeira-frame-intent-') as scratch:
    folder = Path(scratch)
    app = folder / 'FrameIntent.app'
    app.mkdir()
    swift = folder / 'Fixture.swift'
    swift.write_text('import Foundation\nimport UIKit\nimport QuartzCore\n' + controller + fixture, encoding='utf-8')
    sdk = subprocess.check_output(['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-path'], text=True).strip()
    run('xcrun', '--sdk', 'iphonesimulator', 'swiftc', '-swift-version', '5', '-parse-as-library',
        '-target', 'arm64-apple-ios18.0-simulator', '-sdk', sdk, str(swift), '-o', str(app / 'FrameIntent'))
    bundle = 'org.madeira.frame-intent-fixture'
    with (app / 'Info.plist').open('wb') as output:
        plistlib.dump(dict(CFBundleIdentifier=bundle, CFBundleExecutable='FrameIntent', CFBundleName='Frame Intent Fixture',
                          CFBundleVersion='1', CFBundleShortVersionString='1.0', CFBundlePackageType='APPL',
                          UIDeviceFamily=[2], LSRequiresIPhoneOS=True, UILaunchScreen={}), output)
    run('codesign', '--force', '--sign', '-', str(app))
    devices = json.loads(subprocess.check_output(['xcrun', 'simctl', 'list', 'devices', 'available', '--json']))
    ipads = [d for group in devices['devices'].values() for d in group if d.get('isAvailable') and 'iPad' in d['name']]
    assert ipads, 'iPad Simulator runtime required'
    ipads.sort(key=lambda d: ('13-inch' not in d['name'], 'Pro' not in d['name']))
    device = ipads[0]['udid']
    booted = ipads[0].get('state') == 'Booted'
    subprocess.run(['xcrun', 'simctl', 'boot', device], check=False, capture_output=True)
    run('xcrun', 'simctl', 'bootstatus', device, '-b', timeout=180)
    try:
        run('xcrun', 'simctl', 'install', device, str(app))
        documents = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, bundle, 'data'], text=True).strip()) / 'Documents'
        for name in ['result.json', 'failure.txt']:
            (documents / name).unlink(missing_ok=True)
        run('xcrun', 'simctl', 'launch', device, bundle)
        for _ in range(30):
            if (documents / 'result.json').exists() or (documents / 'failure.txt').exists(): break
            time.sleep(1)
        if (documents / 'failure.txt').exists(): raise AssertionError((documents / 'failure.txt').read_text())
        assert (documents / 'result.json').exists(), 'Native display-intent fixture timed out'
        result = json.loads((documents / 'result.json').read_text())
        output = Path(os.environ.get('BITRISE_DEPLOY_DIR', folder))
        output.mkdir(exist_ok=True, parents=True)
        shutil.copy2(documents / 'result.json', output / 'frame-intent-validation.json')
        print('PASS: production UIKit CADisplayLink uses selected 60/30, preserves MAX and other games, reuses the link and clears it on quit')
        print(json.dumps(result, sort_keys=True))
    finally:
        subprocess.run(['xcrun', 'simctl', 'terminate', device, bundle], check=False, capture_output=True)
        if not booted: run('xcrun', 'simctl', 'shutdown', device)
