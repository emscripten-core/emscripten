#include "xmlvm.h"
#include "org_apache_harmony_luni_platform_Endianness.h"

#include "org_apache_harmony_luni_platform_IMemorySystem.h"

__TIB_DEFINITION_org_apache_harmony_luni_platform_IMemorySystem __TIB_org_apache_harmony_luni_platform_IMemorySystem = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_platform_IMemorySystem, // classInitializer
    "org.apache.harmony.luni.platform.IMemorySystem", // className
    "org.apache.harmony.luni.platform", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IMemorySystem;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IMemorySystem_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IMemorySystem_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IMemorySystem_3ARRAY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_ONLY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_WRITE;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_WRITE_COPY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_org_apache_harmony_luni_platform_IMemorySystem()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_platform_IMemorySystem);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_platform_IMemorySystem.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_platform_IMemorySystem);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_platform_IMemorySystem.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_platform_IMemorySystem.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_platform_IMemorySystem.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.platform.IMemorySystem")
        __INIT_IMPL_org_apache_harmony_luni_platform_IMemorySystem();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_platform_IMemorySystem()
{
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.numInterfaces = 0;
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_ONLY = 1;
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_WRITE = 2;
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_WRITE_COPY = 4;
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_org_apache_harmony_luni_platform_IMemorySystem.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_org_apache_harmony_luni_platform_IMemorySystem = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_platform_IMemorySystem);
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.clazz = __CLASS_org_apache_harmony_luni_platform_IMemorySystem;
    __TIB_org_apache_harmony_luni_platform_IMemorySystem.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_platform_IMemorySystem_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IMemorySystem);
    __CLASS_org_apache_harmony_luni_platform_IMemorySystem_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IMemorySystem_1ARRAY);
    __CLASS_org_apache_harmony_luni_platform_IMemorySystem_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IMemorySystem_2ARRAY);

    __TIB_org_apache_harmony_luni_platform_IMemorySystem.classInitialized = 1;
}

JAVA_INT org_apache_harmony_luni_platform_IMemorySystem_GET_MMAP_READ_ONLY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IMemorySystem)
    return _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_ONLY;
}

void org_apache_harmony_luni_platform_IMemorySystem_PUT_MMAP_READ_ONLY(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_ONLY = v;
}

JAVA_INT org_apache_harmony_luni_platform_IMemorySystem_GET_MMAP_READ_WRITE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IMemorySystem)
    return _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_WRITE;
}

void org_apache_harmony_luni_platform_IMemorySystem_PUT_MMAP_READ_WRITE(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_READ_WRITE = v;
}

JAVA_INT org_apache_harmony_luni_platform_IMemorySystem_GET_MMAP_WRITE_COPY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IMemorySystem)
    return _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_WRITE_COPY;
}

void org_apache_harmony_luni_platform_IMemorySystem_PUT_MMAP_WRITE_COPY(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IMemorySystem_MMAP_WRITE_COPY = v;
}

