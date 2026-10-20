/* Native ARM64 helpers use Wine's alias table, not FEX's x64/EC table.
 * Keep this policy allocation-free: it runs on the loader's native stack. */
#ifndef MADEIRA_IOS_FEX_ALIAS_POLICY_H
#define MADEIRA_IOS_FEX_ALIAS_POLICY_H
#include <stdint.h>
#include <stddef.h>

typedef int (*ios_alias_read_fn)(uintptr_t, void *, size_t);
typedef void (*ios_alias_push_fn)(unsigned long long, unsigned long long, unsigned long long);

/* Only exclude a positively identified native PE32+ image. In particular an
 * ARM64X image can have Machine=ARM64 and CHPE metadata, so Machine alone is
 * insufficient. Unreadable/malformed/unknown images retain the old behavior. */
static int ios_alias_native_arm64(uintptr_t base, size_t size, ios_alias_read_fn read)
{
    uint16_t mz, machine, optional_size, magic;
    uint32_t pe, signature, directories, config[2], config_size;
    uint64_t chpe;
    uintptr_t opt;
    if (!base || size < 64 || size > UINTPTR_MAX - base ||
        !read(base, &mz, 2) || mz != 0x5a4d || !read(base + 60, &pe, 4) ||
        pe < 64 || pe > size - 24 || !read(base + pe, &signature, 4) || signature != 0x4550 ||
        !read(base + pe + 4, &machine, 2) || machine != 0xaa64 ||
        !read(base + pe + 20, &optional_size, 2) || optional_size < 112 ||
        optional_size > size - pe - 24) return 0;
    opt = base + pe + 24;
    if (!read(opt, &magic, 2) || magic != 0x20b ||
        !read(opt + 108, &directories, 4)) return 0;
    if (directories <= 10) return 1;
    if (optional_size < 200 || !read(opt + 192, config, sizeof(config))) return 0;
    if (!config[0] && !config[1]) return 1;
    if (!config[0] || config[1] < 4 || config[0] > size || config[1] > size - config[0] ||
        !read(base + config[0], &config_size, 4) || config_size < 4 || config_size > config[1]) return 0;
    if (config_size <= 200) return 1; /* older load config has no CHPE field */
    if (config_size < 208 || !read(base + config[0] + 200, &chpe, 8)) return 0;
    return chpe == 0;
}

/* Remember published ranges even after Wine retires its own mapping. A native
 * image that reuses an old x64/EC VA must replace that FEX alias, otherwise FEX
 * would still translate the reused VA to a dead pool copy. This is bookkeeping
 * only; all actual translation and overlap retirement stays in FEX. */
#define IOS_ALIAS_HISTORY_CAPACITY 512
struct ios_alias_history {
    ios_alias_push_fn callback;
    unsigned count;
    int full;
    struct { uint64_t base, size; } ranges[IOS_ALIAS_HISTORY_CAPACITY];
};

static int ios_alias_overlap(uint64_t a, uint64_t as, uint64_t b, uint64_t bs)
{
    return as && bs && (a <= b ? b - a < as : a - b < bs);
}

static void ios_alias_publish(struct ios_alias_history *history, ios_alias_push_fn callback,
                              uint64_t pe, uint64_t jit, uint64_t size, int native_arm64)
{
    unsigned i, slot;
    int needed = !native_arm64;
    if (!callback || !pe || !jit || !size) return; /* no retired entries */
    if (history->callback != callback) {
        history->callback = callback;
        history->count = 0;
        history->full = 0;
    }
    /* Sub-floor records are query-only windows, never native PE aliases. */
    if (pe < UINT64_C(0x100000000)) { callback(pe, jit, size); return; }
    slot = history->count;
    for (i = 0; i < history->count; ++i) {
        if (ios_alias_overlap(pe, size, history->ranges[i].base, history->ranges[i].size)) {
            needed = 1;
            history->ranges[i].size = 0;
        }
        if (!history->ranges[i].size && slot == history->count) slot = i;
    }
    if (!needed && !history->full) return;
    if (slot < IOS_ALIAS_HISTORY_CAPACITY) {
        history->ranges[slot].base = pe;
        history->ranges[slot].size = size;
        if (slot == history->count) ++history->count;
    } else {
        /* Never omit a required replacement if the bookkeeping is exhausted. */
        history->full = 1;
    }
    callback(pe, jit, size);
}
#endif
