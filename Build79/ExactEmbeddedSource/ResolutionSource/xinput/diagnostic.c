/* Real-device consumer only: never opens or writes the controller input page. */
#include "xinput_abi.h"
typedef void *HANDLE;
typedef uint16_t WCHAR;
#define IMPORT __declspec(dllimport)
IMPORT HANDLE CreateFileW(const WCHAR *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMPORT BOOL CloseHandle(HANDLE);
IMPORT HANDLE LoadLibraryW(const WCHAR *);
IMPORT void *GetProcAddress(HANDLE, const char *);
IMPORT DWORD GetModuleFileNameW(HANDLE, WCHAR *, DWORD);
IMPORT int WideCharToMultiByte(unsigned, DWORD, const WCHAR *, int, char *, int, const char *, BOOL *);
IMPORT DWORD GetLastError(void);
IMPORT HANDLE GetStdHandle(DWORD);
IMPORT BOOL WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMPORT void Sleep(DWORD);
IMPORT uint64_t GetTickCount64(void);
IMPORT BOOL QueryPerformanceCounter(int64_t *);
IMPORT BOOL QueryPerformanceFrequency(int64_t *);
IMPORT void ExitProcess(DWORD);

static HANDLE log_file;
static char line[512];
static unsigned line_size;
static WCHAR module_path[512];
static char utf8_path[1536];
static XINPUT_STATE previous[4];
static DWORD previous_status[4];

static uint64_t monotonic_ms(void) {
    static int64_t frequency;
    int64_t counter;
    if (!frequency && (!QueryPerformanceFrequency(&frequency) || frequency <= 0))
        frequency = -1;
    if (frequency > 0 && QueryPerformanceCounter(&counter) && counter >= 0)
        return (uint64_t)(counter / frequency) * 1000u +
               (uint64_t)(counter % frequency) * 1000u / (uint64_t)frequency;
    return GetTickCount64();
}

static void append(const char *s) {
    while (*s && line_size < sizeof line - 1) line[line_size++] = *s++;
}
static void decimal(uint64_t number) {
    char reverse[24]; unsigned count = 0;
    do { reverse[count++] = (char)('0' + number % 10); number /= 10; } while (number);
    while (count && line_size < sizeof line - 1) line[line_size++] = reverse[--count];
}
static void signed_decimal(int32_t number) {
    if (number < 0) { append("-"); decimal((uint64_t)-(int64_t)number); }
    else decimal((uint32_t)number);
}
static void hexadecimal(uint32_t value) {
    static const char digits[] = "0123456789abcdef";
    append("0x");
    for (int shift = 28; shift >= 0; shift -= 4)
        if (line_size < sizeof line - 1) line[line_size++] = digits[(value >> shift) & 15];
}
static void raw(const char *bytes, DWORD count) {
    DWORD written;
    WriteFile(log_file, bytes, count, &written, 0);
    WriteFile(GetStdHandle((DWORD)-11), bytes, count, &written, 0);
}
static void flush(void) { append("\r\n"); raw(line, line_size); line_size = 0; }
static int changed(const XINPUT_STATE *a, const XINPUT_STATE *b) {
    const BYTE *left = (const BYTE *)a, *right = (const BYTE *)b;
    for (unsigned i = 0; i < sizeof *a; ++i) if (left[i] != right[i]) return 1;
    return 0;
}
void *memcpy(void *destination, const void *source, size_t length) {
    BYTE *dst = destination; const BYTE *src = source;
    for (size_t i = 0; i < length; ++i) dst[i] = src[i];
    return destination;
}

typedef DWORD (*GetStateFn)(DWORD, XINPUT_STATE *);
void mainCRTStartup(void)
{
    log_file = CreateFileW((const WCHAR *)L"C:\\Games\\MadeiraController-test.log",
        0x40000000u, 3, 0, 2, 0x80, 0); /* Only output log: write, create/replace. */
    if (log_file == (HANDLE)(intptr_t)-1) ExitProcess(2);
    append("Madeira controller consumer v1; 60 seconds, 50ms polling; no input-page writes."); flush();
    append("status=0 connected; status=1167 disconnected. Movement changes are logged below."); flush();
    HANDLE module = LoadLibraryW((const WCHAR *)L"xinput1_4.dll");
    if (!module) {
        append("LoadLibraryW(xinput1_4.dll) failed; error="); decimal(GetLastError()); flush();
        CloseHandle(log_file); ExitProcess(3);
    }
    DWORD length = GetModuleFileNameW(module, module_path, sizeof module_path / sizeof *module_path);
    append("Loaded DLL path: "); flush();
    if (!length) { append("GetModuleFileNameW error="); decimal(GetLastError()); flush(); }
    else {
        if (length >= sizeof module_path / sizeof *module_path)
            length = sizeof module_path / sizeof *module_path - 1;
        int bytes = WideCharToMultiByte(65001, 0, module_path, (int)length,
            utf8_path, sizeof utf8_path, 0, 0);
        if (bytes > 0) raw(utf8_path, (DWORD)bytes);
        append(""); flush();
    }
    GetStateFn get_state = (GetStateFn)GetProcAddress(module, "XInputGetState");
    if (!get_state) {
        append("GetProcAddress(XInputGetState) failed; error="); decimal(GetLastError()); flush();
        CloseHandle(log_file); ExitProcess(4);
    }
    for (unsigned i = 0; i < 4; ++i) previous_status[i] = 0xffffffffu;
    uint64_t started = monotonic_ms();
    unsigned connected_seen = 0, updates = 0;
    while (monotonic_ms() - started < 60000) {
        for (DWORD i = 0; i < 4; ++i) {
            XINPUT_STATE state = {0};
            DWORD status = get_state(i, &state);
            if (!status) connected_seen |= 1u << i;
            if (status != previous_status[i] || changed(&state, &previous[i])) {
                append("t_ms="); decimal(monotonic_ms() - started);
                append(" slot="); decimal(i);
                append(" status="); decimal(status);
                append(" packet="); decimal(state.dwPacketNumber);
                append(" buttons="); hexadecimal(state.Gamepad.wButtons);
                append(" LT="); decimal(state.Gamepad.bLeftTrigger);
                append(" RT="); decimal(state.Gamepad.bRightTrigger);
                append(" LX="); signed_decimal(state.Gamepad.sThumbLX);
                append(" LY="); signed_decimal(state.Gamepad.sThumbLY);
                append(" RX="); signed_decimal(state.Gamepad.sThumbRX);
                append(" RY="); signed_decimal(state.Gamepad.sThumbRY); flush();
                previous_status[i] = status; previous[i] = state; ++updates;
            }
        }
        Sleep(50);
    }
    append("Finished; connected_slot_mask="); hexadecimal(connected_seen);
    append(" state_changes="); decimal(updates); flush();
    if (!connected_seen) {
        append("No live controller was observed. Check pairing, native writer, active app, backing path and loaded DLL."); flush();
    }
    CloseHandle(log_file);
    ExitProcess(0);
}
