// GPL-3.0-or-later WITH the Madeira Converter Exception.
// GOG protocol references: Heroic Games Launcher / heroic-gogdl (auth.py, api.py).
import Foundation
import SwiftUI
import Security
import WebKit
import CryptoKit

enum GOGError: LocalizedError {
    case message(String)
    var errorDescription: String? { if case .message(let text) = self { return text }; return nil }
}
struct GOGGame: Identifiable, Codable {
    var id: Int
    var title: String
    var image: URL?
}
struct GOGInstallerFile: Identifiable {
    var id: String
    var size: Int64
    var downlink: URL
    var md5: String?
}
enum GOGRules {
    static func official(_ url: URL) -> Bool {
        url.scheme == "https" && ["api.gog.com", "embed.gog.com", "auth.gog.com"].contains(url.host?.lowercased() ?? "")
    }
    static func products(_ data: Data) throws -> ([GOGGame], Int) {
        guard data.count <= 16 * 1024 * 1024,
              let json = try JSONSerialization.jsonObject(with: data) as? [String: Any],
              let products = json["products"] as? [[String: Any]], products.count <= 1000 else {
            throw GOGError.message("GOG returned an invalid library.")
        }
        let games = products.compactMap { item -> GOGGame? in
            guard let id = item["id"] as? Int, id > 0, let title = item["title"] as? String else { return nil }
            if let platforms = item["worksOn"] as? [String: Bool], platforms["Windows"] == false { return nil }
            let image = (item["image"] as? String).flatMap { raw -> URL? in
                let url = URL(string: raw.hasPrefix("//") ? "https:" + raw : raw)
                return url?.scheme == "https" ? url : nil
            }
            return GOGGame(id: id, title: String(title.prefix(300)), image: image)
        }
        let pages = max(1, min(100, json["totalPages"] as? Int ?? 1))
        return (games, pages)
    }
    static func installers(_ data: Data) throws -> [GOGInstallerFile] {
        guard data.count <= 16 * 1024 * 1024,
              let json = try JSONSerialization.jsonObject(with: data) as? [String: Any],
              let downloads = json["downloads"] as? [String: Any],
              let installers = downloads["installers"] as? [[String: Any]] else { throw GOGError.message("GOG returned no installer information.") }
        let windows = installers.filter { ($0["os"] as? String)?.lowercased() == "windows" }
        guard let chosen = windows.first(where: { ($0["language"] as? String) == "en" }) ?? windows.first,
              let files = chosen["files"] as? [[String: Any]], !files.isEmpty, files.count <= 100 else {
            throw GOGError.message("This game has no supported Windows offline installer.")
        }
        return try files.enumerated().map { index, item in
            guard let raw = item["downlink"] as? String, let url = URL(string: raw), official(url),
                  let size = item["size"] as? NSNumber, size.int64Value > 0, size.int64Value <= 200 * 1024 * 1024 * 1024 else {
                throw GOGError.message("GOG returned an invalid installer file.")
            }
            return GOGInstallerFile(id: String(describing: item["id"] ?? index), size: size.int64Value,
                downlink: url, md5: item["md5"] as? String)
        }
    }
    static func safe(_ relative: String, under root: URL) throws -> URL {
        let parts = relative.replacingOccurrences(of: "\\", with: "/").split(separator: "/", omittingEmptySubsequences: false)
        guard !parts.isEmpty, parts.allSatisfy({ !$0.isEmpty && $0 != "." && $0 != ".." && !$0.contains(":") && !$0.contains("\0") }) else {
            throw GOGError.message("The GOG file path is invalid.")
        }
        var result = root.resolvingSymlinksInPath()
        for part in parts {
            result.appendPathComponent(String(part))
            if (try? FileManager.default.attributesOfItem(atPath: result.path)[.type] as? FileAttributeType) == .typeSymbolicLink {
                throw GOGError.message("The GOG folder contains a symbolic link.")
            }
        }
        return result
    }
}

@MainActor final class GOGAuth: ObservableObject {
    static let shared = GOGAuth()
    private struct Tokens: Codable { var access, refresh: String; var expiry: Date; var userID: String }
    private var tokens: Tokens?
    private var epoch = 0
    @Published private(set) var signedIn = false
    @Published var busy = false
    @Published var error: String?
    private let client = "46899977096215655"
    private let secret = "9d85c43b1482497dbbce61f6e4aa173a433796eeae2ca8c5f6129f2dc4de46d9"
    private let redirect = "https://embed.gog.com/on_login_success?origin=client"
    private var query: [String: Any] { [kSecClass as String:kSecClassGenericPassword,
        kSecAttrService as String:"madeira.gog.tokens.v1",kSecAttrAccount as String:"gog"] }
    private init() {
        var q = query; q[kSecReturnData as String] = true; q[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        if SecItemCopyMatching(q as CFDictionary, &result) == errSecSuccess, let data = result as? Data {
            tokens = try? JSONDecoder().decode(Tokens.self, from: data); signedIn = tokens != nil
        }
    }
    var loginURL: URL {
        var url = URLComponents(string:"https://auth.gog.com/auth")!
        url.queryItems = [URLQueryItem(name:"client_id",value:client),URLQueryItem(name:"redirect_uri",value:redirect),
            URLQueryItem(name:"response_type",value:"code"),URLQueryItem(name:"layout",value:"client2")]
        return url.url!
    }
    func authorize(_ code: String) {
        guard !busy, !code.isEmpty, code.utf8.count < 4096 else { return }
        busy = true; error = nil
        let generation = epoch
        Task {
            defer { busy = false }
            do {
                let value = try await request(grant:"authorization_code",value:code)
                guard generation == epoch else { return }
                try save(value); signedIn = true
                await GOGLibrary.shared.refresh()
            } catch { self.error = "GOG sign-in failed. Check the connection and try again." }
        }
    }
    func accessToken() async throws -> String {
        guard let tokens else { throw GOGError.message("Sign in to GOG first.") }
        if tokens.expiry > Date().addingTimeInterval(60) { return tokens.access }
        let generation = epoch
        let renewed = try await request(grant:"refresh_token",value:tokens.refresh)
        guard generation == epoch else { throw GOGError.message("GOG was signed out.") }
        try save(renewed); return renewed.access
    }
    private func request(grant: String, value: String) async throws -> Tokens {
        var url = URLComponents(string:"https://auth.gog.com/token")!
        url.queryItems = [URLQueryItem(name:"client_id",value:client),URLQueryItem(name:"client_secret",value:secret),
            URLQueryItem(name:"grant_type",value:grant),URLQueryItem(name:"redirect_uri",value:redirect),
            URLQueryItem(name:grant == "refresh_token" ? "refresh_token" : "code",value:value)]
        var req = URLRequest(url:url.url!,cachePolicy:.reloadIgnoringLocalCacheData,timeoutInterval:30)
        req.setValue("Madeira",forHTTPHeaderField:"User-Agent")
        let (data,response) = try await URLSession.shared.data(for:req)
        guard (response as? HTTPURLResponse)?.statusCode == 200,
              let json = try JSONSerialization.jsonObject(with:data) as? [String:Any],
              let access = json["access_token"] as? String, let refresh = json["refresh_token"] as? String,
              !access.isEmpty, !refresh.isEmpty else { throw GOGError.message("GOG rejected the sign-in.") }
        return Tokens(access:access,refresh:refresh,expiry:Date().addingTimeInterval((json["expires_in"] as? Double) ?? 3600),
            userID:String(describing:json["user_id"] ?? ""))
    }
    private func save(_ value: Tokens) throws {
        let data = try JSONEncoder().encode(value)
        var q = query; q[kSecValueData as String] = data
        q[kSecAttrAccessible as String] = kSecAttrAccessibleWhenUnlockedThisDeviceOnly
        let status = SecItemUpdate(query as CFDictionary,[kSecValueData as String:data] as CFDictionary)
        guard status == errSecSuccess || (status == errSecItemNotFound && SecItemAdd(q as CFDictionary,nil) == errSecSuccess) else {
            throw GOGError.message("GOG sign-in could not be saved in the Keychain.")
        }
        tokens = value
    }
    func signOut() {
        epoch += 1; SecItemDelete(query as CFDictionary); tokens = nil; signedIn = false
        GOGLibrary.shared.clear()
    }
}

@MainActor final class GOGLibrary: ObservableObject {
    static let shared = GOGLibrary()
    @Published private(set) var games: [GOGGame] = []
    @Published private(set) var busy = false
    @Published var error: String?
    private var generation = 0
    func clear() { generation += 1; games = []; error = nil }
    static func json(_ url: URL) async throws -> Data {
        guard GOGRules.official(url) else { throw GOGError.message("Invalid GOG API address.") }
        var request = URLRequest(url:url,timeoutInterval:30)
        request.setValue("Bearer " + (try await GOGAuth.shared.accessToken()),forHTTPHeaderField:"Authorization")
        let (data,response) = try await URLSession.shared.data(for:request)
        guard (response as? HTTPURLResponse)?.statusCode == 200 else { throw GOGError.message("GOG could not complete the request. Sign in again.") }
        return data
    }
    func refresh() async {
        scanInstalled()
        guard GOGAuth.shared.signedIn, !busy else { return }
        busy = true; error = nil
        let token = generation
        defer { busy = false }
        do {
            var all: [Int:GOGGame] = [:], page = 1, pages = 1
            repeat {
                let url = URL(string:"https://embed.gog.com/account/products?mediaType=game&system=windows&sortBy=title&page=\(page)")!
                let (items,count) = try GOGRules.products(await Self.json(url))
                for game in items { all[game.id] = game }
                pages = count; page += 1
            } while page <= pages
            guard token == generation, GOGAuth.shared.signedIn else { return }
            games = all.values.sorted { $0.title.localizedStandardCompare($1.title) == .orderedAscending }
        } catch { self.error = "Could not load the GOG library. Check the connection or sign in again." }
    }
    func scanInstalled() {
        let base = LibraryModel.drive.appendingPathComponent("Program Files/GOG Games")
        let folders = (try? FileManager.default.contentsOfDirectory(at:base,includingPropertiesForKeys:nil)) ?? []
        for folder in folders.prefix(1000) {
            guard let id = Int(folder.lastPathComponent), id > 0,
                  let info = try? GOGRules.safe("goggame-\(id).info",under:folder),
                  let size = try? info.resourceValues(forKeys:[.fileSizeKey]).fileSize, size < 1024 * 1024,
                  let data = try? Data(contentsOf:info),
                  let json = try? JSONSerialization.jsonObject(with:data) as? [String:Any],
                  let tasks = json["playTasks"] as? [[String:Any]],
                  let task = tasks.first(where: { ($0["isPrimary"] as? Bool) == true && ($0["type"] as? String) == "FileTask" }),
                  let path = task["path"] as? String, path.lowercased().hasSuffix(".exe"),
                  let file = try? GOGRules.safe(path,under:folder), FileManager.default.fileExists(atPath:file.path) else { continue }
            var entry = LibraryModel.shared.entries.first { $0.gogID == id } ?? LibraryEntry(title:json["name"] as? String ?? "GOG game",relativePath:"",bits:0)
            entry.gogWorkingDir = "Program Files/GOG Games/\(id)"
            entry.gogID = id; entry.relativePath = "Program Files/GOG Games/\(id)/" + path.replacingOccurrences(of:"\\",with:"/")
            entry.gogLaunchCommand = task["arguments"] as? String ?? ""
            entry.gogArtworkURL = games.first { $0.id == id }?.image ?? entry.gogArtworkURL
            if let working = task["workingDir"] as? String, !working.isEmpty,
               let directory = try? GOGRules.safe(working,under:folder),
               directory.path.hasPrefix(folder.path + "/") {
                entry.gogWorkingDir = "Program Files/GOG Games/\(id)/" + working.replacingOccurrences(of:"\\",with:"/")
            }
            LibraryModel.shared.save(entry)
        }
    }
}

private final class GOGProgress: NSObject, URLSessionDownloadDelegate, @unchecked Sendable {
    let update: @Sendable (Double) -> Void
    private let lock = NSLock()
    private var last: TimeInterval = 0
    init(update: @escaping @Sendable (Double) -> Void) { self.update = update }
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {}
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten:Int64,
                    totalBytesWritten:Int64,totalBytesExpectedToWrite:Int64) {
        guard totalBytesExpectedToWrite > 0 else { return }
        lock.lock(); let now = Date.timeIntervalSinceReferenceDate; let publish = now - last >= 0.25
        if publish { last = now }; lock.unlock()
        if publish { update(min(1,Double(totalBytesWritten)/Double(totalBytesExpectedToWrite))) }
    }
}
@MainActor final class GOGDownloads: ObservableObject {
    static let shared = GOGDownloads()
    @Published var active: Int?
    @Published var fraction: Double = 0
    @Published var message = ""
    @Published var installers: [Int:URL] = [:]
    private var task: Task<Void,Never>?
    private var indexURL: URL { LibraryModel.drive.appendingPathComponent("gog-downloads/installer-index.json") }
    private init() {
        if let data = try? Data(contentsOf:indexURL), let names = try? JSONDecoder().decode([String:String].self,from:data) {
            for (id,path) in names {
                guard let id = Int(id), let url = try? GOGRules.safe(path,under:LibraryModel.drive),
                    FileManager.default.fileExists(atPath:url.path), url.pathExtension.lowercased() == "exe" else { continue }
                installers[id] = url
            }
        }
    }
    private func saveIndex() {
        let records = installers.reduce(into:[String:String]()) { result,item in
            if item.value.path.hasPrefix(LibraryModel.drive.path + "/") {
                result[String(item.key)] = String(item.value.path.dropFirst(LibraryModel.drive.path.count + 1))
            }
        }
        if let data = try? JSONEncoder().encode(records) { try? data.write(to:indexURL,options:.atomic) }
    }

    func cancel() { task?.cancel() }
    func download(_ game:GOGGame) {
        guard active == nil else { return }
        active = game.id; fraction = 0; message = "Fetching Windows installer…"
        task = Task {
            defer { active = nil; task = nil }
            do {
                let data = try await GOGLibrary.json(URL(string:"https://api.gog.com/products/\(game.id)?expand=downloads")!)
                let files = try GOGRules.installers(data)
                let folder = try GOGRules.safe("gog-downloads/\(game.id)",under:LibraryModel.drive)
                try FileManager.default.createDirectory(at:folder,withIntermediateDirectories:true)
                let required = files.reduce(Int64(0)) { $0 + $1.size }
                if let free = try? folder.resourceValues(forKeys:[.volumeAvailableCapacityForImportantUsageKey]).volumeAvailableCapacityForImportantUsage,
                   free < required + 128 * 1024 * 1024 { throw GOGError.message("Not enough free space for the GOG installer.") }
                var executable: URL?
                for (index,file) in files.enumerated() {
                    try Task.checkCancellation()
                    message = "Downloading installer file \(index + 1) of \(files.count)…"
                    let linkData = try await GOGLibrary.json(file.downlink)
                    guard let link = try JSONSerialization.jsonObject(with:linkData) as? [String:Any],
                          let raw = link["downlink"] as? String, let url = URL(string:raw), url.scheme == "https" else {
                        throw GOGError.message("GOG returned an invalid download address.")
                    }
                    let progress = GOGProgress { [weak self] part in
                        Task { @MainActor in self?.fraction = (Double(index) + part) / Double(files.count) }
                    }
                    let (temporary,response) = try await URLSession.shared.download(for:URLRequest(url:url,timeoutInterval:1800),delegate:progress)
                    defer { try? FileManager.default.removeItem(at:temporary) }
                    guard (response as? HTTPURLResponse)?.statusCode == 200,
                          let size = try temporary.resourceValues(forKeys:[.fileSizeKey]).fileSize, Int64(size) == file.size else {
                        throw GOGError.message("The GOG installer download is incomplete.")
                    }
                    let suggested = response.suggestedFilename ?? (index == 0 ? "setup-\(game.id).exe" : "setup-\(game.id)-\(index).bin")
                    guard suggested == URL(fileURLWithPath:suggested).lastPathComponent,
                          ["exe","bin"].contains(URL(fileURLWithPath:suggested).pathExtension.lowercased()) else {
                        throw GOGError.message("GOG returned an unsupported installer filename.")
                    }
                    if let expected = file.md5 {
                        let actual = try await Task.detached(priority:.utility) {
                            let stream = try FileHandle(forReadingFrom:temporary); defer { try? stream.close() }
                            var hash = Insecure.MD5()
                            while let bytes = try stream.read(upToCount:256 * 1024), !bytes.isEmpty { hash.update(data:bytes) }
                            return hash.finalize().map { String(format:"%02x",$0) }.joined()
                        }.value
                        guard actual.lowercased() == expected.lowercased() else { throw GOGError.message("The GOG installer checksum did not match.") }
                    }
                    let target = try GOGRules.safe(suggested,under:folder)
                    if FileManager.default.fileExists(atPath:target.path) { try FileManager.default.removeItem(at:target) }
                    try FileManager.default.moveItem(at:temporary,to:target)
                    if target.pathExtension.lowercased() == "exe" { executable = target }
                    fraction = Double(index + 1) / Double(files.count)
                }
                guard let executable else { throw GOGError.message("The GOG package contains no Windows installer.") }
                installers[game.id] = executable; saveIndex(); message = "Installer ready. Open it to install the game."
            } catch is CancellationError { message = "Download cancelled. Completed files stay on this device." }
            catch { message = (error as? GOGError)?.errorDescription ?? "GOG download failed. Check the connection and try again." }
        }
    }
    func installerEntry(_ game:GOGGame) -> LibraryEntry? {
        guard let file = installers[game.id], file.path.hasPrefix(LibraryModel.drive.path + "/") else { return nil }
        var entry = LibraryEntry(title:"Install " + game.title,relativePath:String(file.path.dropFirst(LibraryModel.drive.path.count + 1)),bits:0)
        entry.arguments = "/DIR=\"C:\\Program Files\\GOG Games\\\(game.id)\""
        entry.temporarySession = true; return entry
    }
}

struct GOGLoginWeb: UIViewRepresentable {
    var code: (String)->Void
    func makeCoordinator()->Coordinator { Coordinator(code:code) }
    func makeUIView(context:Context)->WKWebView {
        let web=WKWebView();web.navigationDelegate=context.coordinator;web.load(URLRequest(url:GOGAuth.shared.loginURL));return web
    }
    func updateUIView(_ view:WKWebView,context:Context) {}
    final class Coordinator:NSObject,WKNavigationDelegate {
        let code:(String)->Void;var captured=false
        init(code:@escaping(String)->Void) { self.code=code }
        func webView(_ webView:WKWebView,decidePolicyFor action:WKNavigationAction,decisionHandler:@escaping(WKNavigationActionPolicy)->Void) {
            if !captured, let url=action.request.url,url.scheme=="https",url.host=="embed.gog.com",url.path=="/on_login_success",
               let value=URLComponents(url:url,resolvingAgainstBaseURL:false)?.queryItems?.first(where:{$0.name=="code"})?.value,!value.isEmpty {
                captured=true;code(value);decisionHandler(.cancel)
            } else { decisionHandler(.allow) }
        }
    }
}
struct GOGSignInView:View {
    @ObservedObject var auth=GOGAuth.shared
    @Environment(\.dismiss) var dismiss
    var body:some View {
        NavigationStack {
            Group {
                if auth.signedIn { ContentUnavailableView("Signed in to GOG",systemImage:"checkmark.circle") }
                else if auth.busy { ProgressView("Signing in…") }
                else { GOGLoginWeb(code:{auth.authorize($0)}) }
            }.navigationTitle("GOG").toolbar { Button("Done") { dismiss() } }
                .alert("GOG",isPresented:Binding(get:{auth.error != nil},set:{if !$0 {auth.error=nil}})) { Button("OK") {auth.error=nil} } message:{Text(auth.error ?? "")}
        }
    }
}
struct GOGSettingsSection:View {
    var open:(SettingsSheet)->Void
    @ObservedObject var auth=GOGAuth.shared
    @State var confirm=false
    var body:some View {
        Section("GOG") {
            if auth.signedIn { Button("Sign out of GOG",role:.destructive) {confirm=true} }
            else { Button("Sign in to GOG") {open(.gogSignIn)} }
        }.confirmationDialog("Sign out of GOG?",isPresented:$confirm) { Button("Sign out",role:.destructive) {auth.signOut()} }
    }
}
struct GOGGamesSection:View {
    var search:String;var genre:UUID?;var open:(LibraryEntry)->Void
    @ObservedObject var library=GOGLibrary.shared
    @ObservedObject var auth=GOGAuth.shared
    @ObservedObject var model=LibraryModel.shared
    @State var selected:GOGGame?
    private var games:[GOGGame] {
        library.games.filter { game in
            guard search.isEmpty || game.title.localizedCaseInsensitiveContains(search) else {return false}
            if let entry=model.entries.first(where:{$0.gogID==game.id}) {
                return !model.isPrivate(entry) && LibraryVisibility.matches(entry.genreID,selection:genre,genres:model.genres)
            }
            return genre == nil || genre == LibraryGenre.uncategorized
        }
    }
    var body:some View {
        if auth.signedIn {
            VStack(alignment:.leading,spacing:12) {
                Text("GOG library").font(.headline)
                if library.busy {ProgressView()}
                if let error=library.error {Text(error).font(.footnote).foregroundStyle(.orange)}
                ForEach(games) { game in
                    Button {
                        if let entry=model.entries.first(where:{$0.gogID==game.id}) {open(entry)} else {selected=game}
                    } label: { Label(game.title,systemImage:"shippingbox").frame(maxWidth:.infinity,alignment:.leading).padding(10) }
                }
            }.task {await library.refresh()}
                .sheet(item:$selected) {GOGGameSheet(game:$0,open:open)}
        }
    }
}
struct GOGGameSheet:View {
    let game:GOGGame;var open:(LibraryEntry)->Void
    @ObservedObject var downloads=GOGDownloads.shared
    @Environment(\.dismiss) var dismiss
    var body:some View {
        NavigationStack {
            Form {
                Section(game.title) {
                    if downloads.active == game.id {
                        ProgressView(value:downloads.fraction)
                        Button("Cancel download",role:.destructive) {downloads.cancel()}
                    } else if let entry=downloads.installerEntry(game) {
                        Button("Open Windows installer") {dismiss();open(entry)}
                    } else { Button("Download Windows installer") {downloads.download(game)}.disabled(downloads.active != nil) }
                    Text(downloads.message).font(.footnote)
                }
                Section {
                    Button("Find installed game") {GOGLibrary.shared.scanInstalled();if let entry=LibraryModel.shared.entries.first(where:{$0.gogID==game.id}) {dismiss();open(entry)}}
                } footer:{Text("GOG downloads include the original Windows installer and all BIN parts. Install into the suggested folder, then find the installed game. Restart Madeira after running an installer.")}
            }.navigationTitle("GOG game").toolbar {Button("Done") {dismiss()}}
        }
    }
}
