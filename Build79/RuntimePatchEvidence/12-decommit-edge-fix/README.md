# Madeira Decommit v1 test build

The latest fresh BeamNG 0.34 launch deadlocked while its CEF renderer cleared 12 KiB at the beginning of a protected 16 KiB host page. The renderer held Wine's virtual-memory lock; the Mach fault handler then waited for that same lock. The fresh run had no JIT code-pool exhaustion, separating this fault from the earlier pool limit.

The correction prepares both partial host pages before modifying data, checks current and maximum protection, opens ordinary non-executable pages for the clear, verifies zeroed edge bytes, and restores their prior protection. Alias/pool guards and the existing live-module refusal are preserved. Full interior remaps are checked for failure. One caller instruction now propagates failures through Wine's existing error path. The original function bounds, frame, unwind information and every other native byte are preserved.

Validation: 78 source cases with real Linux page protections; 99 exact ARM64 instruction cases using modeled Mach APIs; four caller/status/output cases; an original-binary negative control reproducing the unsafe clear. Independent review found no candidate defect. The physical iPad memory probe and BeamNG retest are still pending.

This is a focused test build, not proof BeamNG works. Real Mach behavior and concurrent device activity are not reproduced by the host fixtures. If the OS refuses permission restoration, the function reports failure but cannot guarantee a complete rollback. General CEF, latest BeamNG address-space, and JIT-pool limits remain separate unresolved concerns. MiSide, fullscreen, Vulkan and controller payloads are unchanged from the preceding build; their latest device behavior has not been retested.
