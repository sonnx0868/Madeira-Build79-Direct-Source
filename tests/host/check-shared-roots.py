#!/usr/bin/env python3
"""Exercise production CA enumeration across sequential and concurrent imports."""
from pathlib import Path
import os, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/crypto-unix/crypt32_unixlib_ios.c').read_text(encoding='utf-8')
body = source[source.index('struct ios_root_walk {'):source.index('const unixlib_entry_t __wine_unix_call_funcs[]')]
assert 'list_remove' not in body and 'free( cert )' not in body
assert 'return ios_enum_root_certs( &params );' in source
fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "wine/list.h"
typedef uint32_t DWORD;
typedef int32_t NTSTATUS;
#define STATUS_SUCCESS 0
#define STATUS_NO_MEMORY ((NTSTATUS)0xc0000017)
#define STATUS_NO_MORE_ENTRIES ((NTSTATUS)0x8000001a)
struct root_cert { struct list entry; size_t size; unsigned char data[1]; };
struct enum_root_certs_params { void *buffer; DWORD size; DWORD *needed; };
static struct list root_cert_list = LIST_INIT(root_cert_list);
static pthread_mutex_t ios_root_lock = PTHREAD_MUTEX_INITIALIZER;
static void ios_load_roots_locked(void) {}
''' + body + r'''
static void *walk_roots(void *arg)
{
    for (unsigned pass = 0; pass < 2; pass++)
    {
        unsigned char buffer[3072];
        DWORD needed;
        struct enum_root_certs_params params = {buffer, sizeof(buffer), &needed};
        for (unsigned i = 0; i < 121; i++)
        {
            params.size = 1;
            assert(enum_root_certs(&params) == STATUS_SUCCESS && needed == sizeof(buffer));
            params.size = sizeof(buffer);
            assert(enum_root_certs(&params) == STATUS_SUCCESS);
            for (unsigned j = 0; j < sizeof(buffer); j++) assert(buffer[j] == i);
            if (!i && arg) usleep(5000); // overlap >32 enumerations
        }
        assert(enum_root_certs(&params) == STATUS_NO_MORE_ENTRIES);
    }
    return NULL;
}
int main(void)
{
    for (unsigned i = 0; i < 121; i++)
    {
        struct root_cert *cert = malloc(offsetof(struct root_cert, data) + 3072);
        assert(cert); cert->size = 3072; memset(cert->data, i, cert->size);
        list_add_tail(&root_cert_list, &cert->entry);
    }
    walk_roots(NULL); // a later import on the same thread must receive all roots again
    pthread_t threads[128];
    for (unsigned i = 0; i < 128; i++) assert(!pthread_create(&threads[i], NULL, walk_roots, (void *)1));
    for (unsigned i = 0; i < 128; i++) assert(!pthread_join(threads[i], NULL));
    walk_roots(NULL);
    unsigned count = 0;
    while (!list_empty(&root_cert_list))
    {
        struct root_cert *cert = LIST_ENTRY(list_head(&root_cert_list), struct root_cert, entry);
        list_remove(&cert->entry); free(cert); count++;
    }
    assert(count == 121);
    puts("PASS: all imports receive every root; resize retries preserve position; 128 concurrent walks remain independent");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-shared-roots-') as tmp:
    path = Path(tmp)
    (path / 'check.c').write_text(fixture, encoding='utf-8')
    cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
    subprocess.run([cc, '-std=gnu11', '-Wall', '-Werror', '-I', str(root / 'wine/include'),
                    '-fsanitize=address,undefined', '-pthread', str(path / 'check.c'), '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=40)
