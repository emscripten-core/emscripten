#include "xmlvm.h"

#include "java_io_Serializable.h"

__TIB_DEFINITION_java_io_Serializable __TIB_java_io_Serializable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_Serializable, // classInitializer
    "java.io.Serializable", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_io_Serializable;
JAVA_OBJECT __CLASS_java_io_Serializable_1ARRAY;
JAVA_OBJECT __CLASS_java_io_Serializable_2ARRAY;
JAVA_OBJECT __CLASS_java_io_Serializable_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_io_Serializable()
{
    staticInitializerLock(&__TIB_java_io_Serializable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_Serializable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_Serializable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_Serializable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_Serializable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_Serializable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_Serializable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.Serializable")
        __INIT_IMPL_java_io_Serializable();
    }
}

void __INIT_IMPL_java_io_Serializable()
{
    __TIB_java_io_Serializable.numInterfaces = 0;
    __TIB_java_io_Serializable.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_Serializable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_io_Serializable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_Serializable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_Serializable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_io_Serializable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_Serializable);
    __TIB_java_io_Serializable.clazz = __CLASS_java_io_Serializable;
    __TIB_java_io_Serializable.baseType = JAVA_NULL;
    __CLASS_java_io_Serializable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Serializable);
    __CLASS_java_io_Serializable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Serializable_1ARRAY);
    __CLASS_java_io_Serializable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_Serializable_2ARRAY);

    __TIB_java_io_Serializable.classInitialized = 1;
}

