#!/usr/bin/env python3
"""Exercise the actual CDN pool, retry selection and native encrypted DNS API."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'app/Madeira/SwiftSteam/Content/DepotDownloader.swift').read_text(encoding='utf-8')
app=(root/'app/Madeira/SwiftSteam/Core/SteamNetworkDNS.swift').read_text(encoding='utf-8')
def block(text,signature):
    start=text.index(signature);end=text.index('{',start)+1;depth=1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required for CDN/DNS policy')
    print('SKIP: CDN/DNS policy requires Mac Swift/Network');raise SystemExit(0)
fixture='import Foundation\nimport Network\n'
fixture+='enum Pool {\n'+block(source,'    nonisolated static func contentPool(')+'\n}\n'
fixture+='enum MadeiraConfig { static var mode:String?; static func get(_ key:String)->String? { mode } }\n'
fixture+=block(app,'enum SteamNetworkDNS {')+'\n'
fixture+='enum SteamLog { static func event(_ message:String) {} }\n'
fixture+=block(source,'final class ContentHostHealth:')+'\n'
start=source.index('            let order = plan.health.order(plan.hosts, seed: seed)')
end=source.index('            let url = ',start)
selection=source[start:end].replace('plan.health','health').replace('plan.hosts','pool')
fixture+=r'''
let fallback="https://steampipe.akamaized.net"
let offered=(0..<20).map { "https://cache\($0).steamcontent.com" }+[fallback]
let pool=Pool.contentPool(offered,limit:6)
assert(pool.count==6 && pool.contains(fallback))
assert(Pool.contentPool([fallback],limit:6)==[fallback])
assert(Pool.contentPool(offered,limit:0).isEmpty)
let health=ContentHostHealth();let seed=0
var tried=Set<String>()
for attempt in 0..<pool.count {
'''+selection+r'''
    health.recordFailure(host,reason:"url-1003")
}
assert(tried.count==pool.count && tried.contains(fallback))
for mode in ["cloudflare","google","system","auto"] {
    MadeiraConfig.mode=mode;SteamNetworkDNS.apply()
}
MadeiraConfig.mode="system";SteamNetworkDNS.apply()
print("PASS: fallback CDN survives pool bounds, changing health visits every host, native encrypted DNS API respects Cloudflare/Google/system choices")
'''

# Execute the production manifest retry function, with deterministic transport
# failures and a parser result. This reaches the same fallback that prefix(6)
# excluded, without using Steam account credentials or a live game depot.
fixture+=r'''
struct DepotManifest: Sendable {
    struct File: Sendable { var flags: UInt32 = 0 }
    static let customExecutableFlag: UInt32 = 1
    var files: [File] = [File()]
}
enum SteamError: Error { case manifestFetchFailed(String) }
@MainActor final class ManifestProbe {
    let hostPoolSize = 6
    var depotCache: URL?
    static var mode = 0
    static var attempted: [String] = []
    static func reset(_ value: Int) { mode = value; attempted = [] }
    func manifestRequestCode(depotID: UInt32, appID: UInt32, manifestGID: UInt64) async throws -> UInt64 { 123 }
    func cdnAuthFragment(depotID: UInt32, appID: UInt32, host: String) async -> String { "?auth=synthetic" }
    static func download(_ url: String) async throws -> Data {
        let host = URL(string: url)!.host!
        attempted.append(host)
        if mode == 2 { throw URLError(.cancelled) }
        if mode == 1 || host != "steampipe.akamaized.net" { throw URLError(.secureConnectionFailed) }
        return Data([42])
    }
    nonisolated static func parseManifestKeepingPayload(_ raw: Data, depotID: UInt32, manifestGID: UInt64, key: Data) throws -> (DepotManifest, Data) {
        assert(raw == Data([42])); return (DepotManifest(), raw)
    }
'''+block(source,'    nonisolated static func contentPool(')+'\n'+block(source,'    private func fetchManifest(')+r'''
    func run(_ hosts: [String]) async throws {
        _ = try await fetchManifest(depotID: 7, appID: 8, manifestGID: 9, key: Data(), hosts: hosts)
    }
}
let manifestProbe = await ManifestProbe()
await ManifestProbe.reset(0)
try await manifestProbe.run(offered)
let manifestAttempts = await ManifestProbe.attempted
assert(manifestAttempts.count == 6 && manifestAttempts.last == "steampipe.akamaized.net")
await ManifestProbe.reset(1)
do { try await manifestProbe.run(offered); assertionFailure("All failed edges must fail the operation") }
catch let error as URLError { assert(error.code == .secureConnectionFailed) }
let failedManifestAttempts = await ManifestProbe.attempted.count
assert(failedManifestAttempts == 6)
await ManifestProbe.reset(2)
do { try await manifestProbe.run(offered); assertionFailure("Cancellation cannot return success") }
catch is CancellationError {}
let cancelledManifestAttempts = await ManifestProbe.attempted.count
assert(cancelledManifestAttempts == 1)
print("PASS: production manifest reaches HTTPS fallback after bad regional edges; failures stay bounded and transport cancellation stops immediately")

let steamURL = URL(string: "https://api.steampowered.com/test")!
MadeiraConfig.mode = "auto"; SteamNetworkDNS.apply()
var transportAttempts = 0
let repaired: Int = try await SteamNetworkDNS.retryGET(url: steamURL) {
    transportAttempts += 1
    if transportAttempts == 1 { throw URLError(.secureConnectionFailed) }
    return 17
}
assert(repaired == 17 && transportAttempts == 2)
// A request already using the automatic Google fallback cannot loop.
transportAttempts = 0
do {
    let _: Int = try await SteamNetworkDNS.retryGET(url: steamURL) {
        transportAttempts += 1; throw URLError(.secureConnectionFailed)
    }
    assertionFailure("Failed transport must remain failed")
} catch let error as URLError { assert(error.code == .secureConnectionFailed) }
assert(transportAttempts == 1)
for mode in ["system", "google", "cloudflare", "auto"] {
    MadeiraConfig.mode = mode; SteamNetworkDNS.apply()
    for code in [URLError.cancelled, .serverCertificateUntrusted] {
        transportAttempts = 0
        do {
            let _: Int = try await SteamNetworkDNS.retryGET(url: steamURL) {
                transportAttempts += 1; throw URLError(code)
            }
            assertionFailure("Cancellation and certificate errors must propagate")
        } catch let error as URLError { assert(error.code == code) }
        assert(transportAttempts == 1)
    }
    if mode != "auto" {
        transportAttempts = 0
        do {
            let _: Int = try await SteamNetworkDNS.retryGET(url: steamURL) {
                transportAttempts += 1; throw URLError(.cannotFindHost)
            }
            assertionFailure("Explicit provider failures must propagate")
        } catch let error as URLError { assert(error.code == .cannotFindHost) }
        assert(transportAttempts == 1)
    }
}
actor ConcurrentGET {
    var counts: [Int: Int] = [:]
    var waiting: [CheckedContinuation<Void, Never>] = []
    func fetch(_ id: Int) async throws -> Int {
        counts[id, default: 0] += 1
        if counts[id]! > 1 { return id }
        await withCheckedContinuation { continuation in
            waiting.append(continuation)
            if waiting.count == 2 {
                let ready = waiting; waiting = []
                for value in ready { value.resume() }
            }
        }
        throw URLError(.secureConnectionFailed)
    }
    func attempts(_ id: Int) -> Int { counts[id, default: 0] }
}
MadeiraConfig.mode = "auto"; SteamNetworkDNS.apply()
let concurrent = ConcurrentGET()
async let firstGET: Int = SteamNetworkDNS.retryGET(url: steamURL) { try await concurrent.fetch(1) }
async let secondGET: Int = SteamNetworkDNS.retryGET(url: steamURL) { try await concurrent.fetch(2) }
let concurrentResults = try await (firstGET, secondGET)
let firstCount = await concurrent.attempts(1), secondCount = await concurrent.attempts(2)
assert(concurrentResults == (1, 2) && firstCount == 2 && secondCount == 2)
MadeiraConfig.mode = "system"; SteamNetworkDNS.apply()
print("PASS: automatic HTTPS recovery retries once through encrypted Google DNS, including concurrent failures; explicit providers, certificate failures and cancellation are preserved")
'''
with tempfile.TemporaryDirectory(prefix='madeira-cdn-') as scratch:
    folder=Path(scratch);path=folder/'main.swift';path.write_text(fixture,encoding='utf-8')
    subprocess.run([swift,str(path),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True,timeout=15)
