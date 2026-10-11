#!/usr/bin/env python3
"""Reproduce the host fmt import that broke Codemagic's Windows cross-build.

Leave the failed CMake cache in place, then run build/fex-arm64ec/build.sh:
the production builder must override the cached host package without deleting
the build directory. This check requires pinned FEX submodules and llvm-mingw.
"""
from pathlib import Path
import argparse, subprocess

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--build-dir', type=Path, default=root/'FEX/build-arm64ec-ios')
args = parser.parse_args()
build = args.build_dir.resolve()
package = build/'test-host-fmt'
package.mkdir(parents=True, exist_ok=True)
(package/'libfmt.dylib').write_bytes(b'host-library-fixture')
(package/'fmt-config.cmake').write_text('''
cmake_policy(SET CMP0111 NEW)
add_library(fmt::fmt SHARED IMPORTED)
set_target_properties(fmt::fmt PROPERTIES
    IMPORTED_CONFIGURATIONS RELEASE
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/libfmt.dylib")
set(fmt_FOUND TRUE)
''', encoding='utf-8')
command = ['cmake', '-S', str(root/'FEX'), '-B', str(build), '-G', 'Ninja',
    '-DCMAKE_BUILD_TYPE=Release',
    '-DCMAKE_TOOLCHAIN_FILE='+str(root/'FEX/Data/CMake/toolchain_mingw.cmake'),
    '-DMINGW_TRIPLE=arm64ec-w64-mingw32', '-DFEX_IOS_HOST_BUILD=ON',
    '-DCMAKE_C_FLAGS=-DFEX_IOS_HOST=1', '-DCMAKE_CXX_FLAGS=-DFEX_IOS_HOST=1',
    '-DTUNE_CPU=none', '-DTUNE_ARCH=generic', '-DENABLE_GUEST_WINDOW=OFF',
    '-DENABLE_LTO=OFF', '-DENABLE_OFFLINE_TELEMETRY=OFF', '-DENABLE_CCACHE=OFF',
    '-DBUILD_TESTING=OFF', '-DBUILD_THUNKS=OFF', '-DENABLE_ASSERTIONS=OFF',
    '-DCMAKE_DISABLE_FIND_PACKAGE_fmt=OFF', '-Dfmt_DIR='+str(package)]
result = subprocess.run(command, capture_output=True, text=True, timeout=180)
output = result.stdout + result.stderr
if result.returncode == 0 or 'IMPORTED_IMPLIB' not in output or 'fmt::fmt' not in output:
    print(output[-6000:])
    raise SystemExit('Host fmt fixture did not reproduce the reported generation failure')
cache = (build/'CMakeCache.txt').read_text(encoding='utf-8')
if package.as_posix() not in cache.replace('\\', '/'):
    raise SystemExit('The host fmt package was not retained in the failed CMake cache')
print('PASS: reproduced fmt::fmt IMPORTED_IMPLIB failure; cached host package retained for the production rebuild')
