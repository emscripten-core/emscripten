#include "xmlvm.h"

#include "java_lang_Comparable.h"

__TIB_DEFINITION_java_lang_Comparable __TIB_java_lang_Comparable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Comparable, // classInitializer
    "java.lang.Comparable", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_Comparable;
JAVA_OBJECT __CLASS_java_lang_Comparable_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Comparable_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Comparable_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_Comparable()
{
    staticInitializerLock(&__TIB_java_lang_Comparable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Comparable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Comparable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Comparable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Comparable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Comparable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Comparable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Comparable")
        __INIT_IMPL_java_lang_Comparable();
    }
}

void __INIT_IMPL_java_lang_Comparable()
{
    __TIB_java_lang_Comparable.numInterfaces = 0;
    __TIB_java_lang_Comparable.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Comparable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_Comparable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Comparable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Comparable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_Comparable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Comparable);
    __TIB_java_lang_Comparable.clazz = __CLASS_java_lang_Comparable;
    __TIB_java_lang_Comparable.baseType = JAVA_NULL;
    __CLASS_java_lang_Comparable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Comparable);
    __CLASS_java_lang_Comparable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Comparable_1ARRAY);
    __CLASS_java_lang_Comparable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Comparable_2ARRAY);

    __TIB_java_lang_Comparable.classInitialized = 1;
}

