import Foundation

/// Raw session bytes, copied on a utility queue independently of UI log parsing.
/// The journal lets the next app launch recover bytes written just before a crash.
final class GameLogArchive: @unchecked Sendable {
    static let shared = GameLogArchive()
    private struct Journal: Codable {
        var filename: String
        var startOffset: UInt64
        var headerBytes: UInt64
    }
    private let queue = DispatchQueue(label: "madeira.game-log-archive", qos: .utility)
    private var timer: DispatchSourceTimer?
    private var source: FileHandle?
    private var destination: FileHandle?
    private var journalURL: URL?
    private var current: Journal?
    private var cursor: UInt64 = 0

    static func filename(title: String, date: Date) -> String {
        let forbidden = CharacterSet.controlCharacters.union(CharacterSet(charactersIn: "/\\:*?\"<>|"))
        var scalars = String.UnicodeScalarView(), bytes = 0
        for input in title.unicodeScalars {
            let value = forbidden.contains(input) ? UnicodeScalar(95)! : input
            let length = String(value).utf8.count
            if bytes + length > 160 { break }
            scalars.append(value); bytes += length
        }
        let safe = String(scalars).trimmingCharacters(in: .whitespacesAndNewlines)
        let formatter = DateFormatter()
        formatter.locale = Locale(identifier: "en_US_POSIX")
        formatter.dateFormat = "yyyy-MM-dd_HH-mm-ss-SSS"
        return (safe.isEmpty || safe == "." || safe == ".." ? "Game" : safe) + "_" + formatter.string(from: date) + ".txt"
    }

    func begin(title: String, sourceURL: URL, metadata: [String: String]) {
        let offset = (try? sourceURL.resourceValues(forKeys: [.fileSizeKey]).fileSize).map { UInt64(max(0, $0)) } ?? 0
        queue.async { [self] in
            finishOnQueue()
            let folder = sourceURL.deletingLastPathComponent().appendingPathComponent("Logs", isDirectory: true)
            do {
                try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
                let name = Self.filename(title: title, date: Date())
                let target = folder.appendingPathComponent(name)
                let data = try JSONSerialization.data(withJSONObject: metadata, options: [.sortedKeys])
                let header = Data(("[game-log-context] " + String(decoding: data, as: UTF8.self) + "\n").utf8)
                try header.write(to: target, options: .withoutOverwriting)
                let record = Journal(filename: name, startOffset: offset, headerBytes: UInt64(header.count))
                let journal = folder.appendingPathComponent(".active-session.json")
                try JSONEncoder().encode(record).write(to: journal, options: .atomic)
                source = try FileHandle(forReadingFrom: sourceURL)
                destination = try FileHandle(forWritingTo: target)
                try destination?.seekToEnd()
                cursor = offset; current = record; journalURL = journal
                let timer = DispatchSource.makeTimerSource(queue: queue)
                timer.schedule(deadline: .now(), repeating: .seconds(1), leeway: .milliseconds(200))
                timer.setEventHandler { [weak self] in self?.drain(limit: 8 * 1024 * 1024) }
                self.timer = timer; timer.resume()
            } catch {
                fputs("[game-log] could not open session archive\n", stderr)
                closeOnQueue()
            }
        }
    }

    func finish() { queue.async { [self] in finishOnQueue() } }

    @discardableResult private func drain(limit: Int = Int.max) -> Bool {
        guard let source, let destination else { return false }
        do {
            try source.seek(toOffset: cursor)
            var remaining = limit
            while remaining > 0 {
                guard let bytes = try source.read(upToCount: min(256 * 1024, remaining)), !bytes.isEmpty else { break }
                try destination.write(contentsOf: bytes)
                cursor += UInt64(bytes.count); remaining -= bytes.count
            }
            return true
        } catch { return false /* Keep the journal for recovery. */ }
    }
    private func finishOnQueue() {
        timer?.cancel(); timer = nil
        if drain() {
            do {
                try destination?.synchronize()
                if let journalURL { try FileManager.default.removeItem(at: journalURL) }
            } catch { /* Recovery uses the actual destination length. */ }
        }
        closeOnQueue()
    }
    private func closeOnQueue() {
        timer?.cancel(); timer = nil
        try? source?.close(); try? destination?.close()
        source = nil; destination = nil; current = nil; journalURL = nil
    }

    /// Call before LogStore rotates the previous app-run log.
    static func recover(sourceURL: URL) {
        let folder = sourceURL.deletingLastPathComponent().appendingPathComponent("Logs", isDirectory: true)
        let marker = folder.appendingPathComponent(".active-session.json")
        guard let markerSize = try? marker.resourceValues(forKeys: [.fileSizeKey]).fileSize,
              markerSize >= 0, markerSize < 16 * 1024,
              let data = try? Data(contentsOf: marker), data.count < 16 * 1024,
              let record = try? JSONDecoder().decode(Journal.self, from: data),
              URL(fileURLWithPath: record.filename).lastPathComponent == record.filename,
              record.filename.hasSuffix(".txt") else { return }
        let target = folder.appendingPathComponent(record.filename)
        guard target.resolvingSymlinksInPath().deletingLastPathComponent() == folder.resolvingSymlinksInPath(),
              let size = try? target.resourceValues(forKeys: [.fileSizeKey]).fileSize,
              size >= 0, UInt64(size) >= record.headerBytes,
              record.startOffset <= UInt64.max - (UInt64(size) - record.headerBytes),
              let source = try? FileHandle(forReadingFrom: sourceURL),
              let destination = try? FileHandle(forWritingTo: target) else { return }
        defer { try? source.close(); try? destination.close() }
        do {
            try source.seek(toOffset: record.startOffset + UInt64(size) - record.headerBytes)
            try destination.seekToEnd()
            while let bytes = try source.read(upToCount: 256 * 1024), !bytes.isEmpty {
                try destination.write(contentsOf: bytes)
            }
            try destination.synchronize()
            try FileManager.default.removeItem(at: marker)
        } catch { /* Preserve the journal and captured file for another recovery. */ }
    }
}
