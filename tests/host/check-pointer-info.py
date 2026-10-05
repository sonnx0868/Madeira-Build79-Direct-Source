#!/usr/bin/env python3
"""Mouse-in-pointer state required by Unity's Input System is not stubbed."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
wine = root / "wine"
input_c = (wine / "dlls/win32u/input.c").read_text(encoding="utf-8")
message = (root / "build/win32u-unix/message_ios.c").read_text(encoding="utf-8")
user32 = (wine / "dlls/user32/misc.c").read_text(encoding="utf-8")
build = (root / "build/wine-pe/build-controller.sh").read_text(encoding="utf-8")
package = (root / "scripts/package-native-deps.sh").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"pointer-info contract failed: {message}")


wrapper = user32[user32.index("BOOL WINAPI GetPointerInfo"):
                 user32.index("LRESULT WINAPI ImeWndProcA")]
require("NtUserGetPointerInfoList" in wrapper and "ERROR_INVALID_PARAMETER" not in wrapper,
        "GetPointerInfo is still the unconditional user32 stub")

impl = input_c[input_c.index("BOOL WINAPI NtUserGetPointerInfoList"):
               input_c.index("static BOOL get_clip_cursor")]
for field in ("pointerType", "pointerId", "frameId", "pointerFlags", "hwndTarget",
              "ptPixelLocation", "dwTime", "PerformanceCount", "ButtonChangeType"):
    require(field in impl, f"tracked POINTER_INFO omits {field}")
require("ERROR_CALL_NOT_IMPLEMENTED" not in impl, "NtUserGetPointerInfoList is still a stub")
require("update_pointer_from_msg( PT_MOUSE, &pointer_msg )" in message,
        "WM_POINTER mouse messages do not update the state GetPointerInfo returns")
require("WM_POINTERDOWN" in message and "WM_POINTERUP" in message and "WM_POINTERUPDATE" in message,
        "mouse-in-pointer does not expose click and motion edges")
require("modules=(win32u user32" in build and "NtUserGetPointerInfoList" in build,
        "the public user32 wrapper and win32u bridge are not rebuilt together")
require("arm64ec-windows/user32.dll" in package,
        "cached builds would restore the old GetPointerInfo stub")

print("mouse pointer-info contract: ok")
