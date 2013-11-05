#include "xmlvm.h"
#include "java_io_FileDescriptor.h"

#include "org_apache_harmony_luni_platform_IFileSystem.h"

__TIB_DEFINITION_org_apache_harmony_luni_platform_IFileSystem __TIB_org_apache_harmony_luni_platform_IFileSystem = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_platform_IFileSystem, // classInitializer
    "org.apache.harmony.luni.platform.IFileSystem", // className
    "org.apache.harmony.luni.platform", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IFileSystem;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IFileSystem_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IFileSystem_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_IFileSystem_3ARRAY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_SHARED_LOCK_TYPE;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_EXCLUSIVE_LOCK_TYPE;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_SET;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_END;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDONLY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_WRONLY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWR;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWRSYNC;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_APPEND;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_CREAT;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_EXCL;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NOCTTY;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NONBLOCK;
static JAVA_INT _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_TRUNC;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_org_apache_harmony_luni_platform_IFileSystem()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_platform_IFileSystem);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_platform_IFileSystem.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_platform_IFileSystem.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_platform_IFileSystem);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_platform_IFileSystem.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_platform_IFileSystem.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_platform_IFileSystem.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.platform.IFileSystem")
        __INIT_IMPL_org_apache_harmony_luni_platform_IFileSystem();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_platform_IFileSystem()
{
    __TIB_org_apache_harmony_luni_platform_IFileSystem.numInterfaces = 0;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SHARED_LOCK_TYPE = 1;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_EXCLUSIVE_LOCK_TYPE = 2;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_SET = 1;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR = 2;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_END = 4;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDONLY = 0;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_WRONLY = 1;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWR = 16;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWRSYNC = 32;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_APPEND = 256;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_CREAT = 4096;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_EXCL = 65536;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NOCTTY = 1048576;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NONBLOCK = 16777216;
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_TRUNC = 268435456;
    __TIB_org_apache_harmony_luni_platform_IFileSystem.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_IFileSystem.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_org_apache_harmony_luni_platform_IFileSystem.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_platform_IFileSystem.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_IFileSystem.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_org_apache_harmony_luni_platform_IFileSystem = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_platform_IFileSystem);
    __TIB_org_apache_harmony_luni_platform_IFileSystem.clazz = __CLASS_org_apache_harmony_luni_platform_IFileSystem;
    __TIB_org_apache_harmony_luni_platform_IFileSystem.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_platform_IFileSystem_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IFileSystem);
    __CLASS_org_apache_harmony_luni_platform_IFileSystem_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IFileSystem_1ARRAY);
    __CLASS_org_apache_harmony_luni_platform_IFileSystem_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_IFileSystem_2ARRAY);

    __TIB_org_apache_harmony_luni_platform_IFileSystem.classInitialized = 1;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_SHARED_LOCK_TYPE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_SHARED_LOCK_TYPE;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_SHARED_LOCK_TYPE(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SHARED_LOCK_TYPE = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_EXCLUSIVE_LOCK_TYPE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_EXCLUSIVE_LOCK_TYPE;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_EXCLUSIVE_LOCK_TYPE(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_EXCLUSIVE_LOCK_TYPE = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_SEEK_SET()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_SET;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_SEEK_SET(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_SET = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_SEEK_CUR()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_SEEK_CUR(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_SEEK_END()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_END;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_SEEK_END(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_SEEK_END = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_RDONLY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDONLY;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_RDONLY(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDONLY = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_WRONLY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_WRONLY;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_WRONLY(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_WRONLY = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_RDWR()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWR;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_RDWR(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWR = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_RDWRSYNC()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWRSYNC;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_RDWRSYNC(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_RDWRSYNC = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_APPEND()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_APPEND;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_APPEND(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_APPEND = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_CREAT()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_CREAT;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_CREAT(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_CREAT = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_EXCL()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_EXCL;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_EXCL(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_EXCL = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_NOCTTY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NOCTTY;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_NOCTTY(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NOCTTY = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_NONBLOCK()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NONBLOCK;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_NONBLOCK(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_NONBLOCK = v;
}

JAVA_INT org_apache_harmony_luni_platform_IFileSystem_GET_O_TRUNC()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IFileSystem)
    return _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_TRUNC;
}

void org_apache_harmony_luni_platform_IFileSystem_PUT_O_TRUNC(JAVA_INT v)
{
    _STATIC_org_apache_harmony_luni_platform_IFileSystem_O_TRUNC = v;
}

