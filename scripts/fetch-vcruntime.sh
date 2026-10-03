#!/bin/bash
# Download and extract the unmodified Microsoft x64 VC runtime payloads.
set -euo pipefail

root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
vcrt="$root/app/Madeira/x86_64-vcruntime"
required_v14=(
  concrt140.dll
  msvcp140.dll
  msvcp140_1.dll
  msvcp140_2.dll
  msvcp140_atomic_wait.dll
  msvcp140_codecvt_ids.dll
  vcamp140.dll
  vccorlib140.dll
  vcomp140.dll
  vcruntime140.dll
  vcruntime140_1.dll
  vcruntime140_threads.dll
)
required_v90=(
  msvcp90.dll
  msvcr90.dll
)
required_vcrt=("${required_v14[@]}" "${required_v90[@]}")

missing_v14=0
for dll in "${required_v14[@]}"; do
    [[ -f "$vcrt/$dll" ]] || missing_v14=1
done

if [[ "$missing_v14" = 1 ]]; then
    temp_root="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}"
    mkdir -p "$temp_root"
    temp_root="$(cd "$temp_root" && pwd -P)"
    work="$temp_root/madeira-vcredist"
    case "$work" in
        "$temp_root"/*) ;;
        *) echo "Unsafe VC runtime work path: $work" >&2; exit 1 ;;
    esac
    rm -rf "$work"
    mkdir -p "$work/inner" "$vcrt"

    curl --fail --location --retry 3 \
      https://aka.ms/vs/17/release/vc_redist.x64.exe \
      --output "$work/vc_redist.x64.exe"
    vc_sha="${VC_REDIST_X64_SHA256:-CC0FF0EB1DC3F5188AE6300FAEF32BF5BEEBA4BDD6E8E445A9184072096B713B}"
    printf '%s  %s\n' "$vc_sha" "$work/vc_redist.x64.exe" | shasum -a 256 -c -

    # A WiX Burn redistributable contains two concatenated CABs. 7-Zip 26.01
    # opens only the first (the small UI container, whose files are u0, u1,
    # ...); the runtime packages live in the much larger attached container.
    # Locate every structurally valid CAB and carve the largest one instead of
    # relying on 7-Zip's choice or on version-specific offsets.
    python3 - "$work/vc_redist.x64.exe" "$work/attached.cab" <<'PY'
from pathlib import Path
import struct
import sys

source, output = Path(sys.argv[1]), Path(sys.argv[2])
data = source.read_bytes()
cabinets = []
offset = 0
while True:
    offset = data.find(b"MSCF", offset)
    if offset < 0:
        break
    if offset + 12 <= len(data):
        size = struct.unpack_from("<I", data, offset + 8)[0]
        if size >= 36 and offset + size <= len(data):
            cabinets.append((size, offset))
    offset += 4
if not cabinets:
    raise SystemExit("No structurally valid CAB found in VC_redist.x64.exe")
size, offset = max(cabinets)
output.write_bytes(data[offset:offset + size])
print(f"Carved attached VC container at {offset} ({size} bytes)")
PY

    mkdir -p "$work/attached"
    7zz x -y "$work/attached.cab" -o"$work/attached" >/dev/null

    # The attached container contains MSI files, Windows update CABs and the
    # actual runtime CABs under anonymous names such as a12 and a13.
    cabs=()
    while IFS= read -r -d '' payload; do
        magic="$(od -An -tx1 -N4 "$payload" 2>/dev/null | tr -d ' \n' || true)"
        if [[ "$payload" == *.cab || "$payload" == *.CAB || "$magic" == 4d534346 ]]; then
            cabs+=("$payload")
        fi
    done < <(find "$work/attached" -type f -print0)

    if (( ${#cabs[@]} == 0 )); then
        echo "No CAB payload found in VC_redist.x64.exe; extracted files:" >&2
        find "$work/attached" -maxdepth 5 -type f -print >&2
        exit 1
    fi

    echo "Found ${#cabs[@]} VC runtime CAB payload(s)"
    cab_index=0
    for cab in "${cabs[@]}"; do
        cab_out="$work/inner/$cab_index"
        mkdir -p "$cab_out"
        echo "Extracting VC payload: ${cab#$work/attached/}"
        7zz x -y "$cab" -o"$cab_out" >/dev/null
        cab_index=$((cab_index + 1))
    done

    python3 - "$work/inner" "$vcrt" "${required_v14[@]}" <<'PY'
from pathlib import Path
import shutil
import struct
import sys

src, dst = Path(sys.argv[1]), Path(sys.argv[2])
files = {}
for path in src.rglob("*"):
    if not path.is_file():
        continue
    name = path.name.lower()
    # Microsoft's CAB members are named e.g. msvcp140.dll_amd64. Accept
    # direct names too, but never select the arm64 payload from the same bundle.
    key = name[:-6] if name.endswith("_amd64") else name
    try:
        data = path.read_bytes()
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        machine = struct.unpack_from("<H", data, pe + 4)[0]
    except (OSError, struct.error):
        continue
    if machine == 0x8664:
        files[key] = path
for name in sys.argv[3:]:
    source = files.get(name.lower())
    if source is None:
        available = "\n  ".join(sorted(files))
        raise SystemExit(
            f"missing VC runtime DLL: {name}\nExtracted files:\n  {available}"
        )
    shutil.copy2(source, dst / name)
PY
fi

missing_v90=0
for dll in "${required_v90[@]}"; do
    [[ -f "$vcrt/$dll" ]] || missing_v90=1
done

if [[ "$missing_v90" = 1 ]]; then
    temp_root="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}"
    mkdir -p "$temp_root"
    temp_root="$(cd "$temp_root" && pwd -P)"
    work="$temp_root/madeira-vcredist90"
    case "$work" in
        "$temp_root"/*) ;;
        *) echo "Unsafe VC90 runtime work path: $work" >&2; exit 1 ;;
    esac
    rm -rf "$work"
    mkdir -p "$work/outer" "$work/inner" "$vcrt"

    # Microsoft Visual C++ 2008 SP1 Redistributable Package MFC Security
    # Update (x64), linked by Microsoft's current legacy-redist documentation.
    curl --fail --location --retry 3 \
      https://download.microsoft.com/download/5/D/8/5D8C65CB-C849-4025-8E95-C3966CAFD8AE/vcredist_x64.exe \
      --output "$work/vcredist_x64.exe"
    vc90_sha="${VC_REDIST_2008_X64_SHA256:-C5E273A4A16AB4D5471E91C7477719A2F45DDADB76C7F98A38FA5074A6838654}"
    printf '%s  %s\n' "$vc90_sha" "$work/vcredist_x64.exe" | shasum -a 256 -c -

    # This legacy self-extractor has one valid embedded CAB. Carve it instead
    # of executing the Windows installer, then unpack its nested vc_red.cab.
    python3 - "$work/vcredist_x64.exe" "$work/outer.cab" <<'PY'
from pathlib import Path
import struct
import sys

source, output = Path(sys.argv[1]), Path(sys.argv[2])
data = source.read_bytes()
cabinets = []
offset = 0
while True:
    offset = data.find(b"MSCF", offset)
    if offset < 0:
        break
    if offset + 12 <= len(data):
        size = struct.unpack_from("<I", data, offset + 8)[0]
        if size >= 36 and offset + size <= len(data):
            cabinets.append((size, offset))
    offset += 4
if not cabinets:
    raise SystemExit("No structurally valid CAB found in VC++ 2008 redist")
size, offset = max(cabinets)
output.write_bytes(data[offset:offset + size])
print(f"Carved VC++ 2008 container at {offset} ({size} bytes)")
PY

    7zz x -y "$work/outer.cab" -o"$work/outer" >/dev/null
    test -s "$work/outer/vc_red.cab" || {
        echo "VC++ 2008 redistributable did not contain vc_red.cab" >&2
        find "$work/outer" -maxdepth 2 -type f -print >&2
        exit 1
    }
    7zz x -y "$work/outer/vc_red.cab" -o"$work/inner" >/dev/null

    python3 - "$work/inner" "$vcrt" "${required_v90[@]}" <<'PY'
from pathlib import Path
import shutil
import struct
import sys

src, dst = Path(sys.argv[1]), Path(sys.argv[2])
for name in sys.argv[3:]:
    matches = []
    for path in src.rglob("*"):
        if not path.is_file() or not path.name.lower().startswith(name.lower() + "."):
            continue
        try:
            data = path.read_bytes()
            pe = struct.unpack_from("<I", data, 0x3C)[0]
            machine = struct.unpack_from("<H", data, pe + 4)[0]
        except (OSError, struct.error):
            continue
        if machine == 0x8664:
            matches.append(path)
    if len(matches) != 1:
        raise SystemExit(f"expected one x64 VC90 payload for {name}, found {matches}")
    shutil.copy2(matches[0], dst / name)
    print(f"Staged VC++ 2008 x64 runtime: {name}")
PY
fi

# Preserve Microsoft's Authenticode payload. A truncated or modified runtime is
# neither a reproducible input nor the unmodified redistributable Madeira needs.
python3 - "$vcrt" "${required_vcrt[@]}" <<'PY'
from pathlib import Path
import struct
import sys

root = Path(sys.argv[1])
for name in sys.argv[2:]:
    path = root / name
    data = path.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    cert_off, cert_size = struct.unpack_from("<II", data, pe + 24 + 112 + 4 * 8)
    if not cert_size or cert_off + cert_size > len(data):
        raise SystemExit(f"VC runtime is missing its Authenticode payload: {path}")
print(f"Validated {len(sys.argv) - 2} signed Microsoft runtime DLLs")
PY
