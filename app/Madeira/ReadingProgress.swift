import SwiftUI
import UIKit

/// One reader for the active game. Nothing is scheduled for disabled games.
/// All repeated file IO runs on a serial utility queue, never the render loop.
final class ReadingProgressModel: ObservableObject {
    static let shared = ReadingProgressModel()
    @Published private(set) var snapshot: ReadingProgressSnapshot?
    @Published private(set) var message = "Waiting for dialogue…"
    @Published private(set) var enabled = false
    @Published private(set) var installed = false
    @Published private(set) var position = ReadingProgressPosition.initial
    var hitRect = CGRect.zero
    private let queue = DispatchQueue(label: "madeira.reading-progress", qos: .utility)
    private var timer: DispatchSourceTimer?
    private var session: String?
    private var folder: URL?
    private var gameID: UUID?
    private var visible = false
    private var foreground = true
    private var observers: [NSObjectProtocol] = []

    private init() {
        for (name, active) in [(UIApplication.didBecomeActiveNotification, true),
                               (UIApplication.willResignActiveNotification, false)] {
            observers.append(NotificationCenter.default.addObserver(forName: name, object: nil, queue: .main) { [weak self] _ in
                self?.foreground = active
                if !active { self?.hitRect = .zero }
                self?.updateReader()
            })
        }
    }

    static func supports(_ entry: LibraryEntry) -> Bool {
        entry.desktop != true && entry.temporarySession != true && entry.gameEngine == "Ren'Py"
    }

    /// Called before the process starts, including Steam/Epic launch routes.
    /// A failed optional helper leaves the normal game launch available.
    func begin(_ entry: LibraryEntry) {
        end()
        gameID = entry.id
        enabled = entry.readingProgress == true
        position = (entry.readingProgressPosition ?? .initial).bounded
        foreground = UIApplication.shared.applicationState == .active
        guard entry.desktop != true, entry.temporarySession != true,
              let gameFolder = ReadingProgressFiles.gameFolder(
                location: LibraryModel.drive.appendingPathComponent(entry.launchRelativePath), drive: LibraryModel.drive) else {
            if enabled { message = "Reading progress is unavailable for this game." }
            return
        }
        let token = UUID().uuidString
        do {
            // A few KB once before launch. The session token prevents stale
            // counters from a previous process or another game being shown.
            try queue.sync {
                let bridge = Bundle.main.url(forResource: "MadeiraReadingProgress", withExtension: "rpy")
                    .flatMap { ReadingProgressFiles.boundedRead($0, limit: 32 * 1024) }
                try ReadingProgressFiles.prepare(folder: gameFolder, enabled: enabled, bridge: bridge, session: token)
            }
            if enabled { folder = gameFolder; session = token; installed = true }
        } catch {
            message = error.localizedDescription
            LogStore.shared.log("[reading-progress] optional helper unavailable: \(error.localizedDescription)")
        }
    }

    func end() {
        visible = false; hitRect = .zero
        timer?.cancel(); timer = nil
        if let folder, let session {
            queue.async { try? ReadingProgressFiles.setEnabled(false, folder: folder, session: session) }
        }
        session = nil; folder = nil; gameID = nil
        snapshot = nil; installed = false; enabled = false
        message = "Waiting for dialogue…"
    }

    func setVisible(_ value: Bool) {
        guard visible != value else { return }
        visible = value
        if !value { hitRect = .zero }
        updateReader()
    }

    func setEnabled(_ value: Bool) {
        guard let id = gameID, installed || !value else { return }
        enabled = value
        if !value { hitRect = .zero }
        if var entry = LibraryModel.shared.entries.first(where: { $0.id == id }) {
            entry.readingProgress = value
            LibraryModel.shared.save(entry)
        }
        updateReader()
    }

    func savePosition(_ value: ReadingProgressPosition) {
        position = value.bounded
        guard let id = gameID, var entry = LibraryModel.shared.entries.first(where: { $0.id == id }) else { return }
        entry.readingProgressPosition = position
        LibraryModel.shared.save(entry)
    }

    private func updateReader() {
        timer?.cancel(); timer = nil
        guard let folder, let session else { return }
        let active = enabled && visible && foreground
        queue.async { try? ReadingProgressFiles.setEnabled(active, folder: folder, session: session) }
        guard active else { return }
        let started = Date()
        let timer = DispatchSource.makeTimerSource(queue: queue)
        timer.schedule(deadline: .now(), repeating: 2, leeway: .milliseconds(500))
        timer.setEventHandler { [weak self] in
            let value = ReadingProgressFiles.boundedRead(folder.appendingPathComponent(ReadingProgressFiles.output))
                .flatMap { ReadingProgressSnapshot.decode($0, session: session) }
            DispatchQueue.main.async { [weak self] in
                guard let self, self.session == session, self.enabled, self.visible, self.foreground else { return }
                if let value {
                    if self.snapshot != value { self.snapshot = value }
                    let status = value.fraction == nil ? "A percentage is unavailable for this script version." : "Dialogue seen across all playthroughs"
                    if self.message != status { self.message = status }
                } else if self.snapshot == nil && Date().timeIntervalSince(started) >= 30 {
                    let status = "Waiting for Ren’Py's reading counters."
                    if self.message != status { self.message = status }
                }
            }
        }
        self.timer = timer
        timer.resume()
    }
}

/// Keep counter publications out of the large LibraryHUD view hierarchy.
struct ReadingProgressOverlay: View {
    let viewport: CGSize
    let insets: EdgeInsets
    @ObservedObject private var model = ReadingProgressModel.shared
    var body: some View {
        if model.enabled { ReadingProgressBubble(viewport: viewport, insets: insets) }
    }
}

struct ReadingProgressMenuRow: View {
    @ObservedObject private var model = ReadingProgressModel.shared
    var body: some View {
        if model.installed || model.enabled {
            Toggle("Reading progress bubble", isOn: Binding(get: { model.enabled }, set: { model.setEnabled($0) }))
            Text(model.message).font(.caption).foregroundStyle(.secondary)
        }
    }
}

/// Native UI above the game surface, with only the circle taking touches.
/// It redraws on a changed counter or a drag; there is no display link.
struct ReadingProgressBubble: View {
    let viewport: CGSize
    let insets: EdgeInsets
    @ObservedObject private var model = ReadingProgressModel.shared
    @GestureState private var drag = CGSize.zero
    @Environment(\.accessibilityReduceMotion) private var reduceMotion
    private let diameter: CGFloat = 78

    private func center(_ translation: CGSize) -> CGPoint {
        let radius = diameter / 2 + 8
        let left = insets.leading + radius, top = insets.top + radius
        return CGPoint(
            x: min(max(left, viewport.width * model.position.x + translation.width), max(left, viewport.width - insets.trailing - radius)),
            y: min(max(top, viewport.height * model.position.y + translation.height), max(top, viewport.height - insets.bottom - radius)))
    }

    var body: some View {
        let point = center(drag)
        let rect = CGRect(x: point.x - diameter / 2, y: point.y - diameter / 2, width: diameter, height: diameter)
        ZStack {
            Circle().fill(LinearGradient(colors: [Color(white: 0.16), Color(white: 0.08)], startPoint: .topLeading, endPoint: .bottomTrailing))
            Circle().strokeBorder(.white.opacity(0.06), lineWidth: 1)
            Circle().stroke(.white.opacity(0.09), lineWidth: 5).padding(7)
            Circle().trim(from: 0, to: model.snapshot?.fraction ?? 0)
                .stroke(Color(red: 0.25, green: 0.62, blue: 0.98), style: StrokeStyle(lineWidth: 5, lineCap: .round))
                .rotationEffect(.degrees(-90)).padding(7)
            VStack(spacing: 1) {
                Text(model.snapshot?.percentage.map { "\($0)%" } ?? "—")
                    .font(.system(size: 23, weight: .semibold, design: .rounded)).monospacedDigit()
                Text("READ").font(.system(size: 8, weight: .medium)).tracking(1.2).foregroundStyle(.white.opacity(0.55))
            }.foregroundStyle(.white)
        }
        .frame(width: diameter, height: diameter)
        .shadow(color: .black.opacity(0.25), radius: 5, y: 2)
        .contentShape(Circle())
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("Reading progress")
        .accessibilityValue(model.snapshot.map { "\($0.seen) of \($0.total) dialogue blocks seen. \(model.message)" } ?? model.message)
        .accessibilityHint("Drag to move. Change visibility in the game menu.")
        .highPriorityGesture(DragGesture(minimumDistance: 4, coordinateSpace: .global).updating($drag) { value, state, transaction in
            transaction.animation = nil; state = value.translation
        }.onEnded { value in
            let point = center(value.translation)
            model.savePosition(ReadingProgressPosition(x: point.x / max(1, viewport.width), y: point.y / max(1, viewport.height)))
        })
        .position(point)
        .onAppear { model.hitRect = rect; model.setVisible(true) }
        .onChange(of: rect) { _, value in model.hitRect = value }
        .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in model.hitRect = rect }
        .onDisappear { model.setVisible(false); model.hitRect = .zero }
        .animation(reduceMotion ? nil : .easeOut(duration: 0.2), value: model.snapshot?.percentage)
    }
}
