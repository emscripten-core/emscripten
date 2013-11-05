#include "xmlvm.h"
#include "java_util_Collection.h"
#include "java_util_Set.h"

#include "java_util_Map.h"

__TIB_DEFINITION_java_util_Map __TIB_java_util_Map = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Map, // classInitializer
    "java.util.Map", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Map;
JAVA_OBJECT __CLASS_java_util_Map_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Map_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Map_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Map()
{
    staticInitializerLock(&__TIB_java_util_Map);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Map.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Map.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Map);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Map.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Map.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Map.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Map")
        __INIT_IMPL_java_util_Map();
    }
}

void __INIT_IMPL_java_util_Map()
{
    __TIB_java_util_Map.numInterfaces = 0;
    __TIB_java_util_Map.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Map.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Map.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Map.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Map.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Map = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Map);
    __TIB_java_util_Map.clazz = __CLASS_java_util_Map;
    __TIB_java_util_Map.baseType = JAVA_NULL;
    __CLASS_java_util_Map_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map);
    __CLASS_java_util_Map_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map_1ARRAY);
    __CLASS_java_util_Map_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Map_2ARRAY);

    __TIB_java_util_Map.classInitialized = 1;
}

