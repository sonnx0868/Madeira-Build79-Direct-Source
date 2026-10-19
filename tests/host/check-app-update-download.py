#!/usr/bin/env python3
"""Run the shipping Foundation downloader against an actual HTTP fixture on macOS."""
from pathlib import Path
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import io
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import zipfile

root = Path(__file__).resolve().parents[2]
swift = shutil.which("swiftc")
if not swift:
    if "--require-swift" in sys.argv:
        raise SystemExit("swiftc is required for the IPA download integration tests")
    print("SKIP: IPA download integration tests require macOS Swift")
    raise SystemExit(0)

payload = io.BytesIO()
with zipfile.ZipFile(payload, "w") as archive:
    archive.writestr("Payload/Madeira.app/Info.plist", b"fixture" + b"x" * (2 * 1024 * 1024))
ipa = payload.getvalue()
digest = "sha256:" + hashlib.sha256(ipa).hexdigest()


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def do_GET(self):
        if self.path == "/redirect.ipa":
            self.send_response(302)
            self.send_header("Location", f"http://127.0.0.1:{self.server.server_port}/good.ipa")
            self.end_headers()
            return
        body = b"<html>Sign in</html>" if self.path == "/html.ipa" else ipa
        self.send_response(503 if self.path == "/error.ipa" else 200)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        try:
            for offset in range(0, len(body), 32768):
                self.wfile.write(body[offset:offset + 32768])
                self.wfile.flush()
                if self.path == "/slow.ipa":
                    time.sleep(0.03)
        except (BrokenPipeError, ConnectionResetError):
            pass


checks = r'''
enum SupportError: LocalizedError {
    case message(String)
    var errorDescription: String? { if case .message(let text) = self { return text }; return nil }
}
final class ProgressCounter: @unchecked Sendable {
    private let lock = NSLock()
    private var received: Int64 = 0
    func update(_ bytes: Int64, _ total: Int64?) { lock.lock(); defer { lock.unlock() }; received = max(received, bytes) }
    var bytes: Int64 { lock.lock(); defer { lock.unlock() }; return received }
}
@main struct Check {
    static func read(_ url: URL) -> Data { try! Data(contentsOf: url) }
    @MainActor static func main() async throws {
        let base = URL(string: CommandLine.arguments[1])!
        let folder = URL(fileURLWithPath: CommandLine.arguments[2], isDirectory: true)
        let size = Int64(CommandLine.arguments[3])!
        let digest = CommandLine.arguments[4]
        let progress = ProgressCounter()
        for value in ["https://github.com/a", "https://release-assets.githubusercontent.com/a?signature=redacted", "https://objects.githubusercontent.com/a"] {
            assert(UpdateRules.downloadRedirect(URL(string: value)!))
        }
        for value in ["http://github.com/a", "https://github.com.evil.example/a", "https://evil.example/a", "https://u:p@github.com/a", "https://github.com:444/a"] {
            assert(!UpdateRules.downloadRedirect(URL(string: value)!))
        }
        let oldAPI = Data(#"{"name":"a.ipa","browser_download_url":"https://github.com/a"}"#.utf8)
        let legacyAsset = try JSONDecoder().decode(MadeiraRelease.Asset.self, from: oldAPI)
        assert(legacyAsset.digest == nil)
        let file = try await AppUpdateDownload.fetch(from: base.appendingPathComponent("good.ipa"), expectedBytes: size,
                                                   digest: digest, directory: folder, progress: progress.update)
        assert(progress.bytes == size && file.pathExtension == "ipa")
        let original = try Data(contentsOf: file)
        assert(original.count == size)
        // A repeat download replaces the complete file, never exposing a partial IPA.
        let repeatFile = try await AppUpdateDownload.fetch(from: base.appendingPathComponent("good.ipa"), expectedBytes: size,
                                                          digest: digest, directory: folder, progress: { _, _ in })
        assert(repeatFile == file && read(file) == original)
        for (path, expected, checksum, kind) in [
            ("html.ipa", nil as Int64?, nil as String?, "file"),
            ("error.ipa", nil, nil, "http"),
            ("good.ipa", size + 1, digest, "size"),
            ("good.ipa", 16, digest, "size"),
            ("good.ipa", size, "sha256:bad", "hash"),
            ("good.ipa", AppUpdateDownload.maximumBytes + 1, nil, "limit"),
            ("redirect.ipa", nil, nil, "redirect")
        ] {
            do {
                _ = try await AppUpdateDownload.fetch(from: base.appendingPathComponent(path), expectedBytes: expected,
                                                     digest: checksum, directory: folder, progress: { _, _ in })
                fatalError("accepted invalid response: \(kind)")
            } catch let failure as AppUpdateDownload.Failure {
                switch (kind, failure) {
                case ("file", .invalidFile), ("http", .http(503)), ("size", .sizeMismatch),
                     ("hash", .checksumMismatch), ("limit", .oversized), ("redirect", .redirect): break
                default: fatalError("unexpected failure: \(kind): \(failure)")
                }
            }
            assert(read(file) == original, "failed download damaged the earlier IPA")
        }
        let cancelledProgress = ProgressCounter()
        let cancelled = Task {
            try await AppUpdateDownload.fetch(from: base.appendingPathComponent("slow.ipa"), expectedBytes: size,
                                              digest: digest, directory: folder, progress: cancelledProgress.update)
        }
        while cancelledProgress.bytes == 0 { try await Task.sleep(nanoseconds: 20_000_000) }
        cancelled.cancel()
        do { _ = try await cancelled.value; fatalError("cancel ignored") }
        catch is CancellationError {}
        catch let error as URLError { assert(error.code == .cancelled) }
        assert(!FileManager.default.fileExists(atPath: folder.appendingPathComponent("slow.ipa").path))
        assert(read(file) == original)
        // Exercise the production observable model, including cancellation and retry.
        let model = AppUpdateModel(directory: folder.appendingPathComponent("model"))
        model.available = .init(title: "Fixture", url: base.appendingPathComponent("slow.ipa"), bytes: size, digest: digest)
        model.download()
        while model.receivedBytes == 0 { try await Task.sleep(nanoseconds: 20_000_000) }
        model.cancelDownload()
        while model.downloading { try await Task.sleep(nanoseconds: 20_000_000) }
        assert(model.downloadedIPA == nil && model.status?.contains("cancelled") == true)
        model.available = .init(title: "Fixture", url: base.appendingPathComponent("good.ipa"), bytes: size, digest: digest)
        model.download()
        while model.downloading { try await Task.sleep(nanoseconds: 20_000_000) }
        assert(model.downloadedIPA?.lastPathComponent == "good.ipa", model.status ?? "no status")
        assert(read(model.downloadedIPA!) == original)
        try FileManager.default.removeItem(at: model.downloadedIPA!)
        if CommandLine.arguments.contains("--live-asset") {
            let url = URL(string: "https://github.com/sonnx0868/Madeira-Build79-Direct-Source/releases/download/v0.1.6-build794-9e615ce/Madeira-v0.1.6-build794.ipa")!
            assert(UpdateRules.assetURL(url.absoluteString) != nil)
            let live = try await AppUpdateDownload.fetch(from: url, expectedBytes: 257060622,
                digest: "sha256:0d86083051343fac8b5976c4df8d7011d3b7a233ad5de183c2839b8820679669", directory: folder, progress: { _, _ in })
            try FileManager.default.removeItem(at: live)
            print("PASS: real GitHub → CDN transfer downloaded and SHA-256 verified the public 257 MB IPA with the production downloader")
        }
        print("PASS: IPA streaming/progress, repeat replacement, HTML/HTTP rejection, short/oversized data, checksum, redirect policy, cancellation and model retry preserve prior downloads")
    }
}
'''

server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
threading.Thread(target=server.serve_forever, daemon=True).start()
try:
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory) / "main.swift"
        source.write_text((root / "app/Madeira/AppUpdate.swift").read_text(encoding="utf-8") + checks, encoding="utf-8")
        binary = Path(directory) / "check"
        subprocess.run([swift, "-parse-as-library", str(source), "-o", str(binary)], check=True)
        command = [str(binary), f"http://127.0.0.1:{server.server_port}", str(Path(directory) / "downloads"), str(len(ipa)), digest]
        if "--live-asset" in sys.argv:
            command.append("--live-asset")
        subprocess.run(command, check=True, timeout=240)
finally:
    server.shutdown()
