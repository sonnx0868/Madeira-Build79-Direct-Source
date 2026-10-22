#!/usr/bin/env python3
"""Reject lighting candidates that trade corrected images for slower GPU work.

This scores an offline native Metal experiment. Acceptance here is not a claim
about physical iPad FPS, moving-camera quality, or every game scene.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import statistics

ROOT = Path(__file__).resolve().parents[1]


def score(result, quality, receipt, root, candidates):
    assert result['passed'] is True
    assert result['resolution'] == [1920, 1080]
    assert result['parity']['native-reference']['bit_exact'] == 1
    assert result['parity']['fallback-control']['bit_exact'] == 1
    assert result['parity']['positive-control']['bit_exact'] == 0
    for name in ('adaptive_lighting.hpp', 'adaptive_lighting.metal'):
        data = (root / 'build/dxmt-ios' / name).read_bytes().replace(b'\r\n', b'\n')
        assert hashlib.sha256(data).hexdigest() == receipt['files'][name], ('different tested source', name)
    baseline = result['samples_ms']['native-reference']
    assert len(baseline) >= 10 and all(math.isfinite(x) and x > 0 for x in baseline)
    rows = {}
    assert candidates and all(name not in ('native-reference', 'shipping', 'positive-control') for name in candidates)
    if any(name.startswith('tuned-') for name in candidates):
        assert result['parity']['tuned-fallback512']['bit_exact'] == 1
    for name in candidates:
        samples = result['samples_ms'][name]
        assert len(samples) == len(baseline) and all(math.isfinite(x) and x > 0 for x in samples)
        paired = [x / y for x, y in zip(samples, baseline)]
        relative = statistics.median(samples) / statistics.median(baseline)
        wins = sum(x < 1 for x in paired)
        q = quality[name]
        image_ok = q['pixelsOver8'] == 0 and q['maxDisplayLevels'] <= 8
        # A small difference within run-to-run noise is not an improvement.
        # Require >=5% median gain and a win in >=80% of interleaved rounds.
        speed_ok = relative <= .95 and wins >= math.ceil(len(paired) * .8)
        rows[name] = {'relativeGPUTime': relative, 'pairedWins': wins,
                      'pairedRounds': len(paired), 'imagePassed': image_ok,
                      'speedPassed': speed_ok, 'accepted': image_ok and speed_ok}
    return {'format': 'madeira-adaptive-score-v1', 'candidates': rows,
            'anyAccepted': any(row['accepted'] for row in rows.values()),
            'physicalIPadFPSMeasured': False, 'movingCameraQualityMeasured': False,
            'resolution': [1920, 1080], 'textureQualityChanged': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--result', type=Path, required=True)
    parser.add_argument('--quality', type=Path, required=True)
    parser.add_argument('--receipt', type=Path, required=True)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--candidate', nargs='+', default=['native-adaptive', 'native-balanced', 'native-performance'])
    args = parser.parse_args()
    load = lambda path: json.loads(path.read_text(encoding='utf-8'))
    report = score(load(args.result), load(args.quality), load(args.receipt), args.root, args.candidate)
    text = json.dumps(report, indent=2) + '\n'
    if args.output:
        args.output.write_text(text, encoding='utf-8')
    print(text, end='')
    return 0 if report['anyAccepted'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
