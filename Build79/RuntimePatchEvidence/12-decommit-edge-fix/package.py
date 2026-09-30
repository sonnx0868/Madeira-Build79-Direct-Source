"""Package the exact tested native candidate; retain every other IPA member."""
from pathlib import Path
import hashlib
import json
import zipfile

H = Path(__file__).resolve().parent
ROOT = H.parents[2]
sha = lambda data: hashlib.sha256(data).hexdigest()
base = ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Context-v1-Test.ipa'
assert sha(base.read_bytes()) == '09c4d9979d532e7a2cf464b150a05799682ddebe57855d3f7ae8906335882214'
fixed = (H/'native/Madeira').read_bytes()
assert sha(fixed) == '6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8'
test = json.loads((H/'native/native-test-report.json').read_text())
assert test.get('passed') or test.get('result') == 'PASS'
assert test['native_sha256'] == sha(fixed)
target = ROOT/'outputs/ipad-installers/Madeira-iPad-Decommit-v1-Test.ipa'
member = 'Payload/Madeira.app/Madeira'
with zipfile.ZipFile(base) as old, zipfile.ZipFile(target, 'w') as new:
    assert len(old.namelist()) == len(set(old.namelist()))
    assert sha(old.read(member)) == 'f3269d77d5e9872c0f3506570de11b9a4b707b5753d356176e1ba2691417c1f4'
    for info in old.infolist():
        new.writestr(info, fixed if info.filename == member else old.read(info))
with zipfile.ZipFile(base) as old, zipfile.ZipFile(target) as new:
    assert old.namelist() == new.namelist()
    changed = [n for n in old.namelist() if old.read(n) != new.read(n)]
    assert changed == [member]
report = {
    'package': str(target), 'sha256': sha(target.read_bytes()),
    'native_sha256': sha(fixed), 'baseline_ipa_sha256': sha(base.read_bytes()),
    'changed_members': changed, 'all_other_payload_members_identical': True,
    'change': 'Guarded partial-host-page decommit with exact protection restoration; caller propagates failures.',
    'preserves_prior_runtime_ui_vulkan_controller_fixes': True,
    'limitation': 'Source and native fixtures pass; Mach VM and BeamNG still require physical validation. OS restoration failure returns an error but cannot guarantee physical rollback.',
    'installed': False, 'device_validated': False, 'beamng_working': False,
}
(H/'package-report.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
