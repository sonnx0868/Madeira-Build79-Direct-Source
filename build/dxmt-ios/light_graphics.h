#ifndef MADEIRA_LIGHT_GRAPHICS_H
#define MADEIRA_LIGHT_GRAPHICS_H

/* Keep image dimensions, texture mip levels, LOD and comparison/shadow
 * semantics. Only trim the most expensive ordinary anisotropic filters. */
static inline unsigned madeira_light_anisotropy(unsigned requested, int light, int comparison) {
    return light && !comparison && requested > 8 ? 8 : requested;
}

#endif
