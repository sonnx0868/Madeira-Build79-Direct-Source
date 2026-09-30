"""Package the independently tested four-byte SRW guard without other payload edits."""
from pathlib import Path
import hashlib
import json
import zipfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BASE_SHA = '27992c17871d886e0f3f16f2c86d4252a5c0cd59a5147f543c6793af3e94fd7f'
OLD_NATIVE_SHA = '6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8'
NEW_NATIVE_SHA = '91f4bc3acb90ed4dafa7fbf7cea5087f8c32ae9044990a6f785b116567e43d90'
sha = lambda b: hashlib.sha256(b).hexdigest()
base = ROOT / 'outputs/ipad-installers/Madeira-iPad-Decommit-v1-Test.ipa'
target = ROOT / 'outputs/ipad-installers/Madeira-iPad-SRW-Guard-v1-Test.ipa'
member = 'Payload/Madeira.app/Madeira'
assert sha(base.read_bytes()) == BASE_SHA
fixed = (HERE / 'Madeira').read_bytes()
assert sha(fixed) == NEW_NATIVE_SHA
reports = {}
for name in ('host-test-report.json', 'native-test-report.json'):
    report = json.loads((HERE / name).read_text())
    assert report.get('passed') is True or report.get('result') == 'PASS' or report.get('status') == 'PASS', name
    assert report['candidate_sha256'] == NEW_NATIVE_SHA, name
    reports[name] = {'sha256': sha((HERE / name).read_bytes()), 'passed': True}
with zipfile.ZipFile(base) as archive:
    original = archive.read(member)
    assert sha(original) == OLD_NATIVE_SHA
    assert len(original) == len(fixed)
    assert [i for i,(a,b) in enumerate(zip(original, fixed)) if a != b] == list(range(0x120728, 0x12072c))
    assert original[0x120728:0x12072c] == bytes.fromhex('d6fcff97')
    assert fixed[0x120728:0x12072c] == bytes.fromhex('1f2003d5')
    assert len(archive.namelist()) == len(set(archive.namelist()))
    with zipfile.ZipFile(target, 'w') as output:
        for info in archive.infolist():
            output.writestr(info, fixed if info.filename == member else archive.read(info))
with zipfile.ZipFile(base) as original, zipfile.ZipFile(target) as output:
    assert original.namelist() == output.namelist()
    changes = [name for name in original.namelist() if original.read(name) != output.read(name)]
    assert changes == [member]
report = {
    'package': str(target), 'sha256': sha(target.read_bytes()),
    'baseline_ipa_sha256': BASE_SHA, 'native_sha256': NEW_NATIVE_SHA,
    'changed_members': changes, 'all_other_payload_members_identical': True,
    'native_change_bytes': 4,
    'change': 'Stop the anonymous no-stamp SRW heuristic from clearing a potentially live lock; retain diagnostics and separate explicit dead-owner recovery.',
    'tests': reports, 'installed': False, 'physical_lock_fix_validated': False,
    'game_and_settings_files_changed': False,
    'limitation': 'Local tests validate the targeted corruption prevention. Physical high-graphics gameplay still requires validation.',
}
(HERE / 'package-report.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
