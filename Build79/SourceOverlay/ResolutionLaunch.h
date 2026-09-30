#ifndef MADEIRA_RESOLUTION_LAUNCH_H
#define MADEIRA_RESOLUTION_LAUNCH_H

#include <stddef.h>

/* Pure, locale-independent helpers; no environment access or mutable state. */
static inline int madeira_resolution_launch_same(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

/* Decimal only. Zero is the invalid sentinel, including invalid ranges. */
static inline int madeira_resolution_launch_parse_even(const char *text,
                                                       int minimum,
                                                       int maximum) {
    int value = 0;
    if (!text || !*text || minimum < 0 || maximum <= 0 || minimum > maximum)
        return 0;
    for (; *text; ++text) {
        int digit;
        if (*text < '0' || *text > '9') return 0;
        digit = *text - '0';
        /* Check the final digit too, before multiplying or adding. */
        if (value > maximum / 10 ||
            (value == maximum / 10 && digit > maximum % 10)) return 0;
        value = value * 10 + digit;
    }
    return value >= minimum && !(value & 1) ? value : 0;
}

/* Native dimensions are already oriented by the caller; odd pixels survive. */
static inline int madeira_resolution_launch_native_size_valid(int width,
                                                              int height) {
    return width >= 640 && width <= 4096 && height >= 360 && height <= 4096;
}

/* Custom settings retain the existing even-pixel restriction. */
static inline int madeira_resolution_launch_custom_size_valid(int width,
                                                              int height) {
    return madeira_resolution_launch_native_size_valid(width, height) &&
           !(width & 1) && !(height & 1);
}

/* Match the exact default desktop/services launch. Only end-of-string or
 * ASCII whitespace may follow the executable. Return a pointer into value
 * immediately after the desktop size, preserving the entire launch tail. */
static inline const char *madeira_resolution_launch_suffix(const char *value) {
    static const char desktop[] = "/desktop=shell,1280x720";
    static const char launch[] =
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe";
    const char *p = value;
    const char *expected = launch;
    if (!p) return NULL;
    while (*expected) {
        if (*p != *expected) return NULL;
        ++p;
        ++expected;
    }
    if (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' &&
        *p != '\f' && *p != '\v') return NULL;
    return value + sizeof(desktop) - 1;
}

/* NULL means pass name/value/overwrite through unchanged. Accept effective
 * native sizes here (including odd heights); validate custom input separately.
 * The caller owns setenv success handling and any active launch state. */
static inline const char *madeira_resolution_launch_override_suffix(
    const char *name, const char *value, int overwrite, int width, int height) {
    if (!overwrite || !madeira_resolution_launch_same(name, "MADEIRA_ARGS") ||
        !madeira_resolution_launch_native_size_valid(width, height) ||
        (width == 1280 && height == 720)) return NULL;
    return madeira_resolution_launch_suffix(value);
}

#endif
