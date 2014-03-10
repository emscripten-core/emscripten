#include "xmlvm.h"

#include "java_io_FileDescriptor.h"

#define XMLVM_CURRENT_CLASS_NAME FileDescriptor
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_FileDescriptor

__TIB_DEFINITION_java_io_FileDescriptor __TIB_java_io_FileDescriptor = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_FileDescriptor, // classInitializer
    "java.io.FileDescriptor", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_io_FileDescriptor), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_FileDescriptor;
JAVA_OBJECT __CLASS_java_io_FileDescriptor_1ARRAY;
JAVA_OBJECT __CLASS_java_io_FileDescriptor_2ARRAY;
JAVA_OBJECT __CLASS_java_io_FileDescriptor_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_io_FileDescriptor_in;
static JAVA_OBJECT _STATIC_java_io_FileDescriptor_out;
static JAVA_OBJECT _STATIC_java_io_FileDescriptor_err;

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

void __INIT_java_io_FileDescriptor()
{
    staticInitializerLock(&__TIB_java_io_FileDescriptor);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_FileDescriptor.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_FileDescriptor.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_FileDescriptor);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_FileDescriptor.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_FileDescriptor.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_FileDescriptor.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.FileDescriptor")
        __INIT_IMPL_java_io_FileDescriptor();
    }
}

void __INIT_IMPL_java_io_FileDescriptor()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_io_FileDescriptor.newInstanceFunc = __NEW_INSTANCE_java_io_FileDescriptor;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_FileDescriptor.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_io_FileDescriptor.numImplementedInterfaces = 0;
    __TIB_java_io_FileDescriptor.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_io_FileDescriptor_in = (java_io_FileDescriptor*) JAVA_NULL;
    _STATIC_java_io_FileDescriptor_out = (java_io_FileDescriptor*) JAVA_NULL;
    _STATIC_java_io_FileDescriptor_err = (java_io_FileDescriptor*) JAVA_NULL;

    __TIB_java_io_FileDescriptor.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_FileDescriptor.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_FileDescriptor.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_FileDescriptor.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_FileDescriptor.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_FileDescriptor.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_FileDescriptor.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_FileDescriptor.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_FileDescriptor = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_FileDescriptor);
    __TIB_java_io_FileDescriptor.clazz = __CLASS_java_io_FileDescriptor;
    __TIB_java_io_FileDescriptor.baseType = JAVA_NULL;
    __CLASS_java_io_FileDescriptor_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FileDescriptor);
    __CLASS_java_io_FileDescriptor_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FileDescriptor_1ARRAY);
    __CLASS_java_io_FileDescriptor_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FileDescriptor_2ARRAY);
    java_io_FileDescriptor___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_FileDescriptor]
    //XMLVM_END_WRAPPER

    __TIB_java_io_FileDescriptor.classInitialized = 1;
}

void __DELETE_java_io_FileDescriptor(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_FileDescriptor]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_FileDescriptor(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_io_FileDescriptor*) me)->fields.java_io_FileDescriptor.descriptor_ = 0;
    ((java_io_FileDescriptor*) me)->fields.java_io_FileDescriptor.readOnly_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_FileDescriptor]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_FileDescriptor()
{    XMLVM_CLASS_INIT(java_io_FileDescriptor)
java_io_FileDescriptor* me = (java_io_FileDescriptor*) XMLVM_MALLOC(sizeof(java_io_FileDescriptor));
    me->tib = &__TIB_java_io_FileDescriptor;
    __INIT_INSTANCE_MEMBERS_java_io_FileDescriptor(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_FileDescriptor]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_FileDescriptor()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_io_FileDescriptor();
    java_io_FileDescriptor___INIT___(me);
    return me;
}

JAVA_OBJECT java_io_FileDescriptor_GET_in()
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
    return _STATIC_java_io_FileDescriptor_in;
}

void java_io_FileDescriptor_PUT_in(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
_STATIC_java_io_FileDescriptor_in = v;
}

JAVA_OBJECT java_io_FileDescriptor_GET_out()
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
    return _STATIC_java_io_FileDescriptor_out;
}

void java_io_FileDescriptor_PUT_out(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
_STATIC_java_io_FileDescriptor_out = v;
}

JAVA_OBJECT java_io_FileDescriptor_GET_err()
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
    return _STATIC_java_io_FileDescriptor_err;
}

void java_io_FileDescriptor_PUT_err(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_io_FileDescriptor)
_STATIC_java_io_FileDescriptor_err = v;
}

void java_io_FileDescriptor___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_io_FileDescriptor___CLINIT___]
    XMLVM_ENTER_METHOD("java.io.FileDescriptor", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 43)
    _r0.o = __NEW_java_io_FileDescriptor();
    XMLVM_CHECK_NPE(0)
    java_io_FileDescriptor___INIT___(_r0.o);
    java_io_FileDescriptor_PUT_in( _r0.o);
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 48)
    _r0.o = __NEW_java_io_FileDescriptor();
    XMLVM_CHECK_NPE(0)
    java_io_FileDescriptor___INIT___(_r0.o);
    java_io_FileDescriptor_PUT_out( _r0.o);
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 53)
    _r0.o = __NEW_java_io_FileDescriptor();
    XMLVM_CHECK_NPE(0)
    java_io_FileDescriptor___INIT___(_r0.o);
    java_io_FileDescriptor_PUT_err( _r0.o);
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 66)
    _r0.o = java_io_FileDescriptor_GET_in();
    _r1.l = 0;
    XMLVM_CHECK_NPE(0)
    ((java_io_FileDescriptor*) _r0.o)->fields.java_io_FileDescriptor.descriptor_ = _r1.l;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 67)
    _r0.o = java_io_FileDescriptor_GET_out();
    _r1.l = 1;
    XMLVM_CHECK_NPE(0)
    ((java_io_FileDescriptor*) _r0.o)->fields.java_io_FileDescriptor.descriptor_ = _r1.l;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 68)
    _r0.o = java_io_FileDescriptor_GET_err();
    _r1.l = 2;
    XMLVM_CHECK_NPE(0)
    ((java_io_FileDescriptor*) _r0.o)->fields.java_io_FileDescriptor.descriptor_ = _r1.l;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 70)
    java_io_FileDescriptor_oneTimeInitialization__();
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 38)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_io_FileDescriptor_oneTimeInitialization__()]

void java_io_FileDescriptor___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FileDescriptor___INIT___]
    XMLVM_ENTER_METHOD("java.io.FileDescriptor", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 78)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 59)
    _r0.l = -1;
    XMLVM_CHECK_NPE(2)
    ((java_io_FileDescriptor*) _r2.o)->fields.java_io_FileDescriptor.descriptor_ = _r0.l;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 61)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_io_FileDescriptor*) _r2.o)->fields.java_io_FileDescriptor.readOnly_ = _r0.i;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 79)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_FileDescriptor_sync__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FileDescriptor_sync__]
    XMLVM_ENTER_METHOD("java.io.FileDescriptor", "sync", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 90)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_io_FileDescriptor*) _r1.o)->fields.java_io_FileDescriptor.readOnly_;
    if (_r0.i != 0) goto label7;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 91)
    XMLVM_CHECK_NPE(1)
    java_io_FileDescriptor_syncImpl__(_r1.o);
    label7:;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 93)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_io_FileDescriptor_syncImpl__(JAVA_OBJECT me)]

JAVA_BOOLEAN java_io_FileDescriptor_valid__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FileDescriptor_valid__]
    XMLVM_ENTER_METHOD("java.io.FileDescriptor", "valid", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("FileDescriptor.java", 104)
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_io_FileDescriptor*) _r4.o)->fields.java_io_FileDescriptor.descriptor_;
    _r2.l = -1;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i == 0) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 0;
    goto label9;
    //XMLVM_END_WRAPPER
}

