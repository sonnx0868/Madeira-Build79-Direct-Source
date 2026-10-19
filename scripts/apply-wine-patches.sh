#!/bin/bash
# Apply Madeira-owned Wine changes without requiring push access to the Wine
# submodule's upstream repository. Safe to call repeatedly in local/CI builds.
set -euo pipefail
root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
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

patch="$root/patches/wine-timed-sleep.patch"
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
