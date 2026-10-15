# Epic downloads and Sundered report, 15 October 2026

## Comparison

Inspected Playport `412de5f8c0bdd26d84e0127eb5776ba992e3b152`, specifically
`app/ContentKit/Sources/ContentKit/InstallEngine.swift` and
`app/EpicClient/Sources/EpicClientKit/EpicChunks.swift`. Its installer keeps
16 requests in flight, plans unique chunks across files, scatters each chunk
to its destinations, and retries other asset-provided CDN bases. Its portable
SHA-1 processes full blocks without appending every input byte to an array.
No Playport source was copied into Madeira.

Our preceding Epic installer instead fetched batches of up to four chunks
within one file, waited for the whole batch, and retained only an 8 MB LRU.
Chunks used again in a later file or beyond that cache could be downloaded
again. Every SHA-1 byte passed through a Swift append loop, including chunk
verification, file assembly and resume checks. These are concrete throughput
costs; there is no matched real-network timing for the two apps yet.

The correction uses a global unique-chunk plan and a continuously replenished
task group, up to 16 requests. Width decreases for large chunks, reserving at
most 64 MB of decoded responses including the currently consumed response.
Each verified chunk is written by offset into every unfinished file that uses
it. At most 16 file handles stay open. Each file must pass its complete SHA-1
and synchronize before atomically replacing its previous version. Finished
files remain reusable on resume; unfinished staging is removed on failure or
cancellation. Paths, chunk hashes and file hashes remain checked.

SHA-1 now uses CommonCrypto, already used by Madeira's Steam downloader.
An ephemeral content session reuses connections, allows 16 connections per
host, disables the URL cache and bounds request/resource timeouts. Responses
still spool to temporary files before their size limit is checked. The
manifest's other official CDN bases are retained for bounded retry/failover.
Progress publishes at most every 200 ms plus phase/completion boundaries.

Offline execution checks cover out-of-order completion, a distant reused
chunk beyond the old cache size, cross-file slices, request replenishment,
concurrency bounds, alternate CDN refusal, cancellation, old-file preservation
and Python SHA-1 reference values at padding/block/stream boundaries.
Real account/CDN throughput still needs a device download; fixture speed is
not a prediction of MB/s on the user's network.

## Sundered

Provided WeTransfer file:
`Sundered Eldritch Edition_2026-10-15_15-54-20-322.txt`, 2,335,231 bytes,
build 697, source `25b6acea95025beb2e4eb5fbde7ce83aeb2bfbe4`, iPhone18,2.
Treat the embedded timestamps as the tester's device clock, not this host's
timezone.

- The library reports OpenGL, but runtime loads D3D11/DXGI, selects feature
  level 11_1 and creates the swapchain. The library label alone is not the
  graphics API actually used.
- First present occurs at device time 15:54:26.768. Present-gap/frame counters
  reach 64; no further periodic frame-counter report occurs in the capture.
- A Mono SMC access violation is explicitly handled and execution proceeds
  into D3D11 initialization. It is not evidence of a fatal crash.
- `presented-exit-v1` records status 0 at 15:55:01.956. The file does not say
  whether the tester pressed Quit or the game returned on its own.
- It contains no Unity `output_log.txt`/`Player.log`, managed startup error,
  unhandled exception, or confirmed CDN/content-hash failure. A single-render-
  pass startup frame does not establish shader compilation as the cause.

Requested the selected game's `Sundered_Data/output_log.txt` (or Unity's
LocalLow log). The remote companion collector now probes the exact selected
EXE's `_Data/output_log.txt`, because pre-UnityPlayer.dll games use that
location. It respects the session date window, containment and symlink checks.
No renderer override, game binary change, or claim of a Sundered fix is made
without the missing game-level evidence.
