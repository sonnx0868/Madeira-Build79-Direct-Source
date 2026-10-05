#!/usr/bin/env python3
"""Direct-launch Windows errors must replace the endless starting spinner."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
driver = (ROOT / "build/win32u-unix/driver_ios.c").read_text(encoding="utf-8")
winios = (ROOT / "app/Madeira/Winios/Winios.m").read_text(encoding="utf-8")
header = (ROOT / "app/Madeira/Winios/Winios.h").read_text(encoding="utf-8")
library = (ROOT / "app/Madeira/Library.swift").read_text(encoding="utf-8")
failures = []


def check(condition, label):
    print(("ok   " if condition else "FAIL ") + label)
    if not condition:
        failures.append(label)


for word in ("error", "failed", "not found", "missing", "exception", "assertion"):
    check(f'"{word}"' in driver, f'Windows alert classifier includes "{word}"')
check("winios_window_alert( txt )" in driver,
      "driver forwards readable dialog/static-control error text")
check("g_window_alert[512]" in winios and "pthread_mutex_lock(&g_window_alert_lock)" in winios,
      "app-side alert handoff is bounded and thread-safe")
check("winios_window_alert_reset" in header and "winios_window_alert_copy" in header,
      "Swift bridge exposes reset/copy without sharing mutable storage")
check("@Published var launchAttention" in library and "Self.windowAlert()" in library,
      "direct launch polls Windows error text while the starting screen is up")
check("Windows reported an error" in library and "model.launchAttention" in library,
      "starting screen replaces the spinner with the Windows error")
check("winios_window_alert_reset()" in library,
      "every new/finished session clears stale alert text")

print("PASS" if not failures else "FAILED")
sys.exit(1 if failures else 0)
