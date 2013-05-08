#include "xmlvm.h"
#include "java_lang_CharSequence.h"

#include "java_lang_Appendable.h"

__TIB_DEFINITION_java_lang_Appendable __TIB_java_lang_Appendable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Appendable, // classInitializer
    "java.lang.Appendable", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_Appendable;
JAVA_OBJECT __CLASS_java_lang_Appendable_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Appendable_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Appendable_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_Appendable()
{
    staticInitializerLock(&__TIB_java_lang_Appendable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Appendable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Appendable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Appendable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Appendable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Appendable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Appendable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Appendable")
        __INIT_IMPL_java_lang_Appendable();
    }
}

void __INIT_IMPL_java_lang_Appendable()
{
    __TIB_java_lang_Appendable.numInterfaces = 0;
    __TIB_java_lang_Appendable.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Appendable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_Appendable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Appendable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Appendable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_Appendable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Appendable);
    __TIB_java_lang_Appendable.clazz = __CLASS_java_lang_Appendable;
    __TIB_java_lang_Appendable.baseType = JAVA_NULL;
    __CLASS_java_lang_Appendable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Appendable);
    __CLASS_java_lang_Appendable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Appendable_1ARRAY);
    __CLASS_java_lang_Appendable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Appendable_2ARRAY);

    __TIB_java_lang_Appendable.classInitialized = 1;
}

