/*
 * Copyright (c) 2002-2011 by XMLVM.org
 *
 * Project Info:  http://www.xmlvm.org
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */


#include "hysock.h"


#ifndef __XMLVM_SOCK_H__
#define __XMLVM_SOCK_H__

/* socket structure flags */
#define SOCKET_IPV4_OPEN_MASK '\x1'    /* 00000001 */
#define SOCKET_IPV6_OPEN_MASK '\x2'    /* 00000010 */
#define SOCKET_BOTH_OPEN_MASK '\x3'    /* 00000011 */
#define SOCKET_USE_IPV4_MASK '\x4'    /* 00000100 - this tells which one to pick when doing an operation */

int harmony_supports_ipv6();
int preferIPv4Stack();
int preferIPv6Addresses();

char* netLookupErrorString (I_32 anErrorNum);
U_16 hysock_htons(U_16 port);
U_16 hysock_ntohs (U_16 val);
I_32 hysock_socketIsValid (hysocket_t handle);
U_16 hysock_sockaddr_port (hysockaddr_t handle);
void setJavaIoFileDescriptorContents (JAVA_OBJECT fd, void *value);
void* getJavaIoFileDescriptorContentsAsAPointer (JAVA_OBJECT fd);
void netGetJavaNetInetAddressValue (JAVA_OBJECT anInetAddress, U_8* buffer, U_32* length);
void netGetJavaNetInetAddressScopeId (JAVA_OBJECT anInetAddress, U_32* scope_id);
I_32 netGetSockAddr (JAVA_OBJECT fileDescriptor, hysockaddr_t sockaddrP, JAVA_BOOLEAN preferIPv6Addresses);
JAVA_OBJECT newJavaNetInetAddressGenericBS (JAVA_ARRAY_BYTE* address, U_32 length, const char* hostName, U_32 scope_id);
I_32 hysock_socket (hysocket_t * handle, I_32 family, I_32 socktype, I_32 protocol);
I_32 hysock_sockaddr_init6 (hysockaddr_t handle, U_8 * addr, I_32 addrlength, I_16 family, U_16 nPort, U_32 flowinfo, U_32 scope_id, hysocket_t sock);
I_32 hysock_getnameinfo (hysockaddr_t in_addr, I_32 sockaddr_size, char *name, I_32 name_length, int flags);
I_32 hysock_bind (hysocket_t sock, hysockaddr_t addr);
I_32 hysock_getsockname (hysocket_t handle, hysockaddr_t addrHandle);
I_32 hysock_connect (hysocket_t sock, hysockaddr_t addr);
I_32 hysock_write (hysocket_t sock, U_8 * buf, I_32 nbyte, I_32 flags);
I_32 hysock_read (hysocket_t sock, U_8 * buf, I_32 nbyte, I_32 flags);
I_32 hysock_close (hysocket_t * sock);
I_32 hysock_listen (hysocket_t sock, I_32 backlog);
I_32 hysock_accept (hysocket_t serverSock, hysockaddr_t addrHandle, hysocket_t * sockHandle);
I_32 hysock_timeval_init (U_32 secTime, U_32 uSecTime, hytimeval_t timeP);
I_32 hysock_select_read (hysocket_t hysocketP, I_32 secTime, I_32 uSecTime, BOOLEAN accept);
I_32 hysock_fdset_init (hysocket_t socketP);
I_32 hysock_fdset_size (hysocket_t handle);
I_32 hysock_select (I_32 nfds, hyfdset_t readfds, hyfdset_t writefds, hyfdset_t exceptfds, hytimeval_t timeout);
I_32 platformSocketLevel (I_32 portableSocketLevel);
I_32 platformSocketOption (I_32 portableSocketOption);
I_32 hysock_setopt_bool (hysocket_t socketP, I_32 optlevel, I_32 optname, BOOLEAN * optval);

void throwJavaNetSocketException (I_32 errorNumber);

#endif
