#!/usr/bin/env python3
"""Stamp the exact source/build into the app and a GitHub release manifest."""
import json
import os
from pathlib import Path
import plistlib
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
ipa = sys.argv[1]
if Path(ipa).name != ipa or not ipa.endswith(".ipa"):
    raise SystemExit("Pass the IPA asset filename, without a path.")
project = (root / "app/Madeira.xcodeproj/project.pbxproj").read_text(encoding="utf-8")
versions = set(re.findall(r"MARKETING_VERSION = ([0-9.]+);", project))
if len(versions) != 1:
    raise SystemExit("App and helper versions must agree.")
version = versions.pop()
def git(*args):
    return subprocess.check_output(["git", "-C", str(root), *args], text=True).strip()
commit = git("rev-parse", "HEAD")
build = int(git("rev-list", "--count", "HEAD"))
if git("rev-parse", "--is-shallow-repository") != "false":
    raise SystemExit("Fetch full history before generating a release build number.")
if not 1 <= build <= 9999:
    raise SystemExit("Build count must fit CFBundleVersion's first component; adjust the release scheme.")
path = root / "app/Madeira/Info.plist"
with path.open("rb") as handle:
    info = plistlib.load(handle)
info["MadeiraBuild"] = f"v{version}-source-{commit[:7]}"
info["MadeiraSourceCommit"] = commit
staging = path.with_suffix(".plist.tmp")
with staging.open("wb") as handle:
    plistlib.dump(info, handle, sort_keys=False)
os.replace(staging, path)
out = root / "build/release-info"
out.mkdir(parents=True, exist_ok=True)
(out / "build-number.txt").write_text(str(build), encoding="utf-8")
manifest = {"version": version, "build": build, "commit": commit, "ipa": ipa}
(out / "madeira-update.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
print(f"Release identity: {version} ({build}), source {commit[:7]}, asset {ipa}")
