#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_net_BindException.h"

#define XMLVM_CURRENT_CLASS_NAME BindException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_net_BindException

__TIB_DEFINITION_java_net_BindException __TIB_java_net_BindException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_net_BindException, // classInitializer
    "java.net.BindException", // className
    "java.net", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_net_SocketException, // extends
    sizeof(java_net_BindException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_net_BindException;
JAVA_OBJECT __CLASS_java_net_BindException_1ARRAY;
JAVA_OBJECT __CLASS_java_net_BindException_2ARRAY;
JAVA_OBJECT __CLASS_java_net_BindException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_net_BindException_serialVersionUID;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_java_net_BindException()
{
    staticInitializerLock(&__TIB_java_net_BindException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_net_BindException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_net_BindException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_net_BindException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_net_BindException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_net_BindException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_net_BindException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.net.BindException")
        __INIT_IMPL_java_net_BindException();
    }
}

void __INIT_IMPL_java_net_BindException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_net_SocketException)
    __TIB_java_net_BindException.newInstanceFunc = __NEW_INSTANCE_java_net_BindException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_net_BindException.vtable, __TIB_java_net_SocketException.vtable, sizeof(__TIB_java_net_SocketException.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_net_BindException.numImplementedInterfaces = 1;
    __TIB_java_net_BindException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_net_BindException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_net_BindException.itableBegin = &__TIB_java_net_BindException.itable[0];

    _STATIC_java_net_BindException_serialVersionUID = -5945005768251722951;

    __TIB_java_net_BindException.declaredFields = &__field_reflection_data[0];
    __TIB_java_net_BindException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_net_BindException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_net_BindException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_net_BindException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_net_BindException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_net_BindException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_net_BindException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_net_BindException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_net_BindException);
    __TIB_java_net_BindException.clazz = __CLASS_java_net_BindException;
    __TIB_java_net_BindException.baseType = JAVA_NULL;
    __CLASS_java_net_BindException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_BindException);
    __CLASS_java_net_BindException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_BindException_1ARRAY);
    __CLASS_java_net_BindException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_BindException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_net_BindException]
    //XMLVM_END_WRAPPER

    __TIB_java_net_BindException.classInitialized = 1;
}

void __DELETE_java_net_BindException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_net_BindException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_net_BindException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_net_SocketException(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_net_BindException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_net_BindException()
{    XMLVM_CLASS_INIT(java_net_BindException)
java_net_BindException* me = (java_net_BindException*) XMLVM_MALLOC(sizeof(java_net_BindException));
    me->tib = &__TIB_java_net_BindException;
    __INIT_INSTANCE_MEMBERS_java_net_BindException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_net_BindException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_net_BindException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_net_BindException();
    java_net_BindException___INIT___(me);
    return me;
}

JAVA_LONG java_net_BindException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_net_BindException)
    return _STATIC_java_net_BindException_serialVersionUID;
}

void java_net_BindException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_net_BindException)
_STATIC_java_net_BindException_serialVersionUID = v;
}

void java_net_BindException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_BindException___INIT___]
    XMLVM_ENTER_METHOD("java.net.BindException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("BindException.java", 32)
    XMLVM_CHECK_NPE(0)
    java_net_SocketException___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("BindException.java", 33)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_BindException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_BindException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.net.BindException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("BindException.java", 42)
    XMLVM_CHECK_NPE(0)
    java_net_SocketException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("BindException.java", 43)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

