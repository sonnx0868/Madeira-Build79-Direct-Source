#ifndef MADEIRA_RUNTIME_CABINET_H
#define MADEIRA_RUNTIME_CABINET_H
#include <stdint.h>
/* Outputs use opaque names in an existing, private staging directory. CAB
 * member names are metadata only and never become filesystem paths. */
typedef int (*madeira_cab_member)(void *context, const char *name, const char *output, uint32_t size);
int madeira_cab_extract(const char *cabinet, const char *staging,
                        madeira_cab_member member, void *context);
#endif
