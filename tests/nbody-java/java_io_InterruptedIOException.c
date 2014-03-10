#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_io_InterruptedIOException.h"

#define XMLVM_CURRENT_CLASS_NAME InterruptedIOException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_InterruptedIOException

__TIB_DEFINITION_java_io_InterruptedIOException __TIB_java_io_InterruptedIOException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_InterruptedIOException, // classInitializer
    "java.io.InterruptedIOException", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_IOException, // extends
    sizeof(java_io_InterruptedIOException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_InterruptedIOException;
JAVA_OBJECT __CLASS_java_io_InterruptedIOException_1ARRAY;
JAVA_OBJECT __CLASS_java_io_InterruptedIOException_2ARRAY;
JAVA_OBJECT __CLASS_java_io_InterruptedIOException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_io_InterruptedIOException_serialVersionUID;

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

void __INIT_java_io_InterruptedIOException()
{
    staticInitializerLock(&__TIB_java_io_InterruptedIOException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_InterruptedIOException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_InterruptedIOException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_InterruptedIOException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_InterruptedIOException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_InterruptedIOException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_InterruptedIOException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.InterruptedIOException")
        __INIT_IMPL_java_io_InterruptedIOException();
    }
}

void __INIT_IMPL_java_io_InterruptedIOException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_IOException)
    __TIB_java_io_InterruptedIOException.newInstanceFunc = __NEW_INSTANCE_java_io_InterruptedIOException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_InterruptedIOException.vtable, __TIB_java_io_IOException.vtable, sizeof(__TIB_java_io_IOException.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_io_InterruptedIOException.numImplementedInterfaces = 1;
    __TIB_java_io_InterruptedIOException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_io_InterruptedIOException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_io_InterruptedIOException.itableBegin = &__TIB_java_io_InterruptedIOException.itable[0];

    _STATIC_java_io_InterruptedIOException_serialVersionUID = 4020568460727500567;

    __TIB_java_io_InterruptedIOException.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_InterruptedIOException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_InterruptedIOException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_InterruptedIOException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_InterruptedIOException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_InterruptedIOException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_InterruptedIOException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_InterruptedIOException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_InterruptedIOException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_InterruptedIOException);
    __TIB_java_io_InterruptedIOException.clazz = __CLASS_java_io_InterruptedIOException;
    __TIB_java_io_InterruptedIOException.baseType = JAVA_NULL;
    __CLASS_java_io_InterruptedIOException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InterruptedIOException);
    __CLASS_java_io_InterruptedIOException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InterruptedIOException_1ARRAY);
    __CLASS_java_io_InterruptedIOException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InterruptedIOException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_InterruptedIOException]
    //XMLVM_END_WRAPPER

    __TIB_java_io_InterruptedIOException.classInitialized = 1;
}

void __DELETE_java_io_InterruptedIOException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_InterruptedIOException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_InterruptedIOException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_IOException(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_io_InterruptedIOException*) me)->fields.java_io_InterruptedIOException.bytesTransferred_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_InterruptedIOException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_InterruptedIOException()
{    XMLVM_CLASS_INIT(java_io_InterruptedIOException)
java_io_InterruptedIOException* me = (java_io_InterruptedIOException*) XMLVM_MALLOC(sizeof(java_io_InterruptedIOException));
    me->tib = &__TIB_java_io_InterruptedIOException;
    __INIT_INSTANCE_MEMBERS_java_io_InterruptedIOException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_InterruptedIOException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_InterruptedIOException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_io_InterruptedIOException();
    java_io_InterruptedIOException___INIT___(me);
    return me;
}

JAVA_LONG java_io_InterruptedIOException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_io_InterruptedIOException)
    return _STATIC_java_io_InterruptedIOException_serialVersionUID;
}

void java_io_InterruptedIOException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_io_InterruptedIOException)
_STATIC_java_io_InterruptedIOException_serialVersionUID = v;
}

void java_io_InterruptedIOException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InterruptedIOException___INIT___]
    XMLVM_ENTER_METHOD("java.io.InterruptedIOException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("InterruptedIOException.java", 39)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("InterruptedIOException.java", 40)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_InterruptedIOException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InterruptedIOException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.io.InterruptedIOException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InterruptedIOException.java", 50)
    XMLVM_CHECK_NPE(0)
    java_io_IOException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("InterruptedIOException.java", 51)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

