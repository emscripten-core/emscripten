#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_nio_charset_CharsetDecoder.h"
#include "java_nio_charset_CharsetEncoder.h"
#include "org_apache_harmony_niochar_charset_UTF_8_Decoder.h"
#include "org_apache_harmony_niochar_charset_UTF_8_Encoder.h"

#include "org_apache_harmony_niochar_charset_UTF_8.h"

#define XMLVM_CURRENT_CLASS_NAME UTF_8
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_niochar_charset_UTF_8

__TIB_DEFINITION_org_apache_harmony_niochar_charset_UTF_8 __TIB_org_apache_harmony_niochar_charset_UTF_8 = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_niochar_charset_UTF_8, // classInitializer
    "org.apache.harmony.niochar.charset.UTF_8", // className
    "org.apache.harmony.niochar.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_charset_Charset, // extends
    sizeof(org_apache_harmony_niochar_charset_UTF_8), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingBytes;
static JAVA_OBJECT _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingNumbers;
static JAVA_OBJECT _STATIC_org_apache_harmony_niochar_charset_UTF_8_lowerEncodingLimit;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
    {"remainingBytes",
    &__CLASS_int_1ARRAY,
    0 | java_lang_reflect_Modifier_PRIVATE | java_lang_reflect_Modifier_STATIC,
    0,
    &_STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingBytes,
    "",
    JAVA_NULL},
    {"remainingNumbers",
    &__CLASS_int_1ARRAY,
    0 | java_lang_reflect_Modifier_PRIVATE | java_lang_reflect_Modifier_STATIC,
    0,
    &_STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingNumbers,
    "",
    JAVA_NULL},
    {"lowerEncodingLimit",
    &__CLASS_int_1ARRAY,
    0 | java_lang_reflect_Modifier_PRIVATE | java_lang_reflect_Modifier_STATIC,
    0,
    &_STATIC_org_apache_harmony_niochar_charset_UTF_8_lowerEncodingLimit,
    "",
    JAVA_NULL},
};

static JAVA_OBJECT* __constructor0_arg_types[] = {
    &__CLASS_java_lang_String,
    &__CLASS_java_lang_String_1ARRAY,
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
    {&__constructor0_arg_types[0],
    sizeof(__constructor0_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/lang/String;[Ljava/lang/String;)V",
    JAVA_NULL,
    JAVA_NULL},
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    JAVA_OBJECT obj = __NEW_org_apache_harmony_niochar_charset_UTF_8();
    java_lang_reflect_Constructor* c = (java_lang_reflect_Constructor*) constructor;
    org_xmlvm_runtime_XMLVMArray* args = (org_xmlvm_runtime_XMLVMArray*) arguments;
    JAVA_ARRAY_OBJECT* argsArray = (JAVA_ARRAY_OBJECT*) args->fields.org_xmlvm_runtime_XMLVMArray.array_;
    switch (c->fields.java_lang_reflect_Constructor.slot_) {
    case 0:
        org_apache_harmony_niochar_charset_UTF_8___INIT____java_lang_String_java_lang_String_1ARRAY(obj, argsArray[0], argsArray[1]);
        break;
    default:
        XMLVM_INTERNAL_ERROR();
        break;
    }
    return obj;
}

static JAVA_OBJECT* __method0_arg_types[] = {
    &__CLASS_java_nio_charset_Charset,
};

static JAVA_OBJECT* __method1_arg_types[] = {
};

static JAVA_OBJECT* __method2_arg_types[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
    {"contains",
    &__method0_arg_types[0],
    sizeof(__method0_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/nio/charset/Charset;)Z",
    JAVA_NULL,
    JAVA_NULL},
    {"newDecoder",
    &__method1_arg_types[0],
    sizeof(__method1_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "()Ljava/nio/charset/CharsetDecoder;",
    JAVA_NULL,
    JAVA_NULL},
    {"newEncoder",
    &__method2_arg_types[0],
    sizeof(__method2_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "()Ljava/nio/charset/CharsetEncoder;",
    JAVA_NULL,
    JAVA_NULL},
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    JAVA_OBJECT result = JAVA_NULL;
    java_lang_Object* obj = receiver;
    java_lang_reflect_Method* m = (java_lang_reflect_Method*) method;
    org_xmlvm_runtime_XMLVMArray* args = (org_xmlvm_runtime_XMLVMArray*) arguments;
    JAVA_ARRAY_OBJECT* argsArray = (JAVA_ARRAY_OBJECT*) args->fields.org_xmlvm_runtime_XMLVMArray.array_;
    XMLVMElem conversion;
    switch (m->fields.java_lang_reflect_Method.slot_) {
    case 0:
        conversion.i = (JAVA_BOOLEAN) org_apache_harmony_niochar_charset_UTF_8_contains___java_nio_charset_Charset(receiver, argsArray[0]);
        result = __NEW_java_lang_Boolean();
        java_lang_Boolean___INIT____boolean(result, conversion.i);
        break;
    case 1:
        result = (JAVA_OBJECT) org_apache_harmony_niochar_charset_UTF_8_newDecoder__(receiver);
        break;
    case 2:
        result = (JAVA_OBJECT) org_apache_harmony_niochar_charset_UTF_8_newEncoder__(receiver);
        break;
    default:
        XMLVM_INTERNAL_ERROR();
        break;
    }
    return result;
}

void __INIT_org_apache_harmony_niochar_charset_UTF_8()
{
    staticInitializerLock(&__TIB_org_apache_harmony_niochar_charset_UTF_8);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_niochar_charset_UTF_8.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_niochar_charset_UTF_8.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_niochar_charset_UTF_8);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_niochar_charset_UTF_8.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_niochar_charset_UTF_8.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_niochar_charset_UTF_8.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.niochar.charset.UTF_8")
        __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8();
    }
}

void __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    __TIB_org_apache_harmony_niochar_charset_UTF_8.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_niochar_charset_UTF_8.vtable, __TIB_java_nio_charset_Charset.vtable, sizeof(__TIB_java_nio_charset_Charset.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_niochar_charset_UTF_8.vtable[7] = (VTABLE_PTR) &org_apache_harmony_niochar_charset_UTF_8_contains___java_nio_charset_Charset;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.vtable[8] = (VTABLE_PTR) &org_apache_harmony_niochar_charset_UTF_8_newDecoder__;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.vtable[9] = (VTABLE_PTR) &org_apache_harmony_niochar_charset_UTF_8_newEncoder__;
    // Initialize interface information
    __TIB_org_apache_harmony_niochar_charset_UTF_8.numImplementedInterfaces = 1;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_org_apache_harmony_niochar_charset_UTF_8.implementedInterfaces[0][0] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_org_apache_harmony_niochar_charset_UTF_8.itableBegin = &__TIB_org_apache_harmony_niochar_charset_UTF_8.itable[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_org_apache_harmony_niochar_charset_UTF_8.vtable[6];

    _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingBytes = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingNumbers = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_org_apache_harmony_niochar_charset_UTF_8_lowerEncodingLimit = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_org_apache_harmony_niochar_charset_UTF_8.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8 = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_niochar_charset_UTF_8);
    __TIB_org_apache_harmony_niochar_charset_UTF_8.clazz = __CLASS_org_apache_harmony_niochar_charset_UTF_8;
    __TIB_org_apache_harmony_niochar_charset_UTF_8.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_1ARRAY);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_2ARRAY);
    org_apache_harmony_niochar_charset_UTF_8___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_niochar_charset_UTF_8]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_niochar_charset_UTF_8.classInitialized = 1;
}

void __DELETE_org_apache_harmony_niochar_charset_UTF_8(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_niochar_charset_UTF_8]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_niochar_charset_UTF_8()
{    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
org_apache_harmony_niochar_charset_UTF_8* me = (org_apache_harmony_niochar_charset_UTF_8*) XMLVM_MALLOC(sizeof(org_apache_harmony_niochar_charset_UTF_8));
    me->tib = &__TIB_org_apache_harmony_niochar_charset_UTF_8;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_niochar_charset_UTF_8]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_GET_remainingBytes()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    return _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingBytes;
}

void org_apache_harmony_niochar_charset_UTF_8_PUT_remainingBytes(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
_STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingBytes = v;
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_GET_remainingNumbers()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    return _STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingNumbers;
}

void org_apache_harmony_niochar_charset_UTF_8_PUT_remainingNumbers(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
_STATIC_org_apache_harmony_niochar_charset_UTF_8_remainingNumbers = v;
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_GET_lowerEncodingLimit()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    return _STATIC_org_apache_harmony_niochar_charset_UTF_8_lowerEncodingLimit;
}

void org_apache_harmony_niochar_charset_UTF_8_PUT_lowerEncodingLimit(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
_STATIC_org_apache_harmony_niochar_charset_UTF_8_lowerEncodingLimit = v;
}

void org_apache_harmony_niochar_charset_UTF_8___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8___CLINIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = 4;
    XMLVM_SOURCE_POSITION("UTF_8.java", 42)
    _r0.i = 128;
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r0.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_INT[]){-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1, });
    org_apache_harmony_niochar_charset_UTF_8_PUT_remainingBytes( _r0.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 58)
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r3.i);
    _r1.i = 1;
    _r2.i = 4224;
    XMLVM_SOURCE_POSITION("UTF_8.java", 60)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 2;
    _r2.i = 401536;
    XMLVM_SOURCE_POSITION("UTF_8.java", 61)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 3;
    _r2.i = 29892736;
    XMLVM_SOURCE_POSITION("UTF_8.java", 62)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    org_apache_harmony_niochar_charset_UTF_8_PUT_remainingNumbers( _r0.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 65)
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r3.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_INT[]){-1, 128, 2048, 65536, });
    org_apache_harmony_niochar_charset_UTF_8_PUT_lowerEncodingLimit( _r0.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 27)
    XMLVM_EXIT_METHOD()
    return;
    label40:;
    label300:;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_charset_UTF_8___INIT____java_lang_String_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8___INIT____java_lang_String_java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("UTF_8.java", 68)
    XMLVM_CHECK_NPE(0)
    java_nio_charset_Charset___INIT____java_lang_String_java_lang_String_1ARRAY(_r0.o, _r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 69)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_apache_harmony_niochar_charset_UTF_8_contains___java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_contains___java_nio_charset_Charset]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("UTF_8.java", 74)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "UTF-8"
    _r1.o = xmlvm_create_java_string_from_pool(100);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 75)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "US-ASCII"
    _r1.o = xmlvm_create_java_string_from_pool(190);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 76)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "KOI8-R"
    _r1.o = xmlvm_create_java_string_from_pool(204);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 77)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1250"
    _r1.o = xmlvm_create_java_string_from_pool(207);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 78)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1251"
    _r1.o = xmlvm_create_java_string_from_pool(210);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 79)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1252"
    _r1.o = xmlvm_create_java_string_from_pool(213);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 80)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1253"
    _r1.o = xmlvm_create_java_string_from_pool(216);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 81)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1254"
    _r1.o = xmlvm_create_java_string_from_pool(219);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 82)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "windows-1257"
    _r1.o = xmlvm_create_java_string_from_pool(222);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 83)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-1"
    _r1.o = xmlvm_create_java_string_from_pool(175);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 84)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-2"
    _r1.o = xmlvm_create_java_string_from_pool(237);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 85)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-4"
    _r1.o = xmlvm_create_java_string_from_pool(246);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 86)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-5"
    _r1.o = xmlvm_create_java_string_from_pool(255);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 87)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-7"
    _r1.o = xmlvm_create_java_string_from_pool(263);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 88)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-9"
    _r1.o = xmlvm_create_java_string_from_pool(273);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 89)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-10"
    _r1.o = xmlvm_create_java_string_from_pool(829);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 90)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-13"
    _r1.o = xmlvm_create_java_string_from_pool(281);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 91)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-14"
    _r1.o = xmlvm_create_java_string_from_pool(830);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 92)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-15"
    _r1.o = xmlvm_create_java_string_from_pool(283);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 93)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "ISO-8859-16"
    _r1.o = xmlvm_create_java_string_from_pool(831);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 94)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "UTF-16"
    _r1.o = xmlvm_create_java_string_from_pool(289);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 95)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "UTF-16LE"
    _r1.o = xmlvm_create_java_string_from_pool(292);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    XMLVM_SOURCE_POSITION("UTF_8.java", 96)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_Charset_name__(_r3.o);
    // "UTF-16BE"
    _r1.o = xmlvm_create_java_string_from_pool(295);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label278;
    _r0.i = 0;
    label277:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label278:;
    _r0.i = 1;
    goto label277;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_newDecoder__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_newDecoder__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "newDecoder", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("UTF_8.java", 100)
    _r0.o = __NEW_org_apache_harmony_niochar_charset_UTF_8_Decoder();
    _r1.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Decoder(_r0.o, _r2.o, _r2.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_newEncoder__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_newEncoder__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "newEncoder", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("UTF_8.java", 104)
    _r0.o = __NEW_org_apache_harmony_niochar_charset_UTF_8_Encoder();
    _r1.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Encoder(_r0.o, _r2.o, _r2.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_access$0__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_access$0__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "access$0", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("UTF_8.java", 42)
    _r0.o = org_apache_harmony_niochar_charset_UTF_8_GET_remainingBytes();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_access$1__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_access$1__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "access$1", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("UTF_8.java", 58)
    _r0.o = org_apache_harmony_niochar_charset_UTF_8_GET_remainingNumbers();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_access$2__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_access$2__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8", "access$2", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("UTF_8.java", 65)
    _r0.o = org_apache_harmony_niochar_charset_UTF_8_GET_lowerEncodingLimit();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

