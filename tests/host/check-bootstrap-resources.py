#!/usr/bin/env python3
"""A versioned bootstrap IPA must not drop the v0.1.3 runtime/resource set."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
bootstrap = (root / "scripts/bootstrap-native-deps.sh").read_text(encoding="utf-8")
check = (root / "scripts/check-ios-build.sh").read_text(encoding="utf-8")
package = (root / "scripts/package-native-deps.sh").read_text(encoding="utf-8")
ci = (root / "codemagic.yaml").read_text(encoding="utf-8")
project = (root / "app/Madeira.xcodeproj/project.pbxproj").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"bootstrap resource contract failed: {message}")


for script in ["fetch-vcruntime.sh", "fetch-love-luajit-gc64.sh", "fetch-angle-d3d11.sh"]:
    require(script in bootstrap, f"bootstrap does not run {script}")
for resource in ["libEGL.dll", "libGLESv2.dll", "lua51-gc64.dll", "wintypes.dll"]:
    require(resource in check, f"preflight does not require {resource}")
for resource in ["libEGL.dll", "libGLESv2.dll", "lua51-gc64.dll", "wintypes.dll", "d3dcompiler_47.dll"]:
    require(resource in package, f"reusable native bundle omits {resource}")
for runtime in ["msvcp90.dll", "msvcr90.dll"]:
    require(runtime in check and "x86_64-vcruntime" in package, f"VC90 runtime is not retained: {runtime}")
for embedded in ["StikJIT.xcframework", "MadeiraJITHelper.appex", "Madeira JIT.shortcut"]:
    require(embedded in project or embedded in ci or embedded in check, f"app resource is not guarded: {embedded}")
require("test -d \"$app/Frameworks/StikJIT.framework\"" in ci and
        "test -d \"$app/PlugIns/MadeiraJITHelper.appex\"" in ci,
        "IPA packaging must fail if JIT framework/helper disappears")
require("Madeira-v0.1.5-source-bootstrap.ipa" in ci, "bootstrap artifact is not versioned")

print("bootstrap resource contract: ok")
