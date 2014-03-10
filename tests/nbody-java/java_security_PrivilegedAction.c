#include "xmlvm.h"

#include "java_security_PrivilegedAction.h"

__TIB_DEFINITION_java_security_PrivilegedAction __TIB_java_security_PrivilegedAction = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_security_PrivilegedAction, // classInitializer
    "java.security.PrivilegedAction", // className
    "java.security", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_security_PrivilegedAction;
JAVA_OBJECT __CLASS_java_security_PrivilegedAction_1ARRAY;
JAVA_OBJECT __CLASS_java_security_PrivilegedAction_2ARRAY;
JAVA_OBJECT __CLASS_java_security_PrivilegedAction_3ARRAY;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_security_PrivilegedAction()
{
    staticInitializerLock(&__TIB_java_security_PrivilegedAction);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_security_PrivilegedAction.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_security_PrivilegedAction.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_security_PrivilegedAction);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_security_PrivilegedAction.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_security_PrivilegedAction.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_security_PrivilegedAction.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.security.PrivilegedAction")
        __INIT_IMPL_java_security_PrivilegedAction();
    }
}

void __INIT_IMPL_java_security_PrivilegedAction()
{
    __TIB_java_security_PrivilegedAction.numInterfaces = 0;
    __TIB_java_security_PrivilegedAction.declaredFields = &__field_reflection_data[0];
    __TIB_java_security_PrivilegedAction.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_security_PrivilegedAction.methodDispatcherFunc = method_dispatcher;
    __TIB_java_security_PrivilegedAction.declaredMethods = &__method_reflection_data[0];
    __TIB_java_security_PrivilegedAction.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_security_PrivilegedAction = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_security_PrivilegedAction);
    __TIB_java_security_PrivilegedAction.clazz = __CLASS_java_security_PrivilegedAction;
    __TIB_java_security_PrivilegedAction.baseType = JAVA_NULL;
    __CLASS_java_security_PrivilegedAction_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_PrivilegedAction);
    __CLASS_java_security_PrivilegedAction_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_PrivilegedAction_1ARRAY);
    __CLASS_java_security_PrivilegedAction_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_security_PrivilegedAction_2ARRAY);

    __TIB_java_security_PrivilegedAction.classInitialized = 1;
}

