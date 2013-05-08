#include "xmlvm.h"
#include "java_io_FileDescriptor.h"
#include "java_io_IOException.h"
#include "java_io_ObjectStreamField.h"
#include "java_lang_Class.h"
#include "java_lang_Exception.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"
#include "java_lang_SecurityManager.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Throwable.h"
#include "java_net_Inet6Address.h"
#include "java_net_UnknownHostException.h"
#include "java_security_AccessController.h"
#include "java_util_ArrayList.h"
#include "java_util_Enumeration.h"
#include "java_util_StringTokenizer.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"
#include "org_apache_harmony_luni_platform_INetworkSystem.h"
#include "org_apache_harmony_luni_util_PriviAction.h"

#include "java_net_InetAddress.h"

#define XMLVM_CURRENT_CLASS_NAME InetAddress
#define XMLVM_CURRENT_PKG_CLASS_NAME java_net_InetAddress

__TIB_DEFINITION_java_net_InetAddress __TIB_java_net_InetAddress = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_net_InetAddress, // classInitializer
    "java.net.InetAddress", // className
    "java.net", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_net_InetAddress), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_net_InetAddress;
JAVA_OBJECT __CLASS_java_net_InetAddress_1ARRAY;
JAVA_OBJECT __CLASS_java_net_InetAddress_2ARRAY;
JAVA_OBJECT __CLASS_java_net_InetAddress_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_net_InetAddress_any_bytes;
static JAVA_OBJECT _STATIC_java_net_InetAddress_localhost_bytes;
static JAVA_OBJECT _STATIC_java_net_InetAddress_ANY;
static JAVA_OBJECT _STATIC_java_net_InetAddress_NETIMPL;
static JAVA_OBJECT _STATIC_java_net_InetAddress_LOOPBACK;
static JAVA_OBJECT _STATIC_java_net_InetAddress_ERRMSG_CONNECTION_REFUSED;
static JAVA_LONG _STATIC_java_net_InetAddress_serialVersionUID;
static JAVA_OBJECT _STATIC_java_net_InetAddress_serialPersistentFields;

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

void __INIT_java_net_InetAddress()
{
    staticInitializerLock(&__TIB_java_net_InetAddress);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_net_InetAddress.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_net_InetAddress.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_net_InetAddress);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_net_InetAddress.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_net_InetAddress.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_net_InetAddress.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.net.InetAddress")
        __INIT_IMPL_java_net_InetAddress();
    }
}

void __INIT_IMPL_java_net_InetAddress()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_net_InetAddress.newInstanceFunc = __NEW_INSTANCE_java_net_InetAddress;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_net_InetAddress.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_net_InetAddress.vtable[1] = (VTABLE_PTR) &java_net_InetAddress_equals___java_lang_Object;
    __TIB_java_net_InetAddress.vtable[6] = (VTABLE_PTR) &java_net_InetAddress_getHostAddress__;
    __TIB_java_net_InetAddress.vtable[4] = (VTABLE_PTR) &java_net_InetAddress_hashCode__;
    __TIB_java_net_InetAddress.vtable[15] = (VTABLE_PTR) &java_net_InetAddress_isMulticastAddress__;
    __TIB_java_net_InetAddress.vtable[5] = (VTABLE_PTR) &java_net_InetAddress_toString__;
    __TIB_java_net_InetAddress.vtable[9] = (VTABLE_PTR) &java_net_InetAddress_isLoopbackAddress__;
    __TIB_java_net_InetAddress.vtable[8] = (VTABLE_PTR) &java_net_InetAddress_isLinkLocalAddress__;
    __TIB_java_net_InetAddress.vtable[16] = (VTABLE_PTR) &java_net_InetAddress_isSiteLocalAddress__;
    __TIB_java_net_InetAddress.vtable[10] = (VTABLE_PTR) &java_net_InetAddress_isMCGlobal__;
    __TIB_java_net_InetAddress.vtable[12] = (VTABLE_PTR) &java_net_InetAddress_isMCNodeLocal__;
    __TIB_java_net_InetAddress.vtable[11] = (VTABLE_PTR) &java_net_InetAddress_isMCLinkLocal__;
    __TIB_java_net_InetAddress.vtable[14] = (VTABLE_PTR) &java_net_InetAddress_isMCSiteLocal__;
    __TIB_java_net_InetAddress.vtable[13] = (VTABLE_PTR) &java_net_InetAddress_isMCOrgLocal__;
    __TIB_java_net_InetAddress.vtable[7] = (VTABLE_PTR) &java_net_InetAddress_isAnyLocalAddress__;
    // Initialize interface information
    __TIB_java_net_InetAddress.numImplementedInterfaces = 1;
    __TIB_java_net_InetAddress.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_net_InetAddress.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_net_InetAddress.itableBegin = &__TIB_java_net_InetAddress.itable[0];

    _STATIC_java_net_InetAddress_any_bytes = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_net_InetAddress_localhost_bytes = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_net_InetAddress_ANY = (java_net_InetAddress*) JAVA_NULL;
    _STATIC_java_net_InetAddress_NETIMPL = (org_apache_harmony_luni_platform_INetworkSystem*) JAVA_NULL;
    _STATIC_java_net_InetAddress_LOOPBACK = (java_net_InetAddress*) JAVA_NULL;
    _STATIC_java_net_InetAddress_ERRMSG_CONNECTION_REFUSED = (java_lang_String*) xmlvm_create_java_string_from_pool(69);
    _STATIC_java_net_InetAddress_serialVersionUID = 3286316764910316507;
    _STATIC_java_net_InetAddress_serialPersistentFields = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_net_InetAddress.declaredFields = &__field_reflection_data[0];
    __TIB_java_net_InetAddress.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_net_InetAddress.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_net_InetAddress.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_net_InetAddress.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_net_InetAddress.methodDispatcherFunc = method_dispatcher;
    __TIB_java_net_InetAddress.declaredMethods = &__method_reflection_data[0];
    __TIB_java_net_InetAddress.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_net_InetAddress = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_net_InetAddress);
    __TIB_java_net_InetAddress.clazz = __CLASS_java_net_InetAddress;
    __TIB_java_net_InetAddress.baseType = JAVA_NULL;
    __CLASS_java_net_InetAddress_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_InetAddress);
    __CLASS_java_net_InetAddress_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_InetAddress_1ARRAY);
    __CLASS_java_net_InetAddress_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_InetAddress_2ARRAY);
    java_net_InetAddress___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_net_InetAddress]
    //XMLVM_END_WRAPPER

    __TIB_java_net_InetAddress.classInitialized = 1;
}

void __DELETE_java_net_InetAddress(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_net_InetAddress]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_net_InetAddress(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.hostName_ = (java_lang_String*) JAVA_NULL;
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.waitReachable_ = (java_lang_Object*) JAVA_NULL;
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.reached_ = 0;
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.addrCount_ = 0;
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.family_ = 0;
    ((java_net_InetAddress*) me)->fields.java_net_InetAddress.ipaddress_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_net_InetAddress]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_net_InetAddress()
{    XMLVM_CLASS_INIT(java_net_InetAddress)
java_net_InetAddress* me = (java_net_InetAddress*) XMLVM_MALLOC(sizeof(java_net_InetAddress));
    me->tib = &__TIB_java_net_InetAddress;
    __INIT_INSTANCE_MEMBERS_java_net_InetAddress(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_net_InetAddress]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_net_InetAddress()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_net_InetAddress();
    java_net_InetAddress___INIT___(me);
    return me;
}

JAVA_OBJECT java_net_InetAddress_GET_any_bytes()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_any_bytes;
}

void java_net_InetAddress_PUT_any_bytes(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_any_bytes = v;
}

JAVA_OBJECT java_net_InetAddress_GET_localhost_bytes()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_localhost_bytes;
}

void java_net_InetAddress_PUT_localhost_bytes(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_localhost_bytes = v;
}

JAVA_OBJECT java_net_InetAddress_GET_ANY()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_ANY;
}

void java_net_InetAddress_PUT_ANY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_ANY = v;
}

JAVA_OBJECT java_net_InetAddress_GET_NETIMPL()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_NETIMPL;
}

void java_net_InetAddress_PUT_NETIMPL(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_NETIMPL = v;
}

JAVA_OBJECT java_net_InetAddress_GET_LOOPBACK()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_LOOPBACK;
}

void java_net_InetAddress_PUT_LOOPBACK(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_LOOPBACK = v;
}

JAVA_OBJECT java_net_InetAddress_GET_ERRMSG_CONNECTION_REFUSED()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_ERRMSG_CONNECTION_REFUSED;
}

void java_net_InetAddress_PUT_ERRMSG_CONNECTION_REFUSED(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_ERRMSG_CONNECTION_REFUSED = v;
}

JAVA_LONG java_net_InetAddress_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_serialVersionUID;
}

void java_net_InetAddress_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_serialVersionUID = v;
}

JAVA_OBJECT java_net_InetAddress_GET_serialPersistentFields()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    return _STATIC_java_net_InetAddress_serialPersistentFields;
}

void java_net_InetAddress_PUT_serialPersistentFields(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
_STATIC_java_net_InetAddress_serialPersistentFields = v;
}

//XMLVM_NATIVE[void java_net_InetAddress_oneTimeInitialization___boolean(JAVA_BOOLEAN n1)]

void java_net_InetAddress___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress___INIT___]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 88)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 67)

    
    // Red class access removed: java.net.InetAddress$WaitReachable::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.net.InetAddress$WaitReachable::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.waitReachable_ = _r0.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 73)
    _r0.i = 2;
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.family_ = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 89)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress___INIT____byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 99)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 67)

    
    // Red class access removed: java.net.InetAddress$WaitReachable::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.net.InetAddress$WaitReachable::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.waitReachable_ = _r0.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 73)
    _r0.i = 2;
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.family_ = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 100)
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_ = _r3.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 101)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress___INIT____byte_1ARRAY_java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("InetAddress.java", 111)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 67)

    
    // Red class access removed: java.net.InetAddress$WaitReachable::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.net.InetAddress$WaitReachable::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.waitReachable_ = _r0.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 73)
    _r0.i = 2;
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.family_ = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 112)
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_ = _r3.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 113)
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.hostName_ = _r4.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 114)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_cacheElement__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_cacheElement__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "cacheElement", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 117)

    
    // Red class access removed: java.net.InetAddress$CacheElement::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.InetAddress$CacheElement::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 131)
    if (_r6.o != JAVA_NULL) goto label5;
    _r2 = _r4;
    label4:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 132)
    XMLVM_SOURCE_POSITION("InetAddress.java", 145)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label5:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 134)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[3])(_r6.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(5)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[3])(_r5.o);
    if (_r2.o == _r3.o) goto label17;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 135)
    goto label4;
    label17:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 139)
    _r6.o = _r6.o;
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r0.i = 0;
    label22:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 140)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i >= _r2.i) goto label38;
    XMLVM_SOURCE_POSITION("InetAddress.java", 141)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_net_InetAddress*) _r5.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r2.i == _r3.i) goto label35;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 142)
    goto label4;
    label35:;
    _r0.i = _r0.i + 1;
    goto label22;
    label38:;
    _r2.i = 1;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 156)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.ipaddress_;
    //byte_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->tib->vtable[0])(_r0.o);
    _r1.o = _r1.o;
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getAllByName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getAllByName___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getAllByName", "?")
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
    _r10.o = n1;
    _r9.i = 1;
    _r8.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 175)
    if (_r10.o == JAVA_NULL) goto label10;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(10)
    _r6.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r10.o)->tib->vtable[8])(_r10.o);
    if (_r6.i != 0) goto label26;
    label10:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 176)
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_java_net_InetAddress, _r9.i);
    _r7.i = java_net_InetAddress_preferIPv6Addresses__();
    if (_r7.i == 0) goto label23;
    _r7.o = java_net_Inet6Address_GET_LOOPBACK();
    label20:;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r7.o;
    label22:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 234)
    XMLVM_EXIT_METHOD()
    return _r6.o;
    label23:;
    _r7.o = java_net_InetAddress_GET_LOOPBACK();
    goto label20;
    label26:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 180)
    _r6.i = java_net_InetAddress_isHostName___java_lang_String(_r10.o);
    if (_r6.i == 0) goto label148;
    XMLVM_SOURCE_POSITION("InetAddress.java", 181)
    _r5.o = java_lang_System_getSecurityManager__();
    if (_r5.o == JAVA_NULL) goto label42;
    XMLVM_SOURCE_POSITION("InetAddress.java", 182)
    XMLVM_SOURCE_POSITION("InetAddress.java", 183)
    _r6.i = -1;
    XMLVM_CHECK_NPE(5)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r5.o, _r10.o, _r6.i);
    label42:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 185)

    
    // Red class access removed: java.net.Socket::preferIPv4Stack
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.i == 0) goto label53;
    XMLVM_SOURCE_POSITION("InetAddress.java", 186)
    _r6.o = java_net_InetAddress_getAliasesByNameImpl___java_lang_String(_r10.o);
    goto label22;
    label53:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 191)
    _r4.o = java_net_InetAddress_getAliasesByNameImpl___java_lang_String(_r10.o);
    _r3.o = JAVA_NULL;
    if (_r4.o == JAVA_NULL) goto label146;
    XMLVM_SOURCE_POSITION("InetAddress.java", 193)
    XMLVM_SOURCE_POSITION("InetAddress.java", 194)
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_net_InetAddress, _r6.i);
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 196)
    _r6.i = java_net_InetAddress_preferIPv6Addresses__();
    if (_r6.i == 0) goto label108;
    XMLVM_SOURCE_POSITION("InetAddress.java", 197)
    _r2.i = 0;
    label71:;
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r2.i >= _r6.i) goto label89;
    XMLVM_SOURCE_POSITION("InetAddress.java", 198)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    _r6.i = XMLVM_ISA(_r6.o, __CLASS_java_net_Inet6Address);
    if (_r6.i == 0) goto label86;
    XMLVM_SOURCE_POSITION("InetAddress.java", 199)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r6.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 200)
    _r0.i = _r0.i + 1;
    label86:;
    _r2.i = _r2.i + 1;
    goto label71;
    label89:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 203)
    _r2.i = 0;
    label90:;
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r2.i >= _r6.i) goto label146;
    XMLVM_SOURCE_POSITION("InetAddress.java", 204)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];

    
    // Red class access removed: java.net.Inet4Address::instance-of
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.i == 0) goto label105;
    XMLVM_SOURCE_POSITION("InetAddress.java", 205)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r6.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 206)
    _r0.i = _r0.i + 1;
    label105:;
    _r2.i = _r2.i + 1;
    goto label90;
    label108:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 210)
    _r2.i = 0;
    label109:;
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r2.i >= _r6.i) goto label127;
    XMLVM_SOURCE_POSITION("InetAddress.java", 211)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];

    
    // Red class access removed: java.net.Inet4Address::instance-of
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.i == 0) goto label124;
    XMLVM_SOURCE_POSITION("InetAddress.java", 212)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r6.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 213)
    _r0.i = _r0.i + 1;
    label124:;
    _r2.i = _r2.i + 1;
    goto label109;
    label127:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 216)
    _r2.i = 0;
    label128:;
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r2.i >= _r6.i) goto label146;
    XMLVM_SOURCE_POSITION("InetAddress.java", 217)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CLASS_INIT(java_net_Inet6Address)
    _r6.i = XMLVM_ISA(_r6.o, __CLASS_java_net_Inet6Address);
    if (_r6.i == 0) goto label143;
    XMLVM_SOURCE_POSITION("InetAddress.java", 218)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r6.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 219)
    _r0.i = _r0.i + 1;
    label143:;
    _r2.i = _r2.i + 1;
    goto label128;
    label146:;
    _r6 = _r3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 224)
    goto label22;
    label148:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 227)

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createByteArrayFromIPAddressString
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 228)
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r7.i = 4;
    if (_r6.i != _r7.i) goto label167;
    XMLVM_SOURCE_POSITION("InetAddress.java", 229)
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_java_net_InetAddress, _r9.i);

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r7.o;
    goto label22;
    label167:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 230)
    _r6.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r7.i = 16;
    if (_r6.i != _r7.i) goto label183;
    XMLVM_SOURCE_POSITION("InetAddress.java", 231)
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_java_net_InetAddress, _r9.i);
    _r7.o = __NEW_java_net_Inet6Address();
    XMLVM_CHECK_NPE(7)
    java_net_Inet6Address___INIT____byte_1ARRAY(_r7.o, _r1.o);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r7.o;
    goto label22;
    label183:;
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_java_net_InetAddress, _r9.i);
    _r7.o = __NEW_java_net_InetAddress();
    XMLVM_CHECK_NPE(7)
    java_net_InetAddress___INIT____byte_1ARRAY(_r7.o, _r1.o);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r7.o;
    goto label22;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getByName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getByName___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getByName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 252)
    if (_r2.o == JAVA_NULL) goto label8;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r1.i != 0) goto label11;
    label8:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 253)
    _r1.o = java_net_InetAddress_GET_LOOPBACK();
    label10:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 267)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label11:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 255)
    // "0"
    _r1.o = xmlvm_create_java_string_from_pool(70);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[1])(_r2.o, _r1.o);
    if (_r1.i == 0) goto label22;
    XMLVM_SOURCE_POSITION("InetAddress.java", 256)
    _r1.o = java_net_InetAddress_GET_ANY();
    goto label10;
    label22:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 259)
    _r1.i = java_net_InetAddress_isHostName___java_lang_String(_r2.o);
    if (_r1.i == 0) goto label43;
    XMLVM_SOURCE_POSITION("InetAddress.java", 260)
    _r0.o = java_lang_System_getSecurityManager__();
    if (_r0.o == JAVA_NULL) goto label38;
    XMLVM_SOURCE_POSITION("InetAddress.java", 261)
    XMLVM_SOURCE_POSITION("InetAddress.java", 262)
    _r1.i = -1;
    XMLVM_CHECK_NPE(0)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r0.o, _r2.o, _r1.i);
    label38:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 264)
    _r1.o = java_net_InetAddress_lookupHostByName___java_lang_String(_r2.o);
    goto label10;
    label43:;
    _r1.o = java_net_InetAddress_createHostNameFromIPAddress___java_lang_String(_r2.o);
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getHostAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getHostAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getHostAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 277)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    _r1.i = 0;
    _r0.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r0.o, _r1.i);
    _r0.o = java_net_InetAddress_inetNtoaImpl___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getHostName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getHostName__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getHostName", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    _r5.i = 4;
    XMLVM_TRY_BEGIN(w1175aaac24b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 289)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_;
    if (_r3.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w1175aaac24b1b3->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b3, sizeof(XMLVM_JMP_BUF)); goto label75; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 290)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 291)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r3.i != _r5.i) { XMLVM_MEMCPY(curThread_w1175aaac24b1b3->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b3, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 292)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r4.i = 0;
    _r0.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r3.o, _r4.i);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1175aaac24b1b3->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b3, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 293)
    XMLVM_SOURCE_POSITION("InetAddress.java", 294)
    _r3.o = java_net_InetAddress_inetNtoaImpl___int(_r0.i);
    XMLVM_CHECK_NPE(6)
    ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac24b1b3)
        XMLVM_CATCH_SPECIFIC(w1175aaac24b1b3,java_net_UnknownHostException,64)
    XMLVM_CATCH_END(w1175aaac24b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac24b1b3)
    label26:;
    XMLVM_TRY_BEGIN(w1175aaac24b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 316)
    XMLVM_MEMCPY(curThread_w1175aaac24b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b5, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac24b1b5)
        XMLVM_CATCH_SPECIFIC(w1175aaac24b1b5,java_net_UnknownHostException,64)
    XMLVM_CATCH_END(w1175aaac24b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac24b1b5)
    label27:;
    XMLVM_TRY_BEGIN(w1175aaac24b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 297)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r3.o = java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY(_r3.o);
    XMLVM_CHECK_NPE(3)
    _r3.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.hostName_;
    XMLVM_CHECK_NPE(6)
    ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 298)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_;
    // "localhost"
    _r4.o = xmlvm_create_java_string_from_pool(71);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[1])(_r3.o, _r4.o);
    if (_r3.i == 0) { XMLVM_MEMCPY(curThread_w1175aaac24b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b7, sizeof(XMLVM_JMP_BUF)); goto label75; };
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r3.i != _r5.i) { XMLVM_MEMCPY(curThread_w1175aaac24b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b7, sizeof(XMLVM_JMP_BUF)); goto label75; };
    _r3.i = 2130706433;
    if (_r0.i == _r3.i) { XMLVM_MEMCPY(curThread_w1175aaac24b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1b7, sizeof(XMLVM_JMP_BUF)); goto label75; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 300)
    _r3.o = java_net_InetAddress_inetNtoaImpl___int(_r0.i);
    XMLVM_CHECK_NPE(6)
    ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac24b1b7)
        XMLVM_CATCH_SPECIFIC(w1175aaac24b1b7,java_net_UnknownHostException,64)
    XMLVM_CATCH_END(w1175aaac24b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac24b1b7)
    goto label26;
    label64:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 303)
    java_lang_Thread* curThread_w1175aaac24b1c11 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w1175aaac24b1c11->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 304)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createIPAddrStringFromByteArray
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(6)
    ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_ = _r3.o;
    goto label26;
    label75:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 307)
    _r2.o = java_lang_System_getSecurityManager__();
    XMLVM_SOURCE_POSITION("InetAddress.java", 310)
    if (_r2.o == JAVA_NULL) goto label95;
    XMLVM_TRY_BEGIN(w1175aaac24b1c23)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_;
    _r3.i = java_net_InetAddress_isHostName___java_lang_String(_r3.o);
    if (_r3.i == 0) { XMLVM_MEMCPY(curThread_w1175aaac24b1c23->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac24b1c23, sizeof(XMLVM_JMP_BUF)); goto label95; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 311)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_;
    _r4.i = -1;
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r2.o, _r3.o, _r4.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac24b1c23)
    XMLVM_CATCH_END(w1175aaac24b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac24b1c23)
    label95:;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.hostName_;
    goto label26;
    label98:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 313)
    java_lang_Thread* curThread_w1175aaac24b1c29 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w1175aaac24b1c29->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 314)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createIPAddrStringFromByteArray
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label26;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getCanonicalHostName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getCanonicalHostName__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getCanonicalHostName", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 330)
    _r0.i = 0;
    XMLVM_TRY_BEGIN(w1175aaac25b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 331)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    _r5.i = 4;
    if (_r4.i != _r5.i) { XMLVM_MEMCPY(curThread_w1175aaac25b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac25b1b4, sizeof(XMLVM_JMP_BUF)); goto label21; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 332)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r5.i = 0;
    _r0.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r4.o, _r5.i);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1175aaac25b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac25b1b4, sizeof(XMLVM_JMP_BUF)); goto label21; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 333)
    XMLVM_SOURCE_POSITION("InetAddress.java", 334)
    _r4.o = java_net_InetAddress_inetNtoaImpl___int(_r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac25b1b4)
        XMLVM_CATCH_SPECIFIC(w1175aaac25b1b4,java_net_UnknownHostException,47)
    XMLVM_CATCH_END(w1175aaac25b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac25b1b4)
    label20:;
    XMLVM_TRY_BEGIN(w1175aaac25b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 350)
    XMLVM_MEMCPY(curThread_w1175aaac25b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac25b1b6, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r4.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac25b1b6)
        XMLVM_CATCH_SPECIFIC(w1175aaac25b1b6,java_net_UnknownHostException,47)
    XMLVM_CATCH_END(w1175aaac25b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac25b1b6)
    label21:;
    XMLVM_TRY_BEGIN(w1175aaac25b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 337)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;
    _r4.o = java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY(_r4.o);
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.hostName_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac25b1b8)
        XMLVM_CATCH_SPECIFIC(w1175aaac25b1b8,java_net_UnknownHostException,47)
    XMLVM_CATCH_END(w1175aaac25b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac25b1b8)
    XMLVM_SOURCE_POSITION("InetAddress.java", 341)
    _r3.o = java_lang_System_getSecurityManager__();
    XMLVM_SOURCE_POSITION("InetAddress.java", 344)
    if (_r3.o == JAVA_NULL) goto label45;
    XMLVM_TRY_BEGIN(w1175aaac25b1c13)
    // Begin try
    _r4.i = java_net_InetAddress_isHostName___java_lang_String(_r1.o);
    if (_r4.i == 0) { XMLVM_MEMCPY(curThread_w1175aaac25b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac25b1c13, sizeof(XMLVM_JMP_BUF)); goto label45; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 345)
    _r4.i = -1;
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r3.o, _r1.o, _r4.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac25b1c13)
    XMLVM_CATCH_END(w1175aaac25b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac25b1c13)
    label45:;
    _r4 = _r1;
    goto label20;
    label47:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 338)
    java_lang_Thread* curThread_w1175aaac25b1c19 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w1175aaac25b1c19->fields.java_lang_Thread.xmlvmException_;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 339)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createIPAddrStringFromByteArray
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label20;
    label56:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 347)
    java_lang_Thread* curThread_w1175aaac25b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w1175aaac25b1c27->fields.java_lang_Thread.xmlvmException_;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 348)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_net_InetAddress*) _r6.o)->fields.java_net_InetAddress.ipaddress_;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::createIPAddrStringFromByteArray
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getLocalHost__()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getLocalHost__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getLocalHost", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 363)
    _r1.o = java_net_InetAddress_getHostNameImpl__();
    XMLVM_SOURCE_POSITION("InetAddress.java", 364)
    _r2.o = java_lang_System_getSecurityManager__();
    XMLVM_SOURCE_POSITION("InetAddress.java", 366)
    if (_r2.o == JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("InetAddress.java", 367)
    _r3.i = -1;
    XMLVM_TRY_BEGIN(w1175aaac26b1b9)
    // Begin try
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r2.o, _r1.o, _r3.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac26b1b9)
    XMLVM_CATCH_END(w1175aaac26b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac26b1b9)
    label14:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 372)
    _r3.o = java_net_InetAddress_lookupHostByName___java_lang_String(_r1.o);
    label18:;
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label19:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 369)
    java_lang_Thread* curThread_w1175aaac26b1c17 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1175aaac26b1c17->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("InetAddress.java", 370)
    _r3.o = java_net_InetAddress_GET_LOOPBACK();
    goto label18;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_InetAddress_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_hashCode__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 382)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.ipaddress_;
    _r1.i = 0;
    _r0.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r0.o, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMulticastAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMulticastAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMulticastAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 392)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r0.i = _r0.i & 255;
    _r0.i = ((JAVA_UINT) _r0.i) >> (0x1f & ((JAVA_UINT) 4));
    _r1.i = 14;
    if (_r0.i != _r1.i) goto label15;
    _r0.i = 1;
    label14:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    _r0 = _r2;
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_lookupHostByName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_lookupHostByName___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "lookupHostByName", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    volatile XMLVMElem _r11;
    _r11.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 397)
    XMLVM_CLASS_INIT(java_net_InetAddress)
    _r6.o = __CLASS_java_net_InetAddress;
    java_lang_Object_acquireLockRecursive__(_r6.o);
    _r4.i = -1;
    XMLVM_TRY_BEGIN(w1175aaac29b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 399)
    _r7.o = __NEW_org_apache_harmony_luni_util_PriviAction();
    // "networkaddress.cache.ttl"
    _r8.o = xmlvm_create_java_string_from_pool(72);
    XMLVM_CHECK_NPE(7)
    org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String(_r7.o, _r8.o);
    _r5.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r7.o);
    _r5.o = _r5.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1b6)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1b6,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1b6)
    XMLVM_SOURCE_POSITION("InetAddress.java", 403)
    if (_r5.o == JAVA_NULL) goto label27;
    XMLVM_TRY_BEGIN(w1175aaac29b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 404)
    _r7.o = java_lang_Integer_decode___java_lang_String(_r5.o);
    //java_lang_Integer_intValue__[9]
    XMLVM_CHECK_NPE(7)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Integer*) _r7.o)->tib->vtable[9])(_r7.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1b9)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1b9,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1b9)
    label27:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 409)
    _r2.o = JAVA_NULL;
    if (_r4.i != 0) goto label41;
    XMLVM_TRY_BEGIN(w1175aaac29b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 410)
    XMLVM_SOURCE_POSITION("InetAddress.java", 411)

    
    // Red class access removed: java.net.InetAddress$Cache::clear
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c14)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c14,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c14)
    label33:;
    XMLVM_TRY_BEGIN(w1175aaac29b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 421)
    if (_r2.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1175aaac29b1c16->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c16, sizeof(XMLVM_JMP_BUF)); goto label65; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 422)

    
    // Red class access removed: java.net.InetAddress$CacheElement::inetAddress
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c16)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c16,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c16)
    label39:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 444)
    java_lang_Object_releaseLockRecursive__(_r6.o);
    XMLVM_EXIT_METHOD()
    return _r7.o;
    label41:;
    XMLVM_TRY_BEGIN(w1175aaac29b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 413)

    
    // Red class access removed: java.net.InetAddress$Cache::get
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r2.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1175aaac29b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c22, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 414)
    if (_r4.i <= 0) { XMLVM_MEMCPY(curThread_w1175aaac29b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c22, sizeof(XMLVM_JMP_BUF)); goto label33; };

    
    // Red class access removed: java.net.InetAddress$CacheElement,long::timeAdded
    XMLVM_RED_CLASS_DEPENDENCY();
    _r9.i = _r4.i * 1000;
    _r9.l = (JAVA_LONG) _r9.i;
    _r7.l = _r7.l + _r9.l;
    _r9.l = java_lang_System_currentTimeMillis__();
    _r7.i = _r7.l > _r9.l ? 1 : (_r7.l == _r9.l ? 0 : -1);
    if (_r7.i >= 0) { XMLVM_MEMCPY(curThread_w1175aaac29b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c22, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 418)
    _r2.o = JAVA_NULL;
    { XMLVM_MEMCPY(curThread_w1175aaac29b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c22, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 428)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c22)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c22,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c22)
    label65:;
    XMLVM_TRY_BEGIN(w1175aaac29b1c24)
    // Begin try

    
    // Red class access removed: java.net.NegativeCache::getFailedMessage
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r3.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1175aaac29b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c24, sizeof(XMLVM_JMP_BUF)); goto label80; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 429)
    XMLVM_SOURCE_POSITION("InetAddress.java", 430)
    _r7.o = __NEW_java_net_UnknownHostException();
    XMLVM_CHECK_NPE(7)
    java_net_UnknownHostException___INIT____java_lang_String(_r7.o, _r11.o);
    XMLVM_THROW_CUSTOM(_r7.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c24)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c24,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c24)
    label77:;
    java_lang_Thread* curThread_w1175aaac29b1c26 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r7.o = curThread_w1175aaac29b1c26->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r6.o);
    XMLVM_THROW_CUSTOM(_r7.o)
    label80:;
    XMLVM_TRY_BEGIN(w1175aaac29b1c30)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 435)
    _r7.i = java_net_InetAddress_preferIPv6Addresses__();
    _r0.o = java_net_InetAddress_getHostByNameImpl___java_lang_String_boolean(_r11.o, _r7.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c30)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c30,java_net_UnknownHostException,93)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c30,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c30)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c30)
    XMLVM_TRY_BEGIN(w1175aaac29b1c31)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 443)

    
    // Red class access removed: java.net.InetAddress$Cache::add
    XMLVM_RED_CLASS_DEPENDENCY();
    _r7 = _r0;
    { XMLVM_MEMCPY(curThread_w1175aaac29b1c31->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac29b1c31, sizeof(XMLVM_JMP_BUF)); goto label39; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 436)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c31)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c31,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c31)
    label93:;
    XMLVM_TRY_BEGIN(w1175aaac29b1c33)
    // Begin try
    java_lang_Thread* curThread_w1175aaac29b1c33aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r7.o = curThread_w1175aaac29b1c33aa->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r7;
    XMLVM_SOURCE_POSITION("InetAddress.java", 438)
    //java_net_UnknownHostException_getMessage__[7]
    XMLVM_CHECK_NPE(1)
    _r7.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_UnknownHostException*) _r1.o)->tib->vtable[7])(_r1.o);

    
    // Red class access removed: java.net.NegativeCache::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 440)
    _r7.o = __NEW_java_net_UnknownHostException();
    XMLVM_CHECK_NPE(7)
    java_net_UnknownHostException___INIT____java_lang_String(_r7.o, _r11.o);
    XMLVM_CHECK_NPE(7)
    _r11.o = java_lang_Throwable_initCause___java_lang_Throwable(_r7.o, _r1.o);
    _r11.o = _r11.o;
    XMLVM_THROW_CUSTOM(_r11.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac29b1c33)
        XMLVM_CATCH_SPECIFIC(w1175aaac29b1c33,java_lang_Object,77)
    XMLVM_CATCH_END(w1175aaac29b1c33)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac29b1c33)
    label114:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 406)
    java_lang_Thread* curThread_w1175aaac29b1c36 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r7.o = curThread_w1175aaac29b1c36->fields.java_lang_Thread.xmlvmException_;
    goto label27;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_net_InetAddress_getAliasesByNameImpl___java_lang_String(JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_net_InetAddress_getHostByAddrImpl___byte_1ARRAY(JAVA_OBJECT n1)]

JAVA_INT java_net_InetAddress_inetAddr___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_inetAddr___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "inetAddr", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 471)
    // "255.255.255.255"
    _r0.o = xmlvm_create_java_string_from_pool(73);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[1])(_r1.o, _r0.o);
    if (_r0.i == 0) goto label10;
    _r0.i = -1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = java_net_InetAddress_inetAddrImpl___java_lang_String(_r1.o);
    goto label9;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_net_InetAddress_inetAddrImpl___java_lang_String(JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_net_InetAddress_inetNtoaImpl___int(JAVA_INT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_net_InetAddress_getHostByNameImpl___java_lang_String_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2)]

//XMLVM_NATIVE[JAVA_OBJECT java_net_InetAddress_getHostNameImpl__()]

JAVA_OBJECT java_net_InetAddress_getHostNameInternal___java_lang_String_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getHostNameInternal___java_lang_String_boolean]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getHostNameInternal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("InetAddress.java", 511)
    if (_r2.o == JAVA_NULL) goto label8;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r1.i != 0) goto label15;
    label8:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 512)
    _r1.o = java_net_InetAddress_GET_LOOPBACK();
    //java_net_InetAddress_getHostAddress__[6]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r1.o)->tib->vtable[6])(_r1.o);
    label14:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 523)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label15:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 514)
    _r1.i = java_net_InetAddress_isHostName___java_lang_String(_r2.o);
    if (_r1.i == 0) goto label42;
    XMLVM_SOURCE_POSITION("InetAddress.java", 515)
    if (_r3.i == 0) goto label33;
    XMLVM_SOURCE_POSITION("InetAddress.java", 516)
    _r0.o = java_lang_System_getSecurityManager__();
    if (_r0.o == JAVA_NULL) goto label33;
    XMLVM_SOURCE_POSITION("InetAddress.java", 517)
    XMLVM_SOURCE_POSITION("InetAddress.java", 518)
    _r1.i = -1;
    XMLVM_CHECK_NPE(0)
    java_lang_SecurityManager_checkConnect___java_lang_String_int(_r0.o, _r2.o, _r1.i);
    label33:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 521)
    _r1.o = java_net_InetAddress_lookupHostByName___java_lang_String(_r2.o);
    //java_net_InetAddress_getHostAddress__[6]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r1.o)->tib->vtable[6])(_r1.o);
    goto label14;
    label42:;
    _r1 = _r2;
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_toString__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 534)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.hostName_;
    if (_r1.o != JAVA_NULL) goto label34;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    label11:;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "/"
    _r1.o = xmlvm_create_java_string_from_pool(54);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_net_InetAddress_getHostAddress__[6]
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r2.o)->tib->vtable[6])(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label34:;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.hostName_;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isHostName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isHostName___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isHostName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 627)

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::isValidIPV4Address
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i != 0) goto label14;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::isValidIP6Address
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i != 0) goto label14;
    _r0.i = 1;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 0;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isLoopbackAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isLoopbackAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isLoopbackAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 640)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isLinkLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isLinkLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isLinkLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 656)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isSiteLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isSiteLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isSiteLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 672)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMCGlobal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMCGlobal__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMCGlobal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 690)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMCNodeLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMCNodeLocal__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMCNodeLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 707)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMCLinkLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMCLinkLocal__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMCLinkLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 724)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMCSiteLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMCSiteLocal__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMCSiteLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 742)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isMCOrgLocal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isMCOrgLocal__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isMCOrgLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 760)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isAnyLocalAddress__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isAnyLocalAddress__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isAnyLocalAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 771)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachable___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachable___int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachable", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 790)
    _r0.o = JAVA_NULL;
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    _r0.i = java_net_InetAddress_isReachable___java_net_NetworkInterface_int_int(_r2.o, _r0.o, _r1.i, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachable___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachable___java_net_NetworkInterface_int_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachable", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("InetAddress.java", 815)
    if (_r5.i < 0) goto label5;
    if (_r6.i >= 0) goto label17;
    label5:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 816)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.61"
    _r2.o = xmlvm_create_java_string_from_pool(74);
    _r2.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r2.o);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    label17:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 818)
    _r0.i = 0;
    if (_r4.o != JAVA_NULL) goto label34;
    XMLVM_SOURCE_POSITION("InetAddress.java", 819)
    XMLVM_SOURCE_POSITION("InetAddress.java", 821)
    _r1.o = java_net_InetAddress_GET_NETIMPL();
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_INetworkSystem_isReachableByICMP___java_net_InetAddress_java_net_InetAddress_int_int])(_r1.o, _r3.o, _r2.o, _r5.i, _r6.i);
    if (_r0.i != 0) goto label32;
    XMLVM_SOURCE_POSITION("InetAddress.java", 822)
    XMLVM_SOURCE_POSITION("InetAddress.java", 823)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_net_InetAddress_isReachableByTCP___java_net_InetAddress_java_net_InetAddress_int(_r3.o, _r3.o, _r2.o, _r6.i);
    label32:;
    _r1 = _r0;
    label33:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 838)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label34:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 827)

    
    // Red class access removed: java.net.NetworkInterface,java.net.InetAddress[]::addresses
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r1.o != JAVA_NULL) goto label40;
    XMLVM_SOURCE_POSITION("InetAddress.java", 828)
    _r1.i = 0;
    goto label33;
    label40:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 832)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_net_InetAddress_isReachableByICMPUseMultiThread___java_net_NetworkInterface_int_int(_r3.o, _r4.o, _r5.i, _r6.i);
    if (_r0.i != 0) goto label32;
    XMLVM_SOURCE_POSITION("InetAddress.java", 833)
    XMLVM_SOURCE_POSITION("InetAddress.java", 835)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_net_InetAddress_isReachableByTCPUseMultiThread___java_net_NetworkInterface_int_int(_r3.o, _r4.o, _r5.i, _r6.i);
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachableByMultiThread___java_net_NetworkInterface_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachableByMultiThread___java_net_NetworkInterface_int_int_boolean]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachableByMultiThread", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    volatile XMLVMElem _r11;
    volatile XMLVMElem _r12;
    volatile XMLVMElem _r13;
    volatile XMLVMElem _r14;
    volatile XMLVMElem _r15;
    _r11.o = me;
    _r12.o = n1;
    _r13.i = n2;
    _r14.i = n3;
    _r15.i = n4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 848)

    
    // Red class access removed: java.net.NetworkInterface,java.net.InetAddress[]::addresses
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("InetAddress.java", 849)
    _r0.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 938)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 851)

    
    // Red class access removed: java.net.NetworkInterface::getInetAddresses
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 852)
    XMLVM_CHECK_NPE(11)
    ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.reached_ = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 853)

    
    // Red class access removed: java.net.NetworkInterface,java.net.InetAddress[]::addresses
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(11)
    ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.addrCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 854)
    _r9.i = 0;
    label19:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 855)
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r7.o);
    if (_r0.i == 0) goto label112;
    XMLVM_SOURCE_POSITION("InetAddress.java", 856)
    XMLVM_CHECK_NPE(7)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r7.o);
    _r3.o = _r3.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 859)
    //java_net_InetAddress_isLoopbackAddress__[9]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r3.o)->tib->vtable[9])(_r3.o);
    if (_r0.i == 0) goto label98;
    XMLVM_SOURCE_POSITION("InetAddress.java", 860)

    
    // Red class access removed: java.net.NetworkInterface::getNetworkInterfaces
    XMLVM_RED_CLASS_DEPENDENCY();
    label41:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 862)
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r6.o);
    if (_r0.i == 0) goto label75;
    XMLVM_SOURCE_POSITION("InetAddress.java", 863)
    XMLVM_CHECK_NPE(6)
    _r10.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r6.o);

    
    // Red class access removed: java.net.NetworkInterface::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 865)

    
    // Red class access removed: java.net.NetworkInterface::getInetAddresses
    XMLVM_RED_CLASS_DEPENDENCY();
    label57:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 867)
    XMLVM_CHECK_NPE(8)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r8.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r8.o);
    if (_r0.i == 0) goto label41;
    XMLVM_SOURCE_POSITION("InetAddress.java", 868)
    XMLVM_CHECK_NPE(8)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r8.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r8.o);
    //java_net_InetAddress_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(11)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_net_InetAddress*) _r11.o)->tib->vtable[1])(_r11.o, _r0.o);
    if (_r0.i == 0) goto label57;
    XMLVM_SOURCE_POSITION("InetAddress.java", 870)
    _r0.i = 1;
    goto label5;
    label75:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 875)
    XMLVM_CHECK_NPE(11)
    _r0.o = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.waitReachable_;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w1175aaac51b1c62)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 876)
    XMLVM_CHECK_NPE(11)
    _r1.i = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.addrCount_;
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(11)
    ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.addrCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 878)
    XMLVM_CHECK_NPE(11)
    _r1.i = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.addrCount_;
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w1175aaac51b1c62->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac51b1c62, sizeof(XMLVM_JMP_BUF)); goto label93; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 881)
    XMLVM_CHECK_NPE(11)
    _r1.o = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.waitReachable_;
    XMLVM_CHECK_NPE(1)
    java_lang_Object_notifyAll__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c62)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c62,java_lang_Object,95)
    XMLVM_CATCH_END(w1175aaac51b1c62)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c62)
    label93:;
    XMLVM_TRY_BEGIN(w1175aaac51b1c64)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 883)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    { XMLVM_MEMCPY(curThread_w1175aaac51b1c64->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac51b1c64, sizeof(XMLVM_JMP_BUF)); goto label19; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c64)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c64,java_lang_Object,95)
    XMLVM_CATCH_END(w1175aaac51b1c64)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c64)
    label95:;
    XMLVM_TRY_BEGIN(w1175aaac51b1c66)
    // Begin try
    java_lang_Thread* curThread_w1175aaac51b1c66aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w1175aaac51b1c66aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c66)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c66,java_lang_Object,95)
    XMLVM_CATCH_END(w1175aaac51b1c66)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c66)
    XMLVM_THROW_CUSTOM(_r1.o)
    label98:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 887)
    _r9.i = 1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 888)

    
    // Red class access removed: java.net.InetAddress$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1 = _r11;
    _r2 = _r15;
    _r4 = _r13;
    _r5 = _r14;

    
    // Red class access removed: java.net.InetAddress$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.InetAddress$1::start
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label19;
    label112:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 924)
    if (_r9.i == 0) goto label143;
    XMLVM_SOURCE_POSITION("InetAddress.java", 925)
    XMLVM_CHECK_NPE(11)
    _r0.o = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.waitReachable_;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    label117:;
    XMLVM_TRY_BEGIN(w1175aaac51b1c87)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 927)
    XMLVM_CHECK_NPE(11)
    _r1.i = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.reached_;
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w1175aaac51b1c87->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac51b1c87, sizeof(XMLVM_JMP_BUF)); goto label134; };
    XMLVM_CHECK_NPE(11)
    _r1.i = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.addrCount_;
    if (_r1.i == 0) { XMLVM_MEMCPY(curThread_w1175aaac51b1c87->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac51b1c87, sizeof(XMLVM_JMP_BUF)); goto label134; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 929)
    XMLVM_CHECK_NPE(11)
    _r1.o = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.waitReachable_;
    _r2.l = 1000;
    XMLVM_CHECK_NPE(1)
    java_lang_Object_wait___long(_r1.o, _r2.l);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c87)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c87,java_lang_Object,140)
    XMLVM_CATCH_END(w1175aaac51b1c87)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c87)
    goto label117;
    label133:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 931)
    java_lang_Thread* curThread_w1175aaac51b1c91 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w1175aaac51b1c91->fields.java_lang_Thread.xmlvmException_;
    label134:;
    XMLVM_TRY_BEGIN(w1175aaac51b1c93)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 934)
    XMLVM_CHECK_NPE(11)
    _r1.i = ((java_net_InetAddress*) _r11.o)->fields.java_net_InetAddress.reached_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    _r0 = _r1;
    { XMLVM_MEMCPY(curThread_w1175aaac51b1c93->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1175aaac51b1c93, sizeof(XMLVM_JMP_BUF)); goto label5; };
    XMLVM_SOURCE_POSITION("InetAddress.java", 935)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c93)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c93,java_lang_Object,140)
    XMLVM_CATCH_END(w1175aaac51b1c93)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c93)
    label140:;
    XMLVM_TRY_BEGIN(w1175aaac51b1c95)
    // Begin try
    java_lang_Thread* curThread_w1175aaac51b1c95aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w1175aaac51b1c95aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac51b1c95)
        XMLVM_CATCH_SPECIFIC(w1175aaac51b1c95,java_lang_Object,140)
    XMLVM_CATCH_END(w1175aaac51b1c95)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac51b1c95)
    XMLVM_THROW_CUSTOM(_r1.o)
    label143:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachableByICMPUseMultiThread___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachableByICMPUseMultiThread___java_net_NetworkInterface_int_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachableByICMPUseMultiThread", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 943)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_net_InetAddress_isReachableByMultiThread___java_net_NetworkInterface_int_int_boolean(_r1.o, _r2.o, _r3.i, _r4.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachableByTCPUseMultiThread___java_net_NetworkInterface_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachableByTCPUseMultiThread___java_net_NetworkInterface_int_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachableByTCPUseMultiThread", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 948)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_net_InetAddress_isReachableByMultiThread___java_net_NetworkInterface_int_int_boolean(_r1.o, _r2.o, _r3.i, _r4.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isReachableByTCP___java_net_InetAddress_java_net_InetAddress_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isReachableByTCP___java_net_InetAddress_java_net_InetAddress_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isReachableByTCP", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    volatile XMLVMElem _r11;
    _r8.o = me;
    _r9.o = n1;
    _r10.o = n2;
    _r11.i = n3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 953)
    _r1.o = __NEW_java_io_FileDescriptor();
    XMLVM_CHECK_NPE(1)
    java_io_FileDescriptor___INIT___(_r1.o);
    _r4.i = 0;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 957)
    _r0.o = java_net_InetAddress_GET_NETIMPL();

    
    // Red class access removed: org.apache.harmony.luni.net.NetUtil::preferIPv4Stack
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_BOOLEAN)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_INetworkSystem_createStreamSocket___java_io_FileDescriptor_boolean])(_r0.o, _r1.o, _r2.i);
    XMLVM_SOURCE_POSITION("InetAddress.java", 959)
    if (_r10.o == JAVA_NULL) goto label24;
    XMLVM_TRY_BEGIN(w1175aaac54b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 960)
    _r0.o = java_net_InetAddress_GET_NETIMPL();
    _r2.i = 0;
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_INetworkSystem_bind___java_io_FileDescriptor_java_net_InetAddress_int])(_r0.o, _r1.o, _r10.o, _r2.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac54b1c16)
        XMLVM_CATCH_SPECIFIC(w1175aaac54b1c16,java_io_IOException,39)
    XMLVM_CATCH_END(w1175aaac54b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac54b1c16)
    label24:;
    XMLVM_TRY_BEGIN(w1175aaac54b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 962)
    _r0.o = java_net_InetAddress_GET_NETIMPL();
    _r2.i = 7;
    _r3 = _r11;
    _r5 = _r9;
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_INetworkSystem_connectStreamWithTimeoutSocket___java_io_FileDescriptor_int_int_int_java_net_InetAddress])(_r0.o, _r1.o, _r2.i, _r3.i, _r4.i, _r5.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac54b1c18)
        XMLVM_CATCH_SPECIFIC(w1175aaac54b1c18,java_io_IOException,39)
    XMLVM_CATCH_END(w1175aaac54b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac54b1c18)
    XMLVM_SOURCE_POSITION("InetAddress.java", 964)
    _r7.i = 1;
    label33:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 972)
    _r0.o = java_net_InetAddress_GET_NETIMPL();
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_INetworkSystem_socketClose___java_io_FileDescriptor])(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 974)
    XMLVM_EXIT_METHOD()
    return _r7.i;
    label39:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 965)
    java_lang_Thread* curThread_w1175aaac54b1c29 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1175aaac54b1c29->fields.java_lang_Thread.xmlvmException_;
    _r6 = _r0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 966)
    // "Connection refused"
    _r0.o = xmlvm_create_java_string_from_pool(69);
    //java_io_IOException_getMessage__[7]
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_io_IOException*) _r6.o)->tib->vtable[7])(_r6.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r2.o);
    if (_r0.i == 0) goto label33;
    XMLVM_SOURCE_POSITION("InetAddress.java", 968)
    _r7.i = 1;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getByAddress___byte_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getByAddress___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getByAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 997)
    _r0.i = 0;
    _r0.o = java_net_InetAddress_getByAddress___byte_1ARRAY_int(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getByAddress___byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getByAddress___byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getByAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = n1;
    _r6.i = n2;
    _r4.i = 4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1018)
    if (_r5.o == JAVA_NULL) goto label24;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r2.i != _r4.i) goto label24;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1019)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r4.i);
    _r1.i = 0;
    label9:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1020)
    if (_r1.i >= _r4.i) goto label18;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1021)
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = _r1.i + 1;
    goto label9;
    label18:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1023)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label23:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1038)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label24:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1026)
    if (_r5.o == JAVA_NULL) goto label69;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    _r3.i = 16;
    if (_r2.i != _r3.i) goto label69;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1030)
    _r2.i = java_net_InetAddress_isIPv4MappedAddress___byte_1ARRAY(_r5.o);
    if (_r2.i == 0) goto label57;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1031)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r4.i);
    _r1.i = 0;
    label40:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1032)
    if (_r1.i >= _r4.i) goto label51;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1033)
    _r2.i = _r1.i + 12;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = _r1.i + 1;
    goto label40;
    label51:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1035)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label23;
    label57:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1037)
    //byte_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r5.o)->tib->vtable[0])(_r5.o);
    _r0.o = _r0.o;
    _r2.o = __NEW_java_net_Inet6Address();
    XMLVM_CHECK_NPE(2)
    java_net_Inet6Address___INIT____byte_1ARRAY_int(_r2.o, _r0.o, _r6.i);
    goto label23;
    label69:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1042)
    _r2.o = __NEW_java_net_UnknownHostException();
    // "luni.64"
    _r3.o = xmlvm_create_java_string_from_pool(75);
    _r3.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r3.o);
    XMLVM_CHECK_NPE(2)
    java_net_UnknownHostException___INIT____java_lang_String(_r2.o, _r3.o);
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_isIPv4MappedAddress___byte_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_isIPv4MappedAddress___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "isIPv4MappedAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = n1;
    _r4.i = 10;
    _r3.i = 0;
    _r2.i = -1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1049)
    _r0.i = 0;
    label5:;
    if (_r0.i >= _r4.i) goto label16;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1050)
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.i == 0) goto label13;
    _r1 = _r3;
    label12:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1051)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1059)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label13:;
    _r0.i = _r0.i + 1;
    goto label5;
    label16:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1055)
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r4.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    if (_r1.i != _r2.i) goto label26;
    _r1.i = 11;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r1.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r1.i == _r2.i) goto label28;
    label26:;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1056)
    goto label12;
    label28:;
    _r1.i = 1;
    goto label12;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getByAddress___java_lang_String_byte_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getByAddress___java_lang_String_byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getByAddress", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1088)
    _r0.i = 0;
    _r0.o = java_net_InetAddress_getByAddressInternal___java_lang_String_byte_1ARRAY_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_getByAddressInternal___java_lang_String_byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_getByAddressInternal___java_lang_String_byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "getByAddressInternal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = n1;
    _r6.o = n2;
    _r7.i = n3;
    _r4.i = 16;
    _r3.i = 4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1112)
    if (_r6.o == JAVA_NULL) goto label26;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r2.i != _r3.i) goto label26;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1113)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r3.i);
    _r1.i = 0;
    label11:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1114)
    if (_r1.i >= _r3.i) goto label20;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1115)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = _r1.i + 1;
    goto label11;
    label20:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1117)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label25:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1137)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label26:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1120)
    if (_r6.o == JAVA_NULL) goto label75;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r2.i != _r4.i) goto label75;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1124)
    _r2.i = java_net_InetAddress_isIPv4MappedAddress___byte_1ARRAY(_r6.o);
    if (_r2.i == 0) goto label57;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1125)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r3.i);
    _r1.i = 0;
    label40:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1126)
    if (_r1.i >= _r3.i) goto label51;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1127)
    _r2.i = _r1.i + 12;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = _r1.i + 1;
    goto label40;
    label51:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1129)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label25;
    label57:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1132)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r4.i);
    _r1.i = 0;
    label60:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1133)
    if (_r1.i >= _r4.i) goto label69;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1134)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = _r1.i + 1;
    goto label60;
    label69:;
    _r2.o = __NEW_java_net_Inet6Address();
    XMLVM_CHECK_NPE(2)
    java_net_Inet6Address___INIT____byte_1ARRAY_java_lang_String_int(_r2.o, _r6.o, _r5.o, _r7.i);
    goto label25;
    label75:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1140)
    _r2.o = __NEW_java_net_UnknownHostException();
    // "luni.65"
    _r3.o = xmlvm_create_java_string_from_pool(76);
    _r3.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object(_r3.o, _r5.o);
    XMLVM_CHECK_NPE(2)
    java_net_UnknownHostException___INIT____java_lang_String(_r2.o, _r3.o);
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress_intToBytes___int_byte_1ARRAY_int(JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_intToBytes___int_byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "intToBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.i = n1;
    _r3.o = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1151)
    _r0.i = _r2.i >> 24;
    _r0.i = _r0.i & 255;
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r0.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1152)
    _r0.i = _r4.i + 1;
    _r1.i = _r2.i >> 16;
    _r1.i = _r1.i & 255;
    _r1.i = (_r1.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1153)
    _r0.i = _r4.i + 2;
    _r1.i = _r2.i >> 8;
    _r1.i = _r1.i & 255;
    _r1.i = (_r1.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1154)
    _r0.i = _r4.i + 3;
    _r1.i = _r2.i & 255;
    _r1.i = (_r1.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1155)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_InetAddress_bytesToInt___byte_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_bytesToInt___byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "bytesToInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1169)
    _r1.i = _r4.i + 3;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r1.i = _r1.i & 255;
    _r2.i = _r4.i + 2;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r2.i = _r2.i & 255;
    _r2.i = _r2.i << 8;
    _r1.i = _r1.i | _r2.i;
    _r2.i = _r4.i + 1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r2.i = _r2.i & 255;
    _r2.i = _r2.i << 16;
    _r1.i = _r1.i | _r2.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r2.i = _r2.i & 255;
    _r2.i = _r2.i << 24;
    _r0.i = _r1.i | _r2.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1173)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_createHostNameFromIPAddress___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_createHostNameFromIPAddress___java_lang_String]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "createHostNameFromIPAddress", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    volatile XMLVMElem _r11;
    _r11.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1183)
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1185)

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::isValidIPV4Address
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i == 0) goto label91;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1186)
    _r0.i = 4;
    XMLVM_CLASS_INIT(byte)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1187)
    // "\134."
    _r0.o = xmlvm_create_java_string_from_pool(77);
    XMLVM_CHECK_NPE(11)
    _r3.o = java_lang_String_split___java_lang_String(_r11.o, _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1188)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r0.i = 1;
    if (_r2.i != _r0.i) goto label44;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1189)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1190)
    _r0.i = 0;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r3.l = java_lang_Long_parseLong___java_lang_String(_r0.o);
    _r0.i = 0;
    label28:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1191)
    _r5.i = 4;
    if (_r0.i >= _r5.i) goto label59;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1192)
    _r5.i = 3;
    _r5.i = _r5.i - _r0.i;
    _r5.i = _r5.i * 8;
    _r5.l = _r3.l >> (0x3f & _r5.l);
    _r5.i = (JAVA_INT) _r5.l;
    _r5.i = (_r5.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r5.i;
    _r0.i = _r0.i + 1;
    goto label28;
    label44:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1195)
    _r0.i = 0;
    label45:;
    if (_r0.i >= _r2.i) goto label59;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1196)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r4.i = java_lang_Integer_parseInt___java_lang_String(_r4.o);
    _r4.i = (_r4.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r4.i;
    _r0.i = _r0.i + 1;
    goto label45;
    label59:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1201)
    _r0.i = 2;
    if (_r2.i != _r0.i) goto label72;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1202)
    _r0.i = 3;
    _r3.i = 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r3.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1203)
    _r0.i = 1;
    _r3.i = 0;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    label72:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1205)
    _r0.i = 3;
    if (_r2.i != _r0.i) goto label85;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1206)
    _r0.i = 3;
    _r2.i = 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1207)
    _r0.i = 2;
    _r2.i = 0;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    label85:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1210)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label90:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1211)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1341)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label91:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1213)
    _r0.i = 0;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(11)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r11.o)->tib->vtable[6])(_r11.o, _r0.i);
    _r1.i = 91;
    if (_r0.i != _r1.i) goto label111;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1214)
    _r0.i = 1;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(11)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r11.o)->tib->vtable[8])(_r11.o);
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(11)
    _r11.o = java_lang_String_substring___int_int(_r11.o, _r0.i, _r1.i);
    label111:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1218)
    _r8.o = __NEW_java_util_StringTokenizer();
    // ":.%"
    _r0.o = xmlvm_create_java_string_from_pool(78);
    _r1.i = 1;
    XMLVM_CHECK_NPE(8)
    java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean(_r8.o, _r11.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1220)
    _r3.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(3)
    java_util_ArrayList___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1221)
    _r1.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(1)
    java_util_ArrayList___INIT___(_r1.o);
    _r5.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1223)
    // ""
    _r6.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1224)
    // ""
    _r4.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1225)
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);
    _r0.i = -1;
    _r9 = _r0;
    _r0 = _r2;
    _r2 = _r9;
    _r10 = _r6;
    _r6 = _r5;
    _r5 = _r10;
    label143:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1226)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1234)
    XMLVM_CHECK_NPE(8)
    _r0.i = java_util_StringTokenizer_hasMoreTokens__(_r8.o);
    if (_r0.i == 0) goto label286;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1235)
    _r4 = _r4;
    _r5 = _r5;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1237)
    XMLVM_CHECK_NPE(8)
    _r7.o = java_util_StringTokenizer_nextToken__(_r8.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1239)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r0.o);
    if (_r0.i == 0) goto label195;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1240)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r0.o);
    if (_r0.i == 0) goto label180;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1241)
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[16])(_r3.o);
    _r2 = _r0;
    _r0 = _r4;
    _r4 = _r5;
    _r5 = _r7;
    goto label143;
    label180:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1242)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r0.o);
    if (_r0.i != 0) goto label476;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1243)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[7])(_r3.o, _r5.o);
    _r0 = _r4;
    _r4 = _r5;
    _r5 = _r7;
    goto label143;
    label195:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1245)
    // "."
    _r0.o = xmlvm_create_java_string_from_pool(52);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r0.o);
    if (_r0.i == 0) goto label210;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1246)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[7])(_r1.o, _r5.o);
    _r0 = _r4;
    _r4 = _r5;
    _r5 = _r7;
    goto label143;
    label210:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1247)
    // "%"
    _r0.o = xmlvm_create_java_string_from_pool(80);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r0.o);
    if (_r0.i == 0) goto label476;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1249)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r0.o);
    if (_r0.i != 0) goto label245;
    // "."
    _r0.o = xmlvm_create_java_string_from_pool(52);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r0.o);
    if (_r0.i != 0) goto label245;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1250)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[1])(_r4.o, _r0.o);
    if (_r0.i == 0) goto label264;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1251)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[7])(_r3.o, _r5.o);
    label245:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1258)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    label250:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1259)
    XMLVM_CHECK_NPE(8)
    _r6.i = java_util_StringTokenizer_hasMoreTokens__(_r8.o);
    if (_r6.i == 0) goto label276;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1260)
    XMLVM_CHECK_NPE(8)
    _r6.o = java_util_StringTokenizer_nextToken__(_r8.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r6.o);
    goto label250;
    label264:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1252)
    // "."
    _r0.o = xmlvm_create_java_string_from_pool(52);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[1])(_r4.o, _r0.o);
    if (_r0.i == 0) goto label245;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1253)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[7])(_r1.o, _r5.o);
    goto label245;
    label276:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1262)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    _r6 = _r0;
    _r0 = _r4;
    _r4 = _r5;
    _r5 = _r7;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1263)
    goto label143;
    label286:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1266)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[1])(_r4.o, _r0.o);
    if (_r0.i == 0) goto label341;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1267)
    // ":"
    _r0.o = xmlvm_create_java_string_from_pool(79);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r0.o);
    if (_r0.i == 0) goto label336;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1268)
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[16])(_r3.o);
    label306:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1278)
    _r2.i = 8;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1282)
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(1)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[16])(_r1.o);
    if (_r4.i <= 0) goto label316;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1283)
    _r2.i = _r2.i + -2;
    label316:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1287)
    _r4.i = -1;
    if (_r0.i == _r4.i) goto label354;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1288)
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(3)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[16])(_r3.o);
    _r4.i = _r2.i - _r4.i;
    _r2.i = 0;
    label326:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1289)
    if (_r2.i >= _r4.i) goto label354;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1290)
    // "0"
    _r5.o = xmlvm_create_java_string_from_pool(70);
    //java_util_ArrayList_add___int_java_lang_Object[20]
    XMLVM_CHECK_NPE(3)
    (*(void (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[20])(_r3.o, _r0.i, _r5.o);
    _r2.i = _r2.i + 1;
    goto label326;
    label336:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1270)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[7])(_r3.o, _r5.o);
    _r0 = _r2;
    goto label306;
    label341:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1272)
    // "."
    _r0.o = xmlvm_create_java_string_from_pool(52);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[1])(_r4.o, _r0.o);
    if (_r0.i == 0) goto label352;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1273)
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[7])(_r1.o, _r5.o);
    label352:;
    _r0 = _r2;
    goto label306;
    label354:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1294)
    _r0.i = 16;
    XMLVM_CLASS_INIT(byte)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    _r0.i = 0;
    label359:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1297)
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(3)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[16])(_r3.o);
    if (_r0.i >= _r4.i) goto label379;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1298)
    //java_util_ArrayList_get___int[21]
    XMLVM_CHECK_NPE(3)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[21])(_r3.o, _r0.i);
    _r4.o = _r4.o;
    _r5.i = _r0.i * 2;

    
    // Red class access removed: org.apache.harmony.luni.util.Inet6Util::convertToBytes
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = _r0.i + 1;
    goto label359;
    label379:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1302)
    _r0.i = 0;
    label380:;
    //java_util_ArrayList_size__[16]
    XMLVM_CHECK_NPE(1)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[16])(_r1.o);
    if (_r0.i >= _r3.i) goto label406;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1303)
    _r4.i = _r0.i + 12;
    //java_util_ArrayList_get___int[21]
    XMLVM_CHECK_NPE(1)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[21])(_r1.o, _r0.i);
    _r3.o = _r3.o;
    _r3.i = java_lang_Integer_parseInt___java_lang_String(_r3.o);
    _r3.i = _r3.i & 255;
    _r3.i = (_r3.i << 24) >> 24;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r4.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r3.i;
    _r0.i = _r0.i + 1;
    goto label380;
    label406:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1309)
    _r1.i = 1;
    _r0.i = 0;
    label408:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1310)
    _r3.i = 10;
    if (_r0.i >= _r3.i) goto label474;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1311)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r3.i == 0) goto label450;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1312)
    _r0.i = 0;
    label417:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1313)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1317)
    _r1.i = 10;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = -1;
    if (_r1.i != _r3.i) goto label431;
    _r1.i = 11;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = -1;
    if (_r1.i == _r3.i) goto label432;
    label431:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1318)
    _r0.i = 0;
    label432:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1321)
    if (_r0.i == 0) goto label459;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1322)
    _r0.i = 4;
    XMLVM_CLASS_INIT(byte)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    _r0.i = 0;
    label438:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1323)
    _r3.i = 4;
    if (_r0.i >= _r3.i) goto label453;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1324)
    _r3.i = _r0.i + 12;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    _r0.i = _r0.i + 1;
    goto label438;
    label450:;
    _r0.i = _r0.i + 1;
    goto label408;
    label453:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1326)
    _r0.o = java_net_InetAddress_getByAddress___byte_1ARRAY(_r1.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1327)
    goto label90;
    label459:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1328)
    _r0.i = 0;
    if (_r6.o == JAVA_NULL) goto label466;
    XMLVM_TRY_BEGIN(w1175aaac62b1d353)
    // Begin try
    XMLVM_SOURCE_POSITION("InetAddress.java", 1329)
    XMLVM_SOURCE_POSITION("InetAddress.java", 1331)
    _r0.i = java_lang_Integer_parseInt___java_lang_String(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1175aaac62b1d353)
        XMLVM_CATCH_SPECIFIC(w1175aaac62b1d353,java_lang_Exception,472)
    XMLVM_CATCH_END(w1175aaac62b1d353)
    XMLVM_RESTORE_EXCEPTION_ENV(w1175aaac62b1d353)
    label466:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1337)
    _r0.o = java_net_InetAddress_getByAddress___byte_1ARRAY_int(_r2.o, _r0.i);
    goto label90;
    label472:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1332)
    java_lang_Thread* curThread_w1175aaac62b1d360 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w1175aaac62b1d360->fields.java_lang_Thread.xmlvmException_;
    goto label466;
    label474:;
    _r0 = _r1;
    goto label417;
    label476:;
    _r0 = _r4;
    _r4 = _r5;
    _r5 = _r7;
    goto label143;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_preferIPv6Addresses__()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_preferIPv6Addresses__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "preferIPv6Addresses", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1345)

    
    // Red class access removed: org.apache.harmony.luni.net.NetUtil::preferIPv6Addresses
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r2.i = 0;
    // "address"
    _r3.o = xmlvm_create_java_string_from_pool(81);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1354)

    
    // Red class access removed: java.io.ObjectOutputStream::putFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 1355)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    if (_r1.o != JAVA_NULL) goto label34;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1356)
    // "address"
    _r1.o = xmlvm_create_java_string_from_pool(81);

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    label16:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1360)
    // "family"
    _r1.o = xmlvm_create_java_string_from_pool(82);
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.family_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 1361)
    // "hostName"
    _r1.o = xmlvm_create_java_string_from_pool(83);
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.hostName_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 1363)

    
    // Red class access removed: java.io.ObjectOutputStream::writeFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 1364)
    XMLVM_EXIT_METHOD()
    return;
    label34:;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1358)
    // "address"
    _r1.o = xmlvm_create_java_string_from_pool(81);
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    _r1.i = java_net_InetAddress_bytesToInt___byte_1ARRAY_int(_r1.o, _r2.i);

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label16;
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1368)

    
    // Red class access removed: java.io.ObjectInputStream::readFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("InetAddress.java", 1369)
    // "address"
    _r2.o = xmlvm_create_java_string_from_pool(81);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1370)
    XMLVM_CLASS_INIT(byte)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r2.i);
    XMLVM_CHECK_NPE(4)
    ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_ = _r2.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1371)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.ipaddress_;
    java_net_InetAddress_intToBytes___int_byte_1ARRAY_int(_r0.i, _r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1372)
    // "hostName"
    _r2.o = xmlvm_create_java_string_from_pool(83);
    _r3.o = JAVA_NULL;

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(4)
    ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.hostName_ = _r2.o;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1373)
    // "family"
    _r2.o = xmlvm_create_java_string_from_pool(82);
    _r3.i = 2;

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_net_InetAddress*) _r4.o)->fields.java_net_InetAddress.family_ = _r2.i;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1374)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_readResolve__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_readResolve__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "readResolve", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("InetAddress.java", 1381)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.ipaddress_;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_net_InetAddress*) _r3.o)->fields.java_net_InetAddress.hostName_;

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_access$100__()
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$100__]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$100", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    _r0.o = java_net_InetAddress_GET_NETIMPL();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_access$200___java_net_InetAddress_java_net_InetAddress_java_net_InetAddress_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$200___java_net_InetAddress_java_net_InetAddress_java_net_InetAddress_int]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$200", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    _r4.i = n4;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_net_InetAddress_isReachableByTCP___java_net_InetAddress_java_net_InetAddress_int(_r1.o, _r2.o, _r3.o, _r4.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_net_InetAddress_access$300___java_net_InetAddress(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$300___java_net_InetAddress]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$300", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.waitReachable_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_net_InetAddress_access$402___java_net_InetAddress_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$402___java_net_InetAddress_boolean]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$402", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = n1;
    _r1.i = n2;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    XMLVM_CHECK_NPE(0)
    ((java_net_InetAddress*) _r0.o)->fields.java_net_InetAddress.reached_ = _r1.i;
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_InetAddress_access$510___java_net_InetAddress(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$510___java_net_InetAddress]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$510", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.addrCount_;
    _r1.i = 1;
    _r1.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(2)
    ((java_net_InetAddress*) _r2.o)->fields.java_net_InetAddress.addrCount_ = _r1.i;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_net_InetAddress_access$500___java_net_InetAddress(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_net_InetAddress)
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress_access$500___java_net_InetAddress]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "access$500", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 45)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_net_InetAddress*) _r1.o)->fields.java_net_InetAddress.addrCount_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_net_InetAddress___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_net_InetAddress___CLINIT___]
    XMLVM_ENTER_METHOD("java.net.InetAddress", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r1.i = 4;
    _r5.i = 1;
    XMLVM_SOURCE_POSITION("InetAddress.java", 47)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r1.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_BYTE[]){0, 0, 0, 0, });
    java_net_InetAddress_PUT_any_bytes( _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 49)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r1.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_BYTE[]){127, 0, 0, 1, });
    java_net_InetAddress_PUT_localhost_bytes( _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 51)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = java_net_InetAddress_GET_any_bytes();

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    java_net_InetAddress_PUT_ANY( _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 53)

    
    // Red class access removed: org.apache.harmony.luni.platform.Platform::getNetworkSystem
    XMLVM_RED_CLASS_DEPENDENCY();
    java_net_InetAddress_PUT_NETIMPL( _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 55)

    
    // Red class access removed: java.net.Inet4Address::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = java_net_InetAddress_GET_localhost_bytes();
    // "localhost"
    _r2.o = xmlvm_create_java_string_from_pool(71);

    
    // Red class access removed: java.net.Inet4Address::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    java_net_InetAddress_PUT_LOOPBACK( _r0.o);
    XMLVM_SOURCE_POSITION("InetAddress.java", 81)
    java_net_InetAddress_oneTimeInitialization___boolean(_r5.i);
    XMLVM_SOURCE_POSITION("InetAddress.java", 1348)
    _r0.i = 3;
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_io_ObjectStreamField, _r0.i);
    _r1.i = 0;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "address"
    _r3.o = xmlvm_create_java_string_from_pool(81);
    _r4.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.o = __NEW_java_io_ObjectStreamField();
    // "family"
    _r2.o = xmlvm_create_java_string_from_pool(82);
    _r3.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(1)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r1.o;
    _r1.i = 2;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "hostName"
    _r3.o = xmlvm_create_java_string_from_pool(83);
    XMLVM_CLASS_INIT(java_lang_String)
    _r4.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    java_net_InetAddress_PUT_serialPersistentFields( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    label86:;
    label92:;
    //XMLVM_END_WRAPPER
}

