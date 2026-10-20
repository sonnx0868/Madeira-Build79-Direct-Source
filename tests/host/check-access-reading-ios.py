#!/usr/bin/env python3
"""Typecheck the real privacy/reading views against the real iOS SDK.

The large library/Wine shell is stubbed; the LAContext calls, UIKit lifecycle,
SwiftUI views and bridge file policy are the production sources.
"""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
sdk = subprocess.check_output(['xcrun', '--sdk', 'iphoneos', '--show-sdk-path'], text=True).strip()
with tempfile.TemporaryDirectory(prefix='madeira-access-ios-') as folder:
    stub = Path(folder) / 'Shell.swift'
    stub.write_text('''
import Foundation
import UIKit
struct LibraryEntry {
    var id: UUID
    var desktop: Bool?
    var temporarySession: Bool?
    var readingProgress: Bool?
    var readingProgressPosition: ReadingProgressPosition?
    var launchRelativePath: String
}
final class LibraryModel {
    static let shared = LibraryModel()
    static let drive = URL(fileURLWithPath: "/tmp/drive")
    var entries: [LibraryEntry] = []
    var activeEntry: LibraryEntry?
    var current: UUID?
    var startingJIT: UUID?
    var menu = false
    var showDetail: UUID?
    var error: String?
    var exitNotice: String?
    var launchAttention: String?
    var cloudNotice: String?
    var cloudRetry: String?
    func isPrivate(_ entry: LibraryEntry) -> Bool { false }
    func save(_ entry: LibraryEntry) {}
}
final class MetalHostView { static let shared = UIView() }
final class LogStore { static let shared = LogStore(); func log(_ value: String) {} }
''', encoding='utf-8')
    sources = ['LibraryPIN.swift', 'LibraryAccess.swift', 'ReadingProgressData.swift', 'ReadingProgress.swift']
    subprocess.run(['xcrun', 'swiftc', '-typecheck', '-swift-version', '5', '-target', 'arm64-apple-ios18.0',
                    '-sdk', sdk, *(str(root / 'app/Madeira' / name) for name in sources), str(stub)],
                   check=True, timeout=120)
print('PASS: production privacy/reading views and LocalAuthentication API typecheck on iOS')
