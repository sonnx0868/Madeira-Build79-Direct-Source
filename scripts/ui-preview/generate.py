#!/usr/bin/env python3
"""Run production SwiftUI shelf fragments with fixture data, without Wine/JIT."""
from pathlib import Path
import hashlib
import json
import shutil
import sys

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
library = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
access = (root / 'app/Madeira/LibraryAccess.swift').read_text(encoding='utf-8')


def block(source, signature):
    start = source.index(signature)
    pos = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos] + '\n'


sources = 'import SwiftUI\nimport UIKit\nimport CoreText\nimport ImageIO\nimport Foundation\n'
for sig in ('enum LibraryTheme', 'struct LibraryScenicBackground', 'struct LibraryAccessView'):
    sources += block(access, sig)
for sig in ('final class LibraryHeaderAlignment', 'final class ReportingSearchController',
            'final class ReportingSearchBar', 'struct LibraryInlineSearch', 'struct LibraryNavSearch',
            'actor LibraryArtworkStore', 'struct LibraryArtwork', 'struct LibraryCells'):
    sources += block(library, sig)
sources += (root / 'app/Madeira/LibraryGrouping.swift').read_text(encoding='utf-8')

sources += r'''
struct LibraryEntry: Identifiable, Hashable {
    var id = UUID()
    var title: String
    var relativePath = ""
    var bits = 64
    var desktop: Bool?
    var steamAppID: Int?
    var steamID: Int?
    var genreID: UUID?
    var privateGame: Bool?
    var lastPlayed: Date?
    var folderBytes: Int64?
    var graphicsAPI: String?
    var coverFile: String?
    var epicArtworkURL: URL?
    var epicHeroURL: URL?
    var gogArtworkURL: URL?
    var gogID:Int?
    var gogWorkingDir:String?
    var arguments=""
    var temporarySession:Bool?
    static let desktopID = UUID()
    static var desktopEntry: Self { Self(title: "Desktop", desktop: true) }
}
@MainActor final class LibraryAccess: ObservableObject {
    static let shared = LibraryAccess()
    @Published var unlocked = false
    @Published var requested = false
    var guarded = true
    func toggle() { unlocked.toggle() }
    func lock() { unlocked = false }
    func submit(_ pin: String, confirmation: String) async -> String? { nil }
}
@MainActor final class LibraryModel: ObservableObject {
    static let shared = LibraryModel()
    static var documents: URL { Bundle.main.bundleURL }
    static var drive: URL { Bundle.main.bundleURL }
    func save(_ entry:LibraryEntry) {}
    @Published var entries: [LibraryEntry] = []
    @Published var genres = ["Action","Adventure","Indie","Visual novel"].map { LibraryGenre(title:$0) }
    func isPrivate(_ entry: LibraryEntry) -> Bool { entry.privateGame == true }
    func refreshMetadata(_ id: UUID) async {}
    func seed() {
        let titles = ["Hollow Knight","REPLACED","Hollow Knight: Silksong","Primal Planet","Silver Pines",
            "Liar's Bar","Hollow Knight","REPLACED","Silver Pines","Primal Planet"]
        let ids = [367520,1663850,1030300,2350270,2333000,3097560,367520,1663850,2333000,2350270]
        entries = zip(titles,ids).enumerated().map { i,pair in
            var entry = LibraryEntry(title:pair.0, steamID:pair.1)
            entry.lastPlayed = Date().addingTimeInterval(-Double(i)*86400)
            entry.privateGame = i >= 6
            return entry
        }
    }
}
struct DockGame { var id: Int }
@MainActor final class PreviewSteam: ObservableObject {
    static let shared = PreviewSteam()
    var games: [DockGame] = []
}
struct SteamCatalog {
    static func cover(_ id:Int)->URL? { Bundle.main.url(forResource:"cover_\(id)",withExtension:"jpg",subdirectory:"madeira-art") }
    static func hero(_ id:Int)->URL? { Bundle.main.url(forResource:"hero_\(id)",withExtension:"jpg",subdirectory:"madeira-art") ?? cover(id) }
}
struct SteamGamesSection: View {
    enum Part { case management }
    var search:String; var layout:String; var sort:String; var width:CGFloat
    var part:Part; var genre:UUID?; var recentOnly:Bool; var open:(LibraryEntry)->Void
    static var shown:Bool { false }
    static func refresh() async {}
    var body:some View { EmptyView() }
}
struct EpicGamesSection: View {
    var search:String;var layout:String;var width:CGFloat;var genre:UUID?;var managementOnly:Bool;var open:(LibraryEntry)->Void
    static func refresh() async {}
    var body:some View { EmptyView() }
}

enum SettingsSheet {case gogSignIn}
struct PreviewController { var connected = false }
struct AmbientArt { static func library(_ e:LibraryEntry)->Self { Self() } }
struct AmbientGlowItem { var id:String;var seed:Int;var art:AmbientArt;var pressed:Bool;var bounds:Anchor<CGRect> }
struct LibraryCardArtworkPress: ViewModifier {
    let glow:(Bool,Anchor<CGRect>)->AmbientGlowItem
    func body(content:Content)->some View { content }
}
struct LibraryBadges:View { let entry:LibraryEntry;var body:some View { Text("64-bit").font(.caption) } }
extension View { func libraryCardButtonStyle(grid:Bool)->some View { buttonStyle(.plain) } }
struct LibraryScrollActivity:ViewModifier {
    @Binding var active:Bool
    func body(content:Content)->some View { content }
}
private struct PreviewScrollingKey:EnvironmentKey { static let defaultValue=false }
extension EnvironmentValues {
    var libraryScrolling:Bool { get { self[PreviewScrollingKey.self] } set { self[PreviewScrollingKey.self]=newValue } }
}
struct PreviewLibrary:View {
    @ObservedObject var model = LibraryModel.shared
    @ObservedObject var access = LibraryAccess.shared
    @ObservedObject var steamGames = PreviewSteam.shared
    @Environment(\.horizontalSizeClass) var horizontalSizeClass
    @State var search=""
    @State var sort="played"
    @State var layout="cards"
    @State var privatePlaylist=false
    @State var recentOnly=false
    @State var tab=0
    @State var genre:UUID?
    @State var manageGenres=false
    @State var selected:LibraryEntry?
    @State var focused:UUID?
    @State var libraryScrolling=false
    @State var browser=false
    var installedProfiles:[LibraryEntry]=[]
    let controller=PreviewController()
    var play:(LibraryEntry)->Void = {_ in}
    init(privateMode:Bool) { _privatePlaylist=State(initialValue:privateMode) }
    func switchTab(to tab:Int) { self.tab=tab }
    var body:some View {
        HStack(spacing:0) {
            sidebar.frame(width:224)
            Divider()
            library.frame(maxWidth:.infinity,maxHeight:.infinity)
        }.background(LibraryTheme.background).foregroundStyle(.white)
            .tint(LibraryTheme.accent).preferredColorScheme(.dark)
            .environment(\.horizontalSizeClass,.regular)
    }
'''
start = library.index('struct LibraryView:')
shelf = library[start:]
for sig in ('private var entries:', 'private var libraryOptions:', 'private var shelfActions:',
            'private func navigationRow(', 'private var sidebar:', 'private var recentGames:',
            'private var continuePlaying:', 'private var genreChips:', 'private func cells(', 'private func libraryItem('):
    sources += block(shelf, sig)
body = block(shelf, 'private var library: some View')
body = body[:body.index('        .onReceive(controller.commands)')] + '\n        }\n        }\n    }\n'
sources += body + '\n}\n'
sources += (root / 'app/Madeira/GOG/GOGStore.swift').read_text(encoding='utf-8')

sources += r'''
@main struct MadeiraPreviewApp:App {
    var body:some Scene {
        WindowGroup { Color.black.ignoresSafeArea().task { await capture() } }
    }
    @MainActor func capture() async {
        LibraryModel.shared.seed()
        guard let scene=UIApplication.shared.connectedScenes.compactMap({$0 as? UIWindowScene}).first else { return }
        let size=CGSize(width:1194,height:834)
        let folder=FileManager.default.urls(for:.documentDirectory,in:.userDomainMask)[0]
        for mode in ["library","private","pin"] {
            LibraryAccess.shared.unlocked = mode == "private"
            let root:AnyView = mode == "pin" ? AnyView(LibraryAccessView()) : AnyView(PreviewLibrary(privateMode:mode == "private"))
            let window=UIWindow(windowScene:scene)
            window.frame=CGRect(origin:.zero,size:size)
            let host=UIHostingController(rootView:root.frame(width:size.width,height:size.height)
                .environment(\.horizontalSizeClass,.regular).preferredColorScheme(.dark))
            window.rootViewController=host;window.makeKeyAndVisible()
            host.view.frame=CGRect(origin:.zero,size:size)
            host.view.setNeedsLayout();host.view.layoutIfNeeded()
            try? await Task.sleep(for:.seconds(3))
            let format=UIGraphicsImageRendererFormat();format.scale=2
            let image=UIGraphicsImageRenderer(size:size,format:format).image { _ in
                host.view.drawHierarchy(in:CGRect(origin:.zero,size:size),afterScreenUpdates:true)
            }
            if let png=image.pngData() { try? png.write(to:folder.appendingPathComponent(mode+".png"),options:.atomic) }
            window.isHidden=true
        }
        try? Data("complete".utf8).write(to:folder.appendingPathComponent("screenshots-ready.txt"))
    }
}
'''
(out / 'Preview.swift').write_text(sources, encoding='utf-8')
assets = out / 'Assets.xcassets'
assets.mkdir(exist_ok=True)
shutil.copytree(root / 'app/Madeira/Assets.xcassets/LibraryLandscape.imageset', assets / 'LibraryLandscape.imageset', dirs_exist_ok=True)
(assets / 'Contents.json').write_text('{"info":{"author":"xcode","version":1}}')
receipt = {p: hashlib.sha256((root/p).read_bytes()).hexdigest() for p in (
    'app/Madeira/Library.swift','app/Madeira/LibraryAccess.swift','app/Madeira/LibraryGrouping.swift')}
receipt['scope'] = 'Production shelf/search/PIN SwiftUI; fixture account/game data; no Wine/JIT or release IPA'
(out / 'preview-source.json').write_text(json.dumps(receipt, indent=2))
print('Generated production-view simulator preview:', out)
