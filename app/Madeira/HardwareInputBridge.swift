import Foundation
import GameController
import Combine

/// Routes physical iPad keyboards, mice and trackpads into Wine's existing
/// input queue. GameController gives us key/button *edges* (including key-up),
/// unlike text-input APIs which lose modifiers, function keys and held state.
final class HardwareInputBridge: ObservableObject {
    static let shared = HardwareInputBridge()

    private let lock = NSLock()
    private var enabled = false
    private var observers: [NSObjectProtocol] = []
    private weak var keyboard: GCKeyboard?
    private weak var mouse: GCMouse?
    private var heldKeys = Set<Int32>()
    private var leftDown = false
    private var rightDown = false
    private var middleDown = false
    private var moveCarryX: Float = 0
    private var moveCarryY: Float = 0
    private var wheelCarryX: Float = 0
    private var wheelCarryY: Float = 0

    @Published private(set) var keyboardConnected = false
    @Published private(set) var mouseConnected = false

    var hasPhysicalKeyboard: Bool { keyboardConnected }
    var hasMouse: Bool { mouseConnected }

    private init() {
        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .GCKeyboardDidConnect,
                                             object: nil, queue: .main) { [weak self] note in
            self?.bindKeyboard(note.object as? GCKeyboard)
        })
        observers.append(center.addObserver(forName: .GCKeyboardDidDisconnect,
                                             object: nil, queue: .main) { [weak self] note in
            guard let self else { return }
            if let disconnected = note.object as? GCKeyboard,
               disconnected === self.keyboard {
                self.releaseEverything()
                self.bindKeyboard(nil)
            }
        })
        observers.append(center.addObserver(forName: .GCMouseDidConnect,
                                             object: nil, queue: .main) { [weak self] note in
            self?.bindMouse(note.object as? GCMouse)
        })
        observers.append(center.addObserver(forName: .GCMouseDidDisconnect,
                                             object: nil, queue: .main) { [weak self] note in
            guard let self else { return }
            if let disconnected = note.object as? GCMouse,
               disconnected === self.mouse {
                self.releaseEverything()
                self.bindMouse(nil)
            }
        })
        bindKeyboard(GCKeyboard.coalesced)
        bindMouse(GCMouse.current)
    }

    deinit {
        observers.forEach { NotificationCenter.default.removeObserver($0) }
    }

    /// Enable only for a live Wine session. Disabling emits every missing key
    /// and button-up edge so disconnect/background can never leave input held.
    func setEnabled(_ value: Bool) {
        lock.lock()
        let changed = enabled != value
        enabled = value
        lock.unlock()
        guard changed else { return }

        if value {
            bindKeyboard(GCKeyboard.coalesced)
            bindMouse(GCMouse.current)
        } else {
            releaseEverything()
        }
    }

    private func bindKeyboard(_ device: GCKeyboard?) {
        keyboard?.keyboardInput?.keyChangedHandler = nil
        keyboard = device
        DispatchQueue.main.async { [weak self] in self?.keyboardConnected = device != nil }
        device?.keyboardInput?.keyChangedHandler = { [weak self] _, _, keyCode, pressed in
            guard let self, let vk = Self.windowsVirtualKey(for: Int(keyCode.rawValue)) else { return }
            self.postKey(vk, pressed: pressed)
        }
    }

    private func bindMouse(_ device: GCMouse?) {
        if let old = mouse?.mouseInput {
            old.mouseMovedHandler = nil
            old.leftButton.pressedChangedHandler = nil
            old.rightButton?.pressedChangedHandler = nil
            old.middleButton?.pressedChangedHandler = nil
            old.scroll.valueChangedHandler = nil
        }

        mouse = device
        DispatchQueue.main.async { [weak self] in self?.mouseConnected = device != nil }
        guard let input = device?.mouseInput else { return }
        input.mouseMovedHandler = { [weak self] _, deltaX, deltaY in
            self?.postMouseMove(x: deltaX, y: deltaY)
        }
        input.leftButton.pressedChangedHandler = { [weak self] _, _, pressed in
            self?.postMouseButton(.left, pressed: pressed)
        }
        input.rightButton?.pressedChangedHandler = { [weak self] _, _, pressed in
            self?.postMouseButton(.right, pressed: pressed)
        }
        input.middleButton?.pressedChangedHandler = { [weak self] _, _, pressed in
            self?.postMouseButton(.middle, pressed: pressed)
        }
        input.scroll.valueChangedHandler = { [weak self] _, x, y in
            self?.postScroll(x: x, y: y)
        }
    }

    private func postKey(_ vk: Int32, pressed: Bool) {
        lock.lock()
        guard enabled else { lock.unlock(); return }
        let shouldPost: Bool
        if pressed { shouldPost = heldKeys.insert(vk).inserted }
        else { shouldPost = heldKeys.remove(vk) != nil }
        lock.unlock()
        if shouldPost { winios_post_key(vk, pressed ? 1 : 0) }
    }

    private enum MouseButton { case left, right, middle }

    private func postMouseButton(_ button: MouseButton, pressed: Bool) {
        let flags: UInt32
        lock.lock()
        guard enabled else { lock.unlock(); return }
        switch button {
        case .left:
            guard leftDown != pressed else { lock.unlock(); return }
            leftDown = pressed
            flags = pressed ? 0x0002 : 0x0004
        case .right:
            guard rightDown != pressed else { lock.unlock(); return }
            rightDown = pressed
            flags = pressed ? 0x0008 : 0x0010
        case .middle:
            guard middleDown != pressed else { lock.unlock(); return }
            middleDown = pressed
            flags = pressed ? 0x0020 : 0x0040
        }
        lock.unlock()
        winios_pointer_action(flags, 0)
    }

    private func postMouseMove(x: Float, y: Float) {
        lock.lock()
        guard enabled else { lock.unlock(); return }
        moveCarryX += x
        // GameController uses Cartesian +Y; Windows relative mouse uses screen +Y.
        moveCarryY -= y
        let dx = Int32(max(-32767, min(32767, moveCarryX.rounded(.towardZero))))
        let dy = Int32(max(-32767, min(32767, moveCarryY.rounded(.towardZero))))
        moveCarryX -= Float(dx)
        moveCarryY -= Float(dy)
        lock.unlock()
        if dx != 0 || dy != 0 { winios_pointer_relative(dx, dy, 0x0001, 0) }
    }

    private func postScroll(x: Float, y: Float) {
        lock.lock()
        guard enabled else { lock.unlock(); return }
        wheelCarryX += x * 120
        wheelCarryY += y * 120
        let horizontal = Int32(max(-1200, min(1200, wheelCarryX.rounded(.towardZero))))
        let vertical = Int32(max(-1200, min(1200, wheelCarryY.rounded(.towardZero))))
        wheelCarryX -= Float(horizontal)
        wheelCarryY -= Float(vertical)
        lock.unlock()
        if vertical != 0 {
            winios_pointer_action(0x0800, UInt32(bitPattern: vertical))
        }
        if horizontal != 0 {
            winios_pointer_action(0x1000, UInt32(bitPattern: horizontal))
        }
    }

    private func releaseEverything() {
        lock.lock()
        let keys = heldKeys
        heldKeys.removeAll()
        let releaseLeft = leftDown
        let releaseRight = rightDown
        let releaseMiddle = middleDown
        leftDown = false
        rightDown = false
        middleDown = false
        moveCarryX = 0; moveCarryY = 0
        wheelCarryX = 0; wheelCarryY = 0
        lock.unlock()

        keys.forEach { winios_post_key($0, 0) }
        if releaseLeft { winios_pointer_action(0x0004, 0) }
        if releaseRight { winios_pointer_action(0x0010, 0) }
        if releaseMiddle { winios_pointer_action(0x0040, 0) }
    }

    /// GCKeyCode raw values are USB HID keyboard usages. Mapping raw values
    /// avoids locale/text conversion and preserves modifiers and held keys.
    private static func windowsVirtualKey(for code: Int) -> Int32? {
        switch code {
        case 4...29: return Int32(0x41 + code - 4)       // A...Z
        case 30...38: return Int32(0x31 + code - 30)     // 1...9
        case 39: return 0x30                                  // 0
        case 40: return 0x0D                                  // Enter
        case 41: return 0x1B                                  // Escape
        case 42: return 0x08                                  // Backspace
        case 43: return 0x09                                  // Tab
        case 44: return 0x20                                  // Space
        case 45: return 0xBD                                  // -
        case 46: return 0xBB                                  // =
        case 47: return 0xDB                                  // [
        case 48: return 0xDD                                  // ]
        case 49, 50, 100: return 0xDC                         // \
        case 51: return 0xBA                                  // ;
        case 52: return 0xDE                                  // '
        case 53: return 0xC0                                  // `
        case 54: return 0xBC                                  // ,
        case 55: return 0xBE                                  // .
        case 56: return 0xBF                                  // /
        case 57: return 0x14                                  // Caps Lock
        case 58...69: return Int32(0x70 + code - 58)     // F1...F12
        case 70: return 0x2C                                  // Print Screen
        case 71: return 0x91                                  // Scroll Lock
        case 72: return 0x13                                  // Pause
        case 73: return 0x2D                                  // Insert
        case 74: return 0x24                                  // Home
        case 75: return 0x21                                  // Page Up
        case 76: return 0x2E                                  // Delete
        case 77: return 0x23                                  // End
        case 78: return 0x22                                  // Page Down
        case 79: return 0x27                                  // Right
        case 80: return 0x25                                  // Left
        case 81: return 0x28                                  // Down
        case 82: return 0x26                                  // Up
        case 83: return 0x90                                  // Num Lock
        case 84: return 0x6F                                  // Keypad /
        case 85: return 0x6A                                  // Keypad *
        case 86: return 0x6D                                  // Keypad -
        case 87: return 0x6B                                  // Keypad +
        case 88: return 0x0D                                  // Keypad Enter
        case 89...97: return Int32(0x61 + code - 89)     // Keypad 1...9
        case 98: return 0x60                                  // Keypad 0
        case 99: return 0x6E                                  // Keypad .
        case 101: return 0x5D                                 // Application/Menu
        case 224: return 0xA2                                 // Left Ctrl
        case 225: return 0xA0                                 // Left Shift
        case 226: return 0xA4                                 // Left Alt
        case 227: return 0x5B                                 // Left Command/Win
        case 228: return 0xA3                                 // Right Ctrl
        case 229: return 0xA1                                 // Right Shift
        case 230: return 0xA5                                 // Right Alt
        case 231: return 0x5C                                 // Right Command/Win
        default: return nil
        }
    }
}
