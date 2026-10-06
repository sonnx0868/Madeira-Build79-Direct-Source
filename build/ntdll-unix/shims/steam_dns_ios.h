/* SPDX-License-Identifier: LGPL-2.1-or-later
 * App-local Steam DNS/DoH compatibility for Madeira on iOS. */
#ifndef MADEIRA_STEAM_DNS_IOS_H
#define MADEIRA_STEAM_DNS_IOS_H

#include <stddef.h>
#include <netdb.h>

/* True only for a Dock session's Steam domains, unless the user explicitly
 * chooses "all". This never changes the iPad's network configuration. */
int madeira_steam_dns_should_override( const char *hostname );

/* Native addrinfo list allocated by Madeira; free it with the matching helper. */
int madeira_steam_dns_getaddrinfo( const char *hostname, const char *service,
                                  const struct addrinfo *hints, struct addrinfo **result );
void madeira_steam_dns_freeaddrinfo( struct addrinfo *result );

/* DNS wire-format answer for dnsapi's res_query-compatible path. */
int madeira_steam_dns_query( const char *hostname, int dns_class, int dns_type,
                            unsigned char *answer, int answer_size );

#endif
