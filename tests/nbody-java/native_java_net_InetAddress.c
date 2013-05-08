
#include "xmlvm.h"
#include "java_net_InetAddress.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include "xmlvm-util.h"
#include "xmlvm-sock.h"
#include "java_net_UnknownHostException.h"
//XMLVM_END_NATIVE_IMPLEMENTATION

void java_net_InetAddress_oneTimeInitialization___boolean(JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_oneTimeInitialization___boolean]
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_net_InetAddress_getAliasesByNameImpl___java_lang_String(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_getAliasesByNameImpl___java_lang_String]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY]
    org_xmlvm_runtime_XMLVMArray* addr = n1;
    I_32 result = 0;
    hysockaddr_struct in_addr;
    U_32 length;
    char hostName[OSNIMAXHOST];
    JAVA_ARRAY_BYTE ipaddr[HYSOCK_INADDR6_LEN];
    int address_family = HYADDR_FAMILY_AFINET4;
    length = addr->fields.org_xmlvm_runtime_XMLVMArray.length_;
    
    /* If it's a valid length for an IP address then do the work */
    if (length == HYSOCK_INADDR6_LEN || length == HYSOCK_INADDR_LEN)
    {
        
        if (length == HYSOCK_INADDR6_LEN)
        {
            address_family = HYADDR_FAMILY_AFINET6;
        }
        
        JAVA_ARRAY_BYTE* addrData = (JAVA_ARRAY_BYTE*) addr->fields.org_xmlvm_runtime_XMLVMArray.array_;
        XMLVM_MEMCPY(ipaddr, addrData, length);
        hysock_sockaddr_init6 (&in_addr, (U_8 *) ipaddr, length,
                               (I_16) address_family, 0, 0, 0, NULL);
        result =
        hysock_getnameinfo (&in_addr, sizeof (in_addr.addr), hostName,
                            OSNIMAXHOST, 0);
        if (0 == result)
        {
            return newJavaNetInetAddressGenericBS (ipaddr, length, hostName, 0);
        }
    }

    XMLVM_THROW_WITH_CSTRING(java_net_UnknownHostException, netLookupErrorString(result))
    
    return JAVA_NULL;
    //XMLVM_END_NATIVE
}

JAVA_INT java_net_InetAddress_inetAddrImpl___java_lang_String(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_inetAddrImpl___java_lang_String]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_net_InetAddress_inetNtoaImpl___int(JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_inetNtoaImpl___int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_net_InetAddress_getHostByNameImpl___java_lang_String_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_getHostByNameImpl___java_lang_String_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_net_InetAddress_getHostNameImpl__()
{
    //XMLVM_BEGIN_NATIVE[java_net_InetAddress_getHostNameImpl__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

