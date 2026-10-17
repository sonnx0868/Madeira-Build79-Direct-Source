#!/usr/bin/env python3
"""Bounded, read-only CPU/GPU observation on the original connected iPad.

Requires pymobiledevice3 (installed in an isolated task venv). Does not launch,
kill, suspend, change settings, or induce performance/thermal conditions.
The device chooses which telemetry attributes are available; absent hardware
counters remain absent. Core Animation FPS is not the game's Present FPS.
"""
import asyncio
from contextlib import redirect_stdout
from datetime import datetime, timezone
import json
from pathlib import Path
import time
import traceback
from typing import Annotated, Optional

import typer
from typer_injector import InjectingTyper
from pymobiledevice3.cli.cli_common import ServiceProviderDep, async_command, default_json_encoder
from pymobiledevice3.services.dvt.instruments.condition_inducer import ConditionInducer
from pymobiledevice3.services.dvt.instruments.core_profile_session_tap import CoreProfileSessionTap
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
    kernel: bool = False,
    kernel_scheduler: bool = False,
) -> None:
    """Collect original-device telemetry, e.g. --userspace --seconds 90 OUTPUT."""
    output = output.resolve(); output.mkdir(parents=True, exist_ok=True)
    if (output / 'metadata.json').exists() or (output / 'samples.jsonl').exists():
        raise FileExistsError('Use a new output directory to preserve the previous recording.')
    if kernel_scheduler and not kernel:
        raise ValueError('--kernel-scheduler also requires --kernel.')
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
        available_process = list(await device.sysmon_process_attributes())
        available_system = list(await device.sysmon_system_attributes())
        # Avoid expensive full process accounting at 2 Hz while investigating
        # a CPU/energy regression. CPU load is sampled by the tap separately.
        wanted_process = ['pid', 'name', 'cpuUsage', 'cpuTotalUser', 'cpuTotalSystem',
                          'appSleep', 'priority', 'latencyQosTier', 'throughputQosTier',
                          'physFootprint', 'threadCount', 'numRunning']
        wanted_system = ['physMemSize', 'vmPageIns', 'vmCompressorPageCount', '__vmSwapUsage']
        process_attributes = [name for name in wanted_process if name in available_process]
        system_attributes = [name for name in wanted_system if name in available_system]
        if not {'pid', 'cpuUsage'}.issubset(process_attributes) or not system_attributes:
            raise RuntimeError('Required CPU accounting attributes are not available on this device.')
        metadata = {
            'format': 'madeira-original-device-profile-v1',
            'startedUTC': datetime.now(timezone.utc).isoformat(),
            'durationSeconds': seconds, 'pid': pid,
            'processAttributes': process_attributes,
            'systemAttributes': system_attributes,
            'availableProcessAttributes': available_process,
            'availableSystemAttributes': available_system,
            'sysmonIntervalMs': 1000,
            'graphicsFPSMeaning': 'Core Animation whole-display FPS, not Madeira game Present FPS',
            'hardwareCounterAvailability': 'Only attributes actually returned by this device are recorded',
            'changesToGameSettings': False,
            'kernelTraceRequested': kernel,
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
                tap = Sysmontap(dvt, process_attributes, system_attributes, interval_ms=1000)
                async with tap:
                    async for sample in tap:
                        # Keep the selected app and system metrics. Other app
                        # processes/paths do not become analysis artifacts.
                        if isinstance(sample, dict):
                            row = dict(sample)
                            if 'Processes' in row:
                                row['Processes'] = {str(k): v for k, v in row['Processes'].items() if str(k) == str(pid)}
                                if row['Processes']:
                                    counts['selected-process-rows'] = counts.get('selected-process-rows', 0) + 1
                            emit('sysmon', row)

            async def kernel_samples():
                # Exact subclasses from this device's trace-code table:
                # MACH scheduler/edge policy, CPU PM, GPU perf controller/role.
                # No per-event backtrace sampling or induced hardware state.
                filters = {0x01A90000, 0x01A60000, 0x01A30000,
                           0x05310000, 0x27300000, 0x261E0000, 0x261F0000}
                # Full context-switch tracing can produce ~15 MB/s on M2.
                # Opt in separately; the default keeps cluster/group policy
                # events without collecting every Mach scheduler transition.
                if kernel_scheduler: filters.add(0x01400000)
                time_config = await CoreProfileSessionTap.get_time_config(dvt)
                trace_codes = await CoreProfileSessionTap.get_trace_codes(dvt)
                recorded_time = {k: v for k, v in time_config.items() if k != 'timezone'}
                recorded_time['timezoneOffsetSeconds'] = time_config['timezone'].utcoffset(None).total_seconds()
                (output / 'kernel-time.json').write_text(json.dumps(recorded_time, default=default_json_encoder, indent=2), encoding='utf-8')
                (output / 'kernel-trace-codes.json').write_text(json.dumps(trace_codes, default=default_json_encoder, indent=2), encoding='utf-8')
                emit('kernel-start', {'filters': sorted(filters), 'sampleCallstacks': False})
                async with CoreProfileSessionTap(dvt, time_config, filters, sample_callstacks=False) as tap:
                    with (output / 'kernel.kdebug').open('wb') as raw, (output / 'kernel-receive.log').open('w', encoding='utf-8') as receiving:
                        with redirect_stdout(receiving):
                            try: await tap.dump(raw)
                            finally: raw.flush()

            async def observe(name, operation):
                try: await operation()
                except asyncio.CancelledError: raise
                except Exception as error:
                    emit('service-error', {'service': name, 'error': str(error), 'errorType': type(error).__name__,
                                           'traceback': traceback.format_exc(limit=8)})

            tasks = [asyncio.create_task(observe(name, operation)) for name, operation in
                     [('graphics', graphics_samples), ('energy', energy_samples), ('sysmon', system_samples)]]
            if kernel: tasks.append(asyncio.create_task(observe('kernel', kernel_samples)))
            print(f'Collecting original-device profile for Madeira PID {pid}: {seconds}s', flush=True)
            try: await asyncio.sleep(seconds)
            finally:
                for task in tasks: task.cancel()
                await asyncio.gather(*tasks, return_exceptions=True)
                metadata['finishedUTC'] = datetime.now(timezone.utc).isoformat()
                metadata['sampleCounts'] = counts
                metadata['selectedAppObserved'] = counts.get('selected-process-rows', 0) > 0
                if kernel:
                    raw = output / 'kernel.kdebug'
                    metadata['kernelTraceBytes'] = raw.stat().st_size if raw.exists() else 0
                (output / 'metadata.json').write_text(json.dumps(metadata, default=default_json_encoder, indent=2), encoding='utf-8')
    if not counts.get('graphics') or not counts.get('sysmon'):
        raise RuntimeError('Original-device profile incomplete; inspect service-error records. No performance conclusion is valid from missing samples.')
    print(json.dumps({'output': str(output), 'samples': counts}), flush=True)


if __name__ == '__main__':
    cli()
