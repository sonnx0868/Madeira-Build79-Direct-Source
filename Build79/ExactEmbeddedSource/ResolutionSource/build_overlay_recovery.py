"""Build owned UI updates while preserving the executable and audio fix."""
from pathlib import Path
from zipfile import ZipFile
import plistlib
import subprocess
import sys

HERE = Path(__file__).resolve().parent
SOURCE = HERE / "source"
OUTPUTS = HERE.parents[1] / "outputs"
light_mode = "--light-mode" in sys.argv
tag = "LightMode" if light_mode else "OverlayRecovery"
version = "72" if light_mode else "71"
sys.path.insert(0, str(SOURCE))
from verify_native_link import symbols, unresolved_owned

def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)

run("clang", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
    SOURCE / "test_window_layers.c", "-lm", "-o", HERE / "test_window_layers")
run(HERE / "test_window_layers")
resource = subprocess.check_output(["clang", "-print-resource-dir"], text=True).strip()
obj = HERE / f"MadeiraIPadUI-{tag}.o"
run("clang", "-target", "arm64-apple-ios17.0", "-ffreestanding", "-O2",
    "-Wall", "-Wextra", "-Werror", "-nostdinc", "-isystem", Path(resource) / "include",
    "-I", SOURCE, "-fPIC", "-fobjc-arc", "-fblocks", "-fno-math-errno",
    "-c", SOURCE / "MadeiraIPadUI.m", "-o", obj)
library = HERE / f"MadeiraIPadUI-{tag}.dylib"
run("ld64.lld", "-dylib", "-arch", "arm64", "-platform_version", "ios", "17.0", "17.0",
    "-undefined", "dynamic_lookup", "-install_name", "@executable_path/Frameworks/MadeiraIPadUI.dylib",
    "-needed_library", HERE / "MadeiraControllerInput.dylib", "-o", library,
    obj, HERE / "ResolutionInterpose.o", HERE / "TouchControls.o", HERE / "AudioDiagnostics.o")
defined = symbols(library, "--defined-only") | symbols(HERE / "MadeiraControllerInput.dylib", "--defined-only")
assert not unresolved_owned(symbols(library, "--undefined-only"), defined)
assert "_MadeiraAudioProperty" in defined
assert b"IOBufferDuration\0" in library.read_bytes() and b"ioBufferDuration\0" not in library.read_bytes()
base = OUTPUTS / ("Madeira-Overlay-Recovery.ipa" if light_mode else "Madeira-Audio-Period-Guarded.ipa")
out = OUTPUTS / ("Madeira-Light-Mode.ipa" if light_mode else "Madeira-Overlay-Recovery.ipa")
ui = "Payload/Madeira.app/Frameworks/MadeiraIPadUI.dylib"
info = "Payload/Madeira.app/Info.plist"
exe = "Payload/Madeira.app/Madeira"
legal = "Payload/Madeira.app/legal/ResolutionSource/"
replacements = {legal + name: (SOURCE / name).read_bytes()
                for name in ("MadeiraIPadUI.m", "Platform.h", "WindowLayers.h", "test_window_layers.c")}
replacements[legal + "build_overlay_recovery.py"] = Path(__file__).read_bytes()
with ZipFile(base) as old:
    retained = "MadeiraIPadUI-OverlayRecovery.dylib" if light_mode else "MadeiraIPadUI-AudioPeriodGuarded.dylib"
    assert old.read(ui) == (HERE / retained).read_bytes()
    assert plistlib.loads(old.read(info))["CFBundleVersion"] == ("71" if light_mode else "70")
    original_exe = old.read(exe)
    with ZipFile(out, "w") as new:
        for item in old.infolist():
            data = old.read(item)
            if item.filename == ui:
                data = library.read_bytes()
            elif item.filename == info:
                metadata = plistlib.loads(data)
                metadata["CFBundleVersion"] = version
                data = plistlib.dumps(metadata, fmt=plistlib.FMT_BINARY, sort_keys=True)
            elif item.filename in replacements:
                data = replacements.pop(item.filename)
            new.writestr(item, data)
        for name, data in replacements.items():
            new.writestr(name, data)
with ZipFile(out) as check, ZipFile(base) as old:
    assert check.testzip() is None and check.read(exe) == original_exe
    assert check.read(ui) == library.read_bytes()
    metadata = plistlib.loads(check.read(info))
    assert metadata["CFBundleVersion"] == version and metadata["CFBundleDisplayName"] == "Madeira"
    allowed = {ui, info, legal + "MadeiraIPadUI.m", legal + "Platform.h", legal + "build_overlay_recovery.py"}
    for name in old.namelist():
        if name not in allowed:
            assert check.read(name) == old.read(name), name
print("PASS: native owned-symbol link, audio selector, unchanged executable and other resources, ZIP verification")
print(out)
