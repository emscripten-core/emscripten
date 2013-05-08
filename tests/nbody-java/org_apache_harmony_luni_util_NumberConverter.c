#include "xmlvm.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "java_lang_Integer.h"
#include "java_lang_Math.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"

#include "org_apache_harmony_luni_util_NumberConverter.h"

#define XMLVM_CURRENT_CLASS_NAME NumberConverter
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_luni_util_NumberConverter

__TIB_DEFINITION_org_apache_harmony_luni_util_NumberConverter __TIB_org_apache_harmony_luni_util_NumberConverter = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_util_NumberConverter, // classInitializer
    "org.apache.harmony.luni.util.NumberConverter", // className
    "org.apache.harmony.luni.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_apache_harmony_luni_util_NumberConverter), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_DOUBLE _STATIC_org_apache_harmony_luni_util_NumberConverter_invLogOfTenBaseTwo;
static JAVA_OBJECT _STATIC_org_apache_harmony_luni_util_NumberConverter_TEN_TO_THE;

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

void __INIT_org_apache_harmony_luni_util_NumberConverter()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_util_NumberConverter);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_util_NumberConverter.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_util_NumberConverter.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_util_NumberConverter);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_util_NumberConverter.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_util_NumberConverter.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_util_NumberConverter.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.util.NumberConverter")
        __INIT_IMPL_org_apache_harmony_luni_util_NumberConverter();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_util_NumberConverter()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_apache_harmony_luni_util_NumberConverter.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_luni_util_NumberConverter;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_luni_util_NumberConverter.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_org_apache_harmony_luni_util_NumberConverter.numImplementedInterfaces = 0;
    __TIB_org_apache_harmony_luni_util_NumberConverter.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_org_apache_harmony_luni_util_NumberConverter_invLogOfTenBaseTwo = 0;
    _STATIC_org_apache_harmony_luni_util_NumberConverter_TEN_TO_THE = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_org_apache_harmony_luni_util_NumberConverter.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_NumberConverter.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_util_NumberConverter.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_luni_util_NumberConverter.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_NumberConverter.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_util_NumberConverter.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_util_NumberConverter.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_util_NumberConverter.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_luni_util_NumberConverter = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_util_NumberConverter);
    __TIB_org_apache_harmony_luni_util_NumberConverter.clazz = __CLASS_org_apache_harmony_luni_util_NumberConverter;
    __TIB_org_apache_harmony_luni_util_NumberConverter.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_util_NumberConverter_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_NumberConverter);
    __CLASS_org_apache_harmony_luni_util_NumberConverter_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_NumberConverter_1ARRAY);
    __CLASS_org_apache_harmony_luni_util_NumberConverter_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_util_NumberConverter_2ARRAY);
    org_apache_harmony_luni_util_NumberConverter___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_luni_util_NumberConverter]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_luni_util_NumberConverter.classInitialized = 1;
}

void __DELETE_org_apache_harmony_luni_util_NumberConverter(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_luni_util_NumberConverter]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_NumberConverter(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_luni_util_NumberConverter*) me)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = 0;
    ((org_apache_harmony_luni_util_NumberConverter*) me)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = 0;
    ((org_apache_harmony_luni_util_NumberConverter*) me)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((org_apache_harmony_luni_util_NumberConverter*) me)->fields.org_apache_harmony_luni_util_NumberConverter.firstK_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_NumberConverter]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_luni_util_NumberConverter()
{    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
org_apache_harmony_luni_util_NumberConverter* me = (org_apache_harmony_luni_util_NumberConverter*) XMLVM_MALLOC(sizeof(org_apache_harmony_luni_util_NumberConverter));
    me->tib = &__TIB_org_apache_harmony_luni_util_NumberConverter;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_NumberConverter(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_luni_util_NumberConverter]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_util_NumberConverter()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_apache_harmony_luni_util_NumberConverter();
    org_apache_harmony_luni_util_NumberConverter___INIT___(me);
    return me;
}

JAVA_DOUBLE org_apache_harmony_luni_util_NumberConverter_GET_invLogOfTenBaseTwo()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
    return _STATIC_org_apache_harmony_luni_util_NumberConverter_invLogOfTenBaseTwo;
}

void org_apache_harmony_luni_util_NumberConverter_PUT_invLogOfTenBaseTwo(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
_STATIC_org_apache_harmony_luni_util_NumberConverter_invLogOfTenBaseTwo = v;
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
    return _STATIC_org_apache_harmony_luni_util_NumberConverter_TEN_TO_THE;
}

void org_apache_harmony_luni_util_NumberConverter_PUT_TEN_TO_THE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
_STATIC_org_apache_harmony_luni_util_NumberConverter_TEN_TO_THE = v;
}

void org_apache_harmony_luni_util_NumberConverter___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter___CLINIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.i = 1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 31)
    _r0.d = 2.0;
    _r0.d = java_lang_Math_log___double(_r0.d);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 32)
    _r2.d = 10.0;
    _r2.d = java_lang_Math_log___double(_r2.d);
    _r0.d = _r0.d / _r2.d;
    org_apache_harmony_luni_util_NumberConverter_PUT_invLogOfTenBaseTwo( _r0.d);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 34)
    _r0.i = 20;
    XMLVM_CLASS_INIT(long)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_long, _r0.i);
    org_apache_harmony_luni_util_NumberConverter_PUT_TEN_TO_THE( _r0.o);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 37)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    _r1.i = 0;
    _r2.l = 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.l;
    _r0 = _r7;
    label30:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 38)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i < _r1.i) goto label36;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 21)
    XMLVM_EXIT_METHOD()
    return;
    label36:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 39)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    _r2.i = _r0.i - _r7.i;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.l = ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("NumberConverter.java", 40)
    _r3.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    _r4.l = _r1.l << (0x3f & _r7.l);
    _r6.i = 3;
    _r1.l = _r1.l << (0x3f & _r6.l);
    _r1.l = _r1.l + _r4.l;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.l;
    _r0.i = _r0.i + 1;
    goto label30;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_util_NumberConverter___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter___INIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 21)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 27)
    _r0.i = 64;
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r0.i);
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_luni_util_NumberConverter*) _r1.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_ = _r0.o;
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_getConverter__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_getConverter__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "getConverter", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 45)
    _r0.o = __NEW_org_apache_harmony_luni_util_NumberConverter();
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_NumberConverter___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convert___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_convert___double]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "convert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 49)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_getConverter__();
    XMLVM_CHECK_NPE(0)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_convertD___double(_r0.o, _r1.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convert___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_util_NumberConverter)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_convert___float]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "convert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 53)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_getConverter__();
    XMLVM_CHECK_NPE(0)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_convertF___float(_r0.o, _r1.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convertD___double(JAVA_OBJECT me, JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_convertD___double]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "convertD", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    XMLVMElem _r15;
    XMLVMElem _r16;
    _r14.o = me;
    _r15.d = n1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 57)
    _r0.i = 1075;
    _r1.l = -9223372036854775808;
    _r3.l = 9218868437227405312;
    _r5.l = 4503599627370495;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 64)
    _r7.l = java_lang_Double_doubleToLongBits___double(_r15.d);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 66)
    _r1.l = _r1.l & _r7.l;
    _r9.l = 0;
    _r1.i = _r1.l > _r9.l ? 1 : (_r1.l == _r9.l ? 0 : -1);
    if (_r1.i != 0) goto label69;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    _r9 = _r1;
    label25:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 68)
    _r1.l = _r7.l & _r3.l;
    _r3.i = 52;
    _r1.l = _r1.l >> (0x3f & _r3.l);
    _r4.i = (JAVA_INT) _r1.l;
    _r1.l = _r7.l & _r5.l;
    _r5.l = 0;
    _r3.i = _r1.l > _r5.l ? 1 : (_r1.l == _r5.l ? 0 : -1);
    if (_r3.i != 0) goto label73;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 71)
    _r3.i = 1;
    _r5 = _r3;
    label41:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 72)
    _r3.i = 52;
    _r6.i = 2047;
    if (_r4.i != _r6.i) goto label79;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 74)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 75)
    if (_r5.i == 0) goto label76;
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r9.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    // "Infinity"
    _r1.o = xmlvm_create_java_string_from_pool(9);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    label68:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 106)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label69:;
    // "-"
    _r1.o = xmlvm_create_java_string_from_pool(92);
    _r9 = _r1;
    goto label25;
    label73:;
    _r3.i = 0;
    _r5 = _r3;
    goto label41;
    label76:;
    // "NaN"
    _r0.o = xmlvm_create_java_string_from_pool(8);
    goto label68;
    label79:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 76)
    if (_r4.i != 0) goto label229;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 77)
    if (_r5.i == 0) goto label103;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 78)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r9.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    // "0.0"
    _r1.o = xmlvm_create_java_string_from_pool(93);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label68;
    label103:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 79)
    _r6.l = 1;
    _r6.i = _r1.l > _r6.l ? 1 : (_r1.l == _r6.l ? 0 : -1);
    if (_r6.i != 0) goto label129;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 82)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r9.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    // "4.9E-324"
    _r1.o = xmlvm_create_java_string_from_pool(94);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label68;
    label129:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 83)
    _r6.i = 1;
    _r0.i = _r6.i - _r0.i;
    _r6 = _r1;
    label133:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 85)
    _r10.l = 4503599627370496;
    _r10.l = _r10.l & _r6.l;
    _r12.l = 0;
    _r8.i = _r10.l > _r12.l ? 1 : (_r10.l == _r12.l ? 0 : -1);
    if (_r8.i == 0) goto label224;
    _r6 = _r3;
    _r3 = _r0;
    label144:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 96)
    _r0.i = -59;
    if (_r0.i >= _r3.i) goto label151;
    _r0.i = 6;
    if (_r3.i < _r0.i) goto label157;
    label151:;
    _r0.i = -59;
    if (_r3.i != _r0.i) goto label240;
    if (_r5.i != 0) goto label240;
    label157:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 97)
    if (_r4.i != 0) goto label237;
    _r0.i = 1;
    _r4 = _r0;
    label161:;
    _r0 = _r14;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_NumberConverter_longDigitGenerator___long_int_boolean_boolean_int(_r0.o, _r1.l, _r3.i, _r4.i, _r5.i, _r6.i);
    label165:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 102)
    _r0.d = 1.0E7;
    _r0.i = _r15.d > _r0.d ? 1 : (_r15.d == _r0.d ? 0 : -1);
    if (_r0.i >= 0) goto label201;
    _r0.d = -1.0E7;
    _r0.i = _r15.d > _r0.d ? 1 : (_r15.d == _r0.d ? 0 : -1);
    if (_r0.i <= 0) goto label201;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 103)
    _r0.d = -0.001;
    _r0.i = _r15.d > _r0.d ? 1 : (_r15.d == _r0.d ? 0 : -1);
    if (_r0.i <= 0) goto label252;
    _r0.d = 0.001;
    _r0.i = _r15.d > _r0.d ? 1 : (_r15.d == _r0.d ? 0 : -1);
    if (_r0.i >= 0) goto label252;
    label201:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 104)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r9.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(14)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_freeFormatExponential__(_r14.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label68;
    label224:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 86)
    _r8.i = 1;
    _r6.l = _r6.l << (0x3f & _r8.l);
    _r3.i = _r3.i + -1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 87)
    goto label133;
    label229:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 92)
    _r6.l = 4503599627370496;
    _r1.l = _r1.l | _r6.l;
    _r0.i = _r4.i - _r0.i;
    _r6 = _r3;
    _r3 = _r0;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 93)
    goto label144;
    label237:;
    _r0.i = 0;
    _r4 = _r0;
    goto label161;
    label240:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 99)
    if (_r4.i != 0) goto label249;
    _r0.i = 1;
    _r4 = _r0;
    label244:;
    _r0 = _r14;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int(_r0.o, _r1.l, _r3.i, _r4.i, _r5.i, _r6.i);
    goto label165;
    label249:;
    _r0.i = 0;
    _r4 = _r0;
    goto label244;
    label252:;
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r9.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(14)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_freeFormat__(_r14.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label68;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convertF___float(JAVA_OBJECT me, JAVA_FLOAT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_convertF___float]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "convertF", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    _r12.o = me;
    _r13.f = n1;
    _r6.i = 8388608;
    _r11.i = -59;
    _r10.i = 0;
    _r9.i = 1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 110)
    _r0.i = 150;
    _r1.i = -2147483648;
    _r2.i = 2139095040;
    _r3.i = 8388607;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 115)
    _r4.i = java_lang_Float_floatToIntBits___float(_r13.f);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 117)
    _r1.i = _r1.i & _r4.i;
    if (_r1.i != 0) goto label62;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    _r7 = _r1;
    label25:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 119)
    _r1.i = _r4.i & _r2.i;
    _r8.i = _r1.i >> 23;
    _r1.i = _r4.i & _r3.i;
    if (_r1.i != 0) goto label66;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 122)
    _r5 = _r9;
    label34:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 123)
    _r2.i = 23;
    _r3.i = 255;
    if (_r8.i != _r3.i) goto label71;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 125)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 126)
    if (_r5.i == 0) goto label68;
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r7.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    // "Infinity"
    _r1.o = xmlvm_create_java_string_from_pool(9);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    label61:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 156)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label62:;
    // "-"
    _r1.o = xmlvm_create_java_string_from_pool(92);
    _r7 = _r1;
    goto label25;
    label66:;
    _r5 = _r10;
    goto label34;
    label68:;
    // "NaN"
    _r0.o = xmlvm_create_java_string_from_pool(8);
    goto label61;
    label71:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 127)
    if (_r8.i != 0) goto label187;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 128)
    if (_r5.i == 0) goto label95;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 129)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r7.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    // "0.0"
    _r1.o = xmlvm_create_java_string_from_pool(93);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label61;
    label95:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 130)
    _r0.i = _r9.i - _r0.i;
    _r3.i = 8;
    if (_r1.i >= _r3.i) goto label105;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 131)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 132)
    _r1.i = _r1.i << 2;
    _r0.i = _r0.i + -2;
    label105:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 133)
    _r3 = _r2;
    _r2 = _r1;
    label107:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 136)
    _r4.i = _r2.i & _r6.i;
    if (_r4.i == 0) goto label182;
    _r6 = _r3;
    _r3 = _r0;
    _r0 = _r1;
    label114:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 147)
    if (_r11.i >= _r3.i) goto label120;
    _r1.i = 35;
    if (_r3.i < _r1.i) goto label124;
    label120:;
    if (_r3.i != _r11.i) goto label196;
    if (_r5.i != 0) goto label196;
    label124:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 148)
    _r1.l = (JAVA_LONG) _r0.i;
    if (_r8.i != 0) goto label194;
    _r4 = _r9;
    label128:;
    _r0 = _r12;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_NumberConverter_longDigitGenerator___long_int_boolean_boolean_int(_r0.o, _r1.l, _r3.i, _r4.i, _r5.i, _r6.i);
    label132:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 152)
    _r0.f = 1.0E7;
    _r0.i = _r13.f > _r0.f ? 1 : (_r13.f == _r0.f ? 0 : -1);
    if (_r0.i >= 0) goto label160;
    _r0.f = -1.0E7;
    _r0.i = _r13.f > _r0.f ? 1 : (_r13.f == _r0.f ? 0 : -1);
    if (_r0.i <= 0) goto label160;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 153)
    _r0.f = -0.001;
    _r0.i = _r13.f > _r0.f ? 1 : (_r13.f == _r0.f ? 0 : -1);
    if (_r0.i <= 0) goto label207;
    _r0.f = 0.001;
    _r0.i = _r13.f > _r0.f ? 1 : (_r13.f == _r0.f ? 0 : -1);
    if (_r0.i >= 0) goto label207;
    label160:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 154)
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r7.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(12)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_freeFormatExponential__(_r12.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label61;
    label182:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 137)
    _r2.i = _r2.i << 1;
    _r3.i = _r3.i + -1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 138)
    goto label107;
    label187:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 143)
    _r1.i = _r1.i | _r6.i;
    _r0.i = _r8.i - _r0.i;
    _r6 = _r2;
    _r3 = _r0;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 144)
    goto label114;
    label194:;
    _r4 = _r10;
    goto label128;
    label196:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 150)
    _r1.l = (JAVA_LONG) _r0.i;
    if (_r8.i != 0) goto label205;
    _r4 = _r9;
    label200:;
    _r0 = _r12;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int(_r0.o, _r1.l, _r3.i, _r4.i, _r5.i, _r6.i);
    goto label132;
    label205:;
    _r4 = _r10;
    goto label200;
    label207:;
    _r0.o = __NEW_java_lang_StringBuilder();
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r7.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(12)
    _r1.o = org_apache_harmony_luni_util_NumberConverter_freeFormat__(_r12.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label61;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_freeFormatExponential__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_freeFormatExponential__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "freeFormatExponential", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    _r10.o = me;
    _r9.i = 1;
    _r8.i = 0;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 161)
    _r0.i = 25;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 162)
    XMLVM_CHECK_NPE(10)
    _r1.o = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(10)
    _r2.i = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(10)
    ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = _r3.i;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = _r1.i + 48;
    _r1.i = _r1.i & 0xffff;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r8.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r1.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 163)
    _r1.i = 46;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r9.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r1.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 166)
    _r1.i = 2;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 168)
    XMLVM_CHECK_NPE(10)
    _r2.i = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.firstK_;
    _r3 = _r1;
    _r1 = _r2;
    label30:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 169)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 171)
    _r1.i = _r1.i + -1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 172)
    XMLVM_CHECK_NPE(10)
    _r4.i = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    XMLVM_CHECK_NPE(10)
    _r5.i = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    if (_r4.i < _r5.i) goto label81;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 178)
    _r4.i = _r2.i - _r9.i;
    if (_r1.i != _r4.i) goto label100;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 179)
    _r1.i = _r3.i + 1;
    _r4.i = 48;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.i;
    label48:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 180)
    _r3.i = _r1.i + 1;
    _r4.i = 69;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 181)
    _r1.o = __NEW_java_lang_StringBuilder();
    _r4.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    java_lang_String___INIT____char_1ARRAY_int_int(_r4.o, _r0.o, _r8.i, _r3.i);
    _r0.o = java_lang_String_valueOf___java_lang_Object(_r4.o);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 182)
    _r0.o = java_lang_Integer_toString___int(_r2.i);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r0.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label81:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 175)
    _r4.i = _r3.i + 1;
    XMLVM_CHECK_NPE(10)
    _r5.o = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(10)
    _r6.i = ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    _r7.i = _r6.i + 1;
    XMLVM_CHECK_NPE(10)
    ((org_apache_harmony_luni_util_NumberConverter*) _r10.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = _r7.i;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r6.i);
    _r5.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    _r5.i = _r5.i + 48;
    _r5.i = _r5.i & 0xffff;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r5.i;
    _r3 = _r4;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 170)
    goto label30;
    label100:;
    _r1 = _r3;
    goto label48;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_freeFormat__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_freeFormat__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "freeFormat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    _r11.o = me;
    _r9.i = 46;
    _r8.i = 48;
    _r7.i = 0;
    _r6.i = -1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 187)
    _r0.i = 25;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("NumberConverter.java", 191)
    XMLVM_CHECK_NPE(11)
    _r1.i = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.firstK_;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 192)
    if (_r1.i >= 0) goto label111;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 193)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r7.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r8.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 194)
    _r2.i = 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r9.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 195)
    _r2.i = _r7.i + 2;
    _r3.i = _r1.i + 1;
    _r10 = _r3;
    _r3 = _r2;
    _r2 = _r10;
    label26:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 196)
    if (_r2.i < 0) goto label88;
    _r2 = _r3;
    label29:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 200)
    XMLVM_CHECK_NPE(11)
    _r3.o = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(11)
    _r4.i = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    _r5.i = _r4.i + 1;
    XMLVM_CHECK_NPE(11)
    ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = _r5.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r10 = _r3;
    _r3 = _r2;
    _r2 = _r1;
    _r1 = _r10;
    label43:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 202)
    if (_r1.i == _r6.i) goto label96;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 203)
    _r4.i = _r3.i + 1;
    _r1.i = _r1.i + 48;
    _r1.i = _r1.i & 0xffff;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r1.i;
    _r1 = _r4;
    label53:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 207)
    if (_r2.i != 0) goto label60;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 208)
    _r3.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r9.i;
    _r1 = _r3;
    label60:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 210)
    _r2.i = _r2.i + -1;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 211)
    XMLVM_CHECK_NPE(11)
    _r3.i = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    XMLVM_CHECK_NPE(11)
    _r4.i = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    if (_r3.i >= _r4.i) goto label103;
    XMLVM_CHECK_NPE(11)
    _r3.o = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(11)
    _r4.i = ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_;
    _r5.i = _r4.i + 1;
    XMLVM_CHECK_NPE(11)
    ((org_apache_harmony_luni_util_NumberConverter*) _r11.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = _r5.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    label78:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 212)
    if (_r3.i != _r6.i) goto label105;
    if (_r2.i >= _r6.i) goto label105;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 213)
    _r2.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____char_1ARRAY_int_int(_r2.o, _r0.o, _r7.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label88:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 197)
    _r4.i = _r3.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r8.i;
    _r2.i = _r2.i + 1;
    _r3 = _r4;
    goto label26;
    label96:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 204)
    if (_r2.i < _r6.i) goto label109;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 205)
    _r1.i = _r3.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r8.i;
    goto label53;
    label103:;
    _r3 = _r6;
    goto label78;
    label105:;
    _r10 = _r3;
    _r3 = _r1;
    _r1 = _r10;
    goto label43;
    label109:;
    _r1 = _r3;
    goto label53;
    label111:;
    _r2 = _r7;
    goto label29;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4, JAVA_INT n5)]

void org_apache_harmony_luni_util_NumberConverter_longDigitGenerator___long_int_boolean_boolean_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_util_NumberConverter_longDigitGenerator___long_int_boolean_boolean_int]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.util.NumberConverter", "longDigitGenerator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    XMLVMElem _r15;
    XMLVMElem _r16;
    XMLVMElem _r17;
    XMLVMElem _r18;
    XMLVMElem _r19;
    XMLVMElem _r20;
    XMLVMElem _r21;
    _r15.o = me;
    _r16.l = n1;
    _r18.i = n2;
    _r19.i = n3;
    _r20.i = n4;
    _r21.i = n5;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 222)
    if (_r18.i < 0) goto label130;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 223)
    _r0.l = 1;
    _r0.l = _r0.l << (0x3f & _r18.l);
    if (_r20.i != 0) goto label120;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 224)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 225)
    _r2.i = _r18.i + 1;
    _r2.l = _r16.l << (0x3f & _r2.l);
    _r4.l = 2;
    _r13 = _r4;
    _r4 = _r2;
    _r2 = _r13;
    label17:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 226)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 242)
    _r6.i = _r18.i + _r21.i;
    _r7.i = 1;
    _r6.i = _r6.i - _r7.i;
    _r6.d = (JAVA_DOUBLE) _r6.i;
    _r8.d = org_apache_harmony_luni_util_NumberConverter_GET_invLogOfTenBaseTwo();
    _r6.d = _r6.d * _r8.d;
    _r8.d = 1.0E-10;
    _r6.d = _r6.d - _r8.d;
    _r6.d = java_lang_Math_ceil___double(_r6.d);
    _r6.i = (JAVA_INT) _r6.d;
    if (_r6.i <= 0) goto label164;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 244)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 245)
    _r7.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r6.i);
    _r7.l = ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    _r2.l = _r2.l * _r7.l;
    label43:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 252)
    _r7.l = _r4.l + _r0.l;
    _r7.i = _r7.l > _r2.l ? 1 : (_r7.l == _r2.l ? 0 : -1);
    if (_r7.i <= 0) goto label184;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 253)
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.firstK_ = _r6.i;
    label51:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 260)
    _r6.i = 0;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r6.i;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = _r6.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 263)
    _r6.i = 4;
    XMLVM_CLASS_INIT(long)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_long, _r6.i);
    _r7.i = 0;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r2.l;
    _r7.i = 1;
    _r8.i = 1;
    _r8.l = _r2.l << (0x3f & _r8.l);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r8.l;
    _r7.i = 2;
    _r8.i = 2;
    _r8.l = _r2.l << (0x3f & _r8.l);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r8.l;
    _r7.i = 3;
    _r8.i = 3;
    _r8.l = _r2.l << (0x3f & _r8.l);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r8.l;
    label80:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 269)
    _r7.i = 0;
    _r8.i = 3;
    _r13 = _r8;
    _r14 = _r7;
    _r7 = _r4;
    _r4 = _r13;
    _r5 = _r14;
    label87:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 271)
    if (_r4.i >= 0) goto label196;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 279)
    _r4.i = _r7.l > _r0.l ? 1 : (_r7.l == _r0.l ? 0 : -1);
    if (_r4.i >= 0) goto label213;
    _r4.i = 1;
    label94:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 280)
    _r9.l = _r7.l + _r0.l;
    _r9.i = _r9.l > _r2.l ? 1 : (_r9.l == _r2.l ? 0 : -1);
    if (_r9.i <= 0) goto label215;
    _r9.i = 1;
    label101:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 282)
    if (_r4.i != 0) goto label105;
    if (_r9.i == 0) goto label217;
    label105:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 289)
    if (_r4.i == 0) goto label236;
    if (_r9.i != 0) goto label236;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 290)
    XMLVM_CHECK_NPE(15)
    _r0.o = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(15)
    _r1.i = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    label119:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 297)
    XMLVM_EXIT_METHOD()
    return;
    label120:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 228)
    _r2.i = _r18.i + 2;
    _r2.l = _r16.l << (0x3f & _r2.l);
    _r4.l = 4;
    _r13 = _r4;
    _r4 = _r2;
    _r2 = _r13;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 229)
    goto label17;
    label130:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 232)
    _r0.l = 1;
    if (_r19.i != 0) goto label136;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 233)
    if (_r20.i != 0) goto label150;
    label136:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 234)
    _r2.i = 1;
    _r2.l = _r16.l << (0x3f & _r2.l);
    _r4.l = 1;
    _r6.i = 1;
    _r6.i = _r6.i - _r18.i;
    _r4.l = _r4.l << (0x3f & _r6.l);
    _r13 = _r4;
    _r4 = _r2;
    _r2 = _r13;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 235)
    goto label17;
    label150:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 237)
    _r2.i = 2;
    _r2.l = _r16.l << (0x3f & _r2.l);
    _r4.l = 1;
    _r6.i = 2;
    _r6.i = _r6.i - _r18.i;
    _r4.l = _r4.l << (0x3f & _r6.l);
    _r13 = _r4;
    _r4 = _r2;
    _r2 = _r13;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 238)
    goto label17;
    label164:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 246)
    if (_r6.i >= 0) goto label43;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 247)
    _r7.o = org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
    _r8.i = -_r6.i;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r8.i);
    _r7.l = ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i];
    XMLVM_SOURCE_POSITION("NumberConverter.java", 248)
    _r4.l = _r4.l * _r7.l;
    _r9.l = 1;
    _r9.i = _r0.l > _r9.l ? 1 : (_r0.l == _r9.l ? 0 : -1);
    if (_r9.i != 0) goto label181;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 249)
    _r0 = _r7;
    goto label43;
    label181:;
    _r0.l = _r0.l * _r7.l;
    goto label43;
    label184:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 255)
    _r7.i = 1;
    _r6.i = _r6.i - _r7.i;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.firstK_ = _r6.i;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 256)
    _r6.l = 10;
    _r4.l = _r4.l * _r6.l;
    _r6.l = 10;
    _r0.l = _r0.l * _r6.l;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 257)
    goto label51;
    label196:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 272)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r4.i);
    _r9.l = ((JAVA_ARRAY_LONG*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r9.l = _r7.l - _r9.l;
    _r11.l = 0;
    _r11.i = _r9.l > _r11.l ? 1 : (_r9.l == _r11.l ? 0 : -1);
    if (_r11.i < 0) goto label210;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 273)
    XMLVM_SOURCE_POSITION("NumberConverter.java", 275)
    _r7.i = 1;
    _r7.i = _r7.i << _r4.i;
    _r5.i = _r5.i + _r7.i;
    _r7 = _r9;
    label210:;
    _r4.i = _r4.i + -1;
    goto label87;
    label213:;
    _r4.i = 0;
    goto label94;
    label215:;
    _r9.i = 0;
    goto label101;
    label217:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 285)
    _r9.l = 10;
    _r7.l = _r7.l * _r9.l;
    _r9.l = 10;
    _r0.l = _r0.l * _r9.l;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 287)
    XMLVM_CHECK_NPE(15)
    _r4.o = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(15)
    _r9.i = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    _r10.i = _r9.i + 1;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r10.i;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r9.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r5.i;
    _r4 = _r7;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 264)
    goto label80;
    label236:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 291)
    if (_r9.i == 0) goto label254;
    if (_r4.i != 0) goto label254;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 292)
    XMLVM_CHECK_NPE(15)
    _r0.o = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(15)
    _r1.i = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r2.i;
    _r2.i = _r5.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    goto label119;
    label254:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 293)
    _r0.i = 1;
    _r0.l = _r7.l << (0x3f & _r0.l);
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i >= 0) goto label273;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 294)
    XMLVM_CHECK_NPE(15)
    _r0.o = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(15)
    _r1.i = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    goto label119;
    label273:;
    XMLVM_SOURCE_POSITION("NumberConverter.java", 296)
    XMLVM_CHECK_NPE(15)
    _r0.o = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
    XMLVM_CHECK_NPE(15)
    _r1.i = ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(15)
    ((org_apache_harmony_luni_util_NumberConverter*) _r15.o)->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = _r2.i;
    _r2.i = _r5.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    goto label119;
    //XMLVM_END_WRAPPER
}

