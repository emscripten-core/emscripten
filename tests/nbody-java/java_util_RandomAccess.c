#include "xmlvm.h"

#include "java_util_RandomAccess.h"

__TIB_DEFINITION_java_util_RandomAccess __TIB_java_util_RandomAccess = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_RandomAccess, // classInitializer
    "java.util.RandomAccess", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_RandomAccess;
JAVA_OBJECT __CLASS_java_util_RandomAccess_1ARRAY;
JAVA_OBJECT __CLASS_java_util_RandomAccess_2ARRAY;
JAVA_OBJECT __CLASS_java_util_RandomAccess_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_RandomAccess()
{
    staticInitializerLock(&__TIB_java_util_RandomAccess);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_RandomAccess.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_RandomAccess.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_RandomAccess);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_RandomAccess.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_RandomAccess.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_RandomAccess.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.RandomAccess")
        __INIT_IMPL_java_util_RandomAccess();
    }
}

void __INIT_IMPL_java_util_RandomAccess()
{
    __TIB_java_util_RandomAccess.numInterfaces = 0;
    __TIB_java_util_RandomAccess.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_RandomAccess.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_RandomAccess.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_RandomAccess.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_RandomAccess.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_RandomAccess = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_RandomAccess);
    __TIB_java_util_RandomAccess.clazz = __CLASS_java_util_RandomAccess;
    __TIB_java_util_RandomAccess.baseType = JAVA_NULL;
    __CLASS_java_util_RandomAccess_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_RandomAccess);
    __CLASS_java_util_RandomAccess_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_RandomAccess_1ARRAY);
    __CLASS_java_util_RandomAccess_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_RandomAccess_2ARRAY);

    __TIB_java_util_RandomAccess.classInitialized = 1;
}

