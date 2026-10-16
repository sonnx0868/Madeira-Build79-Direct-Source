/* SPDX-License-Identifier: MIT */
#pragma once

// Expose hookable COM entries as x64 fast-forward sequences. The compiler
// retains the ARM64EC body and supplies the matching native dispatch thunk.
// An x64 caller must not receive the raw ARM64 body as its patch target.
#if defined(DXMT_IOS) && defined(__arm64ec__)
# if !__has_attribute(hybrid_patchable)
#  error ARM64EC swapchain interoperability requires hybrid_patchable support
# endif
# define MADEIRA_SWAPCHAIN_ENTRY __attribute__((hybrid_patchable))
#else
# define MADEIRA_SWAPCHAIN_ENTRY
#endif
