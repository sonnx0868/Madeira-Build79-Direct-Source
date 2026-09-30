"""Remove the register-clobbering SIGUSR1 return redirect only.

This is a bounded iPad validation candidate, not a general x18 restoration fix.
The existing Mach TEB recovery and CPU/TLS PE corrections remain unchanged.
"""
from pathlib import Path
import difflib
import hashlib
import importlib.util
import json
import struct

H = Path(__file__).resolve().parent
ROOT = H.parents[2]
sha = lambda data: hashlib.sha256(data).hexdigest()
spec = importlib.util.spec_from_file_location('macho_offsets', H.parent/'guest-exit-fix/build.py')
offsets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(offsets)
baseline = H.parent/'apc-signal-fix/Madeira'
data = baseline.read_bytes()
assert sha(data) == '1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b'
start, target = 0x100100520, 0x1001005f4
at = offsets.file_offset(data, start, 4)
old = bytes.fromhex('e05a00b0')
assert data[at:at+4] == old
# Branch after restore_context to the original complete stack epilogue.
assert data[at-4:at] == bytes.fromhex('ca210094')
epilogue = offsets.file_offset(data, target, 24)
assert data[epilogue:epilogue+24] == bytes.fromhex('ff430e91fd7b43a9f44f42a9f65741a9fc6fc4a8c0035fd6')
delta = target-start
assert delta % 4 == 0 and 0 <= delta < 1 << 27
new = struct.pack('<I', 0x14000000 | (delta//4))
fixed = data[:at]+new+data[at+4:]
assert len(data) == len(fixed) and fixed[:at] == data[:at] and fixed[at+4:] == data[at+4:]
(H/'Madeira-original').write_bytes(data)
(H/'Madeira').write_bytes(fixed)

source = (ROOT/'work/ipad-ui/audit/signal_arm64_ios.c').read_bytes()
assert sha(source) == '9543df6cb3c827dd9e5dc67d23917b7d4afee2979ca029c1159a50cc7a275389'
before = source.decode()
s = before.index('static void usr1_handler(')
e = before.index('\nstatic void usr2_handler(', s)
function = before[s:e]
old_source = '''#ifdef WINE_IOS
        ios_fixup_x18_for_return( ucontext );
#endif'''
new_source = '''#ifdef WINE_IOS
        /* An APC can interrupt FEX with a live x17. The x18 trampoline
         * replaces that register with PC and corrupts the resumed code.
         * Preserve restore_context's PC and registers on this path, as
         * the existing Mach recovery path already does. This does not
         * solve every Darwin x18-zero case: indirect copies and silent
         * zero-page reads still require separate runtime work. */
#endif'''
assert function.count(old_source) == 1
after = before[:s]+function.replace(old_source, new_source)+before[e:]
(H/'signal_arm64_ios.original.c').write_bytes(source)
(H/'signal_arm64_ios.fixed.c').write_text(after)
patch = ''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True),
    fromfile='a/build/ntdll-unix/signal_arm64_ios.c', tofile='b/build/ntdll-unix/signal_arm64_ios.c'))
(H/'source.patch').write_text(patch)
report = {
    'baseline_sha256': sha(data), 'candidate_sha256': sha(fixed),
    'source_baseline_sha256': sha(source), 'source_patch_sha256': sha(patch.encode()),
    'patch_address': hex(start), 'branch_target': hex(target), 'file_offset': hex(at),
    'old_bytes': old.hex(), 'new_bytes': new.hex(), 'changed_bytes': sum(a != b for a,b in zip(old,new)),
    'only_one_instruction_changed': True,
    'scope': 'Skip only ios_fixup_x18_for_return after the outside-syscall SIGUSR1 restore_context. APC execution, inside-syscall handling, TEB-less guard, other signal handlers and Mach recovery remain unchanged.',
    'preserves_prior_fixes': True,
    'limitation': 'Removes proven x17 corruption; does not guarantee x18 restoration after Darwin sigreturn. Existing direct-access recovery has silent-copy and unsupported-instruction gaps. Physical IO and CPU/TLS regression tests required before BeamNG.',
    'native_test_pending': True, 'packaged': False, 'installed': False, 'beamng_working': False,
}
(H/'build-report.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
