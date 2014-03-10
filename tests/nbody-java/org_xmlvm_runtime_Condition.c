#include "xmlvm.h"
#include "org_xmlvm_runtime_Mutex.h"

#include "org_xmlvm_runtime_Condition.h"

#define XMLVM_CURRENT_CLASS_NAME Condition
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_Condition

__TIB_DEFINITION_org_xmlvm_runtime_Condition __TIB_org_xmlvm_runtime_Condition = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_Condition, // classInitializer
    "org.xmlvm.runtime.Condition", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_xmlvm_runtime_Condition), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION


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

void __INIT_org_xmlvm_runtime_Condition()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_Condition);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_Condition.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_Condition.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_Condition);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_Condition.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_Condition.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_Condition.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.Condition")
        __INIT_IMPL_org_xmlvm_runtime_Condition();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_Condition()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_xmlvm_runtime_Condition.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_Condition;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_Condition.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    xmlvm_init_native_org_xmlvm_runtime_Condition();
    // Initialize interface information
    __TIB_org_xmlvm_runtime_Condition.numImplementedInterfaces = 0;
    __TIB_org_xmlvm_runtime_Condition.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_org_xmlvm_runtime_Condition.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_Condition.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_Condition.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_Condition.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_Condition.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_Condition.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_Condition.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_Condition.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_Condition = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_Condition);
    __TIB_org_xmlvm_runtime_Condition.clazz = __CLASS_org_xmlvm_runtime_Condition;
    __TIB_org_xmlvm_runtime_Condition.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_Condition_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Condition);
    __CLASS_org_xmlvm_runtime_Condition_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Condition_1ARRAY);
    __CLASS_org_xmlvm_runtime_Condition_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Condition_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_Condition]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_Condition.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_Condition(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_Condition]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Condition(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_xmlvm_runtime_Condition*) me)->fields.org_xmlvm_runtime_Condition.pthreadCondT_ = (java_lang_Object*) JAVA_NULL;
    ((org_xmlvm_runtime_Condition*) me)->fields.org_xmlvm_runtime_Condition.synchronizedObject_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Condition]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_Condition()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_Condition)
org_xmlvm_runtime_Condition* me = (org_xmlvm_runtime_Condition*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_Condition));
    me->tib = &__TIB_org_xmlvm_runtime_Condition;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Condition(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_Condition]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_Condition()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void org_xmlvm_runtime_Condition___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Condition___INIT____java_lang_Object]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Condition", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Condition.java", 40)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Condition.java", 41)
    XMLVM_CHECK_NPE(0)
    ((org_xmlvm_runtime_Condition*) _r0.o)->fields.org_xmlvm_runtime_Condition.synchronizedObject_ = _r1.o;
    XMLVM_SOURCE_POSITION("Condition.java", 42)
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Condition_initNativeConditionInstance__(_r0.o);
    XMLVM_SOURCE_POSITION("Condition.java", 43)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_Condition_initNativeConditionInstance__(JAVA_OBJECT me)]

JAVA_OBJECT org_xmlvm_runtime_Condition_getSynchronizedObject__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Condition_getSynchronizedObject__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Condition", "getSynchronizedObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Condition.java", 52)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((org_xmlvm_runtime_Condition*) _r1.o)->fields.org_xmlvm_runtime_Condition.synchronizedObject_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_BOOLEAN org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2)]

//XMLVM_NATIVE[void org_xmlvm_runtime_Condition_signal__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void org_xmlvm_runtime_Condition_broadcast__(JAVA_OBJECT me)]

