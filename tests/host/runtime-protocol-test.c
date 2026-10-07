#include <assert.h>
#include <stdio.h>
#include "../../build/session-host/protocol.h"
int main(void) {
    uint8_t data[80] = {0}; struct md_request request;
    memcpy(data, "MDRUN001", 8); md_put64(data + 8, 1); md_put32(data + 16, MD_CLOSE);
    assert(md_request_decode(data, 44, &request));
    assert(request.generation == 1 && request.operation == MD_CLOSE);
    assert(!md_request_decode(data, 43, &request));
    assert(!md_request_decode(data, 45, &request));
    md_put64(data + 8, 0); assert(!md_request_decode(data, 44, &request)); md_put64(data + 8, 1);
    md_put32(data + 16, MD_START);
    size_t size = 44;
    for (unsigned i = 0; i < MD_FIELDS; ++i) {
        md_put32(data + 20 + i * 4, 4);
        if (i != 3 && i != 5) data[size] = 'a';
        size += 4;
    }
    assert(md_request_decode(data, size, &request));
    data[47] = 1; assert(!md_request_decode(data, size, &request)); data[47] = 0;
    md_put32(data + 20, UINT32_MAX); assert(!md_request_decode(data, size, &request));
    uint32_t random = 0x1930u;
    for (unsigned trial = 0; trial < 10000; ++trial) {
        for (unsigned i = 0; i < sizeof(data); ++i) { random = random * 1664525u + 1013904223u; data[i] = (uint8_t)(random >> 24); }
        (void)md_request_decode(data, trial % sizeof(data), &request);
    }
    puts("PASS: production request decoder, length bounds, UTF-16 terminators, malformed commands and 10k bounded inputs");
    return 0;
}
