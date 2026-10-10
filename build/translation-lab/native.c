/* SPDX-License-Identifier: MIT */
#include "work.h"
#include <windows.h>

__declspec(dllexport) uint64_t madeira_lab_step(const struct translation_packet *p, uint64_t state)
{ return translation_step(p, state); }

__declspec(dllexport) uint64_t madeira_lab_batch(const struct translation_packet *p, uint32_t count,
                                               uint32_t passes, uint64_t state)
{ return translation_run(p, count, passes, state); }

__declspec(dllexport) uint32_t madeira_lab_arch(void)
{
#if defined(__arm64ec__) || defined(_M_ARM64EC)
    return 0xa641;
#elif defined(__x86_64__) || defined(_M_X64)
    return 0x8664;
#else
    return 0;
#endif
}
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{ (void)instance; (void)reason; (void)reserved; return TRUE; }
