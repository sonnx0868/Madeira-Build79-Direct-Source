#!/usr/bin/env python3
"""Release identity must agree across the plist, Xcode and IPA artifacts."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
plist = (root / "app/Madeira/Info.plist").read_text(encoding="utf-8")
project = (root / "app/Madeira.xcodeproj/project.pbxproj").read_text(encoding="utf-8")
ci = (root / "codemagic.yaml").read_text(encoding="utf-8")
source = (root / "app/source.json").read_text(encoding="utf-8")

version = "0.1.5"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"version contract failed: {message}")


require("<string>$(MARKETING_VERSION)</string>" in plist, "CFBundleShortVersionString build setting")
require("<string>$(CURRENT_PROJECT_VERSION)</string>" in plist, "CFBundleVersion build setting")
require(f"<string>v{version}-source</string>" in plist, "visible/logged source build label")
require(project.count(f"MARKETING_VERSION = {version};") == 4, "app and JIT helper Debug/Release marketing versions")
require(project.count("CURRENT_PROJECT_VERSION = 11;") == 4, "app and JIT helper Debug/Release build numbers")
require(f"Madeira-v{version}-source.ipa" in ci and f"Madeira-v{version}-source-bootstrap.ipa" in ci,
        "versioned Codemagic artifacts")
require(f'"version": "{version}"' in source and '"versionDate": "2026-10-07"' in source,
        "AltStore/source metadata")
require("0.1.0" not in plist + project + source and "v0.1.3" not in ci and "Madeira-upstream.ipa" not in ci,
        "stale release identity")

print(f"version contract: {version} source")
