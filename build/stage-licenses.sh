#!/bin/bash
# Refresh the generated licence copies the bundle carries (app/Madeira/licenses is
# a bundled folder reference). The Xcode build phase fails when these are stale.
set -eu
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cp "$R/COPYING" "$R/app/Madeira/licenses/LICENSE-MADEIRA-GPL-3.0.txt"
cp "$R/LICENSE-EXCEPTION.md" "$R/app/Madeira/licenses/LICENSE-MADEIRA-EXCEPTION.txt"
cp "$R/third_party/nutssync/COPYING.LIB" "$R/app/Madeira/licenses/NutsSync-LGPL-2.1.txt"
cp "$R/third_party/nutssync/NOTICE.txt" "$R/app/Madeira/licenses/NutsSync-NOTICE.txt"
cp "$R/third_party/playport/NOTICE.txt" "$R/app/Madeira/licenses/Playport-NOTICE.txt"
echo "licence copies refreshed in app/Madeira/licenses"
