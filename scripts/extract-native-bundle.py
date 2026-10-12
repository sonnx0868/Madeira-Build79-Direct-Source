#!/usr/bin/env python3
"""Select a hash-verified native bundle from a Bitrise deployment container."""
import hashlib
import os
from pathlib import Path
import re
import sys
import tempfile
import zipfile

MEMBER = "madeira-v0.1.6-native-deps.zip"


def extract(container, destination, expected, max_size=256 * 1024 * 1024):
    if not re.fullmatch(r"[0-9a-fA-F]{64}", expected):
        raise ValueError("Expected inner bundle SHA256 is required")
    destination = Path(destination)
    with zipfile.ZipFile(container) as archive:
        matches = [i for i in archive.infolist() if i.filename == MEMBER]
        if len(matches) != 1 or not 0 < matches[0].file_size <= max_size:
            raise ValueError("Deployment container must contain one bounded native bundle")
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=destination.parent, prefix="madeira-native-", delete=False) as out:
                temporary = Path(out.name)
                sha = hashlib.sha256(); total = 0
                with archive.open(matches[0]) as source:
                    for chunk in iter(lambda: source.read(1024 * 1024), b""):
                        total += len(chunk)
                        if total > max_size: raise ValueError("Inner bundle exceeds limit")
                        sha.update(chunk); out.write(chunk)
            if total != matches[0].file_size or sha.hexdigest() != expected.lower():
                raise ValueError("Inner native bundle SHA256 mismatch")
            os.replace(temporary, destination)
            temporary = None
        finally:
            if temporary is not None: temporary.unlink(missing_ok=True)


if __name__ == "__main__":
    if len(sys.argv) != 4: raise SystemExit("Usage: extract-native-bundle.py container.zip native.zip sha256")
    extract(*sys.argv[1:])
    print("Verified native bundle extracted from Bitrise artifact container")
