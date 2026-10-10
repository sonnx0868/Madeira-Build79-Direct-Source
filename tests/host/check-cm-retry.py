#!/usr/bin/env python3
"""Compile the production CM transport and fault-inject its connection attempts."""
from pathlib import Path
import shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
core = root / 'app/Madeira/SwiftSteam/Core'
source = (core / 'SteamConnection.swift').read_text(encoding='utf-8')
assert 'webSocketTask === task' in source
assert 'didReceive challenge' not in source and 'URLCredential(trust:' not in source
checks = r'''
import Foundation
enum SteamError: Error { case noServersAvailable, connectionFailed(String), disconnected }
enum SteamLog { static func trace(_ s: String) {} ; static func event(_ s: String) {} }
enum EMsg {}
struct CMsgProtoBufHeader {}
enum SteamMessageCodec { static func encode(eMsg: EMsg, header: CMsgProtoBufHeader, body: Data) -> Data { body } }
@main struct Checks {
    static func main() async throws {
        var selected: [String] = [], failed: [Int] = []
        try await CMConnectionAttempts.run(next: { excluded in
            let id = ["cm1", "cm2", "cm3"].first { !excluded.contains($0) }!
            selected.append(id); return (id, id)
        }, dial: { server in
            if server != "cm3" { throw URLError(.secureConnectionFailed) }
        }, failed: { _, error, attempt in
            assert((error as NSError).code == NSURLErrorSecureConnectionFailed); failed.append(attempt)
        })
        assert(selected == ["cm1", "cm2", "cm3"] && failed == [1, 2])
        var attempts = 0
        do {
            try await CMConnectionAttempts.run(next: { excluded in
                let id = String(excluded.count); return (id, id)
            }, dial: { _ in attempts += 1; throw URLError(.serverCertificateUntrusted) }, failed: { _, _, _ in })
            assertionFailure("certificate failures must not turn into success")
        } catch { assert((error as NSError).code == NSURLErrorServerCertificateUntrusted && attempts == 3) }
        attempts = 0
        do {
            try await CMConnectionAttempts.run(next: { _ in ("same", "same") },
                dial: { _ in attempts += 1; throw URLError(.secureConnectionFailed) }, failed: { _, _, _ in })
            assertionFailure()
        } catch { assert(attempts == 1) }
        for cancellation in [CancellationError() as Error, URLError(.cancelled) as Error] {
            attempts = 0
            do {
                try await CMConnectionAttempts.run(next: { _ in ("cm", "cm") },
                    dial: { _ in attempts += 1; throw cancellation }, failed: { _, _, _ in assertionFailure() })
                assertionFailure()
            } catch { assert(attempts == 1) }
        }
        print("PASS: distinct TLS retries, bounded failures, certificate errors preserved and cancellation stops reconnecting")
    }
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-cm-retry-') as tmp:
    path = Path(tmp)
    (path / 'checks.swift').write_text(checks, encoding='utf-8')
    subprocess.run([shutil.which('swiftc'), '-swift-version', '5', '-parse-as-library',
                    str(core / 'SteamConnection.swift'), str(core / 'CMServerList.swift'),
                    str(path / 'checks.swift'), '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=30)
