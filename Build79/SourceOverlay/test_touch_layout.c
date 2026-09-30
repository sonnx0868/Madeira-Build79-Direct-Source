#include "TouchLayout.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    double sizes[][2]={{956,440},{440,956},{1376,1032},{1032,1376},{320,252},{160,188}};
    for (unsigned s=0;s<sizeof(sizes)/sizeof(sizes[0]);s++) for (int edit=0;edit<2;edit++)
        for (int x=-2;x<=12;x++) for (int y=-2;y<=12;y++) for (int z=5;z<=30;z++) {
            MTouchLayout r=mtouch_layout(sizes[s][0],sizes[s][1],x/10.0,y/10.0,z/10.0,edit);
            if (edit && sizes[s][1]<252) { assert(!r.valid); continue; }
            assert(r.valid && r.diameter>=44);
            assert(r.x-r.diameter/2>=12-1e-8 && r.x+r.diameter/2<=sizes[s][0]-12+1e-8);
            assert(r.y-r.diameter/2>=(edit ? 184 : 120)-1e-8);
            assert(r.y+r.diameter/2<=sizes[s][1]-12+1e-8);
        }
    assert(!mtouch_layout(NAN,440,.5,.5,1,0).valid);
    assert(!mtouch_layout(956,INFINITY,.5,.5,1,0).valid);
    assert(!mtouch_layout(956,440,NAN,.5,1,0).valid);
    assert(!mtouch_layout(956,440,.5,.5,0,0).valid);
    puts("PASS: native touch/edit layout stays inside edges and below toolbar");
}
