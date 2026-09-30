#!/bin/bash
# Run on a known-good Mac build host after all native build stages complete.
# The resulting ZIP is uploaded to private/versioned storage for Codemagic.
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"
output="${1:-$repo_root/madeira-native-deps.zip}"

required=(
  FEX/build-ios/FEXCore/Source/libFEXCore.a
  FEX/build-ios/FEXCore/Source/libFEXCore_Base.a
  FEX/build-ios/FEXCore/Source/libJemallocLibs.a
  FEX/build-ios/External/fmt/libfmt.a
  FEX/build-ios/External/cephes/libcephes_128bit.a
  FEX/build-ios/External/xxhash/cmake_unofficial/libxxhash.a
  FEX/build-ios/External/SoftFloat-3e/libsoftfloat_3e.a
  app/Madeira/libwineserver.a
  app/Madeira/libntdll_unix.a
  app/Madeira/libwin32u_unix.a
  app/Madeira/libdxmt_combined.a
  app/Madeira/x86_64-vcruntime/concrt140.dll
  app/Madeira/x86_64-vcruntime/msvcp140.dll
  app/Madeira/x86_64-vcruntime/msvcp140_1.dll
  app/Madeira/x86_64-vcruntime/msvcp140_2.dll
  app/Madeira/x86_64-vcruntime/msvcp140_atomic_wait.dll
  app/Madeira/x86_64-vcruntime/msvcp140_codecvt_ids.dll
  app/Madeira/x86_64-vcruntime/vcamp140.dll
  app/Madeira/x86_64-vcruntime/vccorlib140.dll
  app/Madeira/x86_64-vcruntime/vcomp140.dll
  app/Madeira/x86_64-vcruntime/vcruntime140.dll
  app/Madeira/x86_64-vcruntime/vcruntime140_1.dll
  app/Madeira/x86_64-vcruntime/vcruntime140_threads.dll
)

for path in "${required[@]}"; do
    [[ -e "$repo_root/$path" ]] || {
        echo "Missing native build input: $path" >&2
        exit 1
    }
done

rm -f "$output"
(
    cd "$repo_root"
    zip -q -r "$output" "${required[@]}"
)

echo "Created $output"
shasum -a 256 "$output"
