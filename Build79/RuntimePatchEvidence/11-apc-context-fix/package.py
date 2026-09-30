"""Package the exact tested native candidate; retain every other IPA member."""
from pathlib import Path
import hashlib
import json
import zipfile

H = Path(__file__).resolve().parent
ROOT = H.parents[2]
sha = lambda data: hashlib.sha256(data).hexdigest()
base = ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Signal-v1-Test.ipa'
assert sha(base.read_bytes()) == '36867a7987309889d0c68e64a7f4abcebc64906cd1c02f35a1bfe5aa3f2bf542'
fixed = (H/'Madeira').read_bytes()
assert sha(fixed) == 'f3269d77d5e9872c0f3506570de11b9a4b707b5753d356176e1ba2691417c1f4'
test = json.loads((H/'usr1-native-test-report.json').read_text())
assert test.get('passed') or test.get('result') == 'PASS'
assert test.get('candidate_sha256', test.get('synthetic_candidate_sha256')) == sha(fixed)
target = ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Context-v1-Test.ipa'
member = 'Payload/Madeira.app/Madeira'
with zipfile.ZipFile(base) as old, zipfile.ZipFile(target, 'w') as new:
    assert len(old.namelist()) == len(set(old.namelist()))
    assert sha(old.read(member)) == '1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b'
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
    'change': 'One ARM64 instruction: skip SIGUSR1 live-x17-clobbering return helper.',
    'preserves_prior_runtime_ui_vulkan_controller_fixes': True,
    'limitation': 'Bounded validation candidate. Existing x18-zero recovery limitations remain; real iPad I/O and CPU/TLS tests precede BeamNG.',
    'installed': False, 'device_validated': False, 'beamng_working': False,
}
(H/'package-report.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
