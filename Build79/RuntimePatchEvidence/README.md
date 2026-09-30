# Runtime compatibility patch set

These directories preserve the source changes, hash-guarded binary reproducers,
and tests used between the original Madeira build and the physically verified
BeamNG.drive 0.34 SRW-Guard baseline. They are ordered chronologically by the
directory prefix.

The main areas are Windows static TLS and CPUID metadata, translated module TLS,
guest-process cleanup, async cancellation and APC delivery, reserved-memory
decommit handling, and the final SRW orphan guard. The release builder pins the
resulting main executable to SHA-256
`91f4bc3acb90ed4dafa7fbf7cea5087f8c32ae9044990a6f785b116567e43d90`.

The reproducers intentionally fail when their expected binary or instruction
context differs. A source rebuild should apply the corresponding `.patch` files
to the pinned Madeira/FEX/Wine trees rather than treating raw offsets as portable.
