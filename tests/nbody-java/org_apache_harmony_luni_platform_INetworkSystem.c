#include "xmlvm.h"
#include "java_io_FileDescriptor.h"
#include "java_lang_Long.h"
#include "java_lang_String.h"
#include "java_net_InetAddress.h"

#include "org_apache_harmony_luni_platform_INetworkSystem.h"

__TIB_DEFINITION_org_apache_harmony_luni_platform_INetworkSystem __TIB_org_apache_harmony_luni_platform_INetworkSystem = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_platform_INetworkSystem, // classInitializer
    "org.apache.harmony.luni.platform.INetworkSystem", // className
    "org.apache.harmony.luni.platform", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_INetworkSystem;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_INetworkSystem_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_INetworkSystem_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_INetworkSystem_3ARRAY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_START;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_CHECK;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_org_apache_harmony_luni_platform_INetworkSystem()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_platform_INetworkSystem);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_platform_INetworkSystem.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_platform_INetworkSystem);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_platform_INetworkSystem.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_platform_INetworkSystem.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_platform_INetworkSystem.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.platform.INetworkSystem")
        __INIT_IMPL_org_apache_harmony_luni_platform_INetworkSystem();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_platform_INetworkSystem()
{
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.numInterfaces = 0;
    _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_START = 0;
    _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_CHECK = 1;
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_org_apache_harmony_luni_platform_INetworkSystem.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_org_apache_harmony_luni_platform_INetworkSystem = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_platform_INetworkSystem);
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.clazz = __CLASS_org_apache_harmony_luni_platform_INetworkSystem;
    __TIB_org_apache_harmony_luni_platform_INetworkSystem.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_platform_INetworkSystem_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_INetworkSystem);
    __CLASS_org_apache_harmony_luni_platform_INetworkSystem_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_INetworkSystem_1ARRAY);
    __CLASS_org_apache_harmony_luni_platform_INetworkSystem_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_INetworkSystem_2ARRAY);

    __TIB_org_apache_harmony_luni_platform_INetworkSystem.classInitialized = 1;
}

JAVA_INT org_apache_harmony_luni_platform_INetworkSystem_GET_SOCKET_CONNECT_STEP_START()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_INetworkSystem)
    return _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_START;
}

void org_apache_harmony_luni_platform_INetworkSystem_PUT_SOCKET_CONNECT_STEP_START(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_START = v;
}

JAVA_INT org_apache_harmony_luni_platform_INetworkSystem_GET_SOCKET_CONNECT_STEP_CHECK()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_INetworkSystem)
    return _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_CHECK;
}

void org_apache_harmony_luni_platform_INetworkSystem_PUT_SOCKET_CONNECT_STEP_CHECK(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_INetworkSystem_SOCKET_CONNECT_STEP_CHECK = v;
}

