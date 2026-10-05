#!/usr/bin/env python3
"""Direct-folder Unity games get bounded, visible performance defaults."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
docs = (root / "docs/LIBRARY.md").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"unity folder profile failed: {message}")


require("var unityOptimizations: Bool?" in library and 'Toggle("Unity smoothness profile"' in library,
        "the optimization is not a visible per-game choice")
require('else if unityProfile { setenv("MADEIRA_CPU_COUNT", "4", 1) }' in library,
        "Unity's worker fan-out is not bounded when CPU count is automatic")
require('MadeiraConfig.get("env.MADEIRA_MIP_CLAMP_AUTO") == nil' in library and
        'setenv("MADEIRA_MIP_CLAMP_AUTO", "1", 1)' in library,
        "pressure-aware texture control is absent or overwrites an explicit setting")
require('MadeiraConfig.flag("MADEIRA_UNITY_OPTIMIZATIONS")' in library,
        "there is no global kill switch")
require("four CPU cores" in docs and "pressure-aware mip clamp" in docs,
        "the quality/performance tradeoff is not documented")

print("Unity direct-folder profile contract: ok")
