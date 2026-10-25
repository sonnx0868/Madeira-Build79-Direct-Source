/* Scalar ARM64 accesses to a permitted Windows subpage next to a guard.
 * No exclusive/RMW instructions: an exclusive monitor cannot be recreated by
 * pretending that a store succeeded. Unknown encodings leave the fault intact.
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
};

static int ios_guard_decode( uint32_t insn, const uint64_t regs[31], uint64_t sp,
                             struct ios_guard_access *access )
{
    unsigned int size = insn >> 30, opc = (insn >> 22) & 3;
    unsigned int rn = (insn >> 5) & 31;
    uint64_t base = rn == 31 ? sp : regs[rn];
    int64_t offset = 0;

    memset( access, 0, sizeof(*access) );
    access->width = 1u << size;
    access->count = 1;
    access->rt = insn & 31;
    if ((insn & 0x3ffffc00) == 0x089ffc00) access->kind = IOS_GUARD_STORE_RELEASE; /* STLR */
    else if ((insn & 0x3ffffc00) == 0x08dffc00 ||
             (insn & 0x3ffffc00) == 0x38bfc000) access->kind = IOS_GUARD_LOAD_ACQUIRE; /* LDAR/LDAPR */
    else if ((insn & 0x3f000000) == 0x39000000) /* scalar unsigned immediate */
    {
        if ((size == 3 && opc >= 2) || (size == 2 && opc == 3)) return 0;
        offset = ((insn >> 10) & 4095) * access->width;
        access->kind = opc ? IOS_GUARD_LOAD : IOS_GUARD_STORE;
        if (opc >= 2) access->sign_bits = opc == 3 ? 32 : 64;
    }
    else if ((insn & 0x3f200c00) == 0x38000000 || /* scalar LDUR/STUR */
             (insn & 0x3f200c00) == 0x19000000)   /* LDAPUR/STLUR */
    {
        int rcpc = (insn & 0x3f200c00) == 0x19000000;
        if ((size == 3 && opc >= 2) || (size == 2 && opc == 3)) return 0;
        offset = (int64_t)((insn >> 12) & 511);
        if (offset & 256) offset -= 512;
        access->kind = opc ? (rcpc ? IOS_GUARD_LOAD_ACQUIRE : IOS_GUARD_LOAD) :
                             (rcpc ? IOS_GUARD_STORE_RELEASE : IOS_GUARD_STORE);
        if (opc >= 2) access->sign_bits = opc == 3 ? 32 : 64;
    }
    else if ((insn & 0x3f800000) == 0x29000000) /* scalar LDP/STP, no writeback */
    {
        if (size == 3 || (size == 1 && !((insn >> 22) & 1))) return 0;
        access->width = size == 2 ? 8 : 4;
        access->count = 2;
        access->rt2 = (insn >> 10) & 31;
        if (size == 1) access->sign_bits = 64; /* LDPSW */
        offset = (int64_t)((insn >> 15) & 127);
        if (offset & 64) offset -= 128;
        offset *= access->width;
        access->kind = (insn & (1u << 22)) ? IOS_GUARD_LOAD : IOS_GUARD_STORE;
        if (access->kind == IOS_GUARD_LOAD && access->rt == access->rt2) return 0;
    }
    else return 0;

    if (offset < 0 && base < (uint64_t)-offset) return 0;
    if (offset > 0 && base > UINT64_MAX - (uint64_t)offset) return 0;
    access->address = base + offset;
    if (access->address > UINT64_MAX - access->width * access->count) return 0;
    /* C atomics require alignment; leave unaligned acquire/release to the
     * existing alignment handler, rather than faulting the exception server. */
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

static void ios_guard_execute( const struct ios_guard_access *access, void *alias, uint64_t regs[31] )
{
    unsigned int i;
    for (i = 0; i < access->count; ++i)
    {
        unsigned int rt = i ? access->rt2 : access->rt;
        char *ptr = (char *)alias + i * access->width;
        uint64_t value = rt == 31 ? 0 : regs[rt];
        if (access->kind == IOS_GUARD_STORE_RELEASE) ios_guard_atomic_store( ptr, access->width, value );
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
}
#endif
