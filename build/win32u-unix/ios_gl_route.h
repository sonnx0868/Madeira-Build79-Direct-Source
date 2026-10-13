/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef MADEIRA_IOS_GL_ROUTE_H
#define MADEIRA_IOS_GL_ROUTE_H

#include <string.h>

/* SDL/ANGLE still asks Wine for window pixel formats. Ren'Py's angle2
 * renderer uses bundled EGL -> D3D11 -> Metal; that WGL query must not
 * initialize the unrelated desktop Mesa/Zink backend. Keep working native
 * GLES pixel formats, and leave explicit desktop GL choices intact. */
static inline int ios_gl_renpy_angle_probe( const char *backend, const char *mode, const char *renderer )
{
    return (!backend || !strcmp( backend, "auto" ) || !strcmp( backend, "angle" )) &&
           mode && !strcmp( mode, "renpy" ) &&
           renderer && (!strcmp( renderer, "angle2" ) || !strcmp( renderer, "angle" ));
}

#endif
