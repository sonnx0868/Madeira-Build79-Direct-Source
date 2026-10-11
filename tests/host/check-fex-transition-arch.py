#!/usr/bin/env python3
"""Validate actual ARM64EC transition instructions, not C++ marker strings.

Build 597 passed C++ cache tests but assembled Module.S without FEX_IOS_HOST.
Its enter_jit loaded from x18 after pushing an x64 return address (SP % 16 == 8).
Wine's generic x18 patch trampoline then pushed a scratch register through SP,
causing the alignment fault in abe0dade at pool PC 0x149e1b710. The iOS assembly
already has direct TSD reads, alias translation, FFS bypass and sweep fences.
"""
from pathlib import Path
import argparse, struct

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--dll', type=Path, default=root/'app/Madeira/arm64ec-windows/xtajit64.dll')
args = parser.parse_args()

def verify(path):
    data = path.read_bytes()
    def u16(at): return struct.unpack_from('<H', data, at)[0]
    def u32(at): return struct.unpack_from('<I', data, at)[0]
    pe = u32(60)
    if data[pe:pe+4] != b'PE\0\0' or u16(pe+4) not in (0x8664, 0xa641) or u16(pe+24) != 0x20b:
        raise ValueError('not an ARM64EC PE64 DLL')
    optional = pe+24
    sections = optional+u16(pe+20)
    def offset(rva, size):
        for i in range(u16(pe+6)):
            at = sections+i*40
            start, length, raw = u32(at+12), u32(at+16), u32(at+20)
            if start <= rva and rva+size <= start+length and raw+rva-start+size <= len(data):
                return raw+rva-start
        raise ValueError('export RVA outside PE file data')
    export_rva, export_size = u32(optional+112), u32(optional+116)
    at = offset(export_rva, 40)
    functions = offset(u32(at+28), u32(at+20)*4)
    names = offset(u32(at+32), u32(at+24)*4)
    ordinals = offset(u32(at+36), u32(at+24)*2)
    exports = {}
    for i in range(u32(at+24)):
        name_at = offset(u32(names+i*4), 1)
        end = data.index(b'\0', name_at)
        ordinal = u16(ordinals+i*2)
        if ordinal >= u32(at+20): raise ValueError('invalid export ordinal')
        exports[data[name_at:end].decode('ascii')] = u32(functions+ordinal*4)
    def words(name, count):
        rva = exports.get(name)
        if rva is None or export_rva <= rva < export_rva+export_size:
            raise ValueError('missing or forwarded transition export '+name)
        return struct.unpack_from('<'+'I'*count, data, offset(rva, count*4))
    # TSD reads must use TPIDRRO_EL0, mask low bits, then use the destination
    # register as the load base. The placeholder slot gets retargeted on iOS.
    ret = words('RetToEntryThunk', 5)
    if ret[:3] != (0xaa1e03e9, 0xd53bd070, 0x927df210) or \
       ret[3] & 0xffc003ff != 0xf9400210 or ret[4] != 0xf9403210:
        raise ValueError('RetToEntryThunk/check_target_ec uses non-iOS assembly')
    begin = words('BeginSimulation', 4)
    if begin[:2] != (0xd53bd071, 0x927df231) or \
       begin[2] & 0xffc003ff != 0xf9400231 or begin[3] != 0xf94bc631:
        raise ValueError('BeginSimulation lacks the stack-free iOS TEB read')
    if words('ExitToX64', 1)[0] != 0xf9400130:
        raise ValueError('ExitToX64 lacks the iOS fast-forward bypass')
    print('PASS: DLL uses iOS transition assembly (TSD reads and FFS bypass)')

try:
    verify(args.dll)
except (ValueError, KeyError, struct.error, IndexError) as error:
    raise SystemExit('INVALID: '+str(error)+'; rebuild the ARM64EC translator with iOS ASM flags')
