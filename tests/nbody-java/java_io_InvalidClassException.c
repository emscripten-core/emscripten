#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"

#include "java_io_InvalidClassException.h"

#define XMLVM_CURRENT_CLASS_NAME InvalidClassException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_InvalidClassException

__TIB_DEFINITION_java_io_InvalidClassException __TIB_java_io_InvalidClassException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_InvalidClassException, // classInitializer
    "java.io.InvalidClassException", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_ObjectStreamException, // extends
    sizeof(java_io_InvalidClassException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_InvalidClassException;
JAVA_OBJECT __CLASS_java_io_InvalidClassException_1ARRAY;
JAVA_OBJECT __CLASS_java_io_InvalidClassException_2ARRAY;
JAVA_OBJECT __CLASS_java_io_InvalidClassException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_io_InvalidClassException_serialVersionUID;

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

void __INIT_java_io_InvalidClassException()
{
    staticInitializerLock(&__TIB_java_io_InvalidClassException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_InvalidClassException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_InvalidClassException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_InvalidClassException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_InvalidClassException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_InvalidClassException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_InvalidClassException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.InvalidClassException")
        __INIT_IMPL_java_io_InvalidClassException();
    }
}

void __INIT_IMPL_java_io_InvalidClassException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_ObjectStreamException)
    __TIB_java_io_InvalidClassException.newInstanceFunc = __NEW_INSTANCE_java_io_InvalidClassException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_InvalidClassException.vtable, __TIB_java_io_ObjectStreamException.vtable, sizeof(__TIB_java_io_ObjectStreamException.vtable));
    // Initialize vtable for this class
    __TIB_java_io_InvalidClassException.vtable[7] = (VTABLE_PTR) &java_io_InvalidClassException_getMessage__;
    // Initialize interface information
    __TIB_java_io_InvalidClassException.numImplementedInterfaces = 1;
    __TIB_java_io_InvalidClassException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_io_InvalidClassException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_io_InvalidClassException.itableBegin = &__TIB_java_io_InvalidClassException.itable[0];

    _STATIC_java_io_InvalidClassException_serialVersionUID = -4333316296251054416;

    __TIB_java_io_InvalidClassException.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_InvalidClassException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_InvalidClassException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_InvalidClassException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_InvalidClassException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_InvalidClassException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_InvalidClassException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_InvalidClassException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_InvalidClassException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_InvalidClassException);
    __TIB_java_io_InvalidClassException.clazz = __CLASS_java_io_InvalidClassException;
    __TIB_java_io_InvalidClassException.baseType = JAVA_NULL;
    __CLASS_java_io_InvalidClassException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidClassException);
    __CLASS_java_io_InvalidClassException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidClassException_1ARRAY);
    __CLASS_java_io_InvalidClassException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_InvalidClassException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_InvalidClassException]
    //XMLVM_END_WRAPPER

    __TIB_java_io_InvalidClassException.classInitialized = 1;
}

void __DELETE_java_io_InvalidClassException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_InvalidClassException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_InvalidClassException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamException(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_io_InvalidClassException*) me)->fields.java_io_InvalidClassException.classname_ = (java_lang_String*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_InvalidClassException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_InvalidClassException()
{    XMLVM_CLASS_INIT(java_io_InvalidClassException)
java_io_InvalidClassException* me = (java_io_InvalidClassException*) XMLVM_MALLOC(sizeof(java_io_InvalidClassException));
    me->tib = &__TIB_java_io_InvalidClassException;
    __INIT_INSTANCE_MEMBERS_java_io_InvalidClassException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_InvalidClassException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_InvalidClassException()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_io_InvalidClassException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_io_InvalidClassException)
    return _STATIC_java_io_InvalidClassException_serialVersionUID;
}

void java_io_InvalidClassException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_io_InvalidClassException)
_STATIC_java_io_InvalidClassException_serialVersionUID = v;
}

void java_io_InvalidClassException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InvalidClassException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.io.InvalidClassException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 51)
    XMLVM_CHECK_NPE(0)
    java_io_ObjectStreamException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 52)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_InvalidClassException___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InvalidClassException___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.io.InvalidClassException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 65)
    XMLVM_CHECK_NPE(0)
    java_io_ObjectStreamException___INIT____java_lang_String(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 66)
    XMLVM_CHECK_NPE(0)
    ((java_io_InvalidClassException*) _r0.o)->fields.java_io_InvalidClassException.classname_ = _r1.o;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 67)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_InvalidClassException_getMessage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_InvalidClassException_getMessage__]
    XMLVM_ENTER_METHOD("java.io.InvalidClassException", "getMessage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 80)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_Throwable_getMessage__(_r3.o);
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 81)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_io_InvalidClassException*) _r3.o)->fields.java_io_InvalidClassException.classname_;
    if (_r1.o == JAVA_NULL) goto label33;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 82)
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_io_InvalidClassException*) _r3.o)->fields.java_io_InvalidClassException.classname_;
    _r2.o = java_lang_String_valueOf___java_lang_Object(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    // "; "
    _r2.o = xmlvm_create_java_string_from_pool(820);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r0.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    label33:;
    XMLVM_SOURCE_POSITION("InvalidClassException.java", 84)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

