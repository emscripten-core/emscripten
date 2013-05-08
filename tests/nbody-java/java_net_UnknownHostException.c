#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_net_UnknownHostException.h"

#define XMLVM_CURRENT_CLASS_NAME UnknownHostException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_net_UnknownHostException

__TIB_DEFINITION_java_net_UnknownHostException __TIB_java_net_UnknownHostException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_net_UnknownHostException, // classInitializer
    "java.net.UnknownHostException", // className
    "java.net", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_IOException, // extends
    sizeof(java_net_UnknownHostException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_net_UnknownHostException;
JAVA_OBJECT __CLASS_java_net_UnknownHostException_1ARRAY;
JAVA_OBJECT __CLASS_java_net_UnknownHostException_2ARRAY;
JAVA_OBJECT __CLASS_java_net_UnknownHostException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_net_UnknownHostException_serialVersionUID;

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

void __INIT_java_net_UnknownHostException()
{
    staticInitializerLock(&__TIB_java_net_UnknownHostException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_net_UnknownHostException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_net_UnknownHostException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_net_UnknownHostException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_net_UnknownHostException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_net_UnknownHostException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_net_UnknownHostException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.net.UnknownHostException")
        __INIT_IMPL_java_net_UnknownHostException();
    }
}

void __INIT_IMPL_java_net_UnknownHostException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_IOException)
    __TIB_java_net_UnknownHostException.newInstanceFunc = __NEW_INSTANCE_java_net_UnknownHostException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_net_UnknownHostException.vtable, __TIB_java_io_IOException.vtable, sizeof(__TIB_java_io_IOException.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_net_UnknownHostException.numImplementedInterfaces = 1;
    __TIB_java_net_UnknownHostException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_net_UnknownHostException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_net_UnknownHostException.itableBegin = &__TIB_java_net_UnknownHostException.itable[0];

    _STATIC_java_net_UnknownHostException_serialVersionUID = -4639126076052875403;

    __TIB_java_net_UnknownHostException.declaredFields = &__field_reflection_data[0];
    __TIB_java_net_UnknownHostException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_net_UnknownHostException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_net_UnknownHostException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_net_UnknownHostException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_net_UnknownHostException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_net_UnknownHostException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_net_UnknownHostException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_net_UnknownHostException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_net_UnknownHostException);
    __TIB_java_net_UnknownHostException.clazz = __CLASS_java_net_UnknownHostException;
    __TIB_java_net_UnknownHostException.baseType = JAVA_NULL;
    __CLASS_java_net_UnknownHostException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_UnknownHostException);
    __CLASS_java_net_UnknownHostException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_UnknownHostException_1ARRAY);
    __CLASS_java_net_UnknownHostException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_net_UnknownHostException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_net_UnknownHostException]
    //XMLVM_END_WRAPPER

    __TIB_java_net_UnknownHostException.classInitialized = 1;
}

void __DELETE_java_net_UnknownHostException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_net_UnknownHostException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_net_UnknownHostException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_IOException(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_net_UnknownHostException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_net_UnknownHostException()
{    XMLVM_CLASS_INIT(java_net_UnknownHostException)
java_net_UnknownHostException* me = (java_net_UnknownHostException*) XMLVM_MALLOC(sizeof(java_net_UnknownHostException));
    me->tib = &__TIB_java_net_UnknownHostException;
    __INIT_INSTANCE_MEMBERS_java_net_UnknownHostException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_net_UnknownHostException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_net_UnknownHostException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_net_UnknownHostException();
    java_net_UnknownHostException___INIT___(me);
    return me;
}

JAVA_LONG java_net_UnknownHostException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_net_UnknownHostException)
    return _STATIC_java_net_UnknownHostException_serialVersionUID;
}

void java_net_UnknownHostException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_net_UnknownHostException)
_STATIC_java_net_UnknownHostException_serialVersionUID = v;
}

void java_net_UnknownHostException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_net_UnknownHostException___INIT___]
    XMLVM_ENTER_METHOD("java.net.UnknownHostException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("UnknownHostException.java", 34)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("UnknownHostException.java", 35)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_net_UnknownHostException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_net_UnknownHostException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.net.UnknownHostException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("UnknownHostException.java", 45)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("UnknownHostException.java", 46)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

