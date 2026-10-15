#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 125hz
# Madeira Converter Exception: see LICENSE-EXCEPTION.md
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
patch="$root/patches/madeira-dock-external-launch.patch"
if git -C "$root/madeira-dock" apply --reverse --check "$patch" >/dev/null 2>&1; then
  echo 'Dock external-program adapter already applied'
elif git -C "$root/madeira-dock" apply --check "$patch"; then
  git -C "$root/madeira-dock" apply "$patch"
else
  echo 'Dock source does not match the external-program adapter patch' >&2
  exit 1
fi
