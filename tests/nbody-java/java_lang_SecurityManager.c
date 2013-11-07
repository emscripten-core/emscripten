#include "xmlvm.h"
#include "java_io_File.h"
#include "java_io_FileDescriptor.h"
#include "java_lang_Class.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_ClassNotFoundException.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_RuntimePermission.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_Thread.h"
#include "java_lang_ThreadGroup.h"
#include "java_lang_reflect_Constructor.h"
#include "java_net_InetAddress.h"
#include "java_security_AccessController.h"
#include "java_security_Permission.h"
#include "java_security_PrivilegedAction.h"
#include "java_util_PropertyPermission.h"
#include "org_apache_harmony_luni_util_PriviAction.h"

#include "java_lang_SecurityManager.h"

#define XMLVM_CURRENT_CLASS_NAME SecurityManager
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_SecurityManager

__TIB_DEFINITION_java_lang_SecurityManager __TIB_java_lang_SecurityManager = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_SecurityManager, // classInitializer
    "java.lang.SecurityManager", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_SecurityManager), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_SecurityManager;
JAVA_OBJECT __CLASS_java_lang_SecurityManager_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_SecurityManager_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_SecurityManager_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_lang_SecurityManager_READ_WRITE_ALL_PROPERTIES_PERMISSION;
static JAVA_OBJECT _STATIC_java_lang_SecurityManager_PKG_ACC_KEY;
static JAVA_OBJECT _STATIC_java_lang_SecurityManager_PKG_DEF_KEY;

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

void __INIT_java_lang_SecurityManager()
{
    staticInitializerLock(&__TIB_java_lang_SecurityManager);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_SecurityManager.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_SecurityManager.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_SecurityManager);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_SecurityManager.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_SecurityManager.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_SecurityManager.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.SecurityManager")
        __INIT_IMPL_java_lang_SecurityManager();
    }
}

void __INIT_IMPL_java_lang_SecurityManager()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_SecurityManager.newInstanceFunc = __NEW_INSTANCE_java_lang_SecurityManager;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_SecurityManager.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_SecurityManager.numImplementedInterfaces = 0;
    __TIB_java_lang_SecurityManager.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_SecurityManager_READ_WRITE_ALL_PROPERTIES_PERMISSION = (java_util_PropertyPermission*) JAVA_NULL;
    _STATIC_java_lang_SecurityManager_PKG_ACC_KEY = (java_lang_String*) xmlvm_create_java_string_from_pool(780);
    _STATIC_java_lang_SecurityManager_PKG_DEF_KEY = (java_lang_String*) xmlvm_create_java_string_from_pool(781);

    __TIB_java_lang_SecurityManager.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_SecurityManager.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_SecurityManager.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_SecurityManager.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_SecurityManager.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_SecurityManager.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_SecurityManager.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_SecurityManager.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_SecurityManager = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_SecurityManager);
    __TIB_java_lang_SecurityManager.clazz = __CLASS_java_lang_SecurityManager;
    __TIB_java_lang_SecurityManager.baseType = JAVA_NULL;
    __CLASS_java_lang_SecurityManager_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_SecurityManager);
    __CLASS_java_lang_SecurityManager_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_SecurityManager_1ARRAY);
    __CLASS_java_lang_SecurityManager_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_SecurityManager_2ARRAY);
    java_lang_SecurityManager___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_SecurityManager]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_SecurityManager.classInitialized = 1;
}

void __DELETE_java_lang_SecurityManager(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_SecurityManager]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_SecurityManager(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_SecurityManager*) me)->fields.java_lang_SecurityManager.inCheck_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_SecurityManager]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_SecurityManager()
{    XMLVM_CLASS_INIT(java_lang_SecurityManager)
java_lang_SecurityManager* me = (java_lang_SecurityManager*) XMLVM_MALLOC(sizeof(java_lang_SecurityManager));
    me->tib = &__TIB_java_lang_SecurityManager;
    __INIT_INSTANCE_MEMBERS_java_lang_SecurityManager(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_SecurityManager]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_SecurityManager()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_SecurityManager();
    java_lang_SecurityManager___INIT___(me);
    return me;
}

JAVA_OBJECT java_lang_SecurityManager_GET_READ_WRITE_ALL_PROPERTIES_PERMISSION()
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
    return _STATIC_java_lang_SecurityManager_READ_WRITE_ALL_PROPERTIES_PERMISSION;
}

void java_lang_SecurityManager_PUT_READ_WRITE_ALL_PROPERTIES_PERMISSION(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
_STATIC_java_lang_SecurityManager_READ_WRITE_ALL_PROPERTIES_PERMISSION = v;
}

JAVA_OBJECT java_lang_SecurityManager_GET_PKG_ACC_KEY()
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
    return _STATIC_java_lang_SecurityManager_PKG_ACC_KEY;
}

void java_lang_SecurityManager_PUT_PKG_ACC_KEY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
_STATIC_java_lang_SecurityManager_PKG_ACC_KEY = v;
}

JAVA_OBJECT java_lang_SecurityManager_GET_PKG_DEF_KEY()
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
    return _STATIC_java_lang_SecurityManager_PKG_DEF_KEY;
}

void java_lang_SecurityManager_PUT_PKG_DEF_KEY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
_STATIC_java_lang_SecurityManager_PKG_DEF_KEY = v;
}

void java_lang_SecurityManager___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager___INIT___]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 71)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 81)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkAccept___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkAccept___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkAccept", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 98)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 99)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 101)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    _r2.i = 58;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[6])(_r1.o, _r2.i);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r5.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    // "accept"
    _r2.o = xmlvm_create_java_string_from_pool(782);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 102)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkAccess___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkAccess___java_lang_Thread]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 119)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkAccess___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkAccess___java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 140)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkConnect___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkConnect___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkConnect", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 158)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 159)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 161)
    if (_r5.i <= 0) goto label44;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 162)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    _r2.i = 58;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[6])(_r1.o, _r2.i);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r5.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    // "connect"
    _r2.o = xmlvm_create_java_string_from_pool(783);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    label43:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 166)
    XMLVM_EXIT_METHOD()
    return;
    label44:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 164)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "resolve"
    _r1.o = xmlvm_create_java_string_from_pool(784);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    goto label43;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkConnect___java_lang_String_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkConnect___java_lang_String_int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkConnect", "?")
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
    _r6.o = n3;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 186)
    if (_r5.i <= 0) goto label36;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 187)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    _r2.i = 58;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[6])(_r1.o, _r2.i);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r5.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    // "connect"
    _r2.o = xmlvm_create_java_string_from_pool(783);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission_java_lang_Object(_r3.o, _r0.o, _r6.o);
    label35:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 192)
    XMLVM_EXIT_METHOD()
    return;
    label36:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 190)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "resolve"
    _r1.o = xmlvm_create_java_string_from_pool(784);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission_java_lang_Object(_r3.o, _r0.o, _r6.o);
    goto label35;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkCreateClassLoader__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkCreateClassLoader__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkCreateClassLoader", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 203)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkDelete___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkDelete___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkDelete", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 215)

    
    // Red class access removed: java.io.FilePermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "delete"
    _r1.o = xmlvm_create_java_string_from_pool(785);

    
    // Red class access removed: java.io.FilePermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 216)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkExec___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkExec___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkExec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 228)

    
    // Red class access removed: java.io.FilePermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_io_File();
    XMLVM_CHECK_NPE(1)
    java_io_File___INIT____java_lang_String(_r1.o, _r4.o);
    XMLVM_CHECK_NPE(1)
    _r1.i = java_io_File_isAbsolute__(_r1.o);
    if (_r1.i == 0) goto label23;
    _r1 = _r4;
    label14:;
    // "execute"
    _r2.o = xmlvm_create_java_string_from_pool(786);

    
    // Red class access removed: java.io.FilePermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 230)
    XMLVM_EXIT_METHOD()
    return;
    label23:;
    // "<<ALL FILES>>"
    _r1.o = xmlvm_create_java_string_from_pool(787);
    goto label14;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkExit___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkExit___int]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkExit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 245)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkLink___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkLink___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkLink", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 257)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 258)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 260)
    _r0.o = __NEW_java_lang_RuntimePermission();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "loadLibrary."
    _r2.o = xmlvm_create_java_string_from_pool(788);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 261)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkListen___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkListen___int]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkListen", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    // "listen"
    _r3.o = xmlvm_create_java_string_from_pool(789);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 273)
    if (_r5.i != 0) goto label17;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 274)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "localhost:1024-"
    _r1.o = xmlvm_create_java_string_from_pool(790);
    // "listen"
    _r2.o = xmlvm_create_java_string_from_pool(789);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r4.o, _r0.o);
    label16:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 278)
    XMLVM_EXIT_METHOD()
    return;
    label17:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 276)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "localhost:"
    _r2.o = xmlvm_create_java_string_from_pool(791);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r5.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    // "listen"
    _r2.o = xmlvm_create_java_string_from_pool(789);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r4.o, _r0.o);
    goto label16;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkMemberAccess___java_lang_Class_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkMemberAccess___java_lang_Class_int]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkMemberAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 300)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 301)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 303)
    if (_r4.i != 0) goto label11;
    label10:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 322)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 321)
    _r0.o = __NEW_java_lang_RuntimePermission();
    // "accessDeclaredMembers"
    _r1.o = xmlvm_create_java_string_from_pool(792);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r2.o, _r0.o);
    goto label10;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkMulticast___java_net_InetAddress(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkMulticast___java_net_InetAddress]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkMulticast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 334)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_net_InetAddress_getHostAddress__[6]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r4.o)->tib->vtable[6])(_r4.o);
    // "accept,connect"
    _r2.o = xmlvm_create_java_string_from_pool(793);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 336)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkMulticast___java_net_InetAddress_byte(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BYTE n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkMulticast___java_net_InetAddress_byte]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkMulticast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 353)

    
    // Red class access removed: java.net.SocketPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_net_InetAddress_getHostAddress__[6]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_net_InetAddress*) _r4.o)->tib->vtable[6])(_r4.o);
    // "accept,connect"
    _r2.o = xmlvm_create_java_string_from_pool(793);

    
    // Red class access removed: java.net.SocketPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 355)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPackageAccess___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPackageAccess___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPackageAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 368)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 369)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 371)
    // "package.access"
    _r0.o = xmlvm_create_java_string_from_pool(780);
    _r0.i = java_lang_SecurityManager_checkPackageProperty___java_lang_String_java_lang_String(_r0.o, _r4.o);
    if (_r0.i == 0) goto label43;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 372)
    _r0.o = __NEW_java_lang_RuntimePermission();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "accessClassInPackage."
    _r2.o = xmlvm_create_java_string_from_pool(794);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    label43:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 375)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPackageDefinition___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPackageDefinition___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPackageDefinition", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 388)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 389)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 391)
    // "package.definition"
    _r0.o = xmlvm_create_java_string_from_pool(781);
    _r0.i = java_lang_SecurityManager_checkPackageProperty___java_lang_String_java_lang_String(_r0.o, _r4.o);
    if (_r0.i == 0) goto label43;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 392)
    _r0.o = __NEW_java_lang_RuntimePermission();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "defineClassInPackage."
    _r2.o = xmlvm_create_java_string_from_pool(795);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r3.o, _r0.o);
    label43:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 395)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_SecurityManager_checkPackageProperty___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_SecurityManager)
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPackageProperty___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPackageProperty", "?")
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
    XMLVMElem _r12;
    _r11.o = n1;
    _r12.o = n2;
    _r10.i = 46;
    _r9.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 403)
    _r8.o = org_apache_harmony_luni_util_PriviAction_getSecurityProperty___java_lang_String(_r11.o);
    _r3.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r8.o);
    _r3.o = _r3.o;
    if (_r3.o == JAVA_NULL) goto label93;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 405)
    XMLVM_SOURCE_POSITION("SecurityManager.java", 406)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(12)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r12.o)->tib->vtable[8])(_r12.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 407)
    // ", *"
    _r8.o = xmlvm_create_java_string_from_pool(796);
    XMLVM_CHECK_NPE(3)
    _r7.o = java_lang_String_split___java_lang_String(_r3.o, _r8.o);
    _r0 = _r7;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 408)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r1.i = 0;
    label28:;
    if (_r1.i >= _r2.i) goto label93;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("SecurityManager.java", 409)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(6)
    _r5.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r6.o)->tib->vtable[8])(_r6.o);
    if (_r4.i <= _r5.i) goto label60;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 410)
    XMLVM_CHECK_NPE(12)
    _r8.i = java_lang_String_startsWith___java_lang_String(_r12.o, _r6.o);
    if (_r8.i == 0) goto label60;
    _r8.i = _r5.i - _r9.i;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(6)
    _r8.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r6.o)->tib->vtable[6])(_r6.o, _r8.i);
    if (_r8.i == _r10.i) goto label58;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(12)
    _r8.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r12.o)->tib->vtable[6])(_r12.o, _r5.i);
    if (_r8.i != _r10.i) goto label60;
    label58:;
    _r8 = _r9;
    label59:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 413)
    XMLVM_SOURCE_POSITION("SecurityManager.java", 423)
    XMLVM_EXIT_METHOD()
    return _r8.i;
    label60:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 414)
    if (_r4.i != _r5.i) goto label70;
    XMLVM_CHECK_NPE(6)
    _r8.i = java_lang_String_startsWith___java_lang_String(_r6.o, _r12.o);
    if (_r8.i == 0) goto label70;
    _r8 = _r9;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 415)
    goto label59;
    label70:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 416)
    _r8.i = _r4.i + 1;
    if (_r8.i != _r5.i) goto label90;
    XMLVM_CHECK_NPE(6)
    _r8.i = java_lang_String_startsWith___java_lang_String(_r6.o, _r12.o);
    if (_r8.i == 0) goto label90;
    _r8.i = _r5.i - _r9.i;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(6)
    _r8.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r6.o)->tib->vtable[6])(_r6.o, _r8.i);
    if (_r8.i != _r10.i) goto label90;
    _r8 = _r9;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 418)
    goto label59;
    label90:;
    _r1.i = _r1.i + 1;
    goto label28;
    label93:;
    _r8.i = 0;
    goto label59;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPropertiesAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPropertiesAccess__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPropertiesAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 435)
    _r0.o = java_lang_SecurityManager_GET_READ_WRITE_ALL_PROPERTIES_PERMISSION();
    XMLVM_CHECK_NPE(1)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 436)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPropertyAccess___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPropertyAccess___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPropertyAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 449)
    _r0.o = __NEW_java_util_PropertyPermission();
    // "read"
    _r1.o = xmlvm_create_java_string_from_pool(670);
    XMLVM_CHECK_NPE(0)
    java_util_PropertyPermission___INIT____java_lang_String_java_lang_String(_r0.o, _r3.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 450)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkRead___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkRead___java_io_FileDescriptor]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkRead", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 462)
    if (_r2.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 463)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 466)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkRead___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkRead___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkRead", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 479)

    
    // Red class access removed: java.io.FilePermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "read"
    _r1.o = xmlvm_create_java_string_from_pool(670);

    
    // Red class access removed: java.io.FilePermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 480)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkRead___java_lang_String_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkRead___java_lang_String_java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkRead", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 494)

    
    // Red class access removed: java.io.FilePermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "read"
    _r1.o = xmlvm_create_java_string_from_pool(670);

    
    // Red class access removed: java.io.FilePermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission_java_lang_Object(_r2.o, _r0.o, _r4.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 495)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkSecurityAccess___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkSecurityAccess___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkSecurityAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 508)

    
    // Red class access removed: java.security.SecurityPermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.security.SecurityPermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 509)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkSetFactory__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkSetFactory__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkSetFactory", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 521)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_SecurityManager_checkTopLevelWindow___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkTopLevelWindow___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkTopLevelWindow", "?")
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
    _r9.o = me;
    _r10.o = n1;
    _r8.i = 1;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 535)
    if (_r10.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 536)
    _r4.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(4)
    java_lang_NullPointerException___INIT___(_r4.o);
    XMLVM_THROW_CUSTOM(_r4.o)
    label10:;
    XMLVM_TRY_BEGIN(w5583aaac29b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("SecurityManager.java", 539)
    // "java.awt.AWTPermission"
    _r4.o = xmlvm_create_java_string_from_pool(797);
    _r0.o = java_lang_Class_forName___java_lang_String(_r4.o);
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 540)
    XMLVM_CLASS_INIT(java_lang_Class)
    _r4.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Class, _r4.i);
    _r5.i = 0;
    XMLVM_CLASS_INIT(java_lang_String)
    _r6.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r6.o;
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getConstructor___java_lang_Class_1ARRAY(_r0.o, _r4.o);
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 542)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r4.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r4.i);
    _r5.i = 0;
    // "showWindowWithoutWarningBanner"
    _r6.o = xmlvm_create_java_string_from_pool(798);
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r6.o;
    XMLVM_CHECK_NPE(1)
    _r3.o = java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY(_r1.o, _r4.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 544)
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(9)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r9.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5583aaac29b1c12)
        XMLVM_CATCH_SPECIFIC(w5583aaac29b1c12,java_lang_ClassNotFoundException,59)
    XMLVM_CATCH_END(w5583aaac29b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w5583aaac29b1c12)
    label45:;
    _r4 = _r8;
    label46:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 553)
    XMLVM_EXIT_METHOD()
    return _r4.i;
    label47:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 550)
    java_lang_Thread* curThread_w5583aaac29b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c20->fields.java_lang_Thread.xmlvmException_;
    _r2 = _r4;
    _r4 = _r7;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 551)
    goto label46;
    label51:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 549)
    java_lang_Thread* curThread_w5583aaac29b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c27->fields.java_lang_Thread.xmlvmException_;
    goto label45;
    label53:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 548)
    java_lang_Thread* curThread_w5583aaac29b1c31 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c31->fields.java_lang_Thread.xmlvmException_;
    goto label45;
    label55:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 547)
    java_lang_Thread* curThread_w5583aaac29b1c35 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c35->fields.java_lang_Thread.xmlvmException_;
    goto label45;
    label57:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 546)
    java_lang_Thread* curThread_w5583aaac29b1c39 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c39->fields.java_lang_Thread.xmlvmException_;
    goto label45;
    label59:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 545)
    java_lang_Thread* curThread_w5583aaac29b1c43 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w5583aaac29b1c43->fields.java_lang_Thread.xmlvmException_;
    goto label45;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkSystemClipboardAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkSystemClipboardAccess__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkSystemClipboardAccess", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    XMLVM_TRY_BEGIN(w5583aaac30b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("SecurityManager.java", 566)
    // "java.awt.AWTPermission"
    _r3.o = xmlvm_create_java_string_from_pool(797);
    _r0.o = java_lang_Class_forName___java_lang_String(_r3.o);
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 567)
    XMLVM_CLASS_INIT(java_lang_Class)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Class, _r3.i);
    _r4.i = 0;
    XMLVM_CLASS_INIT(java_lang_String)
    _r5.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getConstructor___java_lang_Class_1ARRAY(_r0.o, _r3.o);
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 569)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r3.i);
    _r4.i = 0;
    // "accessClipboard"
    _r5.o = xmlvm_create_java_string_from_pool(799);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY(_r1.o, _r3.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 570)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(6)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r6.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5583aaac30b1b2)
        XMLVM_CATCH_SPECIFIC(w5583aaac30b1b2,java_lang_ClassNotFoundException,49)
    XMLVM_CATCH_END(w5583aaac30b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w5583aaac30b1b2)
    XMLVM_SOURCE_POSITION("SecurityManager.java", 571)
    XMLVM_EXIT_METHOD()
    return;
    label36:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 576)
    java_lang_Thread* curThread_w5583aaac30b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac30b1b7->fields.java_lang_Thread.xmlvmException_;
    label37:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 578)

    
    // Red class access removed: java.lang.SecurityException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.SecurityException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label43:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 575)
    java_lang_Thread* curThread_w5583aaac30b1c15 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac30b1c15->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label45:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 574)
    java_lang_Thread* curThread_w5583aaac30b1c19 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac30b1c19->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label47:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 573)
    java_lang_Thread* curThread_w5583aaac30b1c23 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac30b1c23->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label49:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 572)
    java_lang_Thread* curThread_w5583aaac30b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac30b1c27->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkAwtEventQueueAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkAwtEventQueueAccess__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkAwtEventQueueAccess", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    XMLVM_TRY_BEGIN(w5583aaac31b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("SecurityManager.java", 591)
    // "java.awt.AWTPermission"
    _r3.o = xmlvm_create_java_string_from_pool(797);
    _r0.o = java_lang_Class_forName___java_lang_String(_r3.o);
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 592)
    XMLVM_CLASS_INIT(java_lang_Class)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Class, _r3.i);
    _r4.i = 0;
    XMLVM_CLASS_INIT(java_lang_String)
    _r5.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getConstructor___java_lang_Class_1ARRAY(_r0.o, _r3.o);
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 594)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r3.i);
    _r4.i = 0;
    // "accessEventQueue"
    _r5.o = xmlvm_create_java_string_from_pool(800);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY(_r1.o, _r3.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 595)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(6)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r6.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5583aaac31b1b2)
        XMLVM_CATCH_SPECIFIC(w5583aaac31b1b2,java_lang_ClassNotFoundException,49)
    XMLVM_CATCH_END(w5583aaac31b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w5583aaac31b1b2)
    XMLVM_SOURCE_POSITION("SecurityManager.java", 596)
    XMLVM_EXIT_METHOD()
    return;
    label36:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 601)
    java_lang_Thread* curThread_w5583aaac31b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac31b1b7->fields.java_lang_Thread.xmlvmException_;
    label37:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 603)

    
    // Red class access removed: java.lang.SecurityException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.SecurityException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label43:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 600)
    java_lang_Thread* curThread_w5583aaac31b1c15 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac31b1c15->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label45:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 599)
    java_lang_Thread* curThread_w5583aaac31b1c19 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac31b1c19->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label47:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 598)
    java_lang_Thread* curThread_w5583aaac31b1c23 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac31b1c23->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    label49:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 597)
    java_lang_Thread* curThread_w5583aaac31b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w5583aaac31b1c27->fields.java_lang_Thread.xmlvmException_;
    goto label37;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPrintJobAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPrintJobAccess__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPrintJobAccess", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 615)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkWrite___java_io_FileDescriptor(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkWrite___java_io_FileDescriptor]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkWrite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 627)
    if (_r2.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 628)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 631)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkWrite___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkWrite___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkWrite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 644)

    
    // Red class access removed: java.io.FilePermission::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "write"
    _r1.o = xmlvm_create_java_string_from_pool(671);

    
    // Red class access removed: java.io.FilePermission::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("SecurityManager.java", 645)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_SecurityManager_getInCheck__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_getInCheck__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "getInCheck", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 656)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_SecurityManager*) _r1.o)->fields.java_lang_SecurityManager.inCheck_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_SecurityManager_getClassContext__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_getClassContext__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "getClassContext", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 668)
    _r0.i = -1;
    _r1.i = 0;
    _r0.o = java_lang_Class_getStackClasses___int_boolean(_r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_SecurityManager_currentClassLoader__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_currentClassLoader__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "currentClassLoader", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 703)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_SecurityManager_classLoaderDepth__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_classLoaderDepth__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "classLoaderDepth", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 738)
    _r0.i = -1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_SecurityManager_currentLoadedClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_currentLoadedClass__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "currentLoadedClass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 772)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_SecurityManager_classDepth___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_classDepth___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "classDepth", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = -1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 788)
    _r2.i = 0;
    _r0.o = java_lang_Class_getStackClasses___int_boolean(_r3.i, _r2.i);
    _r1.i = 0;
    label7:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 789)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i >= _r2.i) goto label27;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 790)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_Class_getName__(_r2.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[1])(_r2.o, _r5.o);
    if (_r2.i == 0) goto label24;
    _r2 = _r1;
    label23:;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 791)
    XMLVM_SOURCE_POSITION("SecurityManager.java", 794)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label24:;
    _r1.i = _r1.i + 1;
    goto label7;
    label27:;
    _r2 = _r3;
    goto label23;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_SecurityManager_inClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_inClass___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "inClass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 809)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_lang_SecurityManager_classDepth___java_lang_String(_r2.o, _r3.o);
    _r1.i = -1;
    if (_r0.i == _r1.i) goto label9;
    _r0.i = 1;
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    _r0.i = 0;
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_SecurityManager_inClassLoader__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_inClassLoader__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "inClassLoader", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 822)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_SecurityManager_currentClassLoader__(_r1.o);
    if (_r0.o == JAVA_NULL) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_SecurityManager_getThreadGroup__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_getThreadGroup__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "getThreadGroup", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 833)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Thread_getThreadGroup__(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_SecurityManager_getSecurityContext__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_getSecurityContext__]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "getSecurityContext", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 845)
    _r0.o = java_security_AccessController_getContext__();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPermission___java_security_Permission(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPermission___java_security_Permission]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPermission", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 865)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager_checkPermission___java_security_Permission_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager_checkPermission___java_security_Permission_java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "checkPermission", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 895)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_SecurityManager___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_SecurityManager___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.SecurityManager", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("SecurityManager.java", 50)
    _r0.o = __NEW_java_util_PropertyPermission();
    // "*"
    _r1.o = xmlvm_create_java_string_from_pool(801);
    // "read,write"
    _r2.o = xmlvm_create_java_string_from_pool(672);
    XMLVM_CHECK_NPE(0)
    java_util_PropertyPermission___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_lang_SecurityManager_PUT_READ_WRITE_ALL_PROPERTIES_PERMISSION( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

