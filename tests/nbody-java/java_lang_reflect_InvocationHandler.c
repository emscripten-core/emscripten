#include "xmlvm.h"
#include "java_lang_reflect_Method.h"

#include "java_lang_reflect_InvocationHandler.h"

__TIB_DEFINITION_java_lang_reflect_InvocationHandler __TIB_java_lang_reflect_InvocationHandler = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_InvocationHandler, // classInitializer
    "java.lang.reflect.InvocationHandler", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_reflect_InvocationHandler;
JAVA_OBJECT __CLASS_java_lang_reflect_InvocationHandler_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_InvocationHandler_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_InvocationHandler_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_reflect_InvocationHandler()
{
    staticInitializerLock(&__TIB_java_lang_reflect_InvocationHandler);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_InvocationHandler.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_InvocationHandler.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_InvocationHandler);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_InvocationHandler.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_InvocationHandler.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_InvocationHandler.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.InvocationHandler")
        __INIT_IMPL_java_lang_reflect_InvocationHandler();
    }
}

void __INIT_IMPL_java_lang_reflect_InvocationHandler()
{
    __TIB_java_lang_reflect_InvocationHandler.numInterfaces = 0;
    __TIB_java_lang_reflect_InvocationHandler.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_InvocationHandler.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_reflect_InvocationHandler.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_InvocationHandler.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_InvocationHandler.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_reflect_InvocationHandler = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_InvocationHandler);
    __TIB_java_lang_reflect_InvocationHandler.clazz = __CLASS_java_lang_reflect_InvocationHandler;
    __TIB_java_lang_reflect_InvocationHandler.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_InvocationHandler_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_InvocationHandler);
    __CLASS_java_lang_reflect_InvocationHandler_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_InvocationHandler_1ARRAY);
    __CLASS_java_lang_reflect_InvocationHandler_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_InvocationHandler_2ARRAY);

    __TIB_java_lang_reflect_InvocationHandler.classInitialized = 1;
}

