#ifndef MADEIRA_OVERRIDE_MERGE_H
#define MADEIRA_OVERRIDE_MERGE_H
#include <stddef.h>
static int mc_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
static char mc_lower(char c) { return c >= 'A' && c <= 'Z' ? (char)(c + 32) : c; }
static int mc_our_module(const char *a, const char *b) {
    while (a < b && mc_space(*a)) ++a;
    while (b > a && mc_space(b[-1])) --b;
    if (a < b && *a == '*') ++a;
    if (b - a >= 4 && b[-4] == '.' && mc_lower(b[-3]) == 'd' && mc_lower(b[-2]) == 'l' && mc_lower(b[-1]) == 'l') b -= 4;
    const char *names[] = {"xinput1_3", "xinput1_4", "xinput9_1_0"};
    for (unsigned i = 0; i < 3; ++i) {
        const char *p = a, *name = names[i];
        while (p < b && *name && mc_lower(*p) == *name) { ++p; ++name; }
        if (p == b && !*name) return 1;
    }
    return 0;
}
static int mc_append(char *out, size_t capacity, size_t *used, const char *a, const char *b) {
    size_t length = (size_t)(b - a);
    if (length >= capacity || *used >= capacity - length) return 0;
    while (a < b) out[(*used)++] = *a++;
    out[*used] = 0;
    return 1;
}
static int mc_punct(char *out, size_t capacity, size_t *used, char value) {
    if (capacity < 2 || *used >= capacity - 1) return 0;
    out[(*used)++] = value; out[*used] = 0; return 1;
}
/* Remove only the three controller names from grouped rules; preserve every
 * unrelated rule/name and append native-first rules for those three modules. */
static int mc_merge_overrides(const char *input, char *out, size_t capacity) {
    if (!capacity) return 0;
    size_t used = 0; out[0] = 0;
    const char *cursor = input ? input : "";
    while (*cursor) {
        const char *end = cursor; while (*end && *end != ';') ++end;
        const char *equal = cursor; while (equal < end && *equal != '=') ++equal;
        size_t clause_start = used;
        if (used && !mc_punct(out, capacity, &used, ';')) return 0;
        unsigned kept = 0;
        if (equal == end) {
            if (!mc_append(out, capacity, &used, cursor, end)) return 0;
            kept = 1;
        } else {
            const char *name = cursor;
            while (name < equal) {
                const char *next = name; while (next < equal && *next != ',') ++next;
                const char *a = name, *b = next;
                while (a < b && mc_space(*a)) ++a;
                while (b > a && mc_space(b[-1])) --b;
                if (a != b && !mc_our_module(a, b)) {
                    if (kept && !mc_punct(out, capacity, &used, ',')) return 0;
                    if (!mc_append(out, capacity, &used, a, b)) return 0;
                    ++kept;
                }
                name = next < equal ? next + 1 : equal;
            }
            if (kept && !mc_append(out, capacity, &used, equal, end)) return 0;
        }
        if (!kept) { used = clause_start; out[used] = 0; }
        cursor = *end ? end + 1 : end;
    }
    const char *rule = "xinput1_3,xinput1_4,xinput9_1_0=n,b";
    const char *end = rule; while (*end) ++end;
    if (used && !mc_punct(out, capacity, &used, ';')) return 0;
    return mc_append(out, capacity, &used, rule, end);
}
#endif
