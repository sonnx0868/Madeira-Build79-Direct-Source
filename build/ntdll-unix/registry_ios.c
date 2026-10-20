/* Wine's registry implementation remains authoritative. The adapters are
 * scoped to a fingerprint-verified game profile published by the app. */
#define NtQueryValueKey madeira_registry_query_raw
#define NtSetValueKey madeira_registry_set_raw
#define NtEnumerateValueKey madeira_registry_enumerate_raw
#define NtQueryKey madeira_registry_key_raw
#include "../../wine/dlls/ntdll/unix/registry.c"
#undef NtQueryValueKey
#undef NtSetValueKey
#undef NtEnumerateValueKey
#undef NtQueryKey

#define MGP_QUERY_KEY madeira_registry_key_raw
#include "ios_graphics_preferences.h"
#include "ios_graphics_enumeration.h"

NTSTATUS WINAPI NtQueryValueKey(HANDLE key, const UNICODE_STRING *name,
    KEY_VALUE_INFORMATION_CLASS cls, void *info, DWORD length, DWORD *needed)
{
    NTSTATUS status = madeira_registry_query_raw(key, name, cls, info, length, needed);
    return madeira_graphics_query(status, key, name, cls, info, length, needed,
                                  madeira_registry_query_raw);
}

NTSTATUS WINAPI NtSetValueKey(HANDLE key, const UNICODE_STRING *name, ULONG index,
    ULONG type, const void *data, ULONG length)
{
    if (madeira_graphics_preserve(key, name, type, data, length, madeira_registry_query_raw))
        return STATUS_SUCCESS;
    return madeira_registry_set_raw(key, name, index, type, data, length);
}

NTSTATUS WINAPI NtEnumerateValueKey(HANDLE key, ULONG index, KEY_VALUE_INFORMATION_CLASS cls,
    void *info, DWORD length, DWORD *needed)
{
    NTSTATUS status=madeira_registry_enumerate_raw(key,index,cls,info,length,needed);
    return madeira_graphics_enumerate(status,key,index,cls,info,length,needed,
        madeira_registry_enumerate_raw,madeira_registry_query_raw);
}

NTSTATUS WINAPI NtQueryKey(HANDLE key, KEY_INFORMATION_CLASS cls, void *info, DWORD length, DWORD *needed)
{
    NTSTATUS status=madeira_registry_key_raw(key,cls,info,length,needed);
    madeira_graphics_key_info(status,key,cls,info,length,madeira_registry_query_raw);
    return status;
}
