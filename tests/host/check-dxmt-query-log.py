#!/usr/bin/env python3
"""DXMT optional QueryInterface warning must never crash ARM64EC games."""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
PATCH = ROOT / "patches/dxmt-madeira-query-log.patch"
TOOL = ROOT / "tools/patch-dxmt-query-log.py"
failures = []


def check(condition, label):
    print(("ok   " if condition else "FAIL ") + label)
    if not condition:
        failures.append(label)


text = PATCH.read_text(encoding="utf-8")
check("#ifndef DXMT_MADEIRA" in text and "return false;" in text,
      "Madeira source path bypasses only QueryInterface warning deduplication")
check("E_NOINTERFACE" in text and "g_loggedQueryInterfaceErrors" in text,
      "source patch documents unchanged COM failure semantics")

apply = subprocess.run(["git", "apply", "--check", "--directory=dxmt", str(PATCH)],
                       cwd=ROOT, capture_output=True, text=True)
check(apply.returncode == 0, "source patch applies to pinned DXMT")
if apply.returncode:
    print(apply.stderr.strip())

binary = subprocess.run([sys.executable, str(TOOL), "--check"], cwd=ROOT,
                        capture_output=True, text=True)
print(binary.stdout, end="")
check(binary.returncode == 0, "tracked d3d11/dxgi PE modules carry the source fix")

bootstrap = (ROOT / "scripts/bootstrap-native-deps.sh").read_text(encoding="utf-8")
restore = (ROOT / "scripts/restore-native-deps.sh").read_text(encoding="utf-8")
preflight = (ROOT / "scripts/check-ios-build.sh").read_text(encoding="utf-8")
check(PATCH.name in bootstrap, "bootstrap applies the canonical DXMT source patch")
check(TOOL.name in bootstrap and TOOL.name in restore and "--check" in preflight,
      "bootstrap, cached restore and preflight enforce the PE patch")

print("PASS" if not failures else "FAILED")
sys.exit(1 if failures else 0)
