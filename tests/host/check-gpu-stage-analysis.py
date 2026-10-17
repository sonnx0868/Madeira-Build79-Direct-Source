#!/usr/bin/env python3
"""Check overlap, missing calibration and unavailable stage interpretation."""
import importlib.util
from pathlib import Path

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('stage_analysis', root / 'tools/analyze-gpu-stages.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
assert module.union_duration([(0, 100), (50, 150), (200, 210)]) == 160
data = {'format': 'madeira-gpu-stages-v1', 'complete': True, 'buffers': [
    {'buffer': 0, 'passes': [
        {'pass': 0, 'kind': 0, 'vertex': {'available': True, 'start': 100, 'end': 200, 'duration': 100},
         'fragment': {'available': True, 'start': 150, 'end': 300, 'duration': 150}},
        {'pass': 1, 'kind': 1, 'encoder': {'available': False}}]}]}
result = module.analyze(data)
assert result['coveredRawDuration'] == 200 and result['unavailableStages'] == 1
assert 'coveredMilliseconds' not in result and 'milliseconds' not in result['rankedStages'][0]
data['clockCalibration'] = {'available': True, 'cpuStart': 100, 'cpuEnd': 200,
    'gpuStart': 1000, 'gpuEnd': 1200, 'cpuMachNumer': 2, 'cpuMachDenom': 1}
result = module.analyze(data)
assert abs(result['coveredMilliseconds'] - 0.0002) < 1e-12
assert result['rankedStages'][0]['stage'] == 'fragment'
data['buffers'][0]['passes'][0]['vertex']['end'] = 99
try:
    module.analyze(data)
except ValueError:
    pass
else:
    raise AssertionError('Negative GPU interval was accepted')
print('PASS: overlapping stages are not summed, missing counters/calibration stay unavailable, invalid intervals are rejected')
