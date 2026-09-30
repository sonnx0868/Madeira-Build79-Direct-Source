"""Reject unresolved Madeira-owned classes/functions before packaging.

Apple SDK imports intentionally use dynamic lookup in this Linux build. That
must never hide a missing implementation of one of our own Objective-C classes.
"""
from pathlib import Path
import re
import subprocess

SOURCE = Path(__file__).resolve().parent
BUILD = SOURCE.parent


def symbols(path, option):
    output = subprocess.check_output(["llvm-nm", "-g", option, str(path)], text=True)
    return {line.split()[-1] for line in output.splitlines() if line.strip()}


def unresolved_owned(undefined, defined):
    prefixes = ("_Madeira", "_madeira", "_OBJC_CLASS_$_Madeira", "_OBJC_METACLASS_$_Madeira")
    return {name for name in undefined if name.startswith(prefixes) and name not in defined}


def verify():
    paths = [BUILD / "MadeiraIPadUI.dylib", BUILD / "MadeiraControllerInput.dylib"]
    defined_by_library = [symbols(path, "--defined-only") for path in paths]
    defined = set().union(*defined_by_library)
    undefined = set().union(*(symbols(path, "--undefined-only") for path in paths))
    missing = unresolved_owned(undefined, defined)
    classes = set()
    for source in SOURCE.glob("*.m"):
        classes.update(re.findall(r"@interface\s+(Madeira\w+)\s*:", source.read_text()))
    missing.update("_OBJC_CLASS_$_" + name for name in classes
                   if "_OBJC_CLASS_$_" + name not in defined)
    if "_mdrenv" not in defined_by_library[0]:
        missing.add("_mdrenv")
    if missing:
        raise SystemExit("Missing native implementation/export: " + ", ".join(sorted(missing)))
    # Regression: the exact build-62 loader failure must always be rejected.
    class_symbol = "_OBJC_CLASS_$_MadeiraTouchSuppression"
    assert unresolved_owned({class_symbol}, set()) == {class_symbol}
    assert not unresolved_owned({class_symbol}, {class_symbol})
    assert not unresolved_owned({"_OBJC_CLASS_$_UIView", "_setenv"}, set())
    print(f"PASS: {len(classes)} Madeira classes implemented; owned imports resolved; resolution alias exported")


if __name__ == "__main__":
    verify()
