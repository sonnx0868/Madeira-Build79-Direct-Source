#!/usr/bin/env python3
"""Bounded Instruments GPU counter recording on the original connected iPad.

Requires the same isolated pymobiledevice3 environment as
profile-device-performance.py. This records the device's schema and binary
packets, without launching games, changing settings or inducing conditions.
Counter collection has overhead: do not treat FPS during sampling as baseline.

Protocol reference: https://github.com/dkw72n/idb/blob/master/instrument_service.py
The service and profile metadata were verified on an M2 running iPadOS 27.0.1.
"""
import asyncio
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
from typing import Annotated, Optional

import typer
from typer_injector import InjectingTyper
from pymobiledevice3.cli.cli_common import ServiceProviderDep, async_command, default_json_encoder
from pymobiledevice3.dtx import (DTXContext, DTXQueue, DTXService, NSError, PInt32, PInt64,
                               dtx_method, dtx_on_dispatch, dtx_on_notification)
from pymobiledevice3.dtx_service import DtxService
from pymobiledevice3.services.dvt.instruments.device_info import DeviceInfo
from pymobiledevice3.services.dvt.instruments.dvt_provider import DvtProvider


class GPUCounterService(DTXService):
    IDENTIFIER = 'com.apple.instruments.server.services.gpu'

    def __init__(self, context: DTXContext):
        super().__init__(context)
        self.events = DTXQueue()

    @dtx_method('requestDeviceGPUInfo')
    async def info(self): ...

    @dtx_method('configureCounters:counterProfile:interval:windowLimit:tracingPID:')
    async def configure(self, accelerator: PInt64, profile: PInt32,
                        interval: PInt64, window_limit: PInt64, pid: PInt32): ...

    @dtx_method('startCollectingCounters')
    async def start(self): ...

    @dtx_method('stopCollectingCounters')
    async def stop(self): ...

    @dtx_on_notification
    async def notice(self, payload):
        await self.events.put(payload)

    @dtx_on_dispatch
    async def dispatch(self, selector, *arguments):
        await self.events.put({'selector': selector, 'arguments': arguments})


class GPUCounters(DtxService[GPUCounterService]):
    pass


cli = InjectingTyper(no_args_is_help=True)


def encode_device_value(value):
    # Unsupported counter profiles may arrive as NSError notifications rather
    # than RPC exceptions. Preserve their domain/code instead of a blank
    # JSON TypeError hiding the reason collection failed.
    if isinstance(value, NSError):
        return {'$NSError': {'domain': value.domain, 'code': value.code,
                             'userInfo': value.user_info}}
    try:
        return default_json_encoder(value)
    except TypeError as error:
        raise TypeError(f'Unsupported GPU response value: {type(value).__name__}') from error


@cli.command()
@async_command
async def record(service_provider: ServiceProviderDep, output: Path,
                 seconds: Annotated[int, typer.Option(min=3, max=15)] = 6,
                 profile: int = 3, pid: Optional[int] = None) -> None:
    """Record --userspace OUTPUT; raw metadata determines available metrics."""
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if any(output.iterdir()):
        raise FileExistsError('Use a new empty directory to preserve earlier recordings.')
    metadata = {'format': 'madeira-device-gpu-counters-v1', 'seconds': seconds,
                'startedUTC': datetime.now(timezone.utc).isoformat(), 'profile': profile,
                'changesToGameSettings': False,
                'attribution': 'Requested tracing PID is not proof of per-encoder or per-process attribution.',
                'fpsMeaning': 'No game FPS is measured by this recorder.'}
    packets = samples = 0
    started = False
    fields = None
    async with DvtProvider(service_provider) as dvt, DeviceInfo(dvt) as device:
        matches = [p for p in await device.proclist() if str(p.get('name', '')).lower() == 'madeira']
        if pid is None and len(matches) == 1:
            pid = int(matches[0]['pid'])
        if pid is None or not any(int(p.get('pid', -1)) == pid for p in matches):
            raise RuntimeError('Select a running Madeira process; this recorder never launches it.')
        metadata['pid'] = pid
        metadata['machTimeInfo'] = await device.mach_time_info()
        async with GPUCounters(dvt) as counters:
            # Enumerate before starting collection. Do not reconfigure/query
            # profiles mid-recording; preserve a single schema for the packets.
            info = await counters.service.info()
            if len(info) != 1 or profile not in info[0].get('supported-counter-profiles', []):
                raise RuntimeError('Expected one GPU supporting the selected counter profile.')
            metadata['deviceGPUInfo'] = info
            await counters.service.configure(info[0]['accelerator-id'], profile, 0, -1, pid)
            with (output / 'events.jsonl').open('w', encoding='utf-8', buffering=1) as events:
                try:
                    started = True
                    await asyncio.wait_for(counters.service.start(), 10)
                    deadline = asyncio.get_running_loop().time() + seconds
                    while asyncio.get_running_loop().time() < deadline:
                        try:
                            event = await asyncio.wait_for(counters.service.events.get(),
                                deadline - asyncio.get_running_loop().time())
                        except asyncio.TimeoutError:
                            break
                        if isinstance(event, (list, tuple)) and event:
                            if event[0] == 0 and len(event) > 4 and isinstance(event[4], dict):
                                # NSKeyedArchiver returns numeric dictionary
                                # keys; JSON renders those same keys as strings.
                                fields = event[4].get(1, event[4].get('1'))
                            if event[0] == 1:
                                if len(event) != 7 or not isinstance(event[4], bytes):
                                    raise ValueError('Unexpected GPU data packet layout; no decoding attempted.')
                                blob = event[4]
                                # Preserve unexpected layouts for inspection;
                                # a recorder must not discard the first useful
                                # packet just because a driver changed format.
                                schema_matches = isinstance(fields, (list, tuple)) and len(blob) == int(event[3]) * (len(fields) + 1) * 8
                                filename = f'counters-{packets:05d}.bin'
                                (output / filename).write_bytes(blob)
                                event = list(event)
                                event[4] = {'file': filename, 'bytes': len(blob),
                                            'sha256': hashlib.sha256(blob).hexdigest(),
                                            'schemaMatches': schema_matches,
                                            'fieldCount': len(fields) if isinstance(fields, (list, tuple)) else None}
                                packets += 1
                                samples += int(event[3])
                        events.write(json.dumps({'receivedUTC': datetime.now(timezone.utc).isoformat(),
                                                 'event': event}, default=encode_device_value) + '\n')
                finally:
                    if started:
                        try:
                            metadata['stoppedAtMachTime'] = await asyncio.wait_for(counters.service.stop(), 5)
                        except Exception as error:
                            metadata['stopError'] = repr(error)
                    metadata.update(finishedUTC=datetime.now(timezone.utc).isoformat(),
                                    packets=packets, samples=samples)
                    (output / 'metadata.json').write_text(json.dumps(metadata, default=default_json_encoder,
                                                                    indent=2), encoding='utf-8')
    if not packets:
        raise RuntimeError('No GPU counter packets received; no counter-based conclusion is valid.')
    print(json.dumps({'output': str(output), 'packets': packets, 'samples': samples}), flush=True)


if __name__ == '__main__':
    cli()
