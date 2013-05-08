#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_net_SocketException.h"

#define XMLVM_CURRENT_CLASS_NAME SocketException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_net_SocketException

__TIB_DEFINITION_java_net_SocketException __TIB_java_net_SocketException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_net_SocketException, // classInitializer
    "java.net.SocketException", // className
    "java.net", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_IOException, // extends
    sizeof(java_net_SocketException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_net_SocketException;
JAVA_OBJECT __CLASS_java_net_SocketException_1ARRAY;
JAVA_OBJECT __CLASS_java_net_SocketException_2ARRAY;
JAVA_OBJECT __CLASS_java_net_SocketException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_net_SocketException_serialVersionUID;

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

void __INIT_java_net_SocketException()
{
    staticInitializerLock(&__TIB_java_net_SocketException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_net_SocketException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_net_SocketException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_net_SocketException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_net_SocketException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_net_SocketException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_net_SocketException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.net.SocketException")
        __INIT_IMPL_java_net_SocketException();
    }
}

void __INIT_IMPL_java_net_SocketException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_IOException)
    __TIB_java_net_SocketException.newInstanceFunc = __NEW_INSTANCE_java_net_SocketException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_net_SocketException.vtable, __TIB_java_io_IOException.vtable, sizeof(__TIB_java_io_IOException.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_net_SocketException.numImplementedInterfaces = 1;
    __TIB_java_net_SocketException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_net_SocketException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_net_SocketException.itableBegin = &__TIB_java_net_SocketException.itable[0];

    _STATIC_java_net_SocketException_serialVersionUID = -5935874303556886934;

    __TIB_java_net_SocketException.declaredFields = &__field_reflection_data[0];
    __TIB_java_net_SocketException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_net_SocketException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_net_SocketException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_net_SocketException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_net_SocketException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_net_SocketException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_net_SocketException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_net_SocketException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_net_SocketException);
    __TIB_java_net_SocketException.clazz = __CLASS_java_net_SocketException;
    __TIB_java_net_SocketException.baseType = JAVA_NULL;
    __CLASS_java_net_SocketException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_SocketException);
    __CLASS_java_net_SocketException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_SocketException_1ARRAY);
    __CLASS_java_net_SocketException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_SocketException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_net_SocketException]
    //XMLVM_END_WRAPPER

    __TIB_java_net_SocketException.classInitialized = 1;
}

void __DELETE_java_net_SocketException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_net_SocketException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_net_SocketException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_IOException(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_net_SocketException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_net_SocketException()
{    XMLVM_CLASS_INIT(java_net_SocketException)
java_net_SocketException* me = (java_net_SocketException*) XMLVM_MALLOC(sizeof(java_net_SocketException));
    me->tib = &__TIB_java_net_SocketException;
    __INIT_INSTANCE_MEMBERS_java_net_SocketException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_net_SocketException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_net_SocketException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_net_SocketException();
    java_net_SocketException___INIT___(me);
    return me;
}

JAVA_LONG java_net_SocketException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_net_SocketException)
    return _STATIC_java_net_SocketException_serialVersionUID;
}

void java_net_SocketException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_net_SocketException)
_STATIC_java_net_SocketException_serialVersionUID = v;
}

void java_net_SocketException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_SocketException___INIT___]
    XMLVM_ENTER_METHOD("java.net.SocketException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("SocketException.java", 35)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("SocketException.java", 36)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_SocketException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_SocketException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.net.SocketException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("SocketException.java", 46)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("SocketException.java", 47)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

