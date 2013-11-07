/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/******************************************************\
		Portable socket library header.
\******************************************************/

#if !defined(hysocket_h)
#define hysocket_h

#include <stddef.h>
#include "hyporterror.h"

/* Socket types, stream & datagram */
#define HYSOCK_STREAM 0
#define HYSOCK_DGRAM 1
#define HYSOCK_AFINET 2
#define HYSOCK_ANY 3
#define HYSOCK_DEFPROTOCOL 0
#define HYSOCK_INADDR_ANY (U_32)0	
#define HYSOCK_NOFLAGS (U_32)0       /* The default flag argument value, as in a recv */
#define HYSOCK_INADDR_LEN 4          /* The length in bytes of a binary IPv4 internet address */
#define HYSOCK_INADDR6_LEN 16        /* The length in bytes of a binary IPv6 internet address */

/* For getaddrinfo (IPv6) -- socket types */
#define HYSOCKET_ANY 0        /* for getaddrinfo hints */
#define HYSOCKET_STREAM 1     /* stream socket */
#define HYSOCKET_DGRAM 2      /* datagram socket */
#define HYSOCKET_RAW 3        /* raw-protocol interface */
#define HYSOCKET_RDM 4        /* reliably-delivered message */
#define HYSOCKET_SEQPACKET 5  /* sequenced packet stream */

/** address family */
#define HYADDR_FAMILY_UNSPEC 0     /* IPv6 */
#define HYADDR_FAMILY_AFINET4 2    /* IPv6 */
#define HYADDR_FAMILY_AFINET6 23   /* IPv6 */

/** protocol family */
#define HYPROTOCOL_FAMILY_UNSPEC  HYADDR_FAMILY_UNSPEC     /* IPv6 */
#define HYPROTOCOL_FAMILY_INET4   HYADDR_FAMILY_AFINET4    /* IPv6 */
#define HYPROTOCOL_FAMILY_INET6   HYADDR_FAMILY_AFINET6    /* IPv6 */

/* Portable defines for socket levels */
#define HY_SOL_SOCKET 1
#define HY_IPPROTO_TCP 2
#define HY_IPPROTO_IP 3
#define HY_IPPROTO_IPV6 4

/* Portable defines for socket options */
#define HY_SO_LINGER 1
#define HY_SO_KEEPALIVE 2
#define HY_TCP_NODELAY 3
#define HY_MCAST_TTL 4
#define HY_MCAST_ADD_MEMBERSHIP 5
#define HY_MCAST_DROP_MEMBERSHIP 6
#define HY_MCAST_INTERFACE 7
#define HY_SO_REUSEADDR 8
#define HY_SO_REUSEPORT 9
#define HY_SO_SNDBUF 11
#define HY_SO_RCVBUF 12
#define HY_SO_BROADCAST 13
#define HY_SO_OOBINLINE 14
#define HY_IP_MULTICAST_LOOP 15
#define HY_IP_TOS 16
#define HY_MCAST_INTERFACE_2 17
#define HY_IPV6_ADD_MEMBERSHIP 18
#define HY_IPV6_DROP_MEMBERSHIP	19

/* Portable defines for socket read/write options */
#define HYSOCK_MSG_PEEK 1
#define HYSOCK_MSG_OOB 2

/* Platform Constants */
typedef struct hysocket_struct *hysocket_t;
typedef struct hysockaddr_struct *hysockaddr_t;
typedef struct hyhostent_struct *hyhostent_t;
typedef struct hyfdset_struct *hyfdset_t;
typedef struct hytimeval_struct *hytimeval_t;
typedef struct hylinger_struct *hylinger_t;
typedef struct hyipmreq_struct *hyipmreq_t;
typedef struct hyaddrinfo_struct *hyaddrinfo_t;    /* IPv6 */
typedef struct hyipv6_mreq_struct *hyipv6_mreq_t;  /* IPv6 */

/* constants for calling multi-call functions */
#define HY_PORT_SOCKET_STEP_START 		10
#define HY_PORT_SOCKET_STEP_CHECK	20
#define HY_PORT_SOCKET_STEP_DONE		30

#endif     /* hysocket_h */
