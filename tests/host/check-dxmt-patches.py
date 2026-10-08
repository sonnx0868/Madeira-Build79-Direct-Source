#!/usr/bin/env python3
"""Reproduce the complete ordered DXMT patch pipeline on LF-only pinned sources."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

PATCH_NAMES = ("dxmt-madeira-query-log.patch", "dxmt-runtime-lifecycle.patch", "dxmt-gameplay-performance.patch")
SOURCE_FILES = ("src/util/com/com_guid.cpp", "src/winemetal/unix/winemetal_unix.c",
                "src/winemetal/unix/cache.c", "src/d3d11/d3d11_pipeline_cache.cpp",
                "src/d3d9/d3d9_shader.cpp", "src/dxmt/dxmt_tasks.hpp", "src/dxmt/dxmt_shader_cache.hpp")


def patched_sources(root):
    pin = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD:dxmt"], text=True).strip()
    with tempfile.TemporaryDirectory(prefix="madeira-dxmt-pipeline-") as scratch:
        folder = Path(scratch)
        checkout = folder / "dxmt"
        patch_folder = folder / "patches"
        patch_folder.mkdir()
        for name in SOURCE_FILES:
            path = checkout / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(subprocess.check_output(["git", "-C", str(root / "dxmt"), "show", pin + ":" + name]))
        patches = []
        for name in PATCH_NAMES:
            path = patch_folder / name
            # Model macOS/Linux checkout regardless of the test host's autocrlf.
            path.write_bytes((root / "patches" / name).read_bytes().replace(b"\r\n", b"\n"))
            patches.append(path)
        script = folder / "scripts/apply-dxmt-patches.sh"
        script.parent.mkdir()
        script.write_bytes((root / "scripts/apply-dxmt-patches.sh").read_bytes().replace(b"\r\n", b"\n"))
        git_bash = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Git/bin/bash.exe"
        bash = str(git_bash) if os.name == "nt" and git_bash.exists() else shutil.which("bash")
        if not bash:
            raise RuntimeError("bash is required to test the production DXMT patch pipeline")
        # Run the actual production shell script for both a clean checkout and
        # an already patched checkout; Windows WSL is not required.
        for _ in range(2):
            result = subprocess.run([bash, script.as_posix()], capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError("DXMT patch pipeline failed:\n" + result.stdout + result.stderr)
        # Verify re-running the production pipeline will recognize every patch,
        # including the lifecycle patch AFTER all gameplay changes have landed.
        for path in patches:
            subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(path)], check=True)
        return {name: (checkout / name).read_text(encoding="utf-8") for name in SOURCE_FILES}


if __name__ == "__main__":
    patched_sources(Path(__file__).resolve().parents[2])
    print("PASS: pinned LF-only DXMT checkout accepts query -> lifecycle -> gameplay patches; all remain idempotent")
