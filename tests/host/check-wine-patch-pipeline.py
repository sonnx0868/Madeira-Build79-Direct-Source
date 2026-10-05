#!/usr/bin/env python3
"""The root branch must carry and apply all dirty Wine submodule work."""

from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[2]
patch = root / "build/wine-pe/madeira-v0.1.4.patch"
apply = (root / "scripts/apply-wine-patches.sh").read_text(encoding="utf-8")
bootstrap = (root / "scripts/bootstrap-native-deps.sh").read_text(encoding="utf-8")
package = (root / "scripts/package-native-deps.sh").read_text(encoding="utf-8")
ci = (root / "codemagic.yaml").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"wine patch pipeline failed: {message}")


text = patch.read_text(encoding="utf-8")
for path in ["dlls/ntdll/unix/file.c", "dlls/wintypes/buffer.c", "dlls/wintypes/wintypes.spec",
             "libs/vkd3d/libs/vkd3d-shader/hlsl_codegen.c"]:
    require(f"diff --git a/{path} b/{path}" in text, f"patch omits {path}")
require("apply --reverse --check" in apply and "apply --check" in apply,
        "patch application must be idempotent and fail closed")
require("scripts/apply-wine-patches.sh" in bootstrap and "build-wintypes.sh" in bootstrap and
        "build-d3dcompiler.sh" in bootstrap, "cold build does not apply/rebuild Wine changes")
require("arm64ec-windows/wintypes.dll" in package and "arm64ec-windows/d3dcompiler_47.dll" in package,
        "cached native bundle omits rebuilt PE modules")
require("scripts/apply-wine-patches.sh" in ci, "cached workflow does not align Wine source")

reverse = subprocess.run(["git", "-C", str(root / "wine"), "apply", "--reverse", "--check", str(patch)],
                         capture_output=True, text=True)
require(reverse.returncode == 0, "stored patch does not reproduce the current Wine worktree")

print("wine patch pipeline: ok")
