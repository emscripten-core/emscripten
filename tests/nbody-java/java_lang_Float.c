#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "org_apache_harmony_luni_util_FloatingPointParser.h"
#include "org_apache_harmony_luni_util_NumberConverter.h"

#include "java_lang_Float.h"

#define XMLVM_CURRENT_CLASS_NAME Float
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Float

__TIB_DEFINITION_java_lang_Float __TIB_java_lang_Float = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Float, // classInitializer
    "java.lang.Float", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Number;Ljava/lang/Comparable<Ljava/lang/Float;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Number, // extends
    sizeof(java_lang_Float), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Float;
JAVA_OBJECT __CLASS_java_lang_Float_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Float_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Float_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Float_serialVersionUID;
static JAVA_FLOAT _STATIC_java_lang_Float_MAX_VALUE;
static JAVA_FLOAT _STATIC_java_lang_Float_MIN_VALUE;
static JAVA_FLOAT _STATIC_java_lang_Float_MIN_NORMAL;
static JAVA_FLOAT _STATIC_java_lang_Float_NaN;
static JAVA_FLOAT _STATIC_java_lang_Float_POSITIVE_INFINITY;
static JAVA_FLOAT _STATIC_java_lang_Float_NEGATIVE_INFINITY;
static JAVA_OBJECT _STATIC_java_lang_Float_TYPE;
static JAVA_INT _STATIC_java_lang_Float_MAX_EXPONENT;
static JAVA_INT _STATIC_java_lang_Float_MIN_EXPONENT;
static JAVA_INT _STATIC_java_lang_Float_SIZE;

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

void __INIT_java_lang_Float()
{
    staticInitializerLock(&__TIB_java_lang_Float);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Float.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Float.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Float);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Float.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Float.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Float.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Float")
        __INIT_IMPL_java_lang_Float();
    }
}

void __INIT_IMPL_java_lang_Float()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Number)
    __TIB_java_lang_Float.newInstanceFunc = __NEW_INSTANCE_java_lang_Float;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Float.vtable, __TIB_java_lang_Number.vtable, sizeof(__TIB_java_lang_Number.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Float.vtable[6] = (VTABLE_PTR) &java_lang_Float_byteValue__;
    __TIB_java_lang_Float.vtable[7] = (VTABLE_PTR) &java_lang_Float_doubleValue__;
    __TIB_java_lang_Float.vtable[1] = (VTABLE_PTR) &java_lang_Float_equals___java_lang_Object;
    __TIB_java_lang_Float.vtable[8] = (VTABLE_PTR) &java_lang_Float_floatValue__;
    __TIB_java_lang_Float.vtable[4] = (VTABLE_PTR) &java_lang_Float_hashCode__;
    __TIB_java_lang_Float.vtable[9] = (VTABLE_PTR) &java_lang_Float_intValue__;
    __TIB_java_lang_Float.vtable[10] = (VTABLE_PTR) &java_lang_Float_longValue__;
    __TIB_java_lang_Float.vtable[11] = (VTABLE_PTR) &java_lang_Float_shortValue__;
    __TIB_java_lang_Float.vtable[5] = (VTABLE_PTR) &java_lang_Float_toString__;
    __TIB_java_lang_Float.vtable[12] = (VTABLE_PTR) &java_lang_Float_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Float.numImplementedInterfaces = 2;
    __TIB_java_lang_Float.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Float.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Float.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Float.itableBegin = &__TIB_java_lang_Float.itable[0];
    __TIB_java_lang_Float.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Float.vtable[12];

    _STATIC_java_lang_Float_serialVersionUID = -2671257302660747028;
    _STATIC_java_lang_Float_MAX_VALUE = 3.4028235E38;
    _STATIC_java_lang_Float_MIN_VALUE = 1.4E-45;
    _STATIC_java_lang_Float_MIN_NORMAL = 1.17549435E-38;
    _STATIC_java_lang_Float_NaN = NaN;
    _STATIC_java_lang_Float_POSITIVE_INFINITY = Infinity;
    _STATIC_java_lang_Float_NEGATIVE_INFINITY = -Infinity;
    _STATIC_java_lang_Float_TYPE = (java_lang_Class*) JAVA_NULL;
    _STATIC_java_lang_Float_MAX_EXPONENT = 127;
    _STATIC_java_lang_Float_MIN_EXPONENT = -126;
    _STATIC_java_lang_Float_SIZE = 32;

    __TIB_java_lang_Float.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Float.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Float.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Float.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Float.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Float.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Float.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Float.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Float = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Float);
    __TIB_java_lang_Float.clazz = __CLASS_java_lang_Float;
    __TIB_java_lang_Float.baseType = JAVA_NULL;
    __CLASS_java_lang_Float_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Float);
    __CLASS_java_lang_Float_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Float_1ARRAY);
    __CLASS_java_lang_Float_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Float_2ARRAY);
    java_lang_Float___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Float]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Float.classInitialized = 1;
}

void __DELETE_java_lang_Float(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Float]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Float(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Float*) me)->fields.java_lang_Float.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Float]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Float()
{    XMLVM_CLASS_INIT(java_lang_Float)
java_lang_Float* me = (java_lang_Float*) XMLVM_MALLOC(sizeof(java_lang_Float));
    me->tib = &__TIB_java_lang_Float;
    __INIT_INSTANCE_MEMBERS_java_lang_Float(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Float]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Float()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Float_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_serialVersionUID;
}

void java_lang_Float_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_serialVersionUID = v;
}

JAVA_FLOAT java_lang_Float_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_MAX_VALUE;
}

void java_lang_Float_PUT_MAX_VALUE(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_MAX_VALUE = v;
}

JAVA_FLOAT java_lang_Float_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_MIN_VALUE;
}

void java_lang_Float_PUT_MIN_VALUE(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_MIN_VALUE = v;
}

JAVA_FLOAT java_lang_Float_GET_MIN_NORMAL()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_MIN_NORMAL;
}

void java_lang_Float_PUT_MIN_NORMAL(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_MIN_NORMAL = v;
}

JAVA_FLOAT java_lang_Float_GET_NaN()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_NaN;
}

void java_lang_Float_PUT_NaN(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_NaN = v;
}

JAVA_FLOAT java_lang_Float_GET_POSITIVE_INFINITY()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_POSITIVE_INFINITY;
}

void java_lang_Float_PUT_POSITIVE_INFINITY(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_POSITIVE_INFINITY = v;
}

JAVA_FLOAT java_lang_Float_GET_NEGATIVE_INFINITY()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_NEGATIVE_INFINITY;
}

void java_lang_Float_PUT_NEGATIVE_INFINITY(JAVA_FLOAT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_NEGATIVE_INFINITY = v;
}

JAVA_OBJECT java_lang_Float_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_TYPE;
}

void java_lang_Float_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_TYPE = v;
}

JAVA_INT java_lang_Float_GET_MAX_EXPONENT()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_MAX_EXPONENT;
}

void java_lang_Float_PUT_MAX_EXPONENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_MAX_EXPONENT = v;
}

JAVA_INT java_lang_Float_GET_MIN_EXPONENT()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_MIN_EXPONENT;
}

void java_lang_Float_PUT_MIN_EXPONENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_MIN_EXPONENT = v;
}

JAVA_INT java_lang_Float_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Float)
    return _STATIC_java_lang_Float_SIZE;
}

void java_lang_Float_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Float)
_STATIC_java_lang_Float_SIZE = v;
}

void java_lang_Float___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Float", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Float.java", 75)
    _r0.i = 0;
    XMLVM_CLASS_INIT(float)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_float, _r0.i);
    XMLVM_SOURCE_POSITION("Float.java", 76)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Float_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Float.java", 26)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Float___INIT____float(JAVA_OBJECT me, JAVA_FLOAT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float___INIT____float]
    XMLVM_ENTER_METHOD("java.lang.Float", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Float.java", 111)
    XMLVM_CHECK_NPE(0)
    java_lang_Number___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Float.java", 112)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Float*) _r0.o)->fields.java_lang_Float.value_ = _r1.f;
    XMLVM_SOURCE_POSITION("Float.java", 113)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Float___INIT____double(JAVA_OBJECT me, JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float___INIT____double]
    XMLVM_ENTER_METHOD("java.lang.Float", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.d = n1;
    XMLVM_SOURCE_POSITION("Float.java", 121)
    XMLVM_CHECK_NPE(1)
    java_lang_Number___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Float.java", 122)
    _r0.f = (JAVA_FLOAT) _r2.d;
    XMLVM_CHECK_NPE(1)
    ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_ = _r0.f;
    XMLVM_SOURCE_POSITION("Float.java", 123)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Float___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Float", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 135)
    _r0.f = java_lang_Float_parseFloat___java_lang_String(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_Float___INIT____float(_r1.o, _r0.f);
    XMLVM_SOURCE_POSITION("Float.java", 136)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Float_compareTo___java_lang_Float(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_compareTo___java_lang_Float]
    XMLVM_ENTER_METHOD("java.lang.Float", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 157)
    XMLVM_CHECK_NPE(2)
    _r0.f = ((java_lang_Float*) _r2.o)->fields.java_lang_Float.value_;
    XMLVM_CHECK_NPE(3)
    _r1.f = ((java_lang_Float*) _r3.o)->fields.java_lang_Float.value_;
    _r0.i = java_lang_Float_compare___float_float(_r0.f, _r1.f);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Float_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 162)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = (JAVA_INT) _r0.f;
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Float_doubleValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_doubleValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "doubleValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 167)
    XMLVM_CHECK_NPE(2)
    _r0.f = ((java_lang_Float*) _r2.o)->fields.java_lang_Float.value_;
    _r0.d = (JAVA_DOUBLE) _r0.f;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Float_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Float", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 182)
    if (_r3.o == _r2.o) goto label24;
    XMLVM_SOURCE_POSITION("Float.java", 183)
    XMLVM_CLASS_INIT(java_lang_Float)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_lang_Float);
    if (_r0.i == 0) goto label22;
    XMLVM_SOURCE_POSITION("Float.java", 184)
    XMLVM_CHECK_NPE(2)
    _r0.f = ((java_lang_Float*) _r2.o)->fields.java_lang_Float.value_;
    _r0.i = java_lang_Float_floatToIntBits___float(_r0.f);
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(3)
    _r1.f = ((java_lang_Float*) _r3.o)->fields.java_lang_Float.value_;
    _r1.i = java_lang_Float_floatToIntBits___float(_r1.f);
    if (_r0.i == _r1.i) goto label24;
    label22:;
    _r0.i = 0;
    label23:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label24:;
    _r0.i = 1;
    goto label23;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_Float_floatToIntBits___float(JAVA_FLOAT n1)]

//XMLVM_NATIVE[JAVA_INT java_lang_Float_floatToRawIntBits___float(JAVA_FLOAT n1)]

JAVA_FLOAT java_lang_Float_floatValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_floatValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "floatValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 223)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Float_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Float", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 228)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = java_lang_Float_floatToIntBits___float(_r0.f);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_FLOAT java_lang_Float_intBitsToFloat___int(JAVA_INT n1)]

JAVA_INT java_lang_Float_intValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_intValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "intValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 246)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = (JAVA_INT) _r0.f;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Float_isInfinite__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_isInfinite__]
    XMLVM_ENTER_METHOD("java.lang.Float", "isInfinite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 256)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = java_lang_Float_isInfinite___float(_r0.f);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Float_isInfinite___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_isInfinite___float]
    XMLVM_ENTER_METHOD("java.lang.Float", "isInfinite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Float.java", 268)
    _r0.f = Infinity;
    _r0.i = _r1.f > _r0.f ? 1 : (_r1.f == _r0.f ? 0 : -1);
    if (_r0.i == 0) goto label14;
    _r0.f = -Infinity;
    _r0.i = _r1.f > _r0.f ? 1 : (_r1.f == _r0.f ? 0 : -1);
    if (_r0.i == 0) goto label14;
    _r0.i = 0;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 1;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Float_isNaN__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_isNaN__]
    XMLVM_ENTER_METHOD("java.lang.Float", "isNaN", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 278)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = java_lang_Float_isNaN___float(_r0.f);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Float_isNaN___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_isNaN___float]
    XMLVM_ENTER_METHOD("java.lang.Float", "isNaN", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Float.java", 291)
    _r0.i = _r1.f > _r1.f ? 1 : (_r1.f == _r1.f ? 0 : -1);
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Float_longValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_longValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "longValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 296)
    XMLVM_CHECK_NPE(2)
    _r0.f = ((java_lang_Float*) _r2.o)->fields.java_lang_Float.value_;
    _r0.l = (JAVA_LONG) _r0.f;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Float_parseFloat___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_parseFloat___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Float", "parseFloat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 313)
    _r0.f = org_apache_harmony_luni_util_FloatingPointParser_parseFloat___java_lang_String(_r1.o);
    XMLVM_SOURCE_POSITION("Float.java", 312)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Float_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Float", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 318)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.i = (JAVA_INT) _r0.f;
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Float_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_toString__]
    XMLVM_ENTER_METHOD("java.lang.Float", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Float.java", 323)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_lang_Float*) _r1.o)->fields.java_lang_Float.value_;
    _r0.o = java_lang_Float_toString___float(_r0.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Float_toString___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_toString___float]
    XMLVM_ENTER_METHOD("java.lang.Float", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Float.java", 335)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_convert___float(_r1.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Float_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Float", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 351)
    _r0.f = java_lang_Float_parseFloat___java_lang_String(_r1.o);
    _r0.o = java_lang_Float_valueOf___float(_r0.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Float_compare___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_compare___float_float]
    XMLVM_ENTER_METHOD("java.lang.Float", "compare", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.f = n1;
    _r5.f = n2;
    _r3.i = 1;
    _r2.i = 0;
    _r1.i = -1;
    XMLVM_SOURCE_POSITION("Float.java", 373)
    _r0.i = _r4.f > _r5.f ? 1 : (_r4.f == _r5.f ? 0 : -1);
    if (_r0.i <= 0) goto label9;
    _r0 = _r3;
    label8:;
    XMLVM_SOURCE_POSITION("Float.java", 374)
    XMLVM_SOURCE_POSITION("Float.java", 399)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("Float.java", 376)
    _r0.i = _r5.f > _r4.f ? 1 : (_r5.f == _r4.f ? 0 : -1);
    if (_r0.i <= 0) goto label15;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Float.java", 377)
    goto label8;
    label15:;
    XMLVM_SOURCE_POSITION("Float.java", 379)
    _r0.i = _r4.f > _r5.f ? 1 : (_r4.f == _r5.f ? 0 : -1);
    if (_r0.i != 0) goto label26;
    _r0.f = 0.0;
    _r0.i = _r0.f > _r4.f ? 1 : (_r0.f == _r4.f ? 0 : -1);
    if (_r0.i == 0) goto label26;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Float.java", 380)
    goto label8;
    label26:;
    XMLVM_SOURCE_POSITION("Float.java", 384)
    _r0.i = java_lang_Float_isNaN___float(_r4.f);
    if (_r0.i == 0) goto label42;
    XMLVM_SOURCE_POSITION("Float.java", 385)
    _r0.i = java_lang_Float_isNaN___float(_r5.f);
    if (_r0.i == 0) goto label40;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Float.java", 386)
    goto label8;
    label40:;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Float.java", 388)
    goto label8;
    label42:;
    XMLVM_SOURCE_POSITION("Float.java", 389)
    _r0.i = java_lang_Float_isNaN___float(_r5.f);
    if (_r0.i == 0) goto label50;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Float.java", 390)
    goto label8;
    label50:;
    XMLVM_SOURCE_POSITION("Float.java", 394)
    _r0.i = java_lang_Float_floatToRawIntBits___float(_r4.f);
    XMLVM_SOURCE_POSITION("Float.java", 395)
    _r1.i = java_lang_Float_floatToRawIntBits___float(_r5.f);
    _r0.i = _r0.i >> 31;
    _r1.i = _r1.i >> 31;
    _r0.i = _r0.i - _r1.i;
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Float_valueOf___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_valueOf___float]
    XMLVM_ENTER_METHOD("java.lang.Float", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Float.java", 411)
    _r0.o = __NEW_java_lang_Float();
    XMLVM_CHECK_NPE(0)
    java_lang_Float___INIT____float(_r0.o, _r1.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Float_toHexString___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Float)
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_toHexString___float]
    XMLVM_ENTER_METHOD("java.lang.Float", "toHexString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.f = n1;
    _r6.i = 48;
    _r5.i = 6;
    XMLVM_SOURCE_POSITION("Float.java", 426)
    _r0.i = _r7.f > _r7.f ? 1 : (_r7.f == _r7.f ? 0 : -1);
    if (_r0.i == 0) goto label10;
    XMLVM_SOURCE_POSITION("Float.java", 427)
    // "NaN"
    _r0.o = xmlvm_create_java_string_from_pool(8);
    label9:;
    XMLVM_SOURCE_POSITION("Float.java", 503)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;
    XMLVM_SOURCE_POSITION("Float.java", 429)
    _r0.f = Infinity;
    _r0.i = _r7.f > _r0.f ? 1 : (_r7.f == _r0.f ? 0 : -1);
    if (_r0.i != 0) goto label19;
    XMLVM_SOURCE_POSITION("Float.java", 430)
    // "Infinity"
    _r0.o = xmlvm_create_java_string_from_pool(9);
    goto label9;
    label19:;
    XMLVM_SOURCE_POSITION("Float.java", 432)
    _r0.f = -Infinity;
    _r0.i = _r7.f > _r0.f ? 1 : (_r7.f == _r0.f ? 0 : -1);
    if (_r0.i != 0) goto label28;
    XMLVM_SOURCE_POSITION("Float.java", 433)
    // "-Infinity"
    _r0.o = xmlvm_create_java_string_from_pool(10);
    goto label9;
    label28:;
    XMLVM_SOURCE_POSITION("Float.java", 436)
    _r0.i = java_lang_Float_floatToIntBits___float(_r7.f);
    XMLVM_SOURCE_POSITION("Float.java", 438)
    _r1.i = -2147483648;
    _r1.i = _r1.i & _r0.i;
    if (_r1.i == 0) goto label58;
    _r1.i = 1;
    label38:;
    XMLVM_SOURCE_POSITION("Float.java", 440)
    _r2.i = 2139095040;
    _r2.i = _r2.i & _r0.i;
    _r2.i = ((JAVA_UINT) _r2.i) >> (0x1f & ((JAVA_UINT) 23));
    _r3.i = 8388607;
    _r0.i = _r0.i & _r3.i;
    _r0.i = _r0.i << 1;
    if (_r2.i != 0) goto label63;
    XMLVM_SOURCE_POSITION("Float.java", 445)
    if (_r0.i != 0) goto label63;
    XMLVM_SOURCE_POSITION("Float.java", 446)
    if (_r1.i == 0) goto label60;
    // "-0x0.0p0"
    _r0.o = xmlvm_create_java_string_from_pool(11);
    goto label9;
    label58:;
    _r1.i = 0;
    goto label38;
    label60:;
    // "0x0.0p0"
    _r0.o = xmlvm_create_java_string_from_pool(12);
    goto label9;
    label63:;
    XMLVM_SOURCE_POSITION("Float.java", 449)
    _r3.o = __NEW_java_lang_StringBuilder();
    _r4.i = 10;
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____int(_r3.o, _r4.i);
    XMLVM_SOURCE_POSITION("Float.java", 450)
    if (_r1.i == 0) goto label126;
    XMLVM_SOURCE_POSITION("Float.java", 451)
    // "-0x"
    _r1.o = xmlvm_create_java_string_from_pool(13);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r1.o);
    label77:;
    XMLVM_SOURCE_POSITION("Float.java", 456)
    if (_r2.i != 0) goto label142;
    XMLVM_SOURCE_POSITION("Float.java", 457)
    // "0."
    _r1.o = xmlvm_create_java_string_from_pool(14);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r1.o);
    _r1 = _r0;
    _r0 = _r5;
    label86:;
    XMLVM_SOURCE_POSITION("Float.java", 462)
    if (_r1.i == 0) goto label92;
    _r2.i = _r1.i & 15;
    if (_r2.i == 0) goto label132;
    label92:;
    XMLVM_SOURCE_POSITION("Float.java", 467)
    _r2.o = java_lang_Integer_toHexString___int(_r1.i);
    XMLVM_SOURCE_POSITION("Float.java", 470)
    if (_r1.i == 0) goto label113;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r0.i <= _r1.i) goto label113;
    XMLVM_SOURCE_POSITION("Float.java", 471)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    _r0.i = _r0.i - _r1.i;
    label109:;
    XMLVM_SOURCE_POSITION("Float.java", 472)
    _r1.i = _r0.i + -1;
    if (_r0.i != 0) goto label137;
    label113:;
    XMLVM_SOURCE_POSITION("Float.java", 476)
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r2.o);
    XMLVM_SOURCE_POSITION("Float.java", 477)
    // "p-126"
    _r0.o = xmlvm_create_java_string_from_pool(15);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    label121:;
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    goto label9;
    label126:;
    XMLVM_SOURCE_POSITION("Float.java", 453)
    // "0x"
    _r1.o = xmlvm_create_java_string_from_pool(16);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r1.o);
    goto label77;
    label132:;
    XMLVM_SOURCE_POSITION("Float.java", 463)
    _r1.i = ((JAVA_UINT) _r1.i) >> (0x1f & ((JAVA_UINT) 4));
    _r0.i = _r0.i + -1;
    XMLVM_SOURCE_POSITION("Float.java", 464)
    goto label86;
    label137:;
    XMLVM_SOURCE_POSITION("Float.java", 473)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r6.i);
    _r0 = _r1;
    goto label109;
    label142:;
    XMLVM_SOURCE_POSITION("Float.java", 479)
    // "1."
    _r1.o = xmlvm_create_java_string_from_pool(17);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r1.o);
    _r1 = _r0;
    _r0 = _r5;
    label149:;
    XMLVM_SOURCE_POSITION("Float.java", 484)
    if (_r1.i == 0) goto label155;
    _r4.i = _r1.i & 15;
    if (_r4.i == 0) goto label196;
    label155:;
    XMLVM_SOURCE_POSITION("Float.java", 489)
    _r4.o = java_lang_Integer_toHexString___int(_r1.i);
    XMLVM_SOURCE_POSITION("Float.java", 492)
    if (_r1.i == 0) goto label176;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    if (_r0.i <= _r1.i) goto label176;
    XMLVM_SOURCE_POSITION("Float.java", 493)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    _r0.i = _r0.i - _r1.i;
    label172:;
    XMLVM_SOURCE_POSITION("Float.java", 494)
    _r1.i = _r0.i + -1;
    if (_r0.i != 0) goto label201;
    label176:;
    XMLVM_SOURCE_POSITION("Float.java", 498)
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    XMLVM_SOURCE_POSITION("Float.java", 499)
    _r0.i = 112;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Float.java", 501)
    _r0.i = 127;
    _r0.i = _r2.i - _r0.i;
    _r0.o = java_lang_Integer_toString___int(_r0.i);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    goto label121;
    label196:;
    XMLVM_SOURCE_POSITION("Float.java", 485)
    _r1.i = ((JAVA_UINT) _r1.i) >> (0x1f & ((JAVA_UINT) 4));
    _r0.i = _r0.i + -1;
    XMLVM_SOURCE_POSITION("Float.java", 486)
    goto label149;
    label201:;
    XMLVM_SOURCE_POSITION("Float.java", 495)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r6.i);
    _r0 = _r1;
    goto label172;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Float_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Float_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Float", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Float.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Float_compareTo___java_lang_Float(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

