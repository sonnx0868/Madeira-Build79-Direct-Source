# LÖVE / ANGLE smoke game

`main.lua` exits after printing the renderer selected by LÖVE. With the
official LÖVE 11.5 Windows x64 package, Madeira's pinned ANGLE DLLs beside the
LÖVE executables, and these variables:

```text
SDL_OPENGL_ES_DRIVER=1
LOVE_GRAPHICS_USE_OPENGLES=1
ANGLE_DEFAULT_PLATFORM=d3d11
```

the verified output is:

```text
ANGLE_SMOKE renderer=OpenGL ES version=OpenGL ES 3.0.0 (ANGLE ...)
device=ANGLE (... Direct3D11 ...)
```

On iOS, that D3D11 device is Madeira's DXMT implementation and presents to
the app's Metal layer.
