#!/usr/bin/env python3
"""Prove native/no-context memory calls bypass emulator callbacks in ARM64EC ntdll."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('wine/dlls/ntdll/signal_arm64ec.c', 'patches/wine-native-callback-context.patch')
FUNCTIONS = ('NtAllocateVirtualMemory', 'NtAllocateVirtualMemoryEx',
             'NtFreeVirtualMemory', 'NtProtectVirtualMemory')
AREA = 0x12345000
TEB = 0x23456000
MASK64 = (1 << 64) - 1


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def condition(cond, flags):
    if cond in (14, 15):
        return True
    if flags is None:
        return None
    n, z, c, v = flags
    values = (z, c, n, v, c and not z, n == v, not z and n == v)
    value = bool(values[cond >> 1])
    return not value if cond & 1 else value


def logic_mask(word, bits):
    n, imms, immr = (word >> 22) & 1, (word >> 10) & 63, (word >> 16) & 63
    length = ((n << 6) | ((~imms) & 63)).bit_length() - 1
    assert 1 <= length <= (6 if bits == 64 else 5), 'bad logical immediate'
    width = 1 << length
    ones, rotate = (imms & (width - 1)) + 1, immr & (width - 1)
    assert ones < width, 'reserved logical immediate'
    value = (1 << ones) - 1
    value = ((value >> rotate) | (value << (width - rotate))) & ((1 << width) - 1)
    return sum(value << offset for offset in range(0, bits, width))


def prove_path(read_word, start, end, raw_target, current_target, native):
    """Bounded abstract execution of the two early-bypass paths, not a CPU emulator.

    NULL CPU area and current-process=true describe the native helper in the
    report. The second case has a CPU area with InSyscallCallback=1. Unknown
    branches are explored both ways; an unrecognized instruction, indirect
    call, unexpected direct call, loop or escaping branch rejects the proof.
    """
    registers = [None] * 31
    registers[0] = MASK64  # NtCurrentProcess() pseudo handle
    registers[18] = TEB
    pending = [(start, tuple(registers), None, 0, 0, 0)]
    visited, instruction_words, successful, area_loads = set(), {}, 0, set()
    max_steps = 0
    while pending:
        pc, saved, flags, calls, loads, steps = pending.pop()
        registers = list(saved)
        state = (pc, saved, flags, calls, loads)
        assert state not in visited, f'cycle or unresolved converging path at {pc:#x}'
        visited.add(state)
        assert len(visited) <= 4096 and steps < 512, 'unbounded callback bypass path'
        max_steps = max(max_steps, steps)
        assert start <= pc < end and pc % 4 == 0, f'branch escaped wrapper at {pc:#x}'
        word = read_word(pc)
        instruction_words[pc] = word
        next_pc = pc + 4
        bits = 64 if word >> 31 else 32
        mask = (1 << bits) - 1
        rd, rn, rm = word & 31, (word >> 5) & 31, (word >> 16) & 31

        def get(reg, width=64, zero=True):
            value = 0 if reg == 31 and zero else (None if reg == 31 else registers[reg])
            return value if value is None else value & ((1 << width) - 1)

        def put(reg, value, width=64):
            if reg != 31:
                registers[reg] = value if value is None else value & ((1 << width) - 1)

        def enqueue(target, new_flags=flags):
            pending.append((target, tuple(registers), new_flags, calls, loads, steps + 1))

        if word & 0x7c000000 == 0x14000000:  # B / BL
            target = pc + signed(word & 0x3ffffff, 26) * 4
            link = bool(word & 0x80000000)
            if target == raw_target:
                calls += 1
                assert calls == 1 and loads, 'raw syscall reached without one context-gate decision'
                if not link:
                    successful += 1
                    continue
                for reg in range(18):
                    registers[reg] = None
                flags = None
            elif link and target == current_target:
                assert registers[0] == MASK64, 'current-process assumption lost its verified pseudo handle'
                for reg in range(18):
                    registers[reg] = None
                registers[0] = 1  # verified RtlIsCurrentProcess call, current target
                flags = None
            elif not link:
                next_pc = target
            else:
                raise AssertionError(f'unexpected call before native bypass at {pc:#x} -> {target:#x}')
        elif word & 0xfffffc1f in (0xd61f0000, 0xd63f0000):
            raise AssertionError(f'indirect emulator/dispatch call reachable on bypass at {pc:#x}')
        elif word & 0xfffffc1f == 0xd65f0000:  # RET
            assert calls == 1 and loads, 'return without exactly one raw syscall'
            successful += 1
            continue
        elif word & 0x7e000000 == 0x34000000:  # CBZ / CBNZ
            target = pc + signed((word >> 5) & 0x7ffff, 19) * 4
            value = get(rd, bits)
            take = None if value is None else ((value != 0) == bool(word & 0x1000000))
            if take is None:
                enqueue(target)
            if take:
                next_pc = target
        elif word & 0x7e000000 == 0x36000000:  # TBZ / TBNZ
            target = pc + signed((word >> 5) & 0x3fff, 14) * 4
            bit = ((word >> 19) & 31) | ((word >> 26) & 32)
            value = get(rd)
            take = None if value is None else (bool(value & (1 << bit)) == bool(word & 0x1000000))
            if take is None:
                enqueue(target)
            if take:
                next_pc = target
        elif word & 0xff000010 == 0x54000000:  # B.cond
            target = pc + signed((word >> 5) & 0x7ffff, 19) * 4
            take = condition(word & 15, flags)
            if take is None:
                enqueue(target)
            if take:
                next_pc = target
        elif word & 0x3f000000 == 0x39000000:  # integer unsigned-offset load/store
            size, operation = word >> 30, (word >> 22) & 3
            offset = ((word >> 10) & 4095) << size
            base = get(rn, zero=False)
            if operation:
                if base == TEB and offset == 0x1788 and size == 3 and operation == 1:
                    put(rd, 0 if native else AREA)
                    loads += 1
                    area_loads.add(pc)
                elif base == AREA and offset == 1 and size == 0 and operation == 1:
                    put(rd, 1)  # existing EC callback owns the guard
                else:
                    assert base != 0, f'NULL context dereference at {pc:#x}'
                    put(rd, None)
            else:
                assert base != 0, f'NULL context store at {pc:#x}'
                assert not (base is not None and base + offset <= AREA + 1 < base + offset + (1 << size)), 'recursive bypass overwrites outer guard'
        elif word & 0x3e000000 == 0x28000000:  # integer paired load/store prologue/epilogue
            assert rn == 31 and word >> 30 in (0, 2), 'non-stack pair or unsupported paired access'
            if word & (1 << 22):
                put(rd, None)
                put((word >> 10) & 31, None)
        elif word & 0x1f000000 == 0x10000000:  # ADR / ADRP
            delta = signed(((word >> 5) & 0x7ffff) << 2 | ((word >> 29) & 3), 21)
            put(rd, ((pc & ~4095) + (delta << 12)) if word >> 31 else pc + delta)
        elif word & 0x1f800000 == 0x12800000:  # MOVN / MOVZ / MOVK
            operation, shift = (word >> 29) & 3, ((word >> 21) & 3) * 16
            value = ((word >> 5) & 65535) << shift
            assert operation != 1, 'invalid move-wide encoding'
            if operation == 0:
                value = ~value
            elif operation == 3:
                prior = get(rd, bits)
                value = None if prior is None else (prior & ~(65535 << shift)) | value
            put(rd, value, bits)
        elif word & 0x1f000000 == 0x11000000:  # ADD / SUB immediate, CMP
            left = get(rn, bits, zero=False)
            right = ((word >> 10) & 4095) << (12 if word & (1 << 22) else 0)
            subtract = bool(word & (1 << 30))
            value = None if left is None else (left - right if subtract else left + right) & mask
            if word & (1 << 29):
                flags = None if value is None else (bool(value >> (bits - 1)), value == 0,
                    left >= right if subtract else left + right > mask,
                    bool(((left ^ right) if subtract else ~(left ^ right)) & (left ^ value) & (1 << (bits - 1))))
            put(rd, value, bits)
        elif word & 0x1f000000 == 0x0a000000 or word & 0x1f800000 == 0x12000000:
            # Logical register/immediate, including MOV, TST and AND #0xff.
            operation = (word >> 29) & 3
            left = get(rn, bits)
            if word & 0x1f800000 == 0x12000000:
                right = logic_mask(word, bits)
            else:
                right = get(rm, bits)
                shift, amount = (word >> 22) & 3, (word >> 10) & 63
                if right is not None:
                    if shift == 0: right = (right << amount) & mask
                    elif shift == 1: right >>= amount
                    elif shift == 2: right = (signed(right, bits) >> amount) & mask
                    else: right = ((right >> amount) | (right << (bits - amount))) & mask
                    if word & (1 << 21): right = ~right & mask
            value = None
            if left is not None and right is not None:
                value = (left & right) if operation in (0, 3) else ((left | right) if operation == 1 else left ^ right)
            elif operation in (0, 3) and (left == 0 or right == 0):
                value = 0
            put(rd, value, bits)
            if operation == 3:
                flags = None if value is None else (bool(value >> (bits - 1)), value == 0, False, False)
        elif word & 0x1fe00000 == 0x1a400000:  # conditional compare, conservatively unknown NZCV
            flags = None
        elif word & 0x1fe00000 == 0x1a800000:  # CSEL / CSINC / CSINV / CSNEG
            take = condition((word >> 12) & 15, flags)
            left, right = get(rn, bits), get(rm, bits)
            if right is not None:
                if word & (1 << 30): right = ~right & mask
                if word & (1 << 10): right = (right + 1) & mask
            put(rd, left if take else (right if take is False else (left if left == right else None)), bits)
        elif word not in (0xd503201f, 0xd503245f, 0xd503233f, 0xd50323bf):
            raise AssertionError(f'unrecognized instruction {word:08x} at {pc:#x}; bypass proof rejected')
        pending.append((next_pc, tuple(registers), flags, calls, loads, steps + 1))
    assert successful, 'no complete native bypass path'
    code = b''.join(struct.pack('<II', pc, word) for pc, word in sorted(instruction_words.items()))
    return {'completed_paths': successful, 'instructions': len(instruction_words), 'max_steps': max_steps,
            'cpu_area_load_rvas': sorted(area_loads), 'path_code_sha256': hashlib.sha256(code).hexdigest()}


def verify_binary(root, data):
    abi = module(root, 'dxmt-swapchain-bundle')
    pe = abi.PE(data)
    syms = abi.symbols(pe)
    _, ranges, _ = abi.hybrid(pe)
    current = syms['RtlIsCurrentProcess']
    result = {}
    for name in FUNCTIONS:
        symbol = '#' + name + '$hp_target'
        start, raw_target = syms[symbol], syms['syscall_' + name]
        end = min(value for value in syms.values() if value > start)
        assert 16 <= end - start <= 8192, ('unexpected wrapper bounds', name, start, end)
        assert any(kind & 3 == 1 and kind & ~3 <= start and end <= (kind & ~3) + length
                   for kind, length in ranges), ('wrapper not entirely ARM64EC', name)
        read_word = lambda at: struct.unpack('<I', pe.read(pe.offset(at, 4), 4))[0]
        result[name] = {'entry_rva': start, 'raw_syscall_rva': raw_target,
                        'native_current': prove_path(read_word, start, end, raw_target, current, True),
                        'recursive_ec_current': prove_path(read_word, start, end, raw_target, current, False)}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--stamp', action='store_true')
    parser.add_argument('--base', type=Path)
    parser.add_argument('--check-binary', type=Path, help='Read-only compiled proof without receipt access')
    args = parser.parse_args()
    root = args.root.resolve()
    path = args.check_binary or root / 'app/Madeira/arm64ec-windows/ntdll.dll'
    data = path.read_bytes()
    proof = verify_binary(root, data)
    if args.check_binary:
        assert not args.stamp and not args.base
        print(json.dumps(proof, indent=2))
        return
    sha = lambda blob: hashlib.sha256(blob).hexdigest()
    record = {'format': 'madeira-native-callback-context-v1', 'sha256': sha(data), 'proof': proof,
              'sources': {name: sha((root / name).read_bytes().replace(b'\r\n', b'\n')) for name in SOURCES}}
    receipt = root / 'build/wine-pe/native-callback-context.json'
    if args.stamp:
        record['producer_commit'] = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        if args.base:
            abi, compat = module(root, 'dxmt-swapchain-bundle'), module(root, 'check-compatibility-binaries')
            old = args.base.read_bytes()
            assert compat.exports(abi.PE(old)) == compat.exports(abi.PE(data)), 'changed ntdll export contract'
            record['base_sha256'] = sha(old)
        receipt.write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    else:
        saved = json.loads(receipt.read_text(encoding='utf-8'))
        for key, value in record.items():
            assert saved.get(key) == value, ('stale native callback context', key)
    print('PASS: compiled ARM64EC alloc/ex/free/protect NULL-context and recursive paths call raw syscalls once without emulator entry; source/PE identity match')


if __name__ == '__main__':
    main()
