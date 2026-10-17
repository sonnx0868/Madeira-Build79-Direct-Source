#!/usr/bin/env python3
"""Rank original-device stage intervals without double-counting overlap.

The optional CPU/GPU timestamp calibration determines milliseconds. Without
valid calibration, preserve raw GPU timestamp units instead of inventing ms.
"""
import argparse
import json
import math
from pathlib import Path


def union_duration(intervals):
    total = 0
    start = end = None
    for a, b in sorted(intervals):
        if start is None:
            start, end = a, b
        elif a <= end:
            end = max(end, b)
        else:
            total += end - start
            start, end = a, b
    return total + (end - start if start is not None else 0)


def analyze(data):
    if data.get('format') != 'madeira-gpu-stages-v1':
        raise ValueError('Unsupported stage timing format')
    calibration = data.get('clockCalibration', {})
    scale = None
    if calibration.get('available'):
        cpu = calibration['cpuEnd'] - calibration['cpuStart']
        gpu = calibration['gpuEnd'] - calibration['gpuStart']
        numer, denom = calibration['cpuMachNumer'], calibration['cpuMachDenom']
        if cpu > 0 and gpu > 0 and numer > 0 and denom > 0:
            candidate = cpu * numer / denom / gpu / 1e6
            if math.isfinite(candidate) and candidate > 0:
                scale = candidate
    rows, intervals = [], []
    unavailable = 0
    for buffer in data.get('buffers', []):
        for p in buffer.get('passes', []):
            for stage in ('vertex', 'fragment', 'encoder'):
                value = p.get(stage)
                if not value:
                    continue
                if not value.get('available'):
                    unavailable += 1
                    continue
                a, b = value['start'], value['end']
                if not isinstance(a, int) or not isinstance(b, int) or a < 0 or b < a or b >= 2**64 - 1:
                    raise ValueError('Invalid GPU timestamp interval')
                if value.get('duration') != b - a:
                    raise ValueError('Stage duration does not match timestamps')
                intervals.append((a, b))
                row = {'buffer': buffer['buffer'], 'pass': p['pass'], 'kind': p['kind'],
                       'stage': stage, 'pipelines': p.get('pipelines', []), 'rawDuration': b - a}
                if scale is not None:
                    row['milliseconds'] = (b - a) * scale
                rows.append(row)
    covered = union_duration(intervals)
    result = {'format': 'madeira-gpu-stage-analysis-v1',
              'complete': bool(data.get('complete')), 'available': bool(rows),
              'sampleRole': data.get('sampleRole'), 'calibrationAvailable': scale is not None,
              'coveredRawDuration': covered, 'unavailableStages': unavailable,
              'rankedStages': sorted(rows, key=lambda r: r['rawDuration'], reverse=True),
              'note': 'Coverage is the union of sampled stage intervals. It is not the sum, total frame time, GPU utilization, or an unperturbed FPS measurement.'}
    if scale is not None:
        result['coveredMilliseconds'] = covered * scale
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = analyze(json.loads(args.input.read_text(encoding='utf-8')))
    encoded = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.write_text(encoded, encoding='utf-8')
    else:
        print(encoded, end='')
