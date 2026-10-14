#!/bin/bash
# Fetch the identical Mesa release archive before expensive native compilation.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
source_dir="${MESA_SRC:-$root/research/mesa-25.0.7}"
[[ ! -f "$source_dir/meson.build" ]] || exit 0
archive="${TMPDIR:-/tmp}/madeira-mesa-25.0.7.tar.xz"
sha=592272df3cf01e85e7db300c449df5061092574d099da275d19e97ef0510f8a6
valid() { printf '%s  %s\n' "$sha" "$1" | shasum -a 256 -c - >/dev/null 2>&1; }
if [[ ! -f "$archive" ]] || ! valid "$archive"; then
  downloaded=false
  for url in \
    'https://archive.mesa3d.org/mesa-25.0.7.tar.xz' \
    'https://ftp.ussg.iu.edu/linux/slackware/slackware64/testing/source/graphics-updates/mesa/mesa-25.0.7.tar.xz' \
    'https://mirrors.nju.edu.cn/salix/i486/slackware-15.0/testing/source/graphics-updates/mesa/mesa-25.0.7.tar.xz'; do
    echo "Fetch pinned Mesa source: $url"
    if curl --fail --location --connect-timeout 15 --max-time 180 --retry 1 --retry-all-errors \
      "$url" -o "$archive.part" && valid "$archive.part"; then
      mv "$archive.part" "$archive"
      downloaded=true
      break
    fi
    rm -f "$archive.part"
  done
  [[ "$downloaded" = true ]] || { echo 'No Mesa source mirror returned the pinned release checksum.' >&2; exit 1; }
fi
valid "$archive"
mkdir -p "$(dirname "$source_dir")"
tar -xJf "$archive" -C "$(dirname "$source_dir")"
test -f "$source_dir/meson.build"
echo 'Verified Mesa 25.0.7 archive SHA256; source ready.'
