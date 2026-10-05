#!/usr/bin/env python3
"""Static contract for direct-folder module-sidecar path repair."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
file_c = (root / "wine/dlls/ntdll/unix/file.c").read_text(encoding="utf-8")
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"folder-compat contract failed: {message}")


start = file_c.index("static BOOL ios_try_folder_module_sidecar")
end = file_c.index("/***********************************************************************", start)
repair = file_c[start:end]

require('getenv( "MADEIRA_FOLDER_COMPAT" )' in repair,
        "repair must be explicitly scoped to folder launches")
require("disposition != FILE_OPEN" in repair and "disposition != FILE_OVERWRITE" not in repair,
        "repair must never redirect creates or writes")
require("ios_folder_compat_sidecar" in repair,
        "repair must allow-list data/config sidecar extensions")
require("dllW" in repair and "dlW" in repair and "missing = 'l'" in repair,
        "repair must verify a complete or one-character-truncated module component")
require("S_ISREG(st.st_mode)" in repair and repair.count("S_ISREG(st.st_mode)") >= 2,
        "both the module and sibling sidecar must already be regular files")
require("[folder-compat] module-sidecar repaired" in repair,
        "every applied repair must be visible in the device log")
require("ios_try_folder_module_sidecar" in file_c[file_c.index("NTSTATUS get_nt_and_unix_names"):],
        "the repair must cover both attribute probes and subsequent file opens")

configure = library[library.index("func configureLaunch()"):library.index("\n    }", library.index("func configureLaunch()"))]
require('unsetenv("MADEIRA_FOLDER_COMPAT")' in configure,
        "launch state must not leak into Dock, Desktop, or later sessions")
require('steamAppID == nil && desktop != true' in configure and
        'MadeiraConfig.get("env.MADEIRA_FOLDER_COMPAT") != "0"' in configure and
        'setenv("MADEIRA_FOLDER_COMPAT", "1", 1)' in configure,
        "only directly imported folder-library games should enable the repair, with a kill switch")

docs = (root / "docs/LIBRARY.md").read_text(encoding="utf-8")
require("[folder-compat]" in docs and "env.MADEIRA_FOLDER_COMPAT = 0" in docs,
        "the bounded fallback and strict-path kill switch must be documented")

print("folder-compat contract: ok")
