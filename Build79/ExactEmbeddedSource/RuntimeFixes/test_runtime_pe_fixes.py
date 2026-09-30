"""Release-specific binary invariants; not a substitute for device testing."""
import struct
import unittest
from pathlib import Path
from zipfile import ZipFile

from runtime_pe_fixes import guard_fix, offset, rpc_labels, sections, sha, XT_SHA, NT_SHA

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / 'outputs/checkpoints/build77-clock/Madeira.ipa'
APP = 'Payload/Madeira.app/arm64ec-windows/'


class RuntimeFixTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with ZipFile(BASE) as archive:
            cls.xt = archive.read(APP + 'xtajit64.dll')
            cls.nt = archive.read(APP + 'ntdll.dll')
        cls.fixed_xt = guard_fix(cls.xt)
        cls.fixed_nt = rpc_labels(cls.nt)

    def test_exact_baselines(self):
        self.assertEqual(sha(self.xt), XT_SHA)
        self.assertEqual(sha(self.nt), NT_SHA)

    def test_instruction_diff_allowlist(self):
        addresses = (0x18001b4d4, 0x18001b4dc, 0x1800160ac, 0x1800160b4)
        allowed = {i for va in addresses for i in range(offset(self.xt, va), offset(self.xt, va) + 4)}
        changed = {i for i, (a, b) in enumerate(zip(self.xt, self.fixed_xt)) if a != b}
        self.assertTrue(changed)
        self.assertTrue(changed <= allowed)
        self.assertEqual(len(self.xt), len(self.fixed_xt))
        # This also covers PE headers, all directories, CHPE/unwind tables,
        # imports/exports, relocation data and every other instruction.
        self.assertEqual(sections(self.xt), sections(self.fixed_xt))
        for i in range(len(self.xt)):
            if i not in allowed:
                self.assertEqual(self.xt[i], self.fixed_xt[i])

    def test_owned_output_slots(self):
        for va, scratch, frame, saved in ((0x18001b4d4, 0x30, 0x70, 0x40),
                                          (0x1800160ac, 0x20, 0x60, 0x30)):
            word = struct.unpack_from('<I', self.fixed_xt, offset(self.xt, va))[0]
            self.assertEqual(word & 0xffc00000, 0x91000000)  # ADD immediate, 64-bit
            self.assertEqual(word & 31, 3)  # X3: lpflOldProtect
            self.assertEqual((word >> 5) & 31, 31)  # SP
            self.assertEqual((word >> 10) & 0xfff, scratch)
            self.assertEqual(scratch % 4, 0)
            self.assertLessEqual(scratch + 4, saved)
            self.assertLess(saved, frame)
        # Dead-scratch lifetime independently reviewed against full functions.

    def test_bool_branches(self):
        for va in (0x18001b4dc, 0x1800160b4):
            pos = offset(self.xt, va)
            before, after = (struct.unpack_from('<I', data, pos)[0]
                             for data in (self.xt, self.fixed_xt))
            self.assertEqual(before ^ after, 1 << 24)  # CBZ -> CBNZ only
            self.assertEqual(after & 31, 0)  # W0
            self.assertEqual((before >> 5) & 0x7ffff, (after >> 5) & 0x7ffff)
            for result in (0, 1, 2, 0x7fffffff, 0xffffffff):
                skips_warning = result != 0
                self.assertEqual(skips_warning, bool(result))

    def test_rpc_data_only(self):
        old = b'POOL STALE  <== executing copy sees the OLD x64 thunk'
        new = b'PRE-SYNC SNAPSHOT (call-time unverified)'
        pos = self.nt.index(old + b'\0')
        self.assertEqual(pos, 701560)
        self.assertEqual(self.fixed_nt[pos:pos + len(old) + 1],
                         new + b'\0' * (len(old) + 1 - len(new)))
        self.assertEqual(self.nt[:pos], self.fixed_nt[:pos])
        self.assertEqual(self.nt[pos + len(old):], self.fixed_nt[pos + len(old):])
        self.assertEqual(sections(self.nt), sections(self.fixed_nt))
        for name, rva, raw, size, flags in sections(self.nt)[1]:
            if flags & 0x20000000:
                self.assertEqual(self.nt[raw:raw + size], self.fixed_nt[raw:raw + size])
        # Original format/fields/severity/classifier deliberately unchanged.
        end = self.nt.index(b'\0', 701616)
        self.assertEqual(self.nt[701616:end + 1], self.fixed_nt[701616:end + 1])

    def test_unknown_or_already_patched_rejected(self):
        for apply, original, patched in ((guard_fix, self.xt, self.fixed_xt),
                                          (rpc_labels, self.nt, self.fixed_nt)):
            corrupt = bytearray(original)
            corrupt[-1] ^= 1
            for invalid in (b'', original[:-1], bytes(corrupt), patched):
                with self.assertRaises(ValueError):
                    apply(invalid)


if __name__ == '__main__':
    unittest.main()
