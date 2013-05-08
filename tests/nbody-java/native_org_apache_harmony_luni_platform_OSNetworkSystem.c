
#include "xmlvm.h"
#include "org_apache_harmony_luni_platform_OSNetworkSystem.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION

#include <fcntl.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <sys/ioctl.h>
#include "poll.h"
#include "java_io_FileDescriptor.h"
#include "java_net_SocketException.h"
#include "java_net_BindException.h"
#include "java_net_ConnectException.h"
#include "java_lang_Thread.h"
#include "xmlvm-sock.h"
#include "java_io_InterruptedIOException.h"


void createSocket (JAVA_OBJECT thisObjFD, int sockType, BOOLEAN preferIPv4Stack)
{
    I_32 result;
    hysocket_t sockdesc;
    int family = HYADDR_FAMILY_AFINET4;
    int supportsIPv6 = 0; //HARMONY_CACHE_GET (env, harmony_supports_ipv6);
    
    if (supportsIPv6 && !(preferIPv4Stack))
    {
        /* We are creating a server socket on the any address */
        family = HYADDR_FAMILY_UNSPEC;
    }
    
    result = hysock_socket (&sockdesc, family, sockType, HYSOCK_DEFPROTOCOL);
    if (0 != result)
    {
        /* ok now if we tried to create an IPv6 socket and it failed it could be that the
         platform does not have IPv6 enabled.  In this case we should revert back to 
         creating an IPv4 socket */
        if (HYADDR_FAMILY_UNSPEC == family)
        {
            /* now try to create an IPv4 socket */
            family = HYADDR_FAMILY_AFINET4;
            result = hysock_socket (&sockdesc, family, sockType, HYSOCK_DEFPROTOCOL);
        }
        
        if (0 != result)
        {
            throwJavaNetSocketException (result);
        }
    }
    
    if (0 == result)
    {
        setJavaIoFileDescriptorContents (thisObjFD, sockdesc);
    }
}


int selectRead(hysocket_t hysocketP, I_32 uSecTime, BOOLEAN accept)
{
    I_32 result = 0;
    I_32 timeout;
    struct pollfd my_pollfd;
    
    timeout = uSecTime >= 0 ? TO_MILLIS(0, uSecTime) : -1;
    my_pollfd.fd = hysocketP->sock;
    my_pollfd.events = POLLIN | POLLPRI;
    my_pollfd.revents = 0;
    do {
        result = poll (&my_pollfd, 1, timeout);
    } while (result == -1 && errno == EINTR);
    
    if (result == 0)
        return HYPORT_ERROR_SOCKET_TIMEOUT;
    
    if (result == -1)
        return HYPORT_ERROR_SOCKET_OPFAILED;
    
    return result;
}



UDATA hytime_msec_clock ()
{
    struct timeval tp;
    struct timezone tzp;
    
    gettimeofday (&tp, &tzp);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}


int pollSelectRead (JAVA_OBJECT fileDescriptor, JAVA_INT timeout, BOOLEAN poll)
{
    
    I_32 result;
    hysocket_t hysocketP;
    
    //PORT_ACCESS_FROM_ENV (env);
    
    if (!poll) {
        UDATA finishTime;
        
        /* 
         * A zero timeout means wait forever. If not polling, return success
         * and call receive() or accept() to block. 
         */
        
        if (!timeout) {
            return 0;
        }
        
        finishTime = hytime_msec_clock() + (UDATA) timeout;
        
    SELECT_NOPOLL:
        
        hysocketP = getJavaIoFileDescriptorContentsAsAPointer (fileDescriptor);
        
        if (!hysock_socketIsValid (hysocketP)) {
            throwJavaNetSocketException (HYPORT_ERROR_SOCKET_BADSOCKET);
            return - 1;
        }
        
        result = hysock_select_read (hysocketP, timeout / 1000,
                                     (timeout % 1000) * 1000, FALSE);
        
        /*
         *  if we time out, then throw the InterruptedIO exception
         *  which gets converted by a caller into the appropriate thing
         * 
         *  if we are interrupted, recalculate our timeout and if we 
         *  have time left or 0, try again.  If no time lest, throw InterruptedIO
         *  Exception
         * 
         *  If some other error, just throw exceptionand bail
         */
        if (HYPORT_ERROR_SOCKET_TIMEOUT == result) {
            XMLVM_THROW_WITH_CSTRING(java_io_InterruptedIOException, netLookupErrorString(result))
        }
        else if (HYPORT_ERROR_SOCKET_INTERRUPTED == result) {
            
            timeout = finishTime - hytime_msec_clock();
            
            if (timeout < 0) {
                XMLVM_THROW_WITH_CSTRING(java_io_InterruptedIOException, netLookupErrorString(result))
            }
            else { 
                goto SELECT_NOPOLL;
            }
        }
        else if (0 > result) {
            XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
        }
    }
    else  /* we are polling */
    {
        I_32 pollTimeoutUSec = 100000, pollMsec = 100;
        UDATA finishTime = 0;
        IDATA timeLeft = timeout;
        BOOLEAN hasTimeout = timeout > 0;
        
        if (hasTimeout) {
            finishTime = hytime_msec_clock () + (UDATA) timeout;
        }
        
    select:
        
        /* 
         * Fetch the handle every time in case the socket is closed.
         */
        
        hysocketP =
        getJavaIoFileDescriptorContentsAsAPointer (fileDescriptor);
        
        if (!hysock_socketIsValid (hysocketP))
        {
            XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_INTERRUPTED))
            return - 1;
        }
        
        if (hasTimeout)
        {
            if (timeLeft - 10 < pollMsec) {
                pollTimeoutUSec = timeLeft <= 0 ? 0 : (timeLeft * 1000);
            }
            
            result = hysock_select_read (hysocketP, 0, pollTimeoutUSec, FALSE);
            
            /*
             *  because we are polling at a time smaller than timeout (presumably)
             *  lets treat an interrupt and timeout the same - go see if we're done
             *  timewise, and then just try again if not
             */         
            if (HYPORT_ERROR_SOCKET_TIMEOUT == result ||
                HYPORT_ERROR_SOCKET_INTERRUPTED == result)
            {
                timeLeft = finishTime - hytime_msec_clock ();
                
                if (timeLeft <= 0) {
                    XMLVM_THROW_WITH_CSTRING(java_io_InterruptedIOException, netLookupErrorString(result))
                }                
                else
                {
                    goto select;
                }
            }   
            else if (0 > result) {
                XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
            }
        }
        else  /* polling with no timeout (why would you do this?)*/
        {
            result = hysock_select_read (hysocketP, 0, pollTimeoutUSec, FALSE);
            
            /* 
             *  if interrupted (or a timeout) just retry
             */
            if (HYPORT_ERROR_SOCKET_TIMEOUT == result ||
                HYPORT_ERROR_SOCKET_INTERRUPTED == result)
            {
                goto select;
            }
            else if (0 > result) {
                XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
            }
        }
    }
    
    return result;
}


void updateSocket(hysockaddr_t sockaddrP, hysocket_t socketNew, JAVA_OBJECT socketImpl, JAVA_OBJECT fileDescriptorSocketImpl)
{
    //PORT_ACCESS_FROM_ENV(env);
    U_8 nipAddress[HYSOCK_INADDR6_LEN];
    U_32 length;
    JAVA_OBJECT anInetAddress;
    U_16 nPort;
    U_32 scope_id = 0;
    
    //hysock_sockaddr_address6(sockaddrP, nipAddress, &length, &scope_id);
    //nPort = hysock_sockaddr_port(sockaddrP);
    //anInetAddress = newJavaNetInetAddressGenericB((jbyte *) nipAddress, length, scope_id);
    
    setJavaIoFileDescriptorContents(fileDescriptorSocketImpl, socketNew);
    //setSocketImplAddress(env, socketImpl, anInetAddress);
    //setSocketImplPort(env, socketImpl, hysock_ntohs(nPort));
}


//XMLVM_END_NATIVE_IMPLEMENTATION

void org_apache_harmony_luni_platform_OSNetworkSystem_accept___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_accept___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_acceptStreamSocket___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_acceptStreamSocket___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int]
    java_io_FileDescriptor* fdServer = n1;
    JAVA_OBJECT socketImpl = n2;
    JAVA_OBJECT fdSocketImpl = n3;
    JAVA_INT timeout = n4;
    
    //PORT_ACCESS_FROM_ENV(env);
    I_32 result;
    hysocket_t socketS, socketNew;
    hysockaddr_struct sockaddrP;
    JAVA_ARRAY_BYTE nlocalAddrBytes[HYSOCK_INADDR6_LEN];
    
select_accept:
    result = pollSelectRead(fdServer, timeout, TRUE);
    if (0 > result) {
        return;
    }
    
    socketS = getJavaIoFileDescriptorContentsAsAPointer(fdServer);
    if (!hysock_socketIsValid(socketS)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return;
    }
    
    hysock_sockaddr_init6(&sockaddrP, (U_8 *) nlocalAddrBytes,
                          HYSOCK_INADDR_LEN, HYADDR_FAMILY_AFINET4, 0, 0, 0,
                          socketS);
    
    result = hysock_accept(socketS, &sockaddrP, &socketNew);
    if (0 != result) {
        // repeat accept if the server was reset
        if (errno == ECONNABORTED) {
            goto select_accept;
        } else {
            XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
            return;
        }
    }
    
    updateSocket(&sockaddrP, socketNew, socketImpl, fdSocketImpl);
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_availableStream___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_availableStream___java_io_FileDescriptor]
    java_io_FileDescriptor* fileDescriptor = (java_io_FileDescriptor*) n1;
    
    hysocket_t hysocketP;
    U_32 nbytes = 0;
    I_32 result;
    
    hysocketP = getJavaIoFileDescriptorContentsAsAPointer(fileDescriptor);
    if (!hysock_socketIsValid(hysocketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return (JAVA_INT) 0;
    }
    
    result = ioctl(hysocketP->sock, FIONREAD, &nbytes);
    if (result != 0) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result));
        return (JAVA_INT) 0;
    }
    
    return (JAVA_INT) nbytes;
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int]
    JAVA_ARRAY_BYTE nlocalAddrBytes[HYSOCK_INADDR6_LEN];
    int length;
    U_16 nPort;
    I_32 result;
    hysocket_t socketP;
    hysockaddr_struct sockaddrP;
    U_32 scope_id = 0;
    java_io_FileDescriptor* fileDescriptor = (java_io_FileDescriptor*) n1;
    java_net_InetAddress* inetAddress = n2;
    JAVA_INT localPort = n3;
    
    socketP = getJavaIoFileDescriptorContentsAsAPointer(fileDescriptor);
    if (!hysock_socketIsValid(socketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return;
    } else {
        netGetJavaNetInetAddressValue(inetAddress, (U_8*) nlocalAddrBytes, (U_32*) &length);
        
        nPort = hysock_htons((U_16) localPort);
        if (length == HYSOCK_INADDR6_LEN) {
            netGetJavaNetInetAddressScopeId(inetAddress, &scope_id);
            hysock_sockaddr_init6(&sockaddrP, (U_8 *) nlocalAddrBytes, length,
                                  HYADDR_FAMILY_AFINET6, nPort, 0, scope_id,
                                  socketP);
        } else {
            hysock_sockaddr_init6(&sockaddrP, (U_8 *) nlocalAddrBytes, length,
                                  HYADDR_FAMILY_AFINET4, nPort, 0, scope_id,
                                  socketP);
        }
        result = hysock_bind(socketP, &sockaddrP);
        if (0 != result) {
            XMLVM_THROW_WITH_CSTRING(java_net_BindException, netLookupErrorString(result))
            return;
        }
    }
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_connect___java_io_FileDescriptor_int_java_net_InetAddress_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_connect___java_io_FileDescriptor_int_java_net_InetAddress_int]
    U_8 nAddrBytes[HYSOCK_INADDR6_LEN];
    U_32 length = 0;
    U_16 nPort;
    I_32 result;
    hysocket_t socketP;
    hysockaddr_struct sockaddrP;
    U_32 scope_id = 0;
    
    java_io_FileDescriptor* fd = n1;
    JAVA_INT trafficClass = n2;
    java_net_InetAddress* inetAddress = n3;
    JAVA_INT remotePort = n4;
    
    socketP = getJavaIoFileDescriptorContentsAsAPointer(fd);
    if (!hysock_socketIsValid(socketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return -1;
    }
    
    netGetJavaNetInetAddressValue(inetAddress, nAddrBytes, &length);
    nPort = hysock_htons((U_16) remotePort);
    
    if (length == HYSOCK_INADDR_LEN) {
        // IPv4
        hysock_sockaddr_init6(&sockaddrP, nAddrBytes, length,
                              HYADDR_FAMILY_AFINET4, nPort, 0, 0, socketP);
    } else {
        // IPv6
        netGetJavaNetInetAddressScopeId(inetAddress, &scope_id);
        hysock_sockaddr_init6(&sockaddrP, nAddrBytes, length,
                              HYADDR_FAMILY_AFINET6, nPort,
                              (trafficClass & 0xFF) << 20, scope_id, socketP);
    }
    
    result = hysock_connect(socketP, &sockaddrP);
    if (0 != result) {
        XMLVM_THROW_WITH_CSTRING(java_net_ConnectException, netLookupErrorString(result))
        return result;
    }
    
    return result;
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_connectDatagram___java_io_FileDescriptor_int_int_java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_OBJECT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_connectDatagram___java_io_FileDescriptor_int_int_java_net_InetAddress]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_INT n4, JAVA_OBJECT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_connectWithTimeout___java_io_FileDescriptor_int_int_java_net_InetAddress_int_int_java_lang_Long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_OBJECT n4, JAVA_INT n5, JAVA_INT n6, JAVA_OBJECT n7)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_connectWithTimeout___java_io_FileDescriptor_int_int_java_net_InetAddress_int_int_java_lang_Long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_createDatagramSocket___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_createDatagramSocket___java_io_FileDescriptor_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_createServerStreamSocket___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_createServerStreamSocket___java_io_FileDescriptor_boolean]
    hysocket_t socketP;
    BOOLEAN value = TRUE;
    
    createSocket(n1, HYSOCK_STREAM, n2);
    
    /* Also sets HY_SO_REUSEADDR = TRUE on Linux only */
    socketP = (hysocket_t) getJavaIoFileDescriptorContentsAsAPointer(n1);
    hysock_setopt_bool (socketP, HY_SOL_SOCKET, HY_SO_REUSEADDR, &value);
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean]
    createSocket(n1, HYSOCK_STREAM, n2);
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_disconnectDatagram___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_disconnectDatagram___java_io_FileDescriptor]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSNetworkSystem_getHostByAddr___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getHostByAddr___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSNetworkSystem_getHostByName___java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getHostByName___java_lang_String_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_getSocketFlags__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getSocketFlags__]
    return 0;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalAddress___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalAddress___java_io_FileDescriptor_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalPort___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalPort___java_io_FileDescriptor_boolean]
    I_32 result;
    hysockaddr_struct sockaddrP;
    U_16 nPort, hPort;
    java_io_FileDescriptor* fileDescriptor = n1;
    JAVA_BOOLEAN preferIPv6Addresses = n2;
    
    result = netGetSockAddr(fileDescriptor, &sockaddrP, preferIPv6Addresses);
    if (0 != result) {
        /* The java spec does not indicate any exceptions on this call */
        return (JAVA_INT) 0;
    }
    
    nPort = hysock_sockaddr_port(&sockaddrP);
    hPort = hysock_ntohs(nPort);
    return (JAVA_INT) hPort;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSNetworkSystem_getSocketOption___java_io_FileDescriptor_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_getSocketOption___java_io_FileDescriptor_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSNetworkSystem_inheritedChannel__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_inheritedChannel__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_isReachableByICMPImpl___java_net_InetAddress_java_net_InetAddress_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_isReachableByICMPImpl___java_net_InetAddress_java_net_InetAddress_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_listenStreamSocket___java_io_FileDescriptor_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_listenStreamSocket___java_io_FileDescriptor_int]
    java_io_FileDescriptor* fd = n1;
    JAVA_INT backlog = n2;
    
    hysocket_t socketP;
    I_32 result;
    
    socketP = getJavaIoFileDescriptorContentsAsAPointer(fd);
    
    if (!hysock_socketIsValid(socketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return;
    }
        
    result = hysock_listen(socketP, (I_32) backlog);
    if (result < 0) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
        return;
    }
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_oneTimeInitializationImpl___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_oneTimeInitializationImpl___boolean]
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_peekDatagram___java_io_FileDescriptor_java_net_InetAddress_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_peekDatagram___java_io_FileDescriptor_java_net_InetAddress_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_read___java_io_FileDescriptor_byte_1ARRAY_int_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_read___java_io_FileDescriptor_byte_1ARRAY_int_int_int]
    JAVA_ARRAY_BYTE* message;
    JAVA_INT result;
    
    java_io_FileDescriptor* fd = n1;
    org_xmlvm_runtime_XMLVMArray* data = n2;
    JAVA_INT offset = n3;
    JAVA_INT count = n4;
    JAVA_INT timeout = n5;
    
    /* Get a pointer to the start of the bytearray */
    message = data->fields.org_xmlvm_runtime_XMLVMArray.array_;
    
    /* Read directly into the byte array */
    result =
    org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int
            (me, fd, (JAVA_LONG) (message + offset), count, timeout);
    
    return result;
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int]
    java_io_FileDescriptor* fd = n1;
    JAVA_LONG address = n2;
    JAVA_INT count = n3;
    JAVA_INT timeout = n4;
    
    hysocket_t hysocketP;
    U_8 *message = (U_8 *) address;
    I_32 result, localCount;
    
    hysocketP = getJavaIoFileDescriptorContentsAsAPointer(fd);
    if (!hysock_socketIsValid(hysocketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return (JAVA_INT) 0;
    }
    
    /* A non-zero timeout will first check, and potentially wait, to see if any
     * bytes are available
     */
    if (timeout != 0) {
        result = selectRead(hysocketP, timeout * 1000, FALSE);
        if (0 > result) {
            if (result == HYPORT_ERROR_SOCKET_TIMEOUT) {
                return (JAVA_INT) 0;  // return zero bytes to indicate timeout
            }

            XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
            return (JAVA_INT) 0;  // Unused, exception takes precedence
        }
    }
    
    /* Limit size of read to 64k bytes */
    localCount = (count < 65536) ? count : 65536;
    result = hysock_read(hysocketP, message, localCount, HYSOCK_NOFLAGS);
    if (0 > result) {
        if (HYPORT_ERROR_SOCKET_WOULDBLOCK == result) {
            /* We were asked to read on a nonblocking socket and there is no data available */
            return (JAVA_INT) 0;
        }

        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
        return (JAVA_INT) 0;
    }
    
    /* If no bytes are read, return -1 to signal 'endOfFile' to the Java input stream */
    return (0 == result) ? (JAVA_INT) -1 : (JAVA_INT) result;
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5, JAVA_INT n6, JAVA_BOOLEAN n7)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_LONG n3, JAVA_INT n4, JAVA_INT n5, JAVA_INT n6, JAVA_BOOLEAN n7)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_receiveStream___java_io_FileDescriptor_byte_1ARRAY_int_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_receiveStream___java_io_FileDescriptor_byte_1ARRAY_int_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5, JAVA_INT n6, JAVA_BOOLEAN n7)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_LONG n3, JAVA_INT n4, JAVA_INT n5, JAVA_INT n6, JAVA_BOOLEAN n7)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_selectImpl___java_io_FileDescriptor_1ARRAY_java_io_FileDescriptor_1ARRAY_int_int_int_1ARRAY_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_OBJECT n5, JAVA_LONG n6)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_selectImpl___java_io_FileDescriptor_1ARRAY_java_io_FileDescriptor_1ARRAY_int_int_int_1ARRAY_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_BOOLEAN n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagramDirect___java_io_FileDescriptor_long_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2, JAVA_INT n3, JAVA_INT n4, JAVA_BOOLEAN n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagramDirect___java_io_FileDescriptor_long_int_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_int_boolean_int_java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5, JAVA_BOOLEAN n6, JAVA_INT n7, JAVA_OBJECT n8)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_int_boolean_int_java_net_InetAddress]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram2___java_io_FileDescriptor_byte_1ARRAY_int_int_int_java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5, JAVA_OBJECT n6)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram2___java_io_FileDescriptor_byte_1ARRAY_int_int_int_java_net_InetAddress]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagramDirect___java_io_FileDescriptor_long_int_int_int_boolean_int_java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5, JAVA_BOOLEAN n6, JAVA_INT n7, JAVA_OBJECT n8)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagramDirect___java_io_FileDescriptor_long_int_int_int_boolean_int_java_net_InetAddress]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_sendUrgentData___java_io_FileDescriptor_byte(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BYTE n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_sendUrgentData___java_io_FileDescriptor_byte]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_setInetAddress___java_net_InetAddress_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_setInetAddress___java_net_InetAddress_byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_setNonBlocking___java_io_FileDescriptor_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_setNonBlocking___java_io_FileDescriptor_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_setSocketOption___java_io_FileDescriptor_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_setSocketOption___java_io_FileDescriptor_int_java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_shutdownInput___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_shutdownInput___java_io_FileDescriptor]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_shutdownOutput___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_shutdownOutput___java_io_FileDescriptor]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSNetworkSystem_socketClose___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_socketClose___java_io_FileDescriptor]
    hysocket_t socketP;
    I_32 result = 0;
    
    java_io_FileDescriptor* fileDescriptor = n1;
    
    socketP = getJavaIoFileDescriptorContentsAsAPointer(fileDescriptor);
    if (hysock_socketIsValid(socketP)) {
        /* Set the file descriptor before closing so the select polling loop will terminate. */
        /* Some platforms wait in the socket close. */
        fileDescriptor->fields.java_io_FileDescriptor.descriptor_ = -1;
        result = hysock_close(&socketP);
    }
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN org_apache_harmony_luni_platform_OSNetworkSystem_supportsUrgentData___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_supportsUrgentData___java_io_FileDescriptor]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_write___java_io_FileDescriptor_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_write___java_io_FileDescriptor_byte_1ARRAY_int_int]
    JAVA_ARRAY_BYTE *message;
    JAVA_INT result;
    
    java_io_FileDescriptor* fd = n1;
    org_xmlvm_runtime_XMLVMArray* data = n2;
    JAVA_INT offset = n3;
    JAVA_INT count = n4;
    
    /* Get a pointer to the start of the bytearray */
    message = data->fields.org_xmlvm_runtime_XMLVMArray.array_;
    
    /* Write directly from the byte array */
    result = org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int
            (me, fd, (JAVA_LONG) (message + offset), count);
    
    return result;
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int]
    java_io_FileDescriptor* fd = n1;
    JAVA_LONG address = n2;
    JAVA_INT count = n3;
    
    JAVA_ARRAY_BYTE *message = (JAVA_ARRAY_BYTE*) address;
    I_32 result;
    
    hysocket_t socketP = getJavaIoFileDescriptorContentsAsAPointer(fd);
    if (!hysock_socketIsValid(socketP)) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(HYPORT_ERROR_SOCKET_BADSOCKET))
        return (JAVA_INT) 0;
    }
    
    result = hysock_write(socketP, (U_8 *) message, (I_32) count, HYSOCK_NOFLAGS);
    if (0 > result) {
        XMLVM_THROW_WITH_CSTRING(java_net_SocketException, netLookupErrorString(result))
        return (JAVA_INT) 0;
    }
    
    return (JAVA_INT) result;
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSNetworkSystem_writev___java_io_FileDescriptor_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSNetworkSystem_writev___java_io_FileDescriptor_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_apache_harmony_luni_platform_OSNetworkSystem()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_accept___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_accept___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_accept___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_acceptStreamSocket___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_acceptStreamSocket___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_acceptStreamSocket___java_io_FileDescriptor_java_net_SocketImpl_java_io_FileDescriptor_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_availableStream___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_availableStream___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_availableStream___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connect___java_io_FileDescriptor_int_java_net_InetAddress_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connect___java_io_FileDescriptor_int_java_net_InetAddress_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_connect___java_io_FileDescriptor_int_java_net_InetAddress_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectDatagram___java_io_FileDescriptor_int_int_java_net_InetAddress
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectDatagram___java_io_FileDescriptor_int_int_java_net_InetAddress] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_connectDatagram___java_io_FileDescriptor_int_int_java_net_InetAddress;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectWithTimeout___java_io_FileDescriptor_int_int_java_net_InetAddress_int_int_java_lang_Long
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_connectWithTimeout___java_io_FileDescriptor_int_int_java_net_InetAddress_int_int_java_lang_Long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_connectWithTimeout___java_io_FileDescriptor_int_int_java_net_InetAddress_int_int_java_lang_Long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createDatagramSocket___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createDatagramSocket___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_createDatagramSocket___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createServerStreamSocket___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createServerStreamSocket___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_createServerStreamSocket___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_disconnectDatagram___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_disconnectDatagram___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_disconnectDatagram___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getHostByAddr___byte_1ARRAY
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getHostByAddr___byte_1ARRAY] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getHostByAddr___byte_1ARRAY;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getHostByName___java_lang_String_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getHostByName___java_lang_String_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getHostByName___java_lang_String_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketFlags__
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketFlags__] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getSocketFlags__;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalAddress___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalAddress___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalAddress___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalPort___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalPort___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getSocketLocalPort___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketOption___java_io_FileDescriptor_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_getSocketOption___java_io_FileDescriptor_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_getSocketOption___java_io_FileDescriptor_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_inheritedChannel__
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_inheritedChannel__] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_inheritedChannel__;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_listenStreamSocket___java_io_FileDescriptor_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_listenStreamSocket___java_io_FileDescriptor_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_listenStreamSocket___java_io_FileDescriptor_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_peekDatagram___java_io_FileDescriptor_java_net_InetAddress_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_peekDatagram___java_io_FileDescriptor_java_net_InetAddress_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_peekDatagram___java_io_FileDescriptor_java_net_InetAddress_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_read___java_io_FileDescriptor_byte_1ARRAY_int_int_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_read___java_io_FileDescriptor_byte_1ARRAY_int_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_read___java_io_FileDescriptor_byte_1ARRAY_int_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_readDirect___java_io_FileDescriptor_long_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_receiveDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveStream___java_io_FileDescriptor_byte_1ARRAY_int_int_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_receiveStream___java_io_FileDescriptor_byte_1ARRAY_int_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_receiveStream___java_io_FileDescriptor_byte_1ARRAY_int_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagram___java_io_FileDescriptor_java_net_DatagramPacket_byte_1ARRAY_int_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_recvConnectedDatagramDirect___java_io_FileDescriptor_java_net_DatagramPacket_long_int_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagramDirect___java_io_FileDescriptor_long_int_int_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagramDirect___java_io_FileDescriptor_long_int_int_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendConnectedDatagramDirect___java_io_FileDescriptor_long_int_int_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_int_boolean_int_java_net_InetAddress
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_int_boolean_int_java_net_InetAddress] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram___java_io_FileDescriptor_byte_1ARRAY_int_int_int_boolean_int_java_net_InetAddress;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram2___java_io_FileDescriptor_byte_1ARRAY_int_int_int_java_net_InetAddress
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram2___java_io_FileDescriptor_byte_1ARRAY_int_int_int_java_net_InetAddress] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagram2___java_io_FileDescriptor_byte_1ARRAY_int_int_int_java_net_InetAddress;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagramDirect___java_io_FileDescriptor_long_int_int_int_boolean_int_java_net_InetAddress
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagramDirect___java_io_FileDescriptor_long_int_int_int_boolean_int_java_net_InetAddress] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendDatagramDirect___java_io_FileDescriptor_long_int_int_int_boolean_int_java_net_InetAddress;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendUrgentData___java_io_FileDescriptor_byte
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_sendUrgentData___java_io_FileDescriptor_byte] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_sendUrgentData___java_io_FileDescriptor_byte;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setInetAddress___java_net_InetAddress_byte_1ARRAY
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setInetAddress___java_net_InetAddress_byte_1ARRAY] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_setInetAddress___java_net_InetAddress_byte_1ARRAY;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setNonBlocking___java_io_FileDescriptor_boolean
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setNonBlocking___java_io_FileDescriptor_boolean] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_setNonBlocking___java_io_FileDescriptor_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setSocketOption___java_io_FileDescriptor_int_java_lang_Object
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_setSocketOption___java_io_FileDescriptor_int_java_lang_Object] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_setSocketOption___java_io_FileDescriptor_int_java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_shutdownInput___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_shutdownInput___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_shutdownInput___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_shutdownOutput___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_shutdownOutput___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_shutdownOutput___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_socketClose___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_socketClose___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_socketClose___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_supportsUrgentData___java_io_FileDescriptor
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_supportsUrgentData___java_io_FileDescriptor] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_supportsUrgentData___java_io_FileDescriptor;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_write___java_io_FileDescriptor_byte_1ARRAY_int_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_write___java_io_FileDescriptor_byte_1ARRAY_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_write___java_io_FileDescriptor_byte_1ARRAY_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_writeDirect___java_io_FileDescriptor_long_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_writev___java_io_FileDescriptor_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int
    __TIB_org_apache_harmony_luni_platform_OSNetworkSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSNetworkSystem_writev___java_io_FileDescriptor_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSNetworkSystem_writev___java_io_FileDescriptor_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int;
#endif
}
