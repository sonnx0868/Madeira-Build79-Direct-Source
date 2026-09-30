/* Replacement for decommit_pages in pinned virtual_ios.c.  The original
 * ios_pool_live_overlap helper is retained.  All helpers are inlined so the
 * native build can preserve the original function's bounds/unwind frame.
 * The heavy dc-census/decommit-zero logging is deliberately omitted: no raw
 * verification read is permitted after a temporary PROT_NONE page is restored.
 */
#define IOS_DC_INLINE static inline __attribute__((always_inline))
struct ios_dc_edge
{
    char *page, *base;
    size_t size;
    vm_prot_t original, maximum;
    int changed;
};

IOS_DC_INLINE int ios_dc_query( char *page, vm_prot_t *current, vm_prot_t *maximum )
{
    mach_vm_address_t address = (mach_vm_address_t)(uintptr_t)page;
    mach_vm_size_t size = 0;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object = MACH_PORT_NULL;
    kern_return_t kr = mach_vm_region( mach_task_self(), &address, &size,
                                      VM_REGION_BASIC_INFO_64,
                                      (vm_region_info_t)&info, &count, &object );
    if (object != MACH_PORT_NULL) mach_port_deallocate( mach_task_self(), object );
    if (kr != KERN_SUCCESS || count != VM_REGION_BASIC_INFO_COUNT_64 ||
        address > (uintptr_t)page || size < host_page_size ||
        (uintptr_t)page - address > size - host_page_size) return 0;
    *current = info.protection;
    *maximum = info.max_protection;
    return 1;
}

IOS_DC_INLINE int ios_dc_prepare( struct ios_dc_edge *edge, char *base, size_t size )
{
    vm_prot_t current, maximum;
    uintptr_t overlap_base = 0, overlap_end = 0;
    uintptr_t pool_rx = (uintptr_t)ios_jit_rx_base_global;
    uintptr_t pool_rw = (uintptr_t)ios_jit_rw_base_global;
    extern int ios_jit_anon_alias_overlaps( void *, size_t, uintptr_t *, uintptr_t * );
    edge->page = ROUND_ADDR( base, host_page_mask );
    edge->base = base;
    edge->size = size;
    edge->changed = 0;
    /* A plain subrange must not grant write access to a neighboring JIT alias
     * sharing this physical host page.  The caller's alias-at-base route stays
     * unchanged; this is a guard on the plain partial-page route only. */
    if (ios_jit_anon_alias_overlaps( edge->page, host_page_size,
                                   &overlap_base, &overlap_end )) return 0;
    /* Neither view of the physical JIT pool may be made writable by this
     * plain-page helper, even if no user-VA alias entry covers it. */
    if (ios_jit_pool_size_global)
    {
        uintptr_t p = (uintptr_t)edge->page;
        if ((pool_rx && ((p >= pool_rx && p - pool_rx < ios_jit_pool_size_global) ||
                         (pool_rx >= p && pool_rx - p < host_page_size))) ||
            (pool_rw && ((p >= pool_rw && p - pool_rw < ios_jit_pool_size_global) ||
                         (pool_rw >= p && pool_rw - p < host_page_size)))) return 0;
    }
    if (!ios_dc_query( edge->page, &current, &maximum )) return 0;
    edge->original = current;
    edge->maximum = maximum;
    if ((current & VM_PROT_EXECUTE) ||
        (maximum & (VM_PROT_READ | VM_PROT_WRITE)) != (VM_PROT_READ | VM_PROT_WRITE)) return 0;
    if ((current & (VM_PROT_READ | VM_PROT_WRITE)) == (VM_PROT_READ | VM_PROT_WRITE)) return 1;
    /* Mark before the call: even an unsuccessful operation is followed by a
     * best-effort restoration and checked error result, never by a raw clear. */
    edge->changed = 1;
    if (mach_vm_protect( mach_task_self(), (mach_vm_address_t)(uintptr_t)edge->page,
                         host_page_size, FALSE, current | VM_PROT_READ | VM_PROT_WRITE ) != KERN_SUCCESS)
        return 0;
    if (!ios_dc_query( edge->page, &current, &maximum )) return 0;
    return current == (edge->original | VM_PROT_READ | VM_PROT_WRITE) && maximum == edge->maximum;
}

IOS_DC_INLINE int ios_dc_restore( struct ios_dc_edge *edge )
{
    vm_prot_t current, maximum;
    if (!edge->changed) return 1;
    if (mach_vm_protect( mach_task_self(), (mach_vm_address_t)(uintptr_t)edge->page,
                         host_page_size, FALSE, edge->original ) != KERN_SUCCESS) return 0;
    if (!ios_dc_query( edge->page, &current, &maximum )) return 0;
    return current == edge->original && maximum == edge->maximum;
}

static NTSTATUS decommit_pages( struct file_view *view, char *base, size_t size )
{
    struct ios_dc_edge edges[2];
    char *end, *host_start, *host_end;
    unsigned count = 0, i;
    NTSTATUS status = STATUS_ACCESS_DENIED;
    uintptr_t rw_alias;
    extern uintptr_t ios_jit_anon_alias_lookup( uintptr_t );

    if (!size) size = view->size;
    /* NtFreeVirtualMemory validates the view/range; keep arithmetic bounded
     * independently because host rounding may otherwise wrap before access. */
    if (!size || (uintptr_t)base > ~(uintptr_t)0 - size ||
        (uintptr_t)base > ~(uintptr_t)0 - host_page_mask) return STATUS_ACCESS_DENIED;
    end = base + size;
    host_start = (char *)(((uintptr_t)base + host_page_mask) & ~(uintptr_t)host_page_mask);
    host_end = ROUND_ADDR( end, host_page_mask );

    rw_alias = ios_jit_anon_alias_lookup( (uintptr_t)base );
    if (rw_alias)
    {
        /* Preserve the existing stale-alias refusal: do not blank a loaded
         * module.  The historical final logical decommit behavior is retained. */
        size_t ledger_offset = 0;
        void *ledger_peb = NULL;
        if (!ios_pool_live_overlap( rw_alias, size, &ledger_offset, &ledger_peb ))
            memset( (void *)rw_alias, 0, size );
        else
            dprintf(2, "[alias-tomb] STALE alias decommit REFUSED: user_va=%p size=0x%lx rw_alias=0x%lx -> pool off=0x%lx (LIVE, peb=%p) — would have zeroed a loaded module\n",
                    base, (unsigned long)size, (unsigned long)rw_alias,
                    (unsigned long)ledger_offset, ledger_peb);
        goto success;
    }

    /* At most two partial host pages.  Equal host_start/host_end can mean two
     * edge pages with no full interior page, not only one subpage interval. */
    if (base < host_start)
    {
        char *edge_end = end < host_start ? end : host_start;
        if (!ios_dc_prepare( &edges[count++], base, edge_end - base )) goto cleanup;
    }
    if (host_end >= host_start && host_end < end)
    {
        if (!ios_dc_prepare( &edges[count++], host_end, end - host_end )) goto cleanup;
    }

    if (host_start < host_end &&
        anon_mmap_fixed( host_start, host_end - host_start, PROT_READ | PROT_WRITE, 0 ) == MAP_FAILED)
    {
        status = STATUS_NO_MEMORY;
        goto cleanup;
    }
    for (i = 0; i < count; ++i)
    {
        volatile const unsigned char *p = (const unsigned char *)edges[i].base;
        size_t n;
        memset( edges[i].base, 0, edges[i].size );
        /* Read while the page is known R/W.  Volatile prevents the compiler
         * replacing this actual readback with knowledge about memset. */
        for (n = 0; n < edges[i].size; ++n) if (p[n]) goto cleanup;
    }
    status = STATUS_SUCCESS;
cleanup:
    while (count) if (!ios_dc_restore( &edges[--count] )) status = STATUS_ACCESS_DENIED;
    if (status != STATUS_SUCCESS) return status;
success:
    set_page_vprot_bits( base, size, 0, VPROT_COMMITTED );
    if (host_start < host_end) kernel_writewatch_register_range( view, host_start, host_end - host_start );
    return STATUS_SUCCESS;
}
#undef IOS_DC_INLINE
