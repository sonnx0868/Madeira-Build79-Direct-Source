// SPDX-License-Identifier: GPL-3.0-or-later
// Madeira Converter Exception: see LICENSE-EXCEPTION.md

import Foundation

/// An enable/shortcut response belongs to one attempt in one app process.
/// Only restoration work survives relaunch; active request identities do not.
struct JITRequestGate: Sendable {
    struct Identity: Equatable, Sendable {
        let run: UUID
        let request: UUID
        let pid: Int32
        var callbackValue: String { run.uuidString + "." + String(pid) + "." + request.uuidString }
    }
    private let run = UUID()
    private(set) var current: Identity?
    private(set) var deadline: TimeInterval = 0
    mutating func begin(pid: Int32, now: TimeInterval, timeout: TimeInterval) -> Identity {
        let value = Identity(run: run, request: UUID(), pid: pid)
        current = value; deadline = now + timeout
        return value
    }
    func accepts(_ value: Identity) -> Bool { current == value }
    func accepts(callback: String) -> Bool { current?.callbackValue == callback }
    func expired(now: TimeInterval) -> Bool { current != nil && now >= deadline }
    @discardableResult mutating func finish(_ value: Identity) -> Bool {
        guard current == value else { return false }
        current = nil; deadline = 0; return true
    }
    static func debuggerReady(flagged: Bool, attached: Bool) -> Bool { flagged && attached }
}

/// Codable messages shared by Madeira and its JIT helper extension. Each travels as JSON
/// in an extension request's item (userInfo[itemKey]), to the helper and back.
struct MadeiraJITRequest: Codable, Sendable {
    static let itemKey = "madeira-jit-request"

    enum Operation: String, Codable, Sendable {
        case prepare
        case enable
        case resetDDI
    }

    let operation: Operation
    let targetPID: Int32?
    let pairingData: Data?
    let scriptBase64: String?

    static func prepare(pairingData: Data) -> MadeiraJITRequest {
        MadeiraJITRequest(operation: .prepare, targetPID: nil,
                          pairingData: pairingData, scriptBase64: nil)
    }

    static func enable(targetPID: Int32, pairingData: Data,
                       scriptBase64: String) -> MadeiraJITRequest {
        MadeiraJITRequest(operation: .enable, targetPID: targetPID,
                          pairingData: pairingData, scriptBase64: scriptBase64)
    }

    static var resetDDI: MadeiraJITRequest {
        MadeiraJITRequest(operation: .resetDDI, targetPID: nil,
                          pairingData: nil, scriptBase64: nil)
    }

    struct Response: Codable, Sendable {
        static let itemKey = "madeira-jit-response"
        let success: Bool
        let message: String
        let txmPresent: Bool?
    }
}
