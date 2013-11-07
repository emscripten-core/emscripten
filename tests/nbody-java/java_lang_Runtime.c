#include "xmlvm.h"
#include "java_io_File.h"
#include "java_io_OutputStream.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_Thread.h"

#include "java_lang_Runtime.h"

#define XMLVM_CURRENT_CLASS_NAME Runtime
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Runtime

__TIB_DEFINITION_java_lang_Runtime __TIB_java_lang_Runtime = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Runtime, // classInitializer
    "java.lang.Runtime", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Runtime), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Runtime;
JAVA_OBJECT __CLASS_java_lang_Runtime_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Runtime_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Runtime_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_lang_Runtime_runtime;

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

void __INIT_java_lang_Runtime()
{
    staticInitializerLock(&__TIB_java_lang_Runtime);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Runtime.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Runtime.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Runtime);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Runtime.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Runtime.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Runtime.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Runtime")
        __INIT_IMPL_java_lang_Runtime();
    }
}

void __INIT_IMPL_java_lang_Runtime()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Runtime.newInstanceFunc = __NEW_INSTANCE_java_lang_Runtime;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Runtime.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    xmlvm_init_native_java_lang_Runtime();
    // Initialize interface information
    __TIB_java_lang_Runtime.numImplementedInterfaces = 0;
    __TIB_java_lang_Runtime.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_Runtime_runtime = (java_lang_Runtime*) JAVA_NULL;

    __TIB_java_lang_Runtime.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Runtime.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Runtime.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Runtime.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Runtime.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Runtime.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Runtime.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Runtime.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Runtime = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Runtime);
    __TIB_java_lang_Runtime.clazz = __CLASS_java_lang_Runtime;
    __TIB_java_lang_Runtime.baseType = JAVA_NULL;
    __CLASS_java_lang_Runtime_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runtime);
    __CLASS_java_lang_Runtime_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runtime_1ARRAY);
    __CLASS_java_lang_Runtime_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runtime_2ARRAY);
    java_lang_Runtime___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Runtime]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Runtime.classInitialized = 1;
}

void __DELETE_java_lang_Runtime(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Runtime]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Runtime(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Runtime]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Runtime()
{    XMLVM_CLASS_INIT(java_lang_Runtime)
java_lang_Runtime* me = (java_lang_Runtime*) XMLVM_MALLOC(sizeof(java_lang_Runtime));
    me->tib = &__TIB_java_lang_Runtime;
    __INIT_INSTANCE_MEMBERS_java_lang_Runtime(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Runtime]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Runtime()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_lang_Runtime_GET_runtime()
{
    XMLVM_CLASS_INIT(java_lang_Runtime)
    return _STATIC_java_lang_Runtime_runtime;
}

void java_lang_Runtime_PUT_runtime(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Runtime)
_STATIC_java_lang_Runtime_runtime = v;
}

void java_lang_Runtime___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Runtime.java", 45)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Runtime.java", 47)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 67)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Runtime.java", 92)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY_java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY_java_io_File]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r4.o = n3;
    XMLVM_SOURCE_POSITION("Runtime.java", 120)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 140)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Runtime.java", 163)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY_java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY_java_io_File]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exec", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r4.o = n3;
    XMLVM_SOURCE_POSITION("Runtime.java", 189)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_exit___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_exit___int]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "exit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 207)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG java_lang_Runtime_freeMemory__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void java_lang_Runtime_gc__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Runtime_getRuntime__()
{
    XMLVM_CLASS_INIT(java_lang_Runtime)
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_getRuntime__]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "getRuntime", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Runtime.java", 231)
    _r0.o = java_lang_Runtime_GET_runtime();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_load___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_load___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "load", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 250)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_loadLibrary___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_loadLibrary___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "loadLibrary", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 268)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_runFinalization__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_runFinalization__]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "runFinalization", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Runtime.java", 276)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_runFinalizersOnExit___boolean(JAVA_BOOLEAN n1)
{
    XMLVM_CLASS_INIT(java_lang_Runtime)
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_runFinalizersOnExit___boolean]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "runFinalizersOnExit", "?")
    XMLVMElem _r0;
    _r0.i = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 292)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG java_lang_Runtime_totalMemory__(JAVA_OBJECT me)]

void java_lang_Runtime_traceInstructions___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_traceInstructions___boolean]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "traceInstructions", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 311)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_traceMethodCalls___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_traceMethodCalls___boolean]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "traceMethodCalls", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 322)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_getLocalizedInputStream___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_getLocalizedInputStream___java_io_InputStream]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "getLocalizedInputStream", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 338)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Runtime_getLocalizedOutputStream___java_io_OutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_getLocalizedOutputStream___java_io_OutputStream]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "getLocalizedOutputStream", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 354)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_addShutdownHook___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_addShutdownHook___java_lang_Thread]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "addShutdownHook", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 396)
    if (_r3.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("Runtime.java", 397)
    _r0.o = __NEW_java_lang_NullPointerException();
    // "null is not allowed here"
    _r1.o = xmlvm_create_java_string_from_pool(779);
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label10:;
    XMLVM_SOURCE_POSITION("Runtime.java", 399)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Runtime_removeShutdownHook___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_removeShutdownHook___java_lang_Thread]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "removeShutdownHook", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 417)
    if (_r3.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("Runtime.java", 418)
    _r0.o = __NEW_java_lang_NullPointerException();
    // "null is not allowed here"
    _r1.o = xmlvm_create_java_string_from_pool(779);
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label10:;
    XMLVM_SOURCE_POSITION("Runtime.java", 420)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime_halt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_halt___int]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "halt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Runtime.java", 439)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Runtime_availableProcessors__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_availableProcessors__]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "availableProcessors", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Runtime.java", 448)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Runtime_maxMemory__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime_maxMemory__]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "maxMemory", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Runtime.java", 459)
    _r0.l = 0;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

void java_lang_Runtime___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Runtime___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Runtime", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Runtime.java", 40)
    _r0.o = __NEW_java_lang_Runtime();
    XMLVM_CHECK_NPE(0)
    java_lang_Runtime___INIT___(_r0.o);
    java_lang_Runtime_PUT_runtime( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

