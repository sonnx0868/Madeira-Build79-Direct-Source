#include "RuntimeClockPolicy.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(mrc_choice(0,0,0,0,0)==1);
    assert(mrc_choice(0,0,0,(const unsigned char *)"0",1)==0);
    assert(mrc_choice(0,0,0,(const unsigned char *)"1",1)==1);
    assert(mrc_choice(1,(const unsigned char *)"0\n",2,(const unsigned char *)"1",1)==0);
    assert(mrc_choice(1,(const unsigned char *)"1\n",2,(const unsigned char *)"0",1)==1);
    assert(mrc_choice(1,0,0,0,0)==0);
    assert(mrc_boolean((const unsigned char *)" \t1\r\n",5)==1);
    assert(mrc_boolean((const unsigned char *)"\n0 ",3)==0);
    assert(mrc_boolean((const unsigned char *)"10",2)==-1);
    assert(mrc_boolean((const unsigned char *)"true",4)==-1);
    assert(mrc_boolean((const unsigned char *)"1\0",2)==-1);
    assert(mrc_boolean((const unsigned char *)"",0)==-1);
    unsigned char buffer[34]={0};
    for (unsigned n=0;n<=34;n++) {
        assert(mrc_boolean(buffer,n)==-1);
        assert(mrc_choice(1,buffer,n,0,0)==0);
    }
    for (unsigned n=0;n<256;n++) {
        unsigned char value=(unsigned char)n;
        assert(mrc_boolean(&value,1)==(n=='0'?0:n=='1'?1:-1));
    }
    puts("PASS: clock default, override precedence, kill switch, bounds and malformed choices");
}
