#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_lang_RuntimePermission.h"
#include "java_lang_String.h"

#include "java_security_BasicPermission.h"

#define XMLVM_CURRENT_CLASS_NAME BasicPermission
#define XMLVM_CURRENT_PKG_CLASS_NAME java_security_BasicPermission

__TIB_DEFINITION_java_security_BasicPermission __TIB_java_security_BasicPermission = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_security_BasicPermission, // classInitializer
    "java.security.BasicPermission", // className
    "java.security", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_security_Permission, // extends
    sizeof(java_security_BasicPermission), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_security_BasicPermission;
JAVA_OBJECT __CLASS_java_security_BasicPermission_1ARRAY;
JAVA_OBJECT __CLASS_java_security_BasicPermission_2ARRAY;
JAVA_OBJECT __CLASS_java_security_BasicPermission_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_security_BasicPermission_serialVersionUID;

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

void __INIT_java_security_BasicPermission()
{
    staticInitializerLock(&__TIB_java_security_BasicPermission);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_security_BasicPermission.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_security_BasicPermission.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_security_BasicPermission);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_security_BasicPermission.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_security_BasicPermission.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_security_BasicPermission.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.security.BasicPermission")
        __INIT_IMPL_java_security_BasicPermission();
    }
}

void __INIT_IMPL_java_security_BasicPermission()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_security_Permission)
    __TIB_java_security_BasicPermission.newInstanceFunc = __NEW_INSTANCE_java_security_BasicPermission;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_security_BasicPermission.vtable, __TIB_java_security_Permission.vtable, sizeof(__TIB_java_security_Permission.vtable));
    // Initialize vtable for this class
    __TIB_java_security_BasicPermission.vtable[1] = (VTABLE_PTR) &java_security_BasicPermission_equals___java_lang_Object;
    __TIB_java_security_BasicPermission.vtable[4] = (VTABLE_PTR) &java_security_BasicPermission_hashCode__;
    __TIB_java_security_BasicPermission.vtable[7] = (VTABLE_PTR) &java_security_BasicPermission_getActions__;
    __TIB_java_security_BasicPermission.vtable[8] = (VTABLE_PTR) &java_security_BasicPermission_implies___java_security_Permission;
    __TIB_java_security_BasicPermission.vtable[9] = (VTABLE_PTR) &java_security_BasicPermission_newPermissionCollection__;
    // Initialize interface information
    __TIB_java_security_BasicPermission.numImplementedInterfaces = 2;
    __TIB_java_security_BasicPermission.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_security_BasicPermission.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_security_Guard)

    __TIB_java_security_BasicPermission.implementedInterfaces[0][1] = &__TIB_java_security_Guard;
    // Initialize itable for this class
    __TIB_java_security_BasicPermission.itableBegin = &__TIB_java_security_BasicPermission.itable[0];
    __TIB_java_security_BasicPermission.itable[XMLVM_ITABLE_IDX_java_security_Guard_checkGuard___java_lang_Object] = __TIB_java_security_BasicPermission.vtable[6];

    _STATIC_java_security_BasicPermission_serialVersionUID = 6279438298436773498;

    __TIB_java_security_BasicPermission.declaredFields = &__field_reflection_data[0];
    __TIB_java_security_BasicPermission.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_security_BasicPermission.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_security_BasicPermission.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_security_BasicPermission.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_security_BasicPermission.methodDispatcherFunc = method_dispatcher;
    __TIB_java_security_BasicPermission.declaredMethods = &__method_reflection_data[0];
    __TIB_java_security_BasicPermission.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_security_BasicPermission = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_security_BasicPermission);
    __TIB_java_security_BasicPermission.clazz = __CLASS_java_security_BasicPermission;
    __TIB_java_security_BasicPermission.baseType = JAVA_NULL;
    __CLASS_java_security_BasicPermission_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_BasicPermission);
    __CLASS_java_security_BasicPermission_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_BasicPermission_1ARRAY);
    __CLASS_java_security_BasicPermission_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_BasicPermission_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_security_BasicPermission]
    //XMLVM_END_WRAPPER

    __TIB_java_security_BasicPermission.classInitialized = 1;
}

void __DELETE_java_security_BasicPermission(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_security_BasicPermission]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_security_BasicPermission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_security_Permission(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_security_BasicPermission]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_security_BasicPermission()
{    XMLVM_CLASS_INIT(java_security_BasicPermission)
java_security_BasicPermission* me = (java_security_BasicPermission*) XMLVM_MALLOC(sizeof(java_security_BasicPermission));
    me->tib = &__TIB_java_security_BasicPermission;
    __INIT_INSTANCE_MEMBERS_java_security_BasicPermission(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_security_BasicPermission]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_security_BasicPermission()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_security_BasicPermission_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_security_BasicPermission)
    return _STATIC_java_security_BasicPermission_serialVersionUID;
}

void java_security_BasicPermission_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_security_BasicPermission)
_STATIC_java_security_BasicPermission_serialVersionUID = v;
}

void java_security_BasicPermission___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 59)
    XMLVM_CHECK_NPE(0)
    java_security_Permission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 60)
    XMLVM_CHECK_NPE(0)
    java_security_BasicPermission_checkName___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 61)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_security_BasicPermission___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 77)
    XMLVM_CHECK_NPE(0)
    java_security_Permission___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 78)
    XMLVM_CHECK_NPE(0)
    java_security_BasicPermission_checkName___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 79)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_security_BasicPermission_checkName___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_checkName___java_lang_String]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "checkName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 85)
    if (_r3.o != JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 86)
    _r0.o = __NEW_java_lang_NullPointerException();
    // "security.28"
    _r1.o = xmlvm_create_java_string_from_pool(165);

    
    // Red class access removed: org.apache.harmony.security.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 88)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    if (_r0.i != 0) goto label32;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 89)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "security.29"
    _r1.o = xmlvm_create_java_string_from_pool(166);

    
    // Red class access removed: org.apache.harmony.security.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label32:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 91)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_security_BasicPermission_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 109)
    if (_r3.o != _r2.o) goto label4;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 110)
    _r0.i = 1;
    label3:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 116)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label4:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 113)
    if (_r3.o == JAVA_NULL) goto label31;
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[3])(_r3.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[3])(_r2.o);
    if (_r0.o != _r1.o) goto label31;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 114)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_security_Permission_getName__(_r2.o);
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(3)
    _r1.o = java_security_Permission_getName__(_r3.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r1.o);
    goto label3;
    label31:;
    _r0.i = 0;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_security_BasicPermission_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_hashCode__]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 130)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_security_Permission_getName__(_r1.o);
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_security_BasicPermission_getActions__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_getActions__]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "getActions", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 141)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_security_BasicPermission_implies___java_security_Permission(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_implies___java_security_Permission]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "implies", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    // "exitVM.*"
    _r4.o = xmlvm_create_java_string_from_pool(167);
    // "exitVM"
    _r3.o = xmlvm_create_java_string_from_pool(168);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 154)
    if (_r7.o == JAVA_NULL) goto label56;
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[3])(_r7.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(6)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[3])(_r6.o);
    if (_r0.o != _r1.o) goto label56;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 155)
    XMLVM_CHECK_NPE(6)
    _r0.o = java_security_Permission_getName__(_r6.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 156)
    XMLVM_CHECK_NPE(7)
    _r1.o = java_security_Permission_getName__(_r7.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 157)
    XMLVM_CLASS_INIT(java_lang_RuntimePermission)
    _r2.i = XMLVM_ISA(_r6.o, __CLASS_java_lang_RuntimePermission);
    if (_r2.i == 0) goto label58;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 158)
    // "exitVM"
    _r2.o = xmlvm_create_java_string_from_pool(168);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[1])(_r1.o, _r3.o);
    if (_r2.i == 0) goto label39;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 159)
    // "exitVM.*"
    _r1.o = xmlvm_create_java_string_from_pool(167);
    _r1 = _r4;
    label39:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 161)
    // "exitVM"
    _r2.o = xmlvm_create_java_string_from_pool(168);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r3.o);
    if (_r2.i == 0) goto label58;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 162)
    // "exitVM.*"
    _r0.o = xmlvm_create_java_string_from_pool(167);
    _r0 = _r1;
    _r1 = _r4;
    label51:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 165)
    _r0.i = java_security_BasicPermission_nameImplies___java_lang_String_java_lang_String(_r1.o, _r0.o);
    label55:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 167)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label56:;
    _r0.i = 0;
    goto label55;
    label58:;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    goto label51;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_security_BasicPermission_nameImplies___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_security_BasicPermission)
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_nameImplies___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "nameImplies", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = n1;
    _r6.o = n2;
    _r4.i = 0;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 176)
    if (_r5.o != _r6.o) goto label6;
    _r0 = _r3;
    label5:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 177)
    XMLVM_SOURCE_POSITION("BasicPermission.java", 196)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 179)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[8])(_r5.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 180)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(6)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r6.o)->tib->vtable[8])(_r6.o);
    if (_r0.i <= _r1.i) goto label18;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 181)
    goto label5;
    label18:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 183)
    _r0.i = _r0.i + -1;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r5.o)->tib->vtable[6])(_r5.o, _r0.i);
    _r2.i = 42;
    if (_r1.i != _r2.i) goto label46;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 184)
    if (_r0.i == 0) goto label40;
    _r1.i = _r0.i - _r3.i;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r5.o)->tib->vtable[6])(_r5.o, _r1.i);
    _r2.i = 46;
    if (_r1.i != _r2.i) goto label46;
    label40:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 186)
    _r0.i = _r0.i + -1;
    label42:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 191)
    if (_r0.i >= 0) goto label55;
    _r0 = _r3;
    goto label5;
    label46:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 187)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(6)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r6.o)->tib->vtable[8])(_r6.o);
    _r1.i = _r1.i - _r3.i;
    if (_r0.i == _r1.i) goto label42;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 189)
    goto label5;
    label55:;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 192)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r5.o)->tib->vtable[6])(_r5.o, _r0.i);
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(6)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r6.o)->tib->vtable[6])(_r6.o, _r0.i);
    if (_r1.i == _r2.i) goto label67;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 193)
    goto label5;
    label67:;
    _r0.i = _r0.i + -1;
    goto label42;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_security_BasicPermission_newPermissionCollection__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_newPermissionCollection__]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "newPermissionCollection", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 214)

    
    // Red class access removed: java.security.BasicPermissionCollection::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.security.BasicPermissionCollection::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_security_BasicPermission_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_security_BasicPermission_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.security.BasicPermission", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("BasicPermission.java", 222)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("BasicPermission.java", 223)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_security_Permission_getName__(_r1.o);
    XMLVM_CHECK_NPE(1)
    java_security_BasicPermission_checkName___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("BasicPermission.java", 224)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

