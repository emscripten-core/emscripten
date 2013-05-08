#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_System.h"
#include "java_lang_ref_Reference.h"

#include "java_lang_ref_ReferenceQueue.h"

#define XMLVM_CURRENT_CLASS_NAME ReferenceQueue
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ref_ReferenceQueue

__TIB_DEFINITION_java_lang_ref_ReferenceQueue __TIB_java_lang_ref_ReferenceQueue = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ref_ReferenceQueue, // classInitializer
    "java.lang.ref.ReferenceQueue", // className
    "java.lang.ref", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_ref_ReferenceQueue), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue;
JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_lang_ref_ReferenceQueue_DEFAULT_QUEUE_SIZE;

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

void __INIT_java_lang_ref_ReferenceQueue()
{
    staticInitializerLock(&__TIB_java_lang_ref_ReferenceQueue);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ref_ReferenceQueue.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ref_ReferenceQueue.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ref_ReferenceQueue);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ref_ReferenceQueue.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ref_ReferenceQueue.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ref_ReferenceQueue.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ref.ReferenceQueue")
        __INIT_IMPL_java_lang_ref_ReferenceQueue();
    }
}

void __INIT_IMPL_java_lang_ref_ReferenceQueue()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_ref_ReferenceQueue.newInstanceFunc = __NEW_INSTANCE_java_lang_ref_ReferenceQueue;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ref_ReferenceQueue.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_ref_ReferenceQueue.numImplementedInterfaces = 0;
    __TIB_java_lang_ref_ReferenceQueue.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_ref_ReferenceQueue_DEFAULT_QUEUE_SIZE = 128;

    __TIB_java_lang_ref_ReferenceQueue.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ref_ReferenceQueue.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ref_ReferenceQueue.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ref_ReferenceQueue.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ref_ReferenceQueue.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ref_ReferenceQueue.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ref_ReferenceQueue.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ref_ReferenceQueue.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ref_ReferenceQueue = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ref_ReferenceQueue);
    __TIB_java_lang_ref_ReferenceQueue.clazz = __CLASS_java_lang_ref_ReferenceQueue;
    __TIB_java_lang_ref_ReferenceQueue.baseType = JAVA_NULL;
    __CLASS_java_lang_ref_ReferenceQueue_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_ReferenceQueue);
    __CLASS_java_lang_ref_ReferenceQueue_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_ReferenceQueue_1ARRAY);
    __CLASS_java_lang_ref_ReferenceQueue_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_ReferenceQueue_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ref_ReferenceQueue]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ref_ReferenceQueue.classInitialized = 1;
}

void __DELETE_java_lang_ref_ReferenceQueue(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ref_ReferenceQueue]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ref_ReferenceQueue*) me)->fields.java_lang_ref_ReferenceQueue.references_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_ref_ReferenceQueue*) me)->fields.java_lang_ref_ReferenceQueue.head_ = 0;
    ((java_lang_ref_ReferenceQueue*) me)->fields.java_lang_ref_ReferenceQueue.tail_ = 0;
    ((java_lang_ref_ReferenceQueue*) me)->fields.java_lang_ref_ReferenceQueue.empty_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ref_ReferenceQueue]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ref_ReferenceQueue()
{    XMLVM_CLASS_INIT(java_lang_ref_ReferenceQueue)
java_lang_ref_ReferenceQueue* me = (java_lang_ref_ReferenceQueue*) XMLVM_MALLOC(sizeof(java_lang_ref_ReferenceQueue));
    me->tib = &__TIB_java_lang_ref_ReferenceQueue;
    __INIT_INSTANCE_MEMBERS_java_lang_ref_ReferenceQueue(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ref_ReferenceQueue]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_ReferenceQueue()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_ref_ReferenceQueue();
    java_lang_ref_ReferenceQueue___INIT___(me);
    return me;
}

JAVA_INT java_lang_ref_ReferenceQueue_GET_DEFAULT_QUEUE_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_ref_ReferenceQueue)
    return _STATIC_java_lang_ref_ReferenceQueue_DEFAULT_QUEUE_SIZE;
}

void java_lang_ref_ReferenceQueue_PUT_DEFAULT_QUEUE_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_ref_ReferenceQueue)
_STATIC_java_lang_ref_ReferenceQueue_DEFAULT_QUEUE_SIZE = v;
}

void java_lang_ref_ReferenceQueue___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue___INIT___]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 43)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 44)
    _r0.i = 128;
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_ref_ReferenceQueue_newArray___int(_r2.o, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ref_ReferenceQueue*) _r2.o)->fields.java_lang_ref_ReferenceQueue.references_ = _r0.o;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 45)
    XMLVM_CHECK_NPE(2)
    ((java_lang_ref_ReferenceQueue*) _r2.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 46)
    XMLVM_CHECK_NPE(2)
    ((java_lang_ref_ReferenceQueue*) _r2.o)->fields.java_lang_ref_ReferenceQueue.tail_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 47)
    _r0.i = 1;
    XMLVM_CHECK_NPE(2)
    ((java_lang_ref_ReferenceQueue*) _r2.o)->fields.java_lang_ref_ReferenceQueue.empty_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 48)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ref_ReferenceQueue_newArray___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue_newArray___int]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "newArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 52)
    XMLVM_CLASS_INIT(java_lang_ref_Reference)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_ref_Reference, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ref_ReferenceQueue_poll__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue_poll__]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "poll", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 65)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w1725aaab7b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 66)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.empty_;
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w1725aaab7b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab7b1b4, sizeof(XMLVM_JMP_BUF)); goto label8; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 67)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    _r0.o = JAVA_NULL;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1b4)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1b4,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1b4)
    label7:;
    XMLVM_TRY_BEGIN(w1725aaab7b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 78)
    XMLVM_MEMCPY(curThread_w1725aaab7b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab7b1b6, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1b6)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1b6,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1b6)
    label8:;
    XMLVM_TRY_BEGIN(w1725aaab7b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 69)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.references_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 70)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference_dequeue__(_r0.o);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 71)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r1.i != _r2.i) { XMLVM_MEMCPY(curThread_w1725aaab7b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab7b1b8, sizeof(XMLVM_JMP_BUF)); goto label31; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 72)
    _r1.i = 0;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1b8)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1b8,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1b8)
    label31:;
    XMLVM_TRY_BEGIN(w1725aaab7b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 74)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    if (_r1.i != _r2.i) { XMLVM_MEMCPY(curThread_w1725aaab7b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab7b1c10, sizeof(XMLVM_JMP_BUF)); goto label40; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 75)
    _r1.i = 1;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.empty_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1c10)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1c10,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1c10)
    label40:;
    XMLVM_TRY_BEGIN(w1725aaab7b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    { XMLVM_MEMCPY(curThread_w1725aaab7b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab7b1c12, sizeof(XMLVM_JMP_BUF)); goto label7; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1c12)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1c12,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1c12)
    label42:;
    XMLVM_TRY_BEGIN(w1725aaab7b1c14)
    // Begin try
    java_lang_Thread* curThread_w1725aaab7b1c14aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1725aaab7b1c14aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab7b1c14)
        XMLVM_CATCH_SPECIFIC(w1725aaab7b1c14,java_lang_Object,42)
    XMLVM_CATCH_END(w1725aaab7b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab7b1c14)
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ref_ReferenceQueue_remove__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue_remove__]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 91)
    _r0.l = 0;
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_ref_ReferenceQueue_remove___long(_r2.o, _r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ref_ReferenceQueue_remove___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue_remove___long]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "remove", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r3.o = me;
    _r4.l = n1;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 112)
    _r0.l = 0;
    _r0.i = _r4.l > _r0.l ? 1 : (_r4.l == _r0.l ? 0 : -1);
    if (_r0.i >= 0) goto label12;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 113)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 117)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w1725aaab9b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 118)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.empty_;
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w1725aaab9b1c14->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c14, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 119)
    XMLVM_CHECK_NPE(3)
    java_lang_Object_wait___long(_r3.o, _r4.l);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 120)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.empty_;
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w1725aaab9b1c14->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c14, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 121)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    _r0.o = JAVA_NULL;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c14)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c14,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c14)
    label26:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 135)
    XMLVM_MEMCPY(curThread_w1725aaab9b1c16->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c16, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c16)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c16,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c16)
    label27:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 124)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.references_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 125)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference_dequeue__(_r0.o);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 126)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r1.i != _r2.i) { XMLVM_MEMCPY(curThread_w1725aaab9b1c18->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c18, sizeof(XMLVM_JMP_BUF)); goto label50; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 127)
    _r1.i = 0;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c18)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c18,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c18)
    label50:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c20)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 129)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.head_;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    if (_r1.i != _r2.i) { XMLVM_MEMCPY(curThread_w1725aaab9b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c20, sizeof(XMLVM_JMP_BUF)); goto label64; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 130)
    _r1.i = 1;
    XMLVM_CHECK_NPE(3)
    ((java_lang_ref_ReferenceQueue*) _r3.o)->fields.java_lang_ref_ReferenceQueue.empty_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c20)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c20,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c20)
    label59:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c22)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    { XMLVM_MEMCPY(curThread_w1725aaab9b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaab9b1c22, sizeof(XMLVM_JMP_BUF)); goto label26; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c22)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c22,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c22)
    label61:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c24)
    // Begin try
    java_lang_Thread* curThread_w1725aaab9b1c24aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1725aaab9b1c24aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c24)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c24,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c24)
    XMLVM_THROW_CUSTOM(_r0.o)
    label64:;
    XMLVM_TRY_BEGIN(w1725aaab9b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 132)
    XMLVM_CHECK_NPE(3)
    java_lang_Object_notifyAll__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaab9b1c27)
        XMLVM_CATCH_SPECIFIC(w1725aaab9b1c27,java_lang_Object,61)
    XMLVM_CATCH_END(w1725aaab9b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaab9b1c27)
    goto label59;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ref_ReferenceQueue_enqueue___java_lang_ref_Reference(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_ReferenceQueue_enqueue___java_lang_ref_Reference]
    XMLVM_ENTER_METHOD("java.lang.ref.ReferenceQueue", "enqueue", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 147)
    java_lang_Object_acquireLockRecursive__(_r6.o);
    XMLVM_TRY_BEGIN(w1725aaac10b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 148)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.empty_;
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1725aaac10b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaac10b1b5, sizeof(XMLVM_JMP_BUF)); goto label68; };
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.head_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    if (_r0.i != _r1.i) { XMLVM_MEMCPY(curThread_w1725aaac10b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaac10b1b5, sizeof(XMLVM_JMP_BUF)); goto label68; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 150)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.d = (JAVA_DOUBLE) _r0.i;
    _r2.d = 1.1;
    _r0.d = _r0.d * _r2.d;
    _r0.i = (JAVA_INT) _r0.d;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 151)
    XMLVM_CHECK_NPE(6)
    _r0.o = java_lang_ref_ReferenceQueue_newArray___int(_r6.o, _r0.i);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 152)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.head_;
    _r3.i = 0;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.head_;
    _r4.i = _r4.i - _r5.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 153)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    if (_r1.i <= 0) { XMLVM_MEMCPY(curThread_w1725aaac10b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaac10b1b5, sizeof(XMLVM_JMP_BUF)); goto label58; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 154)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r2.i = 0;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.head_;
    _r3.i = _r3.i - _r4.i;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaac10b1b5)
        XMLVM_CATCH_SPECIFIC(w1725aaac10b1b5,java_lang_Object,97)
    XMLVM_CATCH_END(w1725aaac10b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaac10b1b5)
    label58:;
    XMLVM_TRY_BEGIN(w1725aaac10b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 156)
    _r1.i = 0;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.head_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 157)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 158)
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_ = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaac10b1b7)
        XMLVM_CATCH_SPECIFIC(w1725aaac10b1b7,java_lang_Object,97)
    XMLVM_CATCH_END(w1725aaac10b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaac10b1b7)
    label68:;
    XMLVM_TRY_BEGIN(w1725aaac10b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 160)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.o;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 161)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_;
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.references_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i != _r1.i) { XMLVM_MEMCPY(curThread_w1725aaac10b1b9->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaac10b1b9, sizeof(XMLVM_JMP_BUF)); goto label88; };
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 162)
    _r0.i = 0;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.tail_ = _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaac10b1b9)
        XMLVM_CATCH_SPECIFIC(w1725aaac10b1b9,java_lang_Object,97)
    XMLVM_CATCH_END(w1725aaac10b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaac10b1b9)
    label88:;
    XMLVM_TRY_BEGIN(w1725aaac10b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 164)
    _r0.i = 0;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ref_ReferenceQueue*) _r6.o)->fields.java_lang_ref_ReferenceQueue.empty_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 165)
    XMLVM_CHECK_NPE(6)
    java_lang_Object_notifyAll__(_r6.o);
    java_lang_Object_releaseLockRecursive__(_r6.o);
    XMLVM_SOURCE_POSITION("ReferenceQueue.java", 167)
    _r0.i = 1;
    XMLVM_MEMCPY(curThread_w1725aaac10b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1725aaac10b1c11, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaac10b1c11)
        XMLVM_CATCH_SPECIFIC(w1725aaac10b1c11,java_lang_Object,97)
    XMLVM_CATCH_END(w1725aaac10b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaac10b1c11)
    label97:;
    XMLVM_TRY_BEGIN(w1725aaac10b1c13)
    // Begin try
    java_lang_Thread* curThread_w1725aaac10b1c13aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1725aaac10b1c13aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1725aaac10b1c13)
        XMLVM_CATCH_SPECIFIC(w1725aaac10b1c13,java_lang_Object,97)
    XMLVM_CATCH_END(w1725aaac10b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w1725aaac10b1c13)
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

