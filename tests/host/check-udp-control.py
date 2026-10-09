#!/usr/bin/env python3
"""Compile Wine's real control converter with Darwin ancillary-message layouts.

This reproduces the one-byte type-27 cmsg seen on SteamNetworkingSockets,
including DSCP/ECN preservation, Winsock INT output and short-buffer failure.
It does not claim a device run or successful Steam matchmaking.
"""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
compiler = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc") or shutil.which("clang")
if not compiler:
    if "--require-tools" in sys.argv:
        raise SystemExit("A host C compiler is required for UDP control tests")
    print("SKIP: UDP control behavior test needs a host C compiler")
    raise SystemExit(0)

source = (root / "wine/dlls/ntdll/unix/socket.c").read_text(encoding="utf-8")
patch = root / "patches/wine-ios-udp-tos.patch"
with tempfile.TemporaryDirectory() as folder:
    work = Path(folder)
    socket = work / "dlls/ntdll/unix/socket.c"
    socket.parent.mkdir(parents=True)
    socket.write_text(source, encoding="utf-8", newline="\n")
    for item in [root / "patches/wine-socket-cmsg-rate-limit.patch", patch]:
        normalized = work / item.name
        normalized.write_text(item.read_text(encoding="utf-8"), encoding="utf-8", newline="\n")
        item = normalized
        applied = subprocess.run(["git", "apply", "--reverse", "--check", str(item)], cwd=work, capture_output=True)
        if applied.returncode:
            subprocess.run(["git", "apply", str(item)], cwd=work, check=True)
        subprocess.run(["git", "apply", "--reverse", "--check", str(item)], cwd=work, check=True)
    source = socket.read_text(encoding="utf-8")
    converter = source[source.index("static WSACMSGHDR *fill_control_message"):
                       source.index("struct cmsghdr_32")]
    harness = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define __APPLE__ 1
#define WINE_IOS 1
#define IPPROTO_IP 0
#define IPPROTO_IPV6 41
#define IP_TOS 3
#define IP_RECVTOS 27
#define WS_IPPROTO_IP 0
#define WS_IP_TOS 3
#define WSA_CMSG_ALIGN(n) (((n) + sizeof(size_t)-1) & ~(sizeof(size_t)-1))
typedef uint32_t ULONG;
typedef int32_t INT;
typedef struct { size_t cmsg_len; int cmsg_level, cmsg_type; } WSACMSGHDR;
typedef struct { ULONG len; char *buf; } WSABUF;
struct cmsghdr { uint32_t cmsg_len; int cmsg_level, cmsg_type; };
struct msghdr { void *msg_control; size_t msg_controllen; };
#define CMSG_DATA(c) ((unsigned char *)(c) + sizeof(struct cmsghdr))
#define CMSG_FIRSTHDR(h) ((h)->msg_controllen >= sizeof(struct cmsghdr) ? (struct cmsghdr *)(h)->msg_control : NULL)
#define CMSG_NXTHDR(h,c) next_cmsg((h),(c))
static struct cmsghdr *next_cmsg(struct msghdr *h, struct cmsghdr *c) {
    char *next = (char *)c + ((c->cmsg_len+3)&~3u);
    return next + sizeof(*c) <= (char *)h->msg_control+h->msg_controllen ? (struct cmsghdr *)next : NULL;
}
static int warnings, announcements;
#define FIXME(...) (++warnings)
static int dprintf(int fd, const char *fmt) { (void)fd; (void)fmt; ++announcements; return 0; }
'''
    checks = r'''
int main(void) {
    union { uint64_t align; unsigned char bytes[32]; } input;
    union { uint64_t align; unsigned char bytes[64]; } output;
    struct cmsghdr *u = (struct cmsghdr *)input.bytes;
    struct msghdr h = {input.bytes, 16};
    WSABUF b;
    for (unsigned int tos=0; tos<256; ++tos) {
        memset(&input, 0, sizeof(input)); memset(&output, 0xa5, sizeof(output));
        u->cmsg_len=13; u->cmsg_level=IPPROTO_IP; u->cmsg_type=IP_RECVTOS;
        *CMSG_DATA(u)=(unsigned char)tos;
        b.buf=(char *)output.bytes; b.len=sizeof(output.bytes);
        assert(convert_control_headers(&h,&b));
        WSACMSGHDR *w=(WSACMSGHDR *)b.buf; INT value=-1;
        memcpy(&value,output.bytes+sizeof(*w),sizeof(value));
        assert(w->cmsg_level==WS_IPPROTO_IP && w->cmsg_type==WS_IP_TOS);
        assert(w->cmsg_len==sizeof(*w)+sizeof(INT) && value==(int)tos);
        assert(b.len==sizeof(*w)+WSA_CMSG_ALIGN(sizeof(INT)));
        assert(output.bytes[b.len]==0xa5);
    }
    assert(!warnings && announcements==1);
    b.len=sizeof(WSACMSGHDR); assert(!convert_control_headers(&h,&b) && b.len==0);
    /* Existing IP_TOS path and a genuinely unknown cmsg keep their behavior. */
    u->cmsg_type=IP_TOS; b.len=sizeof(output.bytes);
    assert(convert_control_headers(&h,&b) && b.len>0);
    u->cmsg_type=99; b.len=sizeof(output.bytes);
    assert(convert_control_headers(&h,&b) && b.len==0 && warnings==1);
    h.msg_controllen=0; b.len=sizeof(output.bytes);
    assert(convert_control_headers(&h,&b) && b.len==0);
    return 0;
}
'''
    test = work / "test.c"
    exe = work / ("test.exe" if os.name == "nt" else "test")
    test.write_text(harness + converter + checks, encoding="utf-8")
    subprocess.run([compiler, "-std=c11", "-O2", str(test), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print("PASS: real Wine converter preserves all 256 TOS values, emits Winsock INT cmsgs, rejects short buffers and logs once")
