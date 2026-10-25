/* Access permitted Windows subpages without consuming a neighbour's guard.
 * Exclusive/RMW operations are declined, never replaced by non-atomic success.
 * Decoder failure never mutates registers or memory.
 */
#ifndef MADEIRA_IOS_GUARD_ACCESS_H
#define MADEIRA_IOS_GUARD_ACCESS_H
#include <stdint.h>
#include <string.h>

enum ios_guard_kind { IOS_GUARD_LOAD, IOS_GUARD_STORE, IOS_GUARD_LOAD_ACQUIRE, IOS_GUARD_STORE_RELEASE };
struct ios_guard_access
{
    uint64_t address;
    unsigned int width, count, rt, rt2, sign_bits;
    enum ios_guard_kind kind;
    unsigned int rn, writeback, vector;
    uint64_t writeback_value;
};

static int ios_guard_add_offset( uint64_t base, int64_t offset, uint64_t *result )
{
    uint64_t magnitude = offset < 0 ? (uint64_t)(-(offset + 1)) + 1 : (uint64_t)offset;
    if (offset < 0 ? base < magnitude : base > UINT64_MAX - magnitude) return 0;
    *result = offset < 0 ? base - magnitude : base + magnitude;
    return 1;
}

static int ios_guard_decode( uint32_t insn, const uint64_t regs[31], uint64_t sp,
                             struct ios_guard_access *access )
{
    unsigned int size = insn >> 30, opc = (insn >> 22) & 3, mode = 0;
    unsigned int rn = (insn >> 5) & 31, vector = (insn >> 26) & 1;
    uint64_t base = rn == 31 ? sp : regs[rn], adjusted;
    int64_t offset = 0;
    int single = 0;

    memset( access, 0, sizeof(*access) );
    access->width = 1u << size;
    access->count = 1;
    access->rt = insn & 31;
    access->rn = rn;
    if ((insn & 0x3ffffc00) == 0x089ffc00) access->kind = IOS_GUARD_STORE_RELEASE;
    else if ((insn & 0x3ffffc00) == 0x08dffc00 ||
             (insn & 0x3ffffc00) == 0x38bfc000) access->kind = IOS_GUARD_LOAD_ACQUIRE;
    else if ((insn & 0x3f200c00) == 0x19000000)
    {
        if ((size == 3 && opc >= 2) || (size == 2 && opc == 3)) return 0;
        offset = (insn >> 12) & 511;
        if (offset & 256) offset -= 512;
        access->kind = opc ? IOS_GUARD_LOAD_ACQUIRE : IOS_GUARD_STORE_RELEASE;
        if (opc >= 2) access->sign_bits = opc == 3 ? 32 : 64;
    }
    else if ((insn & 0x3f000000) == (vector ? 0x3d000000u : 0x39000000u))
    {
        mode = 4; /* unsigned immediate */
        single = 1;
    }
    else if ((insn & 0x3f200c00) == (vector ? 0x3c200800u : 0x38200800u))
    {
        mode = 5; /* register offset, UXTW/LSL/SXTW/SXTX */
        single = 1;
    }
    else if ((insn & 0x3f200000) == (vector ? 0x3c000000u : 0x38000000u))
    {
        mode = (insn >> 10) & 3;
        if (mode == 2) return 0;
        offset = (insn >> 12) & 511;
        if (offset & 256) offset -= 512;
        access->writeback = mode == 1 || mode == 3;
        single = 1;
    }
    else if ((insn & 0x3e000000) == (vector ? 0x2c000000u : 0x28000000u))
    {
        unsigned int load = (insn >> 22) & 1;
        mode = (insn >> 23) & 3;
        if (size == 3 || (!vector && size == 1 && !load)) return 0;
        access->width = vector ? 4u << size : (size == 2 ? 8 : 4);
        access->count = 2;
        access->rt2 = (insn >> 10) & 31;
        access->vector = vector;
        access->writeback = mode == 1 || mode == 3;
        if (!vector && size == 1) access->sign_bits = 64;
        offset = (insn >> 15) & 127;
        if (offset & 64) offset -= 128;
        offset *= access->width;
        access->kind = load ? IOS_GUARD_LOAD : IOS_GUARD_STORE;
        if (load && access->rt == access->rt2) return 0;
        if (!vector && rn != 31 && access->writeback &&
            (rn == access->rt || rn == access->rt2)) return 0;
    }
    else return 0;

    if (single)
    {
        if (vector)
        {
            if (opc >= 2 && size != 0) return 0;
            access->vector = 1;
            access->width = (opc & 2) ? 16 : 1u << size;
            access->kind = (opc & 1) ? IOS_GUARD_LOAD : IOS_GUARD_STORE;
        }
        else
        {
            if ((size == 3 && opc >= 2) || (size == 2 && opc == 3)) return 0;
            access->kind = opc ? IOS_GUARD_LOAD : IOS_GUARD_STORE;
            if (opc >= 2) access->sign_bits = opc == 3 ? 32 : 64;
            if (rn != 31 && access->writeback && rn == access->rt) return 0;
        }
        if (mode == 4) offset = ((insn >> 10) & 4095) * access->width;
        else if (mode == 5)
        {
            unsigned int rm = (insn >> 16) & 31, option = (insn >> 13) & 7, shift = 0;
            uint64_t index = rm == 31 ? 0 : regs[rm];
            if (option != 2 && option != 3 && option != 6 && option != 7) return 0;
            if (option == 2) offset = (uint32_t)index;
            else if (option == 6) offset = (int32_t)index;
            else offset = (int64_t)index;
            if ((insn >> 12) & 1)
                while ((1u << shift) < access->width) ++shift;
            if (offset > (INT64_MAX >> shift) || offset < (INT64_MIN >> shift)) return 0;
            offset *= (int64_t)1 << shift;
        }
    }
    if (!ios_guard_add_offset( base, offset, &adjusted )) return 0;
    access->writeback_value = adjusted;
    access->address = access->writeback && mode == 1 ? base : adjusted;
    if (access->address > UINT64_MAX - access->width * access->count) return 0;
    if ((access->kind == IOS_GUARD_LOAD_ACQUIRE || access->kind == IOS_GUARD_STORE_RELEASE) &&
        (access->address & (access->width - 1))) return 0;
    return 1;
}

static uint64_t ios_guard_atomic_load( const void *address, unsigned int width )
{
    switch (width)
    {
    case 1: return __atomic_load_n( (const uint8_t *)address, __ATOMIC_ACQUIRE );
    case 2: return __atomic_load_n( (const uint16_t *)address, __ATOMIC_ACQUIRE );
    case 4: return __atomic_load_n( (const uint32_t *)address, __ATOMIC_ACQUIRE );
    default: return __atomic_load_n( (const uint64_t *)address, __ATOMIC_ACQUIRE );
    }
}

static void ios_guard_atomic_store( void *address, unsigned int width, uint64_t value )
{
    switch (width)
    {
    case 1: __atomic_store_n( (uint8_t *)address, value, __ATOMIC_RELEASE ); break;
    case 2: __atomic_store_n( (uint16_t *)address, value, __ATOMIC_RELEASE ); break;
    case 4: __atomic_store_n( (uint32_t *)address, value, __ATOMIC_RELEASE ); break;
    default: __atomic_store_n( (uint64_t *)address, value, __ATOMIC_RELEASE ); break;
    }
}

static int ios_guard_execute( const struct ios_guard_access *access, void *alias, uint64_t regs[31],
                              uint64_t *sp, unsigned char vectors[32][16] )
{
    unsigned int i;
    if ((access->vector && !vectors) || (access->writeback && access->rn == 31 && !sp)) return 0;
    for (i = 0; i < access->count; ++i)
    {
        unsigned int rt = i ? access->rt2 : access->rt;
        char *ptr = (char *)alias + i * access->width;
        uint64_t value = rt == 31 ? 0 : regs[rt];
        if (access->vector)
        {
            if (access->kind == IOS_GUARD_LOAD)
            {
                memset( vectors[rt], 0, 16 );
                memcpy( vectors[rt], ptr, access->width );
            }
            else memcpy( ptr, vectors[rt], access->width );
        }
        else if (access->kind == IOS_GUARD_STORE_RELEASE) ios_guard_atomic_store( ptr, access->width, value );
        else if (access->kind == IOS_GUARD_STORE) memcpy( ptr, &value, access->width );
        else
        {
            value = 0;
            if (access->kind == IOS_GUARD_LOAD_ACQUIRE) value = ios_guard_atomic_load( ptr, access->width );
            else memcpy( &value, ptr, access->width );
            if (access->sign_bits)
            {
                unsigned int shift = 64 - 8 * access->width;
                value = (uint64_t)((int64_t)(value << shift) >> shift);
                if (access->sign_bits == 32) value = (uint32_t)value;
            }
            if (rt != 31) regs[rt] = value;
        }
    }
    if (access->writeback)
    {
        if (access->rn == 31) *sp = access->writeback_value;
        else regs[access->rn] = access->writeback_value;
    }
    return 1;
}
#endif
