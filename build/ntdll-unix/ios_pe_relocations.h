/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static inline uint32_t ios_reloc_u32(const void *p) { uint32_t v; memcpy(&v, p, 4); return v; }
static inline uint16_t ios_reloc_u16(const void *p) { uint16_t v; memcpy(&v, p, 2); return v; }

/* The sync source may be an unpacked/replaced image. Never walk a relocation
 * directory or a block past the copied image. Validate before changing bytes;
 * do not allow fixups to rewrite the directory being walked. */
static inline int ios_reloc_sync_dir64(uint8_t *image, size_t image_size,
    uint32_t rva, uint32_t table_size, uint64_t preferred_base, intptr_t delta,
    size_t sync_offset, size_t sync_size)
{
    if (!image || rva > image_size || table_size > image_size - rva ||
        sync_offset > image_size || sync_size > image_size - sync_offset) return -1;
    size_t end = (size_t)rva + table_size;
    for (unsigned pass = 0; pass < 2; ++pass) {
        int changed = 0;
        for (size_t cursor = rva; cursor < end;) {
            if (end - cursor < 8) return -1;
            uint32_t page = ios_reloc_u32(image + cursor);
            uint32_t block_size = ios_reloc_u32(image + cursor + 4);
            if (!page && !block_size) {
                for (size_t i = cursor; i < end; ++i) if (image[i]) return -1;
                break;
            }
            if (block_size < 8 || block_size % 2 || block_size > end - cursor) return -1;
            for (size_t entry = cursor + 8; entry < cursor + block_size; entry += 2) {
                uint16_t value = ios_reloc_u16(image + entry);
                if ((value >> 12) != 10) continue;
                uint64_t offset = (uint64_t)page + (value & 0xfff);
                if (offset > image_size || image_size - offset < 8) return -1;
                if (offset < end && offset + 8 > rva) return -1;
                if (!pass || offset < sync_offset || offset + 8 > sync_offset + sync_size) continue;
                uint64_t pointer;
                memcpy(&pointer, image + offset, 8);
                if (pointer >= preferred_base && pointer - preferred_base < image_size) {
                    pointer += (uint64_t)delta;
                    memcpy(image + offset, &pointer, 8);
                    ++changed;
                }
            }
            cursor += block_size;
        }
        if (pass) return changed;
    }
    return 0;
}
