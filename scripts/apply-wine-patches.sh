#!/bin/bash
# Apply Madeira-owned Wine changes without requiring push access to the Wine
# submodule's upstream repository. Safe to call repeatedly in local/CI builds.
set -euo pipefail
root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
# Remove exact dependent overlays before checking their older loader contexts.
# Preserve unrelated edits; any source drift still fails the later apply gate.
for name in wine-host-page-user-stack.patch wine-lean-sync-diagnostics.patch wine-x64-lock-tls-fastpath.patch wine-module-path.patch; do
    patch="$root/patches/$name"
    if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
        git -C "$root/wine" apply --reverse "$patch"
    fi
done
# Wine's dependency scanner resolves quoted includes even inside WINE_IOS
# guards while regenerating PE Makefiles. Native compilation already finds
# this header via -Ibuild/ntdll-unix/shims; stage the identical owned header
# beside sync.c as well so cached PE-only refreshes can regenerate safely.
install -m 644 "$root/build/ntdll-unix/shims/ios_tid_alert.h" \
    "$root/wine/dlls/ntdll/unix/ios_tid_alert.h"
session_priority_applied=0
if git -C "$root/wine" apply --reverse --check "$root/patches/wine-session-priority.patch" >/dev/null 2>&1; then
    session_priority_applied=1
fi
patch="$root/patches/wine-builtin-xinput.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine builtin XInput selection already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match builtin XInput selection" >&2
    exit 1
fi

host_authority_patch="$root/patches/wine-xinput-host-authority.patch"
if git -C "$root/wine" apply --reverse --check "$host_authority_patch" >/dev/null 2>&1; then
    echo "Wine XInput host authority already applied"
elif git -C "$root/wine" apply --check "$host_authority_patch"; then
    git -C "$root/wine" apply "$host_authority_patch"
else
    echo "Wine source does not match the XInput host authority adapter" >&2
    exit 1
fi

patch="$root/patches/wine-timed-sleep.patch"
host_gamepad_patch="$root/patches/wine-wgi-host-gamepad.patch"
if git -C "$root/wine" apply --reverse --check "$host_gamepad_patch" >/dev/null 2>&1; then
    echo "Wine WGI host gamepad provider already applied"
elif git -C "$root/wine" apply --check "$host_gamepad_patch"; then
    git -C "$root/wine" apply "$host_gamepad_patch"
else
    echo "Wine source does not match the WGI host gamepad provider" >&2
    exit 1
fi

if [[ "$session_priority_applied" = 1 ]] ||
   git -C "$root/wine" apply --reverse --check "$root/patches/wine-yield-budget.patch" >/dev/null 2>&1 ||
   git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine nonzero timed sleeps already avoid scheduler yield"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the timed-sleep patch" >&2
    exit 1
fi
patch="$root/patches/wine-yield-budget.patch"
if [[ "$session_priority_applied" = 1 ]] || git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine elapsed yield budget already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the elapsed yield budget patch" >&2
    exit 1
fi
patch="$root/build/wine-pe/madeira-v0.1.4.patch"

test -s "$patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine Madeira v0.1.4 patch already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
    echo "Applied Wine Madeira v0.1.4 patch"
else
    echo "Wine source does not match the pinned commit required by $patch" >&2
    exit 1
fi

# Upstream ARM64EC execute-protect notifications must leave the callback on
# the early return, including failed and cross-process requests.
patch="$root/patches/wine-exec-callback.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine execute callback exit already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the execute callback exit patch" >&2
    exit 1
fi

# Native helpers share this session's ntdll, but have no FEX CPU area.
# Their syscalls must not enter emulator callbacks that dereference that area.
patch="$root/patches/wine-native-callback-context.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine native callback context guard already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the native callback context guard" >&2
    exit 1
fi

patch="$root/patches/wine-stackwalk-jit-alias.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine ARM64EC stack-walk alias lookup already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the ARM64EC stack-walk alias patch" >&2
    exit 1
fi

# Native ARM64 children of an ARM64EC session need the inverse DLL-farm
# fallback in their own PE loader (before is_valid_binary rejects system32).
patch="$root/patches/wine-native-child-dlls.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine native child DLL fallback already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the native child DLL fallback patch" >&2
    exit 1
fi

patch="$root/patches/wine-window-proc-validation.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine window procedure validation already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the window procedure validation patch" >&2
    exit 1
fi

patch="$root/patches/wine-user-object-generation.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine client user object generation tracking already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the user object generation patch" >&2
    exit 1
fi

patch="$root/patches/wine-opengl-winios.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine winios OpenGL patch already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the pinned OpenGL patch" >&2
    exit 1
fi

patch="$root/patches/wine-native-tick-clock.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine native boot clock already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the native boot clock patch" >&2
    exit 1
fi

patch="$root/patches/wine-tid-alert-parking.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine iOS alert parking handshake already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the iOS alert parking handshake patch" >&2
    exit 1
fi

patch="$root/patches/wine-module-path.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine loaded-module sibling recovery already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the module-path recovery patch" >&2
    exit 1
fi

patch="$root/patches/wine-nutssync.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine NutsSync handoff and APC poll changes already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match NutsSync integration" >&2
    exit 1
fi

patch="$root/patches/wine-session-priority.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine cooperative session priority already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match cooperative session priority" >&2
    exit 1
fi

# WoW64's original class argument may be a MAKEINTATOM value.
patch="$root/patches/wine-wow64-create-window-atom.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine WoW64 window class atoms already preserved"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the WoW64 window class atom patch" >&2
    exit 1
fi

# Keep user-created stacks compatible with the host's protection granularity.
patch="$root/patches/wine-host-page-user-stack.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine user stacks already use the native guard page size"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match native guard-aligned user stacks" >&2
    exit 1
fi

# Reviewed upstream fast paths remain opt-in; lean diagnostics does not alter locks.
for name in wine-x64-lock-tls-fastpath.patch wine-lean-sync-diagnostics.patch; do
    patch="$root/patches/$name"
    if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
        echo "Wine upstream optimization already applied: $name"
    else
        git -C "$root/wine" apply --check "$patch"
        git -C "$root/wine" apply "$patch"
    fi
done
