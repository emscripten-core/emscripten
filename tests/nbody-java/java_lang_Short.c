#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"

#include "java_lang_Short.h"

#define XMLVM_CURRENT_CLASS_NAME Short
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Short

__TIB_DEFINITION_java_lang_Short __TIB_java_lang_Short = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Short, // classInitializer
    "java.lang.Short", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Number;Ljava/lang/Comparable<Ljava/lang/Short;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Number, // extends
    sizeof(java_lang_Short), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Short;
JAVA_OBJECT __CLASS_java_lang_Short_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Short_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Short_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Short_serialVersionUID;
static JAVA_SHORT _STATIC_java_lang_Short_MAX_VALUE;
static JAVA_SHORT _STATIC_java_lang_Short_MIN_VALUE;
static JAVA_INT _STATIC_java_lang_Short_SIZE;
static JAVA_OBJECT _STATIC_java_lang_Short_TYPE;

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

void __INIT_java_lang_Short()
{
    staticInitializerLock(&__TIB_java_lang_Short);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Short.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Short.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Short);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Short.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Short.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Short.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Short")
        __INIT_IMPL_java_lang_Short();
    }
}

void __INIT_IMPL_java_lang_Short()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Number)
    __TIB_java_lang_Short.newInstanceFunc = __NEW_INSTANCE_java_lang_Short;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Short.vtable, __TIB_java_lang_Number.vtable, sizeof(__TIB_java_lang_Number.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Short.vtable[6] = (VTABLE_PTR) &java_lang_Short_byteValue__;
    __TIB_java_lang_Short.vtable[7] = (VTABLE_PTR) &java_lang_Short_doubleValue__;
    __TIB_java_lang_Short.vtable[1] = (VTABLE_PTR) &java_lang_Short_equals___java_lang_Object;
    __TIB_java_lang_Short.vtable[8] = (VTABLE_PTR) &java_lang_Short_floatValue__;
    __TIB_java_lang_Short.vtable[4] = (VTABLE_PTR) &java_lang_Short_hashCode__;
    __TIB_java_lang_Short.vtable[9] = (VTABLE_PTR) &java_lang_Short_intValue__;
    __TIB_java_lang_Short.vtable[10] = (VTABLE_PTR) &java_lang_Short_longValue__;
    __TIB_java_lang_Short.vtable[11] = (VTABLE_PTR) &java_lang_Short_shortValue__;
    __TIB_java_lang_Short.vtable[5] = (VTABLE_PTR) &java_lang_Short_toString__;
    __TIB_java_lang_Short.vtable[12] = (VTABLE_PTR) &java_lang_Short_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Short.numImplementedInterfaces = 2;
    __TIB_java_lang_Short.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Short.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Short.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Short.itableBegin = &__TIB_java_lang_Short.itable[0];
    __TIB_java_lang_Short.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Short.vtable[12];

    _STATIC_java_lang_Short_serialVersionUID = 7515723908773894738;
    _STATIC_java_lang_Short_MAX_VALUE = 32767;
    _STATIC_java_lang_Short_MIN_VALUE = -32768;
    _STATIC_java_lang_Short_SIZE = 16;
    _STATIC_java_lang_Short_TYPE = (java_lang_Class*) JAVA_NULL;

    __TIB_java_lang_Short.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Short.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Short.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Short.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Short.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Short.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Short.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Short.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Short = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Short);
    __TIB_java_lang_Short.clazz = __CLASS_java_lang_Short;
    __TIB_java_lang_Short.baseType = JAVA_NULL;
    __CLASS_java_lang_Short_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Short);
    __CLASS_java_lang_Short_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Short_1ARRAY);
    __CLASS_java_lang_Short_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Short_2ARRAY);
    java_lang_Short___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Short]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Short.classInitialized = 1;
}

void __DELETE_java_lang_Short(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Short]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Short(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Number(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Short*) me)->fields.java_lang_Short.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Short]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Short()
{    XMLVM_CLASS_INIT(java_lang_Short)
java_lang_Short* me = (java_lang_Short*) XMLVM_MALLOC(sizeof(java_lang_Short));
    me->tib = &__TIB_java_lang_Short;
    __INIT_INSTANCE_MEMBERS_java_lang_Short(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Short]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Short()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Short_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Short)
    return _STATIC_java_lang_Short_serialVersionUID;
}

void java_lang_Short_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Short)
_STATIC_java_lang_Short_serialVersionUID = v;
}

JAVA_SHORT java_lang_Short_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Short)
    return _STATIC_java_lang_Short_MAX_VALUE;
}

void java_lang_Short_PUT_MAX_VALUE(JAVA_SHORT v)
{
    XMLVM_CLASS_INIT(java_lang_Short)
_STATIC_java_lang_Short_MAX_VALUE = v;
}

JAVA_SHORT java_lang_Short_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Short)
    return _STATIC_java_lang_Short_MIN_VALUE;
}

void java_lang_Short_PUT_MIN_VALUE(JAVA_SHORT v)
{
    XMLVM_CLASS_INIT(java_lang_Short)
_STATIC_java_lang_Short_MIN_VALUE = v;
}

JAVA_INT java_lang_Short_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Short)
    return _STATIC_java_lang_Short_SIZE;
}

void java_lang_Short_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Short)
_STATIC_java_lang_Short_SIZE = v;
}

JAVA_OBJECT java_lang_Short_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Short)
    return _STATIC_java_lang_Short_TYPE;
}

void java_lang_Short_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Short)
_STATIC_java_lang_Short_TYPE = v;
}

void java_lang_Short___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Short", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Short.java", 58)
    _r0.i = 0;
    XMLVM_CLASS_INIT(short)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_short, _r0.i);
    XMLVM_SOURCE_POSITION("Short.java", 59)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Short_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Short.java", 26)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Short___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Short", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 75)
    _r0.i = java_lang_Short_parseShort___java_lang_String(_r2.o);
    XMLVM_CHECK_NPE(1)
    java_lang_Short___INIT____short(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("Short.java", 76)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Short___INIT____short(JAVA_OBJECT me, JAVA_SHORT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short___INIT____short]
    XMLVM_ENTER_METHOD("java.lang.Short", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Short.java", 84)
    XMLVM_CHECK_NPE(0)
    java_lang_Number___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Short.java", 85)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Short*) _r0.o)->fields.java_lang_Short.value_ = _r1.i;
    XMLVM_SOURCE_POSITION("Short.java", 86)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Short_byteValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_byteValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "byteValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 90)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    _r0.i = (_r0.i << 24) >> 24;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Short_compareTo___java_lang_Short(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_compareTo___java_lang_Short]
    XMLVM_ENTER_METHOD("java.lang.Short", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 109)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Short*) _r2.o)->fields.java_lang_Short.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Short*) _r3.o)->fields.java_lang_Short.value_;
    if (_r0.i <= _r1.i) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Short*) _r2.o)->fields.java_lang_Short.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Short*) _r3.o)->fields.java_lang_Short.value_;
    if (_r0.i >= _r1.i) goto label16;
    _r0.i = -1;
    goto label7;
    label16:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_decode___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_decode___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Short", "decode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 126)
    _r0.o = java_lang_Integer_decode___java_lang_String(_r2.o);
    //java_lang_Integer_intValue__[9]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Integer*) _r0.o)->tib->vtable[9])(_r0.o);
    XMLVM_SOURCE_POSITION("Short.java", 127)
    _r1.i = (_r0.i << 16) >> 16;
    if (_r1.i != _r0.i) goto label16;
    XMLVM_SOURCE_POSITION("Short.java", 128)
    XMLVM_SOURCE_POSITION("Short.java", 129)
    _r0.o = java_lang_Short_valueOf___short(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    XMLVM_SOURCE_POSITION("Short.java", 131)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_Short_doubleValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_doubleValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "doubleValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 136)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Short*) _r2.o)->fields.java_lang_Short.value_;
    _r0.d = (JAVA_DOUBLE) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Short_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Short", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 151)
    XMLVM_CLASS_INIT(java_lang_Short)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_lang_Short);
    if (_r0.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Short.java", 152)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Short*) _r2.o)->fields.java_lang_Short.value_;
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Short*) _r3.o)->fields.java_lang_Short.value_;
    if (_r0.i != _r1.i) goto label14;
    _r0.i = 1;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 0;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_Short_floatValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_floatValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "floatValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 157)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Short_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Short", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 162)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Short_intValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_intValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "intValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 167)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_Short_longValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_longValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "longValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 172)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Short*) _r2.o)->fields.java_lang_Short.value_;
    _r0.l = (JAVA_LONG) _r0.i;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Short_parseShort___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_parseShort___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Short", "parseShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 187)
    _r0.i = 10;
    _r0.i = java_lang_Short_parseShort___java_lang_String_int(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Short_parseShort___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_parseShort___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Short", "parseShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("Short.java", 208)
    _r0.i = java_lang_Integer_parseInt___java_lang_String_int(_r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("Short.java", 209)
    _r1.i = (_r0.i << 16) >> 16;
    if (_r1.i != _r0.i) goto label8;
    XMLVM_SOURCE_POSITION("Short.java", 210)
    XMLVM_SOURCE_POSITION("Short.java", 211)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label8:;
    XMLVM_SOURCE_POSITION("Short.java", 213)

    
    // Red class access removed: java.lang.NumberFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NumberFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Short_shortValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_shortValue__]
    XMLVM_ENTER_METHOD("java.lang.Short", "shortValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 223)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_toString__]
    XMLVM_ENTER_METHOD("java.lang.Short", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Short.java", 228)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Short*) _r1.o)->fields.java_lang_Short.value_;
    _r0.o = java_lang_Integer_toString___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_toString___short(JAVA_SHORT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_toString___short]
    XMLVM_ENTER_METHOD("java.lang.Short", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Short.java", 240)
    _r0.o = java_lang_Integer_toString___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Short", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 256)
    _r0.i = java_lang_Short_parseShort___java_lang_String(_r1.o);
    _r0.o = java_lang_Short_valueOf___short(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_valueOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Short", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Short.java", 278)
    _r0.i = java_lang_Short_parseShort___java_lang_String_int(_r1.o, _r2.i);
    _r0.o = java_lang_Short_valueOf___short(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_lang_Short_reverseBytes___short(JAVA_SHORT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_reverseBytes___short]
    XMLVM_ENTER_METHOD("java.lang.Short", "reverseBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Short.java", 290)
    _r0.i = _r2.i >> 8;
    _r0.i = _r0.i & 255;
    _r1.i = _r2.i & 255;
    _r1.i = _r1.i << 8;
    _r0.i = _r0.i | _r1.i;
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_SOURCE_POSITION("Short.java", 292)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Short_valueOf___short(JAVA_SHORT n1)
{
    XMLVM_CLASS_INIT(java_lang_Short)
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_valueOf___short]
    XMLVM_ENTER_METHOD("java.lang.Short", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Short.java", 308)
    _r0.i = -128;
    if (_r2.i < _r0.i) goto label8;
    _r0.i = 127;
    if (_r2.i <= _r0.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("Short.java", 309)
    _r0.o = __NEW_java_lang_Short();
    XMLVM_CHECK_NPE(0)
    java_lang_Short___INIT____short(_r0.o, _r2.i);
    label13:;
    XMLVM_SOURCE_POSITION("Short.java", 311)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label14:;

    
    // Red class access removed: java.lang.Short$valueOfCache::access$0
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.i = _r2.i + 128;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Short_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Short_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Short", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Short.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Short_compareTo___java_lang_Short(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

