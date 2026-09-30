#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Start Wine process initialization on a background thread.
// Must be called AFTER wineserver is running.
// prefix_path: path to the Wine prefix directory
// Returns 0 on success, -1 on error.
int wine_process_start(const char *prefix_path);

// Configure the immutable launch request consumed by the next Wine thread.
// The strings are copied by the bridge, so callers may release their buffers
// immediately.  Passing NULL clears the corresponding override.  This API is
// preferred over mutating MADEIRA_* environment variables from Swift because
// a second tap cannot race the first launch's argv construction.
void wine_process_configure(const char *windows_exe,
                            const char *arguments,
                            int use_arm64ec,
                            int desktop_mode);

// Check if Wine process is running
int wine_process_is_running(void);

// Steam S0 net-test VPN gate: write C:\madeira-continue.flag into the
// prefix's drive_c so the paused winhttp-test.exe resumes to the Steam
// stage. Called by the "Continue Net Test" UI button after the user has
// detached the JIT debugger and switched VPNs. Returns 0 on success.
int madeira_write_continue_flag(void);

#ifdef __cplusplus
}
#endif
