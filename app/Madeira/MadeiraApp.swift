import SwiftUI
import Network

enum SteamNetworkDNS {
    static func apply() {
        let mode = MadeiraConfig.get("env.MADEIRA_STEAM_DNS")?.lowercased() ?? "auto"
        let context = NWParameters.PrivacyContext.default
        if ["system", "off", "0"].contains(mode) {
            context.requireEncryptedNameResolution(false, fallbackResolver: nil)
        } else {
            let google = mode == "google"
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
}

@main
struct MadeiraApp: App {
    init() { SteamNetworkDNS.apply() }
    var body: some Scene {
        WindowGroup {
            ContentView()
                .modifier(ClaimGamepadEvents())
                .modifier(LibraryAccessPresentation())
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.willResignActiveNotification)) { _ in
                    madeira_flush_pipeline_cache()
                }
                .onAppear {
                    GamepadInput.shared.start()
                    HardwareInput.shared.start()
                    JITNetworkShortcut.shared.restoreLeftover()   // also starts its network path monitor
                }
                // madeira://jit-network/...: the Madeira JIT shortcut returning (JITNetwork.swift).
                .onOpenURL { url in JITNetworkShortcut.shared.handle(url) }
        }
    }
}
