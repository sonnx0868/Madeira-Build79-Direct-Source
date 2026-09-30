#!/usr/bin/env python3
"""Inspect the generated PE bytes, not compiler claims. No third-party modules."""
from pathlib import Path
import hashlib
import json
import os
import struct

ROOT = Path(__file__).resolve().parent
BUILD = ROOT.parents[1] / "xinput"
EXPECTED = {
    'xinput1_3': {2: 'XInputGetState', 3: 'XInputSetState', 4: 'XInputGetCapabilities',
        5: 'XInputEnable', 6: 'XInputGetDSoundAudioDeviceGuids',
        7: 'XInputGetBatteryInformation', 8: 'XInputGetKeystroke', 100: None},
    'xinput1_4': {2: 'XInputGetState', 3: 'XInputSetState', 4: 'XInputGetCapabilities',
        5: 'XInputEnable', 7: 'XInputGetBatteryInformation', 8: 'XInputGetKeystroke',
        10: 'XInputGetAudioDeviceIds', 100: None, 108: None},
    'xinput9_1_0': {1: 'XInputGetCapabilities', 2: 'XInputGetDSoundAudioDeviceGuids',
        3: 'XInputGetState', 4: 'XInputSetState'},
}
IMPORTS = {line.strip() for line in (ROOT / 'kernel32.def').read_text().splitlines()[2:]}
report = {}
for variant, expected in EXPECTED.items():
    data = (BUILD / (variant + '.dll')).read_bytes()
    u16 = lambda at: struct.unpack_from('<H', data, at)[0]
    u32 = lambda at: struct.unpack_from('<I', data, at)[0]
    u64 = lambda at: struct.unpack_from('<Q', data, at)[0]
    cstr = lambda at: data[at:data.index(b'\0', at)].decode('ascii')
    assert data[:2] == b'MZ'
    pe = u32(0x3c)
    assert data[pe:pe+4] == b'PE\0\0' and u16(pe+4) == 0x8664
    assert u16(pe+22) & 0x2000, 'DLL characteristic missing'
    opt = pe + 24
    assert u16(opt) == 0x20b, 'Must be PE32+ AMD64'
    assert u16(opt+70) & 0x140 == 0x140, 'ASLR/NX characteristics missing'
    sections = []
    section_layout = {}
    for i in range(u16(pe+6)):
        at = opt + u16(pe+20) + i*40
        name = data[at:at+8].split(b'\0', 1)[0].decode('ascii')
        virtual_size, va, raw_size, raw = u32(at+8), u32(at+12), u32(at+16), u32(at+20)
        sections.append((va, raw_size, raw))
        section_layout[name] = {'virtual_size': virtual_size, 'raw_size': raw_size}
    assert section_layout.get('.data', {}).get('raw_size', 0) > 0, \
        'Madeira loader requires initialized .data bytes; all-BSS DLL is unsupported'
    def offset(rva):
        for va, raw_size, raw in sections:
            if va <= rva < va + raw_size:
                return raw + rva - va
        raise AssertionError(f'unmapped RVA {rva:x}')
    directories = opt+112
    assert u32(directories+3*8) and u32(directories+3*8+4), 'Windows unwind table missing'
    exp = offset(u32(directories))
    base, count, names, funcs, nameptrs, ordinals = struct.unpack_from('<6I', data, exp+16)
    named = {}
    for i in range(names):
        name = cstr(offset(u32(offset(nameptrs)+i*4)))
        named[base+u16(offset(ordinals)+i*2)] = name
    actual = {base+i: named.get(base+i) for i in range(count) if u32(offset(funcs)+i*4)}
    assert actual == expected, (variant, actual)
    imports = {}
    descriptor = offset(u32(directories+8))
    while u32(descriptor+12):
        dll = cstr(offset(u32(descriptor+12)))
        thunk = offset(u32(descriptor) or u32(descriptor+16))
        functions = []
        while u64(thunk):
            value = u64(thunk)
            assert not value >> 63, 'Unexpected ordinal import'
            functions.append(cstr(offset(value)+2))
            thunk += 8
        imports[dll.lower()] = functions
        descriptor += 20
    assert set(imports) == {'kernel32.dll'}, imports
    assert set(imports['kernel32.dll']) <= IMPORTS
    assert len(imports['kernel32.dll']) == len(IMPORTS), imports
    report[variant+'.dll'] = {'machine': 'AMD64', 'size': len(data),
        'sha256': hashlib.sha256(data).hexdigest(), 'exports': actual,
        'imports': imports, 'section_layout': section_layout,
        'crt_dependencies': [], 'on_device_tested': False}
report['_build_mode'] = {
    'ignore_guest_disable': os.environ.get('MADEIRA_IGNORE_XINPUT_DISABLE') == '1'
}
(BUILD / 'verification.json').write_text(json.dumps(report, indent=2)+'\n')
print(f'Three AMD64 DLLs: exact variant exports/ordinals, {len(IMPORTS)} Kernel32-only imports, ASLR/NX, no CRT dependencies verified.')
