import SwiftUI
import UniformTypeIdentifiers

/// The user-facing launcher.  Developer probes stay in the Advanced section
/// of ContentView; this view only exposes actions a player can understand:
/// import a folder/file, edit the command line, and press Run.
struct GameLibraryView: View {
    @ObservedObject var store: GameLibraryStore
    let isBusy: Bool
    let onLaunch: (GameProfile) -> Void

    @State private var showingImporter = false
    @State private var importerMode: ImporterMode = .folder
    @State private var editingGame: GameProfile?
    @State private var activeAlert: LibraryAlert?
    @State private var showingHelp = false

    private enum ImporterMode {
        case folder
        case executable
    }

    private enum LibraryAlert: Identifiable {
        case removal(GameProfile)
        case message(String)

        var id: String {
            switch self {
            case .removal(let game): return "remove-\(game.id.uuidString)"
            case .message(let text): return "message-\(text)"
            }
        }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            header

            if store.games.isEmpty {
                emptyState
            } else {
                ScrollView {
                    LazyVStack(spacing: 10) {
                        ForEach(store.games) { game in
                            GameCardView(
                                game: game,
                                installed: store.isInstalled(game),
                                isBusy: isBusy,
                                onLaunch: { launch(game) },
                                onEdit: { editingGame = game },
                                onRemove: { activeAlert = .removal(game) }
                            )
                        }
                    }
                }
                .frame(minHeight: 80, maxHeight: 220)
            }

            if case .importing = store.importState {
                HStack(spacing: 8) {
                    ProgressView()
                    Text("Đang chép game vào Wine…")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                .transition(.opacity)
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 10)
        .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16))
        .disabled(isBusy)
        .fileImporter(
            isPresented: $showingImporter,
            allowedContentTypes: importerMode == .folder
                ? [.folder]
                : [GameLibraryStore.windowsExecutableType],
            allowsMultipleSelection: false
        ) { result in
            switch result {
            case .success(let urls):
                guard let url = urls.first else { return }
                Task { await store.importItem(from: url) }
            case .failure(let error):
                activeAlert = .message(error.localizedDescription)
            }
        }
        .sheet(item: $editingGame) { game in
            GameEditSheet(profile: game) { updated in
                store.update(updated)
            }
        }
        .alert(item: $activeAlert) { alert in
            switch alert {
            case .removal(let game):
                return Alert(
                    title: Text("Xoá game khỏi Madeira?"),
                    message: Text("Xoá toàn bộ dữ liệu của \(game.name) khỏi bộ nhớ ứng dụng."),
                    primaryButton: .destructive(Text("Xoá")) {
                        Task {
                            do { try await store.remove(game) }
                            catch { activeAlert = .message(error.localizedDescription) }
                        }
                    },
                    secondaryButton: .cancel(Text("Huỷ"))
                )
            case .message(let message):
                return Alert(title: Text("Không thể hoàn tất"),
                             message: Text(message),
                             dismissButton: .cancel(Text("Đóng")))
            }
        }
        .sheet(isPresented: $showingHelp) {
            NavigationStack {
                ScrollView {
                    VStack(alignment: .leading, spacing: 14) {
                        Label("Nên chọn cả thư mục game", systemImage: "folder.fill")
                            .font(.headline)
                        Text("File .exe thường cần đi kèm thư mục game, DLL, ảnh và dữ liệu. Chọn thư mục gốc sẽ cho kết quả tốt hơn; chọn riêng .exe chỉ phù hợp với ứng dụng độc lập.")
                        Label("Ren’Py", systemImage: "book.closed.fill")
                            .font(.headline)
                        Text("Bản 64-bit được nhận diện tự động. Nếu game có đủ libEGL/libGLESv2, Madeira thử ANGLE2 → D3D11; nếu không sẽ dùng software renderer. Có thể đổi --renderer trong Chỉnh sửa.")
                        Label("Giới hạn hiện tại", systemImage: "info.circle")
                            .font(.headline)
                        Text("Không phải mọi .exe đều chạy được: game 32-bit, DirectX 12/Vulkan, anti-cheat, DRM hoặc API Windows chưa có trong Wine/DXMT sẽ cần bổ sung engine.")
                        Label("Bàn phím và chuột iPad", systemImage: "keyboard")
                            .font(.headline)
                        Text("Kết nối qua USB hoặc Bluetooth trước khi chạy. Madeira tự chuyển phím giữ/modifier, F1–F12, chuột trái/phải/giữa và cuộn sang input Windows. Các tổ hợp hệ thống như Command–H vẫn do iPadOS giữ lại.")
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding()
                }
                .navigationTitle("Cách thêm game")
                .toolbar {
                    ToolbarItem(placement: .topBarTrailing) {
                        Button("Xong") { showingHelp = false }
                    }
                }
            }
            .presentationDetents([.medium, .large])
        }
        .onChange(of: store.importState) { _, state in
            if case .failed(let message) = state { activeAlert = .message(message) }
            if case .imported = state {
                // Keep the success message out of the error alert; the card
                // appearing in the list is the clearest confirmation.
                store.resetImportState()
            }
        }
    }

    private var header: some View {
        HStack(alignment: .firstTextBaseline, spacing: 10) {
            VStack(alignment: .leading, spacing: 2) {
                Text("Thư viện game")
                    .font(.title3.weight(.semibold))
                Text("Thêm thư mục Ren’Py hoặc bất kỳ .exe tương thích")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer(minLength: 4)
            Button {
                showingHelp = true
            } label: {
                Image(systemName: "questionmark.circle")
            }
            .accessibilityLabel("Giới hạn và hướng dẫn thêm game")
            Menu {
                Button {
                    importerMode = .folder
                    showingImporter = true
                } label: {
                    Label("Chọn thư mục game", systemImage: "folder")
                }
                Button {
                    importerMode = .executable
                    showingImporter = true
                } label: {
                    Label("Chọn file .exe", systemImage: "doc.badge.plus")
                }
            } label: {
                Label("Thêm", systemImage: "plus")
                    .labelStyle(.titleAndIcon)
            }
            .buttonStyle(.borderedProminent)
            .accessibilityHint("Mở Files để chọn thư mục hoặc file thực thi Windows")
        }
    }

    private var emptyState: some View {
        VStack(spacing: 8) {
            Image(systemName: "tray.and.arrow.down")
                .font(.system(size: 30))
                .foregroundStyle(.tint)
            Text("Chưa có game")
                .font(.headline)
            Text("Bắt đầu bằng thư mục game đầy đủ để Madeira có thể tìm đúng executable và DLL đi kèm.")
                .font(.footnote)
                .multilineTextAlignment(.center)
                .foregroundStyle(.secondary)
            Button("Chọn thư mục game từ Files") {
                importerMode = .folder
                showingImporter = true
            }
                .buttonStyle(.bordered)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 18)
    }

    private func launch(_ game: GameProfile) {
        guard game.isSupported else {
            activeAlert = .message(game.compatibilityText)
            return
        }
        onLaunch(game)
    }

}

private struct GameCardView: View {
    let game: GameProfile
    let installed: Bool
    let isBusy: Bool
    let onLaunch: () -> Void
    let onEdit: () -> Void
    let onRemove: () -> Void

    var body: some View {
        HStack(spacing: 12) {
            Image(systemName: game.kind.iconName)
                .font(.title2)
                .foregroundStyle(game.kind == .renpy ? Color.purple : Color.accentColor)
                .frame(width: 42, height: 42)
                .background(.quaternary, in: RoundedRectangle(cornerRadius: 10))

            VStack(alignment: .leading, spacing: 4) {
                Text(game.name)
                    .font(.headline)
                    .lineLimit(1)
                HStack(spacing: 6) {
                    Text(game.kind.title)
                    Text("·")
                    Text(game.architecture.shortTitle)
                }
                .font(.caption)
                .foregroundStyle(.secondary)
                Text(installed ? game.compatibilityText : "Thiếu file trong bộ nhớ ứng dụng")
                    .font(.caption2)
                    .foregroundStyle(game.isSupported && installed ? .secondary : .orange)
                    .lineLimit(2)
                if let date = game.lastPlayedAt {
                    Text("Lần thử gần nhất: \(date.formatted(date: .abbreviated, time: .shortened))")
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                }
            }

            Spacer(minLength: 4)

            Button(action: onLaunch) {
                Image(systemName: "play.fill")
                    .frame(width: 34, height: 34)
            }
            .buttonStyle(.borderedProminent)
            .disabled(isBusy || !game.isSupported || !installed)
            .accessibilityLabel("Chạy \(game.name)")

            Menu {
                Button(action: onEdit) {
                    Label("Chỉnh sửa", systemImage: "slider.horizontal.3")
                }
                Button(action: onRemove) {
                    Label("Xoá khỏi thư viện", systemImage: "trash")
                }
            } label: {
                Image(systemName: "ellipsis.circle")
                    .frame(width: 30, height: 34)
            }
            .accessibilityLabel("Tuỳ chọn \(game.name)")
            .disabled(isBusy)
        }
        .padding(10)
        .background(.background.opacity(0.72), in: RoundedRectangle(cornerRadius: 12))
        .overlay {
            RoundedRectangle(cornerRadius: 12)
                .stroke(.quaternary, lineWidth: 1)
        }
    }
}

private struct GameEditSheet: View {
    @Environment(\.dismiss) private var dismiss
    let onSave: (GameProfile) -> Void
    @State private var draft: GameProfile

    init(profile: GameProfile, onSave: @escaping (GameProfile) -> Void) {
        self.onSave = onSave
        _draft = State(initialValue: profile)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Game") {
                    TextField("Tên hiển thị", text: $draft.name)
                    if let options = draft.executableOptions, options.count > 1 {
                        Picker("Executable", selection: $draft.executable) {
                            ForEach(options) { option in
                                Text("\(option.path) · \(option.architecture.shortTitle)")
                                    .tag(option.path)
                            }
                        }
                        .onChange(of: draft.executable) { _, path in
                            if let option = options.first(where: { $0.path == path }) {
                                draft.architecture = option.architecture
                            }
                        }
                    } else {
                        LabeledContent("Executable", value: draft.executable)
                    }
                    LabeledContent("Kiến trúc", value: draft.architecture.title)
                    LabeledContent("Loại", value: draft.kind.title)
                }
                Section("Tham số khởi chạy") {
                    TextField("Ví dụ: --renderer sw", text: $draft.arguments, axis: .vertical)
                        .lineLimit(1...3)
                    Text("Tham số được truyền nguyên dạng; dấu nháy kép được hỗ trợ bởi Wine bridge.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Section("Hiển thị") {
                    Toggle("Wine virtual desktop", isOn: $draft.desktopMode)
                    Text("Bật cho launcher hoặc game cần desktop Windows; game Ren’Py thường để tắt.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Section("Tương thích") {
                    Label(draft.compatibilityText,
                          systemImage: draft.isSupported ? "checkmark.circle" : "exclamationmark.triangle")
                        .foregroundStyle(draft.isSupported ? .secondary : .orange)
                }
            }
            .navigationTitle("Chỉnh sửa game")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Huỷ") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Lưu") {
                        let name = draft.name.trimmingCharacters(in: .whitespacesAndNewlines)
                        if !name.isEmpty { draft.name = name }
                        onSave(draft)
                        dismiss()
                    }
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}
