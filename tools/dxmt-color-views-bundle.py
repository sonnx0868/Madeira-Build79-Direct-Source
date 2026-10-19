#!/usr/bin/env python3
"""Bind the reviewed colour-view policy to its compiled ARM64EC frontend."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess

SOURCES = ('patches/dxmt-lossless-color-views.patch',
           'dxmt/src/d3d11/d3d11_resource_helper.cpp')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--stamp', action='store_true')
    args = parser.parse_args()
    root = args.root.resolve()
    data = (root/'app/Madeira/arm64ec-windows/d3d11.dll').read_bytes()
    # The existing ABI gate verifies the EC code map, real x64 COM entries and
    # unchanged exports during a frontend refresh. Require that exact PE here.
    abi = json.loads((root/'build/dxmt-pe/swapchain-hooks.json').read_text(encoding='utf-8'))
    digest = hashlib.sha256(data).hexdigest()
    assert abi['sha256'] == digest, 'colour-view PE lacks matching swapchain ABI verification'
    assert b'DXMT_TYPELESS_PFV\0' in data, 'missing compiled colour-view policy'
    record = {'format': 'madeira-lossless-color-views-v1', 'd3d11_sha256': digest,
              'sources': {name: hashlib.sha256((root/name).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
                          for name in SOURCES}}
    receipt = root/'build/dxmt-pe/color-views.json'
    if args.stamp:
        record['producer_commit'] = subprocess.check_output(
            ['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        receipt.write_text(json.dumps(record, indent=2)+'\n', encoding='utf-8')
    else:
        expected = json.loads(receipt.read_text(encoding='utf-8'))
        for key, value in record.items():
            assert expected[key] == value, ('stale colour-view frontend receipt', key)
    print('PASS: lossless colour-view source and compiled ARM64EC D3D11 identity match')


if __name__ == '__main__':
    main()
