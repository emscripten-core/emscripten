#include "xmlvm.h"
#include "java_lang_String.h"
#include "java_lang_Throwable.h"

#include "java_io_IOException.h"

#define XMLVM_CURRENT_CLASS_NAME IOException
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_IOException

__TIB_DEFINITION_java_io_IOException __TIB_java_io_IOException = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_IOException, // classInitializer
    "java.io.IOException", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Exception, // extends
    sizeof(java_io_IOException), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_IOException;
JAVA_OBJECT __CLASS_java_io_IOException_1ARRAY;
JAVA_OBJECT __CLASS_java_io_IOException_2ARRAY;
JAVA_OBJECT __CLASS_java_io_IOException_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_io_IOException_serialVersionUID;

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

void __INIT_java_io_IOException()
{
    staticInitializerLock(&__TIB_java_io_IOException);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_IOException.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_IOException.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_IOException);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_IOException.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_IOException.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_IOException.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.IOException")
        __INIT_IMPL_java_io_IOException();
    }
}

void __INIT_IMPL_java_io_IOException()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Exception)
    __TIB_java_io_IOException.newInstanceFunc = __NEW_INSTANCE_java_io_IOException;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_IOException.vtable, __TIB_java_lang_Exception.vtable, sizeof(__TIB_java_lang_Exception.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_io_IOException.numImplementedInterfaces = 1;
    __TIB_java_io_IOException.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_io_IOException.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_io_IOException.itableBegin = &__TIB_java_io_IOException.itable[0];

    _STATIC_java_io_IOException_serialVersionUID = 7818375828146090155;

    __TIB_java_io_IOException.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_IOException.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_IOException.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_IOException.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_IOException.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_IOException.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_IOException.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_IOException.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_IOException = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_IOException);
    __TIB_java_io_IOException.clazz = __CLASS_java_io_IOException;
    __TIB_java_io_IOException.baseType = JAVA_NULL;
    __CLASS_java_io_IOException_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_IOException);
    __CLASS_java_io_IOException_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_IOException_1ARRAY);
    __CLASS_java_io_IOException_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_IOException_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_IOException]
    //XMLVM_END_WRAPPER

    __TIB_java_io_IOException.classInitialized = 1;
}

void __DELETE_java_io_IOException(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_IOException]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_IOException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Exception(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_IOException]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_IOException()
{    XMLVM_CLASS_INIT(java_io_IOException)
java_io_IOException* me = (java_io_IOException*) XMLVM_MALLOC(sizeof(java_io_IOException));
    me->tib = &__TIB_java_io_IOException;
    __INIT_INSTANCE_MEMBERS_java_io_IOException(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_IOException]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_IOException()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_io_IOException();
    java_io_IOException___INIT___(me);
    return me;
}

JAVA_LONG java_io_IOException_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_io_IOException)
    return _STATIC_java_io_IOException_serialVersionUID;
}

void java_io_IOException_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_io_IOException)
_STATIC_java_io_IOException_serialVersionUID = v;
}

void java_io_IOException___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_IOException___INIT___]
    XMLVM_ENTER_METHOD("java.io.IOException", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("IOException.java", 34)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("IOException.java", 35)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_IOException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_IOException___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.io.IOException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("IOException.java", 45)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("IOException.java", 46)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_IOException___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_io_IOException___INIT____java_lang_String_java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.io.IOException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("IOException.java", 59)
    XMLVM_CHECK_NPE(0)
    java_lang_Exception___INIT____java_lang_String_java_lang_Throwable(_r0.o, _r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("IOException.java", 60)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_IOException___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_IOException___INIT____java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.io.IOException", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("IOException.java", 70)
    if (_r2.o != JAVA_NULL) goto label7;
    _r0.o = JAVA_NULL;
    label3:;
    XMLVM_CHECK_NPE(1)
    java_lang_Exception___INIT____java_lang_String_java_lang_Throwable(_r1.o, _r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("IOException.java", 71)
    XMLVM_EXIT_METHOD()
    return;
    label7:;
    //java_lang_Throwable_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r2.o)->tib->vtable[5])(_r2.o);
    goto label3;
    //XMLVM_END_WRAPPER
}

