#!/usr/bin/env python3
"""The iOS virtual monitor must expose a complete QueryDisplayConfig path."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
source = (root / "build/win32u-unix/sysparams_ios.c").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"virtual display config failed: {message}")


query = source[source.index("LONG WINAPI NtUserQueryDisplayConfig"):
               source.index("static struct monitor *find_monitor_by_index")]
device = source[source.index("NTSTATUS WINAPI NtUserDisplayConfigGetDeviceInfo"):
                source.index("NTSTATUS WINAPI NtGdiDdDDIEnumAdapters2")]
require("if (ios_virtual_monitor_active())" in query,
        "QueryDisplayConfig does not recognize Madeira's source-less virtual monitor")
require("ios_screen_size( &sw, &sh )" in query and
        "devmode.dmPelsWidth = sw" in query and "devmode.dmPelsHeight = sh" in query,
        "the path is not built from the selected session resolution")
for call in ("set_mode_source_info", "set_mode_target_info", "set_path_source_info",
             "set_path_target_info"):
    require(call in query, f"synthetic topology omits {call}")
require("set_mode_desktop_info" in query and "DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE" in query,
        "QDC_VIRTUAL_MODE_AWARE has no desktop-image mode")
require("*paths_count = 1" in query and "*modes_count = required_modes" in query,
        "returned counts do not match the buffer-size query")
require("QueryDisplayConfig virtual=%dx%d@60" in query,
        "device logs cannot confirm the modern display API result")
require("ios_virtual_luid" in query and "ios_virtual_luid" in device,
        "QueryDisplayConfig and its device-info follow-ups do not share a stable adapter")
require("if (ios_virtual_monitor_active())" in device and
        'asciiz_to_unicode( source_name->viewGdiDeviceName, "\\\\\\\\.\\\\DISPLAY1" )' in device,
        "the synthetic source name is missing")
require("Madeira Display" in device and "VIRTUAL_MONITOR" in device,
        "the synthetic target cannot be identified")
require("DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE" in device and
        "preferred->width = sw" in device and "preferred->height = sh" in device,
        "preferred mode does not match the selected virtual resolution")
virtual = device[device.index("if (ios_virtual_monitor_active())"):device.index("#endif")]
require("monitor->source" not in virtual,
        "virtual device-info still dereferences the intentionally source-less monitor")

print("virtual QueryDisplayConfig contract: ok")
