#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_SecurityManager.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"

#include "java_security_Permission.h"

#define XMLVM_CURRENT_CLASS_NAME Permission
#define XMLVM_CURRENT_PKG_CLASS_NAME java_security_Permission

__TIB_DEFINITION_java_security_Permission __TIB_java_security_Permission = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_security_Permission, // classInitializer
    "java.security.Permission", // className
    "java.security", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_security_Permission), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_security_Permission;
JAVA_OBJECT __CLASS_java_security_Permission_1ARRAY;
JAVA_OBJECT __CLASS_java_security_Permission_2ARRAY;
JAVA_OBJECT __CLASS_java_security_Permission_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_security_Permission_serialVersionUID;

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

void __INIT_java_security_Permission()
{
    staticInitializerLock(&__TIB_java_security_Permission);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_security_Permission.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_security_Permission.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_security_Permission);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_security_Permission.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_security_Permission.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_security_Permission.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.security.Permission")
        __INIT_IMPL_java_security_Permission();
    }
}

void __INIT_IMPL_java_security_Permission()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_security_Permission.newInstanceFunc = __NEW_INSTANCE_java_security_Permission;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_security_Permission.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_security_Permission.vtable[6] = (VTABLE_PTR) &java_security_Permission_checkGuard___java_lang_Object;
    __TIB_java_security_Permission.vtable[9] = (VTABLE_PTR) &java_security_Permission_newPermissionCollection__;
    __TIB_java_security_Permission.vtable[5] = (VTABLE_PTR) &java_security_Permission_toString__;
    // Initialize interface information
    __TIB_java_security_Permission.numImplementedInterfaces = 2;
    __TIB_java_security_Permission.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_security_Permission.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_security_Guard)

    __TIB_java_security_Permission.implementedInterfaces[0][1] = &__TIB_java_security_Guard;
    // Initialize itable for this class
    __TIB_java_security_Permission.itableBegin = &__TIB_java_security_Permission.itable[0];
    __TIB_java_security_Permission.itable[XMLVM_ITABLE_IDX_java_security_Guard_checkGuard___java_lang_Object] = __TIB_java_security_Permission.vtable[6];

    _STATIC_java_security_Permission_serialVersionUID = -5636570222231596674;

    __TIB_java_security_Permission.declaredFields = &__field_reflection_data[0];
    __TIB_java_security_Permission.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_security_Permission.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_security_Permission.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_security_Permission.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_security_Permission.methodDispatcherFunc = method_dispatcher;
    __TIB_java_security_Permission.declaredMethods = &__method_reflection_data[0];
    __TIB_java_security_Permission.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_security_Permission = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_security_Permission);
    __TIB_java_security_Permission.clazz = __CLASS_java_security_Permission;
    __TIB_java_security_Permission.baseType = JAVA_NULL;
    __CLASS_java_security_Permission_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Permission);
    __CLASS_java_security_Permission_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Permission_1ARRAY);
    __CLASS_java_security_Permission_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Permission_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_security_Permission]
    //XMLVM_END_WRAPPER

    __TIB_java_security_Permission.classInitialized = 1;
}

void __DELETE_java_security_Permission(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_security_Permission]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_security_Permission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_security_Permission*) me)->fields.java_security_Permission.name_ = (java_lang_String*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_security_Permission]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_security_Permission()
{    XMLVM_CLASS_INIT(java_security_Permission)
java_security_Permission* me = (java_security_Permission*) XMLVM_MALLOC(sizeof(java_security_Permission));
    me->tib = &__TIB_java_security_Permission;
    __INIT_INSTANCE_MEMBERS_java_security_Permission(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_security_Permission]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_security_Permission()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_security_Permission_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_security_Permission)
    return _STATIC_java_security_Permission_serialVersionUID;
}

void java_security_Permission_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_security_Permission)
_STATIC_java_security_Permission_serialVersionUID = v;
}

void java_security_Permission___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_Permission___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.security.Permission", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Permission.java", 100)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Permission.java", 101)
    XMLVM_CHECK_NPE(0)
    ((java_security_Permission*) _r0.o)->fields.java_security_Permission.name_ = _r1.o;
    XMLVM_SOURCE_POSITION("Permission.java", 102)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_security_Permission_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_Permission_getName__]
    XMLVM_ENTER_METHOD("java.security.Permission", "getName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Permission.java", 110)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_security_Permission*) _r1.o)->fields.java_security_Permission.name_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_security_Permission_checkGuard___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_Permission_checkGuard___java_lang_Object]
    XMLVM_ENTER_METHOD("java.security.Permission", "checkGuard", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Permission.java", 127)
    _r0.o = java_lang_System_getSecurityManager__();
    XMLVM_SOURCE_POSITION("Permission.java", 128)
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Permission.java", 129)
    XMLVM_CHECK_NPE(0)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r0.o, _r1.o);
    label9:;
    XMLVM_SOURCE_POSITION("Permission.java", 131)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_security_Permission_newPermissionCollection__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_Permission_newPermissionCollection__]
    XMLVM_ENTER_METHOD("java.security.Permission", "newPermissionCollection", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Permission.java", 145)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_security_Permission_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_Permission_toString__]
    XMLVM_ENTER_METHOD("java.security.Permission", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    // " "
    _r3.o = xmlvm_create_java_string_from_pool(57);
    XMLVM_SOURCE_POSITION("Permission.java", 156)
    //java_security_Permission_getActions__[7]
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_security_Permission*) _r4.o)->tib->vtable[7])(_r4.o);
    XMLVM_SOURCE_POSITION("Permission.java", 157)
    if (_r0.o == JAVA_NULL) goto label14;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    if (_r0.i != 0) goto label64;
    label14:;
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label16:;
    XMLVM_SOURCE_POSITION("Permission.java", 159)
    _r1.o = __NEW_java_lang_StringBuilder();
    // "("
    _r2.o = xmlvm_create_java_string_from_pool(85);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[3])(_r4.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_Class_getName__(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    // " "
    _r2.o = xmlvm_create_java_string_from_pool(57);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r3.o);
    XMLVM_CHECK_NPE(4)
    _r2.o = java_security_Permission_getName__(_r4.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r0.o);
    // ")"
    _r1.o = xmlvm_create_java_string_from_pool(23);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label64:;
    _r0.o = __NEW_java_lang_StringBuilder();
    // " "
    _r1.o = xmlvm_create_java_string_from_pool(57);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r3.o);
    XMLVM_SOURCE_POSITION("Permission.java", 158)
    //java_security_Permission_getActions__[7]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_security_Permission*) _r4.o)->tib->vtable[7])(_r4.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label16;
    //XMLVM_END_WRAPPER
}

