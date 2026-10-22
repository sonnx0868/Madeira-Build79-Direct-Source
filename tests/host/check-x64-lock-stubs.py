#!/usr/bin/env python3
"""Execute the upstream x64 stub bytes on Windows, including slow-path branches.

This checks x64 semantics, not ARM64EC execution or iPad performance.
"""
from pathlib import Path
import ctypes as C
import json
import os
import platform
import re
import struct

root = Path(__file__).resolve().parents[2]
source = (root / 'wine/dlls/ntdll/cs_x64_fastpath.c').read_text(encoding='utf-8')
if os.name != 'nt' or platform.machine().lower() not in ('amd64', 'x86_64'):
    raise SystemExit('SKIP: execute x64 Win64 stubs on the Windows development host')

def array(name):
    text = source.split('static const BYTE ' + name + '[] =', 1)[1].split('};', 1)[0]
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return bytearray(int(n, 0) for n in re.findall(r'0x[0-9a-fA-F]+|\b\d+\b', text))

k = C.WinDLL('kernel32', use_last_error=False)
k.VirtualAlloc.argtypes = [C.c_void_p, C.c_size_t, C.c_ulong, C.c_ulong]
k.VirtualAlloc.restype = C.c_void_p
k.VirtualFree.argtypes = [C.c_void_p, C.c_size_t, C.c_ulong]
k.VirtualProtect.argtypes = [C.c_void_p, C.c_size_t, C.c_ulong, C.POINTER(C.c_ulong)]
k.TlsAlloc.restype = C.c_ulong
k.TlsSetValue.argtypes = [C.c_ulong, C.c_void_p]
k.TlsFree.argtypes = [C.c_ulong]
k.GetCurrentThreadId.restype = C.c_ulong
CALL = C.WINFUNCTYPE(C.c_uint32, C.c_void_p)
TLSCALL = C.WINFUNCTYPE(C.c_void_p, C.c_uint32)
hits = []
def callback(tag):
    def run(pointer):
        hits.append(tag)
        return tag
    return CALL(run)
slow_enter, slow_leave, wake = callback(101), callback(102), callback(103)
tls_slow = TLSCALL(lambda index: 0x12340000 + index)
layout = [
    ('cs_enter_x64', [(0x3c, slow_enter)], CALL),
    ('cs_leave_x64', [(0x34, wake), (0x42, slow_leave)], CALL),
    ('cs_tryenter_x64', [], CALL),
    ('tls_get_x64', [(0x23, tls_slow)], TLSCALL),
]
page = k.VirtualAlloc(None, 4096, 0x3000, 0x04)
assert page, 'test allocation failed'
functions = []
for i, (name, slots, signature) in enumerate(layout):
    code = array(name)
    for offset, target in slots:
        code[offset:offset + 8] = struct.pack('<Q', C.cast(target, C.c_void_p).value)
    C.memmove(page + i * 256, bytes(code), len(code))
    functions.append(signature(page + i * 256))
old = C.c_ulong()
assert k.VirtualProtect(page, 4096, 0x20, C.byref(old))
enter, leave, attempt, tls = functions

class Section(C.Structure):
    _fields_ = [('debug', C.c_void_p), ('count', C.c_int32), ('recursion', C.c_int32),
                ('owner', C.c_void_p), ('semaphore', C.c_void_p), ('spin', C.c_size_t)]

def section(count=-1, recursion=0, owner=None):
    return Section(None, count, recursion, owner, None, 0)

try:
    s = section()
    assert enter(C.byref(s)) == 0
    assert (s.count, s.recursion, s.owner) == (0, 1, k.GetCurrentThreadId())
    assert enter(C.byref(s)) == 0 and (s.count, s.recursion) == (1, 2)
    assert attempt(C.byref(s)) == 1 and (s.count, s.recursion) == (2, 3)
    assert leave(C.byref(s)) == 0 and (s.count, s.recursion) == (1, 2)
    assert leave(C.byref(s)) == 0 and leave(C.byref(s)) == 0
    assert (s.count, s.recursion, s.owner) == (-1, 0, None)
    s = section(0, 1, 0x1234)
    assert attempt(C.byref(s)) == 0 and (s.count, s.recursion, s.owner) == (0, 1, 0x1234)
    assert enter(C.byref(s)) == 101 and hits[-1] == 101
    s = section(1, 1, k.GetCurrentThreadId())
    assert leave(C.byref(s)) == 103 and hits[-1] == 103
    assert (s.count, s.recursion, s.owner) == (0, 0, None)
    s = section()
    assert leave(C.byref(s)) == 102 and hits[-1] == 102
    assert attempt(C.byref(s)) == 1
    assert leave(C.byref(s)) == 0
    index = k.TlsAlloc()
    assert index != 0xffffffff
    try:
        if index < 64:
            assert k.TlsSetValue(index, 0x12345678)
            assert tls(index) == 0x12345678
        assert tls(64) == 0x12340040
        assert tls(0xffffffff) == 0x11233ffff
    finally:
        k.TlsFree(index)
    print(json.dumps({'result': 'PASS', 'scope': 'Executed production x64 bytes on Windows',
                      'cases': ['uncontended', 'recursive', 'try-success', 'try-failure',
                                'contended-fallback', 'wake-fallback', 'invalid-leave',
                                'tls-inline', 'tls-expanded', 'tls-invalid']}))
finally:
    k.VirtualFree(page, 0, 0x8000)
