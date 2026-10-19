import Foundation
import Combine
import CryptoKit

struct ReleaseVersion: Comparable, Equatable {
    let parts: [Int]
    init?(_ value: String) {
        guard let regex = try? NSRegularExpression(pattern: "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)(?:$|[-+])"),
              let match = regex.firstMatch(in: value, range: NSRange(value.startIndex..., in: value)) else { return nil }
        var result: [Int] = []
        for index in 1...3 {
            guard let range = Range(match.range(at: index), in: value), let number = Int(value[range]) else { return nil }
            result.append(number)
        }
        parts = result
    }
    static func < (lhs: Self, rhs: Self) -> Bool { lhs.parts.lexicographicallyPrecedes(rhs.parts) }
}

struct MadeiraUpdateManifest: Decodable {
    let version: String
    let build: Int
    let commit: String
    let ipa: String
}

struct MadeiraRelease: Decodable {
    struct Asset: Decodable {
        let name: String
        let browser_download_url: String
        let size: Int64?
        let digest: String?
    }
    let tag_name: String
    let name: String?
    let draft: Bool
    let prerelease: Bool
    let assets: [Asset]
}

enum UpdateRules {
    static let repository = "sonnx0868/Madeira-Build79-Direct-Source"
    static let releasesURL = URL(string: "https://github.com/\(repository)/releases")!
    static func assetURL(_ value: String) -> URL? {
        guard let url = URL(string: value), url.scheme == "https", url.host == "github.com",
              url.user == nil, url.password == nil, url.port == nil,
              url.path.hasPrefix("/\(repository)/releases/download/"), url.fragment == nil else { return nil }
        return url
    }
    static func newer(version: String, build: Int?, commit: String?, installedVersion: String, installedBuild: Int, installedCommit: String) -> Bool {
        guard let remote = ReleaseVersion(version), let local = ReleaseVersion(installedVersion) else { return false }
        if remote != local { return remote > local }
        if let commit, !installedCommit.isEmpty, commit == installedCommit { return false }
        return (build ?? 0) > installedBuild
    }

    static func downloadRedirect(_ url: URL) -> Bool {
        guard url.scheme == "https", url.user == nil, url.password == nil,
              url.port == nil || url.port == 443 else { return false }
        return url.host == "github.com" || url.host == "release-assets.githubusercontent.com" || url.host == "objects.githubusercontent.com"
    }
}

enum AppUpdateDownload {
    static let maximumBytes: Int64 = 2 * 1024 * 1024 * 1024

    enum Failure: LocalizedError {
        case invalidFile, oversized, sizeMismatch, checksumMismatch, redirect, http(Int)
        var errorDescription: String? {
            switch self {
            case .invalidFile: return "The server did not return an IPA file. Retry the download."
            case .oversized: return "The update exceeds the supported download size."
            case .sizeMismatch: return "The IPA download is incomplete. Retry the download."
            case .checksumMismatch: return "The IPA failed its SHA-256 check. Retry the download."
            case .redirect: return "GitHub redirected the download to an unsupported address."
            case .http(let code): return "The IPA server returned HTTP \(code). Retry the download."
            }
        }
    }

    private final class ProgressDelegate: NSObject, URLSessionDownloadDelegate, @unchecked Sendable {
        let expectedBytes: Int64?
        let progress: @Sendable (Int64, Int64?) -> Void
        private let lock = NSLock()
        private var storedFailure: Error?
        private var continuation: CheckedContinuation<(URL, URLResponse), Error>?
        private var task: URLSessionDownloadTask?
        private var stagedFile: URL?
        private var cancelled = false
        private var lastUpdate = Date.distantPast
        init(expectedBytes: Int64?, progress: @escaping @Sendable (Int64, Int64?) -> Void) {
            self.expectedBytes = expectedBytes; self.progress = progress
        }
        private func fail(_ failure: Error, task: URLSessionTask) {
            lock.lock(); storedFailure = failure; lock.unlock()
            task.cancel()
        }
        func start(session: URLSession, request: URLRequest) async throws -> (URL, URLResponse) {
            try await withCheckedThrowingContinuation { continuation in
                lock.lock()
                guard !cancelled else { lock.unlock(); continuation.resume(throwing: CancellationError()); return }
                let task = session.downloadTask(with: request)
                self.continuation = continuation; self.task = task
                lock.unlock()
                task.resume()
            }
        }
        func cancel() {
            lock.lock(); cancelled = true; let task = task; lock.unlock()
            task?.cancel()
        }
        func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {
            // URLSession deletes its file when this callback returns. Retain it
            // before resuming the async caller; validation runs off the UI thread.
            let staged = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString + ".ipa-download")
            do {
                try FileManager.default.moveItem(at: location, to: staged)
                lock.lock(); stagedFile = staged; lock.unlock()
            } catch { fail(error, task: downloadTask) }
        }
        func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: Error?) {
            lock.lock()
            let continuation = continuation; self.continuation = nil; self.task = nil
            let file = stagedFile; stagedFile = nil
            let failure = storedFailure ?? error ?? (cancelled ? CancellationError() : nil)
            lock.unlock()
            if let failure {
                if let file { try? FileManager.default.removeItem(at: file) }
                continuation?.resume(throwing: failure)
            } else if let file, let response = task.response {
                continuation?.resume(returning: (file, response))
            } else {
                if let file { try? FileManager.default.removeItem(at: file) }
                continuation?.resume(throwing: Failure.invalidFile)
            }
        }
        func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten: Int64,
                        totalBytesWritten: Int64, totalBytesExpectedToWrite: Int64) {
            if totalBytesWritten > AppUpdateDownload.maximumBytes || totalBytesExpectedToWrite > AppUpdateDownload.maximumBytes {
                fail(Failure.oversized, task: downloadTask); return
            }
            if let expectedBytes, totalBytesWritten > expectedBytes {
                fail(Failure.sizeMismatch, task: downloadTask); return
            }
            let now = Date()
            guard now.timeIntervalSince(lastUpdate) >= 0.2 else { return }
            lastUpdate = now
            progress(totalBytesWritten, expectedBytes ?? (totalBytesExpectedToWrite > 0 ? totalBytesExpectedToWrite : nil))
        }
        func urlSession(_ session: URLSession, task: URLSessionTask, willPerformHTTPRedirection response: HTTPURLResponse,
                        newRequest request: URLRequest, completionHandler: @escaping (URLRequest?) -> Void) {
            guard let url = request.url, UpdateRules.downloadRedirect(url) else {
                fail(Failure.redirect, task: task); completionHandler(nil); return
            }
            completionHandler(request)
        }
    }

    // The app passes a release asset URL validated by UpdateRules.assetURL.
    // Keep the transfer independent of the view so dismissal never discards the file.
    static func fetch(from url: URL, expectedBytes: Int64?, digest: String?, directory: URL,
                      progress: @escaping @Sendable (Int64, Int64?) -> Void) async throws -> URL {
        guard url.pathExtension.lowercased() == "ipa" else { throw Failure.invalidFile }
        if let expectedBytes, expectedBytes <= 0 || expectedBytes > maximumBytes { throw Failure.oversized }
        let delegate = ProgressDelegate(expectedBytes: expectedBytes, progress: progress)
        let configuration = URLSessionConfiguration.ephemeral
        configuration.timeoutIntervalForRequest = 60
        configuration.timeoutIntervalForResource = 1800
        let session = URLSession(configuration: configuration, delegate: delegate, delegateQueue: nil)
        defer { session.invalidateAndCancel() }
        var request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        request.setValue("Madeira-IPA-Download", forHTTPHeaderField: "User-Agent")
        let (temporary, response) = try await withTaskCancellationHandler {
            try await delegate.start(session: session, request: request)
        } onCancel: { delegate.cancel() }
        defer { try? FileManager.default.removeItem(at: temporary) }
        try Task.checkCancellation()
        guard let http = response as? HTTPURLResponse else { throw Failure.invalidFile }
        guard http.statusCode == 200 else { throw Failure.http(http.statusCode) }
        let bytes = (try temporary.resourceValues(forKeys: [.fileSizeKey])).fileSize ?? 0
        guard bytes > 4 else { throw Failure.invalidFile }
        guard bytes <= maximumBytes else { throw Failure.oversized }
        if let expectedBytes, Int64(bytes) != expectedBytes { throw Failure.sizeMismatch }
        let file = try FileHandle(forReadingFrom: temporary)
        defer { try? file.close() }
        guard try file.read(upToCount: 4) == Data([0x50, 0x4b, 0x03, 0x04]) else { throw Failure.invalidFile }
        try file.seek(toOffset: 0)
        var hash = SHA256()
        while let block = try file.read(upToCount: 1024 * 1024), !block.isEmpty {
            try Task.checkCancellation()
            hash.update(data: block)
        }
        let checksum = hash.finalize().map { String(format: "%02x", $0) }.joined()
        if let digest, digest.lowercased() != "sha256:" + checksum { throw Failure.checksumMismatch }
        try Task.checkCancellation()
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        let destination = directory.appendingPathComponent(url.lastPathComponent)
        // Only replace an earlier download after the new file has passed every check.
        if FileManager.default.fileExists(atPath: destination.path) {
            _ = try FileManager.default.replaceItemAt(destination, withItemAt: temporary)
        } else {
            try FileManager.default.moveItem(at: temporary, to: destination)
        }
        progress(Int64(bytes), Int64(bytes))
        return destination
    }
}

@MainActor final class AppUpdateModel: ObservableObject {
    static let shared = AppUpdateModel()
    struct Available { let title: String; let url: URL; let bytes: Int64?; let digest: String? }
    @Published var busy = false
    @Published var status: String?
    @Published var available: Available?
    @Published private(set) var downloading = false
    @Published private(set) var receivedBytes: Int64 = 0
    @Published private(set) var totalBytes: Int64?
    @Published private(set) var downloadedIPA: URL?
    private var downloadTask: Task<Void, Never>?
    private let directory: URL?
    init(directory: URL? = nil) { self.directory = directory }

    func download() {
        guard !busy, !downloading, let available else { return }
        downloading = true; receivedBytes = 0; totalBytes = available.bytes; status = nil
        downloadTask = Task { [self] in
            defer { downloading = false; downloadTask = nil }
            do {
                let folder = try directory ?? FileManager.default.url(for: .documentDirectory, in: .userDomainMask,
                                                                     appropriateFor: nil, create: true).appendingPathComponent("Updates", isDirectory: true)
                let file = try await AppUpdateDownload.fetch(from: available.url, expectedBytes: available.bytes,
                                                            digest: available.digest, directory: folder) { [weak self] received, total in
                    Task { @MainActor in
                        guard let self, self.downloading else { return }
                        self.receivedBytes = max(self.receivedBytes, received); self.totalBytes = total
                    }
                }
                downloadedIPA = file
                status = "IPA saved in Files → Madeira → Updates. Share it with your sideloading tool to install."
            } catch {
                if Task.isCancelled { status = "Download cancelled. You can retry." }
                else { status = "IPA download failed: \(error.localizedDescription)" }
            }
        }
    }

    func cancelDownload() { downloadTask?.cancel() }

    func check(includeTestBuilds: Bool) async {
        guard !busy, !downloading else { return }
        busy = true; status = nil; available = nil
        defer { busy = false }
        do {
            var request = URLRequest(url: URL(string: "https://api.github.com/repos/\(UpdateRules.repository)/releases?per_page=30")!)
            request.setValue("application/vnd.github+json", forHTTPHeaderField: "Accept")
            request.setValue("Madeira-Update-Check", forHTTPHeaderField: "User-Agent")
            request.timeoutInterval = 30; request.cachePolicy = .reloadIgnoringLocalCacheData
            let (data, response) = try await URLSession.shared.data(for: request)
            guard let http = response as? HTTPURLResponse, http.statusCode == 200 else {
                throw SupportError.message("GitHub could not check releases. Try again later or open the release page.")
            }
            let releases = try JSONDecoder().decode([MadeiraRelease].self, from: data)
            let info = Bundle.main.infoDictionary ?? [:]
            let version = info["CFBundleShortVersionString"] as? String ?? "0.0.0"
            let build = Int(info["CFBundleVersion"] as? String ?? "0") ?? 0
            let commit = info["MadeiraSourceCommit"] as? String ?? ""
            for release in releases where !release.draft && (includeTestBuilds || !release.prerelease) {
                guard let firstIPA = release.assets.first(where: { $0.name.lowercased().hasSuffix(".ipa") }),
                      var ipaURL = UpdateRules.assetURL(firstIPA.browser_download_url) else { continue }
                var remoteVersion = release.tag_name
                var remoteBuild: Int?
                var remoteCommit: String?
                var selectedIPA = firstIPA
                if let asset = release.assets.first(where: { $0.name == "madeira-update.json" }),
                   let url = UpdateRules.assetURL(asset.browser_download_url) {
                    let (manifestData, manifestResponse) = try await URLSession.shared.data(from: url)
                    guard (manifestResponse as? HTTPURLResponse)?.statusCode == 200 else {
                        throw SupportError.message("The release information could not be loaded. Please retry.")
                    }
                    let manifest = try JSONDecoder().decode(MadeiraUpdateManifest.self, from: manifestData)
                    guard let ipa = release.assets.first(where: { $0.name == manifest.ipa }),
                          let verifiedURL = UpdateRules.assetURL(ipa.browser_download_url), manifest.build > 0 else { continue }
                    ipaURL = verifiedURL; selectedIPA = ipa
                    remoteVersion = manifest.version; remoteBuild = manifest.build; remoteCommit = manifest.commit
                }
                if UpdateRules.newer(version: remoteVersion, build: remoteBuild, commit: remoteCommit,
                                     installedVersion: version, installedBuild: build, installedCommit: commit) {
                    available = Available(title: release.name ?? release.tag_name, url: ipaURL, bytes: selectedIPA.size, digest: selectedIPA.digest)
                    status = "A new Madeira build is available."
                    return
                }
            }
            status = "No newer Madeira build is available for this update channel."
        } catch { status = error.localizedDescription }
    }
}
