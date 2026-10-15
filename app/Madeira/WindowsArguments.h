/* Parse additional Windows arguments before Wine has initialized. The first
 * program argument is supplied separately; no shell or expansion is involved.
 * Rules: Microsoft C startup quoting, including empty arguments, tabs,
 * backslashes before quotes, paired quotes and an open quote at end of input. */
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { MADEIRA_ARGUMENT_OK, MADEIRA_ARGUMENT_TOO_LONG, MADEIRA_ARGUMENT_INVALID_UTF8,
       MADEIRA_ARGUMENT_NO_MEMORY };
enum { MADEIRA_COMMAND_UNITS = 32767, MADEIRA_ARGUMENT_BYTES = 32767 * 3,
       MADEIRA_LAUNCH_PATH_BYTES = 1024 };

struct madeira_windows_arguments {
    char *storage;
    char **argv;
    int argc;
};

static int madeira_argument_utf16_units(const char *text, size_t *units)
{
    const unsigned char *p = (const unsigned char *)text;
    size_t bytes = 0, count = 0;
    while (*p)
    {
        unsigned c = *p++, length, minimum;
        ++bytes;
        if (c < 0x80) { length = 0; minimum = 0; }
        else if (c >= 0xc2 && c <= 0xdf) { c &= 0x1f; length = 1; minimum = 0x80; }
        else if (c >= 0xe0 && c <= 0xef) { c &= 0x0f; length = 2; minimum = 0x800; }
        else if (c >= 0xf0 && c <= 0xf4) { c &= 7; length = 3; minimum = 0x10000; }
        else return MADEIRA_ARGUMENT_INVALID_UTF8;
        for (unsigned i = 0; i < length; ++i)
        {
            if ((*p & 0xc0) != 0x80) return MADEIRA_ARGUMENT_INVALID_UTF8;
            c = (c << 6) | (*p++ & 0x3f);
            ++bytes;
        }
        if (c < minimum || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff))
            return MADEIRA_ARGUMENT_INVALID_UTF8;
        count += c > 0xffff ? 2 : 1;
        if (bytes > MADEIRA_ARGUMENT_BYTES || count >= MADEIRA_COMMAND_UNITS)
            return MADEIRA_ARGUMENT_TOO_LONG;
    }
    *units = count;
    return MADEIRA_ARGUMENT_OK;
}

/* Exact serialized length of one decoded argument in Wine's existing
 * build_command_line, including doubled slashes and its first-argument rule. */
static size_t madeira_argument_quoted_units(const char *text, size_t units, int first)
{
    int quoted = first || !*text || strchr(text, ' ') || strchr(text, '\t');
    size_t extra = quoted ? 2 : 0, slashes = 0;
    if (quoted || strchr(text, '"'))
    {
        for (const char *p = text; *p; ++p)
        {
            if (*p == '\\') ++slashes;
            else
            {
                if (*p == '"') extra += slashes + 1;
                slashes = 0;
            }
        }
        if (quoted) extra += slashes;
    }
    return units + extra;
}

static void madeira_windows_arguments_dispose(struct madeira_windows_arguments *args)
{
    free(args->argv);
    free(args->storage);
    memset(args, 0, sizeof(*args));
}

static int madeira_windows_arguments_init(struct madeira_windows_arguments *args,
                                         const char *executable, const char *source)
{
    size_t executable_units, source_units, total, bytes, capacity;
    int status;
    char *read, *write;
    memset(args, 0, sizeof(*args));
    if (!executable || !*executable) return MADEIRA_ARGUMENT_INVALID_UTF8;
    if (!source) source = "";
    if ((status = madeira_argument_utf16_units(executable, &executable_units)) ||
        (status = madeira_argument_utf16_units(source, &source_units))) return status;
    total = madeira_argument_quoted_units(executable, executable_units, 1);
    if (total + (*source ? 1 + source_units : 0) >= MADEIRA_COMMAND_UNITS)
        return MADEIRA_ARGUMENT_TOO_LONG;
    bytes = strlen(source);
    /* A nonempty argument needs a byte and a separator; empty quoted ones
     * need at least two bytes. Include wine, the executable and final NULL. */
    capacity = bytes / 2 + 4;
    size_t executable_bytes = strlen(executable);
    args->storage = (char *)malloc(executable_bytes + bytes + 2);
    args->argv = (char **)calloc(capacity, sizeof(char *));
    if (!args->storage || !args->argv)
    {
        madeira_windows_arguments_dispose(args);
        return MADEIRA_ARGUMENT_NO_MEMORY;
    }
    memcpy(args->storage, executable, executable_bytes + 1);
    char *arguments = args->storage + executable_bytes + 1;
    memcpy(arguments, source, bytes + 1);
    args->argv[0] = (char *)"wine";
    args->argv[1] = args->storage;
    args->argc = 2;
    read = write = arguments;
    while (*read)
    {
        int quoted = 0;
        while (*read == ' ' || *read == '\t') ++read;
        if (!*read) break;
        args->argv[args->argc++] = write;
        for (;;)
        {
            size_t slashes = 0;
            while (*read == '\\') { ++slashes; ++read; }
            if (*read == '"')
            {
                for (size_t i = 0; i < slashes / 2; ++i) *write++ = '\\';
                if (slashes & 1) { *write++ = '"'; ++read; }
                else if (quoted && read[1] == '"') { *write++ = '"'; read += 2; }
                else { quoted = !quoted; ++read; }
            }
            else
            {
                for (size_t i = 0; i < slashes; ++i) *write++ = '\\';
                if (!*read || (!quoted && (*read == ' ' || *read == '\t'))) break;
                *write++ = *read++;
            }
        }
        /* Advance before terminating: an unquoted token's read/write
         * pointers can coincide at this very separator. */
        while (*read == ' ' || *read == '\t') ++read;
        *write++ = 0;
        {
            const char *argument = args->argv[args->argc - 1];
            size_t units;
            status = madeira_argument_utf16_units(argument, &units);
            if (status) { madeira_windows_arguments_dispose(args); return status; }
            total += 1 + madeira_argument_quoted_units(argument, units, 0);
            if (total >= MADEIRA_COMMAND_UNITS)
            {
                madeira_windows_arguments_dispose(args);
                return MADEIRA_ARGUMENT_TOO_LONG;
            }
        }
    }
    args->argv[args->argc] = NULL;
    return MADEIRA_ARGUMENT_OK;
}
