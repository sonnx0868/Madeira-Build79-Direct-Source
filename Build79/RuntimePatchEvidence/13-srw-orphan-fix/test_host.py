"""Extract pinned functions; compile and run deterministic adversarial schedules."""
from pathlib import Path
import hashlib, json, os, subprocess
H = Path(__file__).resolve().parent
sha = lambda data: hashlib.sha256(data).hexdigest()

def extract(text, signature):
    start = text.index(signature)
    at = text.index('{', start)
    depth = 1
    end = at + 1
    # These five specific function bodies have balanced braces in their
    # comments/strings too; pinning source hashes makes this parser bounded.
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end] + '\n'

def main():
    manifest = json.loads((H/'build-report.json').read_text())
    original = (H/'sync.original.c').read_bytes()
    fixed = (H/'sync.fixed.c').read_bytes()
    assert sha(original) == manifest['source_sha256']
    assert sha(fixed) == manifest['fixed_source_sha256']
    candidate = (H/'Madeira').read_bytes()
    assert sha(candidate) == manifest['candidate_sha256']
    pe_path = H.parents[1]/'vulkan/build-audit/wine-source/dlls/ntdll/sync.c'
    pe_bytes = pe_path.read_bytes()
    assert sha(pe_bytes) == '78217755402c8d6dadbcff30eae02cca27ecbebd4dbbab88e5372c5df73b07d2'
    pe = pe_bytes.decode()
    shared = ''.join(extract(pe, s) for s in [
        'void WINAPI RtlReleaseSRWLockExclusive(',
        'void WINAPI RtlReleaseSRWLockShared(',
        'BOOLEAN WINAPI RtlTryAcquireSRWLockExclusive(',
    ])
    results = []
    template = (H/'host_fixture.c.in').read_text()
    for name, text in [('baseline', original.decode()), ('candidate', fixed.decode())]:
        functions = ''.join(extract(text, s) for s in [
            'void ios_srw_reap_exclusive(', 'void ios_orphan_check(']) + shared
        c = template.replace('/*EXTRACTED_PINNED_FUNCTIONS*/', functions)
        assert c != template
        source = H/f'host-{name}.c'
        binary = H/f'host-{name}'
        source.write_text(c)
        command = ['clang','-std=c11','-O1','-g','-Wall','-Wextra',
                   '-Wno-unused-function','-fsanitize=address,undefined','-fno-omit-frame-pointer']
        if name == 'baseline': command += ['-DBASELINE']
        command += [str(source), '-o', str(binary)]
        subprocess.run(command, check=True, capture_output=True, text=True)
        env = os.environ.copy()
        # The sandbox uses ptrace; LeakSanitizer cannot enumerate threads here.
        # Address/undefined behavior instrumentation remains enabled.
        env['ASAN_OPTIONS'] = 'detect_leaks=0'
        proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20, env=env)
        (H/f'host-{name}.log').write_text(proc.stdout + proc.stderr)
        assert proc.returncode == 0, proc.stdout + proc.stderr
        assert proc.stdout.count('\nPASS ') == 10 and 'PASS cases=10' in proc.stdout
        results.append({'variant':name,'passed':True,'cases':10,
                        'source_sha256':sha(c.encode()), 'binary_sha256':sha(binary.read_bytes()),
                        'exit_code':proc.returncode,'log':str(H/f'host-{name}.log')})
    report = {
        'passed':True, 'result':'PASS', 'cases':10, 'executions':20,
        'baseline_sha256':manifest['baseline_sha256'],
        'candidate_sha256':sha(candidate), 'source_sha256':sha(original),
        'fixed_source_sha256':sha(fixed), 'pe_source_sha256':sha(pe_bytes),
        'sanitizers':'ASan + UBSan, both variants clean; LeakSanitizer disabled because sandbox ptrace prevents its thread enumeration', 'variants':results,
        'coverage':'Actual extracted orphan/reaper/PE release/try-acquire code, real mmap/msync and atomic CAS; one-census false live-owner reap negative controls; preserved owner and ordinary release/wake; correct stamp/fewer waiters/non-target shape; exact observed underflow.',
        'limits':'Deterministic adversarial interleavings, no OS-scheduled contention. Wake calls are instrumented. This does not test Darwin Mach delivery, native ARM64 execution, existing explicit-dead-owner policy, or actual game stability.',
        'physical_fix_proven':False,
    }
    (H/'host-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__ == '__main__':
    main()
