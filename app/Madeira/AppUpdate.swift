import Foundation
import Combine

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
    struct Asset: Decodable { let name: String; let browser_download_url: String }
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
}

@MainActor final class AppUpdateModel: ObservableObject {
    struct Available { let title: String; let url: URL }
    @Published var busy = false
    @Published var status: String?
    @Published var available: Available?

    func check(includeTestBuilds: Bool) async {
        guard !busy else { return }
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
                if let asset = release.assets.first(where: { $0.name == "madeira-update.json" }),
                   let url = UpdateRules.assetURL(asset.browser_download_url) {
                    let (manifestData, manifestResponse) = try await URLSession.shared.data(from: url)
                    guard (manifestResponse as? HTTPURLResponse)?.statusCode == 200 else {
                        throw SupportError.message("The release information could not be loaded. Please retry.")
                    }
                    let manifest = try JSONDecoder().decode(MadeiraUpdateManifest.self, from: manifestData)
                    guard let ipa = release.assets.first(where: { $0.name == manifest.ipa }),
                          let verifiedURL = UpdateRules.assetURL(ipa.browser_download_url), manifest.build > 0 else { continue }
                    ipaURL = verifiedURL; remoteVersion = manifest.version; remoteBuild = manifest.build; remoteCommit = manifest.commit
                }
                if UpdateRules.newer(version: remoteVersion, build: remoteBuild, commit: remoteCommit,
                                     installedVersion: version, installedBuild: build, installedCommit: commit) {
                    available = Available(title: release.name ?? release.tag_name, url: ipaURL)
                    status = "A new Madeira build is available."
                    return
                }
            }
            status = "No newer Madeira build is available for this update channel."
        } catch { status = error.localizedDescription }
    }
}
