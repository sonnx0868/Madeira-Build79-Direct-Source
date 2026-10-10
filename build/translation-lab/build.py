#!/usr/bin/env python3
"""Build the translation experiment; does not modify Wine, FEX or game files."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, struct, subprocess, zipfile

parser = argparse.ArgumentParser()
parser.add_argument('--toolchain', type=Path, required=True, help='llvm-mingw bin directory')
parser.add_argument('--out', type=Path, required=True)
parser.add_argument('--smoke', action='store_true', help='Run the x64 control on Windows')
parser.add_argument('--stage', type=Path, help='Stage validated native tools into the iOS bundle folder')
args = parser.parse_args()
src = Path(__file__).resolve().parent
args.out.mkdir(parents=True, exist_ok=True)

def compiler(arch):
    suffix = '.exe' if os.name == 'nt' else ''
    path = args.toolchain / (arch + '-w64-mingw32-clang' + suffix)
    if not path.is_file(): raise SystemExit('Missing compiler: ' + str(path))
    return str(path)

common = ['-O2', '-std=c11', '-Wall', '-Wextra', '-Werror', '-Wl,--no-insert-timestamp']
subprocess.run([compiler('arm64ec'), *common, '-shared', str(src / 'native.c'),
                '-o', str(args.out / 'madeira-native-work.dll')], check=True)
subprocess.run([compiler('x86_64'), *common, '-shared', '-nostdlib', '-Wl,--entry,DllMain', str(src / 'native.c'),
                '-o', str(args.out / 'madeira-control-work.dll')], check=True)
subprocess.run([compiler('x86_64'), *common, str(src / 'probe.c'),
                '-o', str(args.out / 'madeira-translation-lab.exe')], check=True)

def pe_info(path):
    data = path.read_bytes()
    pe = struct.unpack_from('<I', data, 60)[0]
    assert data[pe:pe+4] == b'PE\0\0'
    machine, sections = struct.unpack_from('<HH', data, pe+4)
    optional_size = struct.unpack_from('<H', data, pe+20)[0]
    optional = pe+24
    assert struct.unpack_from('<H', data, optional)[0] == 0x20b
    image_base = struct.unpack_from('<Q', data, optional+24)[0]
    mappings = []
    for i in range(sections):
        at = optional+optional_size+i*40
        virtual_size, rva, raw_size, offset = struct.unpack_from('<IIII', data, at+8)
        mappings.append((rva, min(max(virtual_size, raw_size), raw_size), offset))
    def file_offset(rva, length=4):
        for start, size, offset in mappings:
            if start <= rva and rva+length <= start+size:
                return offset+rva-start
        raise ValueError('RVA outside file data: ' + hex(rva))
    load_rva, load_size = struct.unpack_from('<II', data, optional+112+10*8)
    metadata = 0
    if load_rva and load_size >= 208:
        metadata = struct.unpack_from('<Q', data, file_offset(load_rva+200, 8))[0]
    version, native_ranges, entries = 0, 0, 0
    if metadata:
        at = file_offset(metadata-image_base, 56)
        version, code_map, count = struct.unpack_from('<III', data, at)
        assert count < 100000
        for i in range(count):
            start, length = struct.unpack_from('<II', data, file_offset(code_map+i*8, 8))
            if start & 1 and length: native_ranges += 1
        entries = struct.unpack_from('<I', data, at+48)[0]
    return {'machine': hex(machine), 'chpe_version': version, 'native_code_ranges': native_ranges,
            'entry_thunks': entries, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}

receipt = {name: pe_info(args.out/name) for name in
           ['madeira-native-work.dll', 'madeira-control-work.dll', 'madeira-translation-lab.exe']}
native = receipt['madeira-native-work.dll']
assert native['machine'] == '0x8664' and native['chpe_version'] > 0
assert native['native_code_ranges'] > 0 and native['entry_thunks'] >= 3
assert receipt['madeira-translation-lab.exe']['chpe_version'] == 0
(args.out/'receipt.json').write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
shutil.copyfile(src/'README.md', args.out/'README.md')
notices = 'Translation lab source: SPDX-License-Identifier: MIT.\n\n'
for name, path in [
    ('LLVM toolchain', args.toolchain.parent/'LICENSE.TXT'),
    ('MinGW-w64 runtime', args.toolchain.parent/'arm64ec-w64-mingw32/share/mingw32/COPYING.MinGW-w64-runtime.txt')]:
    if not path.is_file(): raise SystemExit('Missing runtime notice: ' + str(path))
    notices += name + '\n' + path.read_text(encoding='utf-8', errors='replace') + '\n\n'
(args.out/'notices.txt').write_text(notices, encoding='utf-8')
with zipfile.ZipFile(args.out/'Madeira-translation-lab.zip', 'w', zipfile.ZIP_DEFLATED) as package:
    for name in ['madeira-native-work.dll', 'madeira-translation-lab.exe', 'README.md', 'receipt.json', 'notices.txt']:
        package.write(args.out/name, arcname=name)
print('PASS: genuine ARM64EC code ranges/entry thunks and x64 caller built; package ready')
if args.stage:
    args.stage.mkdir(parents=True, exist_ok=True)
    for name in ['madeira-native-work.dll', 'madeira-translation-lab.exe', 'receipt.json', 'notices.txt']:
        shutil.copyfile(args.out/name, args.stage/name)
if args.smoke:
    subprocess.run([str(args.out/'madeira-translation-lab.exe'), str(args.out/'madeira-control-work.dll'), '--smoke'],
                    check=True, timeout=30)
