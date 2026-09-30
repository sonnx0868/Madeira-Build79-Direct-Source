#include "TouchHit.h"
#include "TouchState.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    assert(mtouch_original_hit(844,390,.12,.75,1,844*.12,390*.75));
    assert(mtouch_original_hit(844,390,.125,.75,1,844*.125+32,390*.75));
    assert(!mtouch_original_hit(844,390,.12,.75,1,844*.12+32.01,390*.75));
    assert(!mtouch_original_hit(390,844,.12,.75,1,390*.12,844*.75));
    assert(!mtouch_original_hit(844,390,NAN,.75,1,100,100));
    assert(mtouch_original_toolbar(844,422,50));
    assert(!mtouch_original_toolbar(844,422,69));
    assert(mtouch_original_directions(0,0,64)==0);
    assert(mtouch_original_directions(0,-40,64)==1);
    assert(mtouch_original_directions(40,0,64)==2);
    assert(mtouch_original_directions(0,40,64)==4);
    assert(mtouch_original_directions(-40,0,64)==8);
    assert(mtouch_original_directions(40,-40,64)==3);
    assert(mtouch_original_directions(40,40,64)==6);
    assert(mtouch_original_directions(-40,40,64)==12);
    assert(mtouch_original_directions(-40,-40,64)==9);
    assert(mtouch_original_directions(NAN,40,64)==0);
    // Gas button plus W from original WASD stick. Releasing either one must
    // never release the other's W. Ownership is the raw contact, not UI state.
    TouchState state; touch_state_init(&state,.12f);
    assert(touch_state_begin(&state,1,1,TOUCH_STATE_ROLE_NONE,0,0,0)==TOUCH_STATE_OK);
    assert(touch_state_begin(&state,2,0,TOUCH_STATE_ROLE_NONE,0,0,0)==TOUCH_STATE_OK);
    assert(touch_state_update_buttons(&state,2,mtouch_original_directions(0,-40,64))==TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons==1);
    assert(touch_state_end(&state,1)==TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons==1);
    assert(touch_state_end(&state,2)==TOUCH_STATE_OK);
    assert(touch_state_output(&state).buttons==0);
    assert(touch_state_begin(&state,3,1,TOUCH_STATE_ROLE_NONE,0,0,0)==TOUCH_STATE_OK);
    touch_state_reset(&state); // rotation/editor/background/modal
    assert(touch_state_output(&state).buttons==0);
    assert(touch_state_end(&state,3)==TOUCH_STATE_UNKNOWN); // late UI callback
    puts("PASS: original geometry/eight-way stick; simultaneous holds and reset cleanup");
}
