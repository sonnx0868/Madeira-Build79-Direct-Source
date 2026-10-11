#!/usr/bin/env python3
"""Exercise release stamping with shallow history and unavailable old gitlinks."""
import json
import os
from pathlib import Path
import plistlib
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
env = os.environ.copy()
env.update(GIT_TERMINAL_PROMPT="0", GIT_CONFIG_NOSYSTEM="1")
bash = shutil.which("bash")
if bash is None:
    raise SystemExit("Release history check requires bash.")


def run(cwd, *args, check=True):
    result = subprocess.run(args, cwd=cwd, env=env, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if check and result.returncode:
        raise RuntimeError(f"{args}:\n{result.stdout}\n{result.stderr}")
    return result


def git(cwd, *args, check=True):
    return run(cwd, "git", "-c", "protocol.file.allow=always", *args, check=check)


def init(path):
    path.mkdir()
    git(path, "init", "-b", "main")
    git(path, "config", "user.name", "Release test")
    git(path, "config", "user.email", "release-test@example.invalid")


with tempfile.TemporaryDirectory(prefix="madeira-release-history-") as tmp:
    base = Path(tmp)
    module, origin = base / "module", base / "origin"
    init(module)
    (module / "native.txt").write_text("built native dependency\n")
    git(module, "add", ".")
    git(module, "commit", "-m", "Published dependency")
    pin = git(module, "rev-parse", "HEAD").stdout.strip()

    init(origin)
    (origin / "scripts").mkdir()
    for name in ("prepare-release.sh", "release-metadata.py"):
        shutil.copyfile(ROOT / "scripts" / name, origin / "scripts" / name)
    (origin / "app/Madeira.xcodeproj").mkdir(parents=True)
    (origin / "app/Madeira").mkdir()
    (origin / "app/Madeira.xcodeproj/project.pbxproj").write_text("MARKETING_VERSION = 0.1.6;\n")
    (origin / "app/Madeira/Info.plist").write_bytes(plistlib.dumps({"MadeiraBuild": "unstamped"}))
    (origin / ".gitmodules").write_text(
        f'[submodule "FEX"]\n\tpath = FEX\n\turl = {module.as_uri()}\n')
    git(origin, "add", ".")
    # An unavailable historical pin must not prevent building the current pin.
    missing = "1" * 40
    git(origin, "update-index", "--add", "--cacheinfo", f"160000,{missing},FEX")
    git(origin, "commit", "-m", "Historical dependency no longer published")
    git(origin, "update-index", "--cacheinfo", f"160000,{pin},FEX")
    git(origin, "commit", "-m", "Current published dependency")

    def clone(name):
        path = base / name
        git(base, "clone", "--depth=1", origin.as_uri(), str(path))
        git(path, "submodule", "update", "--init", "FEX")
        git(path, "config", "fetch.recurseSubmodules", "on-demand")
        assert git(path, "rev-parse", "--is-shallow-repository").stdout.strip() == "true"
        return path

    broken = clone("original")
    fixed = clone("fixed")
    # The remote may advance while a CI checkout is compiling dependencies.
    # Fetching those parent commits must not chase their unpublished gitlinks.
    git(origin, "update-index", "--cacheinfo", f"160000,{missing},FEX")
    git(origin, "commit", "-m", "Remote advances to an unavailable dependency")
    result = git(broken, "fetch", "--unshallow", "origin", check=False)
    assert result.returncode != 0 and missing in result.stderr, result.stdout + result.stderr
    print("PASS: original recursive history fetch reproduces missing submodule commit failure")

    before = git(fixed / "FEX", "rev-parse", "HEAD").stdout.strip()
    asset = "Madeira-v0.1.6-source-bootstrap.ipa"
    run(fixed, bash, "scripts/prepare-release.sh", asset)
    assert git(fixed, "rev-parse", "--is-shallow-repository").stdout.strip() == "false"
    head = git(fixed, "rev-parse", "HEAD").stdout.strip()
    manifest = json.loads((fixed / "build/release-info/madeira-update.json").read_text())
    assert manifest == {"version": "0.1.6", "build": 2, "commit": head, "ipa": asset}, manifest
    assert (fixed / "build/release-info/build-number.txt").read_text() == "2"
    info = plistlib.loads((fixed / "app/Madeira/Info.plist").read_bytes())
    assert info["MadeiraSourceCommit"] == head
    assert info["MadeiraBuild"] == f"v0.1.6-source-{head[:7]}"
    assert git(fixed / "FEX", "rev-parse", "HEAD").stdout.strip() == before == pin
    assert not git(fixed / "FEX", "status", "--porcelain").stdout.strip()
    print("PASS: production release script obtains full parent history and stamps exact identity while preserving native pin")

    git(fixed, "remote", "set-url", "origin", (base / "unavailable").as_uri())
    run(fixed, bash, "scripts/prepare-release.sh", asset)
    assert json.loads((fixed / "build/release-info/madeira-update.json").read_text()) == manifest
    print("PASS: full-history checkout stamps release without another fetch")
