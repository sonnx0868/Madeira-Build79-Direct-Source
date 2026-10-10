import Foundation
import CryptoKit

/// Bundled tools use the normal launch gate and JIT path, in their own folder.
enum TranslationTools {
    static let launch = Notification.Name("Madeira.RunTranslationLab")
    static let relativeFolder = "Madeira/Tools/translation-lab-v1"
    static let reportName = "madeira-translation-lab.txt"
    static let files = ["madeira-translation-lab.exe", "madeira-native-work.dll"]

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
        let folder = drive.appendingPathComponent(relativeFolder, isDirectory: true)
        let resolvedDrive = drive.resolvingSymlinksInPath().standardizedFileURL.path + "/"
        guard folder.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(resolvedDrive) else {
            throw NSError(domain: "TranslationTools", code: 3, userInfo: [NSLocalizedDescriptionKey: "The tools folder must stay inside drive_c."])
        }
        try fm.createDirectory(at: folder, withIntermediateDirectories: true)
        for name in files {
            let target = folder.appendingPathComponent(name)
            guard target.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(resolvedDrive) else {
                throw NSError(domain: "TranslationTools", code: 3, userInfo: [NSLocalizedDescriptionKey: "The tool path leaves drive_c."])
            }
            try contents[name]!.write(to: target, options: .atomic)
        }
        return relativeFolder + "/madeira-translation-lab.exe"
    }

    static func latestReport(drive: URL) -> String? {
        let url = drive.appendingPathComponent(reportName)
        guard url.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(
            drive.resolvingSymlinksInPath().standardizedFileURL.path + "/") else { return nil }
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
    static func apply(mode: String?, budget: Int?, relativePath: String, drive: URL) {
        if let budget, budgets.contains(budget) { setenv("FEX_MAXINST", String(budget), 1) }
        else { unsetenv("FEX_MAXINST") }
        let choice = mode ?? MadeiraConfig.get("env.MADEIRA_CPU_CACHE") ?? "off"
        guard ["verify", "reuse"].contains(choice) else {
            setenv("MADEIRA_CPU_CACHE", "0", 1); unsetenv("MADEIRA_CPU_CACHE_PATH"); return
        }
        do {
            let folder = drive.appendingPathComponent("Madeira/Cache/cpu-v1", isDirectory: true)
            guard folder.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(
                drive.resolvingSymlinksInPath().standardizedFileURL.path + "/") else { throw CocoaError(.fileWriteInvalidFileName) }
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
            let build = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as? String ?? "local"
            let key = SHA256.hash(data: Data((build + "\0" + relativePath).utf8)).map { String(format: "%02x", $0) }.joined()
            let file = folder.appendingPathComponent(key + ".bin")
            guard file.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(folder.resolvingSymlinksInPath().standardizedFileURL.path + "/") else {
                throw CocoaError(.fileWriteInvalidFileName)
            }
            setenv("MADEIRA_CPU_CACHE", choice, 1)
            setenv("MADEIRA_CPU_CACHE_PATH", "C:\\Madeira\\Cache\\cpu-v1\\\(key).bin", 1)
            LogStore.shared.log("[cpu-profile] cache=\(choice) instructionBudget=\(budget.map(String.init) ?? "default")")
        } catch {
            setenv("MADEIRA_CPU_CACHE", "0", 1); unsetenv("MADEIRA_CPU_CACHE_PATH")
            LogStore.shared.log("[cpu-profile] cache unavailable: \(error.localizedDescription)")
        }
    }
}
