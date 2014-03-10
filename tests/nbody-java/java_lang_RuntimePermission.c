#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_lang_RuntimePermission.h"

#define XMLVM_CURRENT_CLASS_NAME RuntimePermission
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_RuntimePermission

__TIB_DEFINITION_java_lang_RuntimePermission __TIB_java_lang_RuntimePermission = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_RuntimePermission, // classInitializer
    "java.lang.RuntimePermission", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_security_BasicPermission, // extends
    sizeof(java_lang_RuntimePermission), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_RuntimePermission;
JAVA_OBJECT __CLASS_java_lang_RuntimePermission_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_RuntimePermission_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_RuntimePermission_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_RuntimePermission_serialVersionUID;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToSetSecurityManager;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToCreateSecurityManager;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToGetProtectionDomain;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToGetClassLoader;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToCreateClassLoader;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToModifyThread;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToModifyThreadGroup;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToExitVM;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToReadFileDescriptor;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToWriteFileDescriptor;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToQueuePrintJob;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToSetFactory;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToSetIO;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToStopThread;
static JAVA_OBJECT _STATIC_java_lang_RuntimePermission_permissionToSetContextClassLoader;

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

void __INIT_java_lang_RuntimePermission()
{
    staticInitializerLock(&__TIB_java_lang_RuntimePermission);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_RuntimePermission.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_RuntimePermission.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_RuntimePermission);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_RuntimePermission.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_RuntimePermission.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_RuntimePermission.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.RuntimePermission")
        __INIT_IMPL_java_lang_RuntimePermission();
    }
}

void __INIT_IMPL_java_lang_RuntimePermission()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_security_BasicPermission)
    __TIB_java_lang_RuntimePermission.newInstanceFunc = __NEW_INSTANCE_java_lang_RuntimePermission;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_RuntimePermission.vtable, __TIB_java_security_BasicPermission.vtable, sizeof(__TIB_java_security_BasicPermission.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_RuntimePermission.numImplementedInterfaces = 2;
    __TIB_java_lang_RuntimePermission.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_RuntimePermission.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_security_Guard)

    __TIB_java_lang_RuntimePermission.implementedInterfaces[0][1] = &__TIB_java_security_Guard;
    // Initialize itable for this class
    __TIB_java_lang_RuntimePermission.itableBegin = &__TIB_java_lang_RuntimePermission.itable[0];
    __TIB_java_lang_RuntimePermission.itable[XMLVM_ITABLE_IDX_java_security_Guard_checkGuard___java_lang_Object] = __TIB_java_lang_RuntimePermission.vtable[6];

    _STATIC_java_lang_RuntimePermission_serialVersionUID = 7399184964622342223;
    _STATIC_java_lang_RuntimePermission_permissionToSetSecurityManager = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToCreateSecurityManager = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToGetProtectionDomain = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToGetClassLoader = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToCreateClassLoader = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToModifyThread = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToModifyThreadGroup = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToExitVM = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToReadFileDescriptor = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToWriteFileDescriptor = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToQueuePrintJob = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToSetFactory = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToSetIO = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToStopThread = (java_lang_RuntimePermission*) JAVA_NULL;
    _STATIC_java_lang_RuntimePermission_permissionToSetContextClassLoader = (java_lang_RuntimePermission*) JAVA_NULL;

    __TIB_java_lang_RuntimePermission.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_RuntimePermission.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_RuntimePermission.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_RuntimePermission.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_RuntimePermission.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_RuntimePermission.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_RuntimePermission.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_RuntimePermission.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_RuntimePermission = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_RuntimePermission);
    __TIB_java_lang_RuntimePermission.clazz = __CLASS_java_lang_RuntimePermission;
    __TIB_java_lang_RuntimePermission.baseType = JAVA_NULL;
    __CLASS_java_lang_RuntimePermission_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_RuntimePermission);
    __CLASS_java_lang_RuntimePermission_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_RuntimePermission_1ARRAY);
    __CLASS_java_lang_RuntimePermission_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_RuntimePermission_2ARRAY);
    java_lang_RuntimePermission___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_RuntimePermission]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_RuntimePermission.classInitialized = 1;
}

void __DELETE_java_lang_RuntimePermission(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_RuntimePermission]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_RuntimePermission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_security_BasicPermission(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_RuntimePermission]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_RuntimePermission()
{    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
java_lang_RuntimePermission* me = (java_lang_RuntimePermission*) XMLVM_MALLOC(sizeof(java_lang_RuntimePermission));
    me->tib = &__TIB_java_lang_RuntimePermission;
    __INIT_INSTANCE_MEMBERS_java_lang_RuntimePermission(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_RuntimePermission]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_RuntimePermission()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_RuntimePermission_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_serialVersionUID;
}

void java_lang_RuntimePermission_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_serialVersionUID = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToSetSecurityManager()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToSetSecurityManager;
}

void java_lang_RuntimePermission_PUT_permissionToSetSecurityManager(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToSetSecurityManager = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToCreateSecurityManager()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToCreateSecurityManager;
}

void java_lang_RuntimePermission_PUT_permissionToCreateSecurityManager(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToCreateSecurityManager = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToGetProtectionDomain()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToGetProtectionDomain;
}

void java_lang_RuntimePermission_PUT_permissionToGetProtectionDomain(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToGetProtectionDomain = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToGetClassLoader()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToGetClassLoader;
}

void java_lang_RuntimePermission_PUT_permissionToGetClassLoader(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToGetClassLoader = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToCreateClassLoader()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToCreateClassLoader;
}

void java_lang_RuntimePermission_PUT_permissionToCreateClassLoader(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToCreateClassLoader = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToModifyThread()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToModifyThread;
}

void java_lang_RuntimePermission_PUT_permissionToModifyThread(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToModifyThread = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToModifyThreadGroup()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToModifyThreadGroup;
}

void java_lang_RuntimePermission_PUT_permissionToModifyThreadGroup(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToModifyThreadGroup = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToExitVM()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToExitVM;
}

void java_lang_RuntimePermission_PUT_permissionToExitVM(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToExitVM = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToReadFileDescriptor()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToReadFileDescriptor;
}

void java_lang_RuntimePermission_PUT_permissionToReadFileDescriptor(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToReadFileDescriptor = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToWriteFileDescriptor()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToWriteFileDescriptor;
}

void java_lang_RuntimePermission_PUT_permissionToWriteFileDescriptor(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToWriteFileDescriptor = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToQueuePrintJob()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToQueuePrintJob;
}

void java_lang_RuntimePermission_PUT_permissionToQueuePrintJob(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToQueuePrintJob = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToSetFactory()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToSetFactory;
}

void java_lang_RuntimePermission_PUT_permissionToSetFactory(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToSetFactory = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToSetIO()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToSetIO;
}

void java_lang_RuntimePermission_PUT_permissionToSetIO(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToSetIO = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToStopThread()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToStopThread;
}

void java_lang_RuntimePermission_PUT_permissionToStopThread(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToStopThread = v;
}

JAVA_OBJECT java_lang_RuntimePermission_GET_permissionToSetContextClassLoader()
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    return _STATIC_java_lang_RuntimePermission_permissionToSetContextClassLoader;
}

void java_lang_RuntimePermission_PUT_permissionToSetContextClassLoader(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
_STATIC_java_lang_RuntimePermission_permissionToSetContextClassLoader = v;
}

void java_lang_RuntimePermission___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_RuntimePermission___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.RuntimePermission", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 34)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 35)
    // "setSecurityManager"
    _r1.o = xmlvm_create_java_string_from_pool(694);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToSetSecurityManager( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 37)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 38)
    // "createSecurityManager"
    _r1.o = xmlvm_create_java_string_from_pool(695);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToCreateSecurityManager( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 40)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 41)
    // "getProtectionDomain"
    _r1.o = xmlvm_create_java_string_from_pool(696);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToGetProtectionDomain( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 43)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 44)
    // "getClassLoader"
    _r1.o = xmlvm_create_java_string_from_pool(697);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToGetClassLoader( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 46)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 47)
    // "createClassLoader"
    _r1.o = xmlvm_create_java_string_from_pool(698);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToCreateClassLoader( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 49)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 50)
    // "modifyThread"
    _r1.o = xmlvm_create_java_string_from_pool(699);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToModifyThread( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 52)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 53)
    // "modifyThreadGroup"
    _r1.o = xmlvm_create_java_string_from_pool(700);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToModifyThreadGroup( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 55)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 56)
    // "exitVM"
    _r1.o = xmlvm_create_java_string_from_pool(168);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToExitVM( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 58)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 59)
    // "readFileDescriptor"
    _r1.o = xmlvm_create_java_string_from_pool(701);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToReadFileDescriptor( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 61)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 62)
    // "writeFileDescriptor"
    _r1.o = xmlvm_create_java_string_from_pool(702);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToWriteFileDescriptor( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 64)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 65)
    // "queuePrintJob"
    _r1.o = xmlvm_create_java_string_from_pool(703);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToQueuePrintJob( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 67)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 68)
    // "setFactory"
    _r1.o = xmlvm_create_java_string_from_pool(704);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToSetFactory( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 70)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 71)
    // "setIO"
    _r1.o = xmlvm_create_java_string_from_pool(705);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToSetIO( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 73)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 74)
    // "stopThread"
    _r1.o = xmlvm_create_java_string_from_pool(706);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToStopThread( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 76)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 77)
    // "setContextClassLoader"
    _r1.o = xmlvm_create_java_string_from_pool(707);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_lang_RuntimePermission_PUT_permissionToSetContextClassLoader( _r0.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 27)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_RuntimePermission___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_RuntimePermission___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.RuntimePermission", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 86)
    XMLVM_CHECK_NPE(0)
    java_security_BasicPermission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 87)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_RuntimePermission___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_RuntimePermission___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.RuntimePermission", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 99)
    XMLVM_CHECK_NPE(0)
    java_security_BasicPermission___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("RuntimePermission.java", 100)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

