#!/usr/bin/env python3
"""Report exact cold-build source drift after a cached-build failure; no writes."""
from pathlib import Path
import importlib.util
import json

root = Path(__file__).resolve().parents[2]
receipt = root / 'build/full-native-rebuild.json'
if receipt.exists():
    spec = importlib.util.spec_from_file_location('native_source_proof', root / 'tools/full-native-bundle.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    expected = json.loads(receipt.read_text())['sources']
    actual = module.source_inventory(root)
    changed = sorted(name for name in expected.keys() | actual.keys() if expected.get(name) != actual.get(name))
    print('NATIVE SOURCE DRIFT:', len(changed), 'of', len(expected), 'expected source files')
    for name in changed[:128]:
        print('NATIVE SOURCE DRIFT:', name, 'expected=' + str(expected.get(name)), 'actual=' + str(actual.get(name)))
else:
    print('NATIVE SOURCE DRIFT: no full-build receipt was restored')
