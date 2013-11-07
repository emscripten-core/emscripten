#include "xmlvm.h"
#include "java_lang_Character.h"
#include "java_lang_Class.h"
#include "java_lang_Long_valueOfCache.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_System.h"

#include "java_lang_Long.h"

#define XMLVM_CURRENT_CLASS_NAME Long
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Long

__TIB_DEFINITION_java_lang_Long __TIB_java_lang_Long = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Long, // classInitializer
    "java.lang.Long", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Number;Ljava/lang/Comparable<Ljava/lang/Long;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Number, // extends
    sizeof(java_lang_Long), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Long;
JAVA_OBJECT __CLASS_java_lang_Long_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Long_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Long_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Long_serialVersionUID;
static JAVA_LONG _STATIC_java_lang_Long_MAX_VALUE;
static JAVA_LONG _STATIC_java_lang_Long_MIN_VALUE;
static JAVA_OBJECT _STATIC_java_lang_Long_TYPE;
static JAVA_INT _STATIC_java_lang_Long_SIZE;

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

void __INIT_java_lang_Long()
{
    staticInitializerLock(&__TIB_java_lang_Long);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Long.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Long.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Long);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Long.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Long.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Long.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Long")
        __INIT_IMPL_java_lang_Long();
    }
}

void __INIT_IMPL_java_lang_Long()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Number)
    __TIB_java_lang_Long.newInstanceFunc = __NEW_INSTANCE_java_lang_Long;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Long.vtable, __TIB_java_lang_Number.vtable, sizeof(__TIB_java_lang_Number.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Long.vtable[6] = (VTABLE_PTR) &java_lang_Long_byteValue__;
    __TIB_java_lang_Long.vtable[7] = (VTABLE_PTR) &java_lang_Long_doubleValue__;
    __TIB_java_lang_Long.vtable[1] = (VTABLE_PTR) &java_lang_Long_equals___java_lang_Object;
    __TIB_java_lang_Long.vtable[8] = (VTABLE_PTR) &java_lang_Long_floatValue__;
    __TIB_java_lang_Long.vtable[4] = (VTABLE_PTR) &java_lang_Long_hashCode__;
    __TIB_java_lang_Long.vtable[9] = (VTABLE_PTR) &java_lang_Long_intValue__;
    __TIB_java_lang_Long.vtable[10] = (VTABLE_PTR) &java_lang_Long_longValue__;
    __TIB_java_lang_Long.vtable[11] = (VTABLE_PTR) &java_lang_Long_shortValue__;
    __TIB_java_lang_Long.vtable[5] = (VTABLE_PTR) &java_lang_Long_toString__;
    __TIB_java_lang_Long.vtable[12] = (VTABLE_PTR) &java_lang_Long_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Long.numImplementedInterfaces = 2;
    __TIB_java_lang_Long.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Long.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Long.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Long.itableBegin = &__TIB_java_lang_Long.itable[0];
    __TIB_java_lang_Long.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Long.vtable[12];

    _STATIC_java_lang_Long_serialVersionUID = 4290774380558885855;
    _STATIC_java_lang_Long_MAX_VALUE = 9223372036854775807;
    _STATIC_java_lang_Long_MIN_VALUE = -9223372036854775808;
    _STATIC_java_lang_Long_TYPE = (java_lang_Class*) JAVA_NULL;
    _STATIC_java_lang_Long_SIZE = 64;

    __TIB_java_lang_Long.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Long.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Long.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Long.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Long.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Long.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Long.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Long.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Long = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Long);
    __TIB_java_lang_Long.clazz = __CLASS_java_lang_Long;
    __TIB_java_lang_Long.baseType = JAVA_NULL;
    __CLASS_java_lang_Long_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Long);
    __CLASS_java_lang_Long_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Long_1ARRAY);
    __CLASS_java_lang_Long_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Long_2ARRAY);
    java_lang_Long___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Long]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Long.classInitialized = 1;
}

void __DELETE_java_lang_Long(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Long]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Long(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Long*) me)->fields.java_lang_Long.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Long]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Long()
{    XMLVM_CLASS_INIT(java_lang_Long)
java_lang_Long* me = (java_lang_Long*) XMLVM_MALLOC(sizeof(java_lang_Long));
    me->tib = &__TIB_java_lang_Long;
    __INIT_INSTANCE_MEMBERS_java_lang_Long(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Long]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Long()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Long_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Long)
    return _STATIC_java_lang_Long_serialVersionUID;
}

void java_lang_Long_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Long)
_STATIC_java_lang_Long_serialVersionUID = v;
}

JAVA_LONG java_lang_Long_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Long)
    return _STATIC_java_lang_Long_MAX_VALUE;
}

void java_lang_Long_PUT_MAX_VALUE(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Long)
_STATIC_java_lang_Long_MAX_VALUE = v;
}

JAVA_LONG java_lang_Long_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Long)
    return _STATIC_java_lang_Long_MIN_VALUE;
}

void java_lang_Long_PUT_MIN_VALUE(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Long)
_STATIC_java_lang_Long_MIN_VALUE = v;
}

JAVA_OBJECT java_lang_Long_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Long)
    return _STATIC_java_lang_Long_TYPE;
}

void java_lang_Long_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Long)
_STATIC_java_lang_Long_TYPE = v;
}

JAVA_INT java_lang_Long_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Long)
    return _STATIC_java_lang_Long_SIZE;
}

void java_lang_Long_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Long)
_STATIC_java_lang_Long_SIZE = v;
}

void java_lang_Long___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Long", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Long.java", 54)
    _r0.i = 0;
    XMLVM_CLASS_INIT(long)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_long, _r0.i);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_SOURCE_POSITION("Long.java", 55)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Long_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Long.java", 31)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Long___INIT____long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long___INIT____long]
    XMLVM_ENTER_METHOD("java.lang.Long", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 75)
    XMLVM_CHECK_NPE(0)
    java_lang_Number___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Long.java", 76)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Long*) _r0.o)->fields.java_lang_Long.value_ = _r1.l;
    XMLVM_SOURCE_POSITION("Long.java", 77)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Long___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Long", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 89)
    _r0.l = java_lang_Long_parseLong___java_lang_String(_r3.o);
    XMLVM_CHECK_NPE(2)
    java_lang_Long___INIT____long(_r2.o, _r0.l);
    XMLVM_SOURCE_POSITION("Long.java", 90)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Long_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 94)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.i = (JAVA_INT) _r0.l;
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_compareTo___java_lang_Long(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_compareTo___java_lang_Long]
    XMLVM_ENTER_METHOD("java.lang.Long", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 111)
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_lang_Long*) _r4.o)->fields.java_lang_Long.value_;
    XMLVM_CHECK_NPE(5)
    _r2.l = ((java_lang_Long*) _r5.o)->fields.java_lang_Long.value_;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i <= 0) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_lang_Long*) _r4.o)->fields.java_lang_Long.value_;
    XMLVM_CHECK_NPE(5)
    _r2.l = ((java_lang_Long*) _r5.o)->fields.java_lang_Long.value_;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i >= 0) goto label20;
    _r0.i = -1;
    goto label9;
    label20:;
    _r0.i = 0;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_decode___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_decode___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Long", "decode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r8.o = n1;
    _r6.i = 16;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Long.java", 127)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(8)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r8.o)->tib->vtable[8])(_r8.o);
    XMLVM_SOURCE_POSITION("Long.java", 128)
    if (_r0.i != 0) goto label16;
    XMLVM_SOURCE_POSITION("Long.java", 129)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("Long.java", 131)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(8)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r8.o)->tib->vtable[6])(_r8.o, _r3.i);
    XMLVM_SOURCE_POSITION("Long.java", 132)
    _r2.i = 45;
    if (_r1.i != _r2.i) goto label35;
    _r2 = _r4;
    label25:;
    XMLVM_SOURCE_POSITION("Long.java", 133)
    if (_r2.i == 0) goto label46;
    XMLVM_SOURCE_POSITION("Long.java", 134)
    if (_r0.i != _r4.i) goto label37;
    XMLVM_SOURCE_POSITION("Long.java", 135)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label35:;
    _r2 = _r3;
    goto label25;
    label37:;
    XMLVM_SOURCE_POSITION("Long.java", 137)
    _r1.i = _r3.i + 1;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(8)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r8.o)->tib->vtable[6])(_r8.o, _r1.i);
    _r7 = _r3;
    _r3 = _r1;
    _r1 = _r7;
    label46:;
    XMLVM_SOURCE_POSITION("Long.java", 140)
    _r4.i = 10;
    _r5.i = 48;
    if (_r1.i != _r5.i) goto label99;
    XMLVM_SOURCE_POSITION("Long.java", 141)
    XMLVM_SOURCE_POSITION("Long.java", 142)
    _r1.i = _r3.i + 1;
    if (_r1.i != _r0.i) goto label63;
    XMLVM_SOURCE_POSITION("Long.java", 143)
    _r0.l = 0;
    _r0.o = java_lang_Long_valueOf___long(_r0.l);
    label62:;
    XMLVM_SOURCE_POSITION("Long.java", 163)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label63:;
    XMLVM_SOURCE_POSITION("Long.java", 145)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(8)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r8.o)->tib->vtable[6])(_r8.o, _r1.i);
    _r4.i = 120;
    if (_r3.i == _r4.i) goto label75;
    _r4.i = 88;
    if (_r3.i != _r4.i) goto label96;
    label75:;
    XMLVM_SOURCE_POSITION("Long.java", 146)
    if (_r1.i != _r0.i) goto label83;
    XMLVM_SOURCE_POSITION("Long.java", 147)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label83:;
    XMLVM_SOURCE_POSITION("Long.java", 149)
    _r0.i = _r1.i + 1;
    _r1 = _r0;
    _r0 = _r6;
    label87:;
    XMLVM_SOURCE_POSITION("Long.java", 150)
    XMLVM_SOURCE_POSITION("Long.java", 162)
    _r0.l = java_lang_Long_parse___java_lang_String_int_int_boolean(_r8.o, _r1.i, _r0.i, _r2.i);
    _r0.o = java_lang_Long_valueOf___long(_r0.l);
    goto label62;
    label96:;
    XMLVM_SOURCE_POSITION("Long.java", 152)
    _r0.i = 8;
    goto label87;
    label99:;
    XMLVM_SOURCE_POSITION("Long.java", 154)
    _r5.i = 35;
    if (_r1.i != _r5.i) goto label116;
    XMLVM_SOURCE_POSITION("Long.java", 155)
    if (_r3.i != _r0.i) goto label111;
    XMLVM_SOURCE_POSITION("Long.java", 156)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label111:;
    XMLVM_SOURCE_POSITION("Long.java", 158)
    _r0.i = _r3.i + 1;
    _r1 = _r0;
    _r0 = _r6;
    XMLVM_SOURCE_POSITION("Long.java", 159)
    goto label87;
    label116:;
    _r0 = _r4;
    _r1 = _r3;
    goto label87;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Long_doubleValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_doubleValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "doubleValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 168)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.d = (JAVA_DOUBLE) _r0.l;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Long_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Long", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 183)
    XMLVM_CLASS_INIT(java_lang_Long)
    _r0.i = XMLVM_ISA(_r5.o, __CLASS_java_lang_Long);
    if (_r0.i == 0) goto label16;
    XMLVM_SOURCE_POSITION("Long.java", 184)
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_lang_Long*) _r4.o)->fields.java_lang_Long.value_;
    _r5.o = _r5.o;
    XMLVM_CHECK_NPE(5)
    _r2.l = ((java_lang_Long*) _r5.o)->fields.java_lang_Long.value_;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i != 0) goto label16;
    _r0.i = 1;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0.i = 0;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Long_floatValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_floatValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "floatValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 189)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.f = (JAVA_FLOAT) _r0.l;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_getLong___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_getLong___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Long", "getLong", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = n1;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Long.java", 203)
    if (_r2.o == JAVA_NULL) goto label9;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r0.i != 0) goto label11;
    label9:;
    _r0 = _r1;
    label10:;
    XMLVM_SOURCE_POSITION("Long.java", 204)
    XMLVM_SOURCE_POSITION("Long.java", 213)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label11:;
    XMLVM_SOURCE_POSITION("Long.java", 206)
    _r0.o = java_lang_System_getProperty___java_lang_String(_r2.o);
    XMLVM_SOURCE_POSITION("Long.java", 207)
    if (_r0.o != JAVA_NULL) goto label19;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Long.java", 208)
    goto label10;
    label19:;
    XMLVM_TRY_BEGIN(w4223aaac15b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Long.java", 211)
    _r0.o = java_lang_Long_decode___java_lang_String(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4223aaac15b1c22)
    XMLVM_CATCH_END(w4223aaac15b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w4223aaac15b1c22)
    goto label10;
    label24:;
    java_lang_Thread* curThread_w4223aaac15b1c25 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4223aaac15b1c25->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r1;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_getLong___java_lang_String_long(JAVA_OBJECT n1, JAVA_LONG n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_getLong___java_lang_String_long]
    XMLVM_ENTER_METHOD("java.lang.Long", "getLong", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r1.o = n1;
    _r2.l = n2;
    XMLVM_SOURCE_POSITION("Long.java", 232)
    if (_r1.o == JAVA_NULL) goto label8;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r0.i != 0) goto label13;
    label8:;
    XMLVM_SOURCE_POSITION("Long.java", 233)
    _r0.o = java_lang_Long_valueOf___long(_r2.l);
    label12:;
    XMLVM_SOURCE_POSITION("Long.java", 242)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label13:;
    XMLVM_SOURCE_POSITION("Long.java", 235)
    _r0.o = java_lang_System_getProperty___java_lang_String(_r1.o);
    XMLVM_SOURCE_POSITION("Long.java", 236)
    if (_r0.o != JAVA_NULL) goto label24;
    XMLVM_SOURCE_POSITION("Long.java", 237)
    _r0.o = java_lang_Long_valueOf___long(_r2.l);
    goto label12;
    label24:;
    XMLVM_TRY_BEGIN(w4223aaac16b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Long.java", 240)
    _r0.o = java_lang_Long_decode___java_lang_String(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4223aaac16b1c22)
    XMLVM_CATCH_END(w4223aaac16b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w4223aaac16b1c22)
    goto label12;
    label29:;
    java_lang_Thread* curThread_w4223aaac16b1c25 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4223aaac16b1c25->fields.java_lang_Thread.xmlvmException_;
    _r0.o = java_lang_Long_valueOf___long(_r2.l);
    goto label12;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_getLong___java_lang_String_java_lang_Long(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_getLong___java_lang_String_java_lang_Long]
    XMLVM_ENTER_METHOD("java.lang.Long", "getLong", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Long.java", 261)
    if (_r1.o == JAVA_NULL) goto label8;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r0.i != 0) goto label10;
    label8:;
    _r0 = _r2;
    label9:;
    XMLVM_SOURCE_POSITION("Long.java", 262)
    XMLVM_SOURCE_POSITION("Long.java", 271)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;
    XMLVM_SOURCE_POSITION("Long.java", 264)
    _r0.o = java_lang_System_getProperty___java_lang_String(_r1.o);
    XMLVM_SOURCE_POSITION("Long.java", 265)
    if (_r0.o != JAVA_NULL) goto label18;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Long.java", 266)
    goto label9;
    label18:;
    XMLVM_TRY_BEGIN(w4223aaac17b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Long.java", 269)
    _r0.o = java_lang_Long_decode___java_lang_String(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4223aaac17b1c22)
    XMLVM_CATCH_END(w4223aaac17b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w4223aaac17b1c22)
    goto label9;
    label23:;
    java_lang_Thread* curThread_w4223aaac17b1c25 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4223aaac17b1c25->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r2;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Long", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 277)
    XMLVM_CHECK_NPE(5)
    _r0.l = ((java_lang_Long*) _r5.o)->fields.java_lang_Long.value_;
    XMLVM_CHECK_NPE(5)
    _r2.l = ((java_lang_Long*) _r5.o)->fields.java_lang_Long.value_;
    _r4.i = 32;
    _r2.l = ((JAVA_ULONG) _r2.l) >> (0x3f & ((JAVA_ULONG) _r4.l));
    _r0.l = _r0.l ^ _r2.l;
    _r0.i = (JAVA_INT) _r0.l;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_intValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_intValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "intValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 282)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.i = (JAVA_INT) _r0.l;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_longValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_longValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "longValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 292)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_parseLong___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_parseLong___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Long", "parseLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 307)
    _r0.i = 10;
    _r0.l = java_lang_Long_parseLong___java_lang_String_int(_r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_parseLong___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_parseLong___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Long", "parseLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = n1;
    _r5.i = n2;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Long.java", 328)
    if (_r4.o == JAVA_NULL) goto label10;
    _r0.i = 2;
    if (_r5.i < _r0.i) goto label10;
    XMLVM_SOURCE_POSITION("Long.java", 329)
    _r0.i = 36;
    if (_r5.i <= _r0.i) goto label16;
    label10:;
    XMLVM_SOURCE_POSITION("Long.java", 330)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("Long.java", 332)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    XMLVM_SOURCE_POSITION("Long.java", 333)
    if (_r0.i != 0) goto label28;
    XMLVM_SOURCE_POSITION("Long.java", 334)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_SOURCE_POSITION("Long.java", 336)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r4.o)->tib->vtable[6])(_r4.o, _r3.i);
    _r2.i = 45;
    if (_r1.i != _r2.i) goto label49;
    _r1.i = 1;
    label37:;
    XMLVM_SOURCE_POSITION("Long.java", 337)
    if (_r1.i == 0) goto label57;
    _r2.i = _r3.i + 1;
    if (_r2.i != _r0.i) goto label51;
    XMLVM_SOURCE_POSITION("Long.java", 338)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label49:;
    _r1 = _r3;
    goto label37;
    label51:;
    _r0 = _r2;
    label52:;
    XMLVM_SOURCE_POSITION("Long.java", 341)
    _r0.l = java_lang_Long_parse___java_lang_String_int_int_boolean(_r4.o, _r0.i, _r5.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label57:;
    _r0 = _r3;
    goto label52;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_parse___java_lang_String_int_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_parse___java_lang_String_int_int_boolean]
    XMLVM_ENTER_METHOD("java.lang.Long", "parse", "?")
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
    _r11.o = n1;
    _r12.i = n2;
    _r13.i = n3;
    _r14.i = n4;
    XMLVM_SOURCE_POSITION("Long.java", 346)
    _r0.l = -9223372036854775808;
    _r2.l = (JAVA_LONG) _r13.i;
    _r0.l = _r0.l / _r2.l;
    XMLVM_SOURCE_POSITION("Long.java", 347)
    _r2.l = 0;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(11)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r11.o)->tib->vtable[8])(_r11.o);
    _r4.l = (JAVA_LONG) _r4.i;
    label11:;
    XMLVM_SOURCE_POSITION("Long.java", 348)
    _r6.l = (JAVA_LONG) _r12.i;
    _r6.i = _r6.l > _r4.l ? 1 : (_r6.l == _r4.l ? 0 : -1);
    if (_r6.i < 0) goto label31;
    XMLVM_SOURCE_POSITION("Long.java", 362)
    if (_r14.i != 0) goto label79;
    XMLVM_SOURCE_POSITION("Long.java", 363)
    _r12.l = -_r2.l;
    _r0.l = 0;
    _r14.i = _r12.l > _r0.l ? 1 : (_r12.l == _r0.l ? 0 : -1);
    if (_r14.i >= 0) goto label77;
    XMLVM_SOURCE_POSITION("Long.java", 364)
    XMLVM_SOURCE_POSITION("Long.java", 365)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r12.o)
    label31:;
    XMLVM_SOURCE_POSITION("Long.java", 349)
    _r6.i = _r12.i + 1;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(11)
    _r12.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r11.o)->tib->vtable[6])(_r11.o, _r12.i);
    _r12.i = java_lang_Character_digit___char_int(_r12.i, _r13.i);
    XMLVM_SOURCE_POSITION("Long.java", 350)
    _r7.i = -1;
    if (_r12.i != _r7.i) goto label50;
    XMLVM_SOURCE_POSITION("Long.java", 351)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r12.o)
    label50:;
    XMLVM_SOURCE_POSITION("Long.java", 353)
    _r7.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r7.i <= 0) goto label60;
    XMLVM_SOURCE_POSITION("Long.java", 354)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r12.o)
    label60:;
    XMLVM_SOURCE_POSITION("Long.java", 356)
    _r7.l = (JAVA_LONG) _r13.i;
    _r7.l = _r7.l * _r2.l;
    _r9.l = (JAVA_LONG) _r12.i;
    _r7.l = _r7.l - _r9.l;
    _r12.i = _r7.l > _r2.l ? 1 : (_r7.l == _r2.l ? 0 : -1);
    if (_r12.i <= 0) goto label74;
    XMLVM_SOURCE_POSITION("Long.java", 357)
    XMLVM_SOURCE_POSITION("Long.java", 358)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r12.o)
    label74:;
    _r2 = _r7;
    _r12 = _r6;
    XMLVM_SOURCE_POSITION("Long.java", 360)
    goto label11;
    label77:;
    _r11 = _r12;
    label78:;
    XMLVM_SOURCE_POSITION("Long.java", 368)
    XMLVM_EXIT_METHOD()
    return _r11.l;
    label79:;
    _r11 = _r2;
    goto label78;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Long_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Long", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 373)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.i = (JAVA_INT) _r0.l;
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toBinaryString___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toBinaryString___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "toBinaryString", "?")
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
    _r9.l = n1;
    _r4.l = 0;
    _r8.i = 1;
    XMLVM_SOURCE_POSITION("Long.java", 385)
    XMLVM_SOURCE_POSITION("Long.java", 388)
    _r0.i = _r9.l > _r4.l ? 1 : (_r9.l == _r4.l ? 0 : -1);
    if (_r0.i >= 0) goto label44;
    XMLVM_SOURCE_POSITION("Long.java", 389)
    _r0.i = 64;
    label9:;
    XMLVM_SOURCE_POSITION("Long.java", 396)
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    _r2 = _r9;
    label12:;
    XMLVM_SOURCE_POSITION("Long.java", 398)
    _r0.i = _r0.i + -1;
    _r4.l = 1;
    _r4.l = _r4.l & _r2.l;
    _r6.l = 48;
    _r4.l = _r4.l + _r6.l;
    _r4.i = (JAVA_INT) _r4.l;
    _r4.i = _r4.i & 0xffff;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r4.i;
    XMLVM_SOURCE_POSITION("Long.java", 399)
    _r2.l = _r2.l >> (0x3f & _r8.l);
    if (_r0.i > 0) goto label12;
    XMLVM_SOURCE_POSITION("Long.java", 400)
    XMLVM_SOURCE_POSITION("Long.java", 401)
    _r0.o = __NEW_java_lang_String();
    _r2.i = 0;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r2.i, _r3.i, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label35:;
    XMLVM_SOURCE_POSITION("Long.java", 392)
    _r2.i = _r2.i + 1;
    label37:;
    XMLVM_SOURCE_POSITION("Long.java", 391)
    _r0.l = _r0.l >> (0x3f & _r8.l);
    _r3.i = _r0.l > _r4.l ? 1 : (_r0.l == _r4.l ? 0 : -1);
    if (_r3.i != 0) goto label35;
    _r0 = _r2;
    goto label9;
    label44:;
    _r0 = _r9;
    _r2 = _r8;
    goto label37;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toHexString___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toHexString___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "toHexString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r7.l = n1;
    _r4.l = 0;
    _r6.i = 4;
    XMLVM_SOURCE_POSITION("Long.java", 414)
    _r0.i = 1;
    _r1.i = _r7.l > _r4.l ? 1 : (_r7.l == _r4.l ? 0 : -1);
    if (_r1.i >= 0) goto label54;
    XMLVM_SOURCE_POSITION("Long.java", 417)
    XMLVM_SOURCE_POSITION("Long.java", 418)
    _r0.i = 16;
    label10:;
    XMLVM_SOURCE_POSITION("Long.java", 425)
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    _r2 = _r7;
    label13:;
    XMLVM_SOURCE_POSITION("Long.java", 427)
    _r4.l = 15;
    _r4.l = _r4.l & _r2.l;
    _r4.i = (JAVA_INT) _r4.l;
    _r5.i = 9;
    if (_r4.i <= _r5.i) goto label51;
    XMLVM_SOURCE_POSITION("Long.java", 428)
    XMLVM_SOURCE_POSITION("Long.java", 429)
    _r5.i = 10;
    _r4.i = _r4.i - _r5.i;
    _r4.i = _r4.i + 97;
    label26:;
    XMLVM_SOURCE_POSITION("Long.java", 433)
    _r0.i = _r0.i + -1;
    _r4.i = _r4.i & 0xffff;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r4.i;
    XMLVM_SOURCE_POSITION("Long.java", 434)
    _r2.l = _r2.l >> (0x3f & _r6.l);
    if (_r0.i > 0) goto label13;
    XMLVM_SOURCE_POSITION("Long.java", 435)
    XMLVM_SOURCE_POSITION("Long.java", 436)
    _r0.o = __NEW_java_lang_String();
    _r2.i = 0;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r2.i, _r3.i, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label42:;
    XMLVM_SOURCE_POSITION("Long.java", 421)
    _r2.i = _r2.i + 1;
    label44:;
    XMLVM_SOURCE_POSITION("Long.java", 420)
    _r0.l = _r0.l >> (0x3f & _r6.l);
    _r3.i = _r0.l > _r4.l ? 1 : (_r0.l == _r4.l ? 0 : -1);
    if (_r3.i != 0) goto label42;
    _r0 = _r2;
    goto label10;
    label51:;
    XMLVM_SOURCE_POSITION("Long.java", 431)
    _r4.i = _r4.i + 48;
    goto label26;
    label54:;
    _r2 = _r0;
    _r0 = _r7;
    goto label44;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toOctalString___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toOctalString___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "toOctalString", "?")
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
    _r9.l = n1;
    _r4.l = 0;
    _r8.i = 3;
    XMLVM_SOURCE_POSITION("Long.java", 448)
    _r0.i = 1;
    _r1.i = _r9.l > _r4.l ? 1 : (_r9.l == _r4.l ? 0 : -1);
    if (_r1.i >= 0) goto label45;
    XMLVM_SOURCE_POSITION("Long.java", 451)
    XMLVM_SOURCE_POSITION("Long.java", 452)
    _r0.i = 22;
    label10:;
    XMLVM_SOURCE_POSITION("Long.java", 459)
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    _r2 = _r9;
    label13:;
    XMLVM_SOURCE_POSITION("Long.java", 461)
    _r0.i = _r0.i + -1;
    _r4.l = 7;
    _r4.l = _r4.l & _r2.l;
    _r6.l = 48;
    _r4.l = _r4.l + _r6.l;
    _r4.i = (JAVA_INT) _r4.l;
    _r4.i = _r4.i & 0xffff;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r4.i;
    XMLVM_SOURCE_POSITION("Long.java", 462)
    _r2.l = ((JAVA_ULONG) _r2.l) >> (0x3f & ((JAVA_ULONG) _r8.l));
    if (_r0.i > 0) goto label13;
    XMLVM_SOURCE_POSITION("Long.java", 463)
    XMLVM_SOURCE_POSITION("Long.java", 464)
    _r0.o = __NEW_java_lang_String();
    _r2.i = 0;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r2.i, _r3.i, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    XMLVM_SOURCE_POSITION("Long.java", 455)
    _r2.i = _r2.i + 1;
    label38:;
    XMLVM_SOURCE_POSITION("Long.java", 454)
    _r0.l = ((JAVA_ULONG) _r0.l) >> (0x3f & ((JAVA_ULONG) _r8.l));
    _r3.i = _r0.l > _r4.l ? 1 : (_r0.l == _r4.l ? 0 : -1);
    if (_r3.i != 0) goto label36;
    _r0 = _r2;
    goto label10;
    label45:;
    _r2 = _r0;
    _r0 = _r9;
    goto label38;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toString__]
    XMLVM_ENTER_METHOD("java.lang.Long", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Long.java", 469)
    XMLVM_CHECK_NPE(2)
    _r0.l = ((java_lang_Long*) _r2.o)->fields.java_lang_Long.value_;
    _r0.o = java_lang_Long_toString___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toString___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toString___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 482)
    _r0.i = 10;
    _r0.o = java_lang_Long_toString___long_int(_r1.l, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_toString___long_int(JAVA_LONG n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_toString___long_int]
    XMLVM_ENTER_METHOD("java.lang.Long", "toString", "?")
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
    _r13.l = n1;
    _r15.i = n2;
    _r12.i = 10;
    _r2.i = 2;
    _r4.i = 1;
    _r10.l = 0;
    _r9.i = 0;
    XMLVM_SOURCE_POSITION("Long.java", 500)
    if (_r15.i < _r2.i) goto label13;
    _r0.i = 36;
    if (_r15.i <= _r0.i) goto label87;
    label13:;
    _r0 = _r12;
    label14:;
    XMLVM_SOURCE_POSITION("Long.java", 501)
    XMLVM_SOURCE_POSITION("Long.java", 503)
    _r1.i = _r13.l > _r10.l ? 1 : (_r13.l == _r10.l ? 0 : -1);
    if (_r1.i != 0) goto label21;
    XMLVM_SOURCE_POSITION("Long.java", 504)
    // "0"
    _r0.o = xmlvm_create_java_string_from_pool(70);
    label20:;
    XMLVM_SOURCE_POSITION("Long.java", 531)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label21:;
    XMLVM_SOURCE_POSITION("Long.java", 509)
    _r1.i = _r13.l > _r10.l ? 1 : (_r13.l == _r10.l ? 0 : -1);
    if (_r1.i >= 0) goto label75;
    _r1 = _r4;
    label26:;
    XMLVM_SOURCE_POSITION("Long.java", 510)
    if (_r1.i != 0) goto label83;
    XMLVM_SOURCE_POSITION("Long.java", 512)
    _r2.l = -_r13.l;
    _r5 = _r13;
    label30:;
    XMLVM_SOURCE_POSITION("Long.java", 514)
    _r7.l = (JAVA_LONG) _r0.i;
    _r5.l = _r5.l / _r7.l;
    _r7.i = _r5.l > _r10.l ? 1 : (_r5.l == _r10.l ? 0 : -1);
    if (_r7.i != 0) goto label77;
    XMLVM_SOURCE_POSITION("Long.java", 518)
    XMLVM_CLASS_INIT(char)
    _r5.o = XMLVMArray_createSingleDimension(__CLASS_char, _r4.i);
    label38:;
    XMLVM_SOURCE_POSITION("Long.java", 520)
    _r6.l = (JAVA_LONG) _r0.i;
    _r6.l = _r2.l % _r6.l;
    _r6.i = (JAVA_INT) _r6.l;
    _r6.i = _r9.i - _r6.i;
    _r7.i = 9;
    if (_r6.i <= _r7.i) goto label80;
    XMLVM_SOURCE_POSITION("Long.java", 521)
    XMLVM_SOURCE_POSITION("Long.java", 522)
    _r6.i = _r6.i - _r12.i;
    _r6.i = _r6.i + 97;
    label51:;
    XMLVM_SOURCE_POSITION("Long.java", 526)
    _r4.i = _r4.i + -1;
    _r6.i = _r6.i & 0xffff;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r4.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r6.i;
    XMLVM_SOURCE_POSITION("Long.java", 527)
    _r6.l = (JAVA_LONG) _r0.i;
    _r2.l = _r2.l / _r6.l;
    _r6.i = _r2.l > _r10.l ? 1 : (_r2.l == _r10.l ? 0 : -1);
    if (_r6.i != 0) goto label38;
    XMLVM_SOURCE_POSITION("Long.java", 528)
    if (_r1.i == 0) goto label68;
    XMLVM_SOURCE_POSITION("Long.java", 529)
    _r0.i = 45;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r9.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r0.i;
    label68:;
    _r0.o = __NEW_java_lang_String();
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r9.i, _r1.i, _r5.o);
    goto label20;
    label75:;
    _r1 = _r9;
    goto label26;
    label77:;
    XMLVM_SOURCE_POSITION("Long.java", 515)
    _r4.i = _r4.i + 1;
    goto label30;
    label80:;
    XMLVM_SOURCE_POSITION("Long.java", 524)
    _r6.i = _r6.i + 48;
    goto label51;
    label83:;
    _r4 = _r2;
    _r5 = _r13;
    _r2 = _r13;
    goto label30;
    label87:;
    _r0 = _r15;
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Long", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 547)
    _r0.l = java_lang_Long_parseLong___java_lang_String(_r2.o);
    _r0.o = java_lang_Long_valueOf___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_valueOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Long", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("Long.java", 569)
    _r0.l = java_lang_Long_parseLong___java_lang_String_int(_r2.o, _r3.i);
    _r0.o = java_lang_Long_valueOf___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_highestOneBit___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_highestOneBit___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "highestOneBit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.l = n1;
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("Long.java", 584)
    _r0.l = _r6.l >> (0x3f & _r4.l);
    _r0.l = _r0.l | _r6.l;
    _r2.i = 2;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 4;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 8;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 16;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 32;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.l = ((JAVA_ULONG) _r0.l) >> (0x3f & ((JAVA_ULONG) _r4.l));
    _r4.l = -1;
    _r2.l = _r2.l ^ _r4.l;
    _r0.l = _r0.l & _r2.l;
    XMLVM_SOURCE_POSITION("Long.java", 590)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_lowestOneBit___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_lowestOneBit___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "lowestOneBit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 605)
    _r0.l = -_r2.l;
    _r0.l = _r0.l & _r2.l;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_numberOfLeadingZeros___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_numberOfLeadingZeros___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "numberOfLeadingZeros", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 618)
    _r0.i = 1;
    _r0.l = _r4.l >> (0x3f & _r0.l);
    _r0.l = _r0.l | _r4.l;
    _r2.i = 2;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 4;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 8;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 16;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.i = 32;
    _r2.l = _r0.l >> (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    _r2.l = -1;
    _r0.l = _r0.l ^ _r2.l;
    XMLVM_SOURCE_POSITION("Long.java", 624)
    _r0.i = java_lang_Long_bitCount___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_numberOfTrailingZeros___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_numberOfTrailingZeros___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "numberOfTrailingZeros", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 637)
    _r0.l = -_r4.l;
    _r0.l = _r0.l & _r4.l;
    _r2.l = 1;
    _r0.l = _r0.l - _r2.l;
    _r0.i = java_lang_Long_bitCount___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_bitCount___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_bitCount___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "bitCount", "?")
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
    _r12.l = n1;
    _r10.l = 6148914691236517205;
    _r8.l = 3689348814741910323;
    _r7.i = 252645135;
    _r6.i = 16711935;
    _r5.i = 65535;
    XMLVM_SOURCE_POSITION("Long.java", 650)
    _r0.l = _r12.l & _r10.l;
    _r2.i = 1;
    _r2.l = _r12.l >> (0x3f & _r2.l);
    _r2.l = _r2.l & _r10.l;
    _r0.l = _r0.l + _r2.l;
    _r2.l = _r0.l & _r8.l;
    _r4.i = 2;
    _r0.l = _r0.l >> (0x3f & _r4.l);
    _r0.l = _r0.l & _r8.l;
    _r0.l = _r0.l + _r2.l;
    _r2.i = 32;
    _r2.l = ((JAVA_ULONG) _r0.l) >> (0x3f & ((JAVA_ULONG) _r2.l));
    _r0.l = _r0.l + _r2.l;
    _r0.i = (JAVA_INT) _r0.l;
    _r1.i = _r0.i & _r7.i;
    _r0.i = _r0.i >> 4;
    _r0.i = _r0.i & _r7.i;
    _r0.i = _r0.i + _r1.i;
    _r1.i = _r0.i & _r6.i;
    _r0.i = _r0.i >> 8;
    _r0.i = _r0.i & _r6.i;
    _r0.i = _r0.i + _r1.i;
    _r1.i = _r0.i & _r5.i;
    _r0.i = _r0.i >> 16;
    _r0.i = _r0.i & _r5.i;
    _r0.i = _r0.i + _r1.i;
    XMLVM_SOURCE_POSITION("Long.java", 657)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_rotateLeft___long_int(JAVA_LONG n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_rotateLeft___long_int]
    XMLVM_ENTER_METHOD("java.lang.Long", "rotateLeft", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.l = n1;
    _r6.i = n2;
    XMLVM_SOURCE_POSITION("Long.java", 672)
    if (_r6.i != 0) goto label4;
    _r0 = _r4;
    label3:;
    XMLVM_SOURCE_POSITION("Long.java", 673)
    XMLVM_SOURCE_POSITION("Long.java", 680)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label4:;
    _r0.l = _r4.l << (0x3f & _r6.l);
    _r2.i = -_r6.i;
    _r2.l = ((JAVA_ULONG) _r4.l) >> (0x3f & ((JAVA_ULONG) _r2.l));
    _r0.l = _r0.l | _r2.l;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_rotateRight___long_int(JAVA_LONG n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_rotateRight___long_int]
    XMLVM_ENTER_METHOD("java.lang.Long", "rotateRight", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.l = n1;
    _r6.i = n2;
    XMLVM_SOURCE_POSITION("Long.java", 696)
    if (_r6.i != 0) goto label4;
    _r0 = _r4;
    label3:;
    XMLVM_SOURCE_POSITION("Long.java", 697)
    XMLVM_SOURCE_POSITION("Long.java", 704)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label4:;
    _r0.l = ((JAVA_ULONG) _r4.l) >> (0x3f & ((JAVA_ULONG) _r6.l));
    _r2.i = -_r6.i;
    _r2.l = _r4.l << (0x3f & _r2.l);
    _r0.l = _r0.l | _r2.l;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_reverseBytes___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_reverseBytes___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "reverseBytes", "?")
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
    _r15.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 716)
    _r0.i = 56;
    _r0.l = ((JAVA_ULONG) _r15.l) >> (0x3f & ((JAVA_ULONG) _r0.l));
    _r2.i = 40;
    _r2.l = ((JAVA_ULONG) _r15.l) >> (0x3f & ((JAVA_ULONG) _r2.l));
    _r4.l = 65280;
    _r2.l = _r2.l & _r4.l;
    _r4.i = 24;
    _r4.l = ((JAVA_ULONG) _r15.l) >> (0x3f & ((JAVA_ULONG) _r4.l));
    _r6.l = 16711680;
    _r4.l = _r4.l & _r6.l;
    _r6.i = 8;
    _r6.l = ((JAVA_ULONG) _r15.l) >> (0x3f & ((JAVA_ULONG) _r6.l));
    _r8.l = 4278190080;
    _r6.l = _r6.l & _r8.l;
    _r8.l = 4278190080;
    _r8.l = _r8.l & _r15.l;
    _r10.i = 8;
    _r8.l = _r8.l << (0x3f & _r10.l);
    _r10.l = 16711680;
    _r10.l = _r10.l & _r15.l;
    _r12.i = 24;
    _r10.l = _r10.l << (0x3f & _r12.l);
    _r12.l = 65280;
    _r12.l = _r12.l & _r15.l;
    _r14.i = 40;
    _r12.l = _r12.l << (0x3f & _r14.l);
    _r14.i = 56;
    _r15.l = _r15.l << (0x3f & _r14.l);
    _r15.l = _r15.l | _r12.l;
    _r15.l = _r15.l | _r10.l;
    _r15.l = _r15.l | _r8.l;
    _r15.l = _r15.l | _r6.l;
    _r15.l = _r15.l | _r4.l;
    _r15.l = _r15.l | _r2.l;
    _r15.l = _r15.l | _r0.l;
    XMLVM_SOURCE_POSITION("Long.java", 724)
    XMLVM_EXIT_METHOD()
    return _r15.l;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Long_reverse___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_reverse___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "reverse", "?")
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
    _r12.l = n1;
    _r10.l = 3689348814741910323;
    _r8.l = 1085102592571150095;
    _r7.i = 4;
    _r6.i = 2;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Long.java", 737)
    _r0.l = 6148914691236517205;
    _r0.l = _r0.l & _r12.l;
    _r0.l = _r0.l << (0x3f & _r2.l);
    _r2.l = _r12.l >> (0x3f & _r2.l);
    _r4.l = 6148914691236517205;
    _r2.l = _r2.l & _r4.l;
    _r0.l = _r0.l | _r2.l;
    _r2.l = _r0.l & _r10.l;
    _r2.l = _r2.l << (0x3f & _r6.l);
    _r0.l = _r0.l >> (0x3f & _r6.l);
    _r0.l = _r0.l & _r10.l;
    _r0.l = _r0.l | _r2.l;
    _r2.l = _r0.l & _r8.l;
    _r2.l = _r2.l << (0x3f & _r7.l);
    _r0.l = _r0.l >> (0x3f & _r7.l);
    _r0.l = _r0.l & _r8.l;
    _r0.l = _r0.l | _r2.l;
    XMLVM_SOURCE_POSITION("Long.java", 743)
    _r0.l = java_lang_Long_reverseBytes___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_signum___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_signum___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "signum", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.l = n1;
    _r1.l = 0;
    XMLVM_SOURCE_POSITION("Long.java", 757)
    _r0.i = _r3.l > _r1.l ? 1 : (_r3.l == _r1.l ? 0 : -1);
    if (_r0.i != 0) goto label8;
    _r0.i = 0;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = _r3.l > _r1.l ? 1 : (_r3.l == _r1.l ? 0 : -1);
    if (_r0.i >= 0) goto label14;
    _r0.i = -1;
    goto label7;
    label14:;
    _r0.i = 1;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Long_valueOf___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_Long)
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_valueOf___long]
    XMLVM_ENTER_METHOD("java.lang.Long", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.l = n1;
    XMLVM_SOURCE_POSITION("Long.java", 773)
    _r0.l = -128;
    _r0.i = _r2.l > _r0.l ? 1 : (_r2.l == _r0.l ? 0 : -1);
    if (_r0.i < 0) goto label12;
    _r0.l = 127;
    _r0.i = _r2.l > _r0.l ? 1 : (_r2.l == _r0.l ? 0 : -1);
    if (_r0.i <= 0) goto label18;
    label12:;
    XMLVM_SOURCE_POSITION("Long.java", 774)
    _r0.o = __NEW_java_lang_Long();
    XMLVM_CHECK_NPE(0)
    java_lang_Long___INIT____long(_r0.o, _r2.l);
    label17:;
    XMLVM_SOURCE_POSITION("Long.java", 776)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label18:;
    _r0.o = java_lang_Long_valueOfCache_GET_CACHE();
    _r1.i = (JAVA_INT) _r2.l;
    _r1.i = _r1.i + 128;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Long_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Long_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Long", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Long.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Long_compareTo___java_lang_Long(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

