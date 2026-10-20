import SwiftUI
import Security
import LocalAuthentication

final class LibraryAccess: ObservableObject {
    static let shared = LibraryAccess()
    static let logoHoldDuration = 0.7
    @Published private(set) var unlocked = false
    @Published var requested = false
    @Published private(set) var configured = UserDefaults.standard.bool(forKey: "madeiraLibraryAccessConfigured")
    @Published private(set) var biometricsEnabled = false
    @Published private(set) var biometricsAvailable = false
    @Published private(set) var biometricName = "Face ID / Touch ID"
    @Published private(set) var biometricIcon = "faceid"
    private var biometricContext: LAContext?
    private var epoch = 0
    private var afterAccess: (() -> Void)?
    var privateLibraryVisible = false
    private let service = "madeira.library.access.v1"
    private init() {
        let saved = try? record()
        biometricsEnabled = saved?.biometricDomain != nil
        if !configured, saved != nil {
            configured = true
            UserDefaults.standard.set(true, forKey: "madeiraLibraryAccessConfigured")
        }
    }

    var hasPrivateGames: Bool { LibraryModel.shared.entries.contains { LibraryModel.shared.isPrivate($0) } }
    var guarded: Bool { configured || hasPrivateGames }
    var privateSession: Bool { LibraryModel.shared.activeEntry.map { LibraryModel.shared.isPrivate($0) } ?? false }
    var protectedSurface: Bool { privateSession && !unlocked }

    func toggle() {
        if unlocked { lock() } else { request() }
    }
    func request(_ action: (() -> Void)? = nil) {
        if unlocked { action?(); return }
        afterAccess = action; requested = true
    }
    func cancelRequest() {
        epoch += 1; requested = false; afterAccess = nil
        biometricContext?.invalidate(); biometricContext = nil
    }
    func lock() {
        cancelRequest(); unlocked = false
        privateLibraryVisible = false
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
    func background(enteringBackground: Bool = false) {
        // Apple's biometric sheet temporarily makes the app inactive. Do not
        // cancel that very authentication; a real app switch still cancels it.
        if biometricContext != nil && !enteringBackground { return }
        biometricContext?.invalidate(); biometricContext = nil
        // Cancel a pending PIN verification, but keep an authorized live game.
        // The switcher snapshot is covered separately. Quit locks the shelf.
        if LibraryModel.shared.current != nil || (unlocked && LibraryModel.shared.startingJIT != nil) {
            epoch += 1; requested = false; afterAccess = nil
        } else { lock() }
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
    private func updateRecord(_ value: LibraryPIN.Record) throws {
        let data = try JSONEncoder().encode(value)
        let status = SecItemUpdate(query() as CFDictionary, [kSecValueData as String: data] as CFDictionary)
        guard status == errSecSuccess else { throw NSError(domain: "MadeiraAccess", code: Int(status)) }
    }
    func refreshBiometrics() {
        let context = LAContext()
        biometricsAvailable = context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, error: nil)
        switch context.biometryType {
        case .faceID: biometricName = "Face ID"; biometricIcon = "faceid"
        case .touchID: biometricName = "Touch ID"; biometricIcon = "touchid"
        default: biometricName = "Face ID / Touch ID"; biometricIcon = "faceid"
        }
    }
    @MainActor func setBiometricsEnabled(_ enabled: Bool) async -> String? {
        guard unlocked, configured, biometricContext == nil else { return "Unlock your private playlist with its PIN first." }
        do {
            guard var saved = try record() else { return "The private playlist PIN is unavailable." }
            if !enabled {
                saved.biometricDomain = nil
                try updateRecord(saved); biometricsEnabled = false
                return nil
            }
            let context = LAContext()
            context.localizedFallbackTitle = ""
            guard context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, error: nil) else {
                return "Set up Face ID or Touch ID in device Settings first. You can still use your Madeira PIN."
            }
            let token = epoch
            biometricContext = context
            defer { if biometricContext === context { biometricContext = nil } }
            let accepted = try await context.evaluatePolicy(.deviceOwnerAuthenticationWithBiometrics,
                localizedReason: "Use biometrics to open your private playlist.")
            guard token == epoch, unlocked, UIApplication.shared.applicationState != .background else { return nil }
            guard accepted, let domain = context.evaluatedPolicyDomainState, !domain.isEmpty else {
                return "Biometric access could not be enabled. Your PIN is unchanged."
            }
            saved.biometricDomain = domain
            try updateRecord(saved); biometricsEnabled = true
            return nil
        } catch { return "Biometric access was not changed. You can still use your Madeira PIN." }
    }
    @MainActor func authenticateBiometrics() async -> String? {
        guard requested, !unlocked, biometricsEnabled, biometricContext == nil else { return nil }
        let token = epoch
        let context = LAContext()
        context.localizedFallbackTitle = "Use Madeira PIN"
        context.localizedCancelTitle = "Use PIN"
        guard context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, error: nil) else {
            return "Biometrics are unavailable. Enter your Madeira PIN."
        }
        do {
            guard let saved = try record(), let enrolled = saved.biometricDomain,
                  enrolled == context.evaluatedPolicyDomainState else {
                return "Biometric settings changed. Use your PIN, then enable biometric access again."
            }
            biometricContext = context
            defer { if biometricContext === context { biometricContext = nil } }
            let accepted = try await context.evaluatePolicy(.deviceOwnerAuthenticationWithBiometrics,
                localizedReason: "Open your private playlist.")
            guard LibraryBiometricPolicy.accepts(success: accepted, currentRequest: token == epoch,
                requested: requested, backgrounded: UIApplication.shared.applicationState == .background,
                enrolled: enrolled, current: context.evaluatedPolicyDomainState) else { return nil }
            finishAccess()
            return nil
        } catch {
            guard token == epoch else { return nil }
            if let error = error as? LAError,
               [.userCancel, .userFallback, .appCancel, .systemCancel].contains(error.code) { return nil }
            return "Biometric verification did not complete. Enter your Madeira PIN."
        }
    }
    private func finishAccess() {
        unlocked = true; requested = false
        let action = afterAccess; afterAccess = nil; action?()
        if privateSession { MetalHostView.shared.isHidden = false }
        LibraryPrivacyShield.shared.refresh()
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
            finishAccess()
            return nil
        } catch { return "Access could not be verified. Unlock this device and try again." }
    }
}

enum LibraryTheme {
    static let background = Color(red: 0.035, green: 0.055, blue: 0.065)
    static let panel = Color(red: 0.075, green: 0.11, blue: 0.135)
    static let accent = Color(red: 0.23, green: 0.77, blue: 0.94)
}

/// One static bundled illustration. No per-frame blur, animation or network fetch.
struct LibraryScenicBackground: View {
    var sidebar = false
    var body: some View {
        GeometryReader { geometry in
            Image("LibraryLandscape").resizable().scaledToFill()
                .frame(width: geometry.size.width, height: geometry.size.height, alignment: .leading).clipped()
                .overlay {
                    if sidebar {
                        LinearGradient(colors: [LibraryTheme.background, LibraryTheme.background.opacity(0.95),
                            LibraryTheme.background.opacity(0.35), .clear], startPoint: .top, endPoint: .bottom)
                    }
                }
        }.accessibilityHidden(true)
    }
}

struct LibraryAccessView: View {
    @ObservedObject private var access = LibraryAccess.shared
    @Environment(\.dismiss) private var dismiss
    @State private var pin = ""
    @State private var confirmation = ""
    @State private var error: String?
    @State private var busy = false
    private var enteringConfirmation: Bool { !access.guarded && !confirmation.isEmpty }
    private var heading: String {
        access.guarded ? "Enter Passcode" : enteringConfirmation ? "Confirm Passcode" : "Create Passcode"
    }
    @MainActor private func useBiometrics() async {
        guard !busy else { return }
        busy = true; error = nil
        error = await access.authenticateBiometrics()
        busy = false
    }
    private func digit(_ value: Int) {
        guard !busy, pin.count < 6 else { return }
        error = nil; pin += String(value)
        if pin.count == 6 {
            if !access.guarded && confirmation.isEmpty { confirmation = pin; pin = ""; return }
            let candidate = access.guarded ? pin : confirmation
            let repeated = pin
            busy = true
            Task { @MainActor in
                error = await access.submit(candidate, confirmation: repeated)
                busy = false; pin = ""
                if error != nil && !access.guarded { confirmation = "" }
            }
        }
    }
    private func key(_ value: Int) -> some View {
        let letters = ["", "", "ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ"]
        return Button { digit(value) } label: {
            VStack(spacing: 2) {
                Text(String(value)).font(.system(size: 30, weight: .regular, design: .rounded))
                if !letters[value].isEmpty { Text(letters[value]).font(.system(size: 9, weight: .medium)).tracking(2) }
            }.frame(width: 76, height: 76)
                .background(.white.opacity(0.09), in: Circle()).contentShape(Circle())
        }.buttonStyle(.plain).disabled(busy)
            .keyboardShortcut(KeyEquivalent(Character(String(value))), modifiers: [])
            .accessibilityLabel(String(value))
    }
    var body: some View {
        ZStack {
            LibraryScenicBackground().ignoresSafeArea()
            Color.black.opacity(0.34).ignoresSafeArea()
            ViewThatFits(in: .vertical) {
                keypad.padding(.vertical, 40)
                ScrollView { keypad.padding(.vertical, 20) }
            }
        }.foregroundStyle(.white).preferredColorScheme(.dark)
            .interactiveDismissDisabled(busy).presentationDragIndicator(.hidden)
            .task {
                access.refreshBiometrics()
                if access.biometricsEnabled { await useBiometrics() }
            }
            .onDisappear { access.cancelRequest() }
    }
    private var keypad: some View {
        VStack(spacing: 22) {
            Image(systemName: "lock.fill").font(.title2).foregroundStyle(.cyan)
                .frame(width: 58, height: 58).background(.white.opacity(0.06), in: Circle())
            VStack(spacing: 8) {
                Text(heading).font(.title2.weight(.semibold))
                Text("Open your private playlist").font(.subheadline).foregroundStyle(.white.opacity(0.6))
            }
            HStack(spacing: 15) {
                ForEach(0..<6, id: \.self) { index in
                    Circle().fill(index < pin.count ? Color.cyan : .clear)
                        .overlay(Circle().stroke(.white.opacity(0.55), lineWidth: 1))
                        .frame(width: 13, height: 13)
                }
            }.accessibilityElement(children: .ignore).accessibilityLabel("\(pin.count) of 6 digits entered")
            ZStack {
                if busy { ProgressView().tint(.white) }
                else if let error { Text(error).font(.caption).foregroundStyle(.red).multilineTextAlignment(.center) }
            }.frame(height: 30)
            VStack(spacing: 15) {
                ForEach(0..<3, id: \.self) { row in
                    HStack(spacing: 25) { ForEach(1..<4, id: \.self) { column in key(row * 3 + column) } }
                }
                HStack(spacing: 25) {
                    Button("Cancel") { access.cancelRequest(); dismiss() }
                        .font(.caption).frame(width: 76, height: 76)
                        .background(.white.opacity(0.09), in: Circle()).keyboardShortcut(.cancelAction)
                    key(0)
                    Button { if !pin.isEmpty { pin.removeLast(); error = nil } }
                        label: { Image(systemName: "delete.left").frame(width: 76, height: 76) }
                        .keyboardShortcut(.delete, modifiers: []).accessibilityLabel("Delete digit")
                }.buttonStyle(.plain).disabled(busy)
            }
            if access.biometricsEnabled {
                Button {
                    Task { @MainActor in await useBiometrics() }
                } label: { Label("Use \(access.biometricName)", systemImage: access.biometricIcon) }
                    .disabled(busy)
            }
            if !access.guarded {
                Text("Briefly hold the Madeira logo to open or hide your private playlist. Game files remain accessible in Files.")
                    .font(.caption2).foregroundStyle(.white.opacity(0.45)).multilineTextAlignment(.center)
                    .frame(maxWidth: 300)
            }
        }.padding(.horizontal, 24).frame(maxWidth: .infinity)
    }
}

/// Only shown inside an already unlocked playlist; no public-screen prompt.
struct LibraryBiometricSetting: View {
    @ObservedObject private var access = LibraryAccess.shared
    @State private var busy = false
    @State private var error: String?
    var body: some View {
        if access.unlocked {
            VStack(alignment: .leading, spacing: 6) {
                Toggle("Unlock with \(access.biometricName)", isOn: Binding(
                    get: { access.biometricsEnabled }, set: { value in
                        guard !busy else { return }
                        busy = true; error = nil
                        Task { @MainActor in
                            error = await access.setBiometricsEnabled(value)
                            busy = false
                        }
                    }))
                    .disabled(busy || (!access.biometricsAvailable && !access.biometricsEnabled))
                Text(error ?? "Your Madeira PIN remains available as a fallback.")
                    .font(.caption).foregroundStyle(error == nil ? Color.secondary : Color.orange)
            }.frame(maxWidth: 360, alignment: .leading)
                .task { access.refreshBiometrics() }
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
        let access = LibraryAccess.shared
        // A configured PIN alone does not make a public screen private.
        guard access.guarded && (access.privateSession || access.privateLibraryVisible) else { return }
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
        guard UIApplication.shared.applicationState == .active else { return }
        if LibraryAccess.shared.protectedSurface { cover() }
        else {
            revision += 1
            let token = revision
            if LibraryAccess.shared.unlocked || LibraryModel.shared.current != nil ||
                LibraryModel.shared.startingJIT != nil || windows.allSatisfy({ $0.isHidden }) {
                for window in windows { window.isHidden = true }
            } else {
                // Let SwiftUI finish dismissing details sheets before exposing
                // the public library after an automatic lock/foreground return.
                DispatchQueue.main.async { [weak self] in
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
        VStack { Text("Madeira").font(.largeTitle.bold()).onLongPressGesture(minimumDuration: LibraryAccess.logoHoldDuration) { access.toggle() } }
            .frame(maxWidth: .infinity, maxHeight: .infinity).background(Color(uiColor: .systemBackground))
            .sheet(isPresented: Binding(get: { showsAccess && access.requested },
                                       set: { if !$0 { access.cancelRequest() } })) { LibraryAccessView() }
    }
}

struct LibraryAccessPresentation: ViewModifier {
    @ObservedObject private var access = LibraryAccess.shared
    func body(content: Content) -> some View {
        content
            .sheet(isPresented: Binding(get: { access.requested && !access.protectedSurface },
                                       set: { if !$0 { access.cancelRequest() } })) { LibraryAccessView() }
            .onReceive(NotificationCenter.default.publisher(for: UIApplication.willResignActiveNotification)) { _ in
                LibraryPrivacyShield.shared.cover(); access.background()
            }
            .onReceive(NotificationCenter.default.publisher(for: UIApplication.didEnterBackgroundNotification)) { _ in
                access.background(enteringBackground: true)
            }
            .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in
                LibraryPrivacyShield.shared.refresh()
            }
            .onChange(of: access.unlocked) { _, _ in LibraryPrivacyShield.shared.refresh() }
            .onAppear { LibraryPrivacyShield.shared.refresh() }
    }
}
