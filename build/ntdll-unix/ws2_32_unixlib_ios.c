/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Madeira's iOS Winsock Unix entry point.
 *
 * Keep app-only resolver declarations outside Wine's source include graph.
 * Wine makedep scans includes even inside #ifdef WINE_IOS while configuring
 * macOS/ARM64EC, and cannot find headers in Madeira's iOS shims directory.
 * The iOS archive build compiles this wrapper with both include directories.
 */
#include "steam_dns_ios.h"
#include "unixlib.c"
