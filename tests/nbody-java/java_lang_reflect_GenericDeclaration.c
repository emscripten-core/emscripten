#include "xmlvm.h"

#include "java_lang_reflect_GenericDeclaration.h"

__TIB_DEFINITION_java_lang_reflect_GenericDeclaration __TIB_java_lang_reflect_GenericDeclaration = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_GenericDeclaration, // classInitializer
    "java.lang.reflect.GenericDeclaration", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_reflect_GenericDeclaration;
JAVA_OBJECT __CLASS_java_lang_reflect_GenericDeclaration_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_GenericDeclaration_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_GenericDeclaration_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_reflect_GenericDeclaration()
{
    staticInitializerLock(&__TIB_java_lang_reflect_GenericDeclaration);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_GenericDeclaration.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_GenericDeclaration.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_GenericDeclaration);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_GenericDeclaration.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_GenericDeclaration.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_GenericDeclaration.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.GenericDeclaration")
        __INIT_IMPL_java_lang_reflect_GenericDeclaration();
    }
}

void __INIT_IMPL_java_lang_reflect_GenericDeclaration()
{
    __TIB_java_lang_reflect_GenericDeclaration.numInterfaces = 0;
    __TIB_java_lang_reflect_GenericDeclaration.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_GenericDeclaration.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_reflect_GenericDeclaration.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_GenericDeclaration.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_GenericDeclaration.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_reflect_GenericDeclaration = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_GenericDeclaration);
    __TIB_java_lang_reflect_GenericDeclaration.clazz = __CLASS_java_lang_reflect_GenericDeclaration;
    __TIB_java_lang_reflect_GenericDeclaration.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_GenericDeclaration_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_GenericDeclaration);
    __CLASS_java_lang_reflect_GenericDeclaration_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_GenericDeclaration_1ARRAY);
    __CLASS_java_lang_reflect_GenericDeclaration_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_GenericDeclaration_2ARRAY);

    __TIB_java_lang_reflect_GenericDeclaration.classInitialized = 1;
}

