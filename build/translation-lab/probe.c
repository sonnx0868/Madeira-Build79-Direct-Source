/* SPDX-License-Identifier: MIT */
#include "work.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef uint64_t (*step_fn)(const struct translation_packet *, uint64_t);
typedef uint64_t (*batch_fn)(const struct translation_packet *, uint32_t, uint32_t, uint64_t);
typedef uint32_t (*arch_fn)(void);
enum { count = 4096, passes = 16, repeats = 3, chunk = 64 };
static double elapsed_ms(LARGE_INTEGER before, LARGE_INTEGER after, LARGE_INTEGER frequency)
{ return (double)(after.QuadPart - before.QuadPart) * 1000.0 / frequency.QuadPart; }

int main(int argc, char **argv)
{
    const char *dll = argc > 1 ? argv[1] : "madeira-native-work.dll";
    const int smoke = argc > 2 && !strcmp(argv[2], "--smoke");
    setvbuf(stdout, NULL, _IONBF, 0);
    HMODULE module = LoadLibraryA(dll);
    if (!module) { printf("[translation-lab] helper-load-failed win32=%lu\n", GetLastError()); return 2; }
    step_fn step = (step_fn)(void *)GetProcAddress(module, "madeira_lab_step");
    batch_fn batch = (batch_fn)(void *)GetProcAddress(module, "madeira_lab_batch");
    arch_fn arch = (arch_fn)(void *)GetProcAddress(module, "madeira_lab_arch");
    if (!step || !batch || !arch) { puts("[translation-lab] helper-exports-missing"); FreeLibrary(module); return 3; }
    uint32_t machine = arch();
    printf("[translation-lab] helper=%s architecture=%04x synthetic=1\n", machine == 0xa641 ? "ARM64EC" : "x64-control", machine);
    if (!smoke && machine != 0xa641) {
        puts("[translation-lab] not-an-ARM64EC-comparison; run the EC helper on Madeira"); FreeLibrary(module); return 4;
    }
    struct translation_packet *data = malloc(count * sizeof(*data));
    if (!data) { FreeLibrary(module); return 5; }
    uint64_t init = UINT64_C(0x123456789abcdef0);
    for (uint32_t i = 0; i < count; i++) {
        data[i] = (struct translation_packet){i * UINT64_C(0x100000003), ~((uint64_t)i), i ^ init, i * 17 + 3};
    }
    // Warm both paths before timing: excludes the helper's first translation/load.
    uint64_t expected = translation_run(data, count, passes, init);
    if (batch(data, count, passes, init) != expected) { free(data); FreeLibrary(module); return 6; }
    LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency);
    double best[4] = {1e100, 1e100, 1e100, 1e100};
    for (unsigned sample = 0; sample < repeats; sample++) {
        for (unsigned mode = 0; mode < 4; mode++) {
            LARGE_INTEGER before, after;
            uint64_t result = init;
            QueryPerformanceCounter(&before);
            if (mode == 0) result = translation_run(data, count, passes, result);
            else if (mode == 1) {
                for (unsigned pass = 0; pass < passes; pass++)
                    for (unsigned i = 0; i < count; i++) result = step(data + i, result);
            } else if (mode == 2) {
                for (unsigned pass = 0; pass < passes; pass++)
                    for (unsigned i = 0; i < count; i += chunk) result = batch(data + i, chunk, 1, result);
            } else result = batch(data, count, passes, result);
            QueryPerformanceCounter(&after);
            if (result != expected) {
                printf("[translation-lab] result-mismatch mode=%u\n", mode);
                free(data); FreeLibrary(module); return 7;
            }
            double ms = elapsed_ms(before, after, frequency);
            if (ms < best[mode]) best[mode] = ms;
        }
    }
    printf("[translation-lab] work-items=%u checksums=equal local-x64-ms=%.4f native-small-ms=%.4f native-chunk64-ms=%.4f native-batch-ms=%.4f\n",
        count * passes, best[0], best[1], best[2], best[3]);
    printf("[translation-lab] boundary-calls small=%u chunk64=%u batch=1; results-do-not-predict-game-FPS\n",
        count * passes, count * passes / chunk);
    if (!smoke && machine == 0xa641) {
        FILE *report = fopen("C:\\madeira-translation-lab.txt", "w");
        if (report) {
            fprintf(report, "[translation-lab] helper=ARM64EC work-items=%u checksums=equal synthetic=1\n", count * passes);
            fprintf(report, "[translation-lab] local-x64-ms=%.4f native-small-ms=%.4f native-chunk64-ms=%.4f native-batch-ms=%.4f\n",
                best[0], best[1], best[2], best[3]);
            fprintf(report, "[translation-lab] boundary-calls small=%u chunk64=%u batch=1; results-do-not-predict-game-FPS\n",
                count * passes, count * passes / chunk);
            fclose(report);
        } else printf("[translation-lab] report-write-failed errno=%d\n", errno);
    }
    free(data); FreeLibrary(module);
    return 0;
}
