#include "xmlvm.h"
#include "java_lang_System.h"
#include "org_xmlvm_runtime_Condition.h"
#include "org_xmlvm_runtime_FinalizerNotifier.h"
#include "org_xmlvm_runtime_Mutex.h"

#include "org_xmlvm_runtime_FinalizerNotifier_1.h"

#define XMLVM_CURRENT_CLASS_NAME FinalizerNotifier_1
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_FinalizerNotifier_1

__TIB_DEFINITION_org_xmlvm_runtime_FinalizerNotifier_1 __TIB_org_xmlvm_runtime_FinalizerNotifier_1 = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_FinalizerNotifier_1, // classInitializer
    "org.xmlvm.runtime.FinalizerNotifier$1", // className
    "org.xmlvm.runtime", // package
    "org.xmlvm.runtime.FinalizerNotifier", // enclosingClassName
    "startFinalizerThread:()Ljava/lang/Thread;", // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Thread, // extends
    sizeof(org_xmlvm_runtime_FinalizerNotifier_1), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_3ARRAY;
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

void __INIT_org_xmlvm_runtime_FinalizerNotifier_1()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_FinalizerNotifier_1);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_FinalizerNotifier_1.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_FinalizerNotifier_1);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_FinalizerNotifier_1.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_FinalizerNotifier_1.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_FinalizerNotifier_1.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.FinalizerNotifier$1")
        __INIT_IMPL_org_xmlvm_runtime_FinalizerNotifier_1();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_FinalizerNotifier_1()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Thread)
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_FinalizerNotifier_1;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_FinalizerNotifier_1.vtable, __TIB_java_lang_Thread.vtable, sizeof(__TIB_java_lang_Thread.vtable));
    // Initialize vtable for this class
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.vtable[6] = (VTABLE_PTR) &org_xmlvm_runtime_FinalizerNotifier_1_run__;
    // Initialize interface information
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.numImplementedInterfaces = 1;
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Runnable)

    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.implementedInterfaces[0][0] = &__TIB_java_lang_Runnable;
    // Initialize itable for this class
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.itableBegin = &__TIB_org_xmlvm_runtime_FinalizerNotifier_1.itable[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.itable[XMLVM_ITABLE_IDX_java_lang_Runnable_run__] = __TIB_org_xmlvm_runtime_FinalizerNotifier_1.vtable[6];


    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_1 = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_FinalizerNotifier_1);
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.clazz = __CLASS_org_xmlvm_runtime_FinalizerNotifier_1;
    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier_1);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier_1_1ARRAY);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier_1_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_FinalizerNotifier_1]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_FinalizerNotifier_1.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_FinalizerNotifier_1(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_FinalizerNotifier_1]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier_1(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Thread(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier_1]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_FinalizerNotifier_1()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier_1)
org_xmlvm_runtime_FinalizerNotifier_1* me = (org_xmlvm_runtime_FinalizerNotifier_1*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_FinalizerNotifier_1));
    me->tib = &__TIB_org_xmlvm_runtime_FinalizerNotifier_1;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier_1(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_FinalizerNotifier_1]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_FinalizerNotifier_1()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_xmlvm_runtime_FinalizerNotifier_1();
    org_xmlvm_runtime_FinalizerNotifier_1___INIT___(me);
    return me;
}

void org_xmlvm_runtime_FinalizerNotifier_1___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_1___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier$1", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 34)
    XMLVM_CHECK_NPE(0)
    java_lang_Thread___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_FinalizerNotifier_1_run__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_1_run__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier$1", "run", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 37)
    _r0.i = 0;
    label2:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 38)
    if (_r0.i != 0) goto label63;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 41)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_access$000__();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_lock__(_r1.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 50)
    _r1.i = org_xmlvm_runtime_FinalizerNotifier_access$100__();
    if (_r1.i != 0) goto label31;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 54)
    org_xmlvm_runtime_FinalizerNotifier_access$200___boolean(_r3.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 59)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_access$300__();
    _r2.o = org_xmlvm_runtime_FinalizerNotifier_access$000__();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex(_r1.o, _r2.o);
    label31:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 62)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_access$000__();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_unlock__(_r1.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 64)
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_access$400__();
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 67)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_access$000__();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_lock__(_r1.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 68)
    org_xmlvm_runtime_FinalizerNotifier_access$200___boolean(_r3.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 69)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_access$000__();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_unlock__(_r1.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 73)
    java_lang_System_gc__();
    goto label2;
    label63:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 77)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

