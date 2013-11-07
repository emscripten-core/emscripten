#include "xmlvm.h"

#include "java_util_Map_Entry.h"

__TIB_DEFINITION_java_util_Map_Entry __TIB_java_util_Map_Entry = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Map_Entry, // classInitializer
    "java.util.Map$Entry", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Map_Entry;
JAVA_OBJECT __CLASS_java_util_Map_Entry_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Map_Entry_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Map_Entry_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Map_Entry()
{
    staticInitializerLock(&__TIB_java_util_Map_Entry);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Map_Entry.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Map_Entry.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Map_Entry);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Map_Entry.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Map_Entry.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Map_Entry.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Map$Entry")
        __INIT_IMPL_java_util_Map_Entry();
    }
}

void __INIT_IMPL_java_util_Map_Entry()
{
    __TIB_java_util_Map_Entry.numInterfaces = 0;
    __TIB_java_util_Map_Entry.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Map_Entry.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Map_Entry.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Map_Entry.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Map_Entry.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Map_Entry = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Map_Entry);
    __TIB_java_util_Map_Entry.clazz = __CLASS_java_util_Map_Entry;
    __TIB_java_util_Map_Entry.baseType = JAVA_NULL;
    __CLASS_java_util_Map_Entry_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map_Entry);
    __CLASS_java_util_Map_Entry_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map_Entry_1ARRAY);
    __CLASS_java_util_Map_Entry_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map_Entry_2ARRAY);

    __TIB_java_util_Map_Entry.classInitialized = 1;
}

