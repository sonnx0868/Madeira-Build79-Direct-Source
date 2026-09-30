"""Build a no-tone telemetry candidate without replacing build-66 artifacts."""
from pathlib import Path
from zipfile import ZipFile
import subprocess
import sys
import plistlib
import argparse
parser=argparse.ArgumentParser()
tests=parser.add_mutually_exclusive_group()
tests.add_argument("--clock-test",action="store_true")
tests.add_argument("--period-test",action="store_true")
options=parser.parse_args()
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
SOURCE=HERE/"source"
sys.path.insert(0,str(SOURCE))
from patch_audio_binding import patch_audio_binding
from verify_native_link import symbols,unresolved_owned
def run(*args):subprocess.run([str(a) for a in args],check=True)
run("clang","-std=c11","-O2","-Wall","-Wextra","-Werror",SOURCE/"test_audio_meter.c","-o",HERE/"test_audio_meter")
run(HERE/"test_audio_meter")
run("clang","-std=c11","-O2","-Wall","-Wextra","-Werror",SOURCE/"test_audio_clock.c","-o",HERE/"test_audio_clock")
run(HERE/"test_audio_clock")
run("clang","-std=c11","-O2","-Wall","-Wextra","-Werror",SOURCE/"test_audio_period.c","-o",HERE/"test_audio_period")
run(HERE/"test_audio_period")
resource=Path(subprocess.check_output(["clang","-print-resource-dir"],text=True).strip())
run("clang","-target","arm64-apple-ios17.0","-ffreestanding","-O2","-Wall","-Wextra","-Werror","-nostdinc","-isystem",resource/"include","-I",SOURCE,"-fPIC","-fobjc-arc","-fblocks","-DMADEIRA_AUDIO_DEVICE_CLOCK="+str(int(options.clock_test)),"-DMADEIRA_AUDIO_PERIOD_TEST="+str(int(options.period_test)),"-c",SOURCE/"AudioDiagnostics.m","-o",HERE/"AudioDiagnostics.o")
library=HERE/("MadeiraIPadUI-AudioPeriodGuarded.dylib" if options.period_test else "MadeiraIPadUI-AudioClockTest.dylib" if options.clock_test else "MadeiraIPadUI-AudioDiagnostic.dylib")
run("ld64.lld","-dylib","-arch","arm64","-platform_version","ios","17.0","17.0","-undefined","dynamic_lookup","-install_name","@executable_path/Frameworks/MadeiraIPadUI.dylib","-needed_library",HERE/"MadeiraControllerInput.dylib","-o",library,HERE/"MadeiraIPadUI.o",HERE/"ResolutionInterpose.o",HERE/"TouchControls.o",HERE/"AudioDiagnostics.o")
base=ROOT/"outputs/Madeira-Controller-Game-Detection.ipa"
out=ROOT/"outputs"/("Madeira-Audio-Period-Guarded.ipa" if options.period_test else "Madeira-Audio-Clock-Test.ipa" if options.clock_test else "Madeira-Audio-Diagnostic.ipa")
exe="Payload/Madeira.app/Madeira"; ui="Payload/Madeira.app/Frameworks/MadeiraIPadUI.dylib"; info="Payload/Madeira.app/Info.plist"
defined=symbols(library,"--defined-only")|symbols(HERE/"MadeiraControllerInput.dylib","--defined-only")
assert "_MadeiraAudioProperty" in defined
assert not unresolved_owned(symbols(library,"--undefined-only"),defined)
if options.period_test:
    binary=library.read_bytes()
    assert b"IOBufferDuration\0" in binary and b"ioBufferDuration\0" not in binary
    body=(SOURCE/"AudioDiagnostics.m").read_text()
    assert "respondsToSelector:@selector(IOBufferDuration)" in body
with ZipFile(base) as old:
    assert old.read(ui)==(HERE/"MadeiraIPadUI.dylib").read_bytes()
    original=old.read(exe); patched=patch_audio_binding(original)
    for bad in (b"",original[:-1],patched):
        try:patch_audio_binding(bad)
        except ValueError:pass
        else:raise AssertionError("Uninspected input accepted")
    with ZipFile(out,"w") as new:
        for item in old.infolist():
            data=old.read(item)
            if item.filename==exe:data=patched
            if item.filename==ui:data=library.read_bytes()
            if item.filename==info:
                metadata=plistlib.loads(data); metadata["CFBundleVersion"]="70" if options.period_test else "68" if options.clock_test else "67"
                data=plistlib.dumps(metadata,fmt=plistlib.FMT_BINARY,sort_keys=True)
            new.writestr(item,data)
        for name in ("AudioDiagnostics.m","AudioMeter.h","AudioClock.h","AudioPeriod.h","test_audio_period.c","test_audio_clock.c","test_audio_meter.c","patch_audio_binding.py"):
            new.write(SOURCE/name,"Payload/Madeira.app/legal/ResolutionSource/"+name)
        new.write(Path(__file__),"Payload/Madeira.app/legal/ResolutionSource/build_audio_diagnostic.py")
with ZipFile(out) as check:
    assert check.testzip() is None and check.read(exe)==patched and check.read(ui)==library.read_bytes()
    assert plistlib.loads(check.read(info))["CFBundleDisplayName"]=="Madeira"
print("PASS: exact main-image-only audio import, alias/native link, owned imports, bad-input guards and ZIP readback")
print(out)
