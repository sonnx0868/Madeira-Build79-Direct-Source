import SwiftUI
import Security

final class LibraryAccess: ObservableObject {
    static let shared = LibraryAccess()
    @Published private(set) var unlocked = false
    @Published var requested = false
    @Published private(set) var configured = UserDefaults.standard.bool(forKey: "madeiraLibraryAccessConfigured")
    private var epoch = 0
    private var afterAccess: (() -> Void)?
    private let service = "madeira.library.access.v1"
    private init() {
        if !configured, (try? record()) != nil {
            configured = true
            UserDefaults.standard.set(true, forKey: "madeiraLibraryAccessConfigured")
        }
    }

    var hasPrivateGames: Bool { LibraryModel.shared.entries.contains { LibraryModel.shared.isPrivate($0) } }
    var guarded: Bool { configured || hasPrivateGames }
    var privateSession: Bool { LibraryModel.shared.activeEntry.map { LibraryModel.shared.isPrivate($0) } ?? false }
    var protectedSurface: Bool { guarded && (!FrontendChoice.startup.useNew || privateSession) && !unlocked }

    func toggle() {
        if unlocked { lock() } else { request() }
    }
    func request(_ action: (() -> Void)? = nil) {
        if unlocked { action?(); return }
        afterAccess = action; requested = true
    }
    func lock() {
        epoch += 1; unlocked = false; requested = false; afterAccess = nil
        let model = LibraryModel.shared
        model.menu = false; model.showDetail = nil
        if guarded {
            model.error = nil; model.exitNotice = nil; model.launchAttention = nil
            model.cloudNotice = nil; model.cloudRetry = nil
        }
        if privateSession {
            model.error = nil; model.exitNotice = nil; model.launchAttention = nil
            model.cloudNotice = nil; model.cloudRetry = nil
            MetalHostView.shared.isHidden = true
        }
    }
    private func query() -> [String: Any] {
        [kSecClass as String: kSecClassGenericPassword, kSecAttrService as String: service,
         kSecAttrAccount as String: "library", kSecAttrSynchronizable as String: false]
    }
    private func record() throws -> LibraryPIN.Record? {
        var fields = query(); fields[kSecReturnData as String] = true
        fields[kSecMatchLimit as String] = kSecMatchLimitOne
        var item: CFTypeRef?
        let status = SecItemCopyMatching(fields as CFDictionary, &item)
        if status == errSecItemNotFound { return nil }
        guard status == errSecSuccess, let data = item as? Data else {
            throw NSError(domain: "MadeiraAccess", code: Int(status))
        }
        return try JSONDecoder().decode(LibraryPIN.Record.self, from: data)
    }
    @MainActor func submit(_ pin: String, confirmation: String) async -> String? {
        guard LibraryPIN.valid(pin) else { return "Enter a 6-digit PIN." }
        let defaults = UserDefaults.standard
        let until = defaults.double(forKey: "madeiraLibraryAccessRetryAt")
        guard Date().timeIntervalSince1970 >= until else { return "Please wait before trying again." }
        let token = epoch
        do {
            let existing = try record()
            if existing == nil && (configured || hasPrivateGames) {
                return "Access is unavailable. Unlock this device and use the original app signing identity."
            }
            if let existing {
                let accepted = try await Task.detached(priority: .userInitiated) {
                    try LibraryPIN.verify(pin, record: existing)
                }.value
                guard token == epoch, UIApplication.shared.applicationState == .active else { return nil }
                guard accepted else {
                    let failures = defaults.integer(forKey: "madeiraLibraryAccessFailures") + 1
                    defaults.set(failures, forKey: "madeiraLibraryAccessFailures")
                    if failures >= 5 {
                        defaults.set(Date().timeIntervalSince1970 + Double(min(600, 30 * failures)),
                                     forKey: "madeiraLibraryAccessRetryAt")
                    }
                    return "The PIN is incorrect."
                }
            } else {
                guard confirmation == pin else { return "The PINs do not match." }
                var salt = Data(count: 16)
                let status = salt.withUnsafeMutableBytes { SecRandomCopyBytes(kSecRandomDefault, 16, $0.baseAddress!) }
                guard status == errSecSuccess else { throw NSError(domain: "MadeiraAccess", code: Int(status)) }
                let fixedSalt = salt
                let digest = try await Task.detached(priority: .userInitiated) {
                    try LibraryPIN.derive(pin, salt: fixedSalt, rounds: 120_000)
                }.value
                guard token == epoch, UIApplication.shared.applicationState == .active else { return nil }
                let data = try JSONEncoder().encode(LibraryPIN.Record(salt: salt, digest: digest, rounds: 120_000))
                var fields = query(); fields[kSecValueData as String] = data
                fields[kSecAttrAccessible as String] = kSecAttrAccessibleWhenUnlockedThisDeviceOnly
                guard SecItemAdd(fields as CFDictionary, nil) == errSecSuccess else {
                    return "The access code could not be saved securely."
                }
                configured = true; defaults.set(true, forKey: "madeiraLibraryAccessConfigured")
            }
            defaults.removeObject(forKey: "madeiraLibraryAccessFailures")
            defaults.removeObject(forKey: "madeiraLibraryAccessRetryAt")
            unlocked = true; requested = false
            let action = afterAccess; afterAccess = nil; action?()
            if privateSession { MetalHostView.shared.isHidden = false }
            LibraryPrivacyShield.shared.refresh()
            return nil
        } catch { return "Access could not be verified. Unlock this device and try again." }
    }
}

struct LibraryAccessView: View {
    @ObservedObject private var access = LibraryAccess.shared
    @Environment(\.dismiss) private var dismiss
    @State private var pin = "", confirmation = "", error: String?
    @State private var busy = false
    var body: some View {
        NavigationStack {
            Form {
                Section {
                    SecureField("6-digit PIN", text: $pin).keyboardType(.numberPad)
                    if !access.guarded { SecureField("Confirm PIN", text: $confirmation).keyboardType(.numberPad) }
                    Button(busy ? "Verifying…" : (access.guarded ? "Continue" : "Set PIN")) {
                        busy = true; error = nil
                        Task { error = await access.submit(pin, confirmation: confirmation); busy = false; pin = ""; confirmation = "" }
                    }.disabled(busy)
                } footer: {
                    if !access.guarded {
                        Text("Hold the Madeira logo for two seconds to show or hide selected games. Access locks when you leave the app. Game files remain accessible in Files; this does not encrypt them.")
                    }
                }
                if let error { Section { Text(error).foregroundStyle(.red) } }
            }.navigationTitle("Access").navigationBarTitleDisplayMode(.inline)
                .toolbar { ToolbarItem(placement: .cancellationAction) { Button("Cancel") { access.requested = false; dismiss() } } }
        }
    }
}

// Above sheets and native guest windows, installed synchronously before iOS
// captures the app switcher. A protected live session stays covered on return.
final class LibraryPrivacyShield {
    static let shared = LibraryPrivacyShield()
    private var windows: [UIWindow] = []
    private var revision = 0
    func cover() {
        guard LibraryAccess.shared.guarded else { return }
        revision += 1
        if windows.isEmpty {
            for scene in UIApplication.shared.connectedScenes.compactMap({ $0 as? UIWindowScene }) {
                let window = UIWindow(windowScene: scene)
                window.frame = scene.coordinateSpace.bounds
                window.windowLevel = .alert + 5
                window.rootViewController = UIHostingController(rootView: LibraryNeutralAccess())
                windows.append(window)
            }
        }
        for window in windows { window.isHidden = false }
    }
    func refresh() {
        if LibraryAccess.shared.protectedSurface { cover() }
        else {
            revision += 1
            let token = revision
            if LibraryAccess.shared.unlocked {
                for window in windows { window.isHidden = true }
            } else {
                // Let SwiftUI finish dismissing details sheets before exposing
                // the public library after an automatic lock/foreground return.
                DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { [weak self] in
                    guard let self, self.revision == token,
                          UIApplication.shared.applicationState == .active,
                          !LibraryAccess.shared.protectedSurface else { return }
                    for window in self.windows { window.isHidden = true }
                }
            }
        }
    }
}

struct LibraryNeutralAccess: View {
    var showsAccess = true
    @ObservedObject private var access = LibraryAccess.shared
    var body: some View {
        VStack { Text("Madeira").font(.largeTitle.bold()).onLongPressGesture(minimumDuration: 2) { access.toggle() } }
            .frame(maxWidth: .infinity, maxHeight: .infinity).background(Color(uiColor: .systemBackground))
            .sheet(isPresented: Binding(get: { showsAccess && access.requested },
                                       set: { if !$0 { access.requested = false } })) { LibraryAccessView() }
    }
}

struct LibraryAccessPresentation: ViewModifier {
    @ObservedObject private var access = LibraryAccess.shared
    func body(content: Content) -> some View {
        content
            .sheet(isPresented: Binding(get: { access.requested && !access.protectedSurface },
                                       set: { if !$0 { access.requested = false } })) { LibraryAccessView() }
            .onReceive(NotificationCenter.default.publisher(for: UIApplication.willResignActiveNotification)) { _ in
                LibraryPrivacyShield.shared.cover(); access.lock()
            }
            .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in
                LibraryPrivacyShield.shared.refresh()
            }
            .onChange(of: access.unlocked) { _, _ in LibraryPrivacyShield.shared.refresh() }
            .onAppear { LibraryPrivacyShield.shared.refresh() }
    }
}
