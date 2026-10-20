#!/usr/bin/env python3
"""Load the shipped bridge using a real installed Ren'Py, in an isolated project.

Only the engine/common files are used; the user's game script and saves are not
loaded or modified. No UI or gameplay automation is necessary for this check.
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--engine-root', type=Path, required=True)
parser.add_argument('--python', type=Path, required=True)
options = parser.parse_args()
root = Path(__file__).resolve().parents[2]
engine = options.engine_root.resolve()
launcher = next(path for path in engine.glob('*.py') if b'renpy.bootstrap' in path.read_bytes())
token = '00000000-0000-0000-0000-000000000123'
with tempfile.TemporaryDirectory(prefix='madeira-real-renpy-') as scratch:
    folder = Path(scratch)
    game = folder / 'game'
    game.mkdir()
    (game / 'zz_madeira_reading_progress_v1.rpy').write_bytes(
        (root / 'app/Madeira/MadeiraReadingProgress.rpy').read_bytes())
    (game / '.madeira-reading-session-v1.json').write_text(
        json.dumps(dict(schema=1, session=token, enabled=True)), encoding='ascii')
    (game / 'script.rpy').write_text('''
define config.name = "Madeira reading fixture"
define config.save_directory = None
label start:
    "First test dialogue."
    "Second test dialogue."
    return
init 999 python hide:
    def verify_madeira_bridge():
        import renpy
        import renpy.exports as api
        import json
        import os
        reporters = [callback for callback in renpy.config.interact_callbacks
                     if getattr(callback, "__name__", "") == "sample"]
        if len(reporters) != 1:
            raise RuntimeError("production bridge did not install in real Ren'Py")
        reporters[0]()
        with open(os.path.join(renpy.config.gamedir, ".madeira-reading-progress-v1.json"), "rb") as stream:
            result = json.loads(stream.read())
        if result["total"] != api.count_dialogue_blocks() or result["total"] < 2:
            raise RuntimeError("dialogue denominator mismatch")
        if result["seen"] != api.count_seen_dialogue_blocks():
            raise RuntimeError("seen dialogue mismatch")
        # End before a window or a real game/save session is created.
        raise renpy.game.QuitException()
    import renpy
    renpy.game.post_init.append(verify_madeira_bridge)
''', encoding='utf-8')
    env = dict(os.environ, RENPY_PATH_TO_SAVES=str(folder / 'saves'),
               RENPY_LOG_TO=str(folder), SDL_AUDIODRIVER='dummy')
    result = subprocess.run([str(options.python.resolve()), str(launcher), str(folder), 'lint'],
                            cwd=folder, env=env, capture_output=True, timeout=40)
    output = game / '.madeira-reading-progress-v1.json'
    if result.returncode or not output.exists():
        print(result.stdout.decode('utf-8', errors='replace'))
        print(result.stderr.decode('utf-8', errors='replace'))
        for name in ('traceback.txt', 'errors.txt', 'log.txt'):
            if (folder / name).exists(): print((folder / name).read_text(encoding='utf-8', errors='replace')[-8000:])
        raise SystemExit('FAIL real engine bridge')
    value = json.loads(output.read_text())
    assert value['session'] == token and value['total'] >= 2
    print('PASS real Ren\'Py engine loaded the production helper and returned actual dialogue counters:', value)
