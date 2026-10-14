#!/usr/bin/env python3
"""Reject cached native bundles that omit the reviewed compatibility changes."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def verify_mesa_implementation(path):
    data = path.read_bytes()
    assert b'ZINK: failed to load' in data, 'OSMesa does not contain the linked Zink implementation'
    assert b'osmesa-screen-v1' in data, 'OSMesa is missing safe screen creation'


def load_pe(root):
    spec = importlib.util.spec_from_file_location('madeira_pe', root / 'tools/cpu-aot-survey.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.PE


def exports(pe):
    rva, size = pe.directory(0)
    at = pe.offset(rva, 40)
    count = pe.u32(at + 24)
    if count > 65536:
        raise ValueError('unbounded export table')
    names = pe.offset(pe.u32(at + 32), count * 4)
    result = set()
    for i in range(count):
        start = pe.offset(pe.u32(names + i * 4))
        end = pe.data.index(b'\0', start, min(start + 512, len(pe.data)))
        result.add(pe.data[start:end].decode('ascii'))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--stamp-ntdll', action='store_true')
    parser.add_argument('--stamp-mesa', action='store_true')
    parser.add_argument('--pe-only', action='store_true')
    args = parser.parse_args()
    root = args.root
    pe_class = load_pe(root)
    ntdll = root / 'app/Madeira/arm64ec-windows/ntdll.dll'
    receipt = root / 'build/wine-pe/exec-callback-pe.json'
    callback_source = root / 'wine/dlls/ntdll/signal_arm64ec.c'
    callback_patch = root / 'patches/wine-exec-callback.patch'
    expected = dict(binary_sha256=digest(ntdll), source_sha256=digest(callback_source),
                    patch_sha256=digest(callback_patch))
    if args.stamp_ntdll:
        source = callback_source.read_text(encoding='utf-8')
        start = source.index('NTSTATUS SYSCALL_API NtProtectVirtualMemory(')
        end = source.index('NTSTATUS SYSCALL_API NtQuerySystemInformation(', start)
        assert 'leave_syscall_callback();\n            return st;' in source[start:end]
        assert pe_class(ntdll.read_bytes()).machine in (0xa641, 0x8664)
        receipt.write_text(json.dumps(expected, indent=2) + '\n', encoding='utf-8')
        print('Stamped rebuilt ARM64EC callback bridge')
        return
    if args.stamp_mesa:
        mesa = root / 'app/Madeira/gl/libOSMesa.dylib'
        verify_mesa_implementation(mesa)
        patches = root / 'build/mesa-ios/patches'
        document = dict(binary_sha256=digest(mesa),
                        patches={p.name: digest(p) for p in sorted(patches.glob('*.patch'))})
        (mesa.parent / 'mesa-patches.json').write_text(json.dumps(document, indent=2) + '\n', encoding='utf-8')
        print('Stamped rebuilt Mesa capability fixes')
        return
    assert json.loads(receipt.read_text(encoding='utf-8')) == expected, 'ARM64EC callback bridge is stale'
    for arch in ('arm64ec-windows', 'aarch64-windows'):
        assert b'tick-direct-v1' in (root / 'app/Madeira' / arch / 'kernelbase.dll').read_bytes(), 'stale native tick clock: ' + arch
    farm = root / 'app/Madeira/arm64ec-windows'
    for name in ('xaudio2_8.dll', 'xaudio2_9.dll'):
        data = (farm / name).read_bytes()
        pe = pe_class(data)
        assert pe.machine in (0xa641, 0x8664), (name, 'not ARM64EC')
        assert {'XAudio2Create', 'CreateAudioReverb', 'CreateAudioVolumeMeter'} <= exports(pe), (name, 'missing exports')
        assert b'Using FAudio version' in data, (name, 'missing FAudio implementation')
        for imported in pe.imports():
            assert (farm / imported).is_file(), (name, 'missing dependency', imported)
    if not args.pe_only:
        assert b'WINE_IOS_JIT_HOLE' in (root / 'app/Madeira/libntdll_unix.a').read_bytes(), 'stale split-pool archive'
        assert b'teb-slots-v1' in (root / 'app/Madeira/libntdll_unix.a').read_bytes(), 'stale trampoline archive'
        mesa = root / 'app/Madeira/gl/libOSMesa.dylib'
        verify_mesa_implementation(mesa)
        document = json.loads((mesa.parent / 'mesa-patches.json').read_text(encoding='utf-8'))
        patches = root / 'build/mesa-ios/patches'
        assert document == dict(binary_sha256=digest(mesa),
                                patches={p.name: digest(p) for p in sorted(patches.glob('*.patch'))}), 'Mesa capability patch is stale'
    print('PASS: rebuilt callback bridge, XAudio2 backends and native compatibility provenance')


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError) as error:
        raise SystemExit('INVALID compatibility bundle: ' + str(error) + '; run native bootstrap')
