/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Copyright 2026 Madeira contributors
 *
 * App-local resolver used only by the Wine/Steam session. DoH changes where
 * names are resolved, not where TCP/UDP connections originate: the device's
 * public IP and the CDN data path remain unchanged. The Objective-C bridge
 * performs HTTPS with NSURLSession/System Trust; this file handles policy,
 * DNS parsing and native addrinfo construction for ws2_32/dnsapi.
 */
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "steam_dns_ios.h"

/* Implemented in WineProcessBridge.m. A weak import keeps a standalone unix
 * library test/build safe; a missing app bridge simply falls back to system DNS. */
extern int madeira_doh_query_name( const char *hostname, unsigned short qtype,
                                   unsigned char *answer, int answer_size,
                                   int *provider ) __attribute__((weak));

enum madeira_dns_mode
{
    DNS_MODE_OFF,
    DNS_MODE_AUTO,
    DNS_MODE_CLOUDFLARE,
    DNS_MODE_GOOGLE,
    DNS_MODE_ALL,
};

static enum madeira_dns_mode get_mode(void)
{
    const char *value = getenv( "MADEIRA_STEAM_DNS" );

    if (value && *value)
    {
        if (!strcasecmp( value, "0" ) || !strcasecmp( value, "off" ) ||
            !strcasecmp( value, "system" )) return DNS_MODE_OFF;
        if (!strcasecmp( value, "cloudflare" )) return DNS_MODE_CLOUDFLARE;
        if (!strcasecmp( value, "google" )) return DNS_MODE_GOOGLE;
        if (!strcasecmp( value, "all" )) return DNS_MODE_ALL;
        return DNS_MODE_AUTO;
    }
    value = getenv( "MADEIRA_DOCK_SESSION" );
    return value && value[0] == '1' ? DNS_MODE_AUTO : DNS_MODE_OFF;
}

static int has_domain_suffix( const char *name, const char *suffix )
{
    size_t name_len, suffix_len;

    if (!name || !(name_len = strlen(name)) || !(suffix_len = strlen(suffix)))
        return 0;
    while (name_len && name[name_len - 1] == '.') name_len--;
    if (name_len < suffix_len) return 0;
    name += name_len - suffix_len;
    if (strcasecmp( name, suffix )) return 0;
    return name_len == suffix_len || name[-1] == '.';
}

int madeira_steam_dns_should_override( const char *hostname )
{
    static const char * const steam_domains[] =
    {
        "steampowered.com", "steamcommunity.com", "steamstatic.com",
        "steamcontent.com", "steamserver.net", "steamgames.com",
        "steamusercontent.com", "steam-chat.com", "steam-api.com",
        "steamcdn-a.akamaihd.net", "steampipe.akamaized.net",
        "steamstore-a.akamaihd.net", "steamuserimages-a.akamaihd.net",
        "valvesoftware.com",
    };
    enum madeira_dns_mode mode = get_mode();
    unsigned int i;

    if (!hostname || !*hostname || mode == DNS_MODE_OFF) return 0;
    if (mode == DNS_MODE_ALL) return 1;
    for (i = 0; i < sizeof(steam_domains) / sizeof(steam_domains[0]); i++)
        if (has_domain_suffix( hostname, steam_domains[i] )) return 1;
    return 0;
}

int madeira_steam_dns_query( const char *hostname, int dns_class, int dns_type,
                            unsigned char *answer, int answer_size )
{
    static unsigned int success_logs, failure_logs;
    int provider = 0, ret;

    if (dns_class != 1 || !madeira_steam_dns_should_override( hostname ) ||
        !madeira_doh_query_name || !answer || answer_size < 12) return -1;
    ret = madeira_doh_query_name( hostname, dns_type, answer, answer_size, &provider );
    if (ret >= 12)
    {
        if (__sync_fetch_and_add( &success_logs, 1 ) < 6)
            dprintf( STDERR_FILENO, "[steam-dns] DoH answer provider=%s type=%d bytes=%d\n",
                     provider == 1 ? "cloudflare" : provider == 2 ? "google" : "cache", dns_type, ret );
        return ret;
    }
    if (__sync_fetch_and_add( &failure_logs, 1 ) < 4)
        dprintf( STDERR_FILENO, "[steam-dns] DoH unavailable type=%d; falling back to system DNS\n", dns_type );
    return -1;
}

static int read_u16( const unsigned char *data, size_t size, size_t offset, unsigned int *value )
{
    if (offset + 2 > size) return 0;
    *value = ((unsigned int)data[offset] << 8) | data[offset + 1];
    return 1;
}

static int skip_name( const unsigned char *data, size_t size, size_t *offset )
{
    size_t p = *offset;
    unsigned int labels = 0;

    while (p < size && labels++ < 128)
    {
        unsigned int len = data[p++];
        if (!len) { *offset = p; return 1; }
        if ((len & 0xc0) == 0xc0)
        {
            if (p >= size) return 0;
            *offset = p + 1;
            return 1;
        }
        if (len & 0xc0 || p + len > size) return 0;
        p += len;
    }
    return 0;
}

struct resolved_address
{
    int family;
    unsigned char bytes[16];
};

static int parse_addresses( const unsigned char *data, size_t size, int wanted_family,
                            struct resolved_address *addresses, int capacity )
{
    unsigned int questions, answers, i, type, dns_class, rdlength;
    size_t offset = 12;
    int count = 0;

    if (size < 12 || !read_u16( data, size, 4, &questions ) ||
        !read_u16( data, size, 6, &answers )) return 0;
    for (i = 0; i < questions; i++)
    {
        if (!skip_name( data, size, &offset ) || offset + 4 > size) return 0;
        offset += 4;
    }
    for (i = 0; i < answers && offset < size; i++)
    {
        if (!skip_name( data, size, &offset ) ||
            !read_u16( data, size, offset, &type ) ||
            !read_u16( data, size, offset + 2, &dns_class ) ||
            !read_u16( data, size, offset + 8, &rdlength ) || offset + 10 + rdlength > size) break;
        offset += 10;
        if (dns_class == 1 && count < capacity &&
            ((type == 1 && rdlength == 4 && wanted_family != AF_INET6) ||
             (type == 28 && rdlength == 16 && wanted_family != AF_INET)))
        {
            addresses[count].family = type == 1 ? AF_INET : AF_INET6;
            memcpy( addresses[count].bytes, data + offset, rdlength );
            count++;
        }
        offset += rdlength;
    }
    return count;
}

static int service_port( const char *service, int socktype )
{
    char *end;
    long port;
    struct servent *entry;

    if (!service || !*service) return 0;
    errno = 0;
    port = strtol( service, &end, 10 );
    if (!errno && !*end && port >= 0 && port <= 65535) return (int)port;
    entry = getservbyname( service, socktype == SOCK_DGRAM ? "udp" : "tcp" );
    return entry ? ntohs(entry->s_port) : -1;
}

static struct addrinfo *new_addrinfo( const struct resolved_address *address,
                                      const struct addrinfo *hints, int socktype,
                                      int protocol, int port, const char *canonname )
{
    struct addrinfo *info = calloc( 1, sizeof(*info) );
    size_t addr_size = address->family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

    if (!info || !(info->ai_addr = calloc( 1, addr_size ))) { free(info); return NULL; }
    info->ai_flags = hints ? hints->ai_flags : 0;
    info->ai_family = address->family;
    info->ai_socktype = socktype;
    info->ai_protocol = protocol;
    info->ai_addrlen = addr_size;
    if (address->family == AF_INET)
    {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        sa->sin_family = AF_INET;
        sa->sin_port = htons( port );
        memcpy( &sa->sin_addr, address->bytes, 4 );
    }
    else
    {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        sa->sin6_family = AF_INET6;
        sa->sin6_port = htons( port );
        memcpy( &sa->sin6_addr, address->bytes, 16 );
    }
    if (canonname && !(info->ai_canonname = strdup(canonname)))
    {
        free( info->ai_addr );
        free( info );
        return NULL;
    }
    return info;
}

void madeira_steam_dns_freeaddrinfo( struct addrinfo *result )
{
    while (result)
    {
        struct addrinfo *next = result->ai_next;
        free( result->ai_addr );
        free( result->ai_canonname );
        free( result );
        result = next;
    }
}

int madeira_steam_dns_getaddrinfo( const char *hostname, const char *service,
                                  const struct addrinfo *hints, struct addrinfo **result )
{
    unsigned char response[4096];
    struct resolved_address addresses[16];
    struct addrinfo *first = NULL, **tail = &first;
    int family = hints ? hints->ai_family : AF_UNSPEC;
    int types[2], protocols[2], type_count, port, count = 0, i, t, ret;

    if (!result || !madeira_steam_dns_should_override( hostname )) return EAI_NONAME;
    *result = NULL;
    if (family != AF_UNSPEC && family != AF_INET && family != AF_INET6) return EAI_FAMILY;

    if (family != AF_INET6 &&
        (ret = madeira_steam_dns_query( hostname, 1, 1, response, sizeof(response) )) >= 12)
        count += parse_addresses( response, ret, AF_INET, addresses + count, 16 - count );
    if (family != AF_INET && count < 16 &&
        (ret = madeira_steam_dns_query( hostname, 1, 28, response, sizeof(response) )) >= 12)
        count += parse_addresses( response, ret, AF_INET6, addresses + count, 16 - count );
    if (!count) return EAI_NONAME;

    if (hints && hints->ai_socktype)
    {
        types[0] = hints->ai_socktype;
        protocols[0] = hints->ai_protocol;
        type_count = 1;
    }
    else
    {
        types[0] = SOCK_STREAM; protocols[0] = IPPROTO_TCP;
        types[1] = SOCK_DGRAM; protocols[1] = IPPROTO_UDP;
        type_count = 2;
    }

    for (i = 0; i < count; i++) for (t = 0; t < type_count; t++)
    {
        struct addrinfo *entry;
        if ((port = service_port( service, types[t] )) < 0) { madeira_steam_dns_freeaddrinfo(first); return EAI_SERVICE; }
        entry = new_addrinfo( &addresses[i], hints, types[t], protocols[t], port,
                              !first && hints && (hints->ai_flags & AI_CANONNAME) ? hostname : NULL );
        if (!entry) { madeira_steam_dns_freeaddrinfo(first); return EAI_MEMORY; }
        *tail = entry;
        tail = &entry->ai_next;
    }
    *result = first;
    return 0;
}
