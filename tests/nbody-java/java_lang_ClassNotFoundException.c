#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_Throwable.h"

#include "java_lang_ClassNotFoundException.h"

#define XMLVM_CURRENT_CLASS_NAME ClassNotFoundException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ClassNotFoundException

__TIB_DEFINITION_java_lang_ClassNotFoundException __TIB_java_lang_ClassNotFoundException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ClassNotFoundException, // classInitializer
    "java.lang.ClassNotFoundException", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Exception, // extends
    sizeof(java_lang_ClassNotFoundException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException;
JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_ClassNotFoundException_serialVersionUID;

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

void __INIT_java_lang_ClassNotFoundException()
{
    staticInitializerLock(&__TIB_java_lang_ClassNotFoundException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ClassNotFoundException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ClassNotFoundException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ClassNotFoundException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ClassNotFoundException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ClassNotFoundException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ClassNotFoundException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ClassNotFoundException")
        __INIT_IMPL_java_lang_ClassNotFoundException();
    }
}

void __INIT_IMPL_java_lang_ClassNotFoundException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Exception)
    __TIB_java_lang_ClassNotFoundException.newInstanceFunc = __NEW_INSTANCE_java_lang_ClassNotFoundException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ClassNotFoundException.vtable, __TIB_java_lang_Exception.vtable, sizeof(__TIB_java_lang_Exception.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_ClassNotFoundException.vtable[6] = (VTABLE_PTR) &java_lang_ClassNotFoundException_getCause__;
    // Initialize interface information
    __TIB_java_lang_ClassNotFoundException.numImplementedInterfaces = 1;
    __TIB_java_lang_ClassNotFoundException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_ClassNotFoundException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_lang_ClassNotFoundException.itableBegin = &__TIB_java_lang_ClassNotFoundException.itable[0];

    _STATIC_java_lang_ClassNotFoundException_serialVersionUID = 9176873029745254542;

    __TIB_java_lang_ClassNotFoundException.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ClassNotFoundException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ClassNotFoundException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ClassNotFoundException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ClassNotFoundException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ClassNotFoundException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ClassNotFoundException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ClassNotFoundException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ClassNotFoundException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ClassNotFoundException);
    __TIB_java_lang_ClassNotFoundException.clazz = __CLASS_java_lang_ClassNotFoundException;
    __TIB_java_lang_ClassNotFoundException.baseType = JAVA_NULL;
    __CLASS_java_lang_ClassNotFoundException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ClassNotFoundException);
    __CLASS_java_lang_ClassNotFoundException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ClassNotFoundException_1ARRAY);
    __CLASS_java_lang_ClassNotFoundException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ClassNotFoundException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ClassNotFoundException]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ClassNotFoundException.classInitialized = 1;
}

void __DELETE_java_lang_ClassNotFoundException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ClassNotFoundException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ClassNotFoundException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Exception(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ClassNotFoundException*) me)->fields.java_lang_ClassNotFoundException.ex_ = (java_lang_Throwable*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ClassNotFoundException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ClassNotFoundException()
{    XMLVM_CLASS_INIT(java_lang_ClassNotFoundException)
java_lang_ClassNotFoundException* me = (java_lang_ClassNotFoundException*) XMLVM_MALLOC(sizeof(java_lang_ClassNotFoundException));
    me->tib = &__TIB_java_lang_ClassNotFoundException;
    __INIT_INSTANCE_MEMBERS_java_lang_ClassNotFoundException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ClassNotFoundException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ClassNotFoundException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_ClassNotFoundException();
    java_lang_ClassNotFoundException___INIT___(me);
    return me;
}

JAVA_LONG java_lang_ClassNotFoundException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_ClassNotFoundException)
    return _STATIC_java_lang_ClassNotFoundException_serialVersionUID;
}

void java_lang_ClassNotFoundException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_ClassNotFoundException)
_STATIC_java_lang_ClassNotFoundException_serialVersionUID = v;
}

void java_lang_ClassNotFoundException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ClassNotFoundException___INIT___]
    XMLVM_ENTER_METHOD("java.lang.ClassNotFoundException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 34)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    java_lang_Exception___INIT____java_lang_Throwable(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 35)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ClassNotFoundException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ClassNotFoundException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.ClassNotFoundException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 45)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    java_lang_Exception___INIT____java_lang_String_java_lang_Throwable(_r1.o, _r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 46)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ClassNotFoundException___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ClassNotFoundException___INIT____java_lang_String_java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.lang.ClassNotFoundException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 59)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 60)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ClassNotFoundException*) _r0.o)->fields.java_lang_ClassNotFoundException.ex_ = _r2.o;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 61)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ClassNotFoundException_getException__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ClassNotFoundException_getException__]
    XMLVM_ENTER_METHOD("java.lang.ClassNotFoundException", "getException", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 69)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ClassNotFoundException*) _r1.o)->fields.java_lang_ClassNotFoundException.ex_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ClassNotFoundException_getCause__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ClassNotFoundException_getCause__]
    XMLVM_ENTER_METHOD("java.lang.ClassNotFoundException", "getCause", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ClassNotFoundException.java", 80)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ClassNotFoundException*) _r1.o)->fields.java_lang_ClassNotFoundException.ex_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

