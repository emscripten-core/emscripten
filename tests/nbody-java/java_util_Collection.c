#include "xmlvm.h"
#include "java_util_Iterator.h"

#include "java_util_Collection.h"

__TIB_DEFINITION_java_util_Collection __TIB_java_util_Collection = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Collection, // classInitializer
    "java.util.Collection", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;Ljava/lang/Iterable<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Collection;
JAVA_OBJECT __CLASS_java_util_Collection_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Collection_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Collection_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Collection()
{
    staticInitializerLock(&__TIB_java_util_Collection);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Collection.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Collection.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Collection);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Collection.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Collection.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Collection.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Collection")
        __INIT_IMPL_java_util_Collection();
    }
}

void __INIT_IMPL_java_util_Collection()
{
    __TIB_java_util_Collection.numInterfaces = 1;
    //__TIB_java_util_Collection.baseInterfaces[0] = &__INTERFACE_java_lang_Iterable;
    __TIB_java_util_Collection.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Collection.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Collection.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Collection.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Collection.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Collection = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Collection);
    __TIB_java_util_Collection.clazz = __CLASS_java_util_Collection;
    __TIB_java_util_Collection.baseType = JAVA_NULL;
    __CLASS_java_util_Collection_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collection);
    __CLASS_java_util_Collection_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collection_1ARRAY);
    __CLASS_java_util_Collection_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collection_2ARRAY);

    __TIB_java_util_Collection.classInitialized = 1;
}

