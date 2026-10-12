/* SPDX-License-Identifier: MIT */
#pragma once
/* A pure ARM64 child has no CHPE CPU area and cannot execute the session's
 * ARM64EC ntdll. The reverse direction already needed a private EC image. */
static inline int ios_child_needs_private_ntdll(unsigned machine, int session_ec) {
    return (machine == 0x8664 && !session_ec) || (machine == 0xaa64 && session_ec);
}
