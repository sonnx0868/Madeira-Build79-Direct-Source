/* Bounded launch classification. A PE32 IL-only assembly is not necessarily
 * an x86 process: preserve AnyCPU, but require WoW64 for 32BITREQUIRED. */
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static uint32_t madeira_pe_u32(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int madeira_pe_read(FILE *f, uint32_t offset, void *out, size_t size)
{
    return fseek(f, (long)offset, SEEK_SET) == 0 && fread(out, 1, size, f) == size;
}

/* Unknown/unreadable images are left to the existing loader. Once an i386
 * header is established, malformed CLR metadata cannot bypass the x86 gate. */
static int madeira_pe_requires_wow64(const char *path)
{
    unsigned char dos[64], coff[24], opt[4096], section[40], clr[20];
    uint32_t base, clr_rva, clr_size, table;
    unsigned sections, opt_size;
    int required = 0;
    FILE *f = path ? fopen(path, "rb") : NULL;
    if (!f) return 0;
    if (!madeira_pe_read(f, 0, dos, sizeof(dos)) || memcmp(dos, "MZ", 2)) goto done;
    base = madeira_pe_u32(dos + 60);
    if (base < 64 || base > (16u << 20) ||
        !madeira_pe_read(f, base, coff, sizeof(coff)) || memcmp(coff, "PE\0\0", 4)) goto done;
    if (coff[4] != 0x4c || coff[5] != 1) goto done;
    required = 1;
    sections = coff[6] | (unsigned)coff[7] << 8;
    opt_size = coff[20] | (unsigned)coff[21] << 8;
    if (!sections || sections > 96 || opt_size < 216 || opt_size > sizeof(opt) ||
        !madeira_pe_read(f, base + 24, opt, opt_size) || opt[0] != 0x0b || opt[1] != 1 ||
        madeira_pe_u32(opt + 92) < 15) goto done;
    clr_rva = madeira_pe_u32(opt + 96 + 14 * 8);
    clr_size = madeira_pe_u32(opt + 100 + 14 * 8);
    if (!clr_rva || clr_size < 20) goto done;
    table = base + 24 + opt_size;
    for (unsigned i = 0; i < sections; ++i)
    {
        uint32_t rva, raw_size, raw, displacement;
        if (!madeira_pe_read(f, table + i * 40, section, sizeof(section))) goto done;
        rva = madeira_pe_u32(section + 12);
        raw_size = madeira_pe_u32(section + 16);
        raw = madeira_pe_u32(section + 20);
        if (clr_rva < rva) continue;
        displacement = clr_rva - rva;
        if (raw_size < 20 || displacement > raw_size - 20 ||
            raw > UINT32_MAX - displacement || raw + displacement > UINT32_MAX - 20) continue;
        if (!madeira_pe_read(f, raw + displacement, clr, sizeof(clr)) ||
            madeira_pe_u32(clr) < 20) goto done;
        /* ILONLY (1), without 32BITREQUIRED (2): Wine selects the native CLR.
         * 32BITPREFERRED is a preference, not a runtime requirement. */
        required = (madeira_pe_u32(clr + 16) & 3) != 1;
        break;
    }
done:
    fclose(f);
    return required;
}
