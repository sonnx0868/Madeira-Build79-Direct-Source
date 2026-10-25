/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Copyright 2026 125hz
 *
 * iOS unix side for dlls/dnsapi/libresolv.c.
 *
 * dnsapi.dll had no unix side on this port, so the unixlib loader bound its
 * module to the generic stub table, whose every entry returns
 * STATUS_NOT_SUPPORTED. DnsQuery_* failed, and so did DnsQueryConfig's DNS
 * server list, which iphlpapi's GetAdaptersAddresses asks for: every call of
 * it returned 0xc00000bb (as a Win32 error) even with the NSI interface and
 * address tables served. Valve's Windows client asserts on exactly that
 * ("CalcUnIPThisBox - GetAdaptersAddresses returned -1073741637") in a
 * Madeira Dock session.
 *
 * Same shape as dwrite_freetype_ios.c: keep upstream's file as the
 * implementation and rewrite only the way it reaches its host library.
 * Upstream dlls/dnsapi/libresolv.c calls res_init()/res_query() and touches
 * `_res` directly, expecting configure to have linked -lresolv. Linking
 * -lresolv here would add a dependency to the app's final link, which this
 * build stage does not own, so the three things it uses are resolved at first
 * use out of /usr/lib/libresolv.9.dylib with dlopen/dlsym (a system library,
 * which an iOS sandbox may dlopen).
 *
 * Bind res_9_ninit, res_9_nquery and res_9_ndestroy as one BIND9 ABI.
 * Each pthread owns zero-initialized resolver storage and releases it on exit.
 * Do not pass a borrowed __res_state result to a differently versioned API.
 * Missing functions and failed initialization return a normal DNS error.
 *
 * virtual_ios.c binds this table for dnsapi.dll; MADEIRA_DNSAPI_UNIXLIB=0
 * keeps the stub table as before.
 */
/* Wine's configure ran for macOS and its config.h does not turn this file on
 * (HAVE_RESOLV is what guards ALL of libresolv.c).  Enable it and the headers
 * it needs here, for this TU only. */
#undef HAVE_RESOLV
#define HAVE_RESOLV 1
#undef HAVE_RESOLV_H
#define HAVE_RESOLV_H 1
#undef HAVE_NETINET_IN_H
#define HAVE_NETINET_IN_H 1
#undef HAVE_ARPA_NAMESER_H
#define HAVE_ARPA_NAMESER_H 1
#undef HAVE_NETDB_H
#define HAVE_NETDB_H 1

/* Deliberately NOT enabled:
 *  - HAVE_RES_GETSERVERS would pull in res_9_getservers and a
 *    union res_sockaddr_union layout for one more dynamic symbol; the
 *    _res.nscount path below it needs nothing extra and reports the same
 *    IPv4 servers.
 *  - HAVE_STRUCT___RES_STATE__U__EXT_NSCOUNT6 would report IPv6 nameservers
 *    out of _u._ext.nsaddrs, which is only populated by res_ninit when the
 *    system resolver configuration lists any; leaving it off costs an IPv6-only
 *    setup its server list and costs a query nothing, because res_nquery uses
 *    the state's own servers either way. */
#undef HAVE_RES_GETSERVERS
#undef HAVE_STRUCT___RES_STATE__U__EXT_NSCOUNT6

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include "steam_dns_ios.h"

/***********************************************************************
 *           the libresolv binding
 */
typedef int (*ios_res_ninit_fn)( struct __res_state * );
typedef int (*ios_res_nquery_fn)( struct __res_state *, const char *, int, int, unsigned char *, int );
typedef void (*ios_res_ndestroy_fn)( struct __res_state * );
static pthread_once_t ios_resolv_once = PTHREAD_ONCE_INIT;
static pthread_key_t ios_resolv_key;
static int ios_resolv_key_valid;
static void *ios_resolv_lib;
static ios_res_ninit_fn ios_resolv_ninit;
static ios_res_nquery_fn ios_resolv_nquery;
static ios_res_ndestroy_fn ios_resolv_ndestroy;
static __thread int ios_resolv_h_errno;
static __thread struct __res_state ios_resolv_null_state;

struct ios_resolver_thread
{
    struct __res_state state;
    int attempted;
};

static void ios_resolv_destroy( void *ptr )
{
    struct ios_resolver_thread *owner = ptr;
    if (!owner) return;
    if (owner->attempted) ios_resolv_ndestroy( &owner->state );
    free( owner );
}

static void ios_resolv_load(void)
{
    /* Bind one ABI family. The unversioned __res_state accessor may belong
     * to another resolver ABI; never hand its storage to res_9_ninit.
     * State is ours, per pthread, and res_9_ndestroy releases it at exit. */
    ios_resolv_lib = dlopen( "/usr/lib/libresolv.9.dylib", RTLD_NOW | RTLD_LOCAL );
    if (ios_resolv_lib)
    {
        ios_resolv_ninit = (ios_res_ninit_fn)dlsym( ios_resolv_lib, "res_9_ninit" );
        ios_resolv_nquery = (ios_res_nquery_fn)dlsym( ios_resolv_lib, "res_9_nquery" );
        ios_resolv_ndestroy = (ios_res_ndestroy_fn)dlsym( ios_resolv_lib, "res_9_ndestroy" );
        if (ios_resolv_ninit && ios_resolv_nquery && ios_resolv_ndestroy &&
            !pthread_key_create( &ios_resolv_key, ios_resolv_destroy ))
            ios_resolv_key_valid = 1;
    }
    dprintf( STDERR_FILENO, "[dns-state-v1] owned-pthread-state=%d abi=bind9 cleanup=ndestroy\n",
             ios_resolv_key_valid );
}

static struct __res_state *ios_resolv_state(void)
{
    struct ios_resolver_thread *owner;
    pthread_once( &ios_resolv_once, ios_resolv_load );
    if (!ios_resolv_key_valid) return &ios_resolv_null_state;
    if (!(owner = pthread_getspecific( ios_resolv_key )))
    {
        if (!(owner = calloc( 1, sizeof(*owner) ))) return &ios_resolv_null_state;
        if (pthread_setspecific( ios_resolv_key, owner ))
        {
            free( owner );
            return &ios_resolv_null_state;
        }
    }
    return &owner->state;
}

static int ios_resolv_init(void)
{
    struct __res_state *state = ios_resolv_state();
    struct ios_resolver_thread *owner;
    if (state != &ios_resolv_null_state)
    {
        owner = pthread_getspecific( ios_resolv_key );
        if (owner->attempted) ios_resolv_ndestroy( state );
        memset( state, 0, sizeof(*state) );
        owner->attempted = 1;
        if (!ios_resolv_ninit( state )) return 0;
        ios_resolv_ndestroy( state );
        memset( state, 0, sizeof(*state) );
        owner->attempted = 0;
    }
    ios_resolv_h_errno = TRY_AGAIN;
    return -1;
}

static int ios_resolv_query( const char *dname, int class, int type,
                             unsigned char *answer, int anslen )
{
    struct __res_state *state = ios_resolv_state();
    int ret;

    /* Steam/Dock compatibility: bypass a poisoned or blocked ISP resolver
     * through the app-local DoH bridge. The wire answer is exactly what
     * upstream libresolv.c expects, so DnsQuery A/AAAA/CNAME keeps its normal
     * parsing and status mapping. System DNS remains the final fallback. */
    if (madeira_steam_dns_should_override( dname ) &&
        (ret = madeira_steam_dns_query( dname, class, type, answer, anslen )) >= 0)
        return ret;

    if (state != &ios_resolv_null_state)
    {
        if ((state->options & RES_INIT) || !ios_resolv_init())
        {
            ret = ios_resolv_nquery( state, dname, class, type, answer, anslen );
            /* res_nquery reports the DNS-level failure here, and map_h_errno()
             * in libresolv.c is what turns it into a DNS_ERROR_RCODE_*. */
            if (ret < 0) ios_resolv_h_errno = state->res_h_errno;
            return ret;
        }
    }
    ios_resolv_h_errno = TRY_AGAIN;   /* -> DNS_ERROR_RCODE_SERVER_FAILURE */
    return -1;
}

/***********************************************************************
 *           upstream, with its three host-resolver references rewritten
 *
 * These have to come AFTER the definitions above, or the shims would rename
 * themselves.  <resolv.h> is already included, so the #include below sees only
 * its include guard and these macros survive.
 */
#undef res_init
#define res_init  ios_resolv_init
#undef res_query
#define res_query ios_resolv_query
#undef _res
#define _res      (*ios_resolv_state())
#undef h_errno
#define h_errno   ios_resolv_h_errno

#include "libresolv.c"
