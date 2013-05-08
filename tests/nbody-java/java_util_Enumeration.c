#include "xmlvm.h"

#include "java_util_Enumeration.h"

__TIB_DEFINITION_java_util_Enumeration __TIB_java_util_Enumeration = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Enumeration, // classInitializer
    "java.util.Enumeration", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Enumeration;
JAVA_OBJECT __CLASS_java_util_Enumeration_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Enumeration_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Enumeration_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Enumeration()
{
    staticInitializerLock(&__TIB_java_util_Enumeration);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Enumeration.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Enumeration.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Enumeration);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Enumeration.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Enumeration.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Enumeration.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Enumeration")
        __INIT_IMPL_java_util_Enumeration();
    }
}

void __INIT_IMPL_java_util_Enumeration()
{
    __TIB_java_util_Enumeration.numInterfaces = 0;
    __TIB_java_util_Enumeration.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Enumeration.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Enumeration.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Enumeration.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Enumeration.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Enumeration = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Enumeration);
    __TIB_java_util_Enumeration.clazz = __CLASS_java_util_Enumeration;
    __TIB_java_util_Enumeration.baseType = JAVA_NULL;
    __CLASS_java_util_Enumeration_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Enumeration);
    __CLASS_java_util_Enumeration_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Enumeration_1ARRAY);
    __CLASS_java_util_Enumeration_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Enumeration_2ARRAY);

    __TIB_java_util_Enumeration.classInitialized = 1;
}

