#ifndef MADEIRA_TOUCH_HIT_H
#define MADEIRA_TOUCH_HIT_H
/* Match the original SwiftUI positions, diameter and eight-way deadzone.
 * No layout clamping, invisible replacement views or editor controls. */
static inline int mtouch_original_toolbar(double width,double x,double y) {
    return x>=width/2-59 && x<width/2+59 && y>=0 && y<68;
}
static inline int mtouch_original_hit(double width,double height,double nx,double ny,double scale,double x,double y) {
    if (!__builtin_isfinite(width) || !__builtin_isfinite(height) || !__builtin_isfinite(nx) ||
        !__builtin_isfinite(ny) || !__builtin_isfinite(scale) || !__builtin_isfinite(x) || !__builtin_isfinite(y) ||
        width<=height || height<=0 || scale<0.5 || scale>3 || nx<0 || nx>1 || ny<0 || ny>1) return 0;
    double dx=x-nx*width,dy=y-ny*height,r=32*scale;
    return dx*dx+dy*dy<=r*r;
}
static inline unsigned mtouch_original_directions(double dx,double dy,double diameter) {
    if (!__builtin_isfinite(dx) || !__builtin_isfinite(dy) || !__builtin_isfinite(diameter) || diameter<=0) return 0;
    double radius=diameter*0.22;
    if (dx*dx+dy*dy<radius*radius) return 0;
    double ax=dx<0 ? -dx : dx,ay=dy<0 ? -dy : dy;
    unsigned mask=0;
    if (ay>=ax*0.414213562373095) mask|=1u<<(dy<0 ? 0 : 2);
    if (ax>=ay*0.414213562373095) mask|=1u<<(dx>0 ? 1 : 3);
    return mask;
}
#endif
