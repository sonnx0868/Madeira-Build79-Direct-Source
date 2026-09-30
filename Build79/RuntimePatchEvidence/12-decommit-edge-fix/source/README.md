# Partial host-page decommit correction: source and real-protection fixture

The fresh BeamNG renderer stalls in a native leading-edge BZERO on a PROT_NONE16KiB host page. decommit_pages holds virtual_mutex; its Mach exception handler waits for that same lock. The source change prevents the invalid access rather than changing fault dispatch or removing that lock.

`decommit_pages.fragment.c` is the reviewed replacement body plus inline helpers. `virtual_ios.original.c`, `virtual_ios.fixed.c`, `source.patch` and `source-provenance.json` record the exact local pinned-source transformation. Only decommit_pages is replaced; the existing ios_pool_live_overlap helper remains. Source provenance SHA256 is `7ece78a5f9f1e7ce79843dff5297b2994be02a1c4d794cbf0ae4b39a0f47c4ae`.

The helper preflights/prepares at most two partial host pages, including a two-edge range with no complete interior host page. It rejects overlap with an anonymous JIT alias or either JIT pool mapping, validates complete Mach mapping coverage/current/max protections, rejects EXEC and insufficient maximum R/W, temporarily opens non-writable plain pages without COPY or changing maximum rights, and queries actual rights before direct access. Both edges are prepared before the checked full-interior mmap. It zeros and verifies only the requested edge bytes while writable, restores exact original current rights, and queries the restoration. Failure is propagated before logical decommit bookkeeping. Existing RW FEX memory is zeroed again even if the logical COMMITTED bit was already clear.

The existing anonymous-alias path and its live-pool-ledger refusal remain, including the refusal message. Its historical success/logical-decommit behavior on refusal is preserved; this change does not claim that refused aliases were zeroed. Final set_page_vprot_bits and conditional kernel_writewatch_register_range remain. The latter is an empty helper on the pinned Darwin build.

Only heavy dc-census and general late decommit-zero readback/logging within this function are removed. In particular, no late raw read may dereference an edge after restoring PROT_NONE. New exact-edge readback uses volatile loads while R/W is active. The old standalone census helper definitions remain in the source file but are no longer called here.

Run the fixture from any directory:

```sh
python3 <workspace>/work/beamng/decommit-edge-fix/source/test_host.py
```

`host-test-report.json` records exact fragment/fixture/executable hashes, compiler, command and complete output. The optimized fixture passes78 cases:

| Coverage | Cases |
|---|---:|
| Baseline exact leading-edge clear faults on real PROT_NONE |1|
| Nine geometries × NONE/READ/RW × logically committed/uncommitted |54|
| Mixed metadata/neighbor canaries, repeated dirty FEX clear, whole-view size0 |3|
| Neighbor alias, both pool mappings, EXEC, max-read-only, missing mapping |8|
| Initial/opened query/protect failures, silent protection failure, mmap failure |6|
| Second-edge preparation failure restores first edge |1|
| Restore refusal/query failure/silent restore failure |3|
| Alias zeroing and retained live-ledger refusal |2|

The fixture calls the exact source fragment and uses real Linux mprotect/mmap against16KiB-aligned groups of4KiB OS pages. It deliberately faults when the original direct memset tries the observed leading-edge geometry. Positive cases verify all neighboring bytes and requested bytes, logical commit state, and actual read/write accessibility after restore; recommit-equivalent R/W access then confirms zero data. Error cases verify returned failure and absence of logical success. It also injects failures/silent errors into the modeled Mach interface. This is meaningful host memory protection testing, not an iOS Mach ABI/kernel or native ARM64 test.

Physical restore failure cannot be made impossible: if the kernel refuses restoring permissions, the helper returns an error and leaves logical decommit unapplied, but some physical pages may remain writable and some contents may already have been zeroed. It does not promise transaction rollback. Query/re-protection cannot prevent arbitrary external/native remaps that bypass Wine's lock. Temporarily opening a16KiB host page also transiently affects neighboring4KiB permissions; the final protection and bytes are preserved, but this is not complete simultaneous Windows4KiB access isolation. Pages requiring an executable writable-alias solution are rejected instead of changing EXEC or requesting RWX.

The native rewrite must also change the old caller's unconditional-success branch: at0x100134a70, branch to0x100134a98 so `_NtFreeVirtualMemory` consumes the returned status, rather than0x100134aa0 which discards it. The caller then follows its existing lock-release/error path. No native build or device claim is made by the host report.
