/*
 * rebind: Intercept bind calls and bind to a different port
 * Copyright 2010 Joel Martin
 * Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)
 *
 * Overload (LD_PRELOAD) bind system call. If REBIND_PORT_OLD and
 * REBIND_PORT_NEW environment variables are set then bind on the new
 * port (of localhost) instead of the old port. 
 *
 * This allows a proxy (such as wsproxy) to run on the old port and translate
 * traffic to/from the new port.
 *
 * Usage:
 *     LD_PRELOAD=./rebind.so \
 *     REBIND_PORT_OLD=23 \
 *     REBIND_PORT_NEW=2023 \
 *     program
 */

//#define DO_DEBUG 1

#include <stdio.h>
#include <stdlib.h>

#define __USE_GNU 1  // Pull in RTLD_NEXT
#include <dlfcn.h>

#include <string.h>
#include <netinet/in.h>


#if defined(DO_DEBUG)
#define DEBUG(...) \
    fprintf(stderr, "rebind: "); \
    fprintf(stderr, __VA_ARGS__);
#else
#define DEBUG(...)
#endif


int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    static void * (*func)();
    int do_move = 0;
    struct sockaddr_in * addr_in = (struct sockaddr_in *)addr;
    struct sockaddr_in addr_tmp;
    socklen_t addrlen_tmp;
    char * PORT_OLD, * PORT_NEW, * end1, * end2;
    int ret, oldport, newport, askport = htons(addr_in->sin_port);
    uint32_t askaddr = htons(addr_in->sin_addr.s_addr);
    if (!func) func = (void *(*)()) dlsym(RTLD_NEXT, "bind");

    DEBUG(">> bind(%d, _, %d), askaddr %d, askport %d\n",
          sockfd, addrlen, askaddr, askport);

    /* Determine if we should move this socket */
    if (addr_in->sin_family == AF_INET) {
        // TODO: support IPv6
        PORT_OLD = getenv("REBIND_OLD_PORT");
        PORT_NEW = getenv("REBIND_NEW_PORT");
        if (PORT_OLD && (*PORT_OLD != '\0') &&
            PORT_NEW && (*PORT_NEW != '\0')) {
            oldport = strtol(PORT_OLD, &end1, 10);
            newport = strtol(PORT_NEW, &end2, 10);
            if (oldport && (*end1 == '\0') &&
                newport && (*end2 == '\0') &&
                (oldport == askport)) {
                do_move = 1;
            }
        }
    }

    if (! do_move) {
        /* Just pass everything right through to the real bind */
        ret = (long) func(sockfd, addr, addrlen);
        DEBUG("<< bind(%d, _, %d) ret %d\n", sockfd, addrlen, ret);
        return ret;
    }

    DEBUG("binding fd %d on localhost:%d instead of 0x%x:%d\n",
        sockfd, newport, ntohl(addr_in->sin_addr.s_addr), oldport);

    /* Use a temporary location for the new address information */
    addrlen_tmp = sizeof(addr_tmp);
    memcpy(&addr_tmp, addr, addrlen_tmp);

    /* Bind to other port on the loopback instead */
    addr_tmp.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr_tmp.sin_port = htons(newport);
    ret = (long) func(sockfd, &addr_tmp, addrlen_tmp);

    DEBUG("<< bind(%d, _, %d) ret %d\n", sockfd, addrlen, ret);
    return ret;
}
