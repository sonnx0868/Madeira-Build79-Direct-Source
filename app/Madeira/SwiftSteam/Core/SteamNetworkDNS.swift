// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 Madeira contributors
// Madeira Converter Exception: see LICENSE-EXCEPTION.md

import Foundation
import Network

enum SteamNetworkDNS {
    private static let lock = NSLock()
    private static var mode = "auto"
    private static var googleFallback = false
    private static var generation: UInt64 = 0

    static func apply() {
        let selected = MadeiraConfig.get("env.MADEIRA_STEAM_DNS")?.lowercased() ?? "auto"
        lock.lock(); defer { lock.unlock() }
        mode = ["system", "off", "0", "google", "cloudflare"].contains(selected) ? selected : "auto"
        googleFallback = false; generation &+= 1
        configure(mode)
    }

    /// Called with lock held. All automatic retries keep encrypted DNS and
    /// URLSession's original HTTPS hostname and certificate validation.
    private static func configure(_ selected: String) {
        let context = NWParameters.PrivacyContext.default
        if ["system", "off", "0"].contains(selected) {
            context.requireEncryptedNameResolution(false, fallbackResolver: nil)
        } else {
            let google = selected == "google"
            let url = URL(string: google ? "https://dns.google/dns-query" : "https://cloudflare-dns.com/dns-query")!
            let ips = google ? ["8.8.8.8", "8.8.4.4", "2001:4860:4860::8888"]
                             : ["1.1.1.1", "1.0.0.1", "2606:4700:4700::1111"]
            let addresses = ips.map { NWEndpoint.hostPort(host: NWEndpoint.Host($0), port: 443) }
            context.requireEncryptedNameResolution(true, fallbackResolver: .https(url, serverAddresses: addresses))
        }
        context.flushCache()
        // The default context also covers URLSession Steam downloads, which
        // never pass through Wine's Winsock-specific DoH bridge.
    }

    private static func snapshot() -> UInt64 {
        lock.lock(); defer { lock.unlock() }; return generation
    }

    private static func recover(_ error: Error, from started: UInt64) -> Bool {
        let error = error as NSError
        guard error.domain == NSURLErrorDomain,
              [NSURLErrorSecureConnectionFailed, NSURLErrorCannotFindHost, NSURLErrorDNSLookupFailed,
               NSURLErrorCannotConnectToHost, NSURLErrorTimedOut].contains(error.code) else { return false }
        lock.lock(); defer { lock.unlock() }
        // Explicit providers and System always keep their selected policy.
        guard mode == "auto" else { return false }
        // Concurrent requests begun on Cloudflare can each retry after one
        // switches the process. A request already begun on Google cannot loop.
        if generation != started { return googleFallback }
        guard !googleFallback else { return false }
        googleFallback = true; generation &+= 1
        configure("google")
        return true
    }

    static func retryGET<T>(url: URL, operation: () async throws -> T) async throws -> T {
        let started = snapshot()
        do { return try await operation() }
        catch {
            try Task.checkCancellation()
            guard url.scheme == "https", recover(error, from: started) else { throw error }
            let code = (error as NSError).code
            SteamLog.event("[steam-network] stage=dns-failover host=\(url.host ?? "invalid") provider=google code=\(code) retry=1")
            return try await operation()
        }
    }

    static func data(from url: URL, session: URLSession = .shared) async throws -> (Data, URLResponse) {
        try await retryGET(url: url) { try await session.data(from: url) }
    }
}

