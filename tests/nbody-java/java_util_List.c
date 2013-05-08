#include "xmlvm.h"
#include "java_util_Iterator.h"

#include "java_util_List.h"

__TIB_DEFINITION_java_util_List __TIB_java_util_List = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_List, // classInitializer
    "java.util.List", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;Ljava/util/Collection<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_List;
JAVA_OBJECT __CLASS_java_util_List_1ARRAY;
JAVA_OBJECT __CLASS_java_util_List_2ARRAY;
JAVA_OBJECT __CLASS_java_util_List_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_List()
{
    staticInitializerLock(&__TIB_java_util_List);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_List.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_List.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_List);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_List.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_List.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_List.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.List")
        __INIT_IMPL_java_util_List();
    }
}

void __INIT_IMPL_java_util_List()
{
    __TIB_java_util_List.numInterfaces = 1;
    //__TIB_java_util_List.baseInterfaces[0] = &__INTERFACE_java_util_Collection;
    __TIB_java_util_List.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_List.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_List.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_List.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_List.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_List = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_List);
    __TIB_java_util_List.clazz = __CLASS_java_util_List;
    __TIB_java_util_List.baseType = JAVA_NULL;
    __CLASS_java_util_List_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_List);
    __CLASS_java_util_List_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_List_1ARRAY);
    __CLASS_java_util_List_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_List_2ARRAY);

    __TIB_java_util_List.classInitialized = 1;
}

