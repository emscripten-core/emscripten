#include "xmlvm.h"
#include "java_io_ObjectStreamField.h"
#include "java_lang_Boolean.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_net_UnknownHostException.h"
#include "java_util_Enumeration.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_net_Inet6Address.h"

#define XMLVM_CURRENT_CLASS_NAME Inet6Address
#define XMLVM_CURRENT_PKG_CLASS_NAME java_net_Inet6Address

__TIB_DEFINITION_java_net_Inet6Address __TIB_java_net_Inet6Address = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_net_Inet6Address, // classInitializer
    "java.net.Inet6Address", // className
    "java.net", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_net_InetAddress, // extends
    sizeof(java_net_Inet6Address), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_net_Inet6Address;
JAVA_OBJECT __CLASS_java_net_Inet6Address_1ARRAY;
JAVA_OBJECT __CLASS_java_net_Inet6Address_2ARRAY;
JAVA_OBJECT __CLASS_java_net_Inet6Address_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_net_Inet6Address_serialVersionUID;
static JAVA_OBJECT _STATIC_java_net_Inet6Address_any_bytes;
static JAVA_OBJECT _STATIC_java_net_Inet6Address_localhost_bytes;
static JAVA_OBJECT _STATIC_java_net_Inet6Address_ANY;
static JAVA_OBJECT _STATIC_java_net_Inet6Address_LOOPBACK;
static JAVA_OBJECT _STATIC_java_net_Inet6Address_serialPersistentFields;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_java_net_Inet6Address()
{
    staticInitializerLock(&__TIB_java_net_Inet6Address);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_net_Inet6Address.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_net_Inet6Address.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_net_Inet6Address);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_net_Inet6Address.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_net_Inet6Address.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_net_Inet6Address.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.net.Inet6Address")
        __INIT_IMPL_java_net_Inet6Address();
    }
}

void __INIT_IMPL_java_net_Inet6Address()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_net_InetAddress)
    __TIB_java_net_Inet6Address.newInstanceFunc = __NEW_INSTANCE_java_net_Inet6Address;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_net_Inet6Address.vtable, __TIB_java_net_InetAddress.vtable, sizeof(__TIB_java_net_InetAddress.vtable));
    // Initialize vtable for this class
    __TIB_java_net_Inet6Address.vtable[15] = (VTABLE_PTR) &java_net_Inet6Address_isMulticastAddress__;
    __TIB_java_net_Inet6Address.vtable[7] = (VTABLE_PTR) &java_net_Inet6Address_isAnyLocalAddress__;
    __TIB_java_net_Inet6Address.vtable[9] = (VTABLE_PTR) &java_net_Inet6Address_isLoopbackAddress__;
    __TIB_java_net_Inet6Address.vtable[8] = (VTABLE_PTR) &java_net_Inet6Address_isLinkLocalAddress__;
    __TIB_java_net_Inet6Address.vtable[16] = (VTABLE_PTR) &java_net_Inet6Address_isSiteLocalAddress__;
    __TIB_java_net_Inet6Address.vtable[10] = (VTABLE_PTR) &java_net_Inet6Address_isMCGlobal__;
    __TIB_java_net_Inet6Address.vtable[12] = (VTABLE_PTR) &java_net_Inet6Address_isMCNodeLocal__;
    __TIB_java_net_Inet6Address.vtable[11] = (VTABLE_PTR) &java_net_Inet6Address_isMCLinkLocal__;
    __TIB_java_net_Inet6Address.vtable[14] = (VTABLE_PTR) &java_net_Inet6Address_isMCSiteLocal__;
    __TIB_java_net_Inet6Address.vtable[13] = (VTABLE_PTR) &java_net_Inet6Address_isMCOrgLocal__;
    __TIB_java_net_Inet6Address.vtable[6] = (VTABLE_PTR) &java_net_Inet6Address_getHostAddress__;
    __TIB_java_net_Inet6Address.vtable[4] = (VTABLE_PTR) &java_net_Inet6Address_hashCode__;
    __TIB_java_net_Inet6Address.vtable[1] = (VTABLE_PTR) &java_net_Inet6Address_equals___java_lang_Object;
    __TIB_java_net_Inet6Address.vtable[5] = (VTABLE_PTR) &java_net_Inet6Address_toString__;
    // Initialize interface information
    __TIB_java_net_Inet6Address.numImplementedInterfaces = 1;
    __TIB_java_net_Inet6Address.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_net_Inet6Address.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_net_Inet6Address.itableBegin = &__TIB_java_net_Inet6Address.itable[0];

    _STATIC_java_net_Inet6Address_serialVersionUID = 6880410070516793377;
    _STATIC_java_net_Inet6Address_any_bytes = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_net_Inet6Address_localhost_bytes = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_net_Inet6Address_ANY = (java_net_InetAddress*) JAVA_NULL;
    _STATIC_java_net_Inet6Address_LOOPBACK = (java_net_InetAddress*) JAVA_NULL;
    _STATIC_java_net_Inet6Address_serialPersistentFields = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_net_Inet6Address.declaredFields = &__field_reflection_data[0];
    __TIB_java_net_Inet6Address.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_net_Inet6Address.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_net_Inet6Address.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_net_Inet6Address.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_net_Inet6Address.methodDispatcherFunc = method_dispatcher;
    __TIB_java_net_Inet6Address.declaredMethods = &__method_reflection_data[0];
    __TIB_java_net_Inet6Address.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_net_Inet6Address = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_net_Inet6Address);
    __TIB_java_net_Inet6Address.clazz = __CLASS_java_net_Inet6Address;
    __TIB_java_net_Inet6Address.baseType = JAVA_NULL;
    __CLASS_java_net_Inet6Address_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_Inet6Address);
    __CLASS_java_net_Inet6Address_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_Inet6Address_1ARRAY);
    __CLASS_java_net_Inet6Address_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_Inet6Address_2ARRAY);
    java_net_Inet6Address___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_net_Inet6Address]
    //XMLVM_END_WRAPPER

    __TIB_java_net_Inet6Address.classInitialized = 1;
}

void __DELETE_java_net_Inet6Address(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_net_Inet6Address]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_net_Inet6Address(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_net_InetAddress(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_net_Inet6Address*) me)->fields.java_net_Inet6Address.scope_id_ = 0;
    ((java_net_Inet6Address*) me)->fields.java_net_Inet6Address.scope_id_set_ = 0;
    ((java_net_Inet6Address*) me)->fields.java_net_Inet6Address.scope_ifname_set_ = 0;
    ((java_net_Inet6Address*) me)->fields.java_net_Inet6Address.ifname_ = (java_lang_String*) JAVA_NULL;
    ((java_net_Inet6Address*) me)->fields.java_net_Inet6Address.scopedIf_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_net_Inet6Address]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_net_Inet6Address()
{    XMLVM_CLASS_INIT(java_net_Inet6Address)
java_net_Inet6Address* me = (java_net_Inet6Address*) XMLVM_MALLOC(sizeof(java_net_Inet6Address));
    me->tib = &__TIB_java_net_Inet6Address;
    __INIT_INSTANCE_MEMBERS_java_net_Inet6Address(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_net_Inet6Address]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_net_Inet6Address()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_net_Inet6Address_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_serialVersionUID;
}

void java_net_Inet6Address_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_serialVersionUID = v;
}

JAVA_OBJECT java_net_Inet6Address_GET_any_bytes()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_any_bytes;
}

void java_net_Inet6Address_PUT_any_bytes(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_any_bytes = v;
}

JAVA_OBJECT java_net_Inet6Address_GET_localhost_bytes()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_localhost_bytes;
}

void java_net_Inet6Address_PUT_localhost_bytes(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_localhost_bytes = v;
}

JAVA_OBJECT java_net_Inet6Address_GET_ANY()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_ANY;
}

void java_net_Inet6Address_PUT_ANY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_ANY = v;
}

JAVA_OBJECT java_net_Inet6Address_GET_LOOPBACK()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_LOOPBACK;
}

void java_net_Inet6Address_PUT_LOOPBACK(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_LOOPBACK = v;
}

JAVA_OBJECT java_net_Inet6Address_GET_serialPersistentFields()
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    return _STATIC_java_net_Inet6Address_serialPersistentFields;
}

void java_net_Inet6Address_PUT_serialPersistentFields(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
_STATIC_java_net_Inet6Address_serialPersistentFields = v;
}

void java_net_Inet6Address___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address___INIT____byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 60)
    XMLVM_CHECK_NPE(1)
    java_net_InetAddress___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 61)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_ = _r2.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 62)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_ = _r0.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 63)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 65)
    XMLVM_CHECK_NPE(1)
    java_net_InetAddress___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 66)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 67)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_ = _r2.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 68)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_ = _r0.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 69)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String_int]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 82)
    XMLVM_CHECK_NPE(1)
    java_net_InetAddress___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 83)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 84)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_ = _r2.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 85)
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_ = _r4.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 86)
    if (_r4.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 87)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_set_ = _r0.i;
    label14:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 89)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "getByAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = n1;
    _r3.o = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 107)
    if (_r3.o == JAVA_NULL) goto label7;
    _r0.i = 16;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r0.i == _r1.i) goto label19;
    label7:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 109)
    _r0.o = __NEW_java_net_UnknownHostException();
    // "luni.62"
    _r1.o = xmlvm_create_java_string_from_pool(821);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_net_UnknownHostException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 111)
    if (_r4.i >= 0) goto label22;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 112)
    _r4.i = 0;
    label22:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 114)
    _r0.o = __NEW_java_net_Inet6Address();
    XMLVM_CHECK_NPE(0)
    java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String_int(_r0.o, _r3.o, _r2.o, _r4.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_java_net_NetworkInterface(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_java_net_NetworkInterface]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "getByAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    _r9.o = n1;
    _r10.o = n2;
    _r11.o = n3;
    _r8.i = 1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 138)
    _r6.i = 0;
    _r1.o = java_net_Inet6Address_getByAddress___java_lang_String_byte_1ARRAY_int(_r9.o, _r10.o, _r6.i);
    if (_r11.o != JAVA_NULL) goto label9;
    label8:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 141)
    XMLVM_SOURCE_POSITION("Inet6Address.java", 169)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label9:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 147)

    
    // Red class access removed: java.net.NetworkInterface::getInetAddresses
    XMLVM_RED_CLASS_DEPENDENCY();
    label13:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 148)
    XMLVM_CHECK_NPE(2)
    _r6.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r2.o);
    if (_r6.i == 0) goto label60;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 149)
    XMLVM_CHECK_NPE(2)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r2.o);
    _r3.o = _r3.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 150)
    XMLVM_CHECK_NPE(3)
    _r6.o = java_net_InetAddress_getAddress__(_r3.o);
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r7.i = 16;
    if (_r6.i != _r7.i) goto label13;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 151)
    _r0 = _r3;
    _r0.o = _r0.o;
    _r5 = _r0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 152)
    XMLVM_CHECK_NPE(5)
    _r4.i = java_net_Inet6Address_compareLocalType___java_net_Inet6Address(_r5.o, _r1.o);
    if (_r4.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 153)
    XMLVM_SOURCE_POSITION("Inet6Address.java", 154)
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_set_ = _r8.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 155)
    XMLVM_CHECK_NPE(5)
    _r6.i = ((java_net_Inet6Address*) _r5.o)->fields.java_net_Inet6Address.scope_id_;
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_ = _r6.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 156)
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_ifname_set_ = _r8.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 157)

    
    // Red class access removed: java.net.NetworkInterface::getName
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.ifname_ = _r6.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 158)
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scopedIf_ = _r11.o;
    label60:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 165)
    XMLVM_CHECK_NPE(1)
    _r6.i = ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_set_;
    if (_r6.i != 0) goto label8;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 167)
    _r6.o = __NEW_java_net_UnknownHostException();
    // "luni.63"
    _r7.o = xmlvm_create_java_string_from_pool(822);
    _r7.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r7.o);
    XMLVM_CHECK_NPE(6)
    java_net_UnknownHostException___INIT____java_lang_String(_r6.o, _r7.o);
    XMLVM_THROW_CUSTOM(_r6.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_compareLocalType___java_net_Inet6Address(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_compareLocalType___java_net_Inet6Address]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "compareLocalType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 182)
    //java_net_Inet6Address_isSiteLocalAddress__[16]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r3.o)->tib->vtable[16])(_r3.o);
    if (_r0.i == 0) goto label15;
    //java_net_Inet6Address_isSiteLocalAddress__[16]
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r2.o)->tib->vtable[16])(_r2.o);
    if (_r0.i == 0) goto label15;
    _r0 = _r1;
    label14:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 183)
    XMLVM_SOURCE_POSITION("Inet6Address.java", 191)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 185)
    //java_net_Inet6Address_isLinkLocalAddress__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r3.o)->tib->vtable[8])(_r3.o);
    if (_r0.i == 0) goto label29;
    //java_net_Inet6Address_isLinkLocalAddress__[8]
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r0.i == 0) goto label29;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 186)
    goto label14;
    label29:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 188)
    //java_net_Inet6Address_isSiteLocalAddress__[16]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r3.o)->tib->vtable[16])(_r3.o);
    if (_r0.i != 0) goto label43;
    //java_net_Inet6Address_isLinkLocalAddress__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_Inet6Address*) _r3.o)->tib->vtable[8])(_r3.o);
    if (_r0.i != 0) goto label43;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 189)
    goto label14;
    label43:;
    _r0.i = 0;
    goto label14;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address___INIT____byte_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address___INIT____byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 203)
    XMLVM_CHECK_NPE(1)
    java_net_InetAddress___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 204)
    XMLVM_CHECK_NPE(1)
    ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_ = _r2.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 205)
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_ = _r3.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 206)
    if (_r3.i == 0) goto label12;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 207)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_set_ = _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 209)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMulticastAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMulticastAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMulticastAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 221)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0 = _r2;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isAnyLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isAnyLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isAnyLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 233)
    _r0.i = 0;
    label1:;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i >= _r1.i) goto label17;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 234)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 235)
    _r1.i = 0;
    label13:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 238)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label14:;
    _r0.i = _r0.i + 1;
    goto label1;
    label17:;
    _r1.i = 1;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isLoopbackAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isLoopbackAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isLoopbackAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    _r4.i = 15;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 252)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_net_InetAddress*) _r5.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    if (_r1.i == _r3.i) goto label12;
    _r1 = _r2;
    label11:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 253)
    XMLVM_SOURCE_POSITION("Inet6Address.java", 263)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label12:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 257)
    _r0.i = 0;
    label13:;
    if (_r0.i >= _r4.i) goto label26;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 258)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_net_InetAddress*) _r5.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.i == 0) goto label23;
    _r1 = _r2;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 259)
    goto label11;
    label23:;
    _r0.i = _r0.i + 1;
    goto label13;
    label26:;
    _r1 = _r3;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isLinkLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isLinkLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isLinkLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 277)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -2;
    if (_r0.i != _r1.i) goto label22;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 255;
    _r0.i = ((JAVA_UINT) _r0.i) >> (0x1f & ((JAVA_UINT) 6));
    _r1.i = 2;
    if (_r0.i != _r1.i) goto label22;
    _r0 = _r3;
    label21:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    _r0 = _r2;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isSiteLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isSiteLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isSiteLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 291)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -2;
    if (_r0.i != _r1.i) goto label22;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 255;
    _r0.i = ((JAVA_UINT) _r0.i) >> (0x1f & ((JAVA_UINT) 6));
    _r1.i = 3;
    if (_r0.i != _r1.i) goto label22;
    _r0 = _r3;
    label21:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    _r0 = _r2;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMCGlobal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMCGlobal__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMCGlobal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 305)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label21;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 15;
    _r1.i = 14;
    if (_r0.i != _r1.i) goto label21;
    _r0 = _r3;
    label20:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label21:;
    _r0 = _r2;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMCNodeLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMCNodeLocal__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMCNodeLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 320)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label19;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r0.i = _r0.i & 15;
    if (_r0.i != _r2.i) goto label19;
    _r0 = _r2;
    label18:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label19:;
    _r0 = _r3;
    goto label18;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMCLinkLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMCLinkLocal__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMCLinkLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 335)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label20;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 15;
    _r1.i = 2;
    if (_r0.i != _r1.i) goto label20;
    _r0 = _r3;
    label19:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label20:;
    _r0 = _r2;
    goto label19;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMCSiteLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMCSiteLocal__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMCSiteLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 350)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label20;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 15;
    _r1.i = 5;
    if (_r0.i != _r1.i) goto label20;
    _r0 = _r3;
    label19:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label20:;
    _r0 = _r2;
    goto label19;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isMCOrgLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isMCOrgLocal__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isMCOrgLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 365)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label21;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r0.i = _r0.i & 15;
    _r1.i = 8;
    if (_r0.i != _r1.i) goto label21;
    _r0 = _r3;
    label20:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label21:;
    _r0 = _r2;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_Inet6Address_getHostAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_getHostAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "getHostAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 375)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createIPAddrStringFromByteArray
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_Inet6Address_getScopeId__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_getScopeId__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "getScopeId", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 386)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_set_;
    if (_r0.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 387)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_id_;
    label6:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 389)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_Inet6Address_getScopedInterface__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_getScopedInterface__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "getScopedInterface", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 399)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scope_ifname_set_;
    if (_r0.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 400)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_net_Inet6Address*) _r1.o)->fields.java_net_Inet6Address.scopedIf_;
    label6:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 402)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    _r0.o = JAVA_NULL;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_Inet6Address_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_hashCode__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 413)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    _r1.i = 12;
    _r0.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r0.o, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 428)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_net_InetAddress_equals___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_Inet6Address_isIPv4CompatibleAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_isIPv4CompatibleAddress__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "isIPv4CompatibleAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 440)
    _r0.i = 0;
    label1:;
    _r1.i = 12;
    if (_r0.i >= _r1.i) goto label16;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 441)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 442)
    _r1.i = 0;
    label12:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 445)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label13:;
    _r0.i = _r0.i + 1;
    goto label1;
    label16:;
    _r1.i = 1;
    goto label12;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    // "ipaddress"
    _r2.o = xmlvm_create_java_string_from_pool(823);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 456)

    
    // Red class access removed: java.io.ObjectOutputStream::putFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 457)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.ipaddress_;
    if (_r1.o != JAVA_NULL) goto label48;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 458)
    // "ipaddress"
    _r1.o = xmlvm_create_java_string_from_pool(823);
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    label16:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 463)
    // "scope_id"
    _r1.o = xmlvm_create_java_string_from_pool(824);
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.scope_id_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 464)
    // "scope_id_set"
    _r1.o = xmlvm_create_java_string_from_pool(825);
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.scope_id_set_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 465)
    // "scope_ifname_set"
    _r1.o = xmlvm_create_java_string_from_pool(826);
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.scope_ifname_set_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 466)
    // "ifname"
    _r1.o = xmlvm_create_java_string_from_pool(827);
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.ifname_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 467)

    
    // Red class access removed: java.io.ObjectOutputStream::writeFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 468)
    XMLVM_EXIT_METHOD()
    return;
    label48:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 460)
    // "ipaddress"
    _r1.o = xmlvm_create_java_string_from_pool(823);
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label16;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.o = JAVA_NULL;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 472)

    
    // Red class access removed: java.io.ObjectInputStream::readFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Inet6Address.java", 473)
    // "ipaddress"
    _r1.o = xmlvm_create_java_string_from_pool(823);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = _r1.o;
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(4)
    ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_ = _r1.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 474)
    // "scope_id"
    _r1.o = xmlvm_create_java_string_from_pool(824);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.scope_id_ = _r1.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 475)
    // "scope_id_set"
    _r1.o = xmlvm_create_java_string_from_pool(825);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.scope_id_set_ = _r1.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 476)
    // "ifname"
    _r1.o = xmlvm_create_java_string_from_pool(827);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(4)
    ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.ifname_ = _r1.o;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 477)
    // "scope_ifname_set"
    _r1.o = xmlvm_create_java_string_from_pool(826);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.scope_ifname_set_ = _r1.i;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 478)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.scope_ifname_set_;
    if (_r1.i == 0) goto label68;
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.ifname_;
    if (_r1.o == JAVA_NULL) goto label68;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 479)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.ifname_;

    
    // Red class access removed: java.net.NetworkInterface::getByName
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_net_Inet6Address*) _r4.o)->fields.java_net_Inet6Address.scopedIf_ = _r1.o;
    label68:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 481)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_Inet6Address_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address_toString__]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    // "%"
    _r2.o = xmlvm_create_java_string_from_pool(80);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 491)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.ifname_;
    if (_r0.o == JAVA_NULL) goto label36;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 492)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_net_InetAddress_toString__(_r3.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "%"
    _r1.o = xmlvm_create_java_string_from_pool(80);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.ifname_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    label35:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 497)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 494)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.scope_id_;
    if (_r0.i == 0) goto label70;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 495)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_net_InetAddress_toString__(_r3.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "%"
    _r1.o = xmlvm_create_java_string_from_pool(80);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_net_Inet6Address*) _r3.o)->fields.java_net_Inet6Address.scope_id_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label35;
    label70:;
    XMLVM_CHECK_NPE(3)
    _r0.o = java_net_InetAddress_toString__(_r3.o);
    goto label35;
    //XMLVM_END_WRAPPER
}

void java_net_Inet6Address___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_net_Inet6Address___CLINIT___]
    XMLVM_ENTER_METHOD("java.net.Inet6Address", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.i = 16;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("Inet6Address.java", 36)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r1.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_BYTE[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, });
    java_net_Inet6Address_PUT_any_bytes( _r0.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 39)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r1.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_BYTE[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, });
    java_net_Inet6Address_PUT_localhost_bytes( _r0.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 42)
    _r0.o = __NEW_java_net_Inet6Address();
    _r1.o = java_net_Inet6Address_GET_any_bytes();
    XMLVM_CHECK_NPE(0)
    java_net_Inet6Address___INIT____byte_1ARRAY(_r0.o, _r1.o);
    java_net_Inet6Address_PUT_ANY( _r0.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 44)
    _r0.o = __NEW_java_net_Inet6Address();
    _r1.o = java_net_Inet6Address_GET_localhost_bytes();
    // "localhost"
    _r2.o = xmlvm_create_java_string_from_pool(71);
    XMLVM_CHECK_NPE(0)
    java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_net_Inet6Address_PUT_LOOPBACK( _r0.o);
    XMLVM_SOURCE_POSITION("Inet6Address.java", 448)
    _r0.i = 5;
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_io_ObjectStreamField, _r0.i);
    _r1.o = __NEW_java_io_ObjectStreamField();
    // "ipaddress"
    _r2.o = xmlvm_create_java_string_from_pool(823);
    XMLVM_CLASS_INIT(byte)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r4.i);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[3])(_r3.o);
    XMLVM_CHECK_NPE(1)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r1.o;
    _r1.i = 1;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "scope_id"
    _r3.o = xmlvm_create_java_string_from_pool(824);
    _r4.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 2;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "scope_id_set"
    _r3.o = xmlvm_create_java_string_from_pool(825);
    _r4.o = java_lang_Boolean_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 3;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "scope_ifname_set"
    _r3.o = xmlvm_create_java_string_from_pool(826);
    _r4.o = java_lang_Boolean_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 4;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "ifname"
    _r3.o = xmlvm_create_java_string_from_pool(827);
    XMLVM_CLASS_INIT(java_lang_String)
    _r4.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    java_net_Inet6Address_PUT_serialPersistentFields( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    label106:;
    label118:;
    //XMLVM_END_WRAPPER
}

