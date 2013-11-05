#include "xmlvm.h"
#include "java_lang_Class.h"

#include "java_lang_reflect_AnnotatedElement.h"

__TIB_DEFINITION_java_lang_reflect_AnnotatedElement __TIB_java_lang_reflect_AnnotatedElement = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_AnnotatedElement, // classInitializer
    "java.lang.reflect.AnnotatedElement", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_reflect_AnnotatedElement;
JAVA_OBJECT __CLASS_java_lang_reflect_AnnotatedElement_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_AnnotatedElement_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_AnnotatedElement_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_reflect_AnnotatedElement()
{
    staticInitializerLock(&__TIB_java_lang_reflect_AnnotatedElement);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_AnnotatedElement.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_AnnotatedElement.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_AnnotatedElement);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_AnnotatedElement.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_AnnotatedElement.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_AnnotatedElement.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.AnnotatedElement")
        __INIT_IMPL_java_lang_reflect_AnnotatedElement();
    }
}

void __INIT_IMPL_java_lang_reflect_AnnotatedElement()
{
    __TIB_java_lang_reflect_AnnotatedElement.numInterfaces = 0;
    __TIB_java_lang_reflect_AnnotatedElement.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_AnnotatedElement.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_reflect_AnnotatedElement.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_AnnotatedElement.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_AnnotatedElement.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_reflect_AnnotatedElement = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_AnnotatedElement);
    __TIB_java_lang_reflect_AnnotatedElement.clazz = __CLASS_java_lang_reflect_AnnotatedElement;
    __TIB_java_lang_reflect_AnnotatedElement.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_AnnotatedElement_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AnnotatedElement);
    __CLASS_java_lang_reflect_AnnotatedElement_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AnnotatedElement_1ARRAY);
    __CLASS_java_lang_reflect_AnnotatedElement_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AnnotatedElement_2ARRAY);

    __TIB_java_lang_reflect_AnnotatedElement.classInitialized = 1;
}

