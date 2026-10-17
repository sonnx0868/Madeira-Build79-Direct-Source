#!/bin/bash
# Inspect Apple's installed analysis tools without running or changing a game.
set -euo pipefail
out="${BITRISE_DEPLOY_DIR:-$PWD/build/gpu-analysis}"
mkdir -p "$out"
exec > >(tee "$out/gpu-analysis.log") 2>&1
xcodebuild -version
sw_vers
echo 'Installed Xcode applications:'
find /Applications -maxdepth 1 -iname '*xcode*' -print
echo 'GPU analysis tools:'
for tool in gpudebug metal-replay metal-capture metal-dsymutil xctrace; do
  command -v "$tool" || true
  xcrun --find "$tool" 2>/dev/null || true
done
echo 'Bundled GPU debugger/replayer executables:'
find /Applications /Library/Developer /var/run/com.apple.security.cryptexd/mnt \
  -type f \( -iname '*gpudebug*' -o -iname '*gputrace*' -o -iname '*metalreplay*' -o -iname '*mtlreplay*' -o -iname '*mtltools*' \) \
  -print 2>/dev/null | head -100 || true
echo 'Metal devices:'
system_profiler SPDisplaysDataType
echo 'System GPU analysis executables:'
find /usr/bin /usr/local/bin /opt/homebrew/bin /System/Library/PrivateFrameworks \
  -type f \( -iname '*gpudebug*' -o -iname '*metalreplay*' -o -iname '*mtlreplay*' \) \
  -print 2>/dev/null | head -60 || true
manfile="$(xcrun --sdk macosx --show-sdk-path)/usr/share/man/man1/gpudebug.1"
if [[ -f "$manfile" ]]; then cp "$manfile" "$out/gpudebug.1"; fi
python3 tools/gpu-analysis-mac.py "$out"
