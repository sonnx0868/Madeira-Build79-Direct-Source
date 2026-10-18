#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
out="$root/build/madsync/refresh"
mkdir -p "$out"
cp "$root/app/Madeira/libntdll_unix.a" "$out/base-ntdll.a"
cp "$root/app/Madeira/libwineserver.a" "$out/base-server.a"
bash "$root/build/ntdll-unix/build.sh" sync-only
bash "$root/build/wineserver/build.sh" sync-objects
objcopy="$(command -v llvm-objcopy || true)"
[[ -x "$objcopy" ]] || objcopy=/opt/homebrew/opt/llvm/bin/llvm-objcopy
if [[ ! -x "$objcopy" ]]; then
  # Cached app runners have Apple's compiler but not GNU-style symbol tools.
  # Only the refreshed server objects need rewriting; retain the other bytes.
  brew install llvm
  objcopy="$(brew --prefix llvm)/bin/llvm-objcopy"
fi
[[ -x "$objcopy" ]] || { echo 'llvm-objcopy is required for Wine server symbol isolation' >&2; exit 1; }
"$objcopy" --version
rename_args=()
for symbol in alloc_user_handle free_user_handle get_virtual_screen_rect destroy_thread_windows \
  get_window_thread is_desktop_class is_message_class is_window_visible mirror_region send_notify_message \
  shared_session user_shared_data; do
  rename_args+=(--redefine-sym "_${symbol}=_ws_${symbol}")
done
candidate="$out/libwineserver.a"
cp "$out/base-server.a" "$candidate"
for object in mach_ios process_ios thread inproc_sync; do
  cp "$root/build/wineserver/obj/$object.o" "$out/$object.o"
  "$objcopy" "${rename_args[@]}" "$out/$object.o"
  xcrun ar r "$candidate" "$out/$object.o"
done
xcrun ranlib "$candidate"
mv "$candidate" "$root/app/Madeira/libwineserver.a"
python3 "$root/tools/nutssync-bundle.py" --root "$root" --stamp \
  --base-ntdll "$out/base-ntdll.a" --base-server "$out/base-server.a"
