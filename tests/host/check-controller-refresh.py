#!/usr/bin/env python3
"""Drive the actual profile-rebinding method with replaceable framework handles."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
root = Path(__file__).resolve().parents[2]
source = (root/'app/Madeira/GamepadInput.swift').read_text(encoding='utf-8')
assert 'let refresh = Timer(timeInterval: 1, repeats: true)' in source
assert 'self.setActive(foreground)' in source and 'RunLoop.main.add(refresh, forMode: .common)' in source
assert '[controller-lifecycle]' in source and '[controller-owner]' in source
method = '@MainActor func refreshControllers() {' + source.split('    @MainActor private func refreshControllers() {',1)[1].split('    private func setActive',1)[0]
swift = shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required')
    print('SKIP: controller profile replacement requires Swift; macOS CI requires it')
    raise SystemExit(0)
prelude = r'''
import Foundation
final class GCExtendedGamepad {
    var valueChangedHandler: ((GCExtendedGamepad, Int) -> Void)?
}
final class GCController {
    static var live: [GCController] = []
    static func controllers() -> [GCController] { live }
    var extendedGamepad: GCExtendedGamepad?
    var handlerQueue: DispatchQueue?
}
final class Probe: @unchecked Sendable {
    let queue = DispatchQueue(label:"controller-test")
    var controllers = [GCController?](repeating:nil,count:4)
    var profiles = [GCExtendedGamepad?](repeating:nil,count:4)
    var samples = 0, timerUpdates = 0
    func updateTimer() { timerUpdates += 1 }
    func sample() { samples += 1 }
'''
checks = r'''
}
MainActor.assumeIsolated {
let probe = Probe(), controller = GCController(), original = GCExtendedGamepad()
controller.extendedGamepad = original; GCController.live = [controller]
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] === controller && probe.profiles[0] === original)
assert(original.valueChangedHandler != nil)
let replacement = GCExtendedGamepad()
controller.extendedGamepad = replacement
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] === controller && probe.profiles[0] === replacement)
assert(original.valueChangedHandler == nil && replacement.valueChangedHandler != nil)
replacement.valueChangedHandler?(replacement, 0); probe.queue.sync {}
let afterRebind = probe.queue.sync { probe.samples }
assert(afterRebind >= 3)
let second = GCController(); second.extendedGamepad = GCExtendedGamepad()
GCController.live = [second,controller] // framework order changes; keep player 1 identity.
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] === controller && probe.controllers[1] === second)
GCController.live = [second]
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] == nil && replacement.valueChangedHandler == nil)
controller.extendedGamepad = nil; GCController.live = [controller,second]
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] == nil)
controller.extendedGamepad = replacement
probe.refreshControllers(); probe.queue.sync {}
assert(probe.controllers[0] === controller && probe.profiles[0] === replacement)
print("PASS: production refresh rebinds replacement profiles, retains player slots, clears disconnected handlers and recovers a delayed profile")
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-controller-refresh-') as temp:
    folder = Path(temp)
    (folder/'main.swift').write_text(prelude+method+checks,encoding='utf-8')
    subprocess.run([swift,str(folder/'main.swift'),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True)
