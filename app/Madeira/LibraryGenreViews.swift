import SwiftUI

struct LibraryGenresView: View {
    @ObservedObject private var model = LibraryModel.shared
    @ObservedObject private var access = LibraryAccess.shared
    @Environment(\.dismiss) private var dismiss
    @State private var newTitle = ""
    @State private var editing: LibraryGenre?
    private var groups: [LibraryGenre] { LibraryVisibility.shownGenres(model.genres, unlocked: access.unlocked) }
    var body: some View {
        NavigationStack {
            Form {
                Section("New genre") {
                    TextField("Name", text: $newTitle)
                    Button("Add") {
                        model.saveGenre(LibraryGenre(title: newTitle)); newTitle = ""
                    }.disabled(newTitle.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
                }
                Section("Genres") {
                    ForEach(groups) { group in
                        Button { editing = group } label: {
                            HStack {
                                Text(group.title).foregroundStyle(.primary)
                                Spacer()
                                Text("\(model.entries.filter { $0.genreID == group.id && model.isVisible($0) }.count)")
                                    .foregroundStyle(.secondary)
                                if access.unlocked && group.isPrivate == true { Image(systemName: "lock.fill") }
                            }
                        }
                    }
                }
                if access.unlocked {
                    Section { Button("Lock now") { access.lock(); dismiss() } }
                }
            }.navigationTitle("Genres").navigationBarTitleDisplayMode(.inline)
                .toolbar { ToolbarItem(placement: .confirmationAction) { Button("Done") { dismiss() } } }
                .sheet(item: $editing) { group in LibraryGenreEditor(group: group) }
                .onChange(of: access.unlocked) { _, unlocked in if !unlocked { editing = nil } }
        }
    }
}

private struct LibraryGenreEditor: View {
    @State var group: LibraryGenre
    @ObservedObject private var model = LibraryModel.shared
    @ObservedObject private var access = LibraryAccess.shared
    @Environment(\.dismiss) private var dismiss
    @State private var deleting = false
    var body: some View {
        NavigationStack {
            Form {
                Section { TextField("Name", text: $group.title) }
                if access.unlocked {
                    Section {
                        Toggle("Keep this genre private", isOn: Binding(get: { group.isPrivate == true },
                            set: { group.isPrivate = $0 ? true : nil }))
                    } footer: { Text("Games in this genre disappear from the library when access is locked.") }
                }
                Section { Button("Delete genre", role: .destructive) { deleting = true } }
            }.navigationTitle("Genre").navigationBarTitleDisplayMode(.inline)
                .toolbar { ToolbarItem(placement: .confirmationAction) { Button("Done") { model.saveGenre(group); dismiss() } } }
                .confirmationDialog("Delete this genre? Games and files are kept.", isPresented: $deleting) {
                    Button("Delete genre", role: .destructive) { model.removeGenre(group.id); dismiss() }
                }
                .onChange(of: access.unlocked) { _, unlocked in if !unlocked { dismiss() } }
        }
    }
}

struct LibraryGenrePicker: View {
    @Binding var entry: LibraryEntry
    @ObservedObject private var model = LibraryModel.shared
    @ObservedObject private var access = LibraryAccess.shared
    var body: some View {
        Section("Genre") {
            Picker("Genre", selection: $entry.genreID) {
                Text("Uncategorized").tag(Optional<UUID>.none)
                ForEach(LibraryVisibility.shownGenres(model.genres, unlocked: access.unlocked)) { group in
                    Text(group.title).tag(Optional(group.id))
                }
            }
            if access.unlocked {
                Toggle("Keep this game private", isOn: Binding(get: { entry.privateGame == true },
                    set: { entry.privateGame = $0 ? true : nil }))
                Text("Private genres also hide their games. Hold the Madeira logo for two seconds to lock or unlock.")
                    .font(.caption).foregroundStyle(.secondary)
            }
        }
    }
}
