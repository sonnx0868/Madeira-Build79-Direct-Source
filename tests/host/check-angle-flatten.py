#!/usr/bin/env python3
"""ANGLE HLSL must fall back to a real branch when [flatten] is impossible."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
source = (root / "wine/libs/vkd3d/libs/vkd3d-shader/hlsl_codegen.c").read_text(encoding="utf-8")
build = (root / "build/wine-pe/build-d3dcompiler.sh").read_text(encoding="utf-8")
docs = (root / "docs/OPENGL.md").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"angle-flatten contract failed: {message}")


predicate = source[source.index("static bool can_flatten_conditional_block"):source.index("static bool lower_conditional_block_stores")]
flatten = source[source.index("static bool flatten_conditional_branches"):source.index("static bool normalize_switch_cases")]

require("Flattening conditional blocks with non-discard jump instructions" not in predicate,
        "an unsupported flatten must not mark a valid shader unimplemented")
require("Conditional branches with side effects cannot be flattened" not in predicate,
        "the pure capability predicate must not poison compiler status")
require("iff->flatten_type = HLSL_IF_FORCE_BRANCH" in flatten,
        "a forced flatten that cannot be lowered must become a semantic branch")
require("return true;" in flatten[flatten.index("iff->flatten_type = HLSL_IF_FORCE_BRANCH"):],
        "demotion must count as progress so the fixed-point pass settles")
require("libs/vkd3d" in build and "dlls/d3dcompiler_47" in build and "dlls/wined3d" in build,
        "every consumer must relink against the changed vkd3d-shader archive")
require("arm64ec-windows/$module.dll" in build and
        "Flattening conditional blocks with non-discard jump instructions" in build,
        "the staging script must copy both DLLs and reject a stale E5017 binary")
require("E5017" in docs and "OpenGL ES -> ANGLE D3D11 -> DXMT -> Metal" in docs,
        "the route, symptom and rebuild must be documented")

print("angle-flatten contract: ok")
