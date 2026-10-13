import Foundation
import CommonCrypto

enum LibraryPIN {
    struct Record: Codable {
        var salt: Data
        var digest: Data
        var rounds: UInt32
    }
    static func valid(_ pin: String) -> Bool {
        pin.utf8.count == 6 && pin.utf8.allSatisfy { (48...57).contains($0) }
    }
    static func derive(_ pin: String, salt: Data, rounds: UInt32) throws -> Data {
        guard !salt.isEmpty, salt.count <= 64, rounds > 0, rounds <= 500_000 else {
            throw NSError(domain: "MadeiraAccess", code: 1)
        }
        var bytes = [UInt8](repeating: 0, count: 32)
        let status = pin.withCString { password in
            salt.withUnsafeBytes { source in
                bytes.withUnsafeMutableBufferPointer { result in
                    CCKeyDerivationPBKDF(CCPBKDFAlgorithm(kCCPBKDF2), password, pin.utf8.count,
                        source.bindMemory(to: UInt8.self).baseAddress, salt.count,
                        CCPseudoRandomAlgorithm(kCCPRFHmacAlgSHA256), rounds, result.baseAddress, result.count)
                }
            }
        }
        guard status == kCCSuccess else { throw NSError(domain: "MadeiraAccess", code: Int(status)) }
        return Data(bytes)
    }
    static func verify(_ pin: String, record: Record) throws -> Bool {
        guard valid(pin), record.salt.count == 16, record.digest.count == 32,
              (120_000...500_000).contains(record.rounds) else { return false }
        let actual = try derive(pin, salt: record.salt, rounds: record.rounds)
        var difference: UInt8 = 0
        for (a, b) in zip(actual, record.digest) { difference |= a ^ b }
        return difference == 0
    }
}
