#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_io_InvalidObjectException.h"

#define XMLVM_CURRENT_CLASS_NAME InvalidObjectException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_InvalidObjectException

__TIB_DEFINITION_java_io_InvalidObjectException __TIB_java_io_InvalidObjectException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_InvalidObjectException, // classInitializer
    "java.io.InvalidObjectException", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_ObjectStreamException, // extends
    sizeof(java_io_InvalidObjectException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_InvalidObjectException;
JAVA_OBJECT __CLASS_java_io_InvalidObjectException_1ARRAY;
JAVA_OBJECT __CLASS_java_io_InvalidObjectException_2ARRAY;
JAVA_OBJECT __CLASS_java_io_InvalidObjectException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_io_InvalidObjectException_serialVersionUID;

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

void __INIT_java_io_InvalidObjectException()
{
    staticInitializerLock(&__TIB_java_io_InvalidObjectException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_InvalidObjectException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_InvalidObjectException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_InvalidObjectException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_InvalidObjectException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_InvalidObjectException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_InvalidObjectException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.InvalidObjectException")
        __INIT_IMPL_java_io_InvalidObjectException();
    }
}

void __INIT_IMPL_java_io_InvalidObjectException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_ObjectStreamException)
    __TIB_java_io_InvalidObjectException.newInstanceFunc = __NEW_INSTANCE_java_io_InvalidObjectException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_InvalidObjectException.vtable, __TIB_java_io_ObjectStreamException.vtable, sizeof(__TIB_java_io_ObjectStreamException.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_io_InvalidObjectException.numImplementedInterfaces = 1;
    __TIB_java_io_InvalidObjectException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_io_InvalidObjectException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_io_InvalidObjectException.itableBegin = &__TIB_java_io_InvalidObjectException.itable[0];

    _STATIC_java_io_InvalidObjectException_serialVersionUID = 3233174318281839583;

    __TIB_java_io_InvalidObjectException.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_InvalidObjectException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_InvalidObjectException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_InvalidObjectException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_InvalidObjectException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_InvalidObjectException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_InvalidObjectException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_InvalidObjectException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_InvalidObjectException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_InvalidObjectException);
    __TIB_java_io_InvalidObjectException.clazz = __CLASS_java_io_InvalidObjectException;
    __TIB_java_io_InvalidObjectException.baseType = JAVA_NULL;
    __CLASS_java_io_InvalidObjectException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidObjectException);
    __CLASS_java_io_InvalidObjectException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidObjectException_1ARRAY);
    __CLASS_java_io_InvalidObjectException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidObjectException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_InvalidObjectException]
    //XMLVM_END_WRAPPER

    __TIB_java_io_InvalidObjectException.classInitialized = 1;
}

void __DELETE_java_io_InvalidObjectException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_InvalidObjectException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_InvalidObjectException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamException(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_InvalidObjectException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_InvalidObjectException()
{    XMLVM_CLASS_INIT(java_io_InvalidObjectException)
java_io_InvalidObjectException* me = (java_io_InvalidObjectException*) XMLVM_MALLOC(sizeof(java_io_InvalidObjectException));
    me->tib = &__TIB_java_io_InvalidObjectException;
    __INIT_INSTANCE_MEMBERS_java_io_InvalidObjectException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_InvalidObjectException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_InvalidObjectException()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_io_InvalidObjectException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_io_InvalidObjectException)
    return _STATIC_java_io_InvalidObjectException_serialVersionUID;
}

void java_io_InvalidObjectException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_io_InvalidObjectException)
_STATIC_java_io_InvalidObjectException_serialVersionUID = v;
}

void java_io_InvalidObjectException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InvalidObjectException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.io.InvalidObjectException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InvalidObjectException.java", 38)
    XMLVM_CHECK_NPE(0)
    java_io_ObjectStreamException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("InvalidObjectException.java", 39)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

