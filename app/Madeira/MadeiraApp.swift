import SwiftUI
import Network

@main
struct MadeiraApp: App {
    init() { SteamNetworkDNS.apply() }
    var body: some Scene {
        WindowGroup {
            ContentView()
                .modifier(ClaimGamepadEvents())
                .modifier(LibraryAccessPresentation())
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.willResignActiveNotification)) { _ in
                    madeira_session_priority_foreground(0)
                    madeira_flush_pipeline_cache()
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in
                    madeira_session_priority_foreground(1)
                }
                .onAppear {
                    madeira_session_priority_foreground(1)
                    GamepadInput.shared.start()
                    HardwareInput.shared.start()
                    JITNetworkShortcut.shared.restoreLeftover()   // also starts its network path monitor
                }
                // madeira://jit-network/...: the Madeira JIT shortcut returning (JITNetwork.swift).
                .onOpenURL { url in JITNetworkShortcut.shared.handle(url) }
        }
    }
}
