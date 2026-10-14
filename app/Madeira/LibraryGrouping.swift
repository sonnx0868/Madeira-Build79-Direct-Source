import Foundation

struct LibraryGenre: Codable, Identifiable, Equatable {
    var id = UUID()
    var title: String
    var isPrivate: Bool?
    static let uncategorized = UUID(uuidString: "00000000-0000-0000-0000-000000000000")!
}

enum LibraryVisibility {
    static func shelfAllows(isPrivate: Bool, unlocked: Bool, privatePlaylist: Bool) -> Bool {
        privatePlaylist ? unlocked && isPrivate : !isPrivate
    }
    static func isPrivate(flag: Bool?, genreID: UUID?, genres: [LibraryGenre]) -> Bool {
        if flag == true { return true }
        guard let genreID else { return false }
        // An orphaned group cannot safely be assumed public after a restore.
        guard let group = genres.first(where: { $0.id == genreID }) else { return true }
        return group.isPrivate == true
    }
    static func allows(flag: Bool?, genreID: UUID?, genres: [LibraryGenre], unlocked: Bool) -> Bool {
        unlocked || !isPrivate(flag: flag, genreID: genreID, genres: genres)
    }
    static func matches(_ genreID: UUID?, selection: UUID?, genres: [LibraryGenre]) -> Bool {
        guard let selection else { return true }
        if selection == LibraryGenre.uncategorized {
            return genreID == nil || !genres.contains(where: { $0.id == genreID })
        }
        return genreID == selection
    }
    static func shownGenres(_ genres: [LibraryGenre], unlocked: Bool) -> [LibraryGenre] {
        genres.filter { unlocked || $0.isPrivate != true }.sorted {
            $0.title.localizedStandardCompare($1.title) == .orderedAscending
        }
    }
}
