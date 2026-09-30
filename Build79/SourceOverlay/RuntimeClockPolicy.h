#ifndef MADEIRA_RUNTIME_CLOCK_POLICY_H
#define MADEIRA_RUNTIME_CLOCK_POLICY_H
#include <stddef.h>
/* Explicit file > explicit environment > corrected default. Malformed explicit
 * choices fail closed; never mistake "10" or arbitrary text for an opt-in. */
static int mrc_space(unsigned char c) {
    return c==' ' || c=='\t' || c=='\r' || c=='\n';
}
static int mrc_boolean(const unsigned char *data, size_t size) {
    if (!data || !size || size>32) return -1;
    size_t begin=0, end=size;
    while (begin<end && mrc_space(data[begin])) ++begin;
    while (end>begin && mrc_space(data[end-1])) --end;
    return end==begin+1 && (data[begin]=='0' || data[begin]=='1') ? data[begin]-'0' : -1;
}
static int mrc_choice(int file_present, const unsigned char *file, size_t file_size,
                      const unsigned char *environment, size_t environment_size) {
    if (file_present) return mrc_boolean(file,file_size)==1;
    if (environment) return mrc_boolean(environment,environment_size)==1;
    return 1;
}
#endif
