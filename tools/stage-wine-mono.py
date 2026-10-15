#!/usr/bin/env python3
"""Stage the exact Wine Mono release expected by the pinned Wine mscoree."""
from pathlib import Path, PurePosixPath
import argparse, hashlib, json, shutil, tarfile, tempfile, urllib.request

VERSION = '11.0.0'
SHA256 = '0cd723aa28897f7d7d2702eed6c72e4262255980e212bc2b3c94bfa234abe5fd'
URL = f'https://github.com/wine-mono/wine-mono/releases/download/wine-mono-{VERSION}/wine-mono-{VERSION}-x86.tar.xz'
SOURCE_URL = f'https://github.com/wine-mono/wine-mono/releases/download/wine-mono-{VERSION}/wine-mono-{VERSION}-src.tar.xz'
SOURCE_SHA256 = 'c504cb0b91ce09b72869e063844ce114564d4ac340479a82aa6789f51510e520'
digest = lambda data: hashlib.sha256(data).hexdigest()

def stage(root, archive=None):
    expected = f'#define WINE_MONO_VERSION "{VERSION}"'
    assert expected in (root / 'wine/dlls/mscoree/mscoree_private.h').read_text(), 'Wine Mono version drift'
    cache = root / 'build/wine-mono/cache'; cache.mkdir(parents=True, exist_ok=True)
    archive = archive or cache / f'wine-mono-{VERSION}-x86.tar.xz'
    if not archive.exists():
        temporary = archive.with_suffix('.download')
        try:
            with urllib.request.urlopen(urllib.request.Request(URL, headers={'User-Agent': 'Madeira-runtime-stager'}), timeout=90) as response, temporary.open('wb') as out:
                shutil.copyfileobj(response, out)
            assert digest(temporary.read_bytes()) == SHA256, 'Wine Mono download SHA-256 mismatch'
            temporary.replace(archive)
        finally: temporary.unlink(missing_ok=True)
    assert digest(archive.read_bytes()) == SHA256, 'Wine Mono archive SHA-256 mismatch'
    parent = root / 'app/Madeira/mono'; parent.mkdir(parents=True, exist_ok=True)
    output = parent / f'wine-mono-{VERSION}'
    with tempfile.TemporaryDirectory(prefix='stage-', dir=cache) as scratch:
        scratch = Path(scratch)
        with tarfile.open(archive) as tar:
            members = tar.getmembers()
            assert len(members) == 3380 and sum(m.size for m in members) == 237414451
            for item in members:
                parts = PurePosixPath(item.name)
                assert not parts.is_absolute() and '..' not in parts.parts and parts.parts[0] == output.name
                assert item.isdir() or item.isfile(), 'Unexpected link or special file in runtime'
            tar.extractall(scratch)
        staged = scratch / output.name
        records = {p.relative_to(staged).as_posix(): digest(p.read_bytes()) for p in sorted(staged.rglob('*')) if p.is_file()}
        for key, machine in [('bin/libmono-2.0-x86.dll', 0x14c), ('bin/libmono-2.0-x86_64.dll', 0x8664)]:
            data = (staged / key).read_bytes(); pe = int.from_bytes(data[60:64], 'little')
            assert data[:2] == b'MZ' and data[pe:pe+4] == b'PE\0\0' and int.from_bytes(data[pe+4:pe+6], 'little') == machine
        assert 'lib/mono/4.5/mscorlib.dll' in records and 'etc/mono/config' in records
        if output.exists():
            assert output.resolve().parent == parent.resolve() and not output.is_symlink()
            shutil.rmtree(output)
        shutil.move(str(staged), str(output))
    receipt = {'version': VERSION, 'archive_url': URL, 'archive_sha256': SHA256,
               'source_url': SOURCE_URL, 'source_sha256': SOURCE_SHA256, 'files': records}
    (parent / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(f'PASS: Wine Mono {VERSION} SHA-256, pinned mscoree version, x86/x64 runtime and {len(records)} files staged')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(); parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1]); parser.add_argument('--archive', type=Path)
    args = parser.parse_args(); stage(args.root.resolve(), args.archive)
