/* SPDX-License-Identifier: GPL-3.0-or-later
 * Madeira Converter Exception: see LICENSE-EXCEPTION.md.
 * App-private, bounded UTF-16LE command transport. No shell, no credentials. */
#ifndef MADEIRA_SESSION_PROTOCOL_H
#define MADEIRA_SESSION_PROTOCOL_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MD_REQUEST_SIZE 44u
#define MD_STATUS_SIZE 40u
#define MD_PACKET_MAX 262144u
#define MD_FIELDS 6u
enum { MD_START = 1, MD_CLOSE = 2, MD_FORCE = 3 };
enum { MD_IDLE = 0, MD_RUNNING = 2, MD_ENDED = 3, MD_ERROR = 4 };
struct md_request { uint64_t generation; uint32_t operation; const uint8_t *field[MD_FIELDS]; uint32_t size[MD_FIELDS]; };
static inline uint32_t md_u32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static inline uint64_t md_u64(const uint8_t *p) { return md_u32(p) | (uint64_t)md_u32(p + 4) << 32; }
static inline void md_put32(uint8_t *p, uint32_t v) { for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (i * 8)); }
static inline void md_put64(uint8_t *p, uint64_t v) { md_put32(p, (uint32_t)v); md_put32(p + 4, (uint32_t)(v >> 32)); }
static inline int md_request_decode(const uint8_t *data, size_t size, struct md_request *out) {
    size_t cursor = MD_REQUEST_SIZE;
    if (!data || !out || size < MD_REQUEST_SIZE || size > MD_PACKET_MAX || memcmp(data, "MDRUN001", 8)) return 0;
    memset(out, 0, sizeof(*out));
    out->generation = md_u64(data + 8); out->operation = md_u32(data + 16);
    if (!out->generation || out->operation < MD_START || out->operation > MD_FORCE) return 0;
    for (unsigned i = 0; i < MD_FIELDS; ++i) {
        uint32_t len = md_u32(data + 20 + i * 4);
        if ((len & 1) || len > 65536u || len > size - cursor) return 0;
        out->field[i] = data + cursor; out->size[i] = len; cursor += len;
        if (out->operation != MD_START) { if (len) return 0; continue; }
        if (len < 2 || out->field[i][len - 1] || out->field[i][len - 2]) return 0;
        /* exe/cmd/cwd/key are single strings; env/registry-values are MULTI_SZ. */
        if (i != 3 && i != 5) {
            for (uint32_t p = 0; p + 2 < len; p += 2)
                if (!out->field[i][p] && !out->field[i][p + 1]) return 0;
        } else if (len < 4 || out->field[i][len - 3] || out->field[i][len - 4]) return 0;
    }
    return cursor == size && (out->operation != MD_START || (out->size[0] > 2 && out->size[1] > 2));
}
#endif
