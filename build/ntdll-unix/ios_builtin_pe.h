/* Explicit builtin preferences must select a Wine PE, even when a native
 * DLL was found earlier in the application directory. iOS has no loadable
 * builtin .so files; use only the architecture farms staged by the app.
 * Included by loader_ios.c with the normal Wine declarations in scope. */
static NTSTATUS ios_load_builtin_pe( const UNICODE_STRING *requested, USHORT machine,
                                    SECTION_IMAGE_INFORMATION *info, void **module,
                                    SIZE_T *size, ULONG_PTR low, ULONG_PTR high, off_t offset )
{
    static const WCHAR root[] = {'\\','?','?','\\','C',':','\\','w','i','n','d','o','w','s','\\'};
    WCHAR path[512];
    UNICODE_STRING name;
    OBJECT_ATTRIBUTES attr;
    IO_STATUS_BLOCK io;
    HANDLE file, mapping;
    NTSTATUS status;
    const char *farm;
    unsigned int length, leaf, count, i;

    if (!requested->Buffer || requested->Length % sizeof(WCHAR) || offset)
        return STATUS_DLL_NOT_FOUND;
    length = requested->Length / sizeof(WCHAR);
    for (leaf = length; leaf && requested->Buffer[leaf-1] != '\\' && requested->Buffer[leaf-1] != '/'; --leaf) {}
    if (leaf == length) return STATUS_DLL_NOT_FOUND;
    /* The calling pseudo-process determines its private farm. The shared
     * session's native_machine may describe a different architecture. */
    if (machine == IMAGE_FILE_MACHINE_I386 || NtCurrentTeb64()) farm = "syswow64";
    else if (ios_is_arm64ec_cur()) farm = "sysx64";
    else farm = "sysaa64";
    count = ARRAY_SIZE(root);
    if (count + strlen(farm) + 1 + length-leaf + 1 > ARRAY_SIZE(path)) return STATUS_DLL_NOT_FOUND;
    memcpy( path, root, sizeof(root) );
    for (i = 0; farm[i]; ++i) path[count++] = farm[i];
    path[count++] = '\\';
    memcpy( path+count, requested->Buffer+leaf, (length-leaf)*sizeof(WCHAR) );
    count += length-leaf;
    path[count] = 0;
    name.Buffer = path;
    name.Length = count * sizeof(WCHAR);
    name.MaximumLength = name.Length + sizeof(WCHAR);
    InitializeObjectAttributes( &attr, &name, OBJ_CASE_INSENSITIVE, NULL, NULL );
    status = NtOpenFile( &file, FILE_READ_DATA | SYNCHRONIZE, &attr, &io,
                         FILE_SHARE_READ | FILE_SHARE_DELETE, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE );
    if (status) return status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND
        ? STATUS_DLL_NOT_FOUND : status;
    status = NtCreateSection( &mapping, SECTION_ALL_ACCESS, NULL, NULL, PAGE_EXECUTE_READ, SEC_IMAGE, file );
    NtClose( file );
    if (status) return status;
    /* This verifies the Wine-builtin flag and the requested machine before
     * mapping. A native file placed in the farm must never satisfy '=b'. */
    status = virtual_map_builtin_module( mapping, module, size, info, low, high, machine, FALSE, 0 );
    NtClose( mapping );
    return status;
}
