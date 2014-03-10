#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"
#include "java_lang_Thread.h"
#include "org_xmlvm_runtime_Condition.h"
#include "org_xmlvm_runtime_FinalizerNotifier_1.h"
#include "org_xmlvm_runtime_Mutex.h"

#include "org_xmlvm_runtime_FinalizerNotifier.h"

#define XMLVM_CURRENT_CLASS_NAME FinalizerNotifier
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_FinalizerNotifier

__TIB_DEFINITION_org_xmlvm_runtime_FinalizerNotifier __TIB_org_xmlvm_runtime_FinalizerNotifier = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_FinalizerNotifier, // classInitializer
    "org.xmlvm.runtime.FinalizerNotifier", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_xmlvm_runtime_FinalizerNotifier), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerMutex;
static JAVA_OBJECT _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerCondition;
static JAVA_OBJECT _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThread;
static JAVA_BOOLEAN _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThreadInvokingFinalizers;
static JAVA_BOOLEAN _STATIC_org_xmlvm_runtime_FinalizerNotifier_gcEnabled;

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

void __INIT_org_xmlvm_runtime_FinalizerNotifier()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_FinalizerNotifier);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_FinalizerNotifier.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_FinalizerNotifier.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_FinalizerNotifier);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_FinalizerNotifier.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_FinalizerNotifier.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_FinalizerNotifier.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.FinalizerNotifier")
        __INIT_IMPL_org_xmlvm_runtime_FinalizerNotifier();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_FinalizerNotifier()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_xmlvm_runtime_FinalizerNotifier.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_FinalizerNotifier;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_FinalizerNotifier.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_org_xmlvm_runtime_FinalizerNotifier.numImplementedInterfaces = 0;
    __TIB_org_xmlvm_runtime_FinalizerNotifier.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerMutex = (org_xmlvm_runtime_Mutex*) JAVA_NULL;
    _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerCondition = (org_xmlvm_runtime_Condition*) JAVA_NULL;
    _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThread = (java_lang_Thread*) JAVA_NULL;
    _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThreadInvokingFinalizers = 0;
    _STATIC_org_xmlvm_runtime_FinalizerNotifier_gcEnabled = 0;

    __TIB_org_xmlvm_runtime_FinalizerNotifier.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_FinalizerNotifier.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_FinalizerNotifier.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_FinalizerNotifier.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_FinalizerNotifier.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_FinalizerNotifier.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_FinalizerNotifier);
    __TIB_org_xmlvm_runtime_FinalizerNotifier.clazz = __CLASS_org_xmlvm_runtime_FinalizerNotifier;
    __TIB_org_xmlvm_runtime_FinalizerNotifier.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier_1ARRAY);
    __CLASS_org_xmlvm_runtime_FinalizerNotifier_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_FinalizerNotifier_2ARRAY);
    org_xmlvm_runtime_FinalizerNotifier___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_FinalizerNotifier]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_FinalizerNotifier.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_FinalizerNotifier(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_FinalizerNotifier]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_FinalizerNotifier()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
org_xmlvm_runtime_FinalizerNotifier* me = (org_xmlvm_runtime_FinalizerNotifier*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_FinalizerNotifier));
    me->tib = &__TIB_org_xmlvm_runtime_FinalizerNotifier;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_FinalizerNotifier]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_FinalizerNotifier()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_xmlvm_runtime_FinalizerNotifier();
    org_xmlvm_runtime_FinalizerNotifier___INIT___(me);
    return me;
}

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_GET_finalizerMutex()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    return _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerMutex;
}

void org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerMutex(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
_STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerMutex = v;
}

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_GET_finalizerCondition()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    return _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerCondition;
}

void org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerCondition(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
_STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerCondition = v;
}

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    return _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThread;
}

void org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerThread(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
_STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThread = v;
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThreadInvokingFinalizers()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    return _STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThreadInvokingFinalizers;
}

void org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerThreadInvokingFinalizers(JAVA_BOOLEAN v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
_STATIC_org_xmlvm_runtime_FinalizerNotifier_finalizerThreadInvokingFinalizers = v;
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_GET_gcEnabled()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    return _STATIC_org_xmlvm_runtime_FinalizerNotifier_gcEnabled;
}

void org_xmlvm_runtime_FinalizerNotifier_PUT_gcEnabled(JAVA_BOOLEAN v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
_STATIC_org_xmlvm_runtime_FinalizerNotifier_gcEnabled = v;
}

void org_xmlvm_runtime_FinalizerNotifier___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_startFinalizerThread__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_startFinalizerThread__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "startFinalizerThread", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 32)
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    _r0.o = __CLASS_org_xmlvm_runtime_FinalizerNotifier;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w1899aaab6b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 33)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    if (_r1.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w1899aaab6b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1899aaab6b1b4, sizeof(XMLVM_JMP_BUF)); goto label32; };
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 34)
    _r1.o = __NEW_org_xmlvm_runtime_FinalizerNotifier_1();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_FinalizerNotifier_1___INIT___(_r1.o);
    org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerThread( _r1.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 79)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    // "Finalizer-thread"
    _r2.o = xmlvm_create_java_string_from_pool(101);
    XMLVM_CHECK_NPE(1)
    java_lang_Thread_setName___java_lang_String(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 83)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    _r2.i = 1;
    XMLVM_CHECK_NPE(1)
    java_lang_Thread_setDaemon___boolean(_r1.o, _r2.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 85)
    _r1.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    XMLVM_CHECK_NPE(1)
    java_lang_Thread_start__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1899aaab6b1b4)
        XMLVM_CATCH_SPECIFIC(w1899aaab6b1b4,java_lang_Object,36)
    XMLVM_CATCH_END(w1899aaab6b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w1899aaab6b1b4)
    label32:;
    XMLVM_TRY_BEGIN(w1899aaab6b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 87)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1899aaab6b1b6)
        XMLVM_CATCH_SPECIFIC(w1899aaab6b1b6,java_lang_Object,36)
    XMLVM_CATCH_END(w1899aaab6b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1899aaab6b1b6)
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 89)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    java_lang_Thread* curThread_w1899aaab6b1c11 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w1899aaab6b1c11->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w1899aaab6b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1899aaab6b1c12)
        XMLVM_CATCH_SPECIFIC(w1899aaab6b1c12,java_lang_Object,36)
    XMLVM_CATCH_END(w1899aaab6b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1899aaab6b1c12)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_FinalizerNotifier_setGCActive___boolean(JAVA_BOOLEAN n1)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_setGCActive___boolean]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "setGCActive", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 103)
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_GET_gcEnabled();
    if (_r0.i == _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 106)
    if (_r1.i != 0) goto label13;
    _r0.i = 1;
    label7:;
    org_xmlvm_runtime_FinalizerNotifier_preventGarbageCollection___boolean(_r0.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 107)
    org_xmlvm_runtime_FinalizerNotifier_PUT_gcEnabled( _r1.i);
    label12:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 109)
    XMLVM_EXIT_METHOD()
    return;
    label13:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_invokeAllFinalizers__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_invokeAllFinalizers__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "invokeAllFinalizers", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 117)
    _r0.i = 0;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 118)
    org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerThreadInvokingFinalizers( _r1.i);
    label4:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 119)
    _r0.i = java_lang_Thread_interrupted__();
    if (_r0.i != 0) goto label20;
    _r1.i = org_xmlvm_runtime_FinalizerNotifier_shouldInvokeFinalizers__();
    if (_r1.i == 0) goto label20;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 120)
    org_xmlvm_runtime_FinalizerNotifier_invokeFinalizers__();
    goto label4;
    label20:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 122)
    _r1.i = 0;
    org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerThreadInvokingFinalizers( _r1.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 128)
    org_xmlvm_runtime_Mutex_destroyFinalizableNativeMutexes__();
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 130)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_FinalizerNotifier_preventGarbageCollection___boolean(JAVA_BOOLEAN n1)]

//XMLVM_NATIVE[JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_currentThreadIsFinalizerThread___java_lang_Thread(JAVA_OBJECT n1)]

void org_xmlvm_runtime_FinalizerNotifier_finalizerNotifier__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_finalizerNotifier__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "finalizerNotifier", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 163)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThread();
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_currentThreadIsFinalizerThread___java_lang_Thread(_r0.o);
    if (_r0.i == 0) goto label16;
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerThreadInvokingFinalizers();
    if (_r0.i != 0) goto label16;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 168)
    org_xmlvm_runtime_FinalizerNotifier_invokeAllFinalizers__();
    label15:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 184)
    XMLVM_EXIT_METHOD()
    return;
    label16:;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 175)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_lock__(_r0.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 178)
    _r0.i = 0;
    org_xmlvm_runtime_FinalizerNotifier_setGCActive___boolean(_r0.i);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 180)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerCondition();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Condition_broadcast__(_r0.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 182)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_unlock__(_r0.o);
    goto label15;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_shouldInvokeFinalizers__()]

//XMLVM_NATIVE[JAVA_INT org_xmlvm_runtime_FinalizerNotifier_invokeFinalizers__()]

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_access$000__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_access$000__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "access$000", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerMutex();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_access$100__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_access$100__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "access$100", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_shouldInvokeFinalizers__();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_FinalizerNotifier_access$200___boolean(JAVA_BOOLEAN n1)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_access$200___boolean]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "access$200", "?")
    XMLVMElem _r0;
    _r0.i = n1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    org_xmlvm_runtime_FinalizerNotifier_setGCActive___boolean(_r0.i);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_xmlvm_runtime_FinalizerNotifier_access$300__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_access$300__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "access$300", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    _r0.o = org_xmlvm_runtime_FinalizerNotifier_GET_finalizerCondition();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_access$400__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_FinalizerNotifier)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier_access$400__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "access$400", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 10)
    _r0.i = org_xmlvm_runtime_FinalizerNotifier_invokeAllFinalizers__();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_FinalizerNotifier___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_FinalizerNotifier___CLINIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.FinalizerNotifier", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 11)
    _r0.o = __NEW_org_xmlvm_runtime_Mutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex___INIT___(_r0.o);
    org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerMutex( _r0.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 12)
    _r0.o = __NEW_org_xmlvm_runtime_Condition();
    _r1.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Condition___INIT____java_lang_Object(_r0.o, _r1.o);
    org_xmlvm_runtime_FinalizerNotifier_PUT_finalizerCondition( _r0.o);
    XMLVM_SOURCE_POSITION("FinalizerNotifier.java", 19)
    _r0.i = 1;
    org_xmlvm_runtime_FinalizerNotifier_PUT_gcEnabled( _r0.i);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

