// Original row label, but Unicode text presentation rather than pale emoji.
// Same public SwiftUI initializers and unchanged stack arguments/gesture.
.text
.globl _MadeiraKeyboardTextArguments
_MadeiraKeyboardTextArguments:
    mov x0, #0x8ce2
    movk x0, #0xefa8, lsl #16
    movk x0, #0x8eb8, lsl #32
    mov x1, #0xa600000000000000
    // Packaging supplies the existing BL relocated for its new position.
    nop
    mov w8, #0x100
    stp xzr, x8, [sp, #-16]!
    and w2, w2, #1
    nop
