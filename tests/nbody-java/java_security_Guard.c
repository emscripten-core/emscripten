#include "xmlvm.h"

#include "java_security_Guard.h"

__TIB_DEFINITION_java_security_Guard __TIB_java_security_Guard = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_security_Guard, // classInitializer
    "java.security.Guard", // className
    "java.security", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_security_Guard;
JAVA_OBJECT __CLASS_java_security_Guard_1ARRAY;
JAVA_OBJECT __CLASS_java_security_Guard_2ARRAY;
JAVA_OBJECT __CLASS_java_security_Guard_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_security_Guard()
{
    staticInitializerLock(&__TIB_java_security_Guard);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_security_Guard.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_security_Guard.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_security_Guard);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_security_Guard.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_security_Guard.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_security_Guard.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.security.Guard")
        __INIT_IMPL_java_security_Guard();
    }
}

void __INIT_IMPL_java_security_Guard()
{
    __TIB_java_security_Guard.numInterfaces = 0;
    __TIB_java_security_Guard.declaredFields = &__field_reflection_data[0];
    __TIB_java_security_Guard.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_security_Guard.methodDispatcherFunc = method_dispatcher;
    __TIB_java_security_Guard.declaredMethods = &__method_reflection_data[0];
    __TIB_java_security_Guard.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_security_Guard = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_security_Guard);
    __TIB_java_security_Guard.clazz = __CLASS_java_security_Guard;
    __TIB_java_security_Guard.baseType = JAVA_NULL;
    __CLASS_java_security_Guard_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Guard);
    __CLASS_java_security_Guard_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Guard_1ARRAY);
    __CLASS_java_security_Guard_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_Guard_2ARRAY);

    __TIB_java_security_Guard.classInitialized = 1;
}

