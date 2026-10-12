#!/usr/bin/env python3
"""Artifact-container selection must preserve old output on verification errors."""
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import warnings
import zipfile
root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("native_wrapper", root / "scripts/extract-native-bundle.py")
module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
with tempfile.TemporaryDirectory(prefix="madeira-native-wrapper-") as scratch:
    tmp = Path(scratch); container = tmp / "artifacts.zip"; output = tmp / "native.zip"
    payload = b"verified fixture native bundle"; digest = hashlib.sha256(payload).hexdigest()
    with zipfile.ZipFile(container, "w") as z:
        z.writestr(module.MEMBER, payload); z.writestr("madeira-build.log", b"build log")
    module.extract(container, output, digest)
    assert output.read_bytes() == payload
    for expected, bound in [("0" * 64, 1024), (digest, 4), ("invalid", 1024)]:
        try: module.extract(container, output, expected, bound); raise AssertionError("accepted invalid input")
        except ValueError: pass
        assert output.read_bytes() == payload
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        with zipfile.ZipFile(container, "w") as z:
            z.writestr(module.MEMBER, payload); z.writestr(module.MEMBER, b"duplicate")
    try: module.extract(container, output, digest); raise AssertionError("accepted duplicate bundle")
    except ValueError: pass
    assert output.read_bytes() == payload
    with zipfile.ZipFile(container, "w") as z: z.writestr("../" + module.MEMBER, payload)
    try: module.extract(container, output, digest); raise AssertionError("accepted wrong member path")
    except ValueError: pass
print("PASS: native container hash, bounds, duplicate/member checks and atomic destination preservation")
