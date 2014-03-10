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

#if !defined(hysock_h)
#define hysock_h
/******************************************************\
		Portable socket library implementation.
\******************************************************/
#include "xmlvm-hy.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if !defined(ZOS)
#include <netinet/tcp.h>
#else /* !defined(ZOS) */
#include <netinet/tcp_var.h>
#endif /* !defined(ZOS) */

#include <netdb.h>
#include <sys/time.h>
#include <stdlib.h>

#if !defined(ZOS)
#if !defined(MACOSX) && !defined(__CYGWIN__)
#include <sys/socketvar.h>
#endif
#else /* !defined(ZOS) */
#include <arpa/inet.h>
#include <xti.h>
#endif /* !defined(ZOS) */

#include "hysocket.h"
#include "hycomp.h"
// #include "hyport.h"
#if defined(DEBUG)
#define HYSOCKDEBUG(x, err) printf(x, err)
#define HYSOCKDEBUGH(x, err) printf(x, err)
#define HYSOCKDEBUGPRINT(x) printf(x)
#else
#define HYSOCKDEBUG(x, err)
#define HYSOCKDEBUGH(x, err)
#define HYSOCKDEBUGPRINT(x)
#endif
/* the following defines are used to determine how gethostby*_r calls should be handled.*/
/* HOSTENT_DATA_R: if the HOSTENT_DATA structure is used */
#define HOSTENT_DATA_R (defined(AIX))
/* GLIBC_R: uses the GLIBC versions */
#define GLIBC_R (defined(LINUX))
/* ORIGINAL_R: the original gethostby* call is thread safe */
#define ORIGINAL_R defined(FREEBSD)
/* TOFIX: FreeBSD man page says:
 *   Though these functions are thread-safe, still it is recommended to use
 *   the getaddrinfo(3) family of functions, instead.
 */
/* NO_R: gethostby*_r calls do not exist and the normal gethostby* calls are not threadsafe */
#define NO_R (defined(MACOSX) || defined(__CYGWIN__))
/* OTHER_R: everything else */
#define OTHER_R ((!HOSTENT_DATA_R)&&(!GLIBC_R)&&(!ORIGINAL_R)&&(!NO_R))
/* Converts (seconds, microseconds) to milliseconds */
#define TO_MILLIS(sec, microsec) (sec * 1000 + (microsec + 999) / 1000)
/* os types */
typedef int OSSOCKET;           /* as returned by socket() */
typedef struct sockaddr_in OSSOCKADDR;  /* as used by bind() and friends */
typedef struct hostent OSHOSTENT;
typedef fd_set OSFDSET;
typedef struct timeval OSTIMEVAL;
typedef struct linger OSLINGER;
typedef struct sockaddr OSADDR;
typedef struct sockaddr_in6 OSSOCKADDR_IN6;     /* IPv6 */
typedef struct ipv6_mreq OSIPMREQ6;
typedef struct addrinfo OSADDRINFO;     /* IPv6 */
typedef struct sockaddr_storage OSSOCKADDR_STORAGE;     /* IPv6 */
/*
 * Socket Types
 */
#define OSSOCK_ANY			 0      /* for getaddrinfo hints */
#define OSSOCK_STREAM     SOCK_STREAM   /* stream socket */
#define OSSOCK_DGRAM      SOCK_DGRAM    /* datagram socket */
#define OSSOCK_RAW        SOCK_RAW      /* raw-protocol interface */
#define OSSOCK_RDM        SOCK_RDM      /* reliably-delivered message */
#define OSSOCK_SEQPACKET  SOCK_SEQPACKET        /* sequenced packet stream */
#if defined(HOSTENT_DATA_R)
typedef struct hostent_data OSHOSTENT_DATA;
#endif
typedef struct ip_mreq OSIPMREQ;
#define OSSOMAXCONN SOMAXCONN
#define OS_BADSOCKET -1         /* Provide bad socket constant */
/* defines for socket levels */
#define OS_SOL_SOCKET SOL_SOCKET
#define OS_IPPROTO_TCP IPPROTO_TCP
#define OS_IPPROTO_IP IPPROTO_IP
#if defined(IPv6_FUNCTION_SUPPORT)
#define OS_IPPROTO_IPV6 IPPROTO_IPV6
#endif
/* defines for socket options */
#define OS_SO_LINGER SO_LINGER
#define OS_SO_KEEPALIVE SO_KEEPALIVE
#define OS_TCP_NODELAY TCP_NODELAY
#define OS_SO_REUSEADDR SO_REUSEADDR
#define OS_SO_SNDBUF SO_SNDBUF
#define OS_SO_RCVBUF SO_RCVBUF
#define OS_SO_BROADCAST SO_BROADCAST    /* Support datagram broadcasts */
#define OS_SO_OOBINLINE SO_OOBINLINE
#define OS_IP_TOS	 IP_TOS
/* defines for socket options, multicast */
#define OS_MCAST_TTL IP_MULTICAST_TTL
#define OS_MCAST_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
#define OS_MCAST_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP
#define OS_MCAST_INTERFACE IP_MULTICAST_IF
#define OS_MCAST_LOOP IP_MULTICAST_LOOP
#if defined(IPv6_FUNCTION_SUPPORT)
#define OS_MCAST_INTERFACE_2 IPV6_MULTICAST_IF
#if defined(FREEBSD) || defined(MACOSX) || defined(ZOS)
#define OS_IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#define OS_IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#else
#define OS_IPV6_ADD_MEMBERSHIP IPV6_ADD_MEMBERSHIP
#define OS_IPV6_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP
#endif
#endif
/* defines for the unix error constants.  These may be overriden for specific platforms. */
#define HYPORT_ERROR_SOCKET_UNIX_CONNRESET 		ECONNRESET
#define HYPORT_ERROR_SOCKET_UNIX_EAGAIN							EAGAIN
#define HYPORT_ERROR_SOCKET_UNIX_EAFNOSUPPORT			EAFNOSUPPORT
#define HYPORT_ERROR_SOCKET_UNIX_EBADF								EBADF
#define HYPORT_ERROR_SOCKET_UNIX_ECONNRESET					ECONNRESET
#define HYPORT_ERROR_SOCKET_UNIX_EINVAL							EINVAL
#define HYPORT_ERROR_SOCKET_UNIX_EINTR								EINTR
#define HYPORT_ERROR_SOCKET_UNIX_EFAULT							EFAULT
#define HYPORT_ERROR_SOCKET_UNIX_ENOPROTOOPT				ENOPROTOOPT
#define HYPORT_ERROR_SOCKET_UNIX_ENOTCONN						ENOTCONN
#define HYPORT_ERROR_SOCKET_UNIX_EPROTONOSUPPORT		EPROTONOSUPPORT
#define HYPORT_ERROR_SOCKET_UNIX_HOSTNOTFOUND			HOST_NOT_FOUND
#define HYPORT_ERROR_SOCKET_UNIX_ENOBUFS						ENOBUFS
#define HYPORT_ERROR_SOCKET_UNIX_NODATA							NO_DATA
#define HYPORT_ERROR_SOCKET_UNIX_NORECOVERY				NO_RECOVERY
#define HYPORT_ERROR_SOCKET_UNIX_ENOTSOCK					ENOTSOCK
#define HYPORT_ERROR_SOCKET_UNIX_TRYAGAIN						TRY_AGAIN
#define HYPORT_ERROR_SOCKET_UNIX_EOPNOTSUP				EOPNOTSUPP
#define HYPORT_ERROR_SOCKET_UNIX_ETIMEDOUT				ETIMEDOUT
#define HYPORT_ERROR_SOCKET_UNIX_CONNREFUSED				ECONNREFUSED
#define HYPORT_ERROR_SOCKET_UNIX_EINPROGRESS				EINPROGRESS
#define HYPORT_ERROR_SOCKET_UNIX_ENETUNREACH				ENETUNREACH
#define HYPORT_ERROR_SOCKET_UNIX_EACCES						EACCES
/* platform constants */
#define HYSOCK_MAXCONN OSSOMAXCONN;
#define HYSOCK_BADSOCKET OS_BADSOCKET   /* Provide bad socket constant */
/* defines added for IPv6 */
#define OS_AF_INET4 AF_INET
#define OS_AF_UNSPEC AF_UNSPEC
#define OS_PF_UNSPEC PF_UNSPEC
#define OS_PF_INET4 PF_INET
#define OS_INET4_ADDRESS_LENGTH INET_ADDRSTRLEN
#if defined(NI_MAXHOST)
#define OSNIMAXHOST NI_MAXHOST
#define OSNIMAXSERV NI_MAXSERV
#else
#define OSNIMAXHOST 1025
#define OSNIMAXSERV 32
#endif

#if defined(AF_INET6)
#define OS_AF_INET6 AF_INET6
#define OS_PF_INET6 PF_INET6
#define OS_INET6_ADDRESS_LENGTH INET6_ADDRSTRLEN
#else
#define OS_AF_INET6 -1
#define OS_PF_INET6 -1
#define OS_INET6_ADDRESS_LENGTH 46
#endif
/*platform structs */
typedef struct hysocket_struct
{
  OSSOCKET sock;
  U_16 family;
} hysocket_struct;
/* The sockets are now structs  */
#if defined(NO_LVALUE_CASTING)
#define SOCKET_CAST(x) ((struct hysocket_struct*)  x)->sock
#else
#define SOCKET_CAST(x) ((struct hysocket_struct*)  x)->sock
#endif
typedef struct hysockaddr_struct
{
#if defined(IPv6_FUNCTION_SUPPORT)
  OSSOCKADDR_STORAGE addr;
#else
  OSSOCKADDR addr;
#endif
} hysockaddr_struct;
typedef struct hyhostent_struct
{
  OSHOSTENT *entity;
} hyhostent_struct;
typedef struct hyfdset_struct
{
  OSFDSET handle;
} hyfdset_struct;
typedef struct hytimeval_struct
{
  OSTIMEVAL time;
} hytimeval_struct;
typedef struct hylinger_struct
{
  OSLINGER linger;
} hylinger_struct;
typedef struct hyipmreq_struct
{
  int addrpair;
} hyipmreq_struct;
#define GET_HOST_BUFFER_SIZE 512
/* The gethostBuffer is allocated bufferSize + EXTRA_SPACE, while gethostby*_r is only aware of bufferSize
 * because there seems to be a bug on Linux 386 where gethostbyname_r writes past the end of the
 * buffer.  This bug has not been observed on other platforms, but EXTRA_SPACE is added anyway as a precaution.*/
#define EXTRA_SPACE 128
/*size is 16 because an IP string is "xxx.xxx.xxx.xxx" + 1 null character */
#define NTOA_SIZE 16
/** new structure for IPv6 mulitcast requests */
typedef struct hyipv6_mreq_struct
{
#if defined(IPv6_FUNCTION_SUPPORT)
  OSIPMREQ6 mreq;
#else
  int dummy;
#endif
} hyipv6_mreq_struct;
/* structure for returning either and IPV4 or IPV6 ip address */
typedef struct hyipAddress_struct
{
  union
  {
#if defined(IPv6_FUNCTION_SUPPORT)
    U_8 bytes[sizeof (struct in6_addr)];
#else
    U_8 bytes[sizeof (struct in_addr)];
#endif
struct in_addr inAddr;
#if defined(IPv6_FUNCTION_SUPPORT)
    struct in6_addr in6Addr;
#endif
} addr;
  U_32 length;
  U_32 scope;
} hyipAddress_struct;
/* structure for returning network interface information */
typedef struct hyNetworkInterface_struct
{
  char *name;
  char *displayName;
  U_32 numberAddresses;
  U_32 index;
  struct hyipAddress_struct *addresses;
} hyNetworkInterface_struct;
/* array of network interface structures */
typedef struct hyNetworkInterfaceArray_struct
{
  U_32 length;
  struct hyNetworkInterface_struct *elements;
} hyNetworkInterfaceArray_struct;
/** new structure for IPv6 addrinfo will either point to a hostent or 
	an addr info depending on the IPv6 support for this OS */
typedef struct hyaddrinfo_struct
{
  void *addr_info;
  int length;
} hyaddrinfo_struct;
#endif

