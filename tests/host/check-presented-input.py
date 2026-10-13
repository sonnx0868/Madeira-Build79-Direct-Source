#!/usr/bin/env python3
"""Exercise the actual native client query and shared input geometry."""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
driver=(root/'build/win32u-unix/driver_ios.c').read_text(encoding='utf-8')
start=driver.index('void winios_drv_publish_input_rect('); end=driver.index('{',start)+1; depth=1
while depth:
    depth+=(driver[end]=='{')-(driver[end]=='}'); end+=1
query=driver[start:end]
shim=(root/'app/Madeira/IOSDisplayShim.m').read_text(encoding='utf-8')
assert 'winios_drv_publish_input_rect(hwnd);' in shim
assert 'pthread_mutex_lock(&g_input_surface_lock)' in shim
view=(root/'app/Madeira/ContentView.swift').read_text(encoding='utf-8')
assert 'client: Self.presentedClientRect()' in view
hardware=(root/'app/Madeira/HardwareInput.swift').read_text(encoding='utf-8')
assert 'MetalBackedView.presentedClientRect()' in hardware
assert '- client.minX' in hardware and '- client.minY' in hardware
assert 'madeira_input_surface_reset()' in (root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
c=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "InputSurfaceState.h"
typedef void *HWND;
typedef uintptr_t ULONG_PTR;
struct rect { int left,top,right,bottom; };
struct window_rects { struct rect window,client,visible; };
#define COORDS_SCREEN 2
#define dprintf(...) ((void)0)
static struct madeira_input_surface state;
static struct window_rects current={.client={640,360,1920,1080}};
static int calls,available=1;
static unsigned get_thread_dpi(void) { return 96; }
static int madeira_input_surface_tracks(unsigned long long hwnd) { return hwnd && state.window==hwnd; }
static int get_window_rects(HWND hwnd,int coords,struct window_rects *out,unsigned dpi) {
    assert((uintptr_t)hwnd==state.window && coords==COORDS_SCREEN && dpi==96);
    ++calls; *out=current; return available;
}
static void madeira_input_surface_update(unsigned long long hwnd,int x,int y,int w,int h) {
    madeira_input_surface_store(&state,hwnd,x,y,w,h);
}
'''+query+r'''
int main(void) {
    madeira_input_surface_select(&state,0x40026);
    winios_drv_publish_input_rect((HWND)0x30026); assert(calls==0 && !state.valid);
    winios_drv_publish_input_rect((HWND)0x40026);
    assert(calls==1 && state.valid && state.x==640 && state.y==360 && state.width==1280 && state.height==720);
    current.client=(struct rect){100,80,900,680};
    winios_drv_publish_input_rect((HWND)0x40026);
    assert(state.x==100 && state.y==80 && state.width==800 && state.height==600);
    madeira_input_surface_select(&state,0x50026); assert(!state.valid);
    winios_drv_publish_input_rect((HWND)0x40026); assert(!state.valid && calls==2);
    winios_drv_publish_input_rect((HWND)0x50026); assert(state.valid);
    available=0; winios_drv_publish_input_rect((HWND)0x50026); assert(!state.valid);
    available=1; current.client=(struct rect){100,80,100,80};
    winios_drv_publish_input_rect((HWND)0x50026); assert(!state.valid);
    madeira_input_surface_select(&state,0); assert(!state.valid);
    puts("PASS: production presented client query handles screen offsets, resize, stale windows, failed queries and empty clients");
}
'''
geometry=(root/'app/Madeira/GuestDisplay.swift').read_text(encoding='utf-8')
geometry=geometry[geometry.index('enum DisplayMode:'):geometry.index('/// The session default')]
swift='import Foundation\n'+geometry+r'''
let monitor=CGSize(width:2560,height:1440), drawable=CGSize(width:1280,height:720)
let client=CGRect(x:640,y:360,width:1280,height:720)
for mode in DisplayMode.allCases {
    for bounds in [CGRect(x:0,y:0,width:1366,height:1024),CGRect(x:17,y:23,width:1024,height:768)] {
        let displayed=GameSurfaceLayout.rect(guest:monitor,aspect:drawable,bounds:bounds,mode:mode)
        for f in [CGPoint(x:0.1,y:0.1),CGPoint(x:0.5,y:0.5),CGPoint(x:0.9,y:0.9)] {
            let finger=CGPoint(x:displayed.minX+f.x*displayed.width,y:displayed.minY+f.y*displayed.height)
            let mouse=GameSurfaceLayout.map(point:finger,guest:monitor,aspect:drawable,bounds:bounds,mode:mode,client:client)
            assert(abs(mouse.x-(client.minX+f.x*client.width))<0.001)
            assert(abs(mouse.y-(client.minY+f.y*client.height))<0.001)
            let arrow=CGPoint(x:(mouse.x-client.minX)*displayed.width/client.width,
                              y:(mouse.y-client.minY)*displayed.height/client.height)
            assert(abs(arrow.x-(finger.x-displayed.minX))<0.001 && abs(arrow.y-(finger.y-displayed.minY))<0.001)
        }
        let corner=GameSurfaceLayout.map(point:CGPoint(x:displayed.minX-100,y:displayed.minY-100),guest:monitor,
            aspect:drawable,bounds:bounds,mode:mode,client:client)
        assert(corner==client.origin)
        let unchanged=GameSurfaceLayout.map(point:CGPoint(x:displayed.midX,y:displayed.midY),guest:monitor,
            aspect:drawable,bounds:bounds,mode:mode)
        assert(unchanged==CGPoint(x:1280,y:720))
    }
}
print("PASS: production touch/mouse geometry maps offset clients through Fit/Fill/Stretch/Aspect with cursor round trips and unchanged monitor fallback")
'''
cc=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc: raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-presented-input-') as scratch:
    folder=Path(scratch); cf=folder/'test.c'; exe=folder/('test.exe' if os.name=='nt' else 'test')
    cf.write_text(c,encoding='utf-8')
    subprocess.run([cc,'-std=gnu11','-Werror','-O2','-I',str(root/'app/Madeira'),str(cf),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
    compiler=shutil.which('swiftc')
    if compiler:
        sf=folder/'main.swift'; sf.write_text(swift,encoding='utf-8')
        subprocess.run([compiler,str(sf),'-o',str(folder/'swift-test')],check=True,timeout=60)
        subprocess.run([str(folder/'swift-test')],check=True)
    elif '--require-swift' in sys.argv: raise SystemExit('Swift compiler required')
    else: print('SKIP: Swift geometry behavior runs on macOS CI')
