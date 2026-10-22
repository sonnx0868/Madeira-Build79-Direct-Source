#!/usr/bin/env python3
"""Provenance for a clean source build, separate from fixed-base migrations.

Every existing producer continues validating its own sources and binaries.
This receipt additionally binds effective submodule sources, compiled object
members and all shipped native payloads. It never accepts a cached native base.
"""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import os
import re
import shutil
import subprocess
import tempfile
import uuid

RECEIPT = 'build/full-native-rebuild.json'
BEGIN = 'build/full-native-start.json'
OBJECT_DIRS = ('build/ntdll-unix/obj', 'build/wineserver/obj', 'build/dxmt-ios/obj',
               'FEX/build-ios', 'FEX/build-arm64ec-ios')

def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()

def source_inventory(root):
    result = {}
    extensions = {'.c', '.cpp', '.h', '.hpp', '.m', '.mm', '.metal', '.patch', '.sh', '.py', '.txt', '.in'}
    for directory, paths in [('', ['build', 'scripts', 'tools', 'patches']),
                             ('wine', []), ('dxmt', []), ('FEX', []), ('madeira-dock', [])]:
        base = root / directory
        entries = subprocess.check_output(['git', '-C', str(base), 'ls-files', '-z', '--', *paths]).decode().split('\0')
        for name in entries:
            path = base / name
            if path.suffix in extensions and path.is_file():
                result[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
    return result

def payload_inventory(root):
    result = {}
    for directory in ('app/Madeira', 'FEX/build-ios'):
        for path in sorted((root / directory).rglob('*')):
            # Official Wine Mono is staged separately after native packaging; its
            # own signed-release receipt validates it. It is not a rebuilt input.
            if directory == 'app/Madeira' and path.is_relative_to(root / 'app/Madeira/mono'):
                continue
            if path.is_file() and path.suffix.lower() in ('.a', '.dll', '.dylib', '.exe'):
                result[path.relative_to(root).as_posix()] = sha(path)
    return result

def begin(root):
    for name in OBJECT_DIRS:
        assert not any((root / name).rglob('*.o')), ('full rebuild refuses preexisting objects', name)
    assert not (root / RECEIPT).exists(), 'full rebuild refuses an inherited full-build receipt'
    start = {'format': 'madeira-cold-native-start-v1', 'nonce': uuid.uuid4().hex,
             'commit': subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip(),
             'ciBuild': os.environ.get('BITRISE_BUILD_SLUG', '')}
    target = root / BEGIN
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(start, indent=2) + '\n')

def verify(root):
    record = json.loads((root / RECEIPT).read_text())
    assert record['format'] == 'madeira-cold-native-rebuild-v1'
    assert record['sources'] == source_inventory(root), 'effective native source differs from full rebuild'
    assert record['payloads'] == payload_inventory(root), 'native payload differs from full rebuild'
    optimizer = module(root, 'dxmt-optimizer-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    archive = root / optimizer.ARCHIVE
    for name, digest in record['dxmtObjects'].items():
        assert hashlib.sha256(srv.object_bytes(archive, name)).hexdigest() == digest
    assert optimizer.compiler_id(srv.object_bytes(archive, 'cache.o')) == record['compilerID']
    assert record['optimizerProof'] == optimizer.replacement_proof(srv.object_bytes(archive, 'airconv_context.o'))
    assert record['paddedProof'] == module(root, 'dxmt-padded-light-bundle').object_proof(
        {name: srv.object_bytes(archive, name) for name in ('airconv_context.o', 'cache.o', 'metallib_writer.o', 'winemetal_unix.o')}, optimizer)
    return record

def stamp(root):
    start = json.loads((root / BEGIN).read_text())
    assert start['format'] == 'madeira-cold-native-start-v1'
    assert start['commit'] == subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
    optimizer = module(root, 'dxmt-optimizer-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    pacing = module(root, 'dxmt-pacing-bundle')
    residency = module(root, 'dxmt-residency-bundle')
    padded = module(root, 'dxmt-padded-light-bundle')
    drain = module(root, 'dxmt-residency-drain-bundle')
    archive = root / optimizer.ARCHIVE
    names = ('airconv_context.o', 'cache.o', 'metallib_writer.o', 'winemetal_unix.o',
             'dxmt_dynamic.o', 'dxmt_command.o')
    objects = {name: srv.object_bytes(archive, name) for name in names}
    for name, data in objects.items():
        assert data == (root / 'build/dxmt-ios/obj' / name).read_bytes(), ('archive does not contain freshly compiled object', name)
    for archive_name, object_names in [('libntdll_unix.a', ('signal_arm64.o', 'thread.o', 'virtual.o', 'sync.o')),
                                       ('libwineserver.a', ('mapping.o',))]:
        for name in object_names:
            folder = 'ntdll-unix' if 'ntdll' in archive_name else 'wineserver'
            compiled = (root / 'build' / folder / 'obj' / name).read_bytes()
            packaged = srv.object_bytes(root / 'app/Madeira' / archive_name, name)
            if archive_name == 'libwineserver.a' and compiled != packaged:
                # The server's documented objcopy sweep prefixes collisions in
                # both definitions and references after compilation. Reproduce
                # that exact transform rather than accepting a different object.
                script = (root / 'build/wineserver/build.sh').read_text()
                body = script.split('COLLISIONS=(', 1)[1].split('\n)', 1)[0]
                collisions = ' '.join(line.split('#', 1)[0] for line in body.splitlines()).split()
                assert collisions and all(re.fullmatch('[a-z_][a-z_0-9]*', n) for n in collisions)
                tool = shutil.which('llvm-objcopy')
                assert tool, 'server object verification requires the producer objcopy tool'
                arguments = [a for n in collisions for a in ('--redefine-sym', '_' + n + '=_ws_' + n)]
                with tempfile.TemporaryDirectory(prefix='madeira-server-proof-') as scratch:
                    target = Path(scratch) / name
                    target.write_bytes(compiled)
                    subprocess.run([tool, *arguments, str(target)], check=True)
                    compiled = target.read_bytes()
            assert packaged == compiled, ('archive differs from its compiled/normalized object', archive_name, name)
    identity = optimizer.compiler_id(objects['cache.o'])
    sources = source_inventory(root)
    common = {'archive_sha256': sha(archive), 'members': pacing.members(archive),
              'producer_commit': start['commit'], 'full_source_build': RECEIPT}
    record = {'format': 'madeira-cold-native-rebuild-v1', 'start': start,
              'sources': sources, 'payloads': payload_inventory(root), 'compilerID': identity,
              'dxmtObjects': {n: hashlib.sha256(d).hexdigest() for n, d in objects.items()},
              'optimizerProof': optimizer.replacement_proof(objects['airconv_context.o']),
              'paddedProof': padded.object_proof({n: objects[n] for n in padded.CHANGED}, optimizer)}
    def write(name, value):
        target = root / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(value, indent=2) + '\n')
    write(RECEIPT, record)
    write('build/dxmt-ios/shader-vector-load.json', dict(common, format='madeira-srv-vector-v1', compilerID=identity,
          sources={s: optimizer.source_sha(root / s) for s in srv.SOURCES}))
    write(optimizer.RECEIPT, dict(common, format='madeira-shader-optimizer-v2', compilerID=identity,
          contextObjectSHA256=record['optimizerProof']['contextObjectSHA256'], object_proof=record['optimizerProof'],
          sources={s: optimizer.source_sha(root / s) for s in optimizer.SOURCES}))
    write(residency.RECEIPT, dict(common, format='madeira-texture-residency-v1',
          object_proof=residency.object_proof(objects['winemetal_unix.o'], optimizer),
          sources={s: optimizer.source_sha(root / s) for s in residency.SOURCES}))
    write(drain.RECEIPT, dict(common, format='madeira-residency-drain-v1', object_sha256=record['dxmtObjects']['winemetal_unix.o'],
          sources={s: optimizer.source_sha(root / s) for s in drain.SOURCES}))
    write(padded.RECEIPT, dict(common, format='madeira-padded-lighting-v1', compilerID=identity,
          objects=record['paddedProof'], sources=padded.source_state(root, optimizer, full=True)))
    verify(root)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--begin', action='store_true')
    parser.add_argument('--stamp', action='store_true')
    args = parser.parse_args()
    assert not (args.begin and args.stamp)
    if args.begin: begin(args.root.resolve())
    elif args.stamp: stamp(args.root.resolve())
    else: verify(args.root.resolve())
    print('PASS: clean native rebuild provenance, effective sources and actual payload bindings')
