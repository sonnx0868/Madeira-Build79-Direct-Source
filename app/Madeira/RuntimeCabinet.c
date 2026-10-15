#include "RuntimeCabinet.h"
#include "vendor/mspack/mspack.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int madeira_cab_extract(const char *path, const char *staging,
                        madeira_cab_member member, void *context) {
    int result = MSPACK_ERR_DATAFORMAT, count = 0;
    uint64_t total = 0;
    struct mscabd_cabinet *cab = NULL;
    struct mscab_decompressor *decoder = mspack_create_cab_decompressor(NULL);
    if (!decoder || !path || !staging || !member) goto done;
    cab = decoder->open(decoder, path);
    if (!cab || (cab->flags & (MSCAB_HDR_PREVCAB | MSCAB_HDR_NEXTCAB))) goto done;
    /* Preflight the complete archive before creating the first output. */
    for (struct mscabd_file *file = cab->files; file; file = file->next) {
        if (++count > 512 || file->length > 128U * 1024 * 1024 ||
            !file->filename || strlen(file->filename) > 1024) goto done;
        total += file->length;
        if (total > 384U * 1024 * 1024) goto done;
    }
    count = 0;
    for (struct mscabd_file *file = cab->files; file; file = file->next) {
        result = MSPACK_ERR_DATAFORMAT;
        char output[4096];
        int length = snprintf(output, sizeof(output), "%s/payload%04d.bin", staging, count++);
        if (length < 0 || length >= (int)sizeof(output)) goto done;
        /* An existing output is a staging collision, never a file to replace. */
        FILE *probe = fopen(output, "rb");
        if (probe) { fclose(probe); goto done; }
        result = decoder->extract(decoder, file, output);
        if (result != MSPACK_ERR_OK) goto done;
        if (member(context, file->filename, output, file->length) != 0) {
            result = MSPACK_ERR_DATAFORMAT; goto done;
        }
    }
    result = MSPACK_ERR_OK;
done:
    if (decoder) {
        if (cab) decoder->close(decoder, cab);
        mspack_destroy_cab_decompressor(decoder);
    }
    return result;
}
