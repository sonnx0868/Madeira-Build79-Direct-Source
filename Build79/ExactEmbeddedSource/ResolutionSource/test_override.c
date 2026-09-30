#include "OverrideMerge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    char out[1024];
    assert(mc_merge_overrides(NULL, out, sizeof(out)));
    assert(!strcmp(out, "xinput1_3,xinput1_4,xinput9_1_0=n,b"));
    assert(mc_merge_overrides("d3d11=n,b;xinput1_3=b;winhttp=n", out, sizeof(out)));
    assert(!strcmp(out, "d3d11=n,b;winhttp=n;xinput1_3,xinput1_4,xinput9_1_0=n,b"));
    assert(mc_merge_overrides("*XINPUT1_4.dll, dinput8 =b;foo=;*xinput9_1_0=n", out, sizeof(out)));
    assert(!strcmp(out, "dinput8=b;foo=;xinput1_3,xinput1_4,xinput9_1_0=n,b"));
    assert(mc_merge_overrides("notxinput1_3=n;unknown-malformed", out, sizeof(out)));
    assert(!strcmp(out, "notxinput1_3=n;unknown-malformed;xinput1_3,xinput1_4,xinput9_1_0=n,b"));
    assert(!mc_merge_overrides("d3d11=n,b", out, 10));
    puts("Override merge preserves unrelated/grouped rules, handles names/case/.dll and rejects insufficient space.");
}
