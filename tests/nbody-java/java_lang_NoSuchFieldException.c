#include "xmlvm.h"
#include "java_lang_String.h"

#include "java_lang_NoSuchFieldException.h"

#define XMLVM_CURRENT_CLASS_NAME NoSuchFieldException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_NoSuchFieldException

__TIB_DEFINITION_java_lang_NoSuchFieldException __TIB_java_lang_NoSuchFieldException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_NoSuchFieldException, // classInitializer
    "java.lang.NoSuchFieldException", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Exception, // extends
    sizeof(java_lang_NoSuchFieldException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException;
JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_NoSuchFieldException_serialVersionUID;

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

void __INIT_java_lang_NoSuchFieldException()
{
    staticInitializerLock(&__TIB_java_lang_NoSuchFieldException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_NoSuchFieldException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_NoSuchFieldException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_NoSuchFieldException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_NoSuchFieldException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_NoSuchFieldException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_NoSuchFieldException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.NoSuchFieldException")
        __INIT_IMPL_java_lang_NoSuchFieldException();
    }
}

void __INIT_IMPL_java_lang_NoSuchFieldException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Exception)
    __TIB_java_lang_NoSuchFieldException.newInstanceFunc = __NEW_INSTANCE_java_lang_NoSuchFieldException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_NoSuchFieldException.vtable, __TIB_java_lang_Exception.vtable, sizeof(__TIB_java_lang_Exception.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_NoSuchFieldException.numImplementedInterfaces = 1;
    __TIB_java_lang_NoSuchFieldException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_NoSuchFieldException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_lang_NoSuchFieldException.itableBegin = &__TIB_java_lang_NoSuchFieldException.itable[0];

    _STATIC_java_lang_NoSuchFieldException_serialVersionUID = -6143714805279938260;

    __TIB_java_lang_NoSuchFieldException.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_NoSuchFieldException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_NoSuchFieldException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_NoSuchFieldException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_NoSuchFieldException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_NoSuchFieldException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_NoSuchFieldException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_NoSuchFieldException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_NoSuchFieldException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_NoSuchFieldException);
    __TIB_java_lang_NoSuchFieldException.clazz = __CLASS_java_lang_NoSuchFieldException;
    __TIB_java_lang_NoSuchFieldException.baseType = JAVA_NULL;
    __CLASS_java_lang_NoSuchFieldException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_NoSuchFieldException);
    __CLASS_java_lang_NoSuchFieldException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_NoSuchFieldException_1ARRAY);
    __CLASS_java_lang_NoSuchFieldException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_NoSuchFieldException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_NoSuchFieldException]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_NoSuchFieldException.classInitialized = 1;
}

void __DELETE_java_lang_NoSuchFieldException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_NoSuchFieldException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_NoSuchFieldException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Exception(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_NoSuchFieldException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_NoSuchFieldException()
{    XMLVM_CLASS_INIT(java_lang_NoSuchFieldException)
java_lang_NoSuchFieldException* me = (java_lang_NoSuchFieldException*) XMLVM_MALLOC(sizeof(java_lang_NoSuchFieldException));
    me->tib = &__TIB_java_lang_NoSuchFieldException;
    __INIT_INSTANCE_MEMBERS_java_lang_NoSuchFieldException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_NoSuchFieldException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_NoSuchFieldException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_NoSuchFieldException();
    java_lang_NoSuchFieldException___INIT___(me);
    return me;
}

JAVA_LONG java_lang_NoSuchFieldException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_NoSuchFieldException)
    return _STATIC_java_lang_NoSuchFieldException_serialVersionUID;
}

void java_lang_NoSuchFieldException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_NoSuchFieldException)
_STATIC_java_lang_NoSuchFieldException_serialVersionUID = v;
}

void java_lang_NoSuchFieldException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_NoSuchFieldException___INIT___]
    XMLVM_ENTER_METHOD("java.lang.NoSuchFieldException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("NoSuchFieldException.java", 33)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("NoSuchFieldException.java", 34)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_NoSuchFieldException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_NoSuchFieldException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.NoSuchFieldException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("NoSuchFieldException.java", 44)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("NoSuchFieldException.java", 45)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

