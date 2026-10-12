import Foundation
import CryptoKit

/// Bundled tools use the normal launch gate and JIT path, in their own folder.
enum TranslationTools {
    static let launch = Notification.Name("Madeira.RunTranslationLab")
    static let relativeFolder = "Madeira/Tools/translation-lab-v1"
    static let reportName = "madeira-translation-lab.txt"
    static let files = ["madeira-translation-lab.exe", "madeira-native-work.dll"]

    /// URL.resolvingSymlinksInPath does not reliably resolve ancestors when
    /// trailing components do not exist. Inspect every existing component
    /// before creating our tool/cache folders; their own paths never use links.
    static func checkedPath(_ relative: String, drive: URL) throws -> URL {
        var current = drive.resolvingSymlinksInPath().standardizedFileURL
        for part in relative.split(separator: "/") {
            guard part != ".", part != ".." else { throw CocoaError(.fileWriteInvalidFileName) }
            current.appendPathComponent(String(part))
            if (try? FileManager.default.attributesOfItem(atPath: current.path)[.type]) as? FileAttributeType == .typeSymbolicLink {
                throw NSError(domain: "TranslationTools", code: 3, userInfo: [NSLocalizedDescriptionKey: "The Madeira tools or cache folder contains a symbolic link."])
            }
        }
        return current
    }

    static func stage(resources: URL, drive: URL) throws -> String {
        let fm = FileManager.default
        let receipt = try JSONSerialization.jsonObject(with: Data(contentsOf: resources.appendingPathComponent("receipt.json")))
        guard let manifest = receipt as? [String: [String: Any]] else {
            throw NSError(domain: "TranslationTools", code: 1, userInfo: [NSLocalizedDescriptionKey: "The CPU comparison tools are missing from this build."])
        }
        // Validate before replacing any staged tool; never accept a download URL
        // or a filename from the receipt as a filesystem destination.
        var contents: [String: Data] = [:]
        for name in files {
            let data = try Data(contentsOf: resources.appendingPathComponent(name))
            let digest = SHA256.hash(data: data).map { String(format: "%02x", $0) }.joined()
            guard data.count <= 2 * 1024 * 1024, manifest[name]?["sha256"] as? String == digest else {
                throw NSError(domain: "TranslationTools", code: 2, userInfo: [NSLocalizedDescriptionKey: "The bundled CPU comparison tools failed validation."])
            }
            contents[name] = data
        }
        let folder = try checkedPath(relativeFolder, drive: drive)
        try fm.createDirectory(at: folder, withIntermediateDirectories: true)
        for name in files {
            let target = try checkedPath(relativeFolder + "/" + name, drive: drive)
            try contents[name]!.write(to: target, options: .atomic)
        }
        return relativeFolder + "/madeira-translation-lab.exe"
    }

    static func latestReport(drive: URL) -> String? {
        guard let url = try? checkedPath(reportName, drive: drive) else { return nil }
        guard let attributes = try? FileManager.default.attributesOfItem(atPath: url.path),
              ((attributes[.size] as? NSNumber)?.intValue ?? Int.max) <= 8192,
              let data = try? Data(contentsOf: url), let text = String(data: data, encoding: .utf8),
              text.contains("[translation-lab]"), text.contains("checksums=equal") else { return nil }
        let date = (attributes[.modificationDate] as? Date)?.formatted(date: .abbreviated, time: .shortened) ?? ""
        return "Last result: \(date)\n\(text)"
    }
}

enum CPUTranslationSettings {
    static let budgets = [512, 2048, 5000]
    static func nativeIdentity(file: URL) -> String? {
        guard let handle = try? FileHandle(forReadingFrom: file) else { return nil }
        defer { try? handle.close() }
        guard let size = try? handle.seekToEnd(), size > 0, size <= 64 * 1024 * 1024,
              (try? handle.seek(toOffset: 0)) != nil else { return nil }
        var hasher = SHA256(), count: UInt64 = 0
        do {
            while let part = try handle.read(upToCount: 256 * 1024), !part.isEmpty {
                count += UInt64(part.count)
                guard count <= size else { return nil }
                hasher.update(data: part)
            }
        } catch { return nil }
        guard count == size else { return nil }
        return hasher.finalize().map { String(format: "%02x", $0) }.joined()
    }
    static let translatorIdentity: String = {
        if let file = Bundle.main.url(forResource: "xtajit64", withExtension: "dll", subdirectory: "arm64ec-windows"),
           let hash = nativeIdentity(file: file) { return "xtajit64-sha256:" + hash }
        // Never share across builds when the translator can't be identified.
        let build = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as? String ?? "local"
        return "unidentified-build:" + build
    }()
    static func fileKey(relativePath: String, identity: String = translatorIdentity) -> String {
        // App-only updates don't discard verified native code. The backend
        // additionally verifies compiler/host/config/IR and runtime targets.
        return SHA256.hash(data: Data((identity + "\0" + relativePath).utf8)).map { String(format: "%02x", $0) }.joined()
    }
    static func clear(relativePath: String, drive: URL) throws {
        let relative = "Madeira/Cache/cpu-v1/" + fileKey(relativePath: relativePath) + ".bin"
        for suffix in ["", ".disabled"] {
            let file = try TranslationTools.checkedPath(relative + suffix, drive: drive)
            if FileManager.default.fileExists(atPath: file.path) { try FileManager.default.removeItem(at: file) }
        }
    }
    static func apply(mode: String?, budget: Int?, relativePath: String, drive: URL) {
        if let budget, budgets.contains(budget) { setenv("FEX_MAXINST", String(budget), 1) }
        else { unsetenv("FEX_MAXINST") }
        let choice = mode ?? MadeiraConfig.get("env.MADEIRA_CPU_CACHE") ?? "off"
        guard ["verify", "reuse"].contains(choice) else {
            setenv("MADEIRA_CPU_CACHE", "0", 1); unsetenv("MADEIRA_CPU_CACHE_PATH"); return
        }
        do {
            let folder = try TranslationTools.checkedPath("Madeira/Cache/cpu-v1", drive: drive)
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
            let key = fileKey(relativePath: relativePath)
            _ = try TranslationTools.checkedPath("Madeira/Cache/cpu-v1/" + key + ".bin", drive: drive)
            _ = try TranslationTools.checkedPath("Madeira/Cache/cpu-v1/" + key + ".bin.disabled", drive: drive)
            setenv("MADEIRA_CPU_CACHE", choice, 1)
            setenv("MADEIRA_CPU_CACHE_PATH", "C:\\Madeira\\Cache\\cpu-v1\\\(key).bin", 1)
            LogStore.shared.log("[cpu-profile] cache=\(choice) instructionBudget=\(budget.map(String.init) ?? "default") namespace=translator-v1")
        } catch {
            setenv("MADEIRA_CPU_CACHE", "0", 1); unsetenv("MADEIRA_CPU_CACHE_PATH")
            LogStore.shared.log("[cpu-profile] cache unavailable: \(error.localizedDescription)")
        }
    }
}

enum ShaderPreparationSettings {
    static func enabled(mode: String?, global: String?, automatic: Bool) -> Bool {
        if let mode { return mode == "on" }
        if let global { return global == "1" }
        return automatic
    }
    static func apply(mode: String?, automatic: Bool, relativePath: String, identity: String, drive: URL) {
        let on = enabled(mode: mode, global: MadeiraConfig.get("env.MADEIRA_SHADER_WARMUP"), automatic: automatic)
        setenv("MADEIRA_SHADER_WARMUP", on ? "1" : "0", 1)
        setenv("_MADEIRA_SHADER_WARMUP_PROFILE", on ? "1" : "0", 1)
        // Stable across app builds, isolated by game. An updated executable
        // starts a new recipe inventory; shader blobs also have content hashes.
        let file = try? TranslationTools.checkedPath(relativePath, drive: drive)
        let attrs = file.flatMap { try? FileManager.default.attributesOfItem(atPath: $0.path) }
        let size = (attrs?[.size] as? NSNumber)?.stringValue ?? "unknown"
        let modified = (attrs?[.modificationDate] as? Date)?.timeIntervalSince1970 ?? 0
        let stamp = "graphics-v1\0\(identity)\0\(relativePath)\0\(size)\0\(modified)"
        let key = SHA256.hash(data: Data(stamp.utf8)).map { String(format: "%02x", $0) }.joined()
        if let configured = MadeiraConfig.get("env.MADEIRA_PIPELINE_CACHE_DIR"), !configured.isEmpty {
            setenv("MADEIRA_PIPELINE_CACHE_DIR", configured, 1)
            setenv("_MADEIRA_PIPELINE_CACHE_DIR_PROFILE", configured, 1)
        } else if let base = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask).first {
            let directory = base.appendingPathComponent("madeira-game-graphics/" + key, isDirectory: true).path
            setenv("MADEIRA_PIPELINE_CACHE_DIR", directory, 1)
            setenv("_MADEIRA_PIPELINE_CACHE_DIR_PROFILE", directory, 1)
        } else { unsetenv("MADEIRA_PIPELINE_CACHE_DIR"); unsetenv("_MADEIRA_PIPELINE_CACHE_DIR_PROFILE") }
        LogStore.shared.log("[shader-profile] replay=\(on ? 1 : 0) inventory=game-v1")
    }
}
