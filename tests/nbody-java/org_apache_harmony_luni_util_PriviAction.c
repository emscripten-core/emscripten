#include "xmlvm.h"
#include "java_lang_String.h"
#include "java_lang_System.h"
#include "java_lang_reflect_AccessibleObject.h"

#include "org_apache_harmony_luni_util_PriviAction.h"

#define XMLVM_CURRENT_CLASS_NAME PriviAction
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_luni_util_PriviAction

__TIB_DEFINITION_org_apache_harmony_luni_util_PriviAction __TIB_org_apache_harmony_luni_util_PriviAction = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_util_PriviAction, // classInitializer
    "org.apache.harmony.luni.util.PriviAction", // className
    "org.apache.harmony.luni.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;Ljava/security/PrivilegedAction<TT;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_apache_harmony_luni_util_PriviAction), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SYSTEM_PROPERTY;
static JAVA_INT _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_POLICY;
static JAVA_INT _STATIC_org_apache_harmony_luni_util_PriviAction_SET_ACCESSIBLE;
static JAVA_INT _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_PROPERTY;

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

void __INIT_org_apache_harmony_luni_util_PriviAction()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_util_PriviAction);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_util_PriviAction.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_util_PriviAction.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_util_PriviAction);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_util_PriviAction.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_util_PriviAction.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_util_PriviAction.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.util.PriviAction")
        __INIT_IMPL_org_apache_harmony_luni_util_PriviAction();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_util_PriviAction()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_apache_harmony_luni_util_PriviAction.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_luni_util_PriviAction;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_luni_util_PriviAction.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_luni_util_PriviAction.vtable[6] = (VTABLE_PTR) &org_apache_harmony_luni_util_PriviAction_run__;
    // Initialize interface information
    __TIB_org_apache_harmony_luni_util_PriviAction.numImplementedInterfaces = 1;
    __TIB_org_apache_harmony_luni_util_PriviAction.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_security_PrivilegedAction)

    __TIB_org_apache_harmony_luni_util_PriviAction.implementedInterfaces[0][0] = &__TIB_java_security_PrivilegedAction;
    // Initialize itable for this class
    __TIB_org_apache_harmony_luni_util_PriviAction.itableBegin = &__TIB_org_apache_harmony_luni_util_PriviAction.itable[0];
    __TIB_org_apache_harmony_luni_util_PriviAction.itable[XMLVM_ITABLE_IDX_java_security_PrivilegedAction_run__] = __TIB_org_apache_harmony_luni_util_PriviAction.vtable[6];

    _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SYSTEM_PROPERTY = 1;
    _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_POLICY = 2;
    _STATIC_org_apache_harmony_luni_util_PriviAction_SET_ACCESSIBLE = 3;
    _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_PROPERTY = 4;

    __TIB_org_apache_harmony_luni_util_PriviAction.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_PriviAction.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_util_PriviAction.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_luni_util_PriviAction.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_PriviAction.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_util_PriviAction.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_util_PriviAction.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_PriviAction.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_luni_util_PriviAction = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_util_PriviAction);
    __TIB_org_apache_harmony_luni_util_PriviAction.clazz = __CLASS_org_apache_harmony_luni_util_PriviAction;
    __TIB_org_apache_harmony_luni_util_PriviAction.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_util_PriviAction_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_PriviAction);
    __CLASS_org_apache_harmony_luni_util_PriviAction_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_PriviAction_1ARRAY);
    __CLASS_org_apache_harmony_luni_util_PriviAction_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_PriviAction_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_luni_util_PriviAction]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_luni_util_PriviAction.classInitialized = 1;
}

void __DELETE_org_apache_harmony_luni_util_PriviAction(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_luni_util_PriviAction]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_PriviAction(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_luni_util_PriviAction*) me)->fields.org_apache_harmony_luni_util_PriviAction.arg1_ = (java_lang_Object*) JAVA_NULL;
    ((org_apache_harmony_luni_util_PriviAction*) me)->fields.org_apache_harmony_luni_util_PriviAction.arg2_ = (java_lang_Object*) JAVA_NULL;
    ((org_apache_harmony_luni_util_PriviAction*) me)->fields.org_apache_harmony_luni_util_PriviAction.action_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_PriviAction]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_luni_util_PriviAction()
{    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
org_apache_harmony_luni_util_PriviAction* me = (org_apache_harmony_luni_util_PriviAction*) XMLVM_MALLOC(sizeof(org_apache_harmony_luni_util_PriviAction));
    me->tib = &__TIB_org_apache_harmony_luni_util_PriviAction;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_PriviAction(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_luni_util_PriviAction]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_util_PriviAction()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_apache_harmony_luni_util_PriviAction();
    org_apache_harmony_luni_util_PriviAction___INIT___(me);
    return me;
}

JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SYSTEM_PROPERTY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
    return _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SYSTEM_PROPERTY;
}

void org_apache_harmony_luni_util_PriviAction_PUT_GET_SYSTEM_PROPERTY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
_STATIC_org_apache_harmony_luni_util_PriviAction_GET_SYSTEM_PROPERTY = v;
}

JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SECURITY_POLICY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
    return _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_POLICY;
}

void org_apache_harmony_luni_util_PriviAction_PUT_GET_SECURITY_POLICY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
_STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_POLICY = v;
}

JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_SET_ACCESSIBLE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
    return _STATIC_org_apache_harmony_luni_util_PriviAction_SET_ACCESSIBLE;
}

void org_apache_harmony_luni_util_PriviAction_PUT_SET_ACCESSIBLE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
_STATIC_org_apache_harmony_luni_util_PriviAction_SET_ACCESSIBLE = v;
}

JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SECURITY_PROPERTY()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
    return _STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_PROPERTY;
}

void org_apache_harmony_luni_util_PriviAction_PUT_GET_SECURITY_PROPERTY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
_STATIC_org_apache_harmony_luni_util_PriviAction_GET_SECURITY_PROPERTY = v;
}

JAVA_OBJECT org_apache_harmony_luni_util_PriviAction_getSecurityProperty___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_PriviAction)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction_getSecurityProperty___java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "getSecurityProperty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("PriviAction.java", 57)
    _r0.o = __NEW_org_apache_harmony_luni_util_PriviAction();
    _r1.i = 4;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_PriviAction___INIT____int_java_lang_Object(_r0.o, _r1.i, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_PriviAction___INIT____int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction___INIT____int_java_lang_Object]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("PriviAction.java", 60)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("PriviAction.java", 61)
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_luni_util_PriviAction*) _r0.o)->fields.org_apache_harmony_luni_util_PriviAction.action_ = _r1.i;
    XMLVM_SOURCE_POSITION("PriviAction.java", 62)
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_luni_util_PriviAction*) _r0.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_ = _r2.o;
    XMLVM_SOURCE_POSITION("PriviAction.java", 63)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_PriviAction___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction___INIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("PriviAction.java", 70)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("PriviAction.java", 71)
    _r0.i = 2;
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.action_ = _r0.i;
    XMLVM_SOURCE_POSITION("PriviAction.java", 72)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_reflect_AccessibleObject(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction___INIT____java_lang_reflect_AccessibleObject]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("PriviAction.java", 84)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("PriviAction.java", 85)
    _r0.i = 3;
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.action_ = _r0.i;
    XMLVM_SOURCE_POSITION("PriviAction.java", 86)
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_ = _r2.o;
    XMLVM_SOURCE_POSITION("PriviAction.java", 87)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("PriviAction.java", 98)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("PriviAction.java", 99)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.action_ = _r0.i;
    XMLVM_SOURCE_POSITION("PriviAction.java", 100)
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_ = _r2.o;
    XMLVM_SOURCE_POSITION("PriviAction.java", 101)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("PriviAction.java", 114)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("PriviAction.java", 115)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.action_ = _r0.i;
    XMLVM_SOURCE_POSITION("PriviAction.java", 116)
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_ = _r2.o;
    XMLVM_SOURCE_POSITION("PriviAction.java", 117)
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg2_ = _r3.o;
    XMLVM_SOURCE_POSITION("PriviAction.java", 118)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_PriviAction_run__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_PriviAction_run__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.PriviAction", "run", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("PriviAction.java", 127)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.action_;
    switch (_r0.i) {
    case 1: goto label7;
    case 2: goto label29;
    case 3: goto label34;
    case 4: goto label20;
    }
    label5:;
    XMLVM_SOURCE_POSITION("PriviAction.java", 137)
    _r0.o = JAVA_NULL;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_SOURCE_POSITION("PriviAction.java", 129)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_;
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(1)
    _r1.o = ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg2_;
    _r1.o = _r1.o;
    _r0.o = java_lang_System_getProperty___java_lang_String_java_lang_String(_r0.o, _r1.o);
    goto label6;
    label20:;
    XMLVM_SOURCE_POSITION("PriviAction.java", 131)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_;
    _r1.o = _r1.o;

    
    // Red class access removed: java.security.Security::getProperty
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label6;
    label29:;
    XMLVM_SOURCE_POSITION("PriviAction.java", 133)

    
    // Red class access removed: java.security.Policy::getPolicy
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label6;
    label34:;
    XMLVM_SOURCE_POSITION("PriviAction.java", 135)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((org_apache_harmony_luni_util_PriviAction*) _r1.o)->fields.org_apache_harmony_luni_util_PriviAction.arg1_;
    _r1.o = _r1.o;
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    java_lang_reflect_AccessibleObject_setAccessible___boolean(_r1.o, _r0.i);
    goto label5;
    label44:;
    //XMLVM_END_WRAPPER
}

