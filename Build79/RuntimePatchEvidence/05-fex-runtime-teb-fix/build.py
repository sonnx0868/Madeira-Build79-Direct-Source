"""Build the personal CPU metadata correction in an isolated PE output tree."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import time

HERE = Path(__file__).resolve().parent
BASE = HERE.parent / 'fex-windows-baseline'
BUILD = HERE / 'build'
TOOL = BASE / 'toolchains/llvm-mingw-20260421-ucrt-ubuntu-22.04-x86_64'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def verify_sources():
    provenance = json.loads((HERE / 'source-provenance.json').read_text())
    for item in provenance['files']:
        for role in ('original', 'patched'):
            if sha(Path(item[role])) != item[role + '_sha256']:
                raise RuntimeError('Changed reviewed source: ' + item[role])
    return provenance

def main():
    provenance = verify_sources()
    baseline_dll = BASE / 'build/Bin/libarm64ecfex.dll'
    baseline_hash = sha(baseline_dll)
    if baseline_hash != 'e2136d47e45958aa3a305fedf2b3e66baddedf4e992543d3ba22e9c24b16c7bd':
        raise RuntimeError('Baseline artifact changed')
    baseline_report = json.loads((BASE / 'build-report.json').read_text())
    commands = []
    for record in baseline_report['commands']:
        cmd = [arg.replace(str(BASE / 'build'), str(BUILD)) for arg in record['argv']]
        cmd = [arg + ' -ivfsoverlay ' + str(HERE / 'overlay.json')
               if arg == '-DCMAKE_CXX_FLAGS=-DFEX_IOS_HOST=1' else arg for arg in cmd]
        if cmd[0] == 'cmake' and '-S' in cmd:
            cmd.append('-DCMAKE_ASM_FLAGS=-DFEX_IOS_HOST=1 -ivfsoverlay ' + str(HERE/'overlay.json'))
        commands.append(cmd)
    if len(commands) != 2 or str(BUILD) not in commands[0] or str(BUILD) not in commands[1]:
        raise RuntimeError('Unexpected baseline build command contract')
    report = {'scope': 'Personal CPU metadata bounds correction; Windows ARM64EC PE only',
              'device_tested': False, 'installed': False, 'baseline_sha256': baseline_hash,
              'source_provenance': provenance, 'overlay_sha256': sha(HERE / 'overlay.json'), 'commands': []}
    env = dict(os.environ)
    env['PATH'] = str(TOOL / 'bin') + os.pathsep + env['PATH']
    for index, cmd in enumerate(commands):
        logpath = HERE / ('configure.log' if index == 0 else 'compile.log')
        start = time.monotonic()
        with logpath.open('w') as log:
            result = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, env=env)
        report['commands'].append({'argv': cmd, 'exit': result.returncode,
                                   'seconds': time.monotonic() - start, 'log': logpath.name})
        (HERE / 'build-report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(logpath.name, 'exit', result.returncode, flush=True)
        if result.returncode:
            print('\n'.join(logpath.read_text(errors='replace').splitlines()[-35:]))
            return result.returncode
    verify_sources()
    if sha(baseline_dll) != baseline_hash:
        raise RuntimeError('Baseline changed during isolated build')
    dll = BUILD / 'Bin/libarm64ecfex.dll'
    assembly=[c for c in json.loads((BUILD/'compile_commands.json').read_text()) if c['file'].endswith('.S')]
    if not assembly or any('-DFEX_IOS_HOST=1' not in c['command'] for c in assembly):
        raise RuntimeError('An assembly input missed the iOS host definition')
    report['assembly_ios_host_definition_verified']=True
    report['assembly_inputs']=[c['file'] for c in assembly]
    report.update(status='linked', artifact={'path': str(dll), 'bytes': dll.stat().st_size, 'sha256': sha(dll)})
    (HERE / 'build-report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['artifact']), flush=True)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
