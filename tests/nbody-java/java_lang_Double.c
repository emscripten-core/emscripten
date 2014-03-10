#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Long.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "org_apache_harmony_luni_util_FloatingPointParser.h"
#include "org_apache_harmony_luni_util_NumberConverter.h"

#include "java_lang_Double.h"

#define XMLVM_CURRENT_CLASS_NAME Double
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Double

__TIB_DEFINITION_java_lang_Double __TIB_java_lang_Double = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Double, // classInitializer
    "java.lang.Double", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Number;Ljava/lang/Comparable<Ljava/lang/Double;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Number, // extends
    sizeof(java_lang_Double), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Double;
JAVA_OBJECT __CLASS_java_lang_Double_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Double_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Double_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Double_serialVersionUID;
static JAVA_DOUBLE _STATIC_java_lang_Double_MAX_VALUE;
static JAVA_DOUBLE _STATIC_java_lang_Double_MIN_VALUE;
static JAVA_DOUBLE _STATIC_java_lang_Double_MIN_NORMAL;
static JAVA_DOUBLE _STATIC_java_lang_Double_NaN;
static JAVA_DOUBLE _STATIC_java_lang_Double_POSITIVE_INFINITY;
static JAVA_INT _STATIC_java_lang_Double_MAX_EXPONENT;
static JAVA_INT _STATIC_java_lang_Double_MIN_EXPONENT;
static JAVA_DOUBLE _STATIC_java_lang_Double_NEGATIVE_INFINITY;
static JAVA_OBJECT _STATIC_java_lang_Double_TYPE;
static JAVA_INT _STATIC_java_lang_Double_SIZE;

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

void __INIT_java_lang_Double()
{
    staticInitializerLock(&__TIB_java_lang_Double);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Double.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Double.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Double);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Double.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Double.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Double.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Double")
        __INIT_IMPL_java_lang_Double();
    }
}

void __INIT_IMPL_java_lang_Double()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Number)
    __TIB_java_lang_Double.newInstanceFunc = __NEW_INSTANCE_java_lang_Double;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Double.vtable, __TIB_java_lang_Number.vtable, sizeof(__TIB_java_lang_Number.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Double.vtable[6] = (VTABLE_PTR) &java_lang_Double_byteValue__;
    __TIB_java_lang_Double.vtable[7] = (VTABLE_PTR) &java_lang_Double_doubleValue__;
    __TIB_java_lang_Double.vtable[1] = (VTABLE_PTR) &java_lang_Double_equals___java_lang_Object;
    __TIB_java_lang_Double.vtable[8] = (VTABLE_PTR) &java_lang_Double_floatValue__;
    __TIB_java_lang_Double.vtable[4] = (VTABLE_PTR) &java_lang_Double_hashCode__;
    __TIB_java_lang_Double.vtable[9] = (VTABLE_PTR) &java_lang_Double_intValue__;
    __TIB_java_lang_Double.vtable[10] = (VTABLE_PTR) &java_lang_Double_longValue__;
    __TIB_java_lang_Double.vtable[11] = (VTABLE_PTR) &java_lang_Double_shortValue__;
    __TIB_java_lang_Double.vtable[5] = (VTABLE_PTR) &java_lang_Double_toString__;
    __TIB_java_lang_Double.vtable[12] = (VTABLE_PTR) &java_lang_Double_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Double.numImplementedInterfaces = 2;
    __TIB_java_lang_Double.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Double.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Double.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Double.itableBegin = &__TIB_java_lang_Double.itable[0];
    __TIB_java_lang_Double.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Double.vtable[12];

    _STATIC_java_lang_Double_serialVersionUID = -9172774392245257468;
    _STATIC_java_lang_Double_MAX_VALUE = 1.7976931348623157E308;
    _STATIC_java_lang_Double_MIN_VALUE = 4.9E-324;
    _STATIC_java_lang_Double_MIN_NORMAL = 2.2250738585072014E-308;
    _STATIC_java_lang_Double_NaN = NaN;
    _STATIC_java_lang_Double_POSITIVE_INFINITY = Infinity;
    _STATIC_java_lang_Double_MAX_EXPONENT = 1023;
    _STATIC_java_lang_Double_MIN_EXPONENT = -1022;
    _STATIC_java_lang_Double_NEGATIVE_INFINITY = -Infinity;
    _STATIC_java_lang_Double_TYPE = (java_lang_Class*) JAVA_NULL;
    _STATIC_java_lang_Double_SIZE = 64;

    __TIB_java_lang_Double.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Double.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Double.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Double.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Double.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Double.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Double.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Double.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Double = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Double);
    __TIB_java_lang_Double.clazz = __CLASS_java_lang_Double;
    __TIB_java_lang_Double.baseType = JAVA_NULL;
    __CLASS_java_lang_Double_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Double);
    __CLASS_java_lang_Double_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Double_1ARRAY);
    __CLASS_java_lang_Double_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Double_2ARRAY);
    java_lang_Double___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Double]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Double.classInitialized = 1;
}

void __DELETE_java_lang_Double(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Double]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Double(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Double*) me)->fields.java_lang_Double.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Double]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Double()
{    XMLVM_CLASS_INIT(java_lang_Double)
java_lang_Double* me = (java_lang_Double*) XMLVM_MALLOC(sizeof(java_lang_Double));
    me->tib = &__TIB_java_lang_Double;
    __INIT_INSTANCE_MEMBERS_java_lang_Double(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Double]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Double()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Double_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_serialVersionUID;
}

void java_lang_Double_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_serialVersionUID = v;
}

JAVA_DOUBLE java_lang_Double_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_MAX_VALUE;
}

void java_lang_Double_PUT_MAX_VALUE(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_MAX_VALUE = v;
}

JAVA_DOUBLE java_lang_Double_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_MIN_VALUE;
}

void java_lang_Double_PUT_MIN_VALUE(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_MIN_VALUE = v;
}

JAVA_DOUBLE java_lang_Double_GET_MIN_NORMAL()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_MIN_NORMAL;
}

void java_lang_Double_PUT_MIN_NORMAL(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_MIN_NORMAL = v;
}

JAVA_DOUBLE java_lang_Double_GET_NaN()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_NaN;
}

void java_lang_Double_PUT_NaN(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_NaN = v;
}

JAVA_DOUBLE java_lang_Double_GET_POSITIVE_INFINITY()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_POSITIVE_INFINITY;
}

void java_lang_Double_PUT_POSITIVE_INFINITY(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_POSITIVE_INFINITY = v;
}

JAVA_INT java_lang_Double_GET_MAX_EXPONENT()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_MAX_EXPONENT;
}

void java_lang_Double_PUT_MAX_EXPONENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_MAX_EXPONENT = v;
}

JAVA_INT java_lang_Double_GET_MIN_EXPONENT()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_MIN_EXPONENT;
}

void java_lang_Double_PUT_MIN_EXPONENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_MIN_EXPONENT = v;
}

JAVA_DOUBLE java_lang_Double_GET_NEGATIVE_INFINITY()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_NEGATIVE_INFINITY;
}

void java_lang_Double_PUT_NEGATIVE_INFINITY(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_NEGATIVE_INFINITY = v;
}

JAVA_OBJECT java_lang_Double_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_TYPE;
}

void java_lang_Double_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_TYPE = v;
}

JAVA_INT java_lang_Double_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Double)
    return _STATIC_java_lang_Double_SIZE;
}

void java_lang_Double_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Double)
_STATIC_java_lang_Double_SIZE = v;
}

void java_lang_Double___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Double", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Double.java", 95)
    _r0.i = 0;
    XMLVM_CLASS_INIT(double)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_double, _r0.i);
    XMLVM_SOURCE_POSITION("Double.java", 96)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Double_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Double.java", 26)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Double___INIT____double(JAVA_OBJECT me, JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double___INIT____double]
    XMLVM_ENTER_METHOD("java.lang.Double", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 116)
    XMLVM_CHECK_NPE(0)
    java_lang_Number___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Double.java", 117)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Double*) _r0.o)->fields.java_lang_Double.value_ = _r1.d;
    XMLVM_SOURCE_POSITION("Double.java", 118)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Double___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Double", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 130)
    _r0.d = java_lang_Double_parseDouble___java_lang_String(_r3.o);
    XMLVM_CHECK_NPE(2)
    java_lang_Double___INIT____double(_r2.o, _r0.d);
    XMLVM_SOURCE_POSITION("Double.java", 131)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Double_compareTo___java_lang_Double(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_compareTo___java_lang_Double]
    XMLVM_ENTER_METHOD("java.lang.Double", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 154)
    XMLVM_CHECK_NPE(4)
    _r0.d = ((java_lang_Double*) _r4.o)->fields.java_lang_Double.value_;
    XMLVM_CHECK_NPE(5)
    _r2.d = ((java_lang_Double*) _r5.o)->fields.java_lang_Double.value_;
    _r0.i = java_lang_Double_compare___double_double(_r0.d, _r2.d);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Double_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 159)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.i = (JAVA_INT) _r0.d;
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG java_lang_Double_doubleToLongBits___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_LONG java_lang_Double_doubleToRawLongBits___double(JAVA_DOUBLE n1)]

JAVA_DOUBLE java_lang_Double_doubleValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_doubleValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "doubleValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 198)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Double_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Double", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 214)
    if (_r5.o == _r4.o) goto label26;
    XMLVM_SOURCE_POSITION("Double.java", 215)
    XMLVM_CLASS_INIT(java_lang_Double)
    _r0.i = XMLVM_ISA(_r5.o, __CLASS_java_lang_Double);
    if (_r0.i == 0) goto label24;
    XMLVM_SOURCE_POSITION("Double.java", 216)
    XMLVM_CHECK_NPE(4)
    _r0.d = ((java_lang_Double*) _r4.o)->fields.java_lang_Double.value_;
    _r0.l = java_lang_Double_doubleToLongBits___double(_r0.d);
    _r5.o = _r5.o;
    XMLVM_CHECK_NPE(5)
    _r2.d = ((java_lang_Double*) _r5.o)->fields.java_lang_Double.value_;
    _r2.l = java_lang_Double_doubleToLongBits___double(_r2.d);
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i == 0) goto label26;
    label24:;
    _r0.i = 0;
    label25:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label26:;
    _r0.i = 1;
    goto label25;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Double_floatValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_floatValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "floatValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 221)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.f = (JAVA_FLOAT) _r0.d;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Double_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Double", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 226)
    XMLVM_CHECK_NPE(4)
    _r0.d = ((java_lang_Double*) _r4.o)->fields.java_lang_Double.value_;
    _r0.l = java_lang_Double_doubleToLongBits___double(_r0.d);
    XMLVM_SOURCE_POSITION("Double.java", 227)
    _r2.i = 32;
    _r2.l = ((JAVA_ULONG) _r0.l) >> (0x3f & ((JAVA_ULONG) _r2.l));
    _r0.l = _r0.l ^ _r2.l;
    _r0.i = (JAVA_INT) _r0.l;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Double_intValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_intValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "intValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 232)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.i = (JAVA_INT) _r0.d;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Double_isInfinite__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_isInfinite__]
    XMLVM_ENTER_METHOD("java.lang.Double", "isInfinite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 242)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.i = java_lang_Double_isInfinite___double(_r0.d);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Double_isInfinite___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_isInfinite___double]
    XMLVM_ENTER_METHOD("java.lang.Double", "isInfinite", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 254)
    _r0.d = Infinity;
    _r0.i = _r2.d > _r0.d ? 1 : (_r2.d == _r0.d ? 0 : -1);
    if (_r0.i == 0) goto label14;
    _r0.d = -Infinity;
    _r0.i = _r2.d > _r0.d ? 1 : (_r2.d == _r0.d ? 0 : -1);
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

JAVA_BOOLEAN java_lang_Double_isNaN__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_isNaN__]
    XMLVM_ENTER_METHOD("java.lang.Double", "isNaN", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 264)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.i = java_lang_Double_isNaN___double(_r0.d);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Double_isNaN___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_isNaN___double]
    XMLVM_ENTER_METHOD("java.lang.Double", "isNaN", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 277)
    _r0.i = _r1.d > _r1.d ? 1 : (_r1.d == _r1.d ? 0 : -1);
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

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Double_longBitsToDouble___long(JAVA_LONG n1)]

JAVA_LONG java_lang_Double_longValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_longValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "longValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 295)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.l = (JAVA_LONG) _r0.d;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Double_parseDouble___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_parseDouble___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Double", "parseDouble", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 311)
    _r0.d = org_apache_harmony_luni_util_FloatingPointParser_parseDouble___java_lang_String(_r2.o);
    XMLVM_SOURCE_POSITION("Double.java", 310)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Double_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Double", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 316)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.i = (JAVA_INT) _r0.d;
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Double_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_toString__]
    XMLVM_ENTER_METHOD("java.lang.Double", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Double.java", 321)
    XMLVM_CHECK_NPE(2)
    _r0.d = ((java_lang_Double*) _r2.o)->fields.java_lang_Double.value_;
    _r0.o = java_lang_Double_toString___double(_r0.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Double_toString___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_toString___double]
    XMLVM_ENTER_METHOD("java.lang.Double", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 333)
    _r0.o = org_apache_harmony_luni_util_NumberConverter_convert___double(_r1.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Double_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Double", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 349)
    _r0.o = __NEW_java_lang_Double();
    _r1.d = java_lang_Double_parseDouble___java_lang_String(_r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_Double___INIT____double(_r0.o, _r1.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Double_compare___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_compare___double_double]
    XMLVM_ENTER_METHOD("java.lang.Double", "compare", "?")
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
    _r6.d = n1;
    _r8.d = n2;
    _r5.i = 63;
    _r4.i = 1;
    _r3.i = 0;
    _r2.i = -1;
    XMLVM_SOURCE_POSITION("Double.java", 370)
    _r0.i = _r6.d > _r8.d ? 1 : (_r6.d == _r8.d ? 0 : -1);
    if (_r0.i <= 0) goto label11;
    _r0 = _r4;
    label10:;
    XMLVM_SOURCE_POSITION("Double.java", 371)
    XMLVM_SOURCE_POSITION("Double.java", 395)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("Double.java", 373)
    _r0.i = _r8.d > _r6.d ? 1 : (_r8.d == _r6.d ? 0 : -1);
    if (_r0.i <= 0) goto label17;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Double.java", 374)
    goto label10;
    label17:;
    XMLVM_SOURCE_POSITION("Double.java", 376)
    _r0.i = _r6.d > _r8.d ? 1 : (_r6.d == _r8.d ? 0 : -1);
    if (_r0.i != 0) goto label29;
    _r0.d = 0.0;
    _r0.i = _r0.d > _r6.d ? 1 : (_r0.d == _r6.d ? 0 : -1);
    if (_r0.i == 0) goto label29;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Double.java", 377)
    goto label10;
    label29:;
    XMLVM_SOURCE_POSITION("Double.java", 381)
    _r0.i = java_lang_Double_isNaN___double(_r6.d);
    if (_r0.i == 0) goto label45;
    XMLVM_SOURCE_POSITION("Double.java", 382)
    _r0.i = java_lang_Double_isNaN___double(_r8.d);
    if (_r0.i == 0) goto label43;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Double.java", 383)
    goto label10;
    label43:;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("Double.java", 385)
    goto label10;
    label45:;
    XMLVM_SOURCE_POSITION("Double.java", 386)
    _r0.i = java_lang_Double_isNaN___double(_r8.d);
    if (_r0.i == 0) goto label53;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Double.java", 387)
    goto label10;
    label53:;
    XMLVM_SOURCE_POSITION("Double.java", 391)
    _r0.l = java_lang_Double_doubleToRawLongBits___double(_r6.d);
    XMLVM_SOURCE_POSITION("Double.java", 392)
    _r2.l = java_lang_Double_doubleToRawLongBits___double(_r8.d);
    _r0.l = _r0.l >> (0x3f & _r5.l);
    _r2.l = _r2.l >> (0x3f & _r5.l);
    _r0.l = _r0.l - _r2.l;
    _r0.i = (JAVA_INT) _r0.l;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Double_valueOf___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_valueOf___double]
    XMLVM_ENTER_METHOD("java.lang.Double", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 407)
    _r0.o = __NEW_java_lang_Double();
    XMLVM_CHECK_NPE(0)
    java_lang_Double___INIT____double(_r0.o, _r1.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Double_toHexString___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Double)
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_toHexString___double]
    XMLVM_ENTER_METHOD("java.lang.Double", "toHexString", "?")
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
    _r11.d = n1;
    XMLVM_SOURCE_POSITION("Double.java", 422)
    _r0.i = _r11.d > _r11.d ? 1 : (_r11.d == _r11.d ? 0 : -1);
    if (_r0.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("Double.java", 423)
    // "NaN"
    _r11.o = xmlvm_create_java_string_from_pool(8);
    label6:;
    XMLVM_SOURCE_POSITION("Double.java", 499)
    XMLVM_EXIT_METHOD()
    return _r11.o;
    label7:;
    XMLVM_SOURCE_POSITION("Double.java", 425)
    _r0.d = Infinity;
    _r0.i = _r11.d > _r0.d ? 1 : (_r11.d == _r0.d ? 0 : -1);
    if (_r0.i != 0) goto label16;
    XMLVM_SOURCE_POSITION("Double.java", 426)
    // "Infinity"
    _r11.o = xmlvm_create_java_string_from_pool(9);
    goto label6;
    label16:;
    XMLVM_SOURCE_POSITION("Double.java", 428)
    _r0.d = -Infinity;
    _r0.i = _r11.d > _r0.d ? 1 : (_r11.d == _r0.d ? 0 : -1);
    if (_r0.i != 0) goto label25;
    XMLVM_SOURCE_POSITION("Double.java", 429)
    // "-Infinity"
    _r11.o = xmlvm_create_java_string_from_pool(10);
    goto label6;
    label25:;
    XMLVM_SOURCE_POSITION("Double.java", 432)
    _r11.l = java_lang_Double_doubleToLongBits___double(_r11.d);
    XMLVM_SOURCE_POSITION("Double.java", 434)
    _r0.l = -9223372036854775808;
    _r0.l = _r0.l & _r11.l;
    _r2.l = 0;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i == 0) goto label68;
    _r0.i = 1;
    label39:;
    XMLVM_SOURCE_POSITION("Double.java", 436)
    _r1.l = 9218868437227405312;
    _r1.l = _r1.l & _r11.l;
    _r3.i = 52;
    _r1.l = ((JAVA_ULONG) _r1.l) >> (0x3f & ((JAVA_ULONG) _r3.l));
    _r3.l = 4503599627370495;
    _r11.l = _r11.l & _r3.l;
    _r3.l = 0;
    _r3.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r3.i != 0) goto label73;
    XMLVM_SOURCE_POSITION("Double.java", 440)
    _r3.l = 0;
    _r3.i = _r11.l > _r3.l ? 1 : (_r11.l == _r3.l ? 0 : -1);
    if (_r3.i != 0) goto label73;
    XMLVM_SOURCE_POSITION("Double.java", 441)
    if (_r0.i == 0) goto label70;
    // "-0x0.0p0"
    _r11.o = xmlvm_create_java_string_from_pool(11);
    goto label6;
    label68:;
    _r0.i = 0;
    goto label39;
    label70:;
    // "0x0.0p0"
    _r11.o = xmlvm_create_java_string_from_pool(12);
    goto label6;
    label73:;
    XMLVM_SOURCE_POSITION("Double.java", 444)
    _r3.o = __NEW_java_lang_StringBuilder();
    _r4.i = 10;
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____int(_r3.o, _r4.i);
    XMLVM_SOURCE_POSITION("Double.java", 445)
    if (_r0.i == 0) goto label157;
    XMLVM_SOURCE_POSITION("Double.java", 446)
    // "-0x"
    _r0.o = xmlvm_create_java_string_from_pool(13);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    label87:;
    XMLVM_SOURCE_POSITION("Double.java", 451)
    _r4.l = 0;
    _r0.i = _r1.l > _r4.l ? 1 : (_r1.l == _r4.l ? 0 : -1);
    if (_r0.i != 0) goto label175;
    XMLVM_SOURCE_POSITION("Double.java", 452)
    // "0."
    _r0.o = xmlvm_create_java_string_from_pool(14);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("Double.java", 454)
    _r0.i = 13;
    _r10 = _r0;
    _r0 = _r11;
    _r11 = _r10;
    label103:;
    XMLVM_SOURCE_POSITION("Double.java", 457)
    _r4.l = 0;
    _r12.i = _r0.l > _r4.l ? 1 : (_r0.l == _r4.l ? 0 : -1);
    if (_r12.i == 0) goto label118;
    _r4.l = 15;
    _r4.l = _r4.l & _r0.l;
    _r6.l = 0;
    _r12.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r12.i == 0) goto label163;
    label118:;
    XMLVM_SOURCE_POSITION("Double.java", 462)
    _r12.o = java_lang_Long_toHexString___long(_r0.l);
    XMLVM_SOURCE_POSITION("Double.java", 465)
    _r4.l = 0;
    _r0.i = _r0.l > _r4.l ? 1 : (_r0.l == _r4.l ? 0 : -1);
    if (_r0.i == 0) goto label143;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(12)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r12.o)->tib->vtable[8])(_r12.o);
    if (_r11.i <= _r0.i) goto label143;
    XMLVM_SOURCE_POSITION("Double.java", 466)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(12)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r12.o)->tib->vtable[8])(_r12.o);
    _r11.i = _r11.i - _r0.i;
    label139:;
    XMLVM_SOURCE_POSITION("Double.java", 467)
    _r0.i = _r11.i + -1;
    if (_r11.i != 0) goto label168;
    label143:;
    XMLVM_SOURCE_POSITION("Double.java", 471)
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r12.o);
    XMLVM_SOURCE_POSITION("Double.java", 472)
    // "p-1022"
    _r11.o = xmlvm_create_java_string_from_pool(708);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r11.o);
    label151:;
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r11.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    goto label6;
    label157:;
    XMLVM_SOURCE_POSITION("Double.java", 448)
    // "0x"
    _r0.o = xmlvm_create_java_string_from_pool(16);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    goto label87;
    label163:;
    XMLVM_SOURCE_POSITION("Double.java", 458)
    _r12.i = 4;
    _r0.l = ((JAVA_ULONG) _r0.l) >> (0x3f & ((JAVA_ULONG) _r12.l));
    _r11.i = _r11.i + -1;
    XMLVM_SOURCE_POSITION("Double.java", 459)
    goto label103;
    label168:;
    XMLVM_SOURCE_POSITION("Double.java", 468)
    _r11.i = 48;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r11.i);
    _r11 = _r0;
    goto label139;
    label175:;
    XMLVM_SOURCE_POSITION("Double.java", 474)
    // "1."
    _r0.o = xmlvm_create_java_string_from_pool(17);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("Double.java", 476)
    _r0.i = 13;
    _r4 = _r11;
    _r11 = _r0;
    label184:;
    XMLVM_SOURCE_POSITION("Double.java", 479)
    _r6.l = 0;
    _r12.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r12.i == 0) goto label199;
    _r6.l = 15;
    _r6.l = _r6.l & _r4.l;
    _r8.l = 0;
    _r12.i = _r6.l > _r8.l ? 1 : (_r6.l == _r8.l ? 0 : -1);
    if (_r12.i == 0) goto label244;
    label199:;
    XMLVM_SOURCE_POSITION("Double.java", 484)
    _r12.o = java_lang_Long_toHexString___long(_r4.l);
    XMLVM_SOURCE_POSITION("Double.java", 487)
    _r6.l = 0;
    _r0.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r0.i == 0) goto label224;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(12)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r12.o)->tib->vtable[8])(_r12.o);
    if (_r11.i <= _r0.i) goto label224;
    XMLVM_SOURCE_POSITION("Double.java", 488)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(12)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r12.o)->tib->vtable[8])(_r12.o);
    _r11.i = _r11.i - _r0.i;
    label220:;
    XMLVM_SOURCE_POSITION("Double.java", 489)
    _r0.i = _r11.i + -1;
    if (_r11.i != 0) goto label249;
    label224:;
    XMLVM_SOURCE_POSITION("Double.java", 494)
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r12.o);
    XMLVM_SOURCE_POSITION("Double.java", 495)
    _r11.i = 112;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r11.i);
    XMLVM_SOURCE_POSITION("Double.java", 497)
    _r11.l = 1023;
    _r11.l = _r1.l - _r11.l;
    _r11.o = java_lang_Long_toString___long(_r11.l);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder_append___java_lang_String(_r3.o, _r11.o);
    goto label151;
    label244:;
    XMLVM_SOURCE_POSITION("Double.java", 480)
    _r12.i = 4;
    _r4.l = ((JAVA_ULONG) _r4.l) >> (0x3f & ((JAVA_ULONG) _r12.l));
    _r11.i = _r11.i + -1;
    XMLVM_SOURCE_POSITION("Double.java", 481)
    goto label184;
    label249:;
    XMLVM_SOURCE_POSITION("Double.java", 490)
    _r11.i = 48;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[6])(_r3.o, _r11.i);
    _r11 = _r0;
    goto label220;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Double_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Double_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Double", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Double.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Double_compareTo___java_lang_Double(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

