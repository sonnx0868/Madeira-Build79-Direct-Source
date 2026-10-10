import SwiftUI
import UniformTypeIdentifiers

struct DiagnosticUploadView: View {
    @ObservedObject private var upload = RemoteDiagnostics.shared
    @Environment(\.dismiss) private var dismiss
    @State private var previous = false
    @State private var label = ""
    @State private var importServer = false
    @State private var preparingLab = false
    @State private var labStatus: String?
    init(gameTitle: String? = nil) {
        _label = State(initialValue: gameTitle ?? "")
    }
    var body: some View {
        NavigationStack {
            Form {
                Section {
                    if let server = URL(string: upload.server), server.scheme == "https",
                       server.host?.hasSuffix(".chatgpt.site") == true {
                        Link("Download iPad server configuration", destination: server)
                    }
                    Button("Import server configuration") { importServer = true }
                    TextField("Server URL", text: $upload.server).keyboardType(.URL).textInputAutocapitalization(.never).autocorrectionDisabled()
                    SecureField("Upload key", text: $upload.key).textInputAutocapitalization(.never).autocorrectionDisabled()
                    Button("Save server") { upload.saveConfiguration() }
                } header: { Text("Log server") } footer: {
                    Text("Connect once: open your server in Safari, sign in, download the iPad configuration, then import it here. The upload key is saved securely on this iPad.")
                }
                Section {
                    Picker("Session", selection: $previous) {
                        Text("Current session").tag(false)
                        Text("Previous session (after restarting Madeira)").tag(true)
                    }
                    TextField("Game / issue", text: $label, axis: .vertical)
                    Button { Task { await upload.send(previous: previous, label: label) } } label: {
                        if upload.busy { HStack { ProgressView(); Text("Sending…") } }
                        else { Label("Send log to server", systemImage: "square.and.arrow.up") }
                    }.disabled(upload.busy)
                } header: { Text("Diagnostic report") } footer: {
                    Text("Sends the selected session log, recent Unity Player.log and Steam network logs, plus build and device details to your private server. Logs are limited to 20 MB total. Your logs stay on this iPad.")
                }
                if let status = upload.status { Section { Text(status).textSelection(.enabled) } }
                Section {
                    Button {
                        preparingLab = true
                        let drive = MadeiraDock.drive
                        let resources = Bundle.main.bundleURL.appendingPathComponent("translation-lab")
                        Task {
                            do {
                                let path = try await Task.detached(priority: .utility) {
                                    try TranslationTools.stage(resources: resources, drive: drive)
                                }.value
                                dismiss()
                                NotificationCenter.default.post(name: TranslationTools.launch, object: path)
                            } catch { labStatus = error.localizedDescription }
                            preparingLab = false
                        }
                    } label: {
                        if preparingLab { HStack { ProgressView(); Text("Preparing…") } }
                        else { Label("Run CPU translation comparison", systemImage: "cpu") }
                    }.disabled(preparingLab || wine_process_is_running() != 0 || wineserver_is_running() != 0)
                    if let labStatus { Text(labStatus).font(.footnote) }
                    if let report = TranslationTools.latestReport(drive: MadeiraDock.drive) {
                        Text(report).font(.caption.monospaced()).textSelection(.enabled)
                    }
                } header: { Text("CPU comparison") } footer: {
                    Text("Uses JIT to compare x64 work with small and grouped ARM64EC calls. The result is included when you send the log. This synthetic test does not predict game FPS. Close the current game before running it.")
                }
                if let id = upload.reportID {
                    Section("Report ID") {
                        Text(id).font(.footnote.monospaced()).textSelection(.enabled)
                        ShareLink(item: "Madeira diagnostic report: \(id)") { Label("Share report ID", systemImage: "square.and.arrow.up") }
                    }
                }
            }
            .disabled(upload.busy)
            .navigationTitle("Send diagnostic log").navigationBarTitleDisplayMode(.inline)
            .toolbar { ToolbarItem(placement: .confirmationAction) { Button("Done") { dismiss() }.disabled(upload.busy) } }
        }
        .interactiveDismissDisabled(upload.busy)
        .fileImporter(isPresented: $importServer, allowedContentTypes: [.json]) { result in
            switch result {
            case .success(let url): upload.importConfiguration(from: url)
            case .failure(let error): upload.status = error.localizedDescription
            }
        }
    }
}

struct AppUpdateSection: View {
    @StateObject private var updates = AppUpdateModel()
    @AppStorage("madeira.updates.testBuilds") private var testBuilds = true
    @Environment(\.openURL) private var openURL
    var body: some View {
        Section {
            LabeledContent("Installed", value: BuildStamp.text)
            Toggle("Include test builds", isOn: $testBuilds)
                .disabled(updates.busy).onChange(of: testBuilds) { _ in updates.available = nil; updates.status = nil }
            Button { Task { await updates.check(includeTestBuilds: testBuilds) } } label: {
                if updates.busy { HStack { ProgressView(); Text("Checking…") } }
                else { Label("Check for updates", systemImage: "arrow.triangle.2.circlepath") }
            }.disabled(updates.busy)
            if let available = updates.available {
                Text(available.title).font(.subheadline)
                Button("Download IPA in browser") { openURL(available.url) }
            }
            if let status = updates.status { Text(status).font(.footnote).foregroundStyle(.secondary) }
            Link("Open GitHub Releases", destination: UpdateRules.releasesURL)
        } header: { Text("App updates") } footer: {
            Text("Downloads open in your browser. Install the IPA with your usual sideloading tool.")
        }
    }
}

struct AppUpdateView: View {
    @Environment(\.dismiss) private var dismiss
    var body: some View {
        NavigationStack {
            Form { AppUpdateSection() }
                .navigationTitle("App updates").navigationBarTitleDisplayMode(.inline)
                .toolbar { ToolbarItem(placement: .confirmationAction) { Button("Done") { dismiss() } } }
        }
    }
}
