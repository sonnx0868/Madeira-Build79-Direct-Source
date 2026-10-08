#!/usr/bin/env python3
"""Exercise real cache identity, bounded compiler scheduler and shutdown lifetime."""
import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("identity", root / "tools/shader-compiler-identity.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
base = [("shader.cpp", b"compiler"), ("llvm.a", b"archive")]
assert module.identity(base) == module.identity(list(reversed(base)))
assert module.identity(base) != module.identity(base + [("sdk.h", b"new")])
assert module.identity(base) != module.identity([(base[0][0], b"changed"), base[1]])
assert module.identity([("a", b"bc")]) != module.identity([("ab", b"c")])
with tempfile.TemporaryDirectory(prefix="madeira-identity-") as scratch:
    folder = Path(scratch)
    for name in ("dxmt/src/airconv", "dxmt/src/dxbc_parser", "dxmt/include", "madeira-d3d12/src"):
        p = folder / name / "compiler.h"; p.parent.mkdir(parents=True); p.write_bytes(b"source")
    for name in ("build/dxmt-ios/build.sh", "build/madeira_cfg.h", "build/llvm-ios/static-libs.txt",
                 "dxmt/src/d3d9/d3d9_shader.cpp", "dxmt/src/d3d11/d3d11_shader.cpp",
                 "dxmt/src/d3d11/d3d11_shader.hpp", "dxmt/src/dxmt/dxmt_shader_cache.hpp"):
        p = folder / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_text("Core\n" if name.endswith(".txt") else "flags")
    llvm = folder / "llvm"; llvm.mkdir(); (llvm / "libCore.a").write_bytes(b"archive")
    sdk = folder / "sdk"; sdk.mkdir(); (sdk / "converter.h").write_bytes(b"sdk")
    out = folder / "generated.h"
    command = [sys.executable, str(root / "tools/shader-compiler-identity.py"), "--root", str(folder),
               "--output", str(out), "--llvm-libs", str(llvm), "--converter-include", str(sdk), "--toolchain", "clang -O2"]
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    first, mtime = out.read_bytes(), out.stat().st_mtime_ns
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    assert out.read_bytes() == first and out.stat().st_mtime_ns == mtime
    # Changing an actual linked LLVM archive must invalidate existing entries.
    (llvm / "libCore.a").write_bytes(b"changed compiler")
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    assert out.read_bytes() != first
print("PASS: production compiler identity is deterministic; changed source/SDK/LLVM inputs invalidate it")

# A clean pinned DXMT checkout must receive every fix from the tracked patch.
patch_files = ["src/d3d9/d3d9_shader.cpp", "src/d3d11/d3d11_pipeline_cache.cpp", "src/dxmt/dxmt_shader_cache.hpp",
               "src/dxmt/dxmt_tasks.hpp", "src/winemetal/unix/cache.c", "src/winemetal/unix/winemetal_unix.c"]
with tempfile.TemporaryDirectory(prefix="madeira-clean-dxmt-") as scratch:
    folder = Path(scratch)
    for name in patch_files:
        p = folder / name; p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(subprocess.check_output(["git", "-C", str(root / "dxmt"), "show", "HEAD:" + name]))
    patch = str(root / "patches/dxmt-gameplay-performance.patch")
    subprocess.run(["git", "-C", str(folder), "apply", patch], check=True)
    subprocess.run(["git", "-C", str(folder), "apply", "--reverse", "--check", patch], check=True)
    for name in patch_files:
        assert (folder / name).read_text(encoding="utf-8") == (root / "dxmt" / name).read_text(encoding="utf-8"), name
print("PASS: clean pinned DXMT sources reproduce all fixes; patch reapplication is detectable")

tasks = (root / "dxmt/src/dxmt/dxmt_tasks.hpp").read_text(encoding="utf-8")
pipeline = (root / "dxmt/src/d3d11/d3d11_pipeline_cache.cpp").read_text(encoding="utf-8")
runtime = (root / "app/Madeira/GameRuntime.swift").read_text(encoding="utf-8")
view = (root / "app/Madeira/ContentView.swift").read_text(encoding="utf-8")
cache = (root / "dxmt/src/winemetal/unix/cache.c").read_text(encoding="utf-8")
assert "~PipelineCache() override" in pipeline and "scheduler_.shutdown();" in pipeline
assert "!requiresDirectLaunch(entry)" in runtime and 'names.contains("unityplayer.dll")' in runtime
launch = view.split("private func startLibraryEntry", 1)[1].split("private func runWineFullSequence", 1)[0]
assert launch.index("GameRuntime.shared.hasEngine && !GameRuntime.supports(entry)") < launch.index("entry.configureLaunch()")
assert "dispatch_async(_queue" in cache and "LOCK_EX | LOCK_NB" in cache
print("PASS: shader workers stop before task destruction; incompatible reuse cannot bootstrap over a live engine")

compiler = os.environ.get("MD_RUNTIME_CXX") or shutil.which("c++")
cc = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc")
if not compiler or not cc:
    if "--require-tools" in sys.argv: raise SystemExit("C and C++ compilers required")
    print("SKIP: native scheduler/cache bound checks need MD_RUNTIME_CC and MD_RUNTIME_CXX")
    sys.exit(0)

# Substitute only platform wrappers; compile the unchanged production scheduler.
body = tasks[tasks.index("namespace dxmt {"):]
guard_header = (root / "dxmt/src/dxmt/dxmt_shader_cache.hpp").read_text(encoding="utf-8")
guard = guard_header.split("  template <typename T> class LockProtected", 1)[1].split("  static ShaderCache", 1)[0]
guard = "struct GuardFixture { template <typename T> class LockProtected" + guard + "};\n"
shim = r'''
#include <algorithm>
#include <atomic>
#include <cassert>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#define DXMT_IOS 1
inline void *GetCurrentThread() { return nullptr; }
inline void SetThreadPriority(void *, int priority) { assert(priority == 0); }
namespace dxmt {
using mutex = std::mutex;
using condition_variable = std::condition_variable;
using thread = std::thread;
namespace env { inline std::string getEnvVar(const char *) { return "1"; } }
namespace str { template<typename T> std::string format(const char *, T) { return "test"; } }
struct Logger { static void info(const std::string &) {} };
}
'''
checks = r'''
struct Work { std::atomic<bool> done{false}; };
static std::atomic<unsigned> entered{0}, finished{0}, active{0}, peak{0};
static std::atomic<bool> release_work{false};
namespace dxmt {
template<> struct task_trait<Work *> {
    Work *run_task(Work *work) {
        unsigned n = ++active;
        unsigned old = peak.load();
        while (old < n && !peak.compare_exchange_weak(old, n)) {}
        ++entered;
        while (!release_work.load()) std::this_thread::yield();
        --active; ++finished; return work;
    }
    bool get_done(Work *work) { return work->done.load(); }
    void set_done(Work *work) { work->done.store(true); }
};
}
int main() {
    std::mutex cache_mutex;
    std::optional<GuardFixture::LockProtected<int>> moved;
    {
        GuardFixture::LockProtected<int> original(cache_mutex, 42);
        moved.emplace(std::move(original));
    }
    assert(!cache_mutex.try_lock()); // moved-from destructor must not unlock it
    moved.reset();
    assert(cache_mutex.try_lock());
    cache_mutex.unlock();
    assert(dxmt::compiler_worker_limit(0, "") == 1);
    assert(dxmt::compiler_worker_limit(16, "") == 4);
    assert(dxmt::compiler_worker_limit(2, "") == 2);
    assert(dxmt::compiler_worker_limit(6, "8") == 8);
    for (auto setting : {"0", "-1", "9", "999999999999", "2junk", " 2"})
        assert(dxmt::compiler_worker_limit(6, setting) == 4);
    Work work;
    dxmt::task_scheduler<Work *> scheduler;
    scheduler.submit(&work);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!entered.load() && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
    assert(entered.load() == 1 && !work.done.load());
    std::thread release([] { std::this_thread::sleep_for(std::chrono::milliseconds(50)); release_work.store(true); });
    scheduler.shutdown(); // must join the worker before work can be destroyed
    assert(work.done.load() && finished.load() == 1 && active.load() == 0 && peak.load() == 1);
    scheduler.shutdown(); // destructor / explicit shutdown are idempotent
    release.join();
    std::cout << "PASS: actual scheduler bounds, malformed override fallback and live-worker shutdown barrier\n";
}
'''
reservation = cache.split("#define CACHE_PENDING_LIMIT", 1)[1].split("@implementation CacheWriter", 1)[0]
reservation = "#define CACHE_PENDING_LIMIT" + reservation
c_checks = r'''
int main(void) {
    _Atomic size_t pending = 0;
    assert(reserve_cache_bytes(&pending, CACHE_PENDING_LIMIT - 1));
    assert(!reserve_cache_bytes(&pending, 2));
    assert(reserve_cache_bytes(&pending, 1));
    assert(!reserve_cache_bytes(&pending, 1));
    atomic_fetch_sub(&pending, CACHE_PENDING_LIMIT);
    assert(reserve_cache_bytes(&pending, 128));
    assert(!reserve_cache_bytes(&pending, SIZE_MAX));
    assert(atomic_load(&pending) == 128);
    puts("PASS: production async cache bound cannot overflow or grow past 32MB");
}
'''
with tempfile.TemporaryDirectory(prefix="madeira-startup-native-") as scratch:
    folder = Path(scratch)
    for name, compiler_path, source_text, flags in [
        ("scheduler", compiler, shim + body + guard + checks, ["-std=c++20"]),
        ("reservation", cc, "#include <stdatomic.h>\n#include <stdint.h>\n#include <stddef.h>\n#include <stdbool.h>\n#include <assert.h>\n#include <stdio.h>\n" + reservation + c_checks, ["-std=c11"]),
    ]:
        source = folder / (name + (".cpp" if name == "scheduler" else ".c"))
        binary = folder / (name + (".exe" if os.name == "nt" else ""))
        source.write_text(source_text, encoding="utf-8")
        platform_flags = ["-static"] if os.name == "nt" else ["-pthread"]
        subprocess.run([compiler_path, *flags, *platform_flags, "-O2", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)], check=True, timeout=60)
        subprocess.run([str(binary)], check=True, timeout=10)
