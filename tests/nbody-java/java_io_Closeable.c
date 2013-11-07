#include "xmlvm.h"

#include "java_io_Closeable.h"

__TIB_DEFINITION_java_io_Closeable __TIB_java_io_Closeable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_Closeable, // classInitializer
    "java.io.Closeable", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_io_Closeable;
JAVA_OBJECT __CLASS_java_io_Closeable_1ARRAY;
JAVA_OBJECT __CLASS_java_io_Closeable_2ARRAY;
JAVA_OBJECT __CLASS_java_io_Closeable_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_io_Closeable()
{
    staticInitializerLock(&__TIB_java_io_Closeable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_Closeable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_Closeable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_Closeable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_Closeable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_Closeable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_Closeable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.Closeable")
        __INIT_IMPL_java_io_Closeable();
    }
}

void __INIT_IMPL_java_io_Closeable()
{
    __TIB_java_io_Closeable.numInterfaces = 0;
    __TIB_java_io_Closeable.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_Closeable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_io_Closeable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_Closeable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_Closeable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_io_Closeable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_Closeable);
    __TIB_java_io_Closeable.clazz = __CLASS_java_io_Closeable;
    __TIB_java_io_Closeable.baseType = JAVA_NULL;
    __CLASS_java_io_Closeable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Closeable);
    __CLASS_java_io_Closeable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Closeable_1ARRAY);
    __CLASS_java_io_Closeable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Closeable_2ARRAY);

    __TIB_java_io_Closeable.classInitialized = 1;
}

