#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"

#include "java_lang_Byte.h"

#define XMLVM_CURRENT_CLASS_NAME Byte
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Byte

__TIB_DEFINITION_java_lang_Byte __TIB_java_lang_Byte = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Byte, // classInitializer
    "java.lang.Byte", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Number;Ljava/lang/Comparable<Ljava/lang/Byte;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Number, // extends
    sizeof(java_lang_Byte), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Byte;
JAVA_OBJECT __CLASS_java_lang_Byte_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Byte_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Byte_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Byte_serialVersionUID;
static JAVA_BYTE _STATIC_java_lang_Byte_MAX_VALUE;
static JAVA_BYTE _STATIC_java_lang_Byte_MIN_VALUE;
static JAVA_INT _STATIC_java_lang_Byte_SIZE;
static JAVA_OBJECT _STATIC_java_lang_Byte_TYPE;
static JAVA_OBJECT _STATIC_java_lang_Byte_CACHE;

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

void __INIT_java_lang_Byte()
{
    staticInitializerLock(&__TIB_java_lang_Byte);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Byte.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Byte.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Byte);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Byte.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Byte.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Byte.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Byte")
        __INIT_IMPL_java_lang_Byte();
    }
}

void __INIT_IMPL_java_lang_Byte()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Number)
    __TIB_java_lang_Byte.newInstanceFunc = __NEW_INSTANCE_java_lang_Byte;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Byte.vtable, __TIB_java_lang_Number.vtable, sizeof(__TIB_java_lang_Number.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Byte.vtable[6] = (VTABLE_PTR) &java_lang_Byte_byteValue__;
    __TIB_java_lang_Byte.vtable[7] = (VTABLE_PTR) &java_lang_Byte_doubleValue__;
    __TIB_java_lang_Byte.vtable[1] = (VTABLE_PTR) &java_lang_Byte_equals___java_lang_Object;
    __TIB_java_lang_Byte.vtable[8] = (VTABLE_PTR) &java_lang_Byte_floatValue__;
    __TIB_java_lang_Byte.vtable[4] = (VTABLE_PTR) &java_lang_Byte_hashCode__;
    __TIB_java_lang_Byte.vtable[9] = (VTABLE_PTR) &java_lang_Byte_intValue__;
    __TIB_java_lang_Byte.vtable[10] = (VTABLE_PTR) &java_lang_Byte_longValue__;
    __TIB_java_lang_Byte.vtable[11] = (VTABLE_PTR) &java_lang_Byte_shortValue__;
    __TIB_java_lang_Byte.vtable[5] = (VTABLE_PTR) &java_lang_Byte_toString__;
    __TIB_java_lang_Byte.vtable[12] = (VTABLE_PTR) &java_lang_Byte_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Byte.numImplementedInterfaces = 2;
    __TIB_java_lang_Byte.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Byte.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Byte.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Byte.itableBegin = &__TIB_java_lang_Byte.itable[0];
    __TIB_java_lang_Byte.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Byte.vtable[12];

    _STATIC_java_lang_Byte_serialVersionUID = -7183698231559129828;
    _STATIC_java_lang_Byte_MAX_VALUE = 127;
    _STATIC_java_lang_Byte_MIN_VALUE = -128;
    _STATIC_java_lang_Byte_SIZE = 8;
    _STATIC_java_lang_Byte_TYPE = (java_lang_Class*) JAVA_NULL;
    _STATIC_java_lang_Byte_CACHE = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_lang_Byte.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Byte.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Byte.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Byte.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Byte.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Byte.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Byte.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Byte.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Byte = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Byte);
    __TIB_java_lang_Byte.clazz = __CLASS_java_lang_Byte;
    __TIB_java_lang_Byte.baseType = JAVA_NULL;
    __CLASS_java_lang_Byte_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Byte);
    __CLASS_java_lang_Byte_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Byte_1ARRAY);
    __CLASS_java_lang_Byte_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Byte_2ARRAY);
    java_lang_Byte___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Byte]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Byte.classInitialized = 1;
}

void __DELETE_java_lang_Byte(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Byte]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Byte(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Byte*) me)->fields.java_lang_Byte.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Byte]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Byte()
{    XMLVM_CLASS_INIT(java_lang_Byte)
java_lang_Byte* me = (java_lang_Byte*) XMLVM_MALLOC(sizeof(java_lang_Byte));
    me->tib = &__TIB_java_lang_Byte;
    __INIT_INSTANCE_MEMBERS_java_lang_Byte(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Byte]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Byte()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Byte_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_serialVersionUID;
}

void java_lang_Byte_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_serialVersionUID = v;
}

JAVA_BYTE java_lang_Byte_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_MAX_VALUE;
}

void java_lang_Byte_PUT_MAX_VALUE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_MAX_VALUE = v;
}

JAVA_BYTE java_lang_Byte_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_MIN_VALUE;
}

void java_lang_Byte_PUT_MIN_VALUE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_MIN_VALUE = v;
}

JAVA_INT java_lang_Byte_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_SIZE;
}

void java_lang_Byte_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_SIZE = v;
}

JAVA_OBJECT java_lang_Byte_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_TYPE;
}

void java_lang_Byte_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_TYPE = v;
}

JAVA_OBJECT java_lang_Byte_GET_CACHE()
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    return _STATIC_java_lang_Byte_CACHE;
}

void java_lang_Byte_PUT_CACHE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
_STATIC_java_lang_Byte_CACHE = v;
}

void java_lang_Byte___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Byte", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Byte.java", 56)
    _r0.i = 0;
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 57)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Byte_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 65)
    _r0.i = 256;
    XMLVM_CLASS_INIT(java_lang_Byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Byte, _r0.i);
    java_lang_Byte_PUT_CACHE( _r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 25)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Byte___INIT____byte(JAVA_OBJECT me, JAVA_BYTE n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte___INIT____byte]
    XMLVM_ENTER_METHOD("java.lang.Byte", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 73)
    XMLVM_CHECK_NPE(0)
    java_lang_Number___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 74)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Byte*) _r0.o)->fields.java_lang_Byte.value_ = _r1.i;
    XMLVM_SOURCE_POSITION("Byte.java", 75)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Byte___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Byte", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 87)
    _r0.i = java_lang_Byte_parseByte___java_lang_String(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_Byte___INIT____byte(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("Byte.java", 88)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Byte_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 97)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Byte_compareTo___java_lang_Byte(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_compareTo___java_lang_Byte]
    XMLVM_ENTER_METHOD("java.lang.Byte", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 114)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Byte*) _r2.o)->fields.java_lang_Byte.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Byte*) _r3.o)->fields.java_lang_Byte.value_;
    if (_r0.i <= _r1.i) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Byte*) _r2.o)->fields.java_lang_Byte.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Byte*) _r3.o)->fields.java_lang_Byte.value_;
    if (_r0.i >= _r1.i) goto label16;
    _r0.i = -1;
    goto label7;
    label16:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_decode___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_decode___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Byte", "decode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 130)
    _r0.o = java_lang_Integer_decode___java_lang_String(_r2.o);
    //java_lang_Integer_intValue__[9]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Integer*) _r0.o)->tib->vtable[9])(_r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 131)
    _r1.i = (_r0.i << 24) >> 24;
    if (_r1.i != _r0.i) goto label16;
    XMLVM_SOURCE_POSITION("Byte.java", 132)
    XMLVM_SOURCE_POSITION("Byte.java", 133)
    _r0.o = java_lang_Byte_valueOf___byte(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    XMLVM_SOURCE_POSITION("Byte.java", 135)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Byte_doubleValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_doubleValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "doubleValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 140)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Byte*) _r2.o)->fields.java_lang_Byte.value_;
    _r0.d = (JAVA_DOUBLE) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Byte_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Byte", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 155)
    if (_r3.o == _r2.o) goto label16;
    XMLVM_CLASS_INIT(java_lang_Byte)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_lang_Byte);
    if (_r0.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Byte.java", 156)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Byte*) _r2.o)->fields.java_lang_Byte.value_;
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Byte*) _r3.o)->fields.java_lang_Byte.value_;
    if (_r0.i == _r1.i) goto label16;
    label14:;
    _r0.i = 0;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0.i = 1;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Byte_floatValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_floatValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "floatValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 161)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Byte_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 166)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Byte_intValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_intValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "intValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 171)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Byte_longValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_longValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "longValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 176)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Byte*) _r2.o)->fields.java_lang_Byte.value_;
    _r0.l = (JAVA_LONG) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Byte_parseByte___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_parseByte___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Byte", "parseByte", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 191)
    _r0.i = java_lang_Integer_parseInt___java_lang_String(_r2.o);
    XMLVM_SOURCE_POSITION("Byte.java", 192)
    _r1.i = (_r0.i << 24) >> 24;
    if (_r1.i != _r0.i) goto label8;
    XMLVM_SOURCE_POSITION("Byte.java", 193)
    XMLVM_SOURCE_POSITION("Byte.java", 194)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label8:;
    XMLVM_SOURCE_POSITION("Byte.java", 196)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Byte_parseByte___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_parseByte___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Byte", "parseByte", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("Byte.java", 217)
    _r0.i = java_lang_Integer_parseInt___java_lang_String_int(_r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("Byte.java", 218)
    _r1.i = (_r0.i << 24) >> 24;
    if (_r1.i != _r0.i) goto label8;
    XMLVM_SOURCE_POSITION("Byte.java", 219)
    XMLVM_SOURCE_POSITION("Byte.java", 220)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label8:;
    XMLVM_SOURCE_POSITION("Byte.java", 222)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Byte_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 227)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_toString__]
    XMLVM_ENTER_METHOD("java.lang.Byte", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Byte.java", 232)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Byte*) _r1.o)->fields.java_lang_Byte.value_;
    _r0.o = java_lang_Integer_toString___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_toString___byte(JAVA_BYTE n1)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_toString___byte]
    XMLVM_ENTER_METHOD("java.lang.Byte", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 244)
    _r0.o = java_lang_Integer_toString___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Byte", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 260)
    _r0.i = java_lang_Byte_parseByte___java_lang_String(_r1.o);
    _r0.o = java_lang_Byte_valueOf___byte(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_valueOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Byte", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Byte.java", 282)
    _r0.i = java_lang_Byte_parseByte___java_lang_String_int(_r1.o, _r2.i);
    _r0.o = java_lang_Byte_valueOf___byte(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Byte_valueOf___byte(JAVA_BYTE n1)
{
    XMLVM_CLASS_INIT(java_lang_Byte)
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_valueOf___byte]
    XMLVM_ENTER_METHOD("java.lang.Byte", "valueOf", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 298)
    _r0.o = java_lang_Byte_GET_CACHE();
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_SOURCE_POSITION("Byte.java", 299)
    _r1.i = -128;
    _r1.i = _r4.i - _r1.i;
    XMLVM_TRY_BEGIN(w5361aaac26b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("Byte.java", 300)
    _r2.o = java_lang_Byte_GET_CACHE();
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("Byte.java", 301)
    if (_r2.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w5361aaac26b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5361aaac26b1b8, sizeof(XMLVM_JMP_BUF)); goto label25; };
    _r2.o = java_lang_Byte_GET_CACHE();
    _r3.o = __NEW_java_lang_Byte();
    XMLVM_CHECK_NPE(3)
    java_lang_Byte___INIT____byte(_r3.o, _r4.i);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.o;
    _r1 = _r3;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5361aaac26b1b8)
        XMLVM_CATCH_SPECIFIC(w5361aaac26b1b8,java_lang_Object,27)
    XMLVM_CATCH_END(w5361aaac26b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w5361aaac26b1b8)
    label23:;
    XMLVM_TRY_BEGIN(w5361aaac26b1c10)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    XMLVM_MEMCPY(curThread_w5361aaac26b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5361aaac26b1c10, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r1.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5361aaac26b1c10)
        XMLVM_CATCH_SPECIFIC(w5361aaac26b1c10,java_lang_Object,27)
    XMLVM_CATCH_END(w5361aaac26b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w5361aaac26b1c10)
    label25:;
    XMLVM_TRY_BEGIN(w5361aaac26b1c12)
    // Begin try
    _r1 = _r2;
    { XMLVM_MEMCPY(curThread_w5361aaac26b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w5361aaac26b1c12, sizeof(XMLVM_JMP_BUF)); goto label23; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5361aaac26b1c12)
        XMLVM_CATCH_SPECIFIC(w5361aaac26b1c12,java_lang_Object,27)
    XMLVM_CATCH_END(w5361aaac26b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w5361aaac26b1c12)
    label27:;
    XMLVM_TRY_BEGIN(w5361aaac26b1c14)
    // Begin try
    java_lang_Thread* curThread_w5361aaac26b1c14aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w5361aaac26b1c14aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5361aaac26b1c14)
        XMLVM_CATCH_SPECIFIC(w5361aaac26b1c14,java_lang_Object,27)
    XMLVM_CATCH_END(w5361aaac26b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w5361aaac26b1c14)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Byte_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Byte_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Byte", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Byte.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Byte_compareTo___java_lang_Byte(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

