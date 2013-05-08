#include "xmlvm.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"

#include "java_lang_StackTraceElement.h"

#define XMLVM_CURRENT_CLASS_NAME StackTraceElement
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_StackTraceElement

__TIB_DEFINITION_java_lang_StackTraceElement __TIB_java_lang_StackTraceElement = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_StackTraceElement, // classInitializer
    "java.lang.StackTraceElement", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_StackTraceElement), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_StackTraceElement;
JAVA_OBJECT __CLASS_java_lang_StackTraceElement_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_StackTraceElement_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_StackTraceElement_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_StackTraceElement_serialVersionUID;

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

void __INIT_java_lang_StackTraceElement()
{
    staticInitializerLock(&__TIB_java_lang_StackTraceElement);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_StackTraceElement.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_StackTraceElement.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_StackTraceElement);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_StackTraceElement.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_StackTraceElement.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_StackTraceElement.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.StackTraceElement")
        __INIT_IMPL_java_lang_StackTraceElement();
    }
}

void __INIT_IMPL_java_lang_StackTraceElement()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_StackTraceElement.newInstanceFunc = __NEW_INSTANCE_java_lang_StackTraceElement;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_StackTraceElement.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_StackTraceElement.vtable[1] = (VTABLE_PTR) &java_lang_StackTraceElement_equals___java_lang_Object;
    __TIB_java_lang_StackTraceElement.vtable[4] = (VTABLE_PTR) &java_lang_StackTraceElement_hashCode__;
    __TIB_java_lang_StackTraceElement.vtable[5] = (VTABLE_PTR) &java_lang_StackTraceElement_toString__;
    // Initialize interface information
    __TIB_java_lang_StackTraceElement.numImplementedInterfaces = 1;
    __TIB_java_lang_StackTraceElement.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_StackTraceElement.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_lang_StackTraceElement.itableBegin = &__TIB_java_lang_StackTraceElement.itable[0];

    _STATIC_java_lang_StackTraceElement_serialVersionUID = 6992337162326171013;

    __TIB_java_lang_StackTraceElement.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_StackTraceElement.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_StackTraceElement.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_StackTraceElement.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_StackTraceElement.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_StackTraceElement.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_StackTraceElement.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_StackTraceElement.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_StackTraceElement = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_StackTraceElement);
    __TIB_java_lang_StackTraceElement.clazz = __CLASS_java_lang_StackTraceElement;
    __TIB_java_lang_StackTraceElement.baseType = JAVA_NULL;
    __CLASS_java_lang_StackTraceElement_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StackTraceElement);
    __CLASS_java_lang_StackTraceElement_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StackTraceElement_1ARRAY);
    __CLASS_java_lang_StackTraceElement_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StackTraceElement_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_StackTraceElement]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_StackTraceElement.classInitialized = 1;
}

void __DELETE_java_lang_StackTraceElement(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_StackTraceElement]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_StackTraceElement(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_StackTraceElement*) me)->fields.java_lang_StackTraceElement.declaringClass_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_StackTraceElement*) me)->fields.java_lang_StackTraceElement.methodName_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_StackTraceElement*) me)->fields.java_lang_StackTraceElement.fileName_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_StackTraceElement*) me)->fields.java_lang_StackTraceElement.lineNumber_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_StackTraceElement]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_StackTraceElement()
{    XMLVM_CLASS_INIT(java_lang_StackTraceElement)
java_lang_StackTraceElement* me = (java_lang_StackTraceElement*) XMLVM_MALLOC(sizeof(java_lang_StackTraceElement));
    me->tib = &__TIB_java_lang_StackTraceElement;
    __INIT_INSTANCE_MEMBERS_java_lang_StackTraceElement(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_StackTraceElement]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_StackTraceElement()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_StackTraceElement_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_StackTraceElement)
    return _STATIC_java_lang_StackTraceElement_serialVersionUID;
}

void java_lang_StackTraceElement_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_StackTraceElement)
_STATIC_java_lang_StackTraceElement_serialVersionUID = v;
}

void java_lang_StackTraceElement___INIT____java_lang_String_java_lang_String_java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement___INIT____java_lang_String_java_lang_String_java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r4.o = n3;
    _r5.i = n4;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 63)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 64)
    if (_r2.o == JAVA_NULL) goto label7;
    if (_r3.o != JAVA_NULL) goto label13;
    label7:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 65)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label13:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 67)
    XMLVM_CHECK_NPE(1)
    ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.declaringClass_ = _r2.o;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 68)
    XMLVM_CHECK_NPE(1)
    ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.methodName_ = _r3.o;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 69)
    XMLVM_CHECK_NPE(1)
    ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.fileName_ = _r4.o;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 70)
    XMLVM_CHECK_NPE(1)
    ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.lineNumber_ = _r5.i;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 71)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_StackTraceElement___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement___INIT___]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 79)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 80)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_StackTraceElement_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 102)
    XMLVM_CLASS_INIT(java_lang_StackTraceElement)
    _r0.i = XMLVM_ISA(_r4.o, __CLASS_java_lang_StackTraceElement);
    if (_r0.i != 0) goto label7;
    _r0 = _r2;
    label6:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 103)
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 135)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 105)
    _r4.o = _r4.o;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 111)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_StackTraceElement*) _r3.o)->fields.java_lang_StackTraceElement.methodName_;
    if (_r0.o == JAVA_NULL) goto label17;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_StackTraceElement*) _r4.o)->fields.java_lang_StackTraceElement.methodName_;
    if (_r0.o != JAVA_NULL) goto label19;
    label17:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 112)
    goto label6;
    label19:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 115)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_StackTraceElement_getMethodName__(_r3.o);
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getMethodName__(_r4.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r1.o);
    if (_r0.i != 0) goto label35;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 116)
    goto label6;
    label35:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 118)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_StackTraceElement_getClassName__(_r3.o);
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getClassName__(_r4.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r1.o);
    if (_r0.i != 0) goto label51;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 119)
    goto label6;
    label51:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 121)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_StackTraceElement_getFileName__(_r3.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 122)
    if (_r0.o != JAVA_NULL) goto label65;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 123)
    XMLVM_CHECK_NPE(4)
    _r0.o = java_lang_StackTraceElement_getFileName__(_r4.o);
    if (_r0.o == JAVA_NULL) goto label77;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 124)
    goto label6;
    label65:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 127)
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getFileName__(_r4.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r1.o);
    if (_r0.i != 0) goto label77;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 128)
    goto label6;
    label77:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 131)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_StackTraceElement_getLineNumber__(_r3.o);
    XMLVM_CHECK_NPE(4)
    _r1.i = java_lang_StackTraceElement_getLineNumber__(_r4.o);
    if (_r0.i == _r1.i) goto label89;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 132)
    goto label6;
    label89:;
    _r0.i = 1;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StackTraceElement_getClassName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_getClassName__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "getClassName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 145)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.declaringClass_;
    if (_r0.o != JAVA_NULL) goto label7;
    // "<unknown class>"
    _r0.o = xmlvm_create_java_string_from_pool(0);
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.declaringClass_;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StackTraceElement_getFileName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_getFileName__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "getFileName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 156)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.fileName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_StackTraceElement_getLineNumber__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_getLineNumber__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "getLineNumber", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 167)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.lineNumber_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StackTraceElement_getMethodName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_getMethodName__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "getMethodName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 178)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.methodName_;
    if (_r0.o != JAVA_NULL) goto label7;
    // "<unknown method>"
    _r0.o = xmlvm_create_java_string_from_pool(1);
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_StackTraceElement*) _r1.o)->fields.java_lang_StackTraceElement.methodName_;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_StackTraceElement_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 187)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_StackTraceElement*) _r2.o)->fields.java_lang_StackTraceElement.methodName_;
    if (_r0.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 189)
    _r0.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 192)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_StackTraceElement*) _r2.o)->fields.java_lang_StackTraceElement.methodName_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_StackTraceElement*) _r2.o)->fields.java_lang_StackTraceElement.declaringClass_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[4])(_r1.o);
    _r0.i = _r0.i ^ _r1.i;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_StackTraceElement_isNativeMethod__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_isNativeMethod__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "isNativeMethod", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 203)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_StackTraceElement*) _r2.o)->fields.java_lang_StackTraceElement.lineNumber_;
    _r1.i = -2;
    if (_r0.i != _r1.i) goto label7;
    _r0.i = 1;
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    _r0.i = 0;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StackTraceElement_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StackTraceElement_toString__]
    XMLVM_ENTER_METHOD("java.lang.StackTraceElement", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 208)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.i = 80;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 210)
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getClassName__(_r4.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 211)
    _r1.i = 46;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 212)
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getMethodName__(_r4.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 214)
    XMLVM_CHECK_NPE(4)
    _r1.i = java_lang_StackTraceElement_isNativeMethod__(_r4.o);
    if (_r1.i == 0) goto label42;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 215)
    // "(Native Method)"
    _r1.o = xmlvm_create_java_string_from_pool(2);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label37:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 233)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label42:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 217)
    XMLVM_CHECK_NPE(4)
    _r1.o = java_lang_StackTraceElement_getFileName__(_r4.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 219)
    if (_r1.o != JAVA_NULL) goto label54;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 220)
    // "(Unknown Source)"
    _r1.o = xmlvm_create_java_string_from_pool(3);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    goto label37;
    label54:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 222)
    XMLVM_CHECK_NPE(4)
    _r2.i = java_lang_StackTraceElement_getLineNumber__(_r4.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 224)
    _r3.i = 40;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r3.i);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 225)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 226)
    if (_r2.i < 0) goto label76;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 227)
    _r1.i = 58;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 228)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___int(_r0.o, _r2.i);
    label76:;
    XMLVM_SOURCE_POSITION("StackTraceElement.java", 230)
    _r1.i = 41;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    goto label37;
    //XMLVM_END_WRAPPER
}

