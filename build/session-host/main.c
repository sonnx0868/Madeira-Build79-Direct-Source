/* SPDX-License-Identifier: GPL-3.0-or-later
 * Madeira Converter Exception: see LICENSE-EXCEPTION.md.
 * Persistent Wine parent for direct executables. This is NOT Steam/Dock.
 * One job at a time, no shell parsing, no inherited handles, no forced restart. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"

static WCHAR channel[512], request_path[560], control_path[560], status_path[560], temporary_path[560];
static HANDLE job, process;
static DWORD game_pid, game_exit, game_error;
static uint64_t generation;
static uint32_t last_state = UINT32_MAX;
static DWORD close_pids[256], close_count;

static int same_key(const WCHAR *a, const WCHAR *b) {
    const WCHAR *ae = wcschr(a, L'='), *be = wcschr(b, L'=');
    return ae && be && ae != a && be != b && ae - a == be - b && !_wcsnicmp(a, b, (size_t)(ae - a));
}
static int env_sort(const void *a, const void *b) { return _wcsicmp(*(const WCHAR * const *)a, *(const WCHAR * const *)b); }
static WCHAR *child_environment(const WCHAR *overrides) {
    LPWCH original = GetEnvironmentStringsW();
    const WCHAR *entries[4096]; unsigned count = 0; size_t chars = 1;
    if (!original) return NULL;
    for (const WCHAR *p = original; *p; p += wcslen(p) + 1) {
        int replaced = 0;
        for (const WCHAR *q = overrides; *q; q += wcslen(q) + 1) if (same_key(p, q)) { replaced = 1; break; }
        if (!replaced) { if (count == 4096) goto failed; entries[count++] = p; }
    }
    for (const WCHAR *q = overrides; *q; q += wcslen(q) + 1) {
        const WCHAR *eq = wcschr(q, L'=');
        if (!eq || eq == q) goto failed;
        if (eq[1]) { if (count == 4096) goto failed; entries[count++] = q; }
    }
    qsort(entries, count, sizeof(entries[0]), env_sort);
    for (unsigned i = 0; i < count; ++i) chars += wcslen(entries[i]) + 1;
    if (chars > 131072) goto failed;
    WCHAR *result = (WCHAR *)calloc(chars + 1, sizeof(WCHAR)), *out = result;
    if (!result) goto failed;
    for (unsigned i = 0; i < count; ++i) { size_t len = wcslen(entries[i]) + 1; wmemcpy(out, entries[i], len); out += len; }
    FreeEnvironmentStringsW(original); return result;
failed:
    FreeEnvironmentStringsW(original); SetLastError(ERROR_BAD_ENVIRONMENT); return NULL;
}

static void status_write(uint32_t state, DWORD active) {
    uint8_t bytes[MD_STATUS_SIZE] = {0}; DWORD wrote = 0;
    memcpy(bytes, "MDSTAT01", 8); md_put64(bytes + 8, generation);
    md_put32(bytes + 16, state); md_put32(bytes + 20, GetCurrentProcessId());
    md_put32(bytes + 24, game_pid); md_put32(bytes + 28, game_exit);
    md_put32(bytes + 32, game_error); md_put32(bytes + 36, active);
    HANDLE file = CreateFileW(temporary_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        BOOL ok = WriteFile(file, bytes, sizeof(bytes), &wrote, NULL);
        CloseHandle(file);
        if (ok && wrote == sizeof(bytes)) MoveFileExW(temporary_path, status_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
    if (state != last_state) {
        fprintf(stderr, "[runtime-host] state=%u generation=%llu pid=%lu error=%lu active=%lu\n",
                state, (unsigned long long)generation, (unsigned long)game_pid, (unsigned long)game_error, (unsigned long)active);
        last_state = state;
    }
}

static BOOL CALLBACK close_window(HWND window, LPARAM unused) {
    DWORD pid = 0; (void)unused; GetWindowThreadProcessId(window, &pid);
    for (DWORD i = 0; i < close_count; ++i)
        if (close_pids[i] == pid) { PostMessageW(window, WM_CLOSE, 0, 0); break; }
    return TRUE;
}
static int close_job_windows(void) {
    struct { DWORD assigned, listed; ULONG_PTR pids[256]; } list = {0};
    if (!QueryInformationJobObject(job, JobObjectBasicProcessIdList, &list, sizeof(list), NULL) || list.listed > 256) return 0;
    close_count = list.listed;
    for (DWORD i = 0; i < close_count; ++i) close_pids[i] = (DWORD)list.pids[i];
    return EnumWindows(close_window, 0) != 0;
}

/* Only missing controller preferences are seeded, through the LIVE registry.
 * The app must not rewrite user.reg while the persistent wineserver owns it. */
static void registry_seed(const WCHAR *key_name, const WCHAR *values) {
    if (!*key_name || wcsncmp(key_name, L"Software\\", 9)) return;
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, key_name, 0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &key, NULL)) return;
    for (const WCHAR *p = values; *p; p += wcslen(p) + 1) {
        WCHAR name[256]; const WCHAR *eq = wcschr(p, L'='); DWORD type = 0, len = 0;
        if (!eq || eq == p || eq - p >= 256) continue;
        wmemcpy(name, p, (size_t)(eq - p)); name[eq - p] = 0;
        if (RegQueryValueExW(key, name, NULL, &type, NULL, &len) == ERROR_FILE_NOT_FOUND) {
            WCHAR *end = NULL; unsigned long value = wcstoul(eq + 1, &end, 10);
            if (end && !*end) { DWORD v = (DWORD)value; RegSetValueExW(key, name, 0, REG_DWORD, (const BYTE *)&v, sizeof(v)); }
        }
    }
    RegCloseKey(key);
}

static DWORD start_game(const struct md_request *request) {
    STARTUPINFOW startup = {0}; PROCESS_INFORMATION created = {0};
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {0};
    const WCHAR *exe = (const WCHAR *)request->field[0];
    const WCHAR *cwd = (const WCHAR *)request->field[2];
    WCHAR *command = (WCHAR *)request->field[1];
    if (job || process) return ERROR_BUSY;
#ifndef MD_RUNTIME_NATIVE_TEST
    if (wcsncmp(exe, L"C:\\", 3) || wcsncmp(cwd, L"C:\\", 3)) return ERROR_INVALID_NAME;
#endif
    job = CreateJobObjectW(NULL, NULL);
    if (!job) return GetLastError();
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))) {
        DWORD error = GetLastError(); CloseHandle(job); job = NULL; return error;
    }
    registry_seed((const WCHAR *)request->field[4], (const WCHAR *)request->field[5]);
    WCHAR *environment = child_environment((const WCHAR *)request->field[3]);
    if (!environment) { DWORD error = GetLastError(); CloseHandle(job); job = NULL; return error; }
    startup.cb = sizeof(startup);
    if (!CreateProcessW(exe, command, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
                        environment, cwd, &startup, &created)) {
        DWORD error = GetLastError(); free(environment); CloseHandle(job); job = NULL; return error;
    }
    free(environment);
    process = created.hProcess; game_pid = created.dwProcessId;
    if (!AssignProcessToJobObject(job, process) || ResumeThread(created.hThread) == (DWORD)-1) {
        DWORD error = GetLastError(); TerminateProcess(process, error);
        CloseHandle(created.hThread); CloseHandle(process); CloseHandle(job); process = job = NULL;
        return error;
    }
    CloseHandle(created.hThread);
    return 0;
}

static void read_command(const WCHAR *path) {
    HANDLE file = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD size = GetFileSize(file, NULL), read = 0;
    uint8_t *data = size <= MD_PACKET_MAX ? (uint8_t *)malloc(size ? size : 1) : NULL;
    BOOL ok = data && ReadFile(file, data, size, &read, NULL) && read == size;
    CloseHandle(file); DeleteFileW(path);
    struct md_request request;
    if (!ok || !md_request_decode(data, size, &request)) { free(data); return; }
    if (request.operation == MD_START && request.generation > generation && !job && !process) {
        generation = request.generation; game_pid = game_exit = game_error = 0;
        game_error = start_game(&request);
        status_write(game_error ? MD_ERROR : MD_RUNNING, process ? 1 : 0);
    } else if (request.generation == generation && job) {
        if (request.operation == MD_CLOSE) {
            if (!close_job_windows()) game_error = GetLastError();
        } else if (request.operation == MD_FORCE) {
            /* Windows job termination on the parent Wine thread; native
             * quiescence/JIT retirement are checked separately by the app. */
            if (!TerminateJobObject(job, 0)) game_error = GetLastError();
        }
    }
    free(data);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR args, int show) {
    (void)instance; (void)previous; (void)show;
    while (*args == L' ' || *args == L'\t') ++args;
    size_t length = wcslen(args);
    while (length && (args[length - 1] == L' ' || args[length - 1] == L'\t')) --length;
    if (length >= 2 && args[0] == L'"' && args[length - 1] == L'"') { ++args; length -= 2; }
    if (!length || length >= 500) return 2;
    wmemcpy(channel, args, length); channel[length] = 0;
    if (wcspbrk(channel, L"\"\r\n")) return 2;
#ifndef MD_RUNTIME_NATIVE_TEST
    if (wcsncmp(channel, L"C:\\madeira-runtime\\", 19)) return 2;
#endif
    swprintf(request_path, 560, L"%ls\\request.bin", channel);
    swprintf(control_path, 560, L"%ls\\control.bin", channel);
    swprintf(status_path, 560, L"%ls\\status.bin", channel);
    swprintf(temporary_path, 560, L"%ls\\status.tmp", channel);
    status_write(MD_IDLE, 0);
    for (;;) {
        read_command(request_path); /* start cannot be overwritten by an early Quit */
        read_command(control_path);
        if (job) {
            JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info = {0};
            if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &info, sizeof(info), NULL)) {
                game_error = GetLastError(); status_write(MD_ERROR, UINT32_MAX);
                /* Failed accounting is not proof of exit. Keep the job alive. */
            } else if (!info.ActiveProcesses) {
                if (process) { GetExitCodeProcess(process, &game_exit); CloseHandle(process); process = NULL; }
                CloseHandle(job); job = NULL; status_write(MD_ENDED, 0);
            }
        }
        Sleep(100);
    }
}
