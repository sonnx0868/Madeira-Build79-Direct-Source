"""Build 73 on verified build 72; update only row appearance instructions."""
from pathlib import Path
from zipfile import ZipFile
import plistlib
import subprocess
import sys

HERE = Path(__file__).resolve().parent
OUTPUTS = HERE.parents[1] / "outputs"
sys.path.insert(0, str(HERE / "source"))
from patch_control_row import patch_control_row

subprocess.run([sys.executable, HERE / "test_control_row.py"], check=True)
base, out = OUTPUTS / "Madeira-Light-Mode.ipa", OUTPUTS / "Madeira-Light-Control-Row.ipa"
exe, info = "Payload/Madeira.app/Madeira", "Payload/Madeira.app/Info.plist"
legal = "Payload/Madeira.app/legal/ResolutionSource/"
sources = {legal + name: (HERE / "source" / name).read_bytes() for name in ("patch_control_row.py", "row_keyboard_text.s")}
sources.update({legal + name: (HERE / name).read_bytes() for name in ("build_control_row.py", "test_control_row.py")})
with ZipFile(base) as old, ZipFile(out, "w") as new:
    assert plistlib.loads(old.read(info))["CFBundleVersion"] == "72"
    patched = patch_control_row(old.read(exe))
    for item in old.infolist():
        data = old.read(item)
        if item.filename == exe:
            data = patched
        elif item.filename == info:
            metadata = plistlib.loads(data)
            metadata["CFBundleVersion"] = "73"
            data = plistlib.dumps(metadata, fmt=plistlib.FMT_BINARY, sort_keys=True)
        new.writestr(item, data)
    for name, data in sources.items():
        assert name not in old.namelist()
        new.writestr(name, data)
with ZipFile(out) as check, ZipFile(base) as old:
    assert check.testzip() is None and check.read(exe) == patched
    for name in old.namelist():
        if name not in (exe, info):
            assert check.read(name) == old.read(name), name
    metadata = plistlib.loads(check.read(info))
    assert metadata["CFBundleVersion"] == "73" and metadata["CFBundleDisplayName"] == "Madeira"
print("PASS: build 73 ZIP, only inspected row appearance/executable and version changed; all libraries/resources retained")
print(out)
