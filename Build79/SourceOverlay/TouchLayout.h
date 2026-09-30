#ifndef MADEIRA_TOUCH_LAYOUT_H
#define MADEIRA_TOUCH_LAYOUT_H
typedef struct { double x, y, diameter; int valid; } MTouchLayout;
static inline MTouchLayout mtouch_layout(double width, double height, double nx,
                                        double ny, double scale, int editing) {
    MTouchLayout result={0};
    if (!__builtin_isfinite(width) || !__builtin_isfinite(height) ||
        !__builtin_isfinite(nx) || !__builtin_isfinite(ny) || !__builtin_isfinite(scale) ||
        width<160 || height<188 || scale<=0) return result;
    double top=editing ? 184 : 120;
    double available=height-top-24;
    if (available<44) return result;
    double diameter=64*scale;
    if (diameter<44) diameter=44;
    if (diameter>available) diameter=available;
    if (diameter>width-24) diameter=width-24;
    double radius=diameter/2, x=nx*width, y=ny*height;
    if (x<radius+12) x=radius+12;
    if (x>width-radius-12) x=width-radius-12;
    if (y<top+radius) y=top+radius;
    if (y>height-radius-12) y=height-radius-12;
    return (MTouchLayout){x,y,diameter,1};
}
#endif
