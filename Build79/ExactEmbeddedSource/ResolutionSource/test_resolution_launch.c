#include "ResolutionLaunch.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "line %d: %s\n", __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static const char launch[] =
    "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe";

static void test_names(void) {
    CHECK(madeira_resolution_launch_same("", ""));
    CHECK(madeira_resolution_launch_same("MADEIRA_ARGS", "MADEIRA_ARGS"));
    CHECK(madeira_resolution_launch_same("MADEIRA_SCREEN_W", "MADEIRA_SCREEN_W"));
    CHECK(madeira_resolution_launch_same("MADEIRA_SCREEN_H", "MADEIRA_SCREEN_H"));
    CHECK(!madeira_resolution_launch_same(NULL, NULL));
    CHECK(!madeira_resolution_launch_same(NULL, ""));
    CHECK(!madeira_resolution_launch_same("", NULL));
    CHECK(!madeira_resolution_launch_same("MADEIRA_ARGS_BAD", "MADEIRA_ARGS"));
    CHECK(!madeira_resolution_launch_same("MADEIRA_ARG", "MADEIRA_ARGS"));
    CHECK(!madeira_resolution_launch_same("madeira_args", "MADEIRA_ARGS"));
}

static void test_numbers(void) {
    const char *bad[] = {NULL, "", "-640", "+640", " 640", "640 ", "640x",
                         "640.0", "0x280", "639", "641", "4097", "0",
                         "999999999999999999999999999999999999999999"};
    size_t i;
    char text[sizeof(int) * CHAR_BIT + 3];
    CHECK(madeira_resolution_launch_parse_even("640", 640, 4096) == 640);
    CHECK(madeira_resolution_launch_parse_even("000640", 640, 4096) == 640);
    CHECK(madeira_resolution_launch_parse_even("4096", 640, 4096) == 4096);
    CHECK(madeira_resolution_launch_parse_even("2752", 640, 4096) == 2752);
    CHECK(madeira_resolution_launch_parse_even("2064", 360, 4096) == 2064);
    CHECK(madeira_resolution_launch_parse_even("2065", 360, 4096) == 0);
    for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
        CHECK(madeira_resolution_launch_parse_even(bad[i], 640, 4096) == 0);
    CHECK(!madeira_resolution_launch_parse_even("640", 4096, 640));
    CHECK(!madeira_resolution_launch_parse_even("640", -1, 4096));
    CHECK(!madeira_resolution_launch_parse_even("0", 0, 0));
    CHECK(!madeira_resolution_launch_parse_even("640", 0, -1));
    snprintf(text, sizeof(text), "%d", INT_MAX - 1);
    CHECK(madeira_resolution_launch_parse_even(text, 0, INT_MAX) == INT_MAX - 1);
    snprintf(text, sizeof(text), "%d", INT_MAX);
    CHECK(!madeira_resolution_launch_parse_even(text, 0, INT_MAX));
    /* Append a digit to a portable INT_MAX representation, forcing overflow. */
    strcat(text, "0");
    CHECK(!madeira_resolution_launch_parse_even(text, 0, INT_MAX));
    CHECK(!madeira_resolution_launch_parse_even("98", 0, 96));
    CHECK(!madeira_resolution_launch_parse_even("10", 0, 9));
}

static void test_launch(void) {
    const char *bad[] = {
        NULL, "", "/desktop=shell,1280x720", "/desktop=shell,1280x720 ",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exeBAD",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe.bak",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe/arg",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe\"",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe-arg",
        "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe;arg",
        "/desktop=shell,1280x720  C:\\windows\\system32\\services.exe",
        "/desktop=shell,1280x720 C:\\windows\\system32\\Services.exe",
        "/desktop=shell,1920x1080 C:\\windows\\system32\\services.exe",
        " /desktop=shell,1280x720 C:\\windows\\system32\\services.exe",
        "C:\\Program Files (x86)\\Steam\\steam.exe -silent",
        "steam://rungameid/123", "PATH=/usr/bin:/bin"
    };
    const char boundaries[] = " \t\n\r\f\v";
    char value[sizeof(launch) + 32];
    size_t i;
    const char *suffix = madeira_resolution_launch_suffix(launch);
    CHECK(suffix == launch + strlen("/desktop=shell,1280x720"));
    CHECK(madeira_resolution_launch_same(suffix, " C:\\windows\\system32\\services.exe"));
    for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
        CHECK(madeira_resolution_launch_suffix(bad[i]) == NULL);
    for (i = 0; i < sizeof(boundaries) - 1; ++i) {
        snprintf(value, sizeof(value), "%s%c--argument", launch, boundaries[i]);
        CHECK(madeira_resolution_launch_suffix(value) ==
              value + strlen("/desktop=shell,1280x720"));
        CHECK(strcmp(value + sizeof(launch), "--argument") == 0);
    }
    /* Every truncated prefix must be rejected, without reading past its NUL. */
    for (i = 0; i < sizeof(launch) - 1; ++i) {
        memcpy(value, launch, i);
        value[i] = '\0';
        CHECK(madeira_resolution_launch_suffix(value) == NULL);
    }
}

static void test_override_policy(void) {
    const char *names[] = {NULL, "", "PATH", "MADEIRA_ARGS_BAD",
                           "MADEIRA_SCREEN_W", "MADEIRA_SCREEN_H"};
    size_t i;
    char value[sizeof(launch)];
    memcpy(value, launch, sizeof(value));
    CHECK(madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, 1,
                                                    2752, 2064) != NULL);
    CHECK(madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, 1,
                                                    2752, 2065) != NULL);
    CHECK(madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, -1,
                                                    2752, 2064) != NULL);
    CHECK(!madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, 0,
                                                     2752, 2064));
    CHECK(!madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, 1,
                                                     1280, 720));
    CHECK(!madeira_resolution_launch_override_suffix("MADEIRA_ARGS", NULL, 1,
                                                     2752, 2064));
    CHECK(!madeira_resolution_launch_override_suffix("MADEIRA_ARGS", "steam.exe", 1,
                                                     2752, 2064));
    CHECK(!madeira_resolution_launch_override_suffix("MADEIRA_ARGS", value, 1,
                                                     0, 2064));
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        CHECK(!madeira_resolution_launch_override_suffix(names[i], value, 1,
                                                         2752, 2064));
    CHECK(memcmp(value, launch, sizeof(value)) == 0);
    CHECK(madeira_resolution_launch_native_size_valid(2752, 2064));
    CHECK(madeira_resolution_launch_native_size_valid(2752, 2065));
    CHECK(madeira_resolution_launch_native_size_valid(2753, 2065));
    CHECK(madeira_resolution_launch_custom_size_valid(2752, 2064));
    CHECK(!madeira_resolution_launch_custom_size_valid(2752, 2065));
    CHECK(!madeira_resolution_launch_custom_size_valid(2753, 2064));
    CHECK(madeira_resolution_launch_native_size_valid(640, 360));
    CHECK(madeira_resolution_launch_native_size_valid(4096, 4096));
    CHECK(!madeira_resolution_launch_native_size_valid(639, 360));
    CHECK(!madeira_resolution_launch_native_size_valid(640, 359));
    CHECK(!madeira_resolution_launch_native_size_valid(4097, 720));
    CHECK(!madeira_resolution_launch_native_size_valid(1280, 4097));
    CHECK(!madeira_resolution_launch_native_size_valid(INT_MIN, INT_MAX));
}

int main(void) {
    test_names();
    test_numbers();
    test_launch();
    test_override_policy();
    if (failures) return 1;
    puts("resolution launch tests: PASS");
    return 0;
}
