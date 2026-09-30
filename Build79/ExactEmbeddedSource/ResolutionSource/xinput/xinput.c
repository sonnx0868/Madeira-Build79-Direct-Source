#include "reader.h"

typedef void *HANDLE;
typedef uint16_t WCHAR;
typedef struct { void *ptr; } SRWLOCK;
#define IMPORT __declspec(dllimport)
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
IMPORT HANDLE CreateFileW(const WCHAR *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMPORT BOOL GetFileSizeEx(HANDLE, int64_t *);
IMPORT HANDLE CreateFileMappingW(HANDLE, void *, DWORD, DWORD, DWORD, const WCHAR *);
IMPORT void *MapViewOfFile(HANDLE, DWORD, DWORD, DWORD, size_t);
IMPORT BOOL UnmapViewOfFile(const void *);
IMPORT BOOL CloseHandle(HANDLE);
IMPORT uint64_t GetTickCount64(void);
IMPORT BOOL QueryPerformanceCounter(int64_t *);
IMPORT BOOL QueryPerformanceFrequency(int64_t *);
IMPORT void Sleep(DWORD);
IMPORT void AcquireSRWLockExclusive(SRWLOCK *);
IMPORT void ReleaseSRWLockExclusive(SRWLOCK *);

static SRWLOCK lock;
static MCReader reader;
static const MCPage *mapping;
static uint64_t next_open_ms, mapped_ms;
static DWORD mapping_confirmation_waited;
#ifndef MADEIRA_IGNORE_XINPUT_DISABLE
static DWORD enabled = 1, enable_generation;
#else
/* Keep a real initialized .data payload. Madeira's PE loader relocates data
 * sections onto 16 KiB pages; an all-BSS section is not a supported layout. */
static volatile DWORD compatibility_data_marker = 0x4d435637u; /* MCV7 */
#endif

/* Madeira's KUSER_SHARED_DATA clock is intentionally gated while its writable
 * alias is validated, so GetTickCount64 can remain zero for a whole run. QPC
 * uses the native monotonic clock and is already proven to advance on-device.
 * Keep GetTickCount64 only as a defensive fallback for ordinary Wine builds. */
static uint64_t monotonic_ms(void)
{
    static int64_t frequency;
    int64_t counter;
    if (!frequency && (!QueryPerformanceFrequency(&frequency) || frequency <= 0))
        frequency = -1;
    if (frequency > 0 && QueryPerformanceCounter(&counter) && counter >= 0)
        return (uint64_t)(counter / frequency) * 1000u +
               (uint64_t)(counter % frequency) * 1000u / (uint64_t)frequency;
    return GetTickCount64();
}

/* Compiler-generated structure copies/clears must not introduce a CRT DLL. */
void *memset(void *dst, int value, size_t count)
{
    BYTE *p = dst;
    for (size_t i = 0; i < count; ++i) p[i] = (BYTE)value;
    return dst;
}
void *memcpy(void *dst, const void *src, size_t count)
{
    BYTE *d = dst; const BYTE *s = src;
    for (size_t i = 0; i < count; ++i) d[i] = s[i];
    return dst;
}

static void try_mapping(uint64_t now)
{
    HANDLE file, section;
    int64_t length;
    if (mapping || now < next_open_ms) return;
    next_open_ms = now + 500;
    file = CreateFileW((const WCHAR *)L"C:\\Games\\MadeiraController.bin",
        0x80000000u, 7, 0, 3, 0x80, 0); /* read; share read/write/delete; existing */
    if (file == INVALID_HANDLE_VALUE) return;
    if (!GetFileSizeEx(file, &length) || length != MC_FILE_SIZE) {
        CloseHandle(file);
        return;
    }
    section = CreateFileMappingW(file, 0, 2, 0, 0, 0); /* PAGE_READONLY */
    CloseHandle(file);
    if (!section) return;
    mapping = MapViewOfFile(section, 4, 0, 0, MC_FILE_SIZE); /* FILE_MAP_READ */
    CloseHandle(section);
    mc_reader_reset(&reader);
    mapping_confirmation_waited = 0;
    mapped_ms = now;
}

/* Lock held. No guest-side writes to the shared mapping. */
static DWORD read_state(DWORD index, XINPUT_STATE *state)
{
    uint64_t now = monotonic_ms();
    try_mapping(now);
    if (!mc_reader_state(&reader, mapping, index, now, state)) {
        /* BeamNG probes XInput only once during its early device enumeration.
         * Confirm a newly mapped writer in that same call instead of requiring
         * the game to poll again. The native heartbeat normally advances at
         * 60 Hz, but its main-runloop timer can be delayed while Wine and the
         * game start. Allow one bounded 500 ms startup confirmation; a stale
         * page still cannot report a controller. */
        if (mapping && reader.observed && !reader.confirmed &&
            !mapping_confirmation_waited) {
            mapping_confirmation_waited = 1;
            for (DWORD retry = 0; retry < 500; ++retry) {
                Sleep(1);
                now = monotonic_ms();
                if (mc_reader_state(&reader, mapping, index, now, state))
                    goto connected;
                if (reader.confirmed) break;
            }
        }
        /* Recover if a crashed writer's file was replaced; an unchanged stale
         * file still needs a new heartbeat before it can report connected. */
        if (mapping && ((reader.observed && now >= reader.changed_ms &&
            now - reader.changed_ms > 1000) || (!reader.observed &&
            now >= mapped_ms && now - mapped_ms > 1000))) {
            UnmapViewOfFile(mapping);
            mapping = 0;
            mc_reader_reset(&reader);
            next_open_ms = now + 500;
        }
        return ERROR_DEVICE_NOT_CONNECTED;
    }
connected:
#ifndef MADEIRA_IGNORE_XINPUT_DISABLE
    state->dwPacketNumber += enable_generation;
    if (!enabled) state->Gamepad = (XINPUT_GAMEPAD){0};
#endif
    return ERROR_SUCCESS;
}

DWORD XInputGetState(DWORD index, XINPUT_STATE *state)
{
    DWORD result;
    if (!state) return ERROR_INVALID_PARAMETER;
    *state = (XINPUT_STATE){0};
    if (index >= MC_SLOT_COUNT) return ERROR_DEVICE_NOT_CONNECTED;
    AcquireSRWLockExclusive(&lock);
    result = read_state(index, state);
    ReleaseSRWLockExclusive(&lock);
    if (!result) state->Gamepad.wButtons &= 0xf3ffu;
    return result;
}

DWORD XInputGetStateEx(DWORD index, XINPUT_STATE *state)
{
    DWORD result;
    if (!state) return ERROR_INVALID_PARAMETER;
    *state = (XINPUT_STATE){0};
    if (index >= MC_SLOT_COUNT) return ERROR_DEVICE_NOT_CONNECTED;
    AcquireSRWLockExclusive(&lock);
    result = read_state(index, state);
    ReleaseSRWLockExclusive(&lock);
    return result; /* Ordinal 100 additionally exposes Guide (0x0400). */
}

void XInputEnable(BOOL value)
{
#ifdef MADEIRA_IGNORE_XINPUT_DISABLE
    /* BeamNG's no-CEF launch never establishes the usual UI/focus path and can
     * leave XInput disabled. Madeira already neutralizes the shared page for
     * app backgrounding and native modals, so guest-side disabling is redundant. */
    (void)value;
#else
    AcquireSRWLockExclusive(&lock);
    if (enabled != !!value) { enabled = !!value; ++enable_generation; }
    ReleaseSRWLockExclusive(&lock);
#endif
}

DWORD XInputGetCapabilities(DWORD index, DWORD flags, XINPUT_CAPABILITIES *caps)
{
    XINPUT_STATE state;
    DWORD result;
    if (!caps) return ERROR_INVALID_PARAMETER;
    *caps = (XINPUT_CAPABILITIES){0};
    if (flags & ~1u) return ERROR_INVALID_PARAMETER;
    if ((result = XInputGetState(index, &state))) return result;
    caps->Type = caps->SubType = 1; /* gamepad */
    caps->Gamepad.wButtons = 0xf3ff;
    caps->Gamepad.bLeftTrigger = caps->Gamepad.bRightTrigger = 255;
    caps->Gamepad.sThumbLX = caps->Gamepad.sThumbLY = 32767;
    caps->Gamepad.sThumbRX = caps->Gamepad.sThumbRY = 32767;
    /* Flags and vibration remain zero: no unsupported rumble/audio claims. */
    return ERROR_SUCCESS;
}

DWORD XInputSetState(DWORD index, const XINPUT_VIBRATION *vibration)
{
    XINPUT_STATE state;
    DWORD result;
    if (!vibration) return ERROR_INVALID_PARAMETER;
    if ((result = XInputGetState(index, &state))) return result;
    return ERROR_NOT_SUPPORTED;
}

DWORD XInputGetBatteryInformation(DWORD index, BYTE type,
                                  XINPUT_BATTERY_INFORMATION *battery)
{
    XINPUT_STATE state;
    DWORD result;
    if (!battery || type > 1) return ERROR_INVALID_PARAMETER;
    *battery = (XINPUT_BATTERY_INFORMATION){0};
    if ((result = XInputGetState(index, &state))) return result;
    battery->BatteryType = 255; /* unknown, not a guessed battery level */
    return ERROR_NOT_SUPPORTED;
}

DWORD XInputGetKeystroke(DWORD index, DWORD reserved, XINPUT_KEYSTROKE *key)
{
    XINPUT_STATE state;
    DWORD result;
    if (!key || reserved) return ERROR_INVALID_PARAMETER;
    *key = (XINPUT_KEYSTROKE){0};
    if (index == 255) {
        for (DWORD i = 0; i < MC_SLOT_COUNT; ++i)
            if (XInputGetState(i, &state) == ERROR_SUCCESS) return ERROR_NOT_SUPPORTED;
        return ERROR_DEVICE_NOT_CONNECTED;
    }
    if ((result = XInputGetState(index, &state))) return result;
    return ERROR_NOT_SUPPORTED;
}

DWORD XInputGetDSoundAudioDeviceGuids(DWORD index, void *render, void *capture)
{
    XINPUT_STATE state;
    DWORD result;
    if (!render || !capture) return ERROR_INVALID_PARAMETER;
    memset(render, 0, 16); memset(capture, 0, 16);
    if ((result = XInputGetState(index, &state))) return result;
    return ERROR_NOT_SUPPORTED;
}

DWORD XInputGetAudioDeviceIds(DWORD index, WCHAR *render, DWORD *render_count,
                             WCHAR *capture, DWORD *capture_count)
{
    XINPUT_STATE state;
    DWORD result;
    if (!render_count || !capture_count) return ERROR_INVALID_PARAMETER;
    if (render && *render_count) *render = 0;
    if (capture && *capture_count) *capture = 0;
    *render_count = *capture_count = 0;
    if ((result = XInputGetState(index, &state))) return result;
    return ERROR_NOT_SUPPORTED;
}

DWORD XInputGetCapabilitiesEx(DWORD reserved, DWORD index, DWORD flags, void *caps)
{
    (void)reserved; (void)index; (void)flags; (void)caps;
    return ERROR_NOT_SUPPORTED; /* No fabricated vendor/product metadata. */
}

BOOL DllMain(void *module, DWORD reason, void *reserved)
{
    (void)module;
#ifdef MADEIRA_IGNORE_XINPUT_DISABLE
    if (reason == 1 && compatibility_data_marker != 0x4d435637u) return 0;
#endif
    /* Mapping starts lazily outside the loader lock. Explicit unload closes
     * the tiny view; the OS handles process-exit teardown (reserved != NULL). */
    if (!reason && !reserved && mapping) {
        UnmapViewOfFile(mapping);
        mapping = 0;
    }
    return 1;
}
