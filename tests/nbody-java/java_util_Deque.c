#include "xmlvm.h"
#include "java_util_Iterator.h"

#include "java_util_Deque.h"

__TIB_DEFINITION_java_util_Deque __TIB_java_util_Deque = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Deque, // classInitializer
    "java.util.Deque", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;Ljava/util/Queue<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_util_Deque;
JAVA_OBJECT __CLASS_java_util_Deque_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Deque_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Deque_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_util_Deque()
{
    staticInitializerLock(&__TIB_java_util_Deque);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Deque.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Deque.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Deque);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Deque.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Deque.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Deque.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Deque")
        __INIT_IMPL_java_util_Deque();
    }
}

void __INIT_IMPL_java_util_Deque()
{
    __TIB_java_util_Deque.numInterfaces = 1;
    //__TIB_java_util_Deque.baseInterfaces[0] = &__INTERFACE_java_util_Queue;
    __TIB_java_util_Deque.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Deque.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_util_Deque.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Deque.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Deque.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_util_Deque = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Deque);
    __TIB_java_util_Deque.clazz = __CLASS_java_util_Deque;
    __TIB_java_util_Deque.baseType = JAVA_NULL;
    __CLASS_java_util_Deque_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Deque);
    __CLASS_java_util_Deque_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Deque_1ARRAY);
    __CLASS_java_util_Deque_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Deque_2ARRAY);

    __TIB_java_util_Deque.classInitialized = 1;
}

