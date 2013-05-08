#ifndef __JAVA_NET_INETADDRESS__
#define __JAVA_NET_INETADDRESS__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_InetAddress 0
// Implemented interfaces:
#include "java_io_Serializable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_FileDescriptor
#define XMLVM_FORWARD_DECL_java_io_FileDescriptor
XMLVM_FORWARD_DECL(java_io_FileDescriptor)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
XMLVM_FORWARD_DECL(java_io_IOException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
XMLVM_FORWARD_DECL(java_io_ObjectStreamField)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Exception
#define XMLVM_FORWARD_DECL_java_lang_Exception
XMLVM_FORWARD_DECL(java_lang_Exception)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_SecurityManager
#define XMLVM_FORWARD_DECL_java_lang_SecurityManager
XMLVM_FORWARD_DECL(java_lang_SecurityManager)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_net_Inet6Address
#define XMLVM_FORWARD_DECL_java_net_Inet6Address
XMLVM_FORWARD_DECL(java_net_Inet6Address)
#endif
#ifndef XMLVM_FORWARD_DECL_java_net_UnknownHostException
#define XMLVM_FORWARD_DECL_java_net_UnknownHostException
XMLVM_FORWARD_DECL(java_net_UnknownHostException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_AccessController
#define XMLVM_FORWARD_DECL_java_security_AccessController
XMLVM_FORWARD_DECL(java_security_AccessController)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_ArrayList
#define XMLVM_FORWARD_DECL_java_util_ArrayList
XMLVM_FORWARD_DECL(java_util_ArrayList)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_StringTokenizer
#define XMLVM_FORWARD_DECL_java_util_StringTokenizer
XMLVM_FORWARD_DECL(java_util_StringTokenizer)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_INetworkSystem
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_INetworkSystem
XMLVM_FORWARD_DECL(org_apache_harmony_luni_platform_INetworkSystem)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_PriviAction)
#endif
// Class declarations for java.net.InetAddress
XMLVM_DEFINE_CLASS(java_net_InetAddress, 17, XMLVM_ITABLE_SIZE_java_net_InetAddress)

extern JAVA_OBJECT __CLASS_java_net_InetAddress;
extern JAVA_OBJECT __CLASS_java_net_InetAddress_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_InetAddress_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_InetAddress_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_InetAddress
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_InetAddress \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT hostName_; \
        JAVA_OBJECT waitReachable_; \
        JAVA_BOOLEAN reached_; \
        JAVA_INT addrCount_; \
        JAVA_INT family_; \
        JAVA_OBJECT ipaddress_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_InetAddress \
    } java_net_InetAddress

struct java_net_InetAddress {
    __TIB_DEFINITION_java_net_InetAddress* tib;
    struct {
        __INSTANCE_FIELDS_java_net_InetAddress;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_InetAddress
#define XMLVM_FORWARD_DECL_java_net_InetAddress
typedef struct java_net_InetAddress java_net_InetAddress;
#endif

#define XMLVM_VTABLE_SIZE_java_net_InetAddress 17
#define XMLVM_VTABLE_IDX_java_net_InetAddress_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_net_InetAddress_getHostAddress__ 6
#define XMLVM_VTABLE_IDX_java_net_InetAddress_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMulticastAddress__ 15
#define XMLVM_VTABLE_IDX_java_net_InetAddress_toString__ 5
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isLoopbackAddress__ 9
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isLinkLocalAddress__ 8
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isSiteLocalAddress__ 16
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMCGlobal__ 10
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMCNodeLocal__ 12
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMCLinkLocal__ 11
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMCSiteLocal__ 14
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isMCOrgLocal__ 13
#define XMLVM_VTABLE_IDX_java_net_InetAddress_isAnyLocalAddress__ 7

void __INIT_java_net_InetAddress();
void __INIT_IMPL_java_net_InetAddress();
void __DELETE_java_net_InetAddress(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_InetAddress(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_InetAddress();
JAVA_OBJECT __NEW_INSTANCE_java_net_InetAddress();
JAVA_OBJECT java_net_InetAddress_GET_any_bytes();
void java_net_InetAddress_PUT_any_bytes(JAVA_OBJECT v);
JAVA_OBJECT java_net_InetAddress_GET_localhost_bytes();
void java_net_InetAddress_PUT_localhost_bytes(JAVA_OBJECT v);
JAVA_OBJECT java_net_InetAddress_GET_ANY();
void java_net_InetAddress_PUT_ANY(JAVA_OBJECT v);
JAVA_OBJECT java_net_InetAddress_GET_NETIMPL();
void java_net_InetAddress_PUT_NETIMPL(JAVA_OBJECT v);
JAVA_OBJECT java_net_InetAddress_GET_LOOPBACK();
void java_net_InetAddress_PUT_LOOPBACK(JAVA_OBJECT v);
JAVA_OBJECT java_net_InetAddress_GET_ERRMSG_CONNECTION_REFUSED();
void java_net_InetAddress_PUT_ERRMSG_CONNECTION_REFUSED(JAVA_OBJECT v);
JAVA_LONG java_net_InetAddress_GET_serialVersionUID();
void java_net_InetAddress_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_net_InetAddress_GET_serialPersistentFields();
void java_net_InetAddress_PUT_serialPersistentFields(JAVA_OBJECT v);
void java_net_InetAddress_oneTimeInitialization___boolean(JAVA_BOOLEAN n1);
void java_net_InetAddress___INIT___(JAVA_OBJECT me);
void java_net_InetAddress___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_net_InetAddress___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_net_InetAddress_cacheElement__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_net_InetAddress_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getAddress__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_getAllByName___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getByName___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 6
JAVA_OBJECT java_net_InetAddress_getHostAddress__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_getHostName__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_getCanonicalHostName__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_getLocalHost__();
// Vtable index: 4
JAVA_INT java_net_InetAddress_hashCode__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_BOOLEAN java_net_InetAddress_isMulticastAddress__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_lookupHostByName___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getAliasesByNameImpl___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_INT java_net_InetAddress_inetAddr___java_lang_String(JAVA_OBJECT n1);
JAVA_INT java_net_InetAddress_inetAddrImpl___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_inetNtoaImpl___int(JAVA_INT n1);
JAVA_OBJECT java_net_InetAddress_getHostByNameImpl___java_lang_String_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2);
JAVA_OBJECT java_net_InetAddress_getHostNameImpl__();
JAVA_OBJECT java_net_InetAddress_getHostNameInternal___java_lang_String_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2);
// Vtable index: 5
JAVA_OBJECT java_net_InetAddress_toString__(JAVA_OBJECT me);
JAVA_BOOLEAN java_net_InetAddress_isHostName___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 9
JAVA_BOOLEAN java_net_InetAddress_isLoopbackAddress__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_net_InetAddress_isLinkLocalAddress__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_BOOLEAN java_net_InetAddress_isSiteLocalAddress__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_net_InetAddress_isMCGlobal__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_BOOLEAN java_net_InetAddress_isMCNodeLocal__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_net_InetAddress_isMCLinkLocal__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_BOOLEAN java_net_InetAddress_isMCSiteLocal__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_BOOLEAN java_net_InetAddress_isMCOrgLocal__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_net_InetAddress_isAnyLocalAddress__(JAVA_OBJECT me);
JAVA_BOOLEAN java_net_InetAddress_isReachable___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_BOOLEAN java_net_InetAddress_isReachable___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_BOOLEAN java_net_InetAddress_isReachableByMultiThread___java_net_NetworkInterface_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4);
JAVA_BOOLEAN java_net_InetAddress_isReachableByICMPUseMultiThread___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_BOOLEAN java_net_InetAddress_isReachableByTCPUseMultiThread___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_BOOLEAN java_net_InetAddress_isReachableByTCP___java_net_InetAddress_java_net_InetAddress_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3);
JAVA_OBJECT java_net_InetAddress_getByAddress___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getByAddress___byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_BOOLEAN java_net_InetAddress_isIPv4MappedAddress___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_getByAddress___java_lang_String_byte_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_net_InetAddress_getByAddressInternal___java_lang_String_byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3);
void java_net_InetAddress_intToBytes___int_byte_1ARRAY_int(JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3);
JAVA_INT java_net_InetAddress_bytesToInt___byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_net_InetAddress_createHostNameFromIPAddress___java_lang_String(JAVA_OBJECT n1);
JAVA_BOOLEAN java_net_InetAddress_preferIPv6Addresses__();
void java_net_InetAddress_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_net_InetAddress_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_net_InetAddress_readResolve__(JAVA_OBJECT me);
JAVA_OBJECT java_net_InetAddress_access$100__();
JAVA_BOOLEAN java_net_InetAddress_access$200___java_net_InetAddress_java_net_InetAddress_java_net_InetAddress_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4);
JAVA_OBJECT java_net_InetAddress_access$300___java_net_InetAddress(JAVA_OBJECT n1);
JAVA_BOOLEAN java_net_InetAddress_access$402___java_net_InetAddress_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2);
JAVA_INT java_net_InetAddress_access$510___java_net_InetAddress(JAVA_OBJECT n1);
JAVA_INT java_net_InetAddress_access$500___java_net_InetAddress(JAVA_OBJECT n1);
void java_net_InetAddress___CLINIT_();

#endif
