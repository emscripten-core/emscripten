#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_Throwable.h"
#include "java_util_WeakHashMap.h"

#include "java_nio_charset_CoderResult.h"

#define XMLVM_CURRENT_CLASS_NAME CoderResult
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_charset_CoderResult

__TIB_DEFINITION_java_nio_charset_CoderResult __TIB_java_nio_charset_CoderResult = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_charset_CoderResult, // classInitializer
    "java.nio.charset.CoderResult", // className
    "java.nio.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_nio_charset_CoderResult), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_charset_CoderResult;
JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_nio_charset_CoderResult_TYPE_UNDERFLOW;
static JAVA_INT _STATIC_java_nio_charset_CoderResult_TYPE_OVERFLOW;
static JAVA_INT _STATIC_java_nio_charset_CoderResult_TYPE_MALFORMED_INPUT;
static JAVA_INT _STATIC_java_nio_charset_CoderResult_TYPE_UNMAPPABLE_CHAR;
static JAVA_OBJECT _STATIC_java_nio_charset_CoderResult_UNDERFLOW;
static JAVA_OBJECT _STATIC_java_nio_charset_CoderResult_OVERFLOW;
static JAVA_OBJECT _STATIC_java_nio_charset_CoderResult__malformedErrors;
static JAVA_OBJECT _STATIC_java_nio_charset_CoderResult__unmappableErrors;

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

void __INIT_java_nio_charset_CoderResult()
{
    staticInitializerLock(&__TIB_java_nio_charset_CoderResult);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_charset_CoderResult.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_charset_CoderResult.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_charset_CoderResult);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_charset_CoderResult.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_charset_CoderResult.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_charset_CoderResult.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.charset.CoderResult")
        __INIT_IMPL_java_nio_charset_CoderResult();
    }
}

void __INIT_IMPL_java_nio_charset_CoderResult()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_nio_charset_CoderResult.newInstanceFunc = __NEW_INSTANCE_java_nio_charset_CoderResult;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_charset_CoderResult.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_charset_CoderResult.vtable[5] = (VTABLE_PTR) &java_nio_charset_CoderResult_toString__;
    // Initialize interface information
    __TIB_java_nio_charset_CoderResult.numImplementedInterfaces = 0;
    __TIB_java_nio_charset_CoderResult.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_nio_charset_CoderResult_TYPE_UNDERFLOW = 1;
    _STATIC_java_nio_charset_CoderResult_TYPE_OVERFLOW = 2;
    _STATIC_java_nio_charset_CoderResult_TYPE_MALFORMED_INPUT = 3;
    _STATIC_java_nio_charset_CoderResult_TYPE_UNMAPPABLE_CHAR = 4;
    _STATIC_java_nio_charset_CoderResult_UNDERFLOW = (java_nio_charset_CoderResult*) JAVA_NULL;
    _STATIC_java_nio_charset_CoderResult_OVERFLOW = (java_nio_charset_CoderResult*) JAVA_NULL;
    _STATIC_java_nio_charset_CoderResult__malformedErrors = (java_util_WeakHashMap*) JAVA_NULL;
    _STATIC_java_nio_charset_CoderResult__unmappableErrors = (java_util_WeakHashMap*) JAVA_NULL;

    __TIB_java_nio_charset_CoderResult.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_charset_CoderResult.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_charset_CoderResult.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_charset_CoderResult.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_charset_CoderResult.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_charset_CoderResult.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_charset_CoderResult.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_charset_CoderResult.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_charset_CoderResult = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_charset_CoderResult);
    __TIB_java_nio_charset_CoderResult.clazz = __CLASS_java_nio_charset_CoderResult;
    __TIB_java_nio_charset_CoderResult.baseType = JAVA_NULL;
    __CLASS_java_nio_charset_CoderResult_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CoderResult);
    __CLASS_java_nio_charset_CoderResult_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CoderResult_1ARRAY);
    __CLASS_java_nio_charset_CoderResult_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CoderResult_2ARRAY);
    java_nio_charset_CoderResult___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_charset_CoderResult]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_charset_CoderResult.classInitialized = 1;
}

void __DELETE_java_nio_charset_CoderResult(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_charset_CoderResult]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_charset_CoderResult(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_charset_CoderResult*) me)->fields.java_nio_charset_CoderResult.type_ = 0;
    ((java_nio_charset_CoderResult*) me)->fields.java_nio_charset_CoderResult.length_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_charset_CoderResult]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_charset_CoderResult()
{    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
java_nio_charset_CoderResult* me = (java_nio_charset_CoderResult*) XMLVM_MALLOC(sizeof(java_nio_charset_CoderResult));
    me->tib = &__TIB_java_nio_charset_CoderResult;
    __INIT_INSTANCE_MEMBERS_java_nio_charset_CoderResult(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_charset_CoderResult]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CoderResult()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_INT java_nio_charset_CoderResult_GET_TYPE_UNDERFLOW()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_TYPE_UNDERFLOW;
}

void java_nio_charset_CoderResult_PUT_TYPE_UNDERFLOW(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_TYPE_UNDERFLOW = v;
}

JAVA_INT java_nio_charset_CoderResult_GET_TYPE_OVERFLOW()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_TYPE_OVERFLOW;
}

void java_nio_charset_CoderResult_PUT_TYPE_OVERFLOW(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_TYPE_OVERFLOW = v;
}

JAVA_INT java_nio_charset_CoderResult_GET_TYPE_MALFORMED_INPUT()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_TYPE_MALFORMED_INPUT;
}

void java_nio_charset_CoderResult_PUT_TYPE_MALFORMED_INPUT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_TYPE_MALFORMED_INPUT = v;
}

JAVA_INT java_nio_charset_CoderResult_GET_TYPE_UNMAPPABLE_CHAR()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_TYPE_UNMAPPABLE_CHAR;
}

void java_nio_charset_CoderResult_PUT_TYPE_UNMAPPABLE_CHAR(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_TYPE_UNMAPPABLE_CHAR = v;
}

JAVA_OBJECT java_nio_charset_CoderResult_GET_UNDERFLOW()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_UNDERFLOW;
}

void java_nio_charset_CoderResult_PUT_UNDERFLOW(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_UNDERFLOW = v;
}

JAVA_OBJECT java_nio_charset_CoderResult_GET_OVERFLOW()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult_OVERFLOW;
}

void java_nio_charset_CoderResult_PUT_OVERFLOW(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult_OVERFLOW = v;
}

JAVA_OBJECT java_nio_charset_CoderResult_GET__malformedErrors()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult__malformedErrors;
}

void java_nio_charset_CoderResult_PUT__malformedErrors(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult__malformedErrors = v;
}

JAVA_OBJECT java_nio_charset_CoderResult_GET__unmappableErrors()
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    return _STATIC_java_nio_charset_CoderResult__unmappableErrors;
}

void java_nio_charset_CoderResult_PUT__unmappableErrors(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
_STATIC_java_nio_charset_CoderResult__unmappableErrors = v;
}

void java_nio_charset_CoderResult___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult___CLINIT___]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("CoderResult.java", 62)
    _r0.o = __NEW_java_nio_charset_CoderResult();
    _r1.i = 1;
    XMLVM_CHECK_NPE(0)
    java_nio_charset_CoderResult___INIT____int_int(_r0.o, _r1.i, _r2.i);
    java_nio_charset_CoderResult_PUT_UNDERFLOW( _r0.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 69)
    _r0.o = __NEW_java_nio_charset_CoderResult();
    _r1.i = 2;
    XMLVM_CHECK_NPE(0)
    java_nio_charset_CoderResult___INIT____int_int(_r0.o, _r1.i, _r2.i);
    java_nio_charset_CoderResult_PUT_OVERFLOW( _r0.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 75)
    _r0.o = __NEW_java_util_WeakHashMap();
    XMLVM_CHECK_NPE(0)
    java_util_WeakHashMap___INIT___(_r0.o);
    java_nio_charset_CoderResult_PUT__malformedErrors( _r0.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 81)
    _r0.o = __NEW_java_util_WeakHashMap();
    XMLVM_CHECK_NPE(0)
    java_util_WeakHashMap___INIT___(_r0.o);
    java_nio_charset_CoderResult_PUT__unmappableErrors( _r0.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 44)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CoderResult___INIT____int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult___INIT____int_int]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("CoderResult.java", 98)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 99)
    XMLVM_CHECK_NPE(0)
    ((java_nio_charset_CoderResult*) _r0.o)->fields.java_nio_charset_CoderResult.type_ = _r1.i;
    XMLVM_SOURCE_POSITION("CoderResult.java", 100)
    XMLVM_CHECK_NPE(0)
    ((java_nio_charset_CoderResult*) _r0.o)->fields.java_nio_charset_CoderResult.length_ = _r2.i;
    XMLVM_SOURCE_POSITION("CoderResult.java", 101)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CoderResult_malformedForLength___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_malformedForLength___int]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "malformedForLength", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("CoderResult.java", 115)
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    _r1.o = __CLASS_java_nio_charset_CoderResult;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    if (_r5.i <= 0) goto label42;
    XMLVM_TRY_BEGIN(w3589aaac12b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 116)
    _r2.o = java_lang_Integer_valueOf___int(_r5.i);
    XMLVM_SOURCE_POSITION("CoderResult.java", 117)
    _r3.o = java_nio_charset_CoderResult_GET__malformedErrors();
    java_lang_Object_acquireLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1b6)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1b6,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac12b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1b6)
    XMLVM_TRY_BEGIN(w3589aaac12b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 118)
    _r0.o = java_nio_charset_CoderResult_GET__malformedErrors();
    //java_util_WeakHashMap_get___java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_WeakHashMap*) _r0.o)->tib->vtable[10])(_r0.o, _r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("CoderResult.java", 119)
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w3589aaac12b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3589aaac12b1b7, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("CoderResult.java", 120)
    _r0.o = __NEW_java_nio_charset_CoderResult();
    _r4.i = 3;
    XMLVM_CHECK_NPE(0)
    java_nio_charset_CoderResult___INIT____int_int(_r0.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("CoderResult.java", 121)
    _r4.o = java_nio_charset_CoderResult_GET__malformedErrors();
    //java_util_WeakHashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(4)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_WeakHashMap*) _r4.o)->tib->vtable[14])(_r4.o, _r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1b7)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1b7,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac12b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1b7)
    label33:;
    XMLVM_TRY_BEGIN(w3589aaac12b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 123)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1b9)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1b9,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac12b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1b9)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    java_lang_Thread* curThread_w3589aaac12b1c13 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3589aaac12b1c13->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3589aaac12b1c14)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1c14)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1c14,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac12b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1c14)
    XMLVM_TRY_BEGIN(w3589aaac12b1c15)
    // Begin try
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1c15)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1c15,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac12b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1c15)
    label39:;
    java_lang_Thread* curThread_w3589aaac12b1c17 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3589aaac12b1c17->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label42:;
    XMLVM_TRY_BEGIN(w3589aaac12b1c21)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 127)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("CoderResult.java", 128)
    // "niochar.08"
    _r2.o = xmlvm_create_java_string_from_pool(180);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac12b1c21)
        XMLVM_CATCH_SPECIFIC(w3589aaac12b1c21,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac12b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac12b1c21)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CoderResult_unmappableForLength___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_unmappableForLength___int]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "unmappableForLength", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("CoderResult.java", 144)
    XMLVM_CLASS_INIT(java_nio_charset_CoderResult)
    _r1.o = __CLASS_java_nio_charset_CoderResult;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    if (_r5.i <= 0) goto label42;
    XMLVM_TRY_BEGIN(w3589aaac13b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 145)
    _r2.o = java_lang_Integer_valueOf___int(_r5.i);
    XMLVM_SOURCE_POSITION("CoderResult.java", 146)
    _r3.o = java_nio_charset_CoderResult_GET__unmappableErrors();
    java_lang_Object_acquireLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1b6)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1b6,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac13b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1b6)
    XMLVM_TRY_BEGIN(w3589aaac13b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 147)
    _r0.o = java_nio_charset_CoderResult_GET__unmappableErrors();
    //java_util_WeakHashMap_get___java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_WeakHashMap*) _r0.o)->tib->vtable[10])(_r0.o, _r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("CoderResult.java", 148)
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w3589aaac13b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3589aaac13b1b7, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("CoderResult.java", 149)
    _r0.o = __NEW_java_nio_charset_CoderResult();
    _r4.i = 4;
    XMLVM_CHECK_NPE(0)
    java_nio_charset_CoderResult___INIT____int_int(_r0.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("CoderResult.java", 150)
    _r4.o = java_nio_charset_CoderResult_GET__unmappableErrors();
    //java_util_WeakHashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(4)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_WeakHashMap*) _r4.o)->tib->vtable[14])(_r4.o, _r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1b7)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1b7,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac13b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1b7)
    label33:;
    XMLVM_TRY_BEGIN(w3589aaac13b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 152)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1b9)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1b9,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac13b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1b9)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    java_lang_Thread* curThread_w3589aaac13b1c13 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3589aaac13b1c13->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3589aaac13b1c14)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1c14)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1c14,java_lang_Object,36)
    XMLVM_CATCH_END(w3589aaac13b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1c14)
    XMLVM_TRY_BEGIN(w3589aaac13b1c15)
    // Begin try
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1c15)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1c15,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac13b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1c15)
    label39:;
    java_lang_Thread* curThread_w3589aaac13b1c17 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3589aaac13b1c17->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label42:;
    XMLVM_TRY_BEGIN(w3589aaac13b1c21)
    // Begin try
    XMLVM_SOURCE_POSITION("CoderResult.java", 156)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("CoderResult.java", 157)
    // "niochar.08"
    _r2.o = xmlvm_create_java_string_from_pool(180);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3589aaac13b1c21)
        XMLVM_CATCH_SPECIFIC(w3589aaac13b1c21,java_lang_Object,39)
    XMLVM_CATCH_END(w3589aaac13b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w3589aaac13b1c21)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CoderResult_isUnderflow__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_isUnderflow__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "isUnderflow", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("CoderResult.java", 166)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    if (_r0.i != _r1.i) goto label7;
    _r0 = _r1;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CoderResult_isError__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_isError__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "isError", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 177)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 3;
    if (_r0.i == _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("CoderResult.java", 178)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 4;
    if (_r0.i == _r1.i) goto label12;
    _r0.i = 0;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    _r0.i = 1;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CoderResult_isMalformed__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_isMalformed__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "isMalformed", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 187)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 3;
    if (_r0.i != _r1.i) goto label7;
    _r0.i = 1;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CoderResult_isOverflow__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_isOverflow__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "isOverflow", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 196)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 2;
    if (_r0.i != _r1.i) goto label7;
    _r0.i = 1;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CoderResult_isUnmappable__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_isUnmappable__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "isUnmappable", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 205)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 4;
    if (_r0.i != _r1.i) goto label7;
    _r0.i = 1;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_charset_CoderResult_length__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_length__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "length", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 217)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 3;
    if (_r0.i == _r1.i) goto label10;
    XMLVM_SOURCE_POSITION("CoderResult.java", 218)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    _r1.i = 4;
    if (_r0.i != _r1.i) goto label13;
    label10:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 219)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.length_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label13:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 223)

    
    // Red class access removed: java.lang.UnsupportedOperationException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("CoderResult.java", 224)
    // "niochar.09"
    _r1.o = xmlvm_create_java_string_from_pool(181);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.UnsupportedOperationException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CoderResult_throwException__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_throwException__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "throwException", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 242)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.type_;
    switch (_r0.i) {
    case 1: goto label11;
    case 2: goto label17;
    case 3: goto label31;
    case 4: goto label23;
    }
    XMLVM_SOURCE_POSITION("CoderResult.java", 252)

    
    // Red class access removed: java.nio.charset.CharacterCodingException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.CharacterCodingException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label11:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 244)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 246)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 248)

    
    // Red class access removed: java.nio.charset.UnmappableCharacterException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.length_;

    
    // Red class access removed: java.nio.charset.UnmappableCharacterException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 250)

    
    // Red class access removed: java.nio.charset.MalformedInputException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_charset_CoderResult*) _r2.o)->fields.java_nio_charset_CoderResult.length_;

    
    // Red class access removed: java.nio.charset.MalformedInputException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label40:;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CoderResult_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CoderResult_toString__]
    XMLVM_ENTER_METHOD("java.nio.charset.CoderResult", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("CoderResult.java", 263)
    XMLVM_SOURCE_POSITION("CoderResult.java", 264)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_nio_charset_CoderResult*) _r3.o)->fields.java_nio_charset_CoderResult.type_;
    switch (_r0.i) {
    case 1: goto label29;
    case 2: goto label32;
    case 3: goto label53;
    case 4: goto label35;
    }
    XMLVM_SOURCE_POSITION("CoderResult.java", 280)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label7:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 283)
    _r1.o = __NEW_java_lang_StringBuilder();
    // "CoderResult["
    _r2.o = xmlvm_create_java_string_from_pool(182);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r0.o);
    // "]"
    _r1.o = xmlvm_create_java_string_from_pool(183);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label29:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 266)
    // "UNDERFLOW error"
    _r0.o = xmlvm_create_java_string_from_pool(184);
    goto label7;
    label32:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 269)
    // "OVERFLOW error"
    _r0.o = xmlvm_create_java_string_from_pool(185);
    goto label7;
    label35:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 272)
    _r0.o = __NEW_java_lang_StringBuilder();
    // "Unmappable-character error with erroneous input length "
    _r1.o = xmlvm_create_java_string_from_pool(186);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 273)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_nio_charset_CoderResult*) _r3.o)->fields.java_nio_charset_CoderResult.length_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label7;
    label53:;
    XMLVM_SOURCE_POSITION("CoderResult.java", 276)
    _r0.o = __NEW_java_lang_StringBuilder();
    // "Malformed-input error with erroneous input length "
    _r1.o = xmlvm_create_java_string_from_pool(187);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("CoderResult.java", 277)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_nio_charset_CoderResult*) _r3.o)->fields.java_nio_charset_CoderResult.length_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label7;
    label72:;
    //XMLVM_END_WRAPPER
}

