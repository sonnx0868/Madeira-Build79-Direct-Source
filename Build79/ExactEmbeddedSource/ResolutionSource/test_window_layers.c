#include "WindowLayers.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
int main(void) {
    MadeiraWindowLayers levels;
    assert(MadeiraWindowLayersForGame(0, &levels));
    assert(levels.pad == 100 && levels.controls == 101 && levels.toolbar == 110);
    for (int i = 0; i < 100; ++i) {
        assert(MadeiraWindowLayersForGame(150, &levels));
        assert(levels.pad == 151 && levels.controls == 152 && levels.toolbar == 153);
    }
    assert(MadeiraWindowLayersForGame(0, &levels));
    assert(levels.toolbar == 110);
    assert(!MadeiraWindowLayersForGame(NAN, &levels));
    assert(!MadeiraWindowLayersForGame(INFINITY, &levels));
    assert(!MadeiraWindowLayersForGame(0, NULL));
    puts("Window levels: original ordering, raised game, no drift, restoration, invalid inputs passed");
}
