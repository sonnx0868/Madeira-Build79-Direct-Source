#!/usr/bin/env python3
"""Steam/Dock DNS compatibility is app-local, encrypted and fully wired."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
bridge = (root / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")
policy = (root / "build/ntdll-unix/steam_dns_ios.c").read_text(encoding="utf-8")
header = (root / "build/ntdll-unix/shims/steam_dns_ios.h").read_text(encoding="utf-8")
dnsapi = (root / "build/ntdll-unix/dnsapi_unixlib_ios.c").read_text(encoding="utf-8")
ws2 = (root / "wine/dlls/ws2_32/unixlib.c").read_text(encoding="utf-8")
build = (root / "build/ntdll-unix/build.sh").read_text(encoding="utf-8")
preflight = (root / "scripts/check-ios-build.sh").read_text(encoding="utf-8")
ui = (root / "app/Madeira/Onboarding.swift").read_text(encoding="utf-8")
docs = (root / "docs/MADEIRA_DOCK.md").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"steam DNS contract failed: {message}")


require("madeira_doh_query_name" in bridge and
        all(endpoint in bridge for endpoint in ("https://1.1.1.1/dns-query", "https://1.0.0.1/dns-query",
                                                "https://8.8.8.8/dns-query", "https://8.8.4.4/dns-query",
                                                "https://[2606:4700:4700::1111]/dns-query",
                                                "https://[2001:4860:4860::8888]/dns-query")),
        "Cloudflare/Google IP-literal DoH fallback is incomplete")
require(bridge.count('application/dns-message') >= 2 and 'HTTPMethod = @"POST"' in bridge,
        "the bridge is not RFC 8484 wire-format POST")
require("System Trust" in bridge and "timeoutIntervalForRequest" in bridge and "dateWithTimeIntervalSinceNow:60" in bridge and
        "cache.count >= 256" in bridge,
        "TLS trust, bounded waits or the short DNS cache is missing")
require("MADEIRA_DOCK_SESSION" in policy and "DNS_MODE_AUTO" in policy and "DNS_MODE_ALL" in policy,
        "default Dock-only policy or diagnostic all-host mode is missing")
for domain in ("steampowered.com", "steamcommunity.com", "steamcontent.com", "steamstatic.com",
               "steamserver.net", "steampipe.akamaized.net"):
    require(domain in policy, f"Steam allowlist omits {domain}")
require("dns_type, answer" in header or "dns_type" in header,
        "wire-query API is not declared")
require("madeira_steam_dns_query( dname" in dnsapi,
        "DnsQuery does not use the app-local resolver")
require("madeira_steam_dns_getaddrinfo" in ws2 and "madeira_steam_dns_freeaddrinfo" in ws2,
        "Winsock getaddrinfo does not use or correctly free DoH answers")
require("try_madeira_gethostbyname" in ws2 and ws2.count("try_madeira_gethostbyname( params, &ret )") == 2,
        "legacy gethostbyname paths can still escape to blocked ISP DNS")
require('compile_one "$BUILD_DIR/steam_dns_ios.c"' in build and '"$OBJ_DIR/steam_dns_ios.o"' in build,
        "the resolver is not in libntdll_unix.a")
require("madeira_steam_dns_getaddrinfo" in preflight,
        "native dependency preflight accepts an old resolver-free archive")
require('Picker("Madeira Dock DNS"' in ui and all(tag in ui for tag in ('.tag("auto")', '.tag("cloudflare")',
                                                                        '.tag("google")', '.tag("system")')),
        "Steam settings do not expose provider selection")
require("public IP does not change" in docs and "not a packet tunnel" in docs and "A and AAAA" in docs,
        "no-VPN/direct-traffic and dual-stack behavior are not documented")

print("Steam app-local DNS/DoH contract: ok")
