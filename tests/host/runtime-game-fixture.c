#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
static DWORD WINAPI naming_exception(void *unused) {
    (void)unused;
    ULONG_PTR info[] = {0x1000, (ULONG_PTR)"Finalizer", (ULONG_PTR)-1, 0};
    volatile ULONG_PTR keep[] = {0x123456789abcdef0ull, 0x1020304050607080ull};
    unsigned handled = 0;
    __try { RaiseException(0x406d1388, 0, 4, info); }
    __except (GetExceptionCode() == 0x406d1388 ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { handled = 1; }
    return handled && keep[0] == 0x123456789abcdef0ull && keep[1] == 0x1020304050607080ull ? 0 : 93;
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE old, PWSTR args, int show) {
    (void)instance; (void)old; (void)show;
    if (!wcscmp(args, L"delay")) { Sleep(550); return 0; }
    if (!wcscmp(args, L"seh")) {
        HANDLE thread = CreateThread(NULL, 0, naming_exception, NULL, 0, NULL);
        DWORD result = 92;
        if (!thread) return result;
        if (WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0) GetExitCodeThread(thread, &result);
        CloseHandle(thread); return (int)result;
    }
    if (!wcscmp(args, L"tree")) {
        WCHAR exe[512], command[1024]; STARTUPINFOW startup = {0}; PROCESS_INFORMATION process = {0};
        GetModuleFileNameW(NULL, exe, 512); swprintf(command, 1024, L"\"%ls\" delay", exe); startup.cb = sizeof(startup);
        if (!CreateProcessW(exe, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) return 99;
        CloseHandle(process.hThread); CloseHandle(process.hProcess); return 0;
    }
    WCHAR value[32], system[1024];
    if (!GetEnvironmentVariableW(L"SYSTEMROOT", system, 1024)) return 98;
    if (!GetEnvironmentVariableW(L"MD_TEST_VALUE", value, 32)) return 97;
    if (!wcscmp(args, L"check-a")) return wcscmp(value, L"A") ? 96 : 0;
    if (!wcscmp(args, L"check-b")) return wcscmp(value, L"B") ? 95 : 7;
    return 94;
}
