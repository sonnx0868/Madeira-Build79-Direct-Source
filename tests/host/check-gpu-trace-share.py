#!/usr/bin/env python3
"""Present the production file handoff on iPad Simulator, including a sheet.

Tests actual UIKit presentation and delegate/error/cancel handling. It cannot
claim a physical device's Files/cloud/third-party destination accepted a ZIP.
"""
from pathlib import Path
import base64, io, json, os, platform, plistlib, subprocess, tempfile, time, zipfile

root = Path(__file__).resolve().parents[2]
text = (root / 'app/Madeira/GPUTraceComparison.swift').read_text(encoding='utf-8')
presenter = text.split('// BEGIN GPU_TRACE_FILE_PRESENTATION', 1)[1].split('// END GPU_TRACE_FILE_PRESENTATION', 1)[0]
assert 'FileRepresentation(' not in text and 'ShareLink(' not in text
assert 'UIActivityViewController(activityItems: [request.url]' in presenter
assert 'UIDocumentPickerViewController(forExporting: [request.url], asCopy: true)' in presenter
assert 'Save ZIP to Files' in text and 'GPUTraceExports' in text
if platform.system() != 'Darwin':
    print('SKIP: actual iPad share/save modal test requires Xcode and iPad Simulator')
    raise SystemExit(0)

def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)

out = Path(os.environ.get('BITRISE_DEPLOY_DIR', root / 'build/gpu-share-artifacts'))
out.mkdir(parents=True, exist_ok=True)
payload = io.BytesIO()
with zipfile.ZipFile(payload, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('fixture.txt', 'UIKit export fixture; not a measured GPU capture')
with tempfile.TemporaryDirectory(prefix='madeira-gpu-share-') as temp:
    work = Path(temp); app = work / 'GPUShareCheck.app'; app.mkdir()
    source = work / 'Check.swift'
    source.write_text('import Foundation\nimport UIKit\nimport SwiftUI\n' + presenter + r'''
@MainActor final class State: ObservableObject {
    @Published var request: GPUTraceFileRequest?
    @Published var diagnostics = false
    var results: [String] = []
    var binding: Binding<GPUTraceFileRequest?> { Binding(get: { self.request }, set: { self.request = $0 }) }
}
struct Fixture: View {
    @ObservedObject var state: State
    var sheet = false
    var body: some View {
        VStack { Text(sheet ? "Diagnostics export fixture" : "Session export fixture").font(.title) }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background {
                if sheet || !state.diagnostics {
                    GPUTraceFilePresenter(request: state.binding) { state.results.append($0) }.frame(width: 1, height: 1)
                }
            }
            .sheet(isPresented: sheet ? .constant(false) : $state.diagnostics) { Fixture(state: state, sheet: true) }
    }
}
@main final class App: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    let state = State()
    let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    func application(_ application: UIApplication, didFinishLaunchingWithOptions options: [UIApplication.LaunchOptionsKey: Any]? = nil) -> Bool {
        window = UIWindow(frame: UIScreen.main.bounds)
        window!.rootViewController = UIHostingController(rootView: Fixture(state: state))
        window!.makeKeyAndVisible()
        Task { @MainActor in
            do { try await exercise() }
            catch { try? Data(String(describing: error).utf8).write(to: docs.appendingPathComponent("failure.txt")) }
        }
        return true
    }
    func controllers() -> [UIViewController] {
        var seen = Set<ObjectIdentifier>(), result: [UIViewController] = []
        func walk(_ vc: UIViewController) {
            guard seen.insert(ObjectIdentifier(vc)).inserted else { return }
            result.append(vc); vc.children.forEach(walk)
            if let presented = vc.presentedViewController { walk(presented) }
        }
        if let root = window?.rootViewController { walk(root) }; return result
    }
    func wait(_ condition: () -> Bool) async throws {
        for _ in 0..<400 {
            if condition() { return }
            try await Task.sleep(nanoseconds: 25_000_000)
        }
        throw NSError(domain: "UIKitFixture", code: 1, userInfo: [NSLocalizedDescriptionKey: "Timed out waiting for UIKit modal/state"])
    }
    func check(_ ok: @autoclosure () -> Bool, _ message: String) throws {
        if !ok() { throw NSError(domain: "UIKitFixture", code: 2, userInfo: [NSLocalizedDescriptionKey: message]) }
    }
    func exercise() async throws {
        let zip = docs.appendingPathComponent("Madeira-GPU-fixture.zip")
        try Data(base64Encoded: "FIXTURE_BASE64")!.write(to: zip)
        let empty = docs.appendingPathComponent("empty.zip"); try Data().write(to: empty)
        var rejected = false
        do { _ = try GPUTraceFileRequest(url: empty, action: .share) } catch { rejected = true }
        try check(rejected, "Empty file accepted")
        var checks: [String] = []
        for nested in [false, true] {
            if nested {
                state.diagnostics = true
                try await wait { self.window?.rootViewController?.presentedViewController?.viewIfLoaded?.window != nil }
                try await Task.sleep(nanoseconds: 600_000_000)
            }
            for action in [GPUTraceFileRequest.Action.share, .save] {
                let request = try GPUTraceFileRequest(url: zip, action: action)
                state.request = request
                try await wait {
                    self.controllers().contains { vc in
                        (action == .share ? vc is UIActivityViewController : vc is UIDocumentPickerViewController)
                            && vc.viewIfLoaded?.window != nil && vc.transitionCoordinator == nil
                    }
                }
                let host = controllers().compactMap { $0 as? GPUTraceFileHostController }.first { $0.request?.id == request.id }
                try check(host != nil, "Missing current SwiftUI presenter")
                let modal = controllers().first { action == .share ? $0 is UIActivityViewController : $0 is UIDocumentPickerViewController }!
                for _ in 0..<10 { host!.presentWhenReady() }
                try check(controllers().filter { $0 is UIActivityViewController || $0 is UIDocumentPickerViewController }.count == 1, "Duplicate modal")
                if let activity = modal as? UIActivityViewController {
                    try check(activity.popoverPresentationController?.sourceView === host!.view, "Missing iPad popover anchor")
                    try check(activity.popoverPresentationController!.sourceRect.width > 0, "Empty popover rect")
                }
                let name = (nested ? "diagnostics-" : "session-") + action.rawValue
                let renderer = UIGraphicsImageRenderer(bounds: window!.bounds)
                let image = renderer.image { _ in window!.drawHierarchy(in: window!.bounds, afterScreenUpdates: true) }
                try image.pngData()!.write(to: docs.appendingPathComponent(name + ".png"))
                let before = state.results.count
                if let activity = modal as? UIActivityViewController {
                    // Exercise cancellation and an extension-reported error.
                    let error: Error? = nested ? NSError(domain: "FixtureDestination", code: 3, userInfo: [NSLocalizedDescriptionKey: "fixture destination rejected"]) : nil
                    activity.completionWithItemsHandler?(nil, false, nil, error)
                    activity.completionWithItemsHandler?(nil, false, nil, error)
                } else {
                    let picker = modal as! UIDocumentPickerViewController
                    host!.documentPickerWasCancelled(picker)
                    host!.documentPickerWasCancelled(picker)
                }
                modal.dismiss(animated: false)
                try await wait { self.state.request == nil && modal.viewIfLoaded?.window == nil }
                try check(state.results.count == before + 1, "Duplicate/missing completion")
                try check(FileManager.default.fileExists(atPath: zip.path), "Cancellation removed ZIP")
                if nested && action == .share { try check(state.results.last!.contains("fixture destination rejected"), "Destination error lost") }
                checks.append(name)
                try await Task.sleep(nanoseconds: 150_000_000)
            }
        }
        let result: [String: Any] = ["passed": checks, "readyURL": true, "emptyFileRejected": true,
            "duplicateCompletionRejected": true, "originalZIPKept": true,
            "physicalFilesTransferTested": false, "thirdPartyTransferTested": false]
        try JSONSerialization.data(withJSONObject: result, options: [.prettyPrinted, .sortedKeys]).write(to: docs.appendingPathComponent("result.json"))
    }
}
'''.replace('FIXTURE_BASE64', base64.b64encode(payload.getvalue()).decode()), encoding='utf-8')
    sdk = subprocess.check_output(['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-path'], text=True).strip()
    run('xcrun', '--sdk', 'iphonesimulator', 'swiftc', '-swift-version', '5', '-parse-as-library',
        '-target', 'arm64-apple-ios17.0-simulator', '-sdk', sdk, str(source), '-o', str(app / 'GPUShareCheck'))
    with (app / 'Info.plist').open('wb') as f:
        plistlib.dump(dict(CFBundleIdentifier='org.madeira.gpu-share-check', CFBundleExecutable='GPUShareCheck',
            CFBundleName='GPU Share Check', CFBundleVersion='1', CFBundleShortVersionString='1.0',
            CFBundlePackageType='APPL', LSRequiresIPhoneOS=True, UIDeviceFamily=[2], UILaunchScreen={},
            UIFileSharingEnabled=True, UISupportsDocumentBrowser=True), f)
    run('codesign', '--force', '--sign', '-', str(app))
    devices = json.loads(subprocess.check_output(['xcrun', 'simctl', 'list', 'devices', 'available', '--json']))
    ipads = [d for group in devices['devices'].values() for d in group if d.get('isAvailable') and 'iPad' in d['name']]
    assert ipads, 'iPad Simulator runtime required for actual share/save presentation gate'
    ipads.sort(key=lambda d: ('13-inch' not in d['name'], 'Pro' not in d['name']))
    device = ipads[0]['udid']
    subprocess.run(['xcrun', 'simctl', 'boot', device], check=False, capture_output=True)
    run('xcrun', 'simctl', 'bootstatus', device, '-b', timeout=180)
    run('xcrun', 'simctl', 'install', device, str(app))
    run('xcrun', 'simctl', 'launch', device, 'org.madeira.gpu-share-check')
    container = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, 'org.madeira.gpu-share-check', 'data'], text=True).strip()) / 'Documents'
    for _ in range(90):
        if (container / 'result.json').exists() or (container / 'failure.txt').exists(): break
        time.sleep(1)
    if (container / 'failure.txt').exists(): raise AssertionError((container / 'failure.txt').read_text())
    assert (container / 'result.json').exists(), 'iPad share/save fixture timed out'
    result = json.loads((container / 'result.json').read_text())
    assert result['passed'] == ['session-share', 'session-save', 'diagnostics-share', 'diagnostics-save'], result
    import shutil
    shutil.copy2(container / 'result.json', out / 'gpu-share-checks.json')
    for name in result['passed']: shutil.copy2(container / (name + '.png'), out / ('gpu-' + name + '.png'))
    run('xcrun', 'simctl', 'terminate', device, 'org.madeira.gpu-share-check')
    print('PASS: actual iPad Simulator presents ready-file Share and Files from Session and diagnostics sheet; cancel/error callbacks keep ZIP and complete once')
    print('NOT TESTED: physical Files/cloud/third-party app ZIP delivery')
