#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
out="$root/build/madsync/refresh"
mkdir -p "$out"
cp "$root/app/Madeira/libntdll_unix.a" "$out/base-ntdll.a"
cp "$root/app/Madeira/libwineserver.a" "$out/base-server.a"
bash "$root/build/ntdll-unix/build.sh" sync-only
bash "$root/build/wineserver/build.sh" sync-objects
candidate="$out/libwineserver.a"
cp "$out/base-server.a" "$candidate"
for object in mach_ios process_ios thread inproc_sync; do
  cp "$root/build/wineserver/obj/$object.o" "$out/$object.o"
  xcrun nm -g "$out/$object.o" > "$out/$object.symbols.txt"
  if grep -Eq ' _(alloc_user_handle|free_user_handle|get_virtual_screen_rect|destroy_thread_windows|get_window_thread|is_desktop_class|is_message_class|is_window_visible|mirror_region|send_notify_message|shared_session|user_shared_data)$' "$out/$object.symbols.txt"; then
    echo "Unisolated server symbols in $object.o" >&2; exit 1
  fi
  xcrun ar r "$candidate" "$out/$object.o"
done
xcrun ranlib "$candidate"
mv "$candidate" "$root/app/Madeira/libwineserver.a"
python3 "$root/tools/nutssync-bundle.py" --root "$root" --stamp \
  --base-ntdll "$out/base-ntdll.a" --base-server "$out/base-server.a"
