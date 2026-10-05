#!/usr/bin/env python3
"""Contracts for direct-folder dependency inspection and installer sessions."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
dock = (root / "app/Madeira/DockInstallers.swift").read_text(encoding="utf-8")
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
content = (root / "app/Madeira/ContentView.swift").read_text(encoding="utf-8")
docs = (root / "docs/LIBRARY.md").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"folder-components contract failed: {message}")


scan_start = dock.index("enum FolderComponents")
runner_start = dock.index("enum FolderInstallerRunner")
scan = dock[scan_start:runner_start]
runner = dock[runner_start:]

for family in ["vc140", "vc120", "vc110", "vc100", "vc90", "vc80", "directx", "dotnet"]:
    require(f'id: "{family}"' in scan, f"missing dependency family {family}")
require("entries > 20_000" in scan and "peFiles > 4_096" in scan,
        "folder scan must remain bounded")
require("DockInstallers.machine(file) == expectedMachine" in scan,
        "imports from backup/foreign-architecture binaries must not contaminate the report")
require("api-ms-win-core-winrt-robuffer-l1-1-0.dll" in scan and 'imports.insert("wintypes.dll")' in scan,
        "WinRT robuffer must diagnose the physical Wine host DLL")
require("URLSession" not in scan and "http://" not in scan and "https://" not in scan,
        "dependency inspection must not download arbitrary DLLs")
require("installAppLocalDLLs" in scan and "protectedDLLs" in scan,
        "the app-local DLL route must protect the shared Windows/Wine core")
require("DockInstallers.machine(source) == expected" in scan and "manager.copyItem" in scan,
        "app-local DLLs must match the game architecture before copying")
require("already exists in the game folder" in scan,
        "app-local import must not overwrite existing game files")

require('case "msi"' in runner and "bundleHasMsiexec" in runner,
        "MSI launch must be gated by a bundled msiexec")
require("machine == 0x14c && !has32Bit" in runner and "bundleHas32Bit" in runner,
        "x86 installers must be gated by the i386 bundle")
for token in ['arguments.contains("&")', 'arguments.contains("|")', 'arguments.contains("%")']:
    require(token in runner, "installer arguments must reject cmd metacharacters")
require("if copy" in runner and "package = source" in runner,
        "picked packages are staged while in-folder installers run beside their payloads")
require("services.exe" in runner and "last-result.txt" in runner and "madeira_status" in runner,
        "installer desktop must start services and retain an exit result")
require("temporarySession = true" in runner,
        "installer and winecfg profiles must be transient")

require('Text("Components & installers")' in library,
        "direct game details need the component manager section")
require("entry.desktop != true && entry.steamAppID == nil" in library,
        "the section must be scoped to directly added games")
require('Button("Choose installer…"' in library and 'Button("Open Wine configuration"' in library,
        "the UI must expose installer import and winecfg")
require('Button("Import app-local DLLs…"' in library and "allowsMultipleSelection: true" in library,
        "the UI must expose bounded multi-DLL app-local import")
require("remember: entry.temporarySession != true" in content,
        "utility sessions must not become library entries")
require("Components and installers" in docs and "C:\\madeira-installers" in docs and "Import app-local DLLs" in docs,
        "user-visible behavior and staging location must be documented")

print("folder-components contract: ok")
