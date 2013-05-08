#include "xmlvm.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"

#include "java_lang_Math.h"

#define XMLVM_CURRENT_CLASS_NAME Math
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Math

__TIB_DEFINITION_java_lang_Math __TIB_java_lang_Math = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Math, // classInitializer
    "java.lang.Math", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Math), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Math;
JAVA_OBJECT __CLASS_java_lang_Math_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Math_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Math_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_lang_Math_FLOAT_EXPONENT_BIAS;
static JAVA_INT _STATIC_java_lang_Math_FLOAT_EXPONENT_MASK;
static JAVA_INT _STATIC_java_lang_Math_DOUBLE_NON_MANTISSA_BITS;
static JAVA_INT _STATIC_java_lang_Math_DOUBLE_MANTISSA_BITS;
static JAVA_INT _STATIC_java_lang_Math_FLOAT_NON_MANTISSA_BITS;
static JAVA_INT _STATIC_java_lang_Math_FLOAT_MANTISSA_BITS;
static JAVA_INT _STATIC_java_lang_Math_DOUBLE_EXPONENT_BIAS;
static JAVA_LONG _STATIC_java_lang_Math_DOUBLE_EXPONENT_MASK;
static JAVA_INT _STATIC_java_lang_Math_FLOAT_MANTISSA_MASK;
static JAVA_INT _STATIC_java_lang_Math_FLOAT_SIGN_MASK;
static JAVA_LONG _STATIC_java_lang_Math_DOUBLE_MANTISSA_MASK;
static JAVA_LONG _STATIC_java_lang_Math_DOUBLE_SIGN_MASK;
static JAVA_DOUBLE _STATIC_java_lang_Math_E;
static JAVA_DOUBLE _STATIC_java_lang_Math_PI;
static JAVA_OBJECT _STATIC_java_lang_Math_random;

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

void __INIT_java_lang_Math()
{
    staticInitializerLock(&__TIB_java_lang_Math);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Math.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Math.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Math);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Math.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Math.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Math.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Math")
        __INIT_IMPL_java_lang_Math();
    }
}

void __INIT_IMPL_java_lang_Math()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Math.newInstanceFunc = __NEW_INSTANCE_java_lang_Math;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Math.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_Math.numImplementedInterfaces = 0;
    __TIB_java_lang_Math.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_Math_FLOAT_EXPONENT_BIAS = 127;
    _STATIC_java_lang_Math_FLOAT_EXPONENT_MASK = 2139095040;
    _STATIC_java_lang_Math_DOUBLE_NON_MANTISSA_BITS = 12;
    _STATIC_java_lang_Math_DOUBLE_MANTISSA_BITS = 52;
    _STATIC_java_lang_Math_FLOAT_NON_MANTISSA_BITS = 9;
    _STATIC_java_lang_Math_FLOAT_MANTISSA_BITS = 23;
    _STATIC_java_lang_Math_DOUBLE_EXPONENT_BIAS = 1023;
    _STATIC_java_lang_Math_DOUBLE_EXPONENT_MASK = 9218868437227405312;
    _STATIC_java_lang_Math_FLOAT_MANTISSA_MASK = 8388607;
    _STATIC_java_lang_Math_FLOAT_SIGN_MASK = -2147483648;
    _STATIC_java_lang_Math_DOUBLE_MANTISSA_MASK = 4503599627370495;
    _STATIC_java_lang_Math_DOUBLE_SIGN_MASK = -9223372036854775808;
    _STATIC_java_lang_Math_E = 2.718281828459045;
    _STATIC_java_lang_Math_PI = 3.141592653589793;
    _STATIC_java_lang_Math_random = (java_lang_Object*) JAVA_NULL;

    __TIB_java_lang_Math.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Math.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Math.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Math.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Math.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Math.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Math.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Math.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Math = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Math);
    __TIB_java_lang_Math.clazz = __CLASS_java_lang_Math;
    __TIB_java_lang_Math.baseType = JAVA_NULL;
    __CLASS_java_lang_Math_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Math);
    __CLASS_java_lang_Math_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Math_1ARRAY);
    __CLASS_java_lang_Math_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Math_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Math]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Math.classInitialized = 1;
}

void __DELETE_java_lang_Math(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Math]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Math(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Math]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Math()
{    XMLVM_CLASS_INIT(java_lang_Math)
java_lang_Math* me = (java_lang_Math*) XMLVM_MALLOC(sizeof(java_lang_Math));
    me->tib = &__TIB_java_lang_Math;
    __INIT_INSTANCE_MEMBERS_java_lang_Math(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Math]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Math()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_INT java_lang_Math_GET_FLOAT_EXPONENT_BIAS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_EXPONENT_BIAS;
}

void java_lang_Math_PUT_FLOAT_EXPONENT_BIAS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_EXPONENT_BIAS = v;
}

JAVA_INT java_lang_Math_GET_FLOAT_EXPONENT_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_EXPONENT_MASK;
}

void java_lang_Math_PUT_FLOAT_EXPONENT_MASK(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_EXPONENT_MASK = v;
}

JAVA_INT java_lang_Math_GET_DOUBLE_NON_MANTISSA_BITS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_NON_MANTISSA_BITS;
}

void java_lang_Math_PUT_DOUBLE_NON_MANTISSA_BITS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_NON_MANTISSA_BITS = v;
}

JAVA_INT java_lang_Math_GET_DOUBLE_MANTISSA_BITS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_MANTISSA_BITS;
}

void java_lang_Math_PUT_DOUBLE_MANTISSA_BITS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_MANTISSA_BITS = v;
}

JAVA_INT java_lang_Math_GET_FLOAT_NON_MANTISSA_BITS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_NON_MANTISSA_BITS;
}

void java_lang_Math_PUT_FLOAT_NON_MANTISSA_BITS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_NON_MANTISSA_BITS = v;
}

JAVA_INT java_lang_Math_GET_FLOAT_MANTISSA_BITS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_MANTISSA_BITS;
}

void java_lang_Math_PUT_FLOAT_MANTISSA_BITS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_MANTISSA_BITS = v;
}

JAVA_INT java_lang_Math_GET_DOUBLE_EXPONENT_BIAS()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_EXPONENT_BIAS;
}

void java_lang_Math_PUT_DOUBLE_EXPONENT_BIAS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_EXPONENT_BIAS = v;
}

JAVA_LONG java_lang_Math_GET_DOUBLE_EXPONENT_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_EXPONENT_MASK;
}

void java_lang_Math_PUT_DOUBLE_EXPONENT_MASK(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_EXPONENT_MASK = v;
}

JAVA_INT java_lang_Math_GET_FLOAT_MANTISSA_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_MANTISSA_MASK;
}

void java_lang_Math_PUT_FLOAT_MANTISSA_MASK(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_MANTISSA_MASK = v;
}

JAVA_INT java_lang_Math_GET_FLOAT_SIGN_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_FLOAT_SIGN_MASK;
}

void java_lang_Math_PUT_FLOAT_SIGN_MASK(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_FLOAT_SIGN_MASK = v;
}

JAVA_LONG java_lang_Math_GET_DOUBLE_MANTISSA_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_MANTISSA_MASK;
}

void java_lang_Math_PUT_DOUBLE_MANTISSA_MASK(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_MANTISSA_MASK = v;
}

JAVA_LONG java_lang_Math_GET_DOUBLE_SIGN_MASK()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_DOUBLE_SIGN_MASK;
}

void java_lang_Math_PUT_DOUBLE_SIGN_MASK(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_DOUBLE_SIGN_MASK = v;
}

JAVA_DOUBLE java_lang_Math_GET_E()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_E;
}

void java_lang_Math_PUT_E(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_E = v;
}

JAVA_DOUBLE java_lang_Math_GET_PI()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_PI;
}

void java_lang_Math_PUT_PI(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_PI = v;
}

JAVA_OBJECT java_lang_Math_GET_random()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    return _STATIC_java_lang_Math_random;
}

void java_lang_Math_PUT_random(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Math)
_STATIC_java_lang_Math_random = v;
}

void java_lang_Math___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Math___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Math", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Math.java", 66)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Math.java", 67)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_abs___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_abs___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "abs", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 85)
    _r0.l = java_lang_Double_doubleToLongBits___double(_r4.d);
    XMLVM_SOURCE_POSITION("Math.java", 86)
    _r2.l = 9223372036854775807;
    _r0.l = _r0.l & _r2.l;
    XMLVM_SOURCE_POSITION("Math.java", 87)
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_abs___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_abs___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "abs", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.f = n1;
    XMLVM_SOURCE_POSITION("Math.java", 107)
    _r0.i = java_lang_Float_floatToIntBits___float(_r2.f);
    XMLVM_SOURCE_POSITION("Math.java", 108)
    _r1.i = 2147483647;
    _r0.i = _r0.i & _r1.i;
    XMLVM_SOURCE_POSITION("Math.java", 109)
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_abs___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_abs___int]
    XMLVM_ENTER_METHOD("java.lang.Math", "abs", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Math.java", 124)
    if (_r1.i < 0) goto label4;
    _r0 = _r1;
    label3:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label4:;
    _r0.i = -_r1.i;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Math_abs___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_abs___long]
    XMLVM_ENTER_METHOD("java.lang.Math", "abs", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.l = n1;
    XMLVM_SOURCE_POSITION("Math.java", 137)
    _r0.l = 0;
    _r0.i = _r2.l > _r0.l ? 1 : (_r2.l == _r0.l ? 0 : -1);
    if (_r0.i < 0) goto label8;
    _r0 = _r2;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label8:;
    _r0.l = -_r2.l;
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_acos___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_asin___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_atan___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_atan2___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_cbrt___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_ceil___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_cos___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_cosh___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_exp___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_expm1___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_floor___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_hypot___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_IEEEremainder___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_log___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_log10___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_log1p___double(JAVA_DOUBLE n1)]

JAVA_DOUBLE java_lang_Math_max___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_max___double_double]
    XMLVM_ENTER_METHOD("java.lang.Math", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r4.d = n1;
    _r6.d = n2;
    XMLVM_SOURCE_POSITION("Math.java", 513)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i <= 0) goto label6;
    _r0 = _r4;
    label5:;
    XMLVM_SOURCE_POSITION("Math.java", 514)
    XMLVM_SOURCE_POSITION("Math.java", 528)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 516)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i >= 0) goto label12;
    _r0 = _r6;
    XMLVM_SOURCE_POSITION("Math.java", 517)
    goto label5;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 520)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i == 0) goto label19;
    XMLVM_SOURCE_POSITION("Math.java", 521)
    _r0.d = NaN;
    goto label5;
    label19:;
    XMLVM_SOURCE_POSITION("Math.java", 525)
    _r0.l = java_lang_Double_doubleToRawLongBits___double(_r4.d);
    _r2.l = 0;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i == 0) goto label31;
    _r0 = _r6;
    XMLVM_SOURCE_POSITION("Math.java", 526)
    goto label5;
    label31:;
    _r0.d = 0.0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_max___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_max___float_float]
    XMLVM_ENTER_METHOD("java.lang.Math", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.f = n1;
    _r2.f = n2;
    XMLVM_SOURCE_POSITION("Math.java", 550)
    _r0.i = _r1.f > _r2.f ? 1 : (_r1.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label6;
    _r0 = _r1;
    label5:;
    XMLVM_SOURCE_POSITION("Math.java", 551)
    XMLVM_SOURCE_POSITION("Math.java", 565)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 553)
    _r0.i = _r1.f > _r2.f ? 1 : (_r1.f == _r2.f ? 0 : -1);
    if (_r0.i >= 0) goto label12;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Math.java", 554)
    goto label5;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 557)
    _r0.i = _r1.f > _r2.f ? 1 : (_r1.f == _r2.f ? 0 : -1);
    if (_r0.i == 0) goto label19;
    XMLVM_SOURCE_POSITION("Math.java", 558)
    _r0.f = NaN;
    goto label5;
    label19:;
    XMLVM_SOURCE_POSITION("Math.java", 562)
    _r0.i = java_lang_Float_floatToRawIntBits___float(_r1.f);
    if (_r0.i == 0) goto label27;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Math.java", 563)
    goto label5;
    label27:;
    _r0.f = 0.0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_max___int_int(JAVA_INT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_max___int_int]
    XMLVM_ENTER_METHOD("java.lang.Math", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Math.java", 579)
    if (_r1.i <= _r2.i) goto label4;
    _r0 = _r1;
    label3:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label4:;
    _r0 = _r2;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Math_max___long_long(JAVA_LONG n1, JAVA_LONG n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_max___long_long]
    XMLVM_ENTER_METHOD("java.lang.Math", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.l = n1;
    _r4.l = n2;
    XMLVM_SOURCE_POSITION("Math.java", 593)
    _r0.i = _r2.l > _r4.l ? 1 : (_r2.l == _r4.l ? 0 : -1);
    if (_r0.i <= 0) goto label6;
    _r0 = _r2;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label6:;
    _r0 = _r4;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_min___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_min___double_double]
    XMLVM_ENTER_METHOD("java.lang.Math", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r4.d = n1;
    _r6.d = n2;
    XMLVM_SOURCE_POSITION("Math.java", 615)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i <= 0) goto label6;
    _r0 = _r6;
    label5:;
    XMLVM_SOURCE_POSITION("Math.java", 616)
    XMLVM_SOURCE_POSITION("Math.java", 630)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 618)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i >= 0) goto label12;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("Math.java", 619)
    goto label5;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 622)
    _r0.i = _r4.d > _r6.d ? 1 : (_r4.d == _r6.d ? 0 : -1);
    if (_r0.i == 0) goto label19;
    XMLVM_SOURCE_POSITION("Math.java", 623)
    _r0.d = NaN;
    goto label5;
    label19:;
    XMLVM_SOURCE_POSITION("Math.java", 627)
    _r0.l = java_lang_Double_doubleToRawLongBits___double(_r4.d);
    _r2.l = -9223372036854775808;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i != 0) goto label32;
    XMLVM_SOURCE_POSITION("Math.java", 628)
    _r0.d = -0.0;
    goto label5;
    label32:;
    _r0 = _r6;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_min___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_min___float_float]
    XMLVM_ENTER_METHOD("java.lang.Math", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.f = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("Math.java", 652)
    _r0.i = _r2.f > _r3.f ? 1 : (_r2.f == _r3.f ? 0 : -1);
    if (_r0.i <= 0) goto label6;
    _r0 = _r3;
    label5:;
    XMLVM_SOURCE_POSITION("Math.java", 653)
    XMLVM_SOURCE_POSITION("Math.java", 667)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 655)
    _r0.i = _r2.f > _r3.f ? 1 : (_r2.f == _r3.f ? 0 : -1);
    if (_r0.i >= 0) goto label12;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Math.java", 656)
    goto label5;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 659)
    _r0.i = _r2.f > _r3.f ? 1 : (_r2.f == _r3.f ? 0 : -1);
    if (_r0.i == 0) goto label19;
    XMLVM_SOURCE_POSITION("Math.java", 660)
    _r0.f = NaN;
    goto label5;
    label19:;
    XMLVM_SOURCE_POSITION("Math.java", 664)
    _r0.i = java_lang_Float_floatToRawIntBits___float(_r2.f);
    _r1.i = -2147483648;
    if (_r0.i != _r1.i) goto label30;
    XMLVM_SOURCE_POSITION("Math.java", 665)
    _r0.f = -0.0;
    goto label5;
    label30:;
    _r0 = _r3;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_min___int_int(JAVA_INT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_min___int_int]
    XMLVM_ENTER_METHOD("java.lang.Math", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Math.java", 681)
    if (_r1.i >= _r2.i) goto label4;
    _r0 = _r1;
    label3:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label4:;
    _r0 = _r2;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Math_min___long_long(JAVA_LONG n1, JAVA_LONG n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_min___long_long]
    XMLVM_ENTER_METHOD("java.lang.Math", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.l = n1;
    _r4.l = n2;
    XMLVM_SOURCE_POSITION("Math.java", 695)
    _r0.i = _r2.l > _r4.l ? 1 : (_r2.l == _r4.l ? 0 : -1);
    if (_r0.i >= 0) goto label6;
    _r0 = _r2;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label6:;
    _r0 = _r4;
    goto label5;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_pow___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_rint___double(JAVA_DOUBLE n1)]

JAVA_LONG java_lang_Math_round___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_round___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "round", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 777)
    _r0.i = _r2.d > _r2.d ? 1 : (_r2.d == _r2.d ? 0 : -1);
    if (_r0.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("Math.java", 778)
    _r0.l = 0;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 780)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label7:;
    _r0.d = 0.5;
    _r0.d = _r0.d + _r2.d;
    _r0.d = java_lang_Math_floor___double(_r0.d);
    _r0.l = (JAVA_LONG) _r0.d;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_round___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_round___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "round", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.f = n1;
    XMLVM_SOURCE_POSITION("Math.java", 804)
    _r0.i = _r2.f > _r2.f ? 1 : (_r2.f == _r2.f ? 0 : -1);
    if (_r0.i == 0) goto label6;
    XMLVM_SOURCE_POSITION("Math.java", 805)
    _r0.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("Math.java", 807)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.f = 0.5;
    _r0.f = _r0.f + _r2.f;
    _r0.d = (JAVA_DOUBLE) _r0.f;
    _r0.d = java_lang_Math_floor___double(_r0.d);
    _r0.i = (JAVA_INT) _r0.d;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_signum___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_signum___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "signum", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 830)

    
    // Red class access removed: java.lang.StrictMath::signum
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_signum___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_signum___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "signum", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("Math.java", 853)

    
    // Red class access removed: java.lang.StrictMath::signum
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_sin___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_sinh___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_sqrt___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_tan___double(JAVA_DOUBLE n1)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_tanh___double(JAVA_DOUBLE n1)]

JAVA_DOUBLE java_lang_Math_random__()
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_random__]
    XMLVM_ENTER_METHOD("java.lang.Math", "random", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Math.java", 964)
    _r0.o = java_lang_Math_GET_random();
    if (_r0.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("Math.java", 965)

    
    // Red class access removed: java.util.Random::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Random::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    java_lang_Math_PUT_random( _r0.o);
    label11:;
    XMLVM_SOURCE_POSITION("Math.java", 967)
    _r0.o = java_lang_Math_GET_random();

    
    // Red class access removed: java.util.Random::nextDouble
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_toRadians___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_toRadians___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "toRadians", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 988)
    _r0.d = 180.0;
    _r0.d = _r4.d / _r0.d;
    _r2.d = 3.141592653589793;
    _r0.d = _r0.d * _r2.d;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_toDegrees___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_toDegrees___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "toDegrees", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 1009)
    _r0.d = 180.0;
    _r0.d = _r0.d * _r4.d;
    _r2.d = 3.141592653589793;
    _r0.d = _r0.d / _r2.d;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_ulp___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_ulp___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "ulp", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.d = n1;
    _r2.d = 1.7976931348623157E308;
    XMLVM_SOURCE_POSITION("Math.java", 1033)
    _r0.i = java_lang_Double_isInfinite___double(_r4.d);
    if (_r0.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Math.java", 1034)
    _r0.d = Infinity;
    label13:;
    XMLVM_SOURCE_POSITION("Math.java", 1039)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label14:;
    XMLVM_SOURCE_POSITION("Math.java", 1035)
    _r0.i = _r4.d > _r2.d ? 1 : (_r4.d == _r2.d ? 0 : -1);
    if (_r0.i == 0) goto label27;
    _r0.d = -1.7976931348623157E308;
    _r0.i = _r4.d > _r0.d ? 1 : (_r4.d == _r0.d ? 0 : -1);
    if (_r0.i != 0) goto label39;
    label27:;
    XMLVM_SOURCE_POSITION("Math.java", 1036)
    _r0.d = 2.0;
    _r2.d = 971.0;
    _r0.d = java_lang_Math_pow___double_double(_r0.d, _r2.d);
    goto label13;
    label39:;
    XMLVM_SOURCE_POSITION("Math.java", 1038)
    _r0.d = java_lang_Math_abs___double(_r4.d);
    _r2.d = java_lang_Math_nextafter___double_double(_r0.d, _r2.d);
    _r0.d = _r2.d - _r0.d;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_ulp___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_ulp___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "ulp", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.f = n1;
    _r1.f = 3.4028235E38;
    XMLVM_SOURCE_POSITION("Math.java", 1063)
    _r0.i = java_lang_Float_isNaN___float(_r4.f);
    if (_r0.i == 0) goto label12;
    XMLVM_SOURCE_POSITION("Math.java", 1064)
    _r0.f = NaN;
    label11:;
    XMLVM_SOURCE_POSITION("Math.java", 1071)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 1065)
    _r0.i = java_lang_Float_isInfinite___float(_r4.f);
    if (_r0.i == 0) goto label21;
    XMLVM_SOURCE_POSITION("Math.java", 1066)
    _r0.f = Infinity;
    goto label11;
    label21:;
    XMLVM_SOURCE_POSITION("Math.java", 1067)
    _r0.i = _r4.f > _r1.f ? 1 : (_r4.f == _r1.f ? 0 : -1);
    if (_r0.i == 0) goto label32;
    _r0.f = -3.4028235E38;
    _r0.i = _r4.f > _r0.f ? 1 : (_r4.f == _r0.f ? 0 : -1);
    if (_r0.i != 0) goto label42;
    label32:;
    XMLVM_SOURCE_POSITION("Math.java", 1068)
    _r0.d = 2.0;
    _r2.d = 104.0;
    _r0.d = java_lang_Math_pow___double_double(_r0.d, _r2.d);
    _r0.f = (JAVA_FLOAT) _r0.d;
    goto label11;
    label42:;
    XMLVM_SOURCE_POSITION("Math.java", 1070)
    _r0.f = java_lang_Math_abs___float(_r4.f);
    _r1.f = java_lang_Math_nextafterf___float_float(_r0.f, _r1.f);
    _r0.f = _r1.f - _r0.f;
    goto label11;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_Math_nextafter___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)]

//XMLVM_NATIVE[JAVA_FLOAT java_lang_Math_nextafterf___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2)]

JAVA_DOUBLE java_lang_Math_copySign___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_copySign___double_double]
    XMLVM_ENTER_METHOD("java.lang.Math", "copySign", "?")
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
    XMLVM_SOURCE_POSITION("Math.java", 1092)
    _r0.l = java_lang_Double_doubleToRawLongBits___double(_r6.d);
    XMLVM_SOURCE_POSITION("Math.java", 1093)
    _r2.l = java_lang_Double_doubleToRawLongBits___double(_r8.d);
    XMLVM_SOURCE_POSITION("Math.java", 1094)
    _r4.l = 9223372036854775807;
    _r0.l = _r0.l & _r4.l;
    _r4.l = -9223372036854775808;
    _r2.l = _r2.l & _r4.l;
    _r0.l = _r0.l | _r2.l;
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_copySign___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_copySign___float_float]
    XMLVM_ENTER_METHOD("java.lang.Math", "copySign", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.f = n1;
    _r4.f = n2;
    XMLVM_SOURCE_POSITION("Math.java", 1112)
    _r0.i = java_lang_Float_floatToRawIntBits___float(_r3.f);
    XMLVM_SOURCE_POSITION("Math.java", 1113)
    _r1.i = java_lang_Float_floatToRawIntBits___float(_r4.f);
    XMLVM_SOURCE_POSITION("Math.java", 1114)
    _r2.i = 2147483647;
    _r0.i = _r0.i & _r2.i;
    _r2.i = -2147483648;
    _r1.i = _r1.i & _r2.i;
    _r0.i = _r0.i | _r1.i;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_getExponent___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_getExponent___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "getExponent", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.f = n1;
    XMLVM_SOURCE_POSITION("Math.java", 1128)
    _r0.i = java_lang_Float_floatToRawIntBits___float(_r2.f);
    XMLVM_SOURCE_POSITION("Math.java", 1129)
    _r1.i = 2139095040;
    _r0.i = _r0.i & _r1.i;
    _r0.i = _r0.i >> 23;
    _r1.i = 127;
    _r0.i = _r0.i - _r1.i;
    XMLVM_SOURCE_POSITION("Math.java", 1130)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_getExponent___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_getExponent___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "getExponent", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.d = n1;
    XMLVM_SOURCE_POSITION("Math.java", 1143)
    _r0.l = java_lang_Double_doubleToRawLongBits___double(_r4.d);
    XMLVM_SOURCE_POSITION("Math.java", 1144)
    _r2.l = 9218868437227405312;
    _r0.l = _r0.l & _r2.l;
    _r2.i = 52;
    _r0.l = _r0.l >> (0x3f & _r2.l);
    _r0.i = (JAVA_INT) _r0.l;
    _r1.i = 1023;
    _r0.i = _r0.i - _r1.i;
    XMLVM_SOURCE_POSITION("Math.java", 1145)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_nextAfter___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_nextAfter___double_double]
    XMLVM_ENTER_METHOD("java.lang.Math", "nextAfter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r3.d = n1;
    _r5.d = n2;
    _r1.d = 0.0;
    XMLVM_SOURCE_POSITION("Math.java", 1162)
    _r0.i = _r1.d > _r3.d ? 1 : (_r1.d == _r3.d ? 0 : -1);
    if (_r0.i != 0) goto label12;
    _r0.i = _r1.d > _r5.d ? 1 : (_r1.d == _r5.d ? 0 : -1);
    if (_r0.i != 0) goto label12;
    _r0 = _r5;
    label11:;
    XMLVM_SOURCE_POSITION("Math.java", 1163)
    XMLVM_SOURCE_POSITION("Math.java", 1165)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label12:;
    _r0.d = java_lang_Math_nextafter___double_double(_r3.d, _r5.d);
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_nextAfter___float_double(JAVA_FLOAT n1, JAVA_DOUBLE n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_nextAfter___float_double]
    XMLVM_ENTER_METHOD("java.lang.Math", "nextAfter", "?")
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
    _r7.f = n1;
    _r8.d = n2;
    _r6.f = 3.4028235E38;
    _r5.f = 1.4E-45;
    _r4.f = -3.4028235E38;
    _r3.f = -1.4E-45;
    _r2.f = 0.0;
    XMLVM_SOURCE_POSITION("Math.java", 1183)
    _r0.i = java_lang_Float_isNaN___float(_r7.f);
    if (_r0.i != 0) goto label23;
    _r0.i = java_lang_Double_isNaN___double(_r8.d);
    if (_r0.i == 0) goto label26;
    label23:;
    XMLVM_SOURCE_POSITION("Math.java", 1184)
    _r0.f = NaN;
    label25:;
    XMLVM_SOURCE_POSITION("Math.java", 1219)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label26:;
    XMLVM_SOURCE_POSITION("Math.java", 1186)
    _r0.i = _r2.f > _r7.f ? 1 : (_r2.f == _r7.f ? 0 : -1);
    if (_r0.i != 0) goto label46;
    _r0.d = 0.0;
    _r0.i = _r0.d > _r8.d ? 1 : (_r0.d == _r8.d ? 0 : -1);
    if (_r0.i != 0) goto label46;
    XMLVM_SOURCE_POSITION("Math.java", 1187)
    _r0.o = __NEW_java_lang_Float();
    XMLVM_CHECK_NPE(0)
    java_lang_Float___INIT____double(_r0.o, _r8.d);
    //java_lang_Float_floatValue__[8]
    XMLVM_CHECK_NPE(0)
    _r0.f = (*(JAVA_FLOAT (*)(JAVA_OBJECT)) ((java_lang_Float*) _r0.o)->tib->vtable[8])(_r0.o);
    goto label25;
    label46:;
    XMLVM_SOURCE_POSITION("Math.java", 1189)
    _r0.i = _r7.f > _r5.f ? 1 : (_r7.f == _r5.f ? 0 : -1);
    if (_r0.i != 0) goto label55;
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i < 0) goto label64;
    label55:;
    XMLVM_SOURCE_POSITION("Math.java", 1190)
    _r0.i = _r7.f > _r3.f ? 1 : (_r7.f == _r3.f ? 0 : -1);
    if (_r0.i != 0) goto label73;
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i <= 0) goto label73;
    label64:;
    XMLVM_SOURCE_POSITION("Math.java", 1191)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label70;
    _r0 = _r2;
    goto label25;
    label70:;
    _r0.f = -0.0;
    goto label25;
    label73:;
    XMLVM_SOURCE_POSITION("Math.java", 1193)
    _r0.i = java_lang_Float_isInfinite___float(_r7.f);
    if (_r0.i == 0) goto label92;
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i == 0) goto label92;
    XMLVM_SOURCE_POSITION("Math.java", 1194)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label90;
    _r0 = _r6;
    goto label25;
    label90:;
    _r0 = _r4;
    goto label25;
    label92:;
    XMLVM_SOURCE_POSITION("Math.java", 1196)
    _r0.i = _r7.f > _r6.f ? 1 : (_r7.f == _r6.f ? 0 : -1);
    if (_r0.i != 0) goto label101;
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i > 0) goto label110;
    label101:;
    XMLVM_SOURCE_POSITION("Math.java", 1197)
    _r0.i = _r7.f > _r4.f ? 1 : (_r7.f == _r4.f ? 0 : -1);
    if (_r0.i != 0) goto label120;
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i >= 0) goto label120;
    label110:;
    XMLVM_SOURCE_POSITION("Math.java", 1198)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label117;
    _r0.f = Infinity;
    goto label25;
    label117:;
    XMLVM_SOURCE_POSITION("Math.java", 1199)
    _r0.f = -Infinity;
    goto label25;
    label120:;
    XMLVM_SOURCE_POSITION("Math.java", 1201)
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i <= 0) goto label159;
    XMLVM_SOURCE_POSITION("Math.java", 1202)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label140;
    XMLVM_SOURCE_POSITION("Math.java", 1203)
    _r0.i = java_lang_Float_floatToIntBits___float(_r7.f);
    _r0.i = _r0.i + 1;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label25;
    label140:;
    XMLVM_SOURCE_POSITION("Math.java", 1205)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i >= 0) goto label156;
    XMLVM_SOURCE_POSITION("Math.java", 1206)
    _r0.i = java_lang_Float_floatToIntBits___float(_r7.f);
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label25;
    label156:;
    _r0 = _r5;
    XMLVM_SOURCE_POSITION("Math.java", 1208)
    goto label25;
    label159:;
    XMLVM_SOURCE_POSITION("Math.java", 1210)
    _r0.d = (JAVA_DOUBLE) _r7.f;
    _r0.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r0.i >= 0) goto label199;
    XMLVM_SOURCE_POSITION("Math.java", 1211)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i <= 0) goto label180;
    XMLVM_SOURCE_POSITION("Math.java", 1212)
    _r0.i = java_lang_Float_floatToIntBits___float(_r7.f);
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label25;
    label180:;
    XMLVM_SOURCE_POSITION("Math.java", 1214)
    _r0.i = _r7.f > _r2.f ? 1 : (_r7.f == _r2.f ? 0 : -1);
    if (_r0.i >= 0) goto label196;
    XMLVM_SOURCE_POSITION("Math.java", 1215)
    _r0.i = java_lang_Float_floatToIntBits___float(_r7.f);
    _r0.i = _r0.i + 1;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label25;
    label196:;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Math.java", 1217)
    goto label25;
    label199:;
    _r0.o = __NEW_java_lang_Float();
    XMLVM_CHECK_NPE(0)
    java_lang_Float___INIT____double(_r0.o, _r8.d);
    //java_lang_Float_floatValue__[8]
    XMLVM_CHECK_NPE(0)
    _r0.f = (*(JAVA_FLOAT (*)(JAVA_OBJECT)) ((java_lang_Float*) _r0.o)->tib->vtable[8])(_r0.o);
    goto label25;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_nextUp___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_nextUp___double]
    XMLVM_ENTER_METHOD("java.lang.Math", "nextUp", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r7.d = n1;
    _r5.l = 1;
    _r3.d = Infinity;
    _r1.d = 0.0;
    XMLVM_SOURCE_POSITION("Math.java", 1232)
    _r0.i = java_lang_Double_isNaN___double(_r7.d);
    if (_r0.i == 0) goto label15;
    XMLVM_SOURCE_POSITION("Math.java", 1233)
    _r0.d = NaN;
    label14:;
    XMLVM_SOURCE_POSITION("Math.java", 1243)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label15:;
    XMLVM_SOURCE_POSITION("Math.java", 1235)
    _r0.i = _r7.d > _r3.d ? 1 : (_r7.d == _r3.d ? 0 : -1);
    if (_r0.i != 0) goto label21;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Math.java", 1236)
    goto label14;
    label21:;
    XMLVM_SOURCE_POSITION("Math.java", 1238)
    _r0.i = _r1.d > _r7.d ? 1 : (_r1.d == _r7.d ? 0 : -1);
    if (_r0.i != 0) goto label28;
    XMLVM_SOURCE_POSITION("Math.java", 1239)
    _r0.d = 4.9E-324;
    goto label14;
    label28:;
    XMLVM_SOURCE_POSITION("Math.java", 1240)
    _r0.i = _r1.d > _r7.d ? 1 : (_r1.d == _r7.d ? 0 : -1);
    if (_r0.i >= 0) goto label42;
    XMLVM_SOURCE_POSITION("Math.java", 1241)
    _r0.l = java_lang_Double_doubleToLongBits___double(_r7.d);
    _r0.l = _r0.l + _r5.l;
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    goto label14;
    label42:;
    _r0.l = java_lang_Double_doubleToLongBits___double(_r7.d);
    _r0.l = _r0.l - _r5.l;
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_nextUp___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_nextUp___float]
    XMLVM_ENTER_METHOD("java.lang.Math", "nextUp", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.f = n1;
    _r2.f = Infinity;
    _r1.f = 0.0;
    XMLVM_SOURCE_POSITION("Math.java", 1257)
    _r0.i = java_lang_Float_isNaN___float(_r3.f);
    if (_r0.i == 0) goto label12;
    XMLVM_SOURCE_POSITION("Math.java", 1258)
    _r0.f = NaN;
    label11:;
    XMLVM_SOURCE_POSITION("Math.java", 1268)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 1260)
    _r0.i = _r3.f > _r2.f ? 1 : (_r3.f == _r2.f ? 0 : -1);
    if (_r0.i != 0) goto label18;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Math.java", 1261)
    goto label11;
    label18:;
    XMLVM_SOURCE_POSITION("Math.java", 1263)
    _r0.i = _r1.f > _r3.f ? 1 : (_r1.f == _r3.f ? 0 : -1);
    if (_r0.i != 0) goto label24;
    XMLVM_SOURCE_POSITION("Math.java", 1264)
    _r0.f = 1.4E-45;
    goto label11;
    label24:;
    XMLVM_SOURCE_POSITION("Math.java", 1265)
    _r0.i = _r1.f > _r3.f ? 1 : (_r1.f == _r3.f ? 0 : -1);
    if (_r0.i >= 0) goto label39;
    XMLVM_SOURCE_POSITION("Math.java", 1266)
    _r0.i = java_lang_Float_floatToIntBits___float(_r3.f);
    _r0.i = _r0.i + 1;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label11;
    label39:;
    _r0.i = java_lang_Float_floatToIntBits___float(_r3.f);
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Math_scalb___double_int(JAVA_DOUBLE n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_scalb___double_int]
    XMLVM_ENTER_METHOD("java.lang.Math", "scalb", "?")
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
    _r8.d = n1;
    _r10.i = n2;
    XMLVM_SOURCE_POSITION("Math.java", 1285)
    _r0.i = java_lang_Double_isNaN___double(_r8.d);
    if (_r0.i != 0) goto label18;
    _r0.i = java_lang_Double_isInfinite___double(_r8.d);
    if (_r0.i != 0) goto label18;
    _r0.d = 0.0;
    _r0.i = _r0.d > _r8.d ? 1 : (_r0.d == _r8.d ? 0 : -1);
    if (_r0.i != 0) goto label19;
    label18:;
    XMLVM_SOURCE_POSITION("Math.java", 1334)
    XMLVM_EXIT_METHOD()
    return _r8.d;
    label19:;
    XMLVM_SOURCE_POSITION("Math.java", 1289)
    _r0.l = java_lang_Double_doubleToLongBits___double(_r8.d);
    XMLVM_SOURCE_POSITION("Math.java", 1291)
    _r2.l = -9223372036854775808;
    _r2.l = _r2.l & _r0.l;
    _r4.l = 9218868437227405312;
    _r4.l = _r4.l & _r0.l;
    _r6.i = 52;
    _r4.l = _r4.l >> (0x3f & _r6.l);
    _r6.l = 1023;
    _r4.l = _r4.l - _r6.l;
    _r6.l = (JAVA_LONG) _r10.i;
    _r4.l = _r4.l + _r6.l;
    _r6.l = 9223372036854775807;
    _r6.l = _r6.l & _r0.l;
    XMLVM_SOURCE_POSITION("Math.java", 1297)
    _r10.i = java_lang_Long_numberOfLeadingZeros___long(_r6.l);
    XMLVM_SOURCE_POSITION("Math.java", 1299)
    _r6.i = 12;
    _r10.i = _r10.i - _r6.i;
    if (_r10.i >= 0) goto label68;
    XMLVM_SOURCE_POSITION("Math.java", 1300)
    XMLVM_SOURCE_POSITION("Math.java", 1302)
    _r10.i = 0;
    label53:;
    XMLVM_SOURCE_POSITION("Math.java", 1306)
    _r6.l = 1023;
    _r6.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r6.i <= 0) goto label74;
    XMLVM_SOURCE_POSITION("Math.java", 1307)
    _r0.d = 0.0;
    _r8.i = _r8.d > _r0.d ? 1 : (_r8.d == _r0.d ? 0 : -1);
    if (_r8.i <= 0) goto label71;
    _r8.d = Infinity;
    goto label18;
    label68:;
    XMLVM_SOURCE_POSITION("Math.java", 1304)
    _r6.l = (JAVA_LONG) _r10.i;
    _r4.l = _r4.l - _r6.l;
    goto label53;
    label71:;
    _r8.d = -Infinity;
    goto label18;
    label74:;
    XMLVM_SOURCE_POSITION("Math.java", 1312)
    _r6.l = -1023;
    _r6.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r6.i > 0) goto label129;
    XMLVM_SOURCE_POSITION("Math.java", 1314)
    _r6.l = 1023;
    _r4.l = _r4.l + _r6.l;
    _r6.l = (JAVA_LONG) _r10.i;
    _r4.l = _r4.l + _r6.l;
    XMLVM_SOURCE_POSITION("Math.java", 1315)
    _r8.d = java_lang_Math_abs___double(_r8.d);
    _r6.d = 2.2250738585072014E-308;
    _r8.i = _r8.d > _r6.d ? 1 : (_r8.d == _r6.d ? 0 : -1);
    if (_r8.i >= 0) goto label111;
    XMLVM_SOURCE_POSITION("Math.java", 1317)
    _r8.l = 4503599627370495;
    _r8.l = _r8.l & _r0.l;
    _r8.l = java_lang_Math_shiftLongBits___long_long(_r8.l, _r4.l);
    label105:;
    _r8.l = _r8.l | _r2.l;
    _r8.d = java_lang_Double_longBitsToDouble___long(_r8.l);
    goto label18;
    label111:;
    XMLVM_SOURCE_POSITION("Math.java", 1320)
    _r8.l = 4503599627370495;
    _r8.l = _r8.l & _r0.l;
    _r0.l = 4503599627370496;
    _r8.l = _r8.l | _r0.l;
    _r0.l = 1;
    _r0.l = _r4.l - _r0.l;
    _r8.l = java_lang_Math_shiftLongBits___long_long(_r8.l, _r0.l);
    goto label105;
    label129:;
    XMLVM_SOURCE_POSITION("Math.java", 1324)
    _r8.d = java_lang_Math_abs___double(_r8.d);
    _r6.d = 2.2250738585072014E-308;
    _r8.i = _r8.d > _r6.d ? 1 : (_r8.d == _r6.d ? 0 : -1);
    if (_r8.i < 0) goto label153;
    XMLVM_SOURCE_POSITION("Math.java", 1326)
    _r8.l = 1023;
    _r8.l = _r8.l + _r4.l;
    _r10.i = 52;
    _r8.l = _r8.l << (0x3f & _r10.l);
    _r4.l = 4503599627370495;
    _r0.l = _r0.l & _r4.l;
    _r8.l = _r8.l | _r0.l;
    goto label105;
    label153:;
    XMLVM_SOURCE_POSITION("Math.java", 1330)
    _r8.l = 1023;
    _r8.l = _r8.l + _r4.l;
    _r4.i = 52;
    _r8.l = _r8.l << (0x3f & _r4.l);
    _r10.i = _r10.i + 1;
    _r0.l = _r0.l << (0x3f & _r10.l);
    _r4.l = 4503599627370495;
    _r0.l = _r0.l & _r4.l;
    _r8.l = _r8.l | _r0.l;
    goto label105;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Math_scalb___float_int(JAVA_FLOAT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_scalb___float_int]
    XMLVM_ENTER_METHOD("java.lang.Math", "scalb", "?")
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
    _r10.f = n1;
    _r11.i = n2;
    _r8.i = 127;
    _r7.f = 1.17549435E-38;
    _r6.f = 0.0;
    _r5.i = 8388607;
    XMLVM_SOURCE_POSITION("Math.java", 1349)
    _r0.i = java_lang_Float_isNaN___float(_r10.f);
    if (_r0.i != 0) goto label24;
    _r0.i = java_lang_Float_isInfinite___float(_r10.f);
    if (_r0.i != 0) goto label24;
    _r0.i = _r6.f > _r10.f ? 1 : (_r6.f == _r10.f ? 0 : -1);
    if (_r0.i != 0) goto label26;
    label24:;
    _r0 = _r10;
    label25:;
    XMLVM_SOURCE_POSITION("Math.java", 1350)
    XMLVM_SOURCE_POSITION("Math.java", 1394)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label26:;
    XMLVM_SOURCE_POSITION("Math.java", 1352)
    _r0.i = java_lang_Float_floatToIntBits___float(_r10.f);
    XMLVM_SOURCE_POSITION("Math.java", 1353)
    _r1.i = -2147483648;
    _r1.i = _r1.i & _r0.i;
    _r2.i = 2139095040;
    _r2.i = _r2.i & _r0.i;
    _r2.i = _r2.i >> 23;
    _r2.i = _r2.i - _r8.i;
    _r2.i = _r2.i + _r11.i;
    _r3.i = 2147483647;
    _r3.i = _r3.i & _r0.i;
    XMLVM_SOURCE_POSITION("Math.java", 1357)
    _r3.i = java_lang_Integer_numberOfLeadingZeros___int(_r3.i);
    XMLVM_SOURCE_POSITION("Math.java", 1359)
    _r4.i = 9;
    _r3.i = _r3.i - _r4.i;
    if (_r3.i >= 0) goto label66;
    XMLVM_SOURCE_POSITION("Math.java", 1360)
    XMLVM_SOURCE_POSITION("Math.java", 1362)
    _r3.i = 0;
    _r9 = _r3;
    _r3 = _r2;
    _r2 = _r9;
    label57:;
    XMLVM_SOURCE_POSITION("Math.java", 1366)
    if (_r3.i <= _r8.i) goto label74;
    XMLVM_SOURCE_POSITION("Math.java", 1367)
    _r0.i = _r10.f > _r6.f ? 1 : (_r10.f == _r6.f ? 0 : -1);
    if (_r0.i <= 0) goto label71;
    _r0.f = Infinity;
    goto label25;
    label66:;
    XMLVM_SOURCE_POSITION("Math.java", 1364)
    _r2.i = _r2.i - _r3.i;
    _r9 = _r3;
    _r3 = _r2;
    _r2 = _r9;
    goto label57;
    label71:;
    _r0.f = -Infinity;
    goto label25;
    label74:;
    XMLVM_SOURCE_POSITION("Math.java", 1372)
    _r4.i = -127;
    if (_r3.i > _r4.i) goto label111;
    XMLVM_SOURCE_POSITION("Math.java", 1374)
    _r3.i = _r3.i + 127;
    _r2.i = _r2.i + _r3.i;
    XMLVM_SOURCE_POSITION("Math.java", 1375)
    _r3.f = java_lang_Math_abs___float(_r10.f);
    _r3.i = _r3.f > _r7.f ? 1 : (_r3.f == _r7.f ? 0 : -1);
    if (_r3.i >= 0) goto label100;
    XMLVM_SOURCE_POSITION("Math.java", 1377)
    _r0.i = _r0.i & _r5.i;
    _r0.i = java_lang_Math_shiftIntBits___int_int(_r0.i, _r2.i);
    label94:;
    _r0.i = _r0.i | _r1.i;
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label25;
    label100:;
    XMLVM_SOURCE_POSITION("Math.java", 1380)
    _r0.i = _r0.i & _r5.i;
    _r3.i = 8388608;
    _r0.i = _r0.i | _r3.i;
    _r3.i = 1;
    _r2.i = _r2.i - _r3.i;
    _r0.i = java_lang_Math_shiftIntBits___int_int(_r0.i, _r2.i);
    goto label94;
    label111:;
    XMLVM_SOURCE_POSITION("Math.java", 1384)
    _r4.f = java_lang_Math_abs___float(_r10.f);
    _r4.i = _r4.f > _r7.f ? 1 : (_r4.f == _r7.f ? 0 : -1);
    if (_r4.i < 0) goto label126;
    XMLVM_SOURCE_POSITION("Math.java", 1386)
    _r2.i = _r3.i + 127;
    _r2.i = _r2.i << 23;
    _r0.i = _r0.i & _r5.i;
    _r0.i = _r0.i | _r2.i;
    goto label94;
    label126:;
    XMLVM_SOURCE_POSITION("Math.java", 1390)
    _r3.i = _r3.i + 127;
    _r3.i = _r3.i << 23;
    _r2.i = _r2.i + 1;
    _r0.i = _r0.i << _r2.i;
    _r0.i = _r0.i & _r5.i;
    _r0.i = _r0.i | _r3.i;
    goto label94;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Math_shiftIntBits___int_int(JAVA_INT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_shiftIntBits___int_int]
    XMLVM_ENTER_METHOD("java.lang.Math", "shiftIntBits", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.i = n1;
    _r6.i = n2;
    _r3.i = 0;
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("Math.java", 1400)
    if (_r6.i <= 0) goto label7;
    XMLVM_SOURCE_POSITION("Math.java", 1401)
    _r0.i = _r5.i << _r6.i;
    label6:;
    XMLVM_SOURCE_POSITION("Math.java", 1420)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label7:;
    XMLVM_SOURCE_POSITION("Math.java", 1404)
    _r0.i = -_r6.i;
    _r1.i = 2147483647;
    _r1.i = _r1.i & _r5.i;
    XMLVM_SOURCE_POSITION("Math.java", 1405)
    _r1.i = java_lang_Integer_numberOfLeadingZeros___int(_r1.i);
    _r2.i = 32;
    _r2.i = _r2.i - _r0.i;
    if (_r1.i <= _r2.i) goto label23;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Math.java", 1406)
    goto label6;
    label23:;
    XMLVM_SOURCE_POSITION("Math.java", 1408)
    _r1.i = _r5.i >> (0x1f & _r0.i);
    _r2.i = _r0.i - _r4.i;
    _r2.i = _r5.i >> (0x1f & _r2.i);
    _r2.i = _r2.i & 1;
    if (_r2.i != _r4.i) goto label60;
    XMLVM_SOURCE_POSITION("Math.java", 1409)
    _r2 = _r4;
    label34:;
    XMLVM_SOURCE_POSITION("Math.java", 1410)
    if (_r2.i == 0) goto label62;
    XMLVM_SOURCE_POSITION("Math.java", 1411)
    _r2.i = java_lang_Integer_numberOfTrailingZeros___int(_r5.i);
    _r3.i = _r0.i - _r4.i;
    if (_r2.i >= _r3.i) goto label46;
    XMLVM_SOURCE_POSITION("Math.java", 1412)
    _r1.i = _r1.i + 1;
    label46:;
    XMLVM_SOURCE_POSITION("Math.java", 1414)
    _r2.i = java_lang_Integer_numberOfTrailingZeros___int(_r5.i);
    _r0.i = _r0.i - _r4.i;
    if (_r2.i != _r0.i) goto label62;
    XMLVM_SOURCE_POSITION("Math.java", 1415)
    _r0.i = _r1.i & 1;
    if (_r0.i != _r4.i) goto label62;
    XMLVM_SOURCE_POSITION("Math.java", 1416)
    _r0.i = _r1.i + 1;
    goto label6;
    label60:;
    _r2 = _r3;
    goto label34;
    label62:;
    _r0 = _r1;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Math_shiftLongBits___long_long(JAVA_LONG n1, JAVA_LONG n2)
{
    XMLVM_CLASS_INIT(java_lang_Math)
    //XMLVM_BEGIN_WRAPPER[java_lang_Math_shiftLongBits___long_long]
    XMLVM_ENTER_METHOD("java.lang.Math", "shiftLongBits", "?")
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
    _r10.l = n1;
    _r12.l = n2;
    _r6.l = 0;
    _r8.l = 1;
    XMLVM_SOURCE_POSITION("Math.java", 1426)
    _r0.i = _r12.l > _r6.l ? 1 : (_r12.l == _r6.l ? 0 : -1);
    if (_r0.i <= 0) goto label12;
    XMLVM_SOURCE_POSITION("Math.java", 1427)
    _r0.i = (JAVA_INT) _r12.l;
    _r0.l = _r10.l << (0x3f & _r0.l);
    label11:;
    XMLVM_SOURCE_POSITION("Math.java", 1448)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label12:;
    XMLVM_SOURCE_POSITION("Math.java", 1430)
    _r0.l = -_r12.l;
    _r2.l = 9223372036854775807;
    _r2.l = _r2.l & _r10.l;
    XMLVM_SOURCE_POSITION("Math.java", 1431)
    _r2.i = java_lang_Long_numberOfLeadingZeros___long(_r2.l);
    _r2.l = (JAVA_LONG) _r2.i;
    _r4.l = 64;
    _r4.l = _r4.l - _r0.l;
    _r2.i = _r2.l > _r4.l ? 1 : (_r2.l == _r4.l ? 0 : -1);
    if (_r2.i <= 0) goto label33;
    _r0 = _r6;
    XMLVM_SOURCE_POSITION("Math.java", 1432)
    goto label11;
    label33:;
    XMLVM_SOURCE_POSITION("Math.java", 1434)
    _r2.i = (JAVA_INT) _r0.l;
    _r2.l = _r10.l >> (0x3f & _r2.l);
    _r4.l = _r0.l - _r8.l;
    _r4.i = (JAVA_INT) _r4.l;
    _r4.l = _r10.l >> (0x3f & _r4.l);
    _r4.l = _r4.l & _r8.l;
    _r4.i = _r4.l > _r8.l ? 1 : (_r4.l == _r8.l ? 0 : -1);
    if (_r4.i != 0) goto label80;
    XMLVM_SOURCE_POSITION("Math.java", 1435)
    _r4.i = 1;
    label47:;
    XMLVM_SOURCE_POSITION("Math.java", 1436)
    if (_r4.i == 0) goto label82;
    XMLVM_SOURCE_POSITION("Math.java", 1439)
    _r4.i = java_lang_Long_numberOfTrailingZeros___long(_r10.l);
    _r4.l = (JAVA_LONG) _r4.i;
    _r6.l = _r0.l - _r8.l;
    _r4.i = _r4.l > _r6.l ? 1 : (_r4.l == _r6.l ? 0 : -1);
    if (_r4.i >= 0) goto label61;
    XMLVM_SOURCE_POSITION("Math.java", 1440)
    _r2.l = _r2.l + _r8.l;
    label61:;
    XMLVM_SOURCE_POSITION("Math.java", 1442)
    _r4.i = java_lang_Long_numberOfTrailingZeros___long(_r10.l);
    _r4.l = (JAVA_LONG) _r4.i;
    _r0.l = _r0.l - _r8.l;
    _r0.i = _r4.l > _r0.l ? 1 : (_r4.l == _r0.l ? 0 : -1);
    if (_r0.i != 0) goto label82;
    XMLVM_SOURCE_POSITION("Math.java", 1443)
    _r0.l = _r2.l & _r8.l;
    _r0.i = _r0.l > _r8.l ? 1 : (_r0.l == _r8.l ? 0 : -1);
    if (_r0.i != 0) goto label82;
    XMLVM_SOURCE_POSITION("Math.java", 1444)
    _r0.l = _r2.l + _r8.l;
    goto label11;
    label80:;
    _r4.i = 0;
    goto label47;
    label82:;
    _r0 = _r2;
    goto label11;
    //XMLVM_END_WRAPPER
}

