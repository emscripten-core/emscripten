#include "xmlvm.h"

#include "java_lang_Number.h"

#define XMLVM_CURRENT_CLASS_NAME Number
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Number

__TIB_DEFINITION_java_lang_Number __TIB_java_lang_Number = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Number, // classInitializer
    "java.lang.Number", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Number), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Number;
JAVA_OBJECT __CLASS_java_lang_Number_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Number_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Number_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Number_serialVersionUID;

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

void __INIT_java_lang_Number()
{
    staticInitializerLock(&__TIB_java_lang_Number);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Number.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Number.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Number);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Number.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Number.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Number.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Number")
        __INIT_IMPL_java_lang_Number();
    }
}

void __INIT_IMPL_java_lang_Number()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Number.newInstanceFunc = __NEW_INSTANCE_java_lang_Number;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Number.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Number.vtable[6] = (VTABLE_PTR) &java_lang_Number_byteValue__;
    __TIB_java_lang_Number.vtable[11] = (VTABLE_PTR) &java_lang_Number_shortValue__;
    // Initialize interface information
    __TIB_java_lang_Number.numImplementedInterfaces = 1;
    __TIB_java_lang_Number.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Number.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_lang_Number.itableBegin = &__TIB_java_lang_Number.itable[0];

    _STATIC_java_lang_Number_serialVersionUID = -8742448824652078965;

    __TIB_java_lang_Number.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Number.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Number.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Number.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Number.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Number.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Number.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Number.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Number = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Number);
    __TIB_java_lang_Number.clazz = __CLASS_java_lang_Number;
    __TIB_java_lang_Number.baseType = JAVA_NULL;
    __CLASS_java_lang_Number_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Number);
    __CLASS_java_lang_Number_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Number_1ARRAY);
    __CLASS_java_lang_Number_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Number_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Number]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Number.classInitialized = 1;
}

void __DELETE_java_lang_Number(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Number]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Number(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Number]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Number()
{    XMLVM_CLASS_INIT(java_lang_Number)
java_lang_Number* me = (java_lang_Number*) XMLVM_MALLOC(sizeof(java_lang_Number));
    me->tib = &__TIB_java_lang_Number;
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Number]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Number()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_Number();
    java_lang_Number___INIT___(me);
    return me;
}

JAVA_LONG java_lang_Number_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Number)
    return _STATIC_java_lang_Number_serialVersionUID;
}

void java_lang_Number_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Number)
_STATIC_java_lang_Number_serialVersionUID = v;
}

void java_lang_Number___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Number___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Number", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Number.java", 33)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Number.java", 34)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Number_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Number_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Number", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Number.java", 43)
    //java_lang_Number_intValue__[9]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Number*) _r1.o)->tib->vtable[9])(_r1.o);
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Number_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Number_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Number", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Number.java", 83)
    //java_lang_Number_intValue__[9]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Number*) _r1.o)->tib->vtable[9])(_r1.o);
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

