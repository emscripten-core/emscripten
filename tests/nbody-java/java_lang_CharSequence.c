#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_lang_CharSequence.h"

__TIB_DEFINITION_java_lang_CharSequence __TIB_java_lang_CharSequence = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_CharSequence, // classInitializer
    "java.lang.CharSequence", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_CharSequence;
JAVA_OBJECT __CLASS_java_lang_CharSequence_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_CharSequence_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_CharSequence_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_CharSequence()
{
    staticInitializerLock(&__TIB_java_lang_CharSequence);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_CharSequence.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_CharSequence.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_CharSequence);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_CharSequence.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_CharSequence.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_CharSequence.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.CharSequence")
        __INIT_IMPL_java_lang_CharSequence();
    }
}

void __INIT_IMPL_java_lang_CharSequence()
{
    __TIB_java_lang_CharSequence.numInterfaces = 0;
    __TIB_java_lang_CharSequence.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_CharSequence.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_CharSequence.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_CharSequence.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_CharSequence.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_CharSequence = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_CharSequence);
    __TIB_java_lang_CharSequence.clazz = __CLASS_java_lang_CharSequence;
    __TIB_java_lang_CharSequence.baseType = JAVA_NULL;
    __CLASS_java_lang_CharSequence_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_CharSequence);
    __CLASS_java_lang_CharSequence_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_CharSequence_1ARRAY);
    __CLASS_java_lang_CharSequence_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_CharSequence_2ARRAY);

    __TIB_java_lang_CharSequence.classInitialized = 1;
}

