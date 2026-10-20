#!/usr/bin/env python3
"""Execute an audited, encrypted Metal fixture; never publish game resources.

This workflow produces private benchmark artifacts, not an app or an FPS claim.
The expected archive SHA is supplied separately from its encrypted download.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import urllib.parse
import urllib.request
import zipfile


def run(args, **kwargs):
    return subprocess.run([str(a) for a in args], check=True, **kwargs)


def main():
    out = Path(os.environ['BITRISE_DEPLOY_DIR'])
    out.mkdir(parents=True, exist_ok=True)
    url = os.environ['MADEIRA_GPU_FIXTURE_URL']
    parsed = urllib.parse.urlsplit(url)
    if parsed.scheme != 'https' or parsed.hostname != 'release-assets.githubusercontent.com':
        raise ValueError('Expected an authenticated GitHub asset redirect')
    digest = os.environ['MADEIRA_GPU_FIXTURE_SHA256']
    key = os.environ['MADEIRA_GPU_FIXTURE_KEY']
    if not all(re.fullmatch('[0-9a-f]{64}', value) for value in (digest, key)):
        raise ValueError('Invalid fixture digest or transfer key')
    with tempfile.TemporaryDirectory(prefix='madeira-metal-fixture-') as temp:
        work = Path(temp)
        encrypted = work / 'input.aesgcm'
        with urllib.request.urlopen(url, timeout=90) as source, encrypted.open('wb') as dest:
            size = 0
            for block in iter(lambda: source.read(1024 * 1024), b''):
                size += len(block)
                if size > 512 * 1024**2:
                    raise ValueError('Fixture exceeds transfer limit')
                dest.write(block)
        decrypt = work / 'decrypt.swift'
        decrypt.write_text('''import Foundation
import CryptoKit
let hex=ProcessInfo.processInfo.environment["MADEIRA_GPU_FIXTURE_KEY"]!
let bytes=stride(from:0,to:hex.count,by:2).map { i -> UInt8 in
 let start=hex.index(hex.startIndex,offsetBy:i)
 return UInt8(hex[start..<hex.index(start,offsetBy:2)],radix:16)!
}
let box=try AES.GCM.SealedBox(combined:Data(contentsOf:URL(fileURLWithPath:CommandLine.arguments[1])))
try AES.GCM.open(box,using:SymmetricKey(data:Data(bytes))).write(to:URL(fileURLWithPath:CommandLine.arguments[2]))
''')
        run(['xcrun', 'swiftc', decrypt, '-o', work / 'decrypt'])
        archive = work / 'input.zip'
        run([work / 'decrypt', encrypted, archive])
        if hashlib.sha256(archive.read_bytes()).hexdigest() != digest:
            raise ValueError('Fixture digest mismatch')
        inputs = work / 'inputs'
        inputs.mkdir()
        with zipfile.ZipFile(archive) as z:
            entries = z.infolist()
            names = [e.filename for e in entries]
            if len(names) != len(set(names)) or len(names) > 256:
                raise ValueError('Invalid fixture member count')
            if sum(e.file_size for e in entries) > 1024 * 1024**2:
                raise ValueError('Fixture exceeds extraction limit')
            for entry in entries:
                if not re.fullmatch(r'[A-Za-z0-9_.-]+', entry.filename):
                    raise ValueError('Fixture members must be flat files')
                if (entry.external_attr >> 16) & 0o170000 == 0o120000:
                    raise ValueError('Fixture must not contain symlinks')
            z.extractall(inputs)
        host = inputs / 'pyramid-replay.m'
        if not host.is_file() or not (inputs / 'pyramid-profile.json').is_file():
            raise ValueError('Expected audited pyramid fixture')
        receipt = {'fixtureSHA256': digest,
                   'files': {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                             for p in sorted(inputs.iterdir())},
                   'gameFPSMeasured': False, 'iPadThermalMeasured': False}
        (out / 'fixture-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
        executable = work / 'pyramid-replay'
        run(['xcrun', 'clang', '-fobjc-arc', '-O2', '-framework', 'Foundation',
             '-framework', 'Metal', host, '-o', executable])
        run([executable, inputs, out / 'pyramid-results.json', '12'],
            env=dict(os.environ, MTL_DEBUG_LAYER='1'), timeout=240)


if __name__ == '__main__':
    main()
