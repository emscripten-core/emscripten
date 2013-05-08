#include "xmlvm.h"

#include "java_lang_Runnable.h"

__TIB_DEFINITION_java_lang_Runnable __TIB_java_lang_Runnable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Runnable, // classInitializer
    "java.lang.Runnable", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_Runnable;
JAVA_OBJECT __CLASS_java_lang_Runnable_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Runnable_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Runnable_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_Runnable()
{
    staticInitializerLock(&__TIB_java_lang_Runnable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Runnable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Runnable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Runnable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Runnable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Runnable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Runnable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Runnable")
        __INIT_IMPL_java_lang_Runnable();
    }
}

void __INIT_IMPL_java_lang_Runnable()
{
    __TIB_java_lang_Runnable.numInterfaces = 0;
    __TIB_java_lang_Runnable.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Runnable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_Runnable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Runnable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Runnable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_Runnable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Runnable);
    __TIB_java_lang_Runnable.clazz = __CLASS_java_lang_Runnable;
    __TIB_java_lang_Runnable.baseType = JAVA_NULL;
    __CLASS_java_lang_Runnable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runnable);
    __CLASS_java_lang_Runnable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runnable_1ARRAY);
    __CLASS_java_lang_Runnable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Runnable_2ARRAY);

    __TIB_java_lang_Runnable.classInitialized = 1;
}

