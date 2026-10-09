#!/usr/bin/env python3
"""Compile the ARM64EC frontend sources without requiring a Mac/Metal toolchain.

Generated shader payload bytes are placeholders. This verifies compilation and
native-only symbol isolation, not Metal execution or the final Wine DLL link.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
dxmt = root / "dxmt"


def sources(relative, variable):
    path = dxmt / relative
    text = "\n".join(line.split("#", 1)[0] for line in path.read_text(encoding="utf-8").splitlines())
    match = re.search(r"\b" + re.escape(variable) + r"\s*=\s*(?:files\s*\()?\s*\[([\s\S]*?)\]", text)
    if not match:
        raise RuntimeError("Missing Meson source list: " + variable)
    return [(path.parent / name).resolve() for name in re.findall(r"'([^']+\.(?:cpp|c))'", match[1])]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler")
    parser.add_argument("--require-tools", action="store_true")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    compiler = args.compiler or os.environ.get("MD_RUNTIME_ARM64EC_CXX") or shutil.which("arm64ec-w64-mingw32-clang++")
    if not compiler:
        candidate = root / "toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin/arm64ec-w64-mingw32-clang++"
        if candidate.exists(): compiler = str(candidate)
    if not compiler:
        if args.require_tools: raise SystemExit("ARM64EC compiler required")
        print("SKIP: ARM64EC source compilation needs llvm-mingw")
        return
    lists = [("src/util/meson.build", "util_src"), ("src/dxmt/meson.build", "dxmt_src"),
             ("src/dxgi/meson.build", "dxgi_src"), ("src/d3d11/meson.build", "d3d11_src"),
             ("src/d3d11/meson.build", "d3d10_src"), ("src/d3d9/meson.build", "d3d9_src"),
             ("src/winemetal/meson.build", "winemetal_src"), ("libs/DXBCParser/meson.build", "dxbc_parser_src")]
    files = sorted(set(path for relative, variable in lists for path in sources(relative, variable)))
    files += [dxmt / "src/util" / name for name in
              ("wsi_monitor_headless.cpp", "wsi_window_headless.cpp", "wsi_platform_win32.cpp")]
    with tempfile.TemporaryDirectory(prefix="madeira-arm64ec-check-") as scratch:
        generated = Path(scratch)
        (generated / "dxmt_command.h").write_text("unsigned char dxmt_command[] = {0};\nunsigned int dxmt_command_len = 1;\n")
        (generated / "version.h").write_text('#define DXMT_VERSION "compile-check"\n')
        includes = [generated, dxmt / "include", dxmt / "libs"]
        includes += [dxmt / "src" / name for name in ("util", "airconv", "winemetal", "dxmt", "dxgi", "d3d11", "d3d10", "d3d9")]
        flags = ["-fsyntax-only", "-DDXMT_IOS=1", "-DDXMT_PAGE_SIZE=4096", "-DNOMINMAX", "-D_WIN32_WINNT=0xa00",
                 "-D_FILE_OFFSET_BITS=64", "-DNDEBUG", "-fblocks", "-Wno-extern-c-compat", "-Wno-microsoft-exception-spec"]
        flags += ["-I" + str(p) for p in includes]
        def compile_one(path):
            language = ["-x", "c", "-std=c11"] if path.suffix == ".c" else ["-std=c++20"]
            result = subprocess.run([compiler, *flags, *language, str(path)], capture_output=True, text=True, timeout=120)
            return path, result
        failures = []
        with ThreadPoolExecutor(max_workers=max(1, min(args.jobs, 8))) as pool:
            for path, result in pool.map(compile_one, files):
                if result.returncode:
                    failures.append(path)
                    print("FAILED: " + str(path.relative_to(dxmt)), flush=True)
                    print(result.stderr[-5000:], flush=True)
        if failures:
            raise SystemExit(f"{len(failures)} of {len(files)} ARM64EC translation units failed")
        # Preprocessing the real PE source must not request the native source
        # header or call a function defined only in the iOS static archive.
        output = subprocess.check_output([compiler, *[f for f in flags if f != "-fsyntax-only"],
            "-std=c++20", "-E", str(dxmt / "src/dxmt/dxmt_command.cpp")], text=True)
        assert "madeira_mtl_new_library_source" not in output
        assert "dxmt_command_source_len" not in output
        print(f"PASS: {len(files)} real ARM64EC frontend translation units compile; native Metal fallback is excluded")


if __name__ == "__main__":
    main()
