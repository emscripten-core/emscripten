#include "xmlvm.h"

#include "java_util_Queue.h"

__TIB_DEFINITION_java_util_Queue __TIB_java_util_Queue = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Queue, // classInitializer
    "java.util.Queue", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;Ljava/util/Collection<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Queue;
JAVA_OBJECT __CLASS_java_util_Queue_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Queue_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Queue_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Queue()
{
    staticInitializerLock(&__TIB_java_util_Queue);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Queue.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Queue.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Queue);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Queue.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Queue.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Queue.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Queue")
        __INIT_IMPL_java_util_Queue();
    }
}

void __INIT_IMPL_java_util_Queue()
{
    __TIB_java_util_Queue.numInterfaces = 1;
    //__TIB_java_util_Queue.baseInterfaces[0] = &__INTERFACE_java_util_Collection;
    __TIB_java_util_Queue.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Queue.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Queue.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Queue.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Queue.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Queue = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Queue);
    __TIB_java_util_Queue.clazz = __CLASS_java_util_Queue;
    __TIB_java_util_Queue.baseType = JAVA_NULL;
    __CLASS_java_util_Queue_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Queue);
    __CLASS_java_util_Queue_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Queue_1ARRAY);
    __CLASS_java_util_Queue_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Queue_2ARRAY);

    __TIB_java_util_Queue.classInitialized = 1;
}

