#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Madeira Converter Exception: see LICENSE-EXCEPTION.md.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
mingw="${LLVM_MINGW:-$root/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin}"
# The supervisor is native ARM64. An x64 supervisor makes each x64 game
# clone a live ARM64EC ntdll; build570 failed restoring Mono's Finalizer
# registers on that route. A native root uses the fresh cross-arch EC loader.
compiler="$mingw/aarch64-w64-mingw32-clang"
test -x "$compiler" || { echo "Missing session-host compiler: $compiler" >&2; exit 1; }
stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -static -municode -mwindows \
  -Wl,--image-base,0x180000000 -Wl,--strip-all -Wl,--no-insert-timestamp \
  "$root/build/session-host/main.c" -ladvapi32 -luser32 -o "$stage/madeira-session-host.exe"
cp "$stage/madeira-session-host.exe" "$root/app/Madeira/aarch64-windows/"
