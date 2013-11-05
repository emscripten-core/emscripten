#ifndef __JAVA_NET_INET6ADDRESS__
#define __JAVA_NET_INET6ADDRESS__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_Inet6Address 0
// Implemented interfaces:
// Super Class:
#include "java_net_InetAddress.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
XMLVM_FORWARD_DECL(java_io_ObjectStreamField)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Boolean
#define XMLVM_FORWARD_DECL_java_lang_Boolean
XMLVM_FORWARD_DECL(java_lang_Boolean)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_net_UnknownHostException
#define XMLVM_FORWARD_DECL_java_net_UnknownHostException
XMLVM_FORWARD_DECL(java_net_UnknownHostException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.net.Inet6Address
XMLVM_DEFINE_CLASS(java_net_Inet6Address, 17, XMLVM_ITABLE_SIZE_java_net_Inet6Address)

extern JAVA_OBJECT __CLASS_java_net_Inet6Address;
extern JAVA_OBJECT __CLASS_java_net_Inet6Address_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_Inet6Address_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_Inet6Address_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_Inet6Address
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_Inet6Address \
    __INSTANCE_FIELDS_java_net_InetAddress; \
    struct { \
        JAVA_INT scope_id_; \
        JAVA_BOOLEAN scope_id_set_; \
        JAVA_BOOLEAN scope_ifname_set_; \
        JAVA_OBJECT ifname_; \
        JAVA_OBJECT scopedIf_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_Inet6Address \
    } java_net_Inet6Address

struct java_net_Inet6Address {
    __TIB_DEFINITION_java_net_Inet6Address* tib;
    struct {
        __INSTANCE_FIELDS_java_net_Inet6Address;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_Inet6Address
#define XMLVM_FORWARD_DECL_java_net_Inet6Address
typedef struct java_net_Inet6Address java_net_Inet6Address;
#endif

#define XMLVM_VTABLE_SIZE_java_net_Inet6Address 17
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMulticastAddress__ 15
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isAnyLocalAddress__ 7
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isLoopbackAddress__ 9
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isLinkLocalAddress__ 8
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isSiteLocalAddress__ 16
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMCGlobal__ 10
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMCNodeLocal__ 12
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMCLinkLocal__ 11
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMCSiteLocal__ 14
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_isMCOrgLocal__ 13
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_getHostAddress__ 6
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_net_Inet6Address_toString__ 5

void __INIT_java_net_Inet6Address();
void __INIT_IMPL_java_net_Inet6Address();
void __DELETE_java_net_Inet6Address(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_Inet6Address(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_Inet6Address();
JAVA_OBJECT __NEW_INSTANCE_java_net_Inet6Address();
JAVA_LONG java_net_Inet6Address_GET_serialVersionUID();
void java_net_Inet6Address_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_net_Inet6Address_GET_any_bytes();
void java_net_Inet6Address_PUT_any_bytes(JAVA_OBJECT v);
JAVA_OBJECT java_net_Inet6Address_GET_localhost_bytes();
void java_net_Inet6Address_PUT_localhost_bytes(JAVA_OBJECT v);
JAVA_OBJECT java_net_Inet6Address_GET_ANY();
void java_net_Inet6Address_PUT_ANY(JAVA_OBJECT v);
JAVA_OBJECT java_net_Inet6Address_GET_LOOPBACK();
void java_net_Inet6Address_PUT_LOOPBACK(JAVA_OBJECT v);
JAVA_OBJECT java_net_Inet6Address_GET_serialPersistentFields();
void java_net_Inet6Address_PUT_serialPersistentFields(JAVA_OBJECT v);
void java_net_Inet6Address___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3);
JAVA_OBJECT java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3);
JAVA_OBJECT java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_java_net_NetworkInterface(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_BOOLEAN java_net_Inet6Address_compareLocalType___java_net_Inet6Address(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_net_Inet6Address___INIT____byte_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 15
JAVA_BOOLEAN java_net_Inet6Address_isMulticastAddress__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_net_Inet6Address_isAnyLocalAddress__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_BOOLEAN java_net_Inet6Address_isLoopbackAddress__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_net_Inet6Address_isLinkLocalAddress__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_BOOLEAN java_net_Inet6Address_isSiteLocalAddress__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_net_Inet6Address_isMCGlobal__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_BOOLEAN java_net_Inet6Address_isMCNodeLocal__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_net_Inet6Address_isMCLinkLocal__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_BOOLEAN java_net_Inet6Address_isMCSiteLocal__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_BOOLEAN java_net_Inet6Address_isMCOrgLocal__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT java_net_Inet6Address_getHostAddress__(JAVA_OBJECT me);
JAVA_INT java_net_Inet6Address_getScopeId__(JAVA_OBJECT me);
JAVA_OBJECT java_net_Inet6Address_getScopedInterface__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_net_Inet6Address_hashCode__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_net_Inet6Address_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_net_Inet6Address_isIPv4CompatibleAddress__(JAVA_OBJECT me);
void java_net_Inet6Address_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_net_Inet6Address_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_net_Inet6Address_toString__(JAVA_OBJECT me);
void java_net_Inet6Address___CLINIT_();

#endif
