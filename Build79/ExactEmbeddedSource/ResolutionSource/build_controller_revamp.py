"""Build and host-test both injected libraries; no signing/device mutations."""
from pathlib import Path
import subprocess

SOURCE = Path(__file__).resolve().parent
BUILD = SOURCE.parent

def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)

for name in ("test_controller_revamp", "test_protocol", "test_override", "test_resolution_launch", "test_touch_state", "test_touch_controller_merge", "test_touch_layout", "test_touch_hit", "test_controller_game_detect"):
    run("clang", "-std=c11", "-D_GNU_SOURCE", "-O2", "-Wall", "-Wextra", "-Werror",
        "-pthread", "-I", SOURCE, SOURCE / (name + ".c"), "-lm", "-o", BUILD / name)
    run(BUILD / name)

run("clang", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-I", SOURCE,
    SOURCE / "test_controller_hotplug.c", SOURCE / "xinput/reader.c",
    "-lm", "-o", BUILD / "test_controller_hotplug")
run(BUILD / "test_controller_hotplug")

resource = subprocess.check_output(["clang", "-print-resource-dir"], text=True).strip()
flags = ("-target", "arm64-apple-ios17.0", "-ffreestanding", "-O2", "-Wall", "-Wextra",
    "-Werror", "-nostdinc", "-isystem", Path(resource) / "include", "-I", SOURCE,
    "-fPIC", "-fobjc-arc", "-fblocks", "-fno-math-errno")
for name in ("ControllerInput", "MadeiraIPadUI"):
    run("clang", *flags, "-c", SOURCE / (name + ".m"), "-o", BUILD / (name + ".o"))
run("clang", *flags, "-c", SOURCE / "ResolutionInterpose.m", "-o", BUILD / "ResolutionInterpose.o")
run("clang", *flags, "-c", SOURCE / "TouchControls.m", "-o", BUILD / "TouchControls.o")
link = ("-dylib", "-arch", "arm64", "-platform_version", "ios", "17.0", "17.0",
    "-undefined", "dynamic_lookup")
run("ld64.lld", *link, "-F", SOURCE / "stubs", "-framework", "GameController",
    "-install_name", "@executable_path/Frameworks/MadeiraControllerInput.dylib",
    "-o", BUILD / "MadeiraControllerInput.dylib", BUILD / "ControllerInput.o")
run("ld64.lld", *link, "-install_name", "@executable_path/Frameworks/MadeiraIPadUI.dylib",
    "-needed_library", BUILD / "MadeiraControllerInput.dylib", "-o", BUILD / "MadeiraIPadUI.dylib",
    BUILD / "MadeiraIPadUI.o", BUILD / "ResolutionInterpose.o", BUILD / "TouchControls.o")
run("python3", SOURCE / "verify_native_link.py")
print("Controller revamp libraries built; device/game verification still required.")
run("python3", SOURCE / "patch_resolution_binding.py")
run("python3", SOURCE / "patch_touch_backend.py")

# Rebuild guest resources as well as iOS input; inherited R6 DLLs are older.
GUEST = BUILD / "xinput"
GUEST.mkdir(exist_ok=True)
run("llvm-dlltool", "-m", "i386:x86-64", "-d", SOURCE / "xinput/kernel32.def", "-l", GUEST / "kernel32.lib")
for name in ("reader", "xinput"):
    run("clang", "-target", "x86_64-pc-windows-msvc", "-ffreestanding", "-fno-builtin",
        "-fno-stack-protector", "-funwind-tables", "-O2", "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-DMADEIRA_IGNORE_XINPUT_DISABLE=1", "-c", SOURCE / "xinput" / (name + ".c"),
        "-o", GUEST / (name + ".obj"))
for name in ("xinput1_3", "xinput1_4", "xinput9_1_0"):
    run("lld-link", "/dll", "/entry:DllMain", "/nodefaultlib", "/machine:x64", "/subsystem:windows",
        "/dynamicbase", "/nxcompat", "/opt:ref", "/opt:icf", "/timestamp:0",
        "/def:" + str(SOURCE / "xinput" / (name + ".def")), "/out:" + str(GUEST / (name + ".dll")),
        GUEST / "reader.obj", GUEST / "xinput.obj", GUEST / "kernel32.lib")
run("clang", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
    SOURCE / "xinput/reader.c", SOURCE / "xinput/test_reader.c", "-o", GUEST / "test_reader")
run(GUEST / "test_reader")
print("Guest DLLs rebuilt with Madeira clock/startup/focus compatibility.")
