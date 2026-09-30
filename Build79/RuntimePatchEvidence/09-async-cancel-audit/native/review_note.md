# Independent async cancellation review

Reviewed the source correction, full ARM64 replacement, pinned original request disassembly, and the existing source/native harnesses. No new implementation defect was found in the proposed correction. This is a host review; it does not establish physical iPad or BeamNG success.

The necessary changes agree across source and assembly: retain each selected async through restoration, attach completion-group membership before entering cancellation callbacks, hold a construction sentinel while building the group, and read a fresh tracked-list head after each potentially destructive release. The callback can synchronously complete and release the queue-owned async reference. Merely retaining the async without moving group attachment would leave a completed async attached to a group, which the original negative control correctly rejects.

## Additional verification

- `review_source_tests.py`: **103 cases passed with ASan and UBSan** using the unchanged production function bodies already extracted into `build/fixed.c`. Cases include a completion callback removing a future process-list member, a callback adding a new operation, already-canceled work receiving its first completion group without duplicate FD cancellation, explicit direct-result completion, non-FIFO real APC-destructor completion, and closing the wait handle before operations finish. Every fixture-owned allocation was released. LeakSanitizer remains disabled because of the host sandbox; explicit allocation accounting is retained.
- `review_native_helpers.py`: **32 cases passed in Unicorn** using the complete replacement handler and the actual pinned native `get_fd_user` and `grab_object` instructions. The old mock FD field at offset zero is poisoned; success requires the native FD user pointer at `+0x78`. Cases include synchronous and deferred completion, filtering, cross-operation completion, and allocation/handle failures.
- The original native `_async_set_result` disassembly independently confirms `async_cancel` at async `+0x100`, group count at `+0x50`, and group sync at `+0x48`. Original `_async_cancel_destroy` releases the sync pointer at `+0x48`. `_grab_object` increments the 32-bit reference count at object offset zero. Symbol addresses match the native build's pinned helper map.

The extra native checks tested SHA-256 `f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984`. Reports are `review_source_report.json` and `review_native_helpers_report.json`. The source fixture and native helper models share some supporting fixture code with the root tests, so these should not be described as an entirely independent implementation of every subsystem.

## Preserved limitation

The pre-existing assertion on a second completion group is unchanged. `device_file_cancel_async` can send an IRP cancellation request without immediately setting `async->terminated`; if such an operation already has `async_cancel` attached, another grouped cancellation selects it and reaches the existing `assert(!async->async_cancel)`. The original source and replacement both retain this behavior. This is separate from the observed CEF cleanup null-list write and was not introduced by the correction. The new first-group test must not be interpreted as proving repeated overlapping groups are supported.

No device actions or implementation edits were made by this review. Reproduction and physical validation remain required after installation, particularly concurrent CEF teardown and whether the original UI failure persists after cleanup survives.
