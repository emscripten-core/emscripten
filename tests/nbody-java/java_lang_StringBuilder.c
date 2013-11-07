#include "xmlvm.h"
#include "java_lang_Character.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"

#include "java_lang_StringBuilder.h"

#define XMLVM_CURRENT_CLASS_NAME StringBuilder
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_StringBuilder

__TIB_DEFINITION_java_lang_StringBuilder __TIB_java_lang_StringBuilder = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_StringBuilder, // classInitializer
    "java.lang.StringBuilder", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_AbstractStringBuilder, // extends
    sizeof(java_lang_StringBuilder), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_StringBuilder;
JAVA_OBJECT __CLASS_java_lang_StringBuilder_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_StringBuilder_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_StringBuilder_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_StringBuilder_serialVersionUID;

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

void __INIT_java_lang_StringBuilder()
{
    staticInitializerLock(&__TIB_java_lang_StringBuilder);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_StringBuilder.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_StringBuilder.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_StringBuilder);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_StringBuilder.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_StringBuilder.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_StringBuilder.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.StringBuilder")
        __INIT_IMPL_java_lang_StringBuilder();
    }
}

void __INIT_IMPL_java_lang_StringBuilder()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_AbstractStringBuilder)
    __TIB_java_lang_StringBuilder.newInstanceFunc = __NEW_INSTANCE_java_lang_StringBuilder;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_StringBuilder.vtable, __TIB_java_lang_AbstractStringBuilder.vtable, sizeof(__TIB_java_lang_AbstractStringBuilder.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_StringBuilder.vtable[6] = (VTABLE_PTR) &java_lang_StringBuilder_append___char;
    __TIB_java_lang_StringBuilder.vtable[7] = (VTABLE_PTR) &java_lang_StringBuilder_append___java_lang_CharSequence;
    __TIB_java_lang_StringBuilder.vtable[8] = (VTABLE_PTR) &java_lang_StringBuilder_append___java_lang_CharSequence_int_int;
    __TIB_java_lang_StringBuilder.vtable[5] = (VTABLE_PTR) &java_lang_StringBuilder_toString__;
    // Initialize interface information
    __TIB_java_lang_StringBuilder.numImplementedInterfaces = 3;
    __TIB_java_lang_StringBuilder.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_StringBuilder.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Appendable)

    __TIB_java_lang_StringBuilder.implementedInterfaces[0][1] = &__TIB_java_lang_Appendable;

    XMLVM_CLASS_INIT(java_lang_CharSequence)

    __TIB_java_lang_StringBuilder.implementedInterfaces[0][2] = &__TIB_java_lang_CharSequence;
    // Initialize itable for this class
    __TIB_java_lang_StringBuilder.itableBegin = &__TIB_java_lang_StringBuilder.itable[0];
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___char] = __TIB_java_lang_StringBuilder.vtable[6];
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence] = __TIB_java_lang_StringBuilder.vtable[7];
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence_int_int] = __TIB_java_lang_StringBuilder.vtable[8];
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__] = __TIB_java_lang_StringBuilder.vtable[5];
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int] = (VTABLE_PTR) &java_lang_AbstractStringBuilder_charAt___int;
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__] = (VTABLE_PTR) &java_lang_AbstractStringBuilder_length__;
    __TIB_java_lang_StringBuilder.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int] = (VTABLE_PTR) &java_lang_AbstractStringBuilder_subSequence___int_int;

    _STATIC_java_lang_StringBuilder_serialVersionUID = 4383685877147921099;

    __TIB_java_lang_StringBuilder.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_StringBuilder.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_StringBuilder.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_StringBuilder.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_StringBuilder.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_StringBuilder.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_StringBuilder.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_StringBuilder.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_StringBuilder = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_StringBuilder);
    __TIB_java_lang_StringBuilder.clazz = __CLASS_java_lang_StringBuilder;
    __TIB_java_lang_StringBuilder.baseType = JAVA_NULL;
    __CLASS_java_lang_StringBuilder_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StringBuilder);
    __CLASS_java_lang_StringBuilder_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StringBuilder_1ARRAY);
    __CLASS_java_lang_StringBuilder_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_StringBuilder_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_StringBuilder]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_StringBuilder.classInitialized = 1;
}

void __DELETE_java_lang_StringBuilder(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_StringBuilder]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_StringBuilder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_AbstractStringBuilder(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_StringBuilder]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_StringBuilder()
{    XMLVM_CLASS_INIT(java_lang_StringBuilder)
java_lang_StringBuilder* me = (java_lang_StringBuilder*) XMLVM_MALLOC(sizeof(java_lang_StringBuilder));
    me->tib = &__TIB_java_lang_StringBuilder;
    __INIT_INSTANCE_MEMBERS_java_lang_StringBuilder(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_StringBuilder]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_StringBuilder()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_StringBuilder();
    java_lang_StringBuilder___INIT___(me);
    return me;
}

JAVA_LONG java_lang_StringBuilder_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_StringBuilder)
    return _STATIC_java_lang_StringBuilder_serialVersionUID;
}

void java_lang_StringBuilder_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_StringBuilder)
_STATIC_java_lang_StringBuilder_serialVersionUID = v;
}

void java_lang_StringBuilder___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder___INIT___]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 54)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 55)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_StringBuilder___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder___INIT____int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 67)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 68)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_StringBuilder___INIT____java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder___INIT____java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 81)
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder___INIT____java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 82)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_StringBuilder___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 95)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 96)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___boolean]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 109)
    if (_r2.i == 0) goto label8;
    // "true"
    _r0.o = xmlvm_create_java_string_from_pool(4);
    label4:;
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 110)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label8:;
    // "false"
    _r0.o = xmlvm_create_java_string_from_pool(178);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___char]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 124)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_append0___char(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 125)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 139)
    _r0.o = java_lang_Integer_toString___int(_r2.i);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 140)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___long]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.l = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 154)
    _r0.o = java_lang_Long_toString___long(_r2.l);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 155)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___float(JAVA_OBJECT me, JAVA_FLOAT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___float]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.f = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 169)
    _r0.o = java_lang_Float_toString___float(_r2.f);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 170)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___double(JAVA_OBJECT me, JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___double]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.d = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 184)
    _r0.o = java_lang_Double_toString___double(_r2.d);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 185)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 199)
    if (_r2.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 200)
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_appendNull__(_r1.o);
    label5:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 204)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label6:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 202)
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[5])(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 216)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 217)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___java_lang_StringBuffer]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 230)
    if (_r4.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 231)
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_appendNull__(_r3.o);
    label5:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 235)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label6:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 233)

    
    // Red class access removed: java.lang.StringBuffer::getValue
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.i = 0;

    
    // Red class access removed: java.lang.StringBuffer::length
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_append0___char_1ARRAY_int_int(_r3.o, _r0.o, _r1.i, _r2.i);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 249)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_append0___char_1ARRAY(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 250)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 271)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_append0___char_1ARRAY_int_int(_r0.o, _r1.o, _r2.i, _r3.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 272)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 285)
    if (_r2.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 286)
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_appendNull__(_r1.o);
    label5:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 290)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label6:;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 288)
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r1.o, _r0.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_append___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_append___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 311)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_append0___java_lang_CharSequence_int_int(_r0.o, _r1.o, _r2.i, _r3.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 312)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_appendCodePoint___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_appendCodePoint___int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "appendCodePoint", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 325)
    _r0.o = java_lang_Character_toChars___int(_r2.i);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_append0___char_1ARRAY(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 326)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_delete___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_delete___int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "delete", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 343)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_delete0___int_int(_r0.o, _r1.i, _r2.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 344)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_deleteCharAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_deleteCharAt___int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "deleteCharAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 359)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_deleteCharAt0___int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 360)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_boolean(JAVA_OBJECT me, JAVA_INT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_boolean]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 380)
    if (_r3.i == 0) goto label8;
    // "true"
    _r0.o = xmlvm_create_java_string_from_pool(4);
    label4:;
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 381)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label8:;
    // "false"
    _r0.o = xmlvm_create_java_string_from_pool(178);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_char]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 400)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_insert0___int_char(_r0.o, _r1.i, _r2.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 401)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 420)
    _r0.o = java_lang_Integer_toString___int(_r3.i);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 421)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_long(JAVA_OBJECT me, JAVA_INT n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_long]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.i = n1;
    _r3.l = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 440)
    _r0.o = java_lang_Long_toString___long(_r3.l);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 441)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_float]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 460)
    _r0.o = java_lang_Float_toString___float(_r3.f);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 461)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_double(JAVA_OBJECT me, JAVA_INT n1, JAVA_DOUBLE n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_double]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.i = n1;
    _r3.d = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 481)
    _r0.o = java_lang_Double_toString___double(_r3.d);
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 482)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 501)
    if (_r3.o != JAVA_NULL) goto label8;
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label4:;
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 502)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label8:;
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[5])(_r3.o);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 519)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r0.o, _r1.i, _r2.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 520)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 539)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY(_r0.o, _r1.i, _r2.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 540)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    _r3.i = n3;
    _r4.i = n4;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 566)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 567)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_CharSequence(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 587)
    if (_r3.o != JAVA_NULL) goto label8;
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label4:;
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r1.o, _r2.i, _r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 588)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label8:;
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r3.o);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_insert___int_java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "insert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    _r3.i = n3;
    _r4.i = n4;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 615)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_CharSequence_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 616)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_replace___int_int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_replace___int_int_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "replace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.i = n1;
    _r2.i = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 637)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_replace0___int_int_java_lang_String(_r0.o, _r1.i, _r2.i, _r3.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 638)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_reverse__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_reverse__]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "reverse", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 647)
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_reverse0__(_r0.o);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 648)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_StringBuilder_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_toString__]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 663)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_AbstractStringBuilder_toString__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_StringBuilder_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 679)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("StringBuilder.java", 680)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("StringBuilder.java", 681)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 682)
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_set___char_1ARRAY_int(_r2.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StringBuilder.java", 683)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_StringBuilder_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_StringBuilder_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.lang.StringBuilder", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("StringBuilder.java", 697)

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("StringBuilder.java", 698)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_AbstractStringBuilder_length__(_r1.o);

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("StringBuilder.java", 699)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_AbstractStringBuilder_getValue__(_r1.o);

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("StringBuilder.java", 700)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

