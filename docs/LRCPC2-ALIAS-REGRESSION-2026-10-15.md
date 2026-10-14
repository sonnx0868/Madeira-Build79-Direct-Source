# Build 680: Mono JIT alias writes and two additional reports

Authenticated, untruncated reports downloaded on 15 October 2026:

| Report | Actual selected game | Finding |
| --- | --- | --- |
| `786a9b45-92e7-4bdd-8127-5ad1c75fcd4c` | Hollow Knight: Silksong | First failing JIT alias write is STLURB `0x190002a6`; Mono subsequently asserts and exits. |
| `ae3385d1-462b-4d3f-a50e-bd8c4d464357` | Hollow Knight Steam | Upload label says Sliver Pines; selected EXE and companion identify Hollow Knight. Same first undecoded STLURB, then exception `0xe0000001`. |
| `03017f15-cf30-42c8-a333-e073dd8f581b` | Silver Pines / Carpenter.exe | Reaches gameplay. FPS windows rise to 56.8 then fall to 27.1 at thermal=0, low-power=0, adaptive cooling disabled, 1920x1080. No observed undecoded STLUR or terminating exception in this capture. |

## Confirmed integration error

Build 680 publishes the actual M2 LRCPC2 feature to the FEX emitter. The
matching Wine native fault-handler consumer had not been imported. The
existing unaligned backpatch support is a different path and cannot handle
a byte store into an RX page through its writable alias. A byte access cannot
be unaligned; patching PC-4 here would corrupt an unrelated instruction.

Silksong's fault has a valid RW alias. The preceding instruction sets W6 to
0x55, but the STLURB never writes that byte. These failures precede the Mono
assertion. No CPU-cache entry was actually reused before that failure.

Imported all five signal-handler hunks verbatim from llamajose/madeira-bcd
commit `ce34c10dd2c59700855ec3b859fd8166e33e9170`: Mach alias stores,
LDAPUR loads, release stores and both subfloor classifier branches.

Added a strong app/native decoder identity handshake and archive receipt.
Cached native bundles without this consumer now fail the compatibility gate
and link instead of silently enabling an unsupported producer. The refresh
build replaces only signal_arm64.o and hashes every other archive member to
verify preservation. FEX and DXMT are not rebuilt by this signal-only refresh.

## Verification and remaining performance investigation

The host fixture assembles 234 real AArch64 RCPC2 forms with clang and runs
the production decoder bodies: all access widths, signed loads, immediate
extremes, SP bases, zero-register operands, unaligned destinations, guard
bytes, invalid forms and the exact failing 0x190002a6 write. It also executes
the app's host-probe body with valid/missing/old native consumers.

These checks reproduce and repair the instruction-emulation failure. They
do not establish that every game reaches gameplay on an iPad, or resolve the
separate gradual FPS decline. Bounded low-FPS CPU/thread sampling and merged
GPU command-buffer interval coverage now cover the reported Hollow Knight,
Silksong, Silver Pines, Primal Planet and REPLACED investigations.
