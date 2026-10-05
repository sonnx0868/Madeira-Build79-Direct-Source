# OpenGL through ANGLE

64-bit OpenGL games such as LÖVE titles use Madeira's bundled ANGLE route:

`OpenGL ES -> ANGLE D3D11 -> DXMT -> Metal`.

ANGLE generates HLSL at run time and compiles it through Wine's
`d3dcompiler_47.dll`, backed by the in-tree vkd3d-shader compiler. Some dynamic
pixel shaders contain an HLSL `[flatten]` hint around control flow with early
returns. Microsoft's compiler retains a branch when that hint cannot be
lowered. The previous vkd3d path instead emitted E5017, rejected the pixel
shader and left the game presenting valid but mostly black frames.

Madeira keeps the shader semantics: if a forced flatten is impossible, the
compiler demotes that one conditional to `HLSL_IF_FORCE_BRANCH` and continues.
Side effects and jumps stay inside the original branch; successful flattening
is unchanged. Rebuild and stage the affected ARM64EC compiler with:

```sh
bash build/wine-pe/build-d3dcompiler.sh
```

The device log should stop repeating `E5017: Flattening conditional blocks
with non-discard jump instructions`; normal frame presentation alone is not a
success criterion because the broken path already presented black frames.
