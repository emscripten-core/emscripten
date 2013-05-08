#include "xmlvm.h"
#include "java_lang_Thread.h"
#include "java_lang_Throwable.h"

#include "java_lang_Thread_UncaughtExceptionHandler.h"

__TIB_DEFINITION_java_lang_Thread_UncaughtExceptionHandler __TIB_java_lang_Thread_UncaughtExceptionHandler = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Thread_UncaughtExceptionHandler, // classInitializer
    "java.lang.Thread$UncaughtExceptionHandler", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_Thread_UncaughtExceptionHandler;
JAVA_OBJECT __CLASS_java_lang_Thread_UncaughtExceptionHandler_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_UncaughtExceptionHandler_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_UncaughtExceptionHandler_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_Thread_UncaughtExceptionHandler()
{
    staticInitializerLock(&__TIB_java_lang_Thread_UncaughtExceptionHandler);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Thread_UncaughtExceptionHandler.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Thread_UncaughtExceptionHandler.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Thread_UncaughtExceptionHandler);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Thread_UncaughtExceptionHandler.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Thread_UncaughtExceptionHandler.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Thread_UncaughtExceptionHandler.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Thread$UncaughtExceptionHandler")
        __INIT_IMPL_java_lang_Thread_UncaughtExceptionHandler();
    }
}

void __INIT_IMPL_java_lang_Thread_UncaughtExceptionHandler()
{
    __TIB_java_lang_Thread_UncaughtExceptionHandler.numInterfaces = 0;
    __TIB_java_lang_Thread_UncaughtExceptionHandler.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Thread_UncaughtExceptionHandler.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_Thread_UncaughtExceptionHandler.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Thread_UncaughtExceptionHandler.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Thread_UncaughtExceptionHandler.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_Thread_UncaughtExceptionHandler = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Thread_UncaughtExceptionHandler);
    __TIB_java_lang_Thread_UncaughtExceptionHandler.clazz = __CLASS_java_lang_Thread_UncaughtExceptionHandler;
    __TIB_java_lang_Thread_UncaughtExceptionHandler.baseType = JAVA_NULL;
    __CLASS_java_lang_Thread_UncaughtExceptionHandler_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_UncaughtExceptionHandler);
    __CLASS_java_lang_Thread_UncaughtExceptionHandler_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_UncaughtExceptionHandler_1ARRAY);
    __CLASS_java_lang_Thread_UncaughtExceptionHandler_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_UncaughtExceptionHandler_2ARRAY);

    __TIB_java_lang_Thread_UncaughtExceptionHandler.classInitialized = 1;
}

