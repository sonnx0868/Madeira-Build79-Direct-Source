#!/usr/bin/env python3
"""Production lifecycle/packet tests + Windows-only real parent/child smoke.

Mach dependencies are substituted; the actual C policy is compiled unchanged.
This does not simulate FEX/iPadOS. Codemagic also requires the real Swift codec.
"""
from pathlib import Path
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parents[2]
runtime = (root / "app/Madeira/GameRuntime.swift").read_text(encoding="utf-8")
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
view = (root / "app/Madeira/ContentView.swift").read_text(encoding="utf-8")
server = (root / "build/ntdll-unix/server_ios.c").read_text(encoding="utf-8")
assert 'GameRuntime.shared.hasEngine { startLibraryEntry(entry); return }' in view
assert "runtimeLaunch.restoreEnvironment(); entry.applyEnvironment(); try runtimeLaunch.publish()" in view
assert "!GameRuntime.shared.ownsSession" in library and "GameRuntime.shared.ended" in library
assert "wine_runtime_prepare_process_exit" in server and "if (runtime_safe) ios_jit_reclaim_process" in server
assert '"control.bin"' in runtime and '"request.bin"' in runtime
assert "stoppedConfirmed" in runtime and "wine_runtime_reuse_ready()" in runtime
assert 'if wineserver_is_running() != 0 { return profile.name }' in library
finish = library.split("private func finish() {", 1)[1].split("enum LibraryError", 1)[0]
actor_body = finish.split("Task { @MainActor in\n", 1)[1].split("\n            }", 1)[0]
assert "SteamOwnedLibrary.shared.sessionChanged(active: false)" in actor_body
assert "LibraryModel.shared.current == nil" in actor_body and "!GameRuntime.shared.ownsSession" in actor_body
print("PASS: real warm-launch route, orderly Quit/home, native retirement barrier, controls and live-registry wiring")

compiler = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc")
if not compiler:
    if "--require-tools" in sys.argv: raise SystemExit("A C compiler is required for runtime lifecycle tests")
    print("SKIP: C behavior needs MD_RUNTIME_CC/cc; Codemagic requires it")
else:
    with tempfile.TemporaryDirectory(prefix="madeira-runtime-check-") as scratch:
        folder = Path(scratch)
        suffix = ".exe" if os.name == "nt" else ""
        lifecycle = folder / ("lifecycle" + suffix)
        subprocess.run([compiler, "-x", "c", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
            "-DMADEIRA_RUNTIME_UNIT_TEST", "-I" + str(root / "tests/host"),
            str(root / "app/Madeira/GameRuntimeBridge.m"), str(root / "tests/host/runtime-lifecycle-test.c"),
            "-o", str(lifecycle)], check=True)
        subprocess.run([str(lifecycle)], check=True)
        subprocess.run([str(lifecycle), "unsafe"], check=True)
        protocol = folder / ("protocol" + suffix)
        subprocess.run([compiler, "-std=c11", "-O2", "-Wall", "-Werror",
            str(root / "tests/host/runtime-protocol-test.c"), "-o", str(protocol)], check=True)
        subprocess.run([str(protocol)], check=True)
        if os.name == "nt":
            # Both executables are our test outputs; no user game/client is run.
            host = folder / "host.exe"
            fixture = folder / "fixture game & unicode.exe"
            for source, output, flags in [(root / "build/session-host/main.c", host, ["-DMD_RUNTIME_NATIVE_TEST", "-ladvapi32", "-luser32"]),
                                           (root / "tests/host/runtime-game-fixture.c", fixture, [])]:
                subprocess.run([compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-static", "-municode", "-mwindows",
                                str(source), *flags, "-o", str(output)], check=True)
            channel = folder / "channel"; channel.mkdir()
            child = subprocess.Popen([str(host), str(channel)], creationflags=subprocess.CREATE_NO_WINDOW,
                                     stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            def status():
                try:
                    data = (channel / "status.bin").read_bytes()
                    if len(data) == 40 and data[:8] == b"MDSTAT01": return struct.unpack("<Q6I", data[8:])
                except (OSError, ValueError): pass
                return None
            def wait_for(predicate, seconds=8):
                deadline = time.monotonic() + seconds
                while time.monotonic() < deadline:
                    current = status()
                    if current and predicate(current): return current
                    if child.poll() is not None: raise AssertionError("persistent host exited during game switching")
                    time.sleep(0.02)
                raise AssertionError(f"runtime state did not arrive: {status()}")
            def start(generation, mode, env):
                strings = [str(fixture), f'"{fixture}" {mode}', str(folder), f"MD_TEST_VALUE={env}\0", "", "\0"]
                fields = [(s + "\0").encode("utf-16le") for s in strings]
                packet = b"MDRUN001" + struct.pack("<QI6I", generation, 1, *(len(f) for f in fields)) + b"".join(fields)
                temporary = channel / "request.tmp"; temporary.write_bytes(packet); temporary.replace(channel / "request.bin")
            try:
                idle = wait_for(lambda s: s[1] == 0)
                start(1, "check-a", "A"); a = wait_for(lambda s: s[0] == 1 and s[1] == 3)
                assert a[2] == idle[2] and a[4] == 0 and a[6] == 0
                start(2, "check-b", "B"); b = wait_for(lambda s: s[0] == 2 and s[1] == 3)
                assert b[2] == idle[2] and b[3] != a[3] and b[4] == 7
                started = time.monotonic(); start(3, "tree", "C")
                tree = wait_for(lambda s: s[0] == 3 and s[1] == 3)
                assert tree[2] == idle[2] and time.monotonic() - started >= 0.4
                start(4, "delay", "D")
                wait_for(lambda s: s[0] == 4 and s[1] == 2)
                # A stale force request must not hit the new child.
                control = b"MDRUN001" + struct.pack("<QI6I", 3, 3, *([0] * 6))
                temporary = channel / "control.tmp"; temporary.write_bytes(control); temporary.replace(channel / "control.bin")
                time.sleep(0.2)
                assert status()[1] == 2
                control = b"MDRUN001" + struct.pack("<QI6I", 4, 3, *([0] * 6))
                temporary.write_bytes(control); temporary.replace(channel / "control.bin")
                forced = wait_for(lambda s: s[0] == 4 and s[1] == 3)
                assert forced[2] == idle[2] and forced[6] == 0
                print("PASS: real Windows host kept one PID across three launches, changed child PID/env, preserved SYSTEMROOT and waited for grandchild exit")
                print("PASS: stale force cannot stop a later game; current-generation force ends only its job, not the host")
            finally:
                child.terminate(); child.communicate(timeout=5)

swift = shutil.which("swiftc")
if not swift:
    if "--require-swift" in sys.argv: raise SystemExit("swiftc is required for the runtime codec tests")
    print("SKIP: Swift behavior needs swiftc; Codemagic requires it")
else:
    codec = runtime.split("/// Immutable launch descriptor", 1)[0].replace("import Combine\n", "")
    checks = r'''
enum SupportError: Error { case message(String) }
let data = try RuntimeWire.command(generation: 1, operation: 2)
assert(data.count == 44 && data.prefix(8) == Data("MDRUN001".utf8))
assert(RuntimeWire.quote("C:\\a b\\x.exe") == "\"C:\\a b\\x.exe\"")
assert(RuntimeWire.quote("a\\") == "\"a\\\\\"")
assert(RuntimeWire.quote("a\"b") == "\"a\\\"b\"")
do { _ = try RuntimeWire.command(generation: 0, operation: 1); fatalError() } catch {}
assert(RuntimeWire.status(Data()) == nil && RuntimeWire.status(Data(repeating: 0, count: 40)) == nil)
print("PASS: production Swift runtime command/quote/status codec")
'''
    # Compile the ACTUAL finish() task against actor-isolated dependencies.
    # A direct nonisolated call would fail this test as it failed Xcode build569.
    actor_checks = r'''
@MainActor final class SteamOwnedLibrary {
    static let shared = SteamOwnedLibrary()
    var calls: [Bool] = []
    func sessionChanged(active: Bool) { calls.append(active) }
}
@MainActor final class LibraryModel { static let shared = LibraryModel(); var current: Int? }
@MainActor final class GameRuntime { static let shared = GameRuntime(); var ownsSession = false }
func finishActorTask() -> Task<Void, Never> {
    Task { @MainActor in
''' + actor_body + r'''
    }
}
Task { @MainActor in
    await finishActorTask().value
    assert(SteamOwnedLibrary.shared.calls == [false])
    let stale = finishActorTask()
    LibraryModel.shared.current = 2
    await stale.value
    assert(SteamOwnedLibrary.shared.calls == [false])
    LibraryModel.shared.current = nil
    let preparing = finishActorTask()
    GameRuntime.shared.ownsSession = true
    await preparing.value
    assert(SteamOwnedLibrary.shared.calls == [false])
    print("PASS: production finish task respects MainActor and never resumes Steam over a newer or preparing game")
    exit(0)
}
dispatchMain()
'''
    with tempfile.TemporaryDirectory(prefix="madeira-runtime-swift-") as scratch:
        folder = Path(scratch); source = folder / "main.swift"; binary = folder / "check"
        source.write_text(codec + checks + actor_checks, encoding="utf-8")
        subprocess.run([swift, str(source), "-o", str(binary)], check=True, timeout=60)
        subprocess.run([str(binary)], check=True, timeout=60)
