#include "xmlvm.h"
#include "java_io_PrintStream.h"
#include "java_lang_Class.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_Long.h"
#include "java_lang_StackTraceElement.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Thread_State.h"
#include "java_lang_Thread_UncaughtExceptionHandler.h"
#include "java_lang_ThreadGroup.h"
#include "java_lang_Throwable.h"
#include "java_util_HashMap.h"
#include "java_util_Map.h"
#include "org_xmlvm_runtime_Condition.h"
#include "org_xmlvm_runtime_Mutex.h"

#include "java_lang_Thread.h"

#define XMLVM_CURRENT_CLASS_NAME Thread
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Thread

__TIB_DEFINITION_java_lang_Thread __TIB_java_lang_Thread = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Thread, // classInitializer
    "java.lang.Thread", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Thread), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Thread;
JAVA_OBJECT __CLASS_java_lang_Thread_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_lang_Thread_threadSafetyMutex;
static JAVA_LONG _STATIC_java_lang_Thread_nextThreadId;
static JAVA_INT _STATIC_java_lang_Thread_numberOfActiveNonDaemonThreads;
static JAVA_OBJECT _STATIC_java_lang_Thread_threadMap;
static JAVA_OBJECT _STATIC_java_lang_Thread_mainThreadGroup;
static JAVA_INT _STATIC_java_lang_Thread_MAX_PRIORITY;
static JAVA_INT _STATIC_java_lang_Thread_MIN_PRIORITY;
static JAVA_INT _STATIC_java_lang_Thread_NORM_PRIORITY;

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

void __INIT_java_lang_Thread()
{
    staticInitializerLock(&__TIB_java_lang_Thread);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Thread.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Thread.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Thread);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Thread.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Thread.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Thread.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Thread")
        __INIT_IMPL_java_lang_Thread();
    }
}

void __INIT_IMPL_java_lang_Thread()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Thread.newInstanceFunc = __NEW_INSTANCE_java_lang_Thread;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Thread.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Thread.vtable[1] = (VTABLE_PTR) &java_lang_Thread_equals___java_lang_Object;
    __TIB_java_lang_Thread.vtable[6] = (VTABLE_PTR) &java_lang_Thread_run__;
    __TIB_java_lang_Thread.vtable[5] = (VTABLE_PTR) &java_lang_Thread_toString__;
    xmlvm_init_native_java_lang_Thread();
    // Initialize interface information
    __TIB_java_lang_Thread.numImplementedInterfaces = 1;
    __TIB_java_lang_Thread.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Runnable)

    __TIB_java_lang_Thread.implementedInterfaces[0][0] = &__TIB_java_lang_Runnable;
    // Initialize itable for this class
    __TIB_java_lang_Thread.itableBegin = &__TIB_java_lang_Thread.itable[0];
    __TIB_java_lang_Thread.itable[XMLVM_ITABLE_IDX_java_lang_Runnable_run__] = __TIB_java_lang_Thread.vtable[6];

    _STATIC_java_lang_Thread_threadSafetyMutex = (org_xmlvm_runtime_Mutex*) JAVA_NULL;
    _STATIC_java_lang_Thread_nextThreadId = 0;
    _STATIC_java_lang_Thread_numberOfActiveNonDaemonThreads = 0;
    _STATIC_java_lang_Thread_threadMap = (java_util_Map*) JAVA_NULL;
    _STATIC_java_lang_Thread_mainThreadGroup = (java_lang_ThreadGroup*) JAVA_NULL;
    _STATIC_java_lang_Thread_MAX_PRIORITY = 10;
    _STATIC_java_lang_Thread_MIN_PRIORITY = 1;
    _STATIC_java_lang_Thread_NORM_PRIORITY = 5;

    __TIB_java_lang_Thread.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Thread.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Thread.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Thread.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Thread.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Thread.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Thread.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Thread.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Thread = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Thread);
    __TIB_java_lang_Thread.clazz = __CLASS_java_lang_Thread;
    __TIB_java_lang_Thread.baseType = JAVA_NULL;
    __CLASS_java_lang_Thread_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread);
    __CLASS_java_lang_Thread_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_1ARRAY);
    __CLASS_java_lang_Thread_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_2ARRAY);
    java_lang_Thread___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Thread]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Thread.classInitialized = 1;
}

void __DELETE_java_lang_Thread(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Thread]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Thread(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Thread*) me)->fields.java_lang_Thread.xmlvmExceptionEnv_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.xmlvmException_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.ptBuffers_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.threadId_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.threadName_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.priority_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.daemon_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.alive_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.targetRunnable_ = (java_lang_Runnable*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.threadGroup_ = (java_lang_ThreadGroup*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.threadState_ = (java_lang_Thread_State*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.interrupted_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.waitingCondition_ = (org_xmlvm_runtime_Condition*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.threadLocalMap_ = (java_util_Map*) JAVA_NULL;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.nativeThreadId_ = 0;
    ((java_lang_Thread*) me)->fields.java_lang_Thread.action_ = (java_lang_Runnable*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Thread]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Thread()
{    XMLVM_CLASS_INIT(java_lang_Thread)
java_lang_Thread* me = (java_lang_Thread*) XMLVM_MALLOC(sizeof(java_lang_Thread));
    me->tib = &__TIB_java_lang_Thread;
    __INIT_INSTANCE_MEMBERS_java_lang_Thread(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Thread]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Thread()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_Thread();
    java_lang_Thread___INIT___(me);
    return me;
}

JAVA_OBJECT java_lang_Thread_GET_threadSafetyMutex()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_threadSafetyMutex;
}

void java_lang_Thread_PUT_threadSafetyMutex(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_threadSafetyMutex = v;
}

JAVA_LONG java_lang_Thread_GET_nextThreadId()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_nextThreadId;
}

void java_lang_Thread_PUT_nextThreadId(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_nextThreadId = v;
}

JAVA_INT java_lang_Thread_GET_numberOfActiveNonDaemonThreads()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_numberOfActiveNonDaemonThreads;
}

void java_lang_Thread_PUT_numberOfActiveNonDaemonThreads(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_numberOfActiveNonDaemonThreads = v;
}

JAVA_OBJECT java_lang_Thread_GET_threadMap()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_threadMap;
}

void java_lang_Thread_PUT_threadMap(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_threadMap = v;
}

JAVA_OBJECT java_lang_Thread_GET_mainThreadGroup()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_mainThreadGroup;
}

void java_lang_Thread_PUT_mainThreadGroup(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_mainThreadGroup = v;
}

JAVA_INT java_lang_Thread_GET_MAX_PRIORITY()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_MAX_PRIORITY;
}

void java_lang_Thread_PUT_MAX_PRIORITY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_MAX_PRIORITY = v;
}

JAVA_INT java_lang_Thread_GET_MIN_PRIORITY()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_MIN_PRIORITY;
}

void java_lang_Thread_PUT_MIN_PRIORITY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_MIN_PRIORITY = v;
}

JAVA_INT java_lang_Thread_GET_NORM_PRIORITY()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    return _STATIC_java_lang_Thread_NORM_PRIORITY;
}

void java_lang_Thread_PUT_NORM_PRIORITY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
_STATIC_java_lang_Thread_NORM_PRIORITY = v;
}

JAVA_LONG java_lang_Thread_getNextThreadId__()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getNextThreadId__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getNextThreadId", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVM_SOURCE_POSITION("Thread.java", 120)
    _r2.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(2)
    org_xmlvm_runtime_Mutex_lock__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 121)
    _r0.l = java_lang_Thread_GET_nextThreadId();
    _r2.l = 1;
    _r2.l = _r2.l + _r0.l;
    java_lang_Thread_PUT_nextThreadId( _r2.l);
    XMLVM_SOURCE_POSITION("Thread.java", 122)
    _r2.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(2)
    org_xmlvm_runtime_Mutex_unlock__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 123)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_addSelfToMap__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_addSelfToMap__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "addSelfToMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 159)
    _r0.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_lock__(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 161)
    _r0.o = java_lang_Thread_GET_threadMap();
    XMLVM_CHECK_NPE(3)
    _r1.l = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.nativeThreadId_;
    _r1.o = java_lang_Long_valueOf___long(_r1.l);
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object])(_r0.o, _r1.o, _r3.o);
    XMLVM_SOURCE_POSITION("Thread.java", 163)
    _r0.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_unlock__(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 164)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_removeSelfFromMap__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_removeSelfFromMap__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "removeSelfFromMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 167)
    _r0.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_lock__(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 169)
    _r0.o = java_lang_Thread_GET_threadMap();
    XMLVM_CHECK_NPE(3)
    _r1.l = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.nativeThreadId_;
    _r1.o = java_lang_Long_valueOf___long(_r1.l);
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object])(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 171)
    _r0.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_unlock__(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 172)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Thread_getNativeThreadId__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getNativeThreadId__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getNativeThreadId", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 177)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.nativeThreadId_;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.l = n1;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 186)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 187)
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initMainThread__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 189)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.nativeThreadId_ = _r3.l;
    XMLVM_SOURCE_POSITION("Thread.java", 192)
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_addSelfToMap__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 194)
    _r0.l = 1;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadId_ = _r0.l;
    XMLVM_SOURCE_POSITION("Thread.java", 195)
    // "main"
    _r0.o = xmlvm_create_java_string_from_pool(663);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadName_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 197)
    _r0.o = java_lang_Thread_GET_mainThreadGroup();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadGroup_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 198)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadGroup_;
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_add___java_lang_Thread(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 199)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_initMainThread__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Thread_getThread___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getThread___long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getThread", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.l = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 211)
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Thread.java", 212)
    _r1.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_lock__(_r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 214)
    _r1.o = java_lang_Thread_GET_threadMap();
    _r2.o = java_lang_Long_valueOf___long(_r3.l);
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object])(_r1.o, _r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 216)
    _r1.o = java_lang_Thread_GET_threadSafetyMutex();
    XMLVM_CHECK_NPE(1)
    org_xmlvm_runtime_Mutex_unlock__(_r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 218)
    if (_r0.o != JAVA_NULL) goto label30;
    XMLVM_SOURCE_POSITION("Thread.java", 221)
    _r0.o = __NEW_java_lang_Thread();
    XMLVM_CHECK_NPE(0)
    java_lang_Thread___INIT____long(_r0.o, _r3.l);
    label30:;
    XMLVM_SOURCE_POSITION("Thread.java", 223)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Thread_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Thread", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r7.o = me;
    _r8.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 227)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 228)
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r3.i = XMLVM_ISA(_r8.o, __CLASS_java_lang_Thread);
    if (_r3.i == 0) goto label19;
    XMLVM_SOURCE_POSITION("Thread.java", 229)
    _r0 = _r8;
    _r0.o = _r0.o;
    _r2 = _r0;
    XMLVM_SOURCE_POSITION("Thread.java", 230)
    XMLVM_CHECK_NPE(7)
    _r3.l = ((java_lang_Thread*) _r7.o)->fields.java_lang_Thread.nativeThreadId_;
    XMLVM_CHECK_NPE(2)
    _r5.l = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.nativeThreadId_;
    _r3.i = _r3.l > _r5.l ? 1 : (_r3.l == _r5.l ? 0 : -1);
    if (_r3.i != 0) goto label20;
    _r3.i = 1;
    _r1 = _r3;
    label19:;
    XMLVM_SOURCE_POSITION("Thread.java", 232)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label20:;
    _r3.i = 0;
    _r1 = _r3;
    goto label19;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "initThread", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r3.o = me;
    _r4.o = n1;
    _r5.o = n2;
    _r6.o = n3;
    XMLVM_SOURCE_POSITION("Thread.java", 259)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.targetRunnable_ = _r5.o;
    XMLVM_SOURCE_POSITION("Thread.java", 260)
    _r0.l = java_lang_Thread_getNextThreadId__();
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadId_ = _r0.l;
    XMLVM_SOURCE_POSITION("Thread.java", 262)
    if (_r6.o != JAVA_NULL) goto label46;
    XMLVM_SOURCE_POSITION("Thread.java", 263)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    // "Thread-"
    _r1.o = xmlvm_create_java_string_from_pool(802);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    _r1.l = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadId_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___long(_r0.o, _r1.l);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadName_ = _r0.o;
    label33:;
    XMLVM_SOURCE_POSITION("Thread.java", 268)
    if (_r4.o != JAVA_NULL) goto label49;
    XMLVM_SOURCE_POSITION("Thread.java", 270)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Thread_getThreadGroup__(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadGroup_ = _r0.o;
    label45:;
    XMLVM_SOURCE_POSITION("Thread.java", 274)
    XMLVM_EXIT_METHOD()
    return;
    label46:;
    XMLVM_SOURCE_POSITION("Thread.java", 265)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadName_ = _r6.o;
    goto label33;
    label49:;
    XMLVM_SOURCE_POSITION("Thread.java", 272)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadGroup_ = _r4.o;
    goto label45;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    _r2.i = 0;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Thread.java", 283)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.daemon_ = _r2.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.alive_ = _r2.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 284)
    XMLVM_CHECK_NPE(3)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r3.o, _r1.o, _r1.o, _r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 285)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_Runnable]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.o = JAVA_NULL;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 297)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 298)
    XMLVM_CHECK_NPE(3)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r3.o, _r2.o, _r4.o, _r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 299)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_Runnable_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 312)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 313)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r2.o, _r0.o, _r3.o, _r4.o);
    XMLVM_SOURCE_POSITION("Thread.java", 314)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.o = JAVA_NULL;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 325)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 326)
    XMLVM_CHECK_NPE(3)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r3.o, _r2.o, _r2.o, _r4.o);
    XMLVM_SOURCE_POSITION("Thread.java", 327)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 345)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 346)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r2.o, _r3.o, _r4.o, _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 347)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_LONG n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String_long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r5.o = n3;
    _r6.l = n4;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 367)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 368)
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r2.o, _r3.o, _r4.o, _r5.o);
    XMLVM_SOURCE_POSITION("Thread.java", 369)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r5.o = n3;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 388)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 389)
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r2.o, _r3.o, _r4.o, _r5.o);
    XMLVM_SOURCE_POSITION("Thread.java", 390)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 406)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 131)
    _r0.i = 5;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.priority_ = _r0.i;
    XMLVM_SOURCE_POSITION("Thread.java", 132)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 133)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 137)
    _r0.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 143)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadLocalMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 407)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(_r2.o, _r3.o, _r0.o, _r4.o);
    XMLVM_SOURCE_POSITION("Thread.java", 408)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_setInterruptAction___java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_setInterruptAction___java_lang_Runnable]
    XMLVM_ENTER_METHOD("java.lang.Thread", "setInterruptAction", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 423)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Thread*) _r0.o)->fields.java_lang_Thread.action_ = _r1.o;
    XMLVM_SOURCE_POSITION("Thread.java", 424)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_Thread_activeCount__()]

//XMLVM_NATIVE[void java_lang_Thread_checkAccess__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_INT java_lang_Thread_countStackFrames__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Thread_currentThread__()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_currentThread__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "currentThread", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Thread.java", 466)
    _r0.l = java_lang_Thread_getCurrentThreadNativeId__();
    _r0.o = java_lang_Thread_getThread___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG java_lang_Thread_getCurrentThreadNativeId__()]

//XMLVM_NATIVE[void java_lang_Thread_destroy__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void java_lang_Thread_dumpStack__()]

//XMLVM_NATIVE[JAVA_INT java_lang_Thread_enumerate___java_lang_Thread_1ARRAY(JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Thread_getAllStackTraces__()]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Thread_getContextClassLoader__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Thread_getDefaultUncaughtExceptionHandler__()]

JAVA_LONG java_lang_Thread_getId__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getId__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getId", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 566)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadId_;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Thread_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getName__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 575)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.threadName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Thread_getPriority__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getPriority__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getPriority", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 585)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.priority_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Thread_getStackTrace__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Thread_getState__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getState__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getState", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 612)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w5666aaac57b1b4)
    // Begin try
    _r0.o = java_lang_System_GET_out();
    // "Thread.getState() is not fully implemented. Specifically, you will not currently find BLOCKED, WAITING and TIMED_WAITING."
    _r1.o = xmlvm_create_java_string_from_pool(803);
    XMLVM_CHECK_NPE(0)
    java_io_PrintStream_println___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 613)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac57b1b4)
        XMLVM_CATCH_SPECIFIC(w5666aaac57b1b4,java_lang_Object,12)
    XMLVM_CATCH_END(w5666aaac57b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac57b1b4)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label12:;
    java_lang_Thread* curThread_w5666aaac57b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac57b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Thread_getThreadGroup__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getThreadGroup__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getThreadGroup", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 622)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.threadGroup_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Thread_getThreadLocal___java_lang_ThreadLocal(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_getThreadLocal___java_lang_ThreadLocal]
    XMLVM_ENTER_METHOD("java.lang.Thread", "getThreadLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 636)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.threadLocalMap_;
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object])(_r0.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Thread_getUncaughtExceptionHandler__(JAVA_OBJECT me)]

void java_lang_Thread_interrupt__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_interrupt__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "interrupt", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 680)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.action_;
    if (_r1.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Thread.java", 681)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.action_;
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Runnable_run__])(_r1.o);
    label9:;
    XMLVM_SOURCE_POSITION("Thread.java", 684)
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Thread.java", 685)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 686)
    _r1.i = 1;
    XMLVM_TRY_BEGIN(w5666aaac61b1c15)
    // Begin try
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.interrupted_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 688)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.waitingCondition_;
    if (_r1.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w5666aaac61b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac61b1c15, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Thread.java", 689)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.waitingCondition_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac61b1c15)
        XMLVM_CATCH_SPECIFIC(w5666aaac61b1c15,java_lang_Object,31)
    XMLVM_CATCH_END(w5666aaac61b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac61b1c15)
    label20:;
    XMLVM_TRY_BEGIN(w5666aaac61b1c17)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 691)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac61b1c17)
        XMLVM_CATCH_SPECIFIC(w5666aaac61b1c17,java_lang_Object,31)
    XMLVM_CATCH_END(w5666aaac61b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac61b1c17)
    XMLVM_SOURCE_POSITION("Thread.java", 695)
    if (_r0.o == JAVA_NULL) goto label30;
    XMLVM_SOURCE_POSITION("Thread.java", 696)
    XMLVM_CHECK_NPE(0)
    _r1.o = org_xmlvm_runtime_Condition_getSynchronizedObject__(_r0.o);
    XMLVM_CHECK_NPE(1)
    java_lang_Object_interruptWait___org_xmlvm_runtime_Condition(_r1.o, _r0.o);
    label30:;
    XMLVM_SOURCE_POSITION("Thread.java", 698)
    XMLVM_EXIT_METHOD()
    return;
    label31:;
    java_lang_Thread* curThread_w5666aaac61b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac61b1c27->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac61b1c28)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac61b1c28)
        XMLVM_CATCH_SPECIFIC(w5666aaac61b1c28,java_lang_Object,31)
    XMLVM_CATCH_END(w5666aaac61b1c28)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac61b1c28)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Thread_interrupted__()
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_interrupted__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "interrupted", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("Thread.java", 712)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 713)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_SOURCE_POSITION("Thread.java", 714)
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w5666aaac62b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 715)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_Thread_isInterrupted__(_r0.o);
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 716)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Thread*) _r0.o)->fields.java_lang_Thread.interrupted_ = _r2.i;
    XMLVM_SOURCE_POSITION("Thread.java", 717)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 718)
    XMLVM_MEMCPY(curThread_w5666aaac62b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac62b1b7, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac62b1b7)
        XMLVM_CATCH_SPECIFIC(w5666aaac62b1b7,java_lang_Object,15)
    XMLVM_CATCH_END(w5666aaac62b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac62b1b7)
    label15:;
    XMLVM_TRY_BEGIN(w5666aaac62b1b9)
    // Begin try
    java_lang_Thread* curThread_w5666aaac62b1b9aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w5666aaac62b1b9aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac62b1b9)
        XMLVM_CATCH_SPECIFIC(w5666aaac62b1b9,java_lang_Object,15)
    XMLVM_CATCH_END(w5666aaac62b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac62b1b9)
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Thread_isAlive__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_isAlive__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "isAlive", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 731)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w5666aaac63b1b4)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.alive_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac63b1b4)
        XMLVM_CATCH_SPECIFIC(w5666aaac63b1b4,java_lang_Object,5)
    XMLVM_CATCH_END(w5666aaac63b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac63b1b4)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label5:;
    java_lang_Thread* curThread_w5666aaac63b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac63b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Thread_isDaemon__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_isDaemon__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "isDaemon", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 745)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w5666aaac64b1b4)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.daemon_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac64b1b4)
        XMLVM_CATCH_SPECIFIC(w5666aaac64b1b4,java_lang_Object,5)
    XMLVM_CATCH_END(w5666aaac64b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac64b1b4)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label5:;
    java_lang_Thread* curThread_w5666aaac64b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac64b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Thread_isInterrupted__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_isInterrupted__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "isInterrupted", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 758)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 759)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w5666aaac65b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 760)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.interrupted_;
    XMLVM_SOURCE_POSITION("Thread.java", 761)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 762)
    XMLVM_MEMCPY(curThread_w5666aaac65b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac65b1b6, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac65b1b6)
        XMLVM_CATCH_SPECIFIC(w5666aaac65b1b6,java_lang_Object,6)
    XMLVM_CATCH_END(w5666aaac65b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac65b1b6)
    label6:;
    XMLVM_TRY_BEGIN(w5666aaac65b1b8)
    // Begin try
    java_lang_Thread* curThread_w5666aaac65b1b8aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac65b1b8aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac65b1b8)
        XMLVM_CATCH_SPECIFIC(w5666aaac65b1b8,java_lang_Object,6)
    XMLVM_CATCH_END(w5666aaac65b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac65b1b8)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_join__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_join__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "join", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 775)
    _r0.l = 0;
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_join___long(_r2.o, _r0.l);
    XMLVM_SOURCE_POSITION("Thread.java", 776)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_join___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_join___long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "join", "?")
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
    volatile XMLVMElem _r11;
    volatile XMLVMElem _r12;
    volatile XMLVMElem _r13;
    _r11.o = me;
    _r12.l = n1;
    _r9.l = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 790)
    java_lang_Object_acquireLockRecursive__(_r11.o);
    XMLVM_TRY_BEGIN(w5666aaac67b1b6)
    // Begin try
    _r0.l = java_lang_System_currentTimeMillis__();
    _r5.l = 0;
    _r7.i = _r12.l > _r9.l ? 1 : (_r12.l == _r9.l ? 0 : -1);
    if (_r7.i >= 0) { XMLVM_MEMCPY(curThread_w5666aaac67b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1b6, sizeof(XMLVM_JMP_BUF)); goto label24; };
    XMLVM_SOURCE_POSITION("Thread.java", 793)
    XMLVM_SOURCE_POSITION("Thread.java", 794)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "timeout value is negative"
    _r8.o = xmlvm_create_java_string_from_pool(804);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r7.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac67b1b6)
        XMLVM_CATCH_SPECIFIC(w5666aaac67b1b6,java_lang_Object,21)
    XMLVM_CATCH_END(w5666aaac67b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac67b1b6)
    label21:;
    java_lang_Thread* curThread_w5666aaac67b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r7.o = curThread_w5666aaac67b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r11.o);
    XMLVM_THROW_CUSTOM(_r7.o)
    label24:;
    XMLVM_SOURCE_POSITION("Thread.java", 797)
    _r7.i = _r12.l > _r9.l ? 1 : (_r12.l == _r9.l ? 0 : -1);
    if (_r7.i != 0) goto label38;
    label28:;
    XMLVM_TRY_BEGIN(w5666aaac67b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 798)
    XMLVM_CHECK_NPE(11)
    _r7.i = java_lang_Thread_isAlive__(_r11.o);
    if (_r7.i == 0) { XMLVM_MEMCPY(curThread_w5666aaac67b1c16->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c16, sizeof(XMLVM_JMP_BUF)); goto label65; };
    XMLVM_SOURCE_POSITION("Thread.java", 800)
    XMLVM_CHECK_NPE(11)
    java_lang_Object_wait__(_r11.o);
    { XMLVM_MEMCPY(curThread_w5666aaac67b1c16->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c16, sizeof(XMLVM_JMP_BUF)); goto label28; };
    XMLVM_SOURCE_POSITION("Thread.java", 803)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac67b1c16)
        XMLVM_CATCH_SPECIFIC(w5666aaac67b1c16,java_lang_Object,21)
    XMLVM_CATCH_END(w5666aaac67b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac67b1c16)
    label38:;
    XMLVM_TRY_BEGIN(w5666aaac67b1c18)
    // Begin try
    _r4.i = 0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac67b1c18)
        XMLVM_CATCH_SPECIFIC(w5666aaac67b1c18,java_lang_Object,21)
    XMLVM_CATCH_END(w5666aaac67b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac67b1c18)
    label39:;
    XMLVM_TRY_BEGIN(w5666aaac67b1c20)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 804)
    if (_r4.i != 0) { XMLVM_MEMCPY(curThread_w5666aaac67b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c20, sizeof(XMLVM_JMP_BUF)); goto label65; };
    XMLVM_CHECK_NPE(11)
    _r7.i = java_lang_Thread_isAlive__(_r11.o);
    if (_r7.i == 0) { XMLVM_MEMCPY(curThread_w5666aaac67b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c20, sizeof(XMLVM_JMP_BUF)); goto label65; };
    XMLVM_SOURCE_POSITION("Thread.java", 805)
    _r2.l = _r12.l - _r5.l;
    _r7.i = _r2.l > _r9.l ? 1 : (_r2.l == _r9.l ? 0 : -1);
    if (_r7.i > 0) { XMLVM_MEMCPY(curThread_w5666aaac67b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c20, sizeof(XMLVM_JMP_BUF)); goto label55; };
    XMLVM_SOURCE_POSITION("Thread.java", 806)
    XMLVM_SOURCE_POSITION("Thread.java", 807)
    _r4.i = 1;
    { XMLVM_MEMCPY(curThread_w5666aaac67b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac67b1c20, sizeof(XMLVM_JMP_BUF)); goto label39; };
    XMLVM_SOURCE_POSITION("Thread.java", 810)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac67b1c20)
        XMLVM_CATCH_SPECIFIC(w5666aaac67b1c20,java_lang_Object,21)
    XMLVM_CATCH_END(w5666aaac67b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac67b1c20)
    label55:;
    XMLVM_TRY_BEGIN(w5666aaac67b1c22)
    // Begin try
    XMLVM_CHECK_NPE(11)
    java_lang_Object_wait___long(_r11.o, _r2.l);
    XMLVM_SOURCE_POSITION("Thread.java", 811)
    _r7.l = java_lang_System_currentTimeMillis__();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac67b1c22)
        XMLVM_CATCH_SPECIFIC(w5666aaac67b1c22,java_lang_Object,21)
    XMLVM_CATCH_END(w5666aaac67b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac67b1c22)
    _r5.l = _r7.l - _r0.l;
    goto label39;
    label65:;
    XMLVM_SOURCE_POSITION("Thread.java", 815)
    java_lang_Object_releaseLockRecursive__(_r11.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_join___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2)]

//XMLVM_NATIVE[void java_lang_Thread_resume__(JAVA_OBJECT me)]

void java_lang_Thread_run0___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_run0___long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "run0", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r6.o = me;
    _r7.l = n1;
    // "Exception in thread \042"
    _r5.o = xmlvm_create_java_string_from_pool(805);
    // "\042 "
    _r4.o = xmlvm_create_java_string_from_pool(806);
    XMLVM_SOURCE_POSITION("Thread.java", 854)
    XMLVM_CHECK_NPE(6)
    ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.nativeThreadId_ = _r7.l;
    XMLVM_SOURCE_POSITION("Thread.java", 855)
    XMLVM_CHECK_NPE(6)
    java_lang_Thread_addSelfToMap__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 857)
    java_lang_Object_acquireLockRecursive__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 858)
    _r1.i = 1;
    XMLVM_TRY_BEGIN(w5666aaac70b1c13)
    // Begin try
    XMLVM_CHECK_NPE(6)
    ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 859)
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c13)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c13,java_lang_Object,41)
    XMLVM_CATCH_END(w5666aaac70b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c13)
    XMLVM_TRY_BEGIN(w5666aaac70b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 862)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.targetRunnable_;
    if (_r1.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w5666aaac70b1c14->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac70b1c14, sizeof(XMLVM_JMP_BUF)); goto label44; };
    XMLVM_SOURCE_POSITION("Thread.java", 863)
    //java_lang_Thread_run__[6]
    XMLVM_CHECK_NPE(6)
    (*(void (*)(JAVA_OBJECT)) ((java_lang_Thread*) _r6.o)->tib->vtable[6])(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c14)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c14,java_lang_Throwable,50)
    XMLVM_CATCH_END(w5666aaac70b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c14)
    label21:;
    XMLVM_SOURCE_POSITION("Thread.java", 877)
    java_lang_Object_acquireLockRecursive__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 878)
    _r1.i = 0;
    XMLVM_TRY_BEGIN(w5666aaac70b1c20)
    // Begin try
    XMLVM_CHECK_NPE(6)
    ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.alive_ = _r1.i;
    XMLVM_SOURCE_POSITION("Thread.java", 881)
    XMLVM_CHECK_NPE(6)
    java_lang_Object_notifyAll__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 882)
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c20)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c20,java_lang_Object,158)
    XMLVM_CATCH_END(w5666aaac70b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c20)
    XMLVM_SOURCE_POSITION("Thread.java", 884)
    XMLVM_CHECK_NPE(6)
    java_lang_Thread_removeSelfFromMap__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 886)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.threadGroup_;
    XMLVM_CHECK_NPE(1)
    java_lang_ThreadGroup_remove___java_lang_Thread(_r1.o, _r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 888)
    XMLVM_CHECK_NPE(6)
    java_lang_Thread_threadTerminating__(_r6.o);
    XMLVM_SOURCE_POSITION("Thread.java", 889)
    XMLVM_EXIT_METHOD()
    return;
    label41:;
    java_lang_Thread* curThread_w5666aaac70b1c31 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac70b1c31->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac70b1c32)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c32)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c32,java_lang_Object,41)
    XMLVM_CATCH_END(w5666aaac70b1c32)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c32)
    XMLVM_THROW_CUSTOM(_r1.o)
    label44:;
    XMLVM_TRY_BEGIN(w5666aaac70b1c35)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 865)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_Thread*) _r6.o)->fields.java_lang_Thread.targetRunnable_;
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Runnable_run__])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c35)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c35,java_lang_Throwable,50)
    XMLVM_CATCH_END(w5666aaac70b1c35)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c35)
    goto label21;
    label50:;
    XMLVM_SOURCE_POSITION("Thread.java", 867)
    java_lang_Thread* curThread_w5666aaac70b1c39 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac70b1c39->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Thread.java", 868)
    _r1.i = java_lang_Thread_stackTracesEnabled__();
    if (_r1.i == 0) goto label96;
    XMLVM_SOURCE_POSITION("Thread.java", 869)
    _r1.o = java_lang_System_GET_out();
    _r2.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT___(_r2.o);
    // "Exception in thread \042"
    _r3.o = xmlvm_create_java_string_from_pool(805);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r5.o);
    XMLVM_CHECK_NPE(6)
    _r3.o = java_lang_Thread_getName__(_r6.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r3.o);
    // "\042 "
    _r3.o = xmlvm_create_java_string_from_pool(806);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_io_PrintStream_print___java_lang_String(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 870)
    XMLVM_CHECK_NPE(0)
    java_lang_Throwable_printStackTrace__(_r0.o);
    goto label21;
    label96:;
    XMLVM_SOURCE_POSITION("Thread.java", 872)
    _r1.o = java_lang_System_GET_out();
    _r2.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT___(_r2.o);
    // "Exception in thread \042"
    _r3.o = xmlvm_create_java_string_from_pool(805);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r5.o);
    XMLVM_CHECK_NPE(6)
    _r3.o = java_lang_Thread_getName__(_r6.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r3.o);
    // "\042 "
    _r3.o = xmlvm_create_java_string_from_pool(806);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r4.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_Class_getName__(_r3.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r3.o);
    // ": "
    _r3.o = xmlvm_create_java_string_from_pool(49);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r3.o);
    //java_lang_Throwable_getMessage__[7]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r0.o)->tib->vtable[7])(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_io_PrintStream_println___java_lang_String(_r1.o, _r2.o);
    goto label21;
    label158:;
    java_lang_Thread* curThread_w5666aaac70b1c81 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac70b1c81->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac70b1c82)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac70b1c82)
        XMLVM_CATCH_SPECIFIC(w5666aaac70b1c82,java_lang_Object,158)
    XMLVM_CATCH_END(w5666aaac70b1c82)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac70b1c82)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Thread_stackTracesEnabled__()]

void java_lang_Thread_threadTerminating__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_threadTerminating__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "threadTerminating", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 894)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w5666aaac72b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 895)
    _r0.o = java_lang_Thread_State_GET_TERMINATED();
    XMLVM_CHECK_NPE(3)
    ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    XMLVM_SOURCE_POSITION("Thread.java", 896)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac72b1b4)
        XMLVM_CATCH_SPECIFIC(w5666aaac72b1b4,java_lang_Object,29)
    XMLVM_CATCH_END(w5666aaac72b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac72b1b4)
    XMLVM_SOURCE_POSITION("Thread.java", 900)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.daemon_;
    if (_r0.i != 0) goto label28;
    XMLVM_SOURCE_POSITION("Thread.java", 901)
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = __CLASS_java_lang_Thread;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w5666aaac72b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 902)
    _r1.i = java_lang_Thread_GET_numberOfActiveNonDaemonThreads();
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    java_lang_Thread_PUT_numberOfActiveNonDaemonThreads( _r1.i);
    XMLVM_SOURCE_POSITION("Thread.java", 904)
    _r1.i = java_lang_Thread_GET_numberOfActiveNonDaemonThreads();
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w5666aaac72b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac72b1c11, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_SOURCE_POSITION("Thread.java", 905)
    _r1.i = 0;
    java_lang_System_exit___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac72b1c11)
        XMLVM_CATCH_SPECIFIC(w5666aaac72b1c11,java_lang_Object,32)
    XMLVM_CATCH_END(w5666aaac72b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac72b1c11)
    label27:;
    XMLVM_TRY_BEGIN(w5666aaac72b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 907)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac72b1c13)
        XMLVM_CATCH_SPECIFIC(w5666aaac72b1c13,java_lang_Object,32)
    XMLVM_CATCH_END(w5666aaac72b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac72b1c13)
    label28:;
    XMLVM_SOURCE_POSITION("Thread.java", 909)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    java_lang_Thread* curThread_w5666aaac72b1c18 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac72b1c18->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac72b1c19)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac72b1c19)
        XMLVM_CATCH_SPECIFIC(w5666aaac72b1c19,java_lang_Object,29)
    XMLVM_CATCH_END(w5666aaac72b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac72b1c19)
    XMLVM_THROW_CUSTOM(_r0.o)
    label32:;
    java_lang_Thread* curThread_w5666aaac72b1c22 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac72b1c22->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac72b1c23)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac72b1c23)
        XMLVM_CATCH_SPECIFIC(w5666aaac72b1c23,java_lang_Object,32)
    XMLVM_CATCH_END(w5666aaac72b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac72b1c23)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_run__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_run__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "run", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 919)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_setContextClassLoader___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1)]

void java_lang_Thread_setDaemon___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_setDaemon___boolean]
    XMLVM_ENTER_METHOD("java.lang.Thread", "setDaemon", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 946)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w5666aaac75b1b5)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_;
    _r1.o = java_lang_Thread_State_GET_NEW();
    if (_r0.o == _r1.o) { XMLVM_MEMCPY(curThread_w5666aaac75b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac75b1b5, sizeof(XMLVM_JMP_BUF)); goto label16; };
    XMLVM_SOURCE_POSITION("Thread.java", 947)

    
    // Red class access removed: java.lang.IllegalThreadStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalThreadStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac75b1b5)
        XMLVM_CATCH_SPECIFIC(w5666aaac75b1b5,java_lang_Object,13)
    XMLVM_CATCH_END(w5666aaac75b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac75b1b5)
    label13:;
    java_lang_Thread* curThread_w5666aaac75b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac75b1b7->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_TRY_BEGIN(w5666aaac75b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 949)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_ = _r3.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac75b1c11)
        XMLVM_CATCH_SPECIFIC(w5666aaac75b1c11,java_lang_Object,13)
    XMLVM_CATCH_END(w5666aaac75b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac75b1c11)
    XMLVM_SOURCE_POSITION("Thread.java", 950)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_setDefaultUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT n1)]

void java_lang_Thread_setName___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_setName___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread", "setName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 977)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Thread*) _r0.o)->fields.java_lang_Thread.threadName_ = _r1.o;
    XMLVM_SOURCE_POSITION("Thread.java", 978)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_setPriority___int(JAVA_OBJECT me, JAVA_INT n1)]

void java_lang_Thread_setThreadLocal___java_lang_ThreadLocal_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_setThreadLocal___java_lang_ThreadLocal_java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Thread", "setThreadLocal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Thread.java", 1008)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.threadLocalMap_;
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object])(_r0.o, _r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("Thread.java", 1009)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT me, JAVA_OBJECT n1)]

void java_lang_Thread_sleep___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_sleep___long]
    XMLVM_ENTER_METHOD("java.lang.Thread", "sleep", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.l = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 1037)
    _r1.l = 0;
    _r1.i = _r3.l > _r1.l ? 1 : (_r3.l == _r1.l ? 0 : -1);
    if (_r1.i == 0) goto label16;
    XMLVM_SOURCE_POSITION("Thread.java", 1038)
    _r0.o = __NEW_java_lang_Object();
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 1039)
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w5666aaac81b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1040)
    XMLVM_CHECK_NPE(0)
    java_lang_Object_wait___long(_r0.o, _r3.l);
    XMLVM_SOURCE_POSITION("Thread.java", 1041)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac81b1c11)
        XMLVM_CATCH_SPECIFIC(w5666aaac81b1c11,java_lang_Object,17)
    XMLVM_CATCH_END(w5666aaac81b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac81b1c11)
    label16:;
    XMLVM_TRY_BEGIN(w5666aaac81b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1043)
    XMLVM_MEMCPY(curThread_w5666aaac81b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac81b1c13, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac81b1c13)
        XMLVM_CATCH_SPECIFIC(w5666aaac81b1c13,java_lang_Object,17)
    XMLVM_CATCH_END(w5666aaac81b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac81b1c13)
    label17:;
    XMLVM_TRY_BEGIN(w5666aaac81b1c15)
    // Begin try
    java_lang_Thread* curThread_w5666aaac81b1c15aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac81b1c15aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac81b1c15)
        XMLVM_CATCH_SPECIFIC(w5666aaac81b1c15,java_lang_Object,17)
    XMLVM_CATCH_END(w5666aaac81b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac81b1c15)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_sleep___long_int(JAVA_LONG n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Thread)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_sleep___long_int]
    XMLVM_ENTER_METHOD("java.lang.Thread", "sleep", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.l = n1;
    _r7.i = n2;
    _r3.l = 1;
    _r1.l = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 1060)
    _r0.i = _r5.l > _r1.l ? 1 : (_r5.l == _r1.l ? 0 : -1);
    if (_r0.i >= 0) goto label16;
    XMLVM_SOURCE_POSITION("Thread.java", 1061)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "sleep milliseconds must be greater than or equal to zero"
    _r1.o = xmlvm_create_java_string_from_pool(807);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("Thread.java", 1062)
    if (_r7.i < 0) goto label23;
    _r0.i = 1000000;
    if (_r7.i < _r0.i) goto label31;
    label23:;
    XMLVM_SOURCE_POSITION("Thread.java", 1063)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "sleep nanoseconds must be greater than or equal to zero and less than 1000000"
    _r1.o = xmlvm_create_java_string_from_pool(808);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    XMLVM_SOURCE_POSITION("Thread.java", 1066)
    _r0.i = _r5.l > _r1.l ? 1 : (_r5.l == _r1.l ? 0 : -1);
    if (_r0.i != 0) goto label41;
    if (_r7.i == 0) goto label41;
    XMLVM_SOURCE_POSITION("Thread.java", 1067)
    java_lang_Thread_sleep___long(_r3.l);
    label40:;
    XMLVM_SOURCE_POSITION("Thread.java", 1074)
    XMLVM_EXIT_METHOD()
    return;
    label41:;
    XMLVM_SOURCE_POSITION("Thread.java", 1069)
    _r0.i = 500000;
    if (_r7.i < _r0.i) goto label47;
    XMLVM_SOURCE_POSITION("Thread.java", 1070)
    _r5.l = _r5.l + _r3.l;
    label47:;
    XMLVM_SOURCE_POSITION("Thread.java", 1072)
    java_lang_Thread_sleep___long(_r5.l);
    goto label40;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_start__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_start__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "start", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Thread.java", 1085)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w5666aaac83b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1086)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_;
    _r1.o = java_lang_Thread_State_GET_NEW();
    if (_r0.o == _r1.o) { XMLVM_MEMCPY(curThread_w5666aaac83b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac83b1b4, sizeof(XMLVM_JMP_BUF)); goto label16; };
    XMLVM_SOURCE_POSITION("Thread.java", 1087)

    
    // Red class access removed: java.lang.IllegalThreadStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalThreadStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac83b1b4)
        XMLVM_CATCH_SPECIFIC(w5666aaac83b1b4,java_lang_Object,13)
    XMLVM_CATCH_END(w5666aaac83b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac83b1b4)
    label13:;
    XMLVM_TRY_BEGIN(w5666aaac83b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1090)
    java_lang_Thread* curThread_w5666aaac83b1b6ab1 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac83b1b6ab1->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac83b1b6)
        XMLVM_CATCH_SPECIFIC(w5666aaac83b1b6,java_lang_Object,13)
    XMLVM_CATCH_END(w5666aaac83b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac83b1b6)
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_TRY_BEGIN(w5666aaac83b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1089)
    _r0.o = java_lang_Thread_State_GET_RUNNABLE();
    XMLVM_CHECK_NPE(2)
    ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadState_ = _r0.o;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac83b1b9)
        XMLVM_CATCH_SPECIFIC(w5666aaac83b1b9,java_lang_Object,13)
    XMLVM_CATCH_END(w5666aaac83b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac83b1b9)
    XMLVM_SOURCE_POSITION("Thread.java", 1094)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.daemon_;
    if (_r0.i != 0) goto label35;
    XMLVM_SOURCE_POSITION("Thread.java", 1095)
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = __CLASS_java_lang_Thread;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w5666aaac83b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1096)
    _r1.i = java_lang_Thread_GET_numberOfActiveNonDaemonThreads();
    _r1.i = _r1.i + 1;
    java_lang_Thread_PUT_numberOfActiveNonDaemonThreads( _r1.i);
    XMLVM_SOURCE_POSITION("Thread.java", 1097)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac83b1c16)
        XMLVM_CATCH_SPECIFIC(w5666aaac83b1c16,java_lang_Object,44)
    XMLVM_CATCH_END(w5666aaac83b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac83b1c16)
    label35:;
    XMLVM_SOURCE_POSITION("Thread.java", 1099)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Thread*) _r2.o)->fields.java_lang_Thread.threadGroup_;
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_add___java_lang_Thread(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 1100)
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_start0__(_r2.o);
    XMLVM_SOURCE_POSITION("Thread.java", 1101)
    XMLVM_EXIT_METHOD()
    return;
    label44:;
    java_lang_Thread* curThread_w5666aaac83b1c26 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5666aaac83b1c26->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w5666aaac83b1c27)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac83b1c27)
        XMLVM_CATCH_SPECIFIC(w5666aaac83b1c27,java_lang_Object,44)
    XMLVM_CATCH_END(w5666aaac83b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac83b1c27)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_start0__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void java_lang_Thread_stop__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void java_lang_Thread_stop___java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[void java_lang_Thread_suspend__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Thread_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_toString__]
    XMLVM_ENTER_METHOD("java.lang.Thread", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    // ","
    _r2.o = xmlvm_create_java_string_from_pool(24);
    XMLVM_SOURCE_POSITION("Thread.java", 1158)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    // "Thread["
    _r1.o = xmlvm_create_java_string_from_pool(809);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadName_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // ","
    _r1.o = xmlvm_create_java_string_from_pool(24);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.priority_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    // ","
    _r1.o = xmlvm_create_java_string_from_pool(24);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadGroup_;
    if (_r1.o != JAVA_NULL) goto label58;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    label43:;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "]"
    _r1.o = xmlvm_create_java_string_from_pool(183);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label58:;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_Thread*) _r3.o)->fields.java_lang_Thread.threadGroup_;
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_ThreadGroup_getName__(_r1.o);
    goto label43;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_Thread_yield__()]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Thread_holdsLock___java_lang_Object(JAVA_OBJECT n1)]

void java_lang_Thread_setWaitingCondition___org_xmlvm_runtime_Condition(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_setWaitingCondition___org_xmlvm_runtime_Condition]
    XMLVM_ENTER_METHOD("java.lang.Thread", "setWaitingCondition", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 1201)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w5666aaac91b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("Thread.java", 1202)
    XMLVM_CHECK_NPE(1)
    ((java_lang_Thread*) _r1.o)->fields.java_lang_Thread.waitingCondition_ = _r2.o;
    XMLVM_SOURCE_POSITION("Thread.java", 1203)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("Thread.java", 1204)
    XMLVM_MEMCPY(curThread_w5666aaac91b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5666aaac91b1b5, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac91b1b5)
        XMLVM_CATCH_SPECIFIC(w5666aaac91b1b5,java_lang_Object,5)
    XMLVM_CATCH_END(w5666aaac91b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac91b1b5)
    label5:;
    XMLVM_TRY_BEGIN(w5666aaac91b1b7)
    // Begin try
    java_lang_Thread* curThread_w5666aaac91b1b7aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5666aaac91b1b7aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5666aaac91b1b7)
        XMLVM_CATCH_SPECIFIC(w5666aaac91b1b7,java_lang_Object,5)
    XMLVM_CATCH_END(w5666aaac91b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w5666aaac91b1b7)
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_lang_Thread___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Thread", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Thread.java", 115)
    _r0.o = __NEW_org_xmlvm_runtime_Mutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex___INIT___(_r0.o);
    java_lang_Thread_PUT_threadSafetyMutex( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 118)
    _r0.l = 2;
    java_lang_Thread_PUT_nextThreadId( _r0.l);
    XMLVM_SOURCE_POSITION("Thread.java", 127)
    _r0.i = 1;
    java_lang_Thread_PUT_numberOfActiveNonDaemonThreads( _r0.i);
    XMLVM_SOURCE_POSITION("Thread.java", 148)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    java_lang_Thread_PUT_threadMap( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 153)
    _r1.o = __NEW_java_lang_ThreadGroup();
    _r0.o = JAVA_NULL;
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(1)
    java_lang_ThreadGroup___INIT____java_lang_ThreadGroup(_r1.o, _r0.o);
    java_lang_Thread_PUT_mainThreadGroup( _r1.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

