#!/usr/bin/env python3
"""Execute the patched WGI adapter and Wine's real Gamepad reading function.

Only the syscall/device backend and COM manager notifications are fixtures.
This is a host contract test, not evidence of input working in a physical game.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
wine = root / 'wine'
pin = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD:wine'], text=True).strip()
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required')


def function(text, name):
    start = text.index(name + '(')
    start = text.rfind('\n', 0, start) + 1
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        if text[end] == '{':
            depth += 1
        elif text[end] == '}':
            depth -= 1
        end += 1
    return text[start:end] + '\n'


with tempfile.TemporaryDirectory(prefix='madeira-wgi-host-') as tmp:
    folder = Path(tmp)
    base = folder / 'dlls/windows.gaming.input'
    base.mkdir(parents=True)
    for name in ('Makefile.in', 'private.h', 'provider.c', 'main.c', 'gamepad.c'):
        (base / name).write_bytes(subprocess.check_output([
            'git', '-C', str(wine), 'show', f'{pin}:dlls/windows.gaming.input/{name}']))
    patch = root / 'patches/wine-wgi-host-gamepad.patch'
    subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True)
    # A second application must be detectable, rather than silently duplicating callbacks.
    subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', '--check', str(patch)], check=True)
    provider = (base / 'provider.c').read_text(encoding='utf-8')
    gamepad = (base / 'gamepad.c').read_text(encoding='utf-8')
    main = (base / 'main.c').read_text(encoding='utf-8')
    idl = subprocess.check_output(['git', '-C', str(wine), 'show',
                                  f'{pin}:include/windows.gaming.input.idl'], text=True)
    enums = ''
    for name in ('GamepadButtons', 'GameControllerSwitchPosition'):
        body = re.search(r'enum ' + name + r'\s*\{(.*?)\};', idl, re.S).group(1)
        body = re.sub(r'\[[^\]]*\]', '', body)
        body = re.sub(r'(?m)^(\s*)(\w+)(\s*=)',
                      lambda m: m[1] + name + '_' + m[2] + m[3], body)
        enums += 'enum ' + name + ' {' + body + '};\n'
    production = '\n'.join(function(provider, name) for name in
                           ('host_gamepad_axis', 'host_gamepad_reading', 'provider_sync_host',
                            'game_provider_get_IsConnected'))
    production += function(gamepad, 'gamepad_GetCurrentReading')
    fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef int32_t HRESULT, LONG;
typedef uint32_t UINT, DWORD;
typedef uint16_t WORD;
typedef int16_t SHORT;
typedef double DOUBLE;
typedef int BOOL;
typedef unsigned char boolean, BOOLEAN, BYTE;
typedef struct {const void *lpVtbl;} IGameControllerProvider, IWineGameControllerProvider;
typedef void IGamepad;
#define WINAPI
#define TRUE 1
#define FALSE 0
#define S_OK 0
#define E_NOTIMPL ((HRESULT)0x80004001)
#define ERROR_DEVICE_NOT_CONNECTED 1167
#define HRESULT_FROM_WIN32(x) ((HRESULT)(0x80070000u | (x)))
#define FAILED(x) ((HRESULT)(x) < 0)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define XUSER_MAX_COUNT 4
#define NtUserGamepadOp_State 0
#define INVALID_HANDLE_VALUE ((void *)(intptr_t)-1)
#define ERR(...) ((void)0)
#define TRACE(...) ((void)0)
#define FIXME(...) ((void)0)
struct WineGameControllerState {
    uint64_t timestamp; DOUBLE axes[32]; BOOLEAN buttons[128];
    enum GameControllerSwitchPosition switches[4];
};
struct GamepadReading {
    uint64_t Timestamp; enum GamepadButtons Buttons;
    DOUBLE LeftTrigger, RightTrigger, LeftThumbstickX, LeftThumbstickY, RightThumbstickX, RightThumbstickY;
};
typedef struct {
    WORD wButtons; BYTE bLeftTrigger, bRightTrigger;
    SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY;
} XINPUT_GAMEPAD;
typedef struct {DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad;} XINPUT_STATE;
_Static_assert(sizeof(XINPUT_STATE) == 16, "XInput snapshot ABI");
struct provider {
    IWineGameControllerProvider IWineGameControllerProvider_iface;
    IGameControllerProvider IGameControllerProvider_iface;
    BOOL host; LONG host_connected; UINT host_index; void *device; LONG ref;
};
struct gamepad {
    struct provider *wine_provider;
    struct WineGameControllerState initial_state;
    BOOL state_changed;
};
static struct provider *impl_from_IGameControllerProvider(IGameControllerProvider *p) {
    return (void *)((char *)p - offsetof(struct provider, IGameControllerProvider_iface));
}
static struct gamepad *impl_from_IGamepad(IGamepad *p) {return (void *)p;}
static LONG InterlockedExchange(LONG *p, LONG v) {LONG old=*p; *p=v; return old;}
static LONG InterlockedCompareExchange(LONG *p, LONG v, LONG e) {LONG old=*p; if(old==e)*p=v; return old;}
static XINPUT_STATE snapshots[4];
static BOOL connected[4];
static uint64_t tick=123456;
static uint64_t GetTickCount64(void) {return tick;}
static BOOL NtUserGetGamepadState(UINT index, UINT op, void *out) {
    assert(op==0 && index<4);
    if(!connected[index]) return FALSE;
    memcpy(out, &snapshots[index], sizeof(XINPUT_STATE)); return TRUE;
}
static unsigned added, removed, released;
static struct provider *last_added, *last_removed;
static const int wine_provider_vtbl=1, game_provider_vtbl=2;
static void manager_on_provider_created(IGameControllerProvider *p) {
    ++added; last_added=impl_from_IGameControllerProvider(p);
    ++last_added->ref; /* External client retaining the old device across reconnect. */
}
static void manager_on_provider_removed(IGameControllerProvider *p) {
    ++removed; last_removed=impl_from_IGameControllerProvider(p);
}
static void IGameControllerProvider_Release(IGameControllerProvider *p) {
    struct provider *impl=impl_from_IGameControllerProvider(p);
    ++released; if(!--impl->ref) free(impl);
}
static HRESULT host_gamepad_reading(UINT index, struct WineGameControllerState *out);
static HRESULT IWineGameControllerProvider_get_State(struct provider *p, struct WineGameControllerState *out) {
    if(!p->host_connected) return HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED);
    return host_gamepad_reading(p->host_index, out);
}
'''
    fixture = '#include <stddef.h>\n' + enums + '\n' + fixture + production + r'''
static void read_pad(struct gamepad *pad, struct GamepadReading *out) {
    assert(gamepad_GetCurrentReading((IGamepad *)pad, out)==S_OK);
}
int main(void) {
    struct WineGameControllerState raw;
    struct GamepadReading out;
    struct gamepad pad={0};
    struct provider *old;
    boolean present=99;
    static const WORD masks[]={0x1000,0x2000,0x4000,0x8000,0x100,0x200,0x20,0x10,0x40,0x80};
    static const enum GamepadButtons wgi[]={GamepadButtons_A,GamepadButtons_B,GamepadButtons_X,
        GamepadButtons_Y,GamepadButtons_LeftShoulder,GamepadButtons_RightShoulder,
        GamepadButtons_View,GamepadButtons_Menu,GamepadButtons_LeftThumbstick,GamepadButtons_RightThumbstick};
    static const WORD directions[]={1,9,8,10,2,6,4,5};
    static const enum GamepadButtons dpads[]={GamepadButtons_DPadUp,
        GamepadButtons_DPadUp|GamepadButtons_DPadRight,GamepadButtons_DPadRight,
        GamepadButtons_DPadDown|GamepadButtons_DPadRight,GamepadButtons_DPadDown,
        GamepadButtons_DPadDown|GamepadButtons_DPadLeft,GamepadButtons_DPadLeft,
        GamepadButtons_DPadUp|GamepadButtons_DPadLeft};
    provider_sync_host(); assert(!added && !removed);
    connected[0]=TRUE;
    provider_sync_host(); assert(added==1 && !removed);
    old=last_added;
    assert(old->host && old->host_index==0 && old->host_connected);
    pad.wine_provider=old;
    assert(host_gamepad_reading(0,&pad.initial_state)==S_OK);
    read_pad(&pad,&out); assert(!out.Buttons && !out.LeftThumbstickX && !out.LeftThumbstickY);
    for(unsigned i=0;i<10;++i) {
        snapshots[0].Gamepad.wButtons=masks[i]; read_pad(&pad,&out); assert(out.Buttons==wgi[i]);
    }
    for(unsigned i=0;i<8;++i) {
        snapshots[0].Gamepad.wButtons=directions[i]; read_pad(&pad,&out); assert(out.Buttons==dpads[i]);
    }
    snapshots[0].Gamepad.wButtons=15; read_pad(&pad,&out); assert(!out.Buttons);
    snapshots[0].Gamepad=(XINPUT_GAMEPAD){0};
    snapshots[0].Gamepad.sThumbLX=32767; snapshots[0].Gamepad.sThumbLY=-32768;
    snapshots[0].Gamepad.sThumbRX=-32768; snapshots[0].Gamepad.sThumbRY=32767;
    snapshots[0].Gamepad.bLeftTrigger=255; snapshots[0].Gamepad.bRightTrigger=255;
    read_pad(&pad,&out);
    assert(out.LeftThumbstickX==1 && out.LeftThumbstickY==-1);
    assert(out.RightThumbstickX==-1 && out.RightThumbstickY==1);
    assert(out.LeftTrigger==1 && out.RightTrigger==1 && out.Timestamp==tick);
    for(int value=-32768;value<=32767;++value) {
        snapshots[0].Gamepad.sThumbLX=(SHORT)value;
        read_pad(&pad,&out);
        double expected=value<0 ? value/32768. : value/32767.;
        assert(fabs(out.LeftThumbstickX-expected)<1e-12);
        assert(out.LeftThumbstickY==-1); /* Horizontal movement never creates vertical input. */
    }
    snapshots[0].Gamepad=(XINPUT_GAMEPAD){0}; read_pad(&pad,&out);
    assert(!out.Buttons && !out.LeftThumbstickX && !out.LeftThumbstickY && !out.RightThumbstickX && !out.RightThumbstickY);
    assert(!out.LeftTrigger && !out.RightTrigger);
    provider_sync_host(); provider_sync_host(); assert(added==1 && !removed);
    connected[3]=TRUE; provider_sync_host(); assert(added==2 && last_added->host_index==3);
    assert(host_gamepad_reading(3,&raw)==S_OK);
    connected[1]=connected[2]=TRUE; provider_sync_host(); assert(added==4 && last_added->host_index==2);
    assert(host_gamepad_reading(1,&raw)==S_OK && host_gamepad_reading(2,&raw)==S_OK);
    connected[0]=FALSE;
    provider_sync_host(); assert(removed==1 && released==1 && last_removed==old && !old->host_connected);
    assert(FAILED(gamepad_GetCurrentReading((IGamepad *)&pad,&out)));
    assert(FAILED(host_gamepad_reading(0,&raw)));
    assert(game_provider_get_IsConnected(&old->IGameControllerProvider_iface,&present)==S_OK && !present);
    connected[0]=TRUE; provider_sync_host(); assert(added==5 && last_added!=old);
    assert(game_provider_get_IsConnected(&old->IGameControllerProvider_iface,&present)==S_OK && !present);
    assert(game_provider_get_IsConnected(&last_added->IGameControllerProvider_iface,&present)==S_OK && present);
    connected[0]=connected[1]=connected[2]=connected[3]=FALSE;
    provider_sync_host(); provider_sync_host(); assert(removed==5 && released==5);
    puts("PASS: real WGI/Gamepad adapter, every stick value, buttons/dpad/triggers, four slots and reconnect lifetime");
}
'''
    source = folder / 'check.c'
    source.write_text(fixture, encoding='utf-8')
    binary = folder / ('check.exe' if os.name == 'nt' else 'check')
    subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                    '-Wno-unused-parameter', str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    assert 'timeout = INFINITE' in main and 'timeout = 250' in main
    assert 'MADEIRA_HOST_GAMEPAD' in main and 'wait == WAIT_TIMEOUT' in main
    assert main.index('provider_sync_host();') < main.index('SetEvent( start_event );')
    assert 'setenv("MADEIRA_HOST_GAMEPAD", "1", 1);' in (root / 'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
    assert 'windows.gaming.input' in (root / 'build/wine-pe/build-controller.sh').read_text(encoding='utf-8')
    print('PASS: patch application, initial discovery before factory returns, opt-in monitor, packaged PE rebuild')
