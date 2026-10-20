#!/usr/bin/env python3
"""Run the patched XInput entry points against the production iOS snapshot.

Wine's raw HID enumerator, locking, and keystroke conversion are fixtures; the
host snapshot, win32u capability/query and XInput routing are production code.
This proves that empty host slots cannot create a raw HID worker. It does not
measure physical-game FPS, iPad power, or Bluetooth event delivery.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
wine = root / 'wine'
pin = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD:wine'], text=True).strip()
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required')


def function(text, name):
    match = re.search(r'(?m)^(?:static )?(?:void|BOOL|DWORD)\b[^\n]*\b' + name + r'\([^;]*?\)\s*\n\{', text)
    if not match:
        raise AssertionError('Function not found: ' + name)
    brace = text.index('{', match.start())
    depth, end = 1, brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end] + '\n'


with tempfile.TemporaryDirectory(prefix='madeira-xinput-authority-') as tmp:
    folder = Path(tmp)
    for name in ('include/ntuser.h', 'dlls/xinput1_3/main.c'):
        target = folder / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(subprocess.check_output(['git', '-C', str(wine), 'show', f'{pin}:{name}']))
    baseline_start = function((folder / 'dlls/xinput1_3/main.c').read_text(encoding='utf-8'), 'start_update_thread')
    patch = root / 'patches/wine-xinput-host-authority.patch'
    subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True)
    subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', '--check', str(patch)], check=True)
    source = (folder / 'dlls/xinput1_3/main.c').read_text(encoding='utf-8')
    header = (folder / 'include/ntuser.h').read_text(encoding='utf-8')
    enum_body = re.search(r'enum\s*\{([^{}]*NtUserGamepadOp_Backend[^{}]*)\};', header).group(1)
    driver = (root / 'build/win32u-unix/driver_ios.c').read_text(encoding='utf-8')
    query = driver[driver.index('/* ml1920: same-task controller snapshots'):]
    names = ('start_update_thread', 'controller_lock', 'controller_unlock',
             'host_pad_state', 'host_pad_any', 'XInputEnable', 'XInputSetState',
             'xinput_get_state', 'XInputGetState', 'XInputGetStateEx',
             'check_for_keystroke', 'XInputGetKeystroke',
             'XInputGetCapabilitiesEx', 'XInputGetCapabilities',
             'XInputGetBatteryInformation', 'XInputGetDSoundAudioDeviceGuids')
    production = '\n'.join(function(source, name) for name in names)
    fixture = r'''
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#define dprintf(fd,...) fprintf(stderr,__VA_ARGS__)
#endif
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef int16_t SHORT;
typedef uint32_t DWORD, UINT;
typedef int32_t LONG;
static LONG InterlockedExchange(LONG *p,LONG v) { return __atomic_exchange_n(p,v,__ATOMIC_SEQ_CST); }
typedef uintptr_t ULONG_PTR;
typedef int BOOL, INIT_ONCE, SRWLOCK;
typedef struct { DWORD Data1; WORD Data2, Data3; BYTE Data4[8]; } GUID;
#define C_ASSERT(x) _Static_assert(x, #x)
#define WINAPI
#define DECLSPEC_HOTPATCH
#define TRUE 1
#define FALSE 0
#define INIT_ONCE_STATIC_INIT 0
#define XUSER_MAX_COUNT 4
#define XUSER_INDEX_ANY 255
#define ERROR_SUCCESS 0
#define ERROR_BAD_ARGUMENTS 160
#define ERROR_DEVICE_NOT_CONNECTED 1167
#define ERROR_NOT_SUPPORTED 50
#define ERROR_EMPTY 4306
#define XINPUT_GAMEPAD_GUIDE 0x0400
#define XINPUT_FLAG_GAMEPAD 1
#define XINPUT_DEVSUBTYPE_GAMEPAD 1
#define BATTERY_TYPE_UNKNOWN 0xff
#define BATTERY_LEVEL_EMPTY 0
#define TRACE(...) ((void)0)
#define FIXME(...) ((void)0)
typedef struct { WORD wButtons; BYTE bLeftTrigger, bRightTrigger;
    SHORT sThumbLX,sThumbLY,sThumbRX,sThumbRY; } XINPUT_GAMEPAD;
typedef struct { DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad; } XINPUT_STATE;
typedef struct { WORD wLeftMotorSpeed,wRightMotorSpeed; } XINPUT_VIBRATION;
typedef struct { BYTE Type,SubType; WORD Flags; XINPUT_GAMEPAD Gamepad;
    XINPUT_VIBRATION Vibration; } XINPUT_CAPABILITIES;
typedef struct { XINPUT_CAPABILITIES Capabilities; WORD VendorId,ProductId,VersionNumber;
    DWORD unk; } XINPUT_CAPABILITIES_EX;
typedef struct { BYTE BatteryType,BatteryLevel; } XINPUT_BATTERY_INFORMATION;
typedef struct { WORD VirtualKey,Unicode,Flags; BYTE UserIndex,HidCode; } XINPUT_KEYSTROKE,*PXINPUT_KEYSTROKE;
typedef struct { DWORD Size; WORD VendorID,ProductID,VersionNumber; } HIDD_ATTRIBUTES;
struct xinput_controller { int crit; void *device; XINPUT_STATE state;
    XINPUT_GAMEPAD last_keystroke; XINPUT_CAPABILITIES caps; };
static struct xinput_controller controllers[4];
static XINPUT_GAMEPAD host_last_keystroke[4];
static LONG host_enabled=TRUE;
static SRWLOCK host_keystroke_lock;
static unsigned init_attempts, hid_workers, hid_enabled, hid_disabled, hid_vibration, query_calls;
static int backend_mode, state_error;
static LONG InterlockedCompareExchange(LONG *p,LONG value,LONG expected) {
    LONG old=*p; if(old==expected)*p=value; return old;
}
static void EnterCriticalSection(int *p) {(void)p;}
static void LeaveCriticalSection(int *p) {(void)p;}
static void AcquireSRWLockExclusive(SRWLOCK *p) {(void)p;}
static void ReleaseSRWLockExclusive(SRWLOCK *p) {(void)p;}
static void controller_enable(struct xinput_controller *p) {(void)p;++hid_enabled;}
static void controller_disable(struct xinput_controller *p) {(void)p;++hid_disabled;}
static DWORD HID_set_state(struct xinput_controller *p,XINPUT_VIBRATION *v) {
    (void)p;(void)v;++hid_vibration;return ERROR_SUCCESS;
}
static BOOL HidD_GetAttributes(void *p,HIDD_ATTRIBUTES *a) {
    assert(p);a->VendorID=123;a->ProductID=456;a->VersionNumber=7;return TRUE;
}
static BOOL start_update_thread_once(INIT_ONCE *once,void *param,void **context) {
    (void)once;(void)param;(void)context;++hid_workers;
    controllers[2].device=(void *)(uintptr_t)1;
    controllers[2].state.Gamepad.wButtons=0x2000;
    controllers[2].caps.SubType=XINPUT_DEVSUBTYPE_GAMEPAD;
    return TRUE;
}
static BOOL InitOnceExecuteOnce(INIT_ONCE *once,BOOL (*fn)(INIT_ONCE *,void *,void **),void *p,void **c) {
    ++init_attempts;if(!*once){*once=1;return fn(once,p,c);}return TRUE;
}
static DWORD keystroke_from_state(DWORD index,XINPUT_KEYSTROKE *out,
                                  XINPUT_GAMEPAD *now,XINPUT_GAMEPAD *last) {
    (void)last;if(!now->wButtons)return ERROR_EMPTY;
    out->UserIndex=(BYTE)index;return ERROR_SUCCESS;
}
'''
    fixture += 'enum {' + enum_body + '};\n' + query + r'''
static BOOL NtUserGetGamepadState(UINT index,UINT op,void *out) {
    ++query_calls;
    if(op==NtUserGamepadOp_Backend) {
        if(backend_mode==1)return FALSE; /* old/non-iOS backend */
        if(backend_mode==2){*(DWORD *)out=0xdeadbeef;return TRUE;}
        if(backend_mode==3){*(DWORD *)out=0x4d475031;return FALSE;}
    }
    if(state_error && op!=NtUserGamepadOp_Backend)return FALSE;
    return (BOOL)ios_gamepad_query(index,op,out);
}
'''
    fixture += production + r'''
static void disconnected(DWORD i) {
    XINPUT_STATE state;
    XINPUT_CAPABILITIES caps;
    XINPUT_CAPABILITIES_EX caps_ex;
    XINPUT_VIBRATION vibration={0};
    XINPUT_KEYSTROKE key;
    XINPUT_BATTERY_INFORMATION battery;
    GUID render,capture;
    memset(&state,0xa5,sizeof(state));
    assert(XInputGetState(i,&state)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetStateEx(i,&state)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetCapabilities(i,0,&caps)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetCapabilitiesEx(1,i,0,&caps_ex)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputSetState(i,&vibration)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetKeystroke(i,0,&key)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetBatteryInformation(i,0,&battery)==ERROR_DEVICE_NOT_CONNECTED);
    assert(XInputGetDSoundAudioDeviceGuids(i,&render,&capture)==ERROR_DEVICE_NOT_CONNECTED);
}
static void connected(DWORD i) {
    XINPUT_STATE state;
    XINPUT_CAPABILITIES caps;
    XINPUT_CAPABILITIES_EX caps_ex;
    XINPUT_VIBRATION vibration={0};
    XINPUT_KEYSTROKE key;
    XINPUT_BATTERY_INFORMATION battery;
    GUID render,capture;
    assert(XInputGetState(i,&state)==ERROR_SUCCESS);
    assert(state.Gamepad.wButtons==0x1000 && state.Gamepad.sThumbLX==-1234);
    assert(XInputGetStateEx(i,&state)==ERROR_SUCCESS && state.Gamepad.wButtons==0x1400);
    assert(XInputGetCapabilities(i,0,&caps)==ERROR_SUCCESS && caps.SubType==1);
    assert(XInputGetCapabilitiesEx(1,i,0,&caps_ex)==ERROR_SUCCESS && caps_ex.VendorId==0x045e);
    assert(XInputSetState(i,&vibration)==ERROR_SUCCESS);
    assert(XInputGetKeystroke(i,0,&key)==ERROR_SUCCESS && key.UserIndex==i);
    assert(XInputGetBatteryInformation(i,0,&battery)==ERROR_SUCCESS && battery.BatteryType==BATTERY_TYPE_UNKNOWN);
    assert(XInputGetDSoundAudioDeviceGuids(i,&render,&capture)==ERROR_NOT_SUPPORTED);
}
int main(int argc,char **argv) {
    backend_mode=argc>1?atoi(argv[1]):0;
    assert(NtUserGamepadOp_Backend==2);
    DWORD marker[3]={0xabababab,0,0xcdcdcdcd};
    assert(ios_gamepad_query(0,2,&marker[1]) && marker[1]==0x4d475031);
    assert(marker[0]==0xabababab && marker[2]==0xcdcdcdcd);
    assert(!ios_gamepad_query(1,2,&marker[1]) && !ios_gamepad_query(0,2,NULL));
    XINPUT_STATE state;
    XINPUT_CAPABILITIES_EX caps;
    XINPUT_VIBRATION vibration={0};
    unsigned before=query_calls;
    assert(XInputGetState(4,&state)==ERROR_BAD_ARGUMENTS);
    assert(XInputGetState(0,NULL)==ERROR_BAD_ARGUMENTS);
    assert(XInputSetState(4,&vibration)==ERROR_BAD_ARGUMENTS);
    assert(XInputSetState(0,NULL)==ERROR_BAD_ARGUMENTS);
    assert(XInputGetCapabilitiesEx(1,4,0,&caps)==ERROR_BAD_ARGUMENTS);
    assert(XInputGetCapabilitiesEx(1,0,0,NULL)==ERROR_BAD_ARGUMENTS);
    assert(before==query_calls && !init_attempts && !hid_workers);
    if(backend_mode==4) {
        /* The unpatched production entry point must reproduce the regression. */
        assert(XInputGetState(1,&state)==ERROR_DEVICE_NOT_CONNECTED);
        assert(init_attempts==1 && hid_workers==1);
        puts("BASELINE: disconnected iOS host slot started the raw HID worker");
        return 0;
    }
    if(backend_mode) {
        assert(XInputGetState(2,&state)==ERROR_SUCCESS && state.Gamepad.wButtons==0x2000);
        assert(init_attempts==1 && hid_workers==1);
        assert(XInputGetCapabilitiesEx(1,2,0,&caps)==ERROR_SUCCESS && caps.VendorId==123);
        assert(XInputSetState(2,&vibration)==ERROR_SUCCESS && hid_vibration==1);
        XInputEnable(FALSE); XInputEnable(TRUE);
        assert(hid_disabled==1 && hid_enabled==1 && hid_workers==1);
        puts("PASS: non-iOS/unsupported/malformed backend preserves Wine HID discovery and results");
        return 0;
    }
    for(unsigned repeat=0;repeat<7000;++repeat)
        for(DWORD i=0;i<4;++i)disconnected(i);
    XInputEnable(FALSE);XInputEnable(TRUE);
    assert(!init_attempts && !hid_workers);
    struct winios_gamepad pad={.connected=1,.buttons=0x1400,.lx=-1234};
    for(DWORD i=0;i<4;++i) {
        winios_gamepad_set_state(i,&pad);connected(i);
        for(DWORD other=i+1;other<4;++other)disconnected(other);
    }
    XInputEnable(FALSE);
    assert(XInputGetState(0,&state)==ERROR_SUCCESS && !state.Gamepad.wButtons && !state.Gamepad.sThumbLX);
    XInputEnable(TRUE);connected(0);
    state_error=1;
    for(DWORD i=0;i<4;++i)disconnected(i);
    state_error=0;
    for(DWORD i=0;i<4;++i)connected(i);
    for(DWORD i=0;i<4;++i) {
        winios_gamepad_set_state(i,NULL);disconnected(i);
        winios_gamepad_set_state(i,&pad);connected(i);
        winios_gamepad_set_state(i,NULL);
    }
    XINPUT_KEYSTROKE key;
    assert(XInputGetKeystroke(XUSER_INDEX_ANY,0,&key)==ERROR_EMPTY);
    assert(!init_attempts && !hid_workers && !hid_enabled && !hid_disabled && !hid_vibration);
    puts("PASS: production XInput+win32u snapshot; zero pads, four slots, reconnect, query errors, disabled input; no HID worker");
}
'''
    generated = folder / 'check.c'
    generated.write_text(fixture, encoding='utf-8')
    binary = folder / ('check.exe' if os.name == 'nt' else 'check')
    flags = ['-std=c11', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-pthread']
    if os.name == 'nt':
        flags += ['-static']
    if sys.platform == 'darwin':
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    subprocess.run([compiler, *flags, '-I', str(root / 'build/win32u-unix'), str(generated),
                    str(root / 'app/Madeira/Winios/WiniosGamepad.c'), '-o', str(binary)], check=True)
    env = dict(os.environ, UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    for mode in range(4):
        subprocess.run([str(binary), str(mode)], env=env, check=True)
    baseline = folder / 'baseline.c'
    baseline.write_text(fixture.replace(function(source, 'start_update_thread'), baseline_start), encoding='utf-8')
    baseline_binary = folder / ('baseline.exe' if os.name == 'nt' else 'baseline')
    subprocess.run([compiler, *flags, '-I', str(root / 'build/win32u-unix'), str(baseline),
                    str(root / 'app/Madeira/Winios/WiniosGamepad.c'), '-o', str(baseline_binary)], check=True)
    subprocess.run([str(baseline_binary), '4'], env=env, check=True)
    assert source.count('InitOnceExecuteOnce(&init_once, start_update_thread_once, NULL, NULL)') == 1
    assert 'wine-xinput-host-authority.patch' in (root / 'scripts/apply-wine-patches.sh').read_text()
    print('PASS: pinned patch application/reverse check and production entry points')
