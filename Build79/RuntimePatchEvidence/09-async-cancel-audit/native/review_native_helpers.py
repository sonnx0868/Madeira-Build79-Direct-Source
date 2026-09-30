#!/usr/bin/env python3
"""Repeat lifecycle checks with two actual native helper bodies, not their mocks."""
from pathlib import Path
import hashlib, importlib.util, json

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('review_harness', HERE / 'test_native.py')
h = importlib.util.module_from_spec(spec)
spec.loader.exec_module(h)

class RealHelpers(h.Run):
    def __init__(self, variant, case):
        super().__init__(variant, case)
        baseline = (HERE / 'Madeira-original').read_bytes()
        for name, length in [('get_fd_user', 8), ('grab_object', 40)]:
            address = h.b.SYMS[name]
            offset = h.b.file_offset(baseline, address, length)
            self.u.mem_write(address, baseline[offset:offset + length])
        # Native get_fd_user loads fd->user from +0x78. This fixture field is
        # deliberately populated separately from the original mock's +0 field.
        for a in self.ops:
            d = self.objects[a]['desc']
            self.w64(a + 0x200 + 0x78,
                     h.TARGET + (0x10000 if d.get('other_object') else 0))
            self.w64(a + 0x200, 0xeeeeeeee)  # wrong offset must fail, not agree

    def hook(self, u, address, size, name):
        assert u.reg_read(h.ar.UC_ARM64_REG_SP) % 16 == 0
        if name in ('get_fd_user', 'grab_object'):
            return  # Execute the pinned original native instructions.
        return super().hook(u, address, size, name)

def main():
    cases = []
    for only in (False, True):
        for n in (0, 1, 2, 8, 24):
            for mode in ('sync', 'deferred'):
                cases.append({'name': f'real-helpers-{only}-{n}-{mode}',
                              'only': only, 'ops': [{'mode': mode} for _ in range(n)]})
        cases.append({'name': f'real-helpers-filters-{only}', 'only': only,
                      'iosb': 7, 'ops': [{}, {'other_object': True},
                      {'other_thread': True}, {'terminated': True}, {'system': True},
                      {'iosb': 99}, {'canceled': True}, {'mode': 'deferred'}]})
        for key in ('cross_previous', 'restore_cross', 'cross_future'):
            cases.append({'name': f'real-helpers-{only}-{key}', 'only': only,
                          key: True, 'ops': [{'mode': 'deferred' if key == 'cross_previous' else 'sync'},
                          {'mode': 'deferred'}, {'mode': 'sync'}]})
    for key in ('invalid_handle', 'alloc_fail', 'sync_fail', 'handle_fail'):
        cases.append({'name': 'real-helpers-' + key, 'only': True, key: True,
                      'ops': [{'mode': 'deferred'}, {}]})
    results = [RealHelpers('Madeira', case).run() for case in cases]
    report = {'passed': True, 'native_cases': len(results), 'cases': results,
              'native_sha256': hashlib.sha256((HERE / 'Madeira').read_bytes()).hexdigest(),
              'scope': 'The complete patched ARM64 request handler plus the original native get_fd_user and grab_object instructions run in Unicorn. The FD mock field at +0 is poisoned; the native +0x78 field is checked by actual instructions. Remaining lifecycle helpers are instrumented models. No physical device execution.'}
    (HERE / 'review_native_helpers_report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'cases'}, indent=2))

if __name__ == '__main__':
    main()
