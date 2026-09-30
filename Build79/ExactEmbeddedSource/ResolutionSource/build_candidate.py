"""Package the R6 UI resolution overlay into an unsigned, source-bearing IPA."""
from pathlib import Path
from zipfile import ZipFile
import hashlib
import plistlib
import sys

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE / "source"))
from patch_touch_backend import patch_touch_backend
from verify_native_link import verify
BASE = Path("/home/_ali/Downloads/Madeira-0.34-Compatibility-R6-Candidate.ipa")
OUT = ROOT / "outputs/Madeira-Controller-Game-Detection.ipa"
EXPECTED = "905a8f2e332ad9d43358319ba1517c4005b5b209bc0c692b184997a12a30f9e2"
UI = "Payload/Madeira.app/Frameworks/MadeiraIPadUI.dylib"
CONTROLLER = "Payload/Madeira.app/Frameworks/MadeiraControllerInput.dylib"
EXE = "Payload/Madeira.app/Madeira"
INFO = "Payload/Madeira.app/Info.plist"
ICONS = HERE / "icons"
# In the original arm64 SwiftUI executable, TouchControlsOverlay.topBar
# constructs its HStack immediately before this top padding instruction.
# The fat arm64 slice begins at file offset 0x4000; its __TEXT segment at
# 0x100000000 begins at slice offset zero. Move only that topBar down 10pt.
TOPBAR_PADDING_OFFSET = 0x4000 + 0x28e70
TOPBAR_10PT = bytes.fromhex("00 90 64 1e")
TOPBAR_20PT = bytes.fromhex("00 90 66 1e")

if hashlib.sha256(BASE.read_bytes()).hexdigest() != EXPECTED:
    raise SystemExit("R6 IPA differs from the inspected base")
dylib = HERE / "MadeiraIPadUI.dylib"
controller_dylib = HERE / "MadeiraControllerInput.dylib"
if not dylib.is_file():
    raise SystemExit("Build the resolution UI dylib first")
if not controller_dylib.is_file():
    raise SystemExit("Build the revamped native controller dylib first")
verify()
OUT.parent.mkdir(parents=True, exist_ok=True)
with ZipFile(BASE) as original, ZipFile(OUT, "w") as candidate:
    for item in original.infolist():
        data = original.read(item)
        if item.filename == UI:
            data = dylib.read_bytes()
        elif item.filename == CONTROLLER:
            data = controller_dylib.read_bytes()
        elif item.filename.startswith("Payload/Madeira.app/ControllerSupport/") and item.filename.endswith(".dll"):
            name = Path(item.filename).name
            candidate.writestr("Payload/Madeira.app/ControllerSupport/Legacy/" + name, data)
            data = (HERE / "xinput" / name).read_bytes()
        elif item.filename == EXE:
            data = patch_touch_backend(data)
            if data[TOPBAR_PADDING_OFFSET:TOPBAR_PADDING_OFFSET + 4] != TOPBAR_10PT:
                raise SystemExit("SwiftUI topBar padding instruction differs from inspected R6")
            data = data[:TOPBAR_PADDING_OFFSET] + TOPBAR_20PT + data[TOPBAR_PADDING_OFFSET + 4:]
        elif item.filename == INFO:
            info = plistlib.loads(data)
            info["CFBundleDisplayName"] = "Madeira"
            info["CFBundleName"] = "Madeira"
            info["CFBundleShortVersionString"] = "0.34.7"
            info["CFBundleVersion"] = "66"
            info["GCSupportsControllerUserInteraction"] = True
            info["GCSupportedGameControllers"] = [{"ProfileName": "ExtendedGamepad"}]
            info["CFBundleIcons"] = {"CFBundlePrimaryIcon": {"CFBundleIconFiles": ["Icon-Small-40", "Icon-60"]}}
            info["CFBundleIcons~ipad"] = {"CFBundlePrimaryIcon": {"CFBundleIconFiles": ["Icon-Small-40", "Icon-76", "Icon-83.5"]}}
            data = plistlib.dumps(info, fmt=plistlib.FMT_BINARY, sort_keys=True)
        candidate.writestr(item, data)
    for source in sorted((HERE / "source").rglob("*")):
        if source.is_file() and "__pycache__" not in source.parts and source.suffix != ".pyc":
            candidate.write(source, "Payload/Madeira.app/legal/ResolutionSource/" + source.relative_to(HERE / "source").as_posix())
    for icon in sorted(ICONS.iterdir()):
        if icon.is_file():
            candidate.write(icon, "Payload/Madeira.app/" + icon.name)
    candidate.write(Path(__file__), "Payload/Madeira.app/legal/ResolutionSource/build_candidate.py")
with ZipFile(OUT) as check:
    assert check.testzip() is None
    assert check.read(UI) == dylib.read_bytes()
    assert check.read(CONTROLLER) == controller_dylib.read_bytes()
    for name in ("xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll"):
        resource = "Payload/Madeira.app/ControllerSupport/" + name
        assert check.read(resource) == (HERE / "xinput" / name).read_bytes()
        with ZipFile(BASE) as base:
            assert check.read("Payload/Madeira.app/ControllerSupport/Legacy/" + name) == base.read(resource)
    assert plistlib.loads(check.read(INFO))["GCSupportsControllerUserInteraction"] is True
    assert plistlib.loads(check.read(INFO))["GCSupportedGameControllers"] == [{"ProfileName": "ExtendedGamepad"}]
    assert check.read(EXE)[TOPBAR_PADDING_OFFSET:TOPBAR_PADDING_OFFSET + 4] == TOPBAR_20PT
    assert check.read("Payload/Madeira.app/Icon-60@3x.png") == (ICONS / "Icon-60@3x.png").read_bytes()
    assert plistlib.loads(check.read(INFO))["CFBundleIcons"]["CFBundlePrimaryIcon"]["CFBundleIconFiles"][-1] == "Icon-60"
    assert not any("_CodeSignature" in name or name.endswith("embedded.mobileprovision")
                   for name in check.namelist())
print(OUT)
