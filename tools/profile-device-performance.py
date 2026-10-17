#!/usr/bin/env python3
"""Bounded, read-only CPU/GPU observation on the original connected iPad.

Requires pymobiledevice3 (installed in an isolated task venv). Does not launch,
kill, suspend, change settings, or induce performance/thermal conditions.
The device chooses which telemetry attributes are available; absent hardware
counters remain absent. Core Animation FPS is not the game's Present FPS.
"""
import asyncio
from datetime import datetime, timezone
import json
from pathlib import Path
import time
from typing import Annotated, Optional

import typer
from typer_injector import InjectingTyper
from pymobiledevice3.cli.cli_common import ServiceProviderDep, async_command, default_json_encoder
from pymobiledevice3.services.dvt.instruments.condition_inducer import ConditionInducer
from pymobiledevice3.services.dvt.instruments.device_info import DeviceInfo
from pymobiledevice3.services.dvt.instruments.dvt_provider import DvtProvider
from pymobiledevice3.services.dvt.instruments.energy_monitor import EnergyMonitor
from pymobiledevice3.services.dvt.instruments.graphics import Graphics
from pymobiledevice3.services.dvt.instruments.sysmontap import Sysmontap

cli = InjectingTyper(no_args_is_help=True)


@cli.command()
@async_command
async def record(
    service_provider: ServiceProviderDep,
    output: Path,
    seconds: Annotated[int, typer.Option(min=10, max=180)] = 90,
    pid: Optional[int] = None,
) -> None:
    """Collect original-device telemetry, e.g. --userspace --seconds 90 OUTPUT."""
    output = output.resolve(); output.mkdir(parents=True, exist_ok=True)
    counts = {}
    tasks = []
    async with DvtProvider(service_provider) as dvt, DeviceInfo(dvt) as device:
        processes = await device.proclist()
        matches = [p for p in processes if str(p.get('name', '')).lower() == 'madeira']
        if pid is None:
            if len(matches) != 1:
                raise RuntimeError('Expected one running Madeira process; open Madeira and REPLACED first, or specify --pid.')
            pid = int(matches[0]['pid'])
        if not any(int(p.get('pid', -1)) == pid for p in matches):
            raise RuntimeError('The selected PID is not a running Madeira process.')
        metadata = {
            'format': 'madeira-original-device-profile-v1',
            'startedUTC': datetime.now(timezone.utc).isoformat(),
            'durationSeconds': seconds, 'pid': pid,
            'processAttributes': list(await device.sysmon_process_attributes()),
            'systemAttributes': list(await device.sysmon_system_attributes()),
            'graphicsFPSMeaning': 'Core Animation whole-display FPS, not Madeira game Present FPS',
            'hardwareCounterAvailability': 'Only attributes actually returned by this device are recorded',
            'changesToGameSettings': False,
        }
        async with ConditionInducer(dvt) as conditions:
            # Inspect available/active condition information; never set/clear it.
            metadata['conditionInformation'] = await conditions.list()
        (output / 'metadata.json').write_text(json.dumps(metadata, default=default_json_encoder, indent=2), encoding='utf-8')
        with (output / 'samples.jsonl').open('w', encoding='utf-8', buffering=1) as log:
            def emit(kind, data):
                counts[kind] = counts.get(kind, 0) + 1
                row = {'kind': kind, 'receivedMonotonic': time.monotonic(),
                       'receivedUTC': datetime.now(timezone.utc).isoformat(), 'data': data}
                log.write(json.dumps(row, default=default_json_encoder) + '\n')

            async def graphics_samples():
                async with Graphics(dvt) as graphics:
                    async for sample in graphics: emit('graphics', sample)

            async def energy_samples():
                async with EnergyMonitor(dvt, [pid]) as monitor:
                    while True:
                        emit('energy', await monitor._sample_once())
                        await asyncio.sleep(1)

            async def system_samples():
                tap = await Sysmontap.create(dvt, interval=500)
                async with tap:
                    async for sample in tap:
                        # Keep the selected app and system metrics. Other app
                        # processes/paths do not become analysis artifacts.
                        if isinstance(sample, dict):
                            row = dict(sample)
                            if 'Processes' in row:
                                row['Processes'] = {str(k): v for k, v in row['Processes'].items() if str(k) == str(pid)}
                            emit('sysmon', row)

            async def observe(name, operation):
                try: await operation()
                except asyncio.CancelledError: raise
                except Exception as error: emit('service-error', {'service': name, 'error': str(error)})

            tasks = [asyncio.create_task(observe(name, operation)) for name, operation in
                     [('graphics', graphics_samples), ('energy', energy_samples), ('sysmon', system_samples)]]
            print(f'Collecting original-device profile for Madeira PID {pid}: {seconds}s', flush=True)
            try: await asyncio.sleep(seconds)
            finally:
                for task in tasks: task.cancel()
                await asyncio.gather(*tasks, return_exceptions=True)
                metadata['finishedUTC'] = datetime.now(timezone.utc).isoformat()
                metadata['sampleCounts'] = counts
                (output / 'metadata.json').write_text(json.dumps(metadata, default=default_json_encoder, indent=2), encoding='utf-8')
    if not counts.get('graphics') or not counts.get('sysmon'):
        raise RuntimeError('Original-device profile incomplete; inspect service-error records. No performance conclusion is valid from missing samples.')
    print(json.dumps({'output': str(output), 'samples': counts}), flush=True)


if __name__ == '__main__':
    cli()
