#!/usr/bin/env python3
"""Modern Unity's WinRT API-set host must be built into the ARM64EC farm."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
schema = (root / "wine/dlls/apisetschema/apisetschema.spec").read_text(encoding="utf-8")
spec = (root / "wine/dlls/wintypes/wintypes.spec").read_text(encoding="utf-8")
buffer = (root / "wine/dlls/wintypes/buffer.c").read_text(encoding="utf-8")
build = (root / "build/wine-pe/build-wintypes.sh").read_text(encoding="utf-8")
check = (root / "scripts/check-ios-build.sh").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"wintypes bundle contract failed: {message}")


require("api-ms-win-core-winrt-robuffer-l1-1-0 = wintypes.dll" in schema,
        "Wine API-set mapping changed")
require("@ stdcall RoGetBufferMarshaler(ptr)" in spec and "@ stub RoGetBufferMarshaler" not in spec,
        "the loader export must be callable, not an exception-raising spec stub")
require("HRESULT WINAPI RoGetBufferMarshaler" in buffer and "return E_NOTIMPL" in buffer,
        "unimplemented marshaling must fail through HRESULT")
require("make -C \"$B/dlls/wintypes\"" in build and "arm64ec-windows/wintypes.dll" in build,
        "the ARM64EC DLL needs a reproducible staging path")
require("app/Madeira/arm64ec-windows/wintypes.dll" in check,
        "IPA preflight must reject a build that would fail IL2CPP at load time")

print("wintypes bundle contract: ok")
