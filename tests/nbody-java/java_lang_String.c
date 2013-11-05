#include "xmlvm.h"
#include "java_lang_Character.h"
#include "java_lang_Class.h"
#include "java_lang_Double.h"
#include "java_lang_Exception.h"
#include "java_lang_Float.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"
#include "java_lang_Math.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String_CaseInsensitiveComparator.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Throwable.h"
#include "java_nio_ByteBuffer.h"
#include "java_nio_CharBuffer.h"
#include "java_nio_charset_Charset.h"
#include "java_security_AccessController.h"
#include "java_util_Comparator.h"
#include "java_util_Locale.h"
#include "org_apache_harmony_luni_util_PriviAction.h"
#include "org_apache_harmony_niochar_charset_UTF_8.h"

#include "java_lang_String.h"

#define XMLVM_CURRENT_CLASS_NAME String
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_String

__TIB_DEFINITION_java_lang_String __TIB_java_lang_String = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_String, // classInitializer
    "java.lang.String", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/io/Serializable;Ljava/lang/Comparable<Ljava/lang/String;>;Ljava/lang/CharSequence;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_String), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_String;
JAVA_OBJECT __CLASS_java_lang_String_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_String_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_String_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_String_serialVersionUID;
static JAVA_OBJECT _STATIC_java_lang_String_CASE_INSENSITIVE_ORDER;
static JAVA_OBJECT _STATIC_java_lang_String_ascii;
static JAVA_OBJECT _STATIC_java_lang_String_DefaultCharset;
static JAVA_OBJECT _STATIC_java_lang_String_lastCharset;

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

void __INIT_java_lang_String()
{
    staticInitializerLock(&__TIB_java_lang_String);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_String.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_String.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_String);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_String.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_String.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_String.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.String")
        __INIT_IMPL_java_lang_String();
    }
}

void __INIT_IMPL_java_lang_String()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_String.newInstanceFunc = __NEW_INSTANCE_java_lang_String;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_String.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_String.vtable[6] = (VTABLE_PTR) &java_lang_String_charAt___int;
    __TIB_java_lang_String.vtable[1] = (VTABLE_PTR) &java_lang_String_equals___java_lang_Object;
    __TIB_java_lang_String.vtable[4] = (VTABLE_PTR) &java_lang_String_hashCode__;
    __TIB_java_lang_String.vtable[8] = (VTABLE_PTR) &java_lang_String_length__;
    __TIB_java_lang_String.vtable[5] = (VTABLE_PTR) &java_lang_String_toString__;
    __TIB_java_lang_String.vtable[9] = (VTABLE_PTR) &java_lang_String_subSequence___int_int;
    __TIB_java_lang_String.vtable[7] = (VTABLE_PTR) &java_lang_String_compareTo___java_lang_Object;
    xmlvm_init_native_java_lang_String();
    // Initialize interface information
    __TIB_java_lang_String.numImplementedInterfaces = 3;
    __TIB_java_lang_String.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_String.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_CharSequence)

    __TIB_java_lang_String.implementedInterfaces[0][1] = &__TIB_java_lang_CharSequence;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_String.implementedInterfaces[0][2] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_String.itableBegin = &__TIB_java_lang_String.itable[0];
    __TIB_java_lang_String.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int] = __TIB_java_lang_String.vtable[6];
    __TIB_java_lang_String.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__] = __TIB_java_lang_String.vtable[8];
    __TIB_java_lang_String.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int] = __TIB_java_lang_String.vtable[9];
    __TIB_java_lang_String.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__] = __TIB_java_lang_String.vtable[5];
    __TIB_java_lang_String.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_String.vtable[7];

    _STATIC_java_lang_String_serialVersionUID = -6849794470754667710;
    _STATIC_java_lang_String_CASE_INSENSITIVE_ORDER = (java_util_Comparator*) JAVA_NULL;
    _STATIC_java_lang_String_ascii = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_String_DefaultCharset = (java_nio_charset_Charset*) JAVA_NULL;
    _STATIC_java_lang_String_lastCharset = (java_nio_charset_Charset*) JAVA_NULL;

    __TIB_java_lang_String.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_String.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_String.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_String.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_String.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_String.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_String.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_String.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_String = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_String);
    __TIB_java_lang_String.clazz = __CLASS_java_lang_String;
    __TIB_java_lang_String.baseType = JAVA_NULL;
    __CLASS_java_lang_String_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_String);
    __CLASS_java_lang_String_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_String_1ARRAY);
    __CLASS_java_lang_String_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_String_2ARRAY);
    java_lang_String___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_String]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_String.classInitialized = 1;
}

void __DELETE_java_lang_String(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_String]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_String(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_String*) me)->fields.java_lang_String.charset_ = (org_apache_harmony_niochar_charset_UTF_8*) JAVA_NULL;
    ((java_lang_String*) me)->fields.java_lang_String.charset2_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_String*) me)->fields.java_lang_String.charset3_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_String*) me)->fields.java_lang_String.value_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_String*) me)->fields.java_lang_String.offset_ = 0;
    ((java_lang_String*) me)->fields.java_lang_String.count_ = 0;
    ((java_lang_String*) me)->fields.java_lang_String.hashCode_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_String]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_String()
{    XMLVM_CLASS_INIT(java_lang_String)
java_lang_String* me = (java_lang_String*) XMLVM_MALLOC(sizeof(java_lang_String));
    me->tib = &__TIB_java_lang_String;
    __INIT_INSTANCE_MEMBERS_java_lang_String(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_String]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_String()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_String();
    java_lang_String___INIT___(me);
    return me;
}

JAVA_LONG java_lang_String_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_String)
    return _STATIC_java_lang_String_serialVersionUID;
}

void java_lang_String_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_String)
_STATIC_java_lang_String_serialVersionUID = v;
}

JAVA_OBJECT java_lang_String_GET_CASE_INSENSITIVE_ORDER()
{
    XMLVM_CLASS_INIT(java_lang_String)
    return _STATIC_java_lang_String_CASE_INSENSITIVE_ORDER;
}

void java_lang_String_PUT_CASE_INSENSITIVE_ORDER(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_String)
_STATIC_java_lang_String_CASE_INSENSITIVE_ORDER = v;
}

JAVA_OBJECT java_lang_String_GET_ascii()
{
    XMLVM_CLASS_INIT(java_lang_String)
    return _STATIC_java_lang_String_ascii;
}

void java_lang_String_PUT_ascii(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_String)
_STATIC_java_lang_String_ascii = v;
}

JAVA_OBJECT java_lang_String_GET_DefaultCharset()
{
    XMLVM_CLASS_INIT(java_lang_String)
    return _STATIC_java_lang_String_DefaultCharset;
}

void java_lang_String_PUT_DefaultCharset(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_String)
_STATIC_java_lang_String_DefaultCharset = v;
}

JAVA_OBJECT java_lang_String_GET_lastCharset()
{
    XMLVM_CLASS_INIT(java_lang_String)
    return _STATIC_java_lang_String_lastCharset;
}

void java_lang_String_PUT_lastCharset(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_String)
_STATIC_java_lang_String_lastCharset = v;
}

void java_lang_String___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT___]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r0.o = JAVA_NULL;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 166)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 167)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r1.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 168)
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.offset_ = _r1.i;
    XMLVM_SOURCE_POSITION("String.java", 169)
    XMLVM_CHECK_NPE(2)
    ((java_lang_String*) _r2.o)->fields.java_lang_String.count_ = _r1.i;
    XMLVM_SOURCE_POSITION("String.java", 170)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_String_char(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_String_char]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.o = n1;
    _r7.i = n2;
    _r4.i = 0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 176)
    XMLVM_CHECK_NPE(5)
    java_lang_Object___INIT___(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 177)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 178)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i + 1;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 179)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 180)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("String.java", 181)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    XMLVM_SOURCE_POSITION("String.java", 182)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 194)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____byte_1ARRAY_int_int(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 195)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 212)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____byte_1ARRAY_int_int_int(_r2.o, _r3.o, _r4.i, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 213)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.i = n2;
    _r8.i = n3;
    _r3.o = JAVA_NULL;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 233)
    XMLVM_CHECK_NPE(5)
    java_lang_Object___INIT___(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset2_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset3_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 235)
    if (_r7.i < 0) goto label55;
    if (_r8.i < 0) goto label55;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r3.i = _r3.i - _r7.i;
    if (_r8.i > _r3.i) goto label55;
    XMLVM_SOURCE_POSITION("String.java", 236)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 237)
    XMLVM_CHECK_NPE(5)
    _r1.o = java_lang_String_defaultCharset__(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 239)
    _r3.o = java_nio_ByteBuffer_wrap___byte_1ARRAY_int_int(_r6.o, _r7.i, _r8.i);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_charset_Charset_decode___java_nio_ByteBuffer(_r1.o, _r3.o);
    XMLVM_SOURCE_POSITION("String.java", 241)
    //java_nio_CharBuffer_length__[22]
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r0.o)->tib->vtable[22])(_r0.o);
    if (_r2.i <= 0) goto label48;
    XMLVM_SOURCE_POSITION("String.java", 242)
    //java_nio_CharBuffer_array__[7]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r0.o)->tib->vtable[7])(_r0.o);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 243)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r2.i;
    label47:;
    XMLVM_SOURCE_POSITION("String.java", 251)
    XMLVM_EXIT_METHOD()
    return;
    label48:;
    XMLVM_SOURCE_POSITION("String.java", 245)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 246)
    XMLVM_CLASS_INIT(char)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_char, _r4.i);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r3.o;
    goto label47;
    label55:;
    XMLVM_SOURCE_POSITION("String.java", 249)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY_int_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY_int_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r4.o = me;
    _r5.o = n1;
    _r6.i = n2;
    _r7.i = n3;
    _r8.i = n4;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 274)
    XMLVM_CHECK_NPE(4)
    java_lang_Object___INIT___(_r4.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.charset_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.charset2_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.charset3_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 277)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    _r2.i = _r2.i - _r7.i;
    if (_r8.i > _r2.i) goto label50;
    if (_r7.i < 0) goto label50;
    if (_r8.i < 0) goto label50;
    XMLVM_SOURCE_POSITION("String.java", 278)
    _r2.i = 0;
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_ = _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 279)
    XMLVM_CLASS_INIT(char)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_char, _r8.i);
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.value_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 280)
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.count_ = _r8.i;
    XMLVM_SOURCE_POSITION("String.java", 281)
    _r6.i = _r6.i << 8;
    _r0.i = 0;
    label30:;
    XMLVM_SOURCE_POSITION("String.java", 282)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r0.i >= _r2.i) goto label56;
    XMLVM_SOURCE_POSITION("String.java", 283)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    _r1.i = _r7.i + 1;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r7.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    _r3.i = _r3.i & 255;
    _r3.i = _r3.i + _r6.i;
    _r3.i = _r3.i & 0xffff;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    _r0.i = _r0.i + 1;
    _r7 = _r1;
    goto label30;
    label50:;
    XMLVM_SOURCE_POSITION("String.java", 286)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r2.o)
    label56:;
    XMLVM_SOURCE_POSITION("String.java", 288)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY_int_int_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_OBJECT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY_int_int_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    _r6.o = me;
    _r7.o = n1;
    _r8.i = n2;
    _r9.i = n3;
    _r10.o = n4;
    _r4.o = JAVA_NULL;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 310)
    XMLVM_CHECK_NPE(6)
    java_lang_Object___INIT___(_r6.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.charset_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.charset2_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.charset3_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 311)
    if (_r10.o != JAVA_NULL) goto label19;
    XMLVM_SOURCE_POSITION("String.java", 312)
    _r4.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(4)
    java_lang_NullPointerException___INIT___(_r4.o);
    XMLVM_THROW_CUSTOM(_r4.o)
    label19:;
    XMLVM_SOURCE_POSITION("String.java", 315)
    if (_r8.i < 0) goto label76;
    if (_r9.i < 0) goto label76;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    _r4.i = _r4.i - _r8.i;
    if (_r9.i > _r4.i) goto label76;
    XMLVM_SOURCE_POSITION("String.java", 316)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_ = _r5.i;
    XMLVM_SOURCE_POSITION("String.java", 317)
    XMLVM_CHECK_NPE(6)
    _r1.o = java_lang_String_getCharset___java_lang_String(_r6.o, _r10.o);
    XMLVM_TRY_BEGIN(w3113aaac18b1c33)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 322)
    _r4.o = java_nio_ByteBuffer_wrap___byte_1ARRAY_int_int(_r7.o, _r8.i, _r9.i);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_charset_Charset_decode___java_nio_ByteBuffer(_r1.o, _r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac18b1c33)
        XMLVM_CATCH_SPECIFIC(w3113aaac18b1c33,java_lang_Exception,56)
    XMLVM_CATCH_END(w3113aaac18b1c33)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac18b1c33)
    label41:;
    XMLVM_SOURCE_POSITION("String.java", 328)
    //java_nio_CharBuffer_length__[22]
    XMLVM_CHECK_NPE(0)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r0.o)->tib->vtable[22])(_r0.o);
    if (_r3.i <= 0) goto label69;
    XMLVM_SOURCE_POSITION("String.java", 329)
    //java_nio_CharBuffer_array__[7]
    XMLVM_CHECK_NPE(0)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r0.o)->tib->vtable[7])(_r0.o);
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.value_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 330)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.count_ = _r3.i;
    label55:;
    XMLVM_SOURCE_POSITION("String.java", 338)
    XMLVM_EXIT_METHOD()
    return;
    label56:;
    XMLVM_SOURCE_POSITION("String.java", 323)
    java_lang_Thread* curThread_w3113aaac18b1c48 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3113aaac18b1c48->fields.java_lang_Thread.xmlvmException_;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("String.java", 326)
    // "?"
    _r4.o = xmlvm_create_java_string_from_pool(170);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_String_toCharArray__(_r4.o);
    _r0.o = java_nio_CharBuffer_wrap___char_1ARRAY(_r4.o);
    goto label41;
    label69:;
    XMLVM_SOURCE_POSITION("String.java", 332)
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.count_ = _r5.i;
    XMLVM_SOURCE_POSITION("String.java", 333)
    XMLVM_CLASS_INIT(char)
    _r4.o = XMLVMArray_createSingleDimension(__CLASS_char, _r5.i);
    XMLVM_CHECK_NPE(6)
    ((java_lang_String*) _r6.o)->fields.java_lang_String.value_ = _r4.o;
    goto label55;
    label76:;
    XMLVM_SOURCE_POSITION("String.java", 336)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r4.o)
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____byte_1ARRAY_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("String.java", 353)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____byte_1ARRAY_int_int_java_lang_String(_r2.o, _r3.o, _r0.i, _r1.i, _r4.o);
    XMLVM_SOURCE_POSITION("String.java", 354)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 367)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____char_1ARRAY_int_int(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 368)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    _r2.i = 0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 387)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 390)
    if (_r5.i < 0) goto label35;
    if (_r6.i < 0) goto label35;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    _r0.i = _r0.i - _r5.i;
    if (_r6.i > _r0.i) goto label35;
    XMLVM_SOURCE_POSITION("String.java", 391)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_ = _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 392)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r6.i);
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 393)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.count_ = _r6.i;
    XMLVM_SOURCE_POSITION("String.java", 394)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r5.i, _r0.o, _r2.i, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 398)
    XMLVM_EXIT_METHOD()
    return;
    label35:;
    XMLVM_SOURCE_POSITION("String.java", 396)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____int_int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____int_int_char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    _r4.o = n3;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 404)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 405)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.value_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 406)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.offset_ = _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 407)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.count_ = _r3.i;
    XMLVM_SOURCE_POSITION("String.java", 408)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____char_1ARRAY_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____char_1ARRAY_int_int_boolean]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    _r7.i = n4;
    _r1.i = 0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 417)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 418)
    if (_r5.i >= 0) goto label38;
    XMLVM_SOURCE_POSITION("String.java", 419)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "offset: "
    _r2.o = xmlvm_create_java_string_from_pool(171);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r5.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label38:;
    XMLVM_SOURCE_POSITION("String.java", 420)
    if (_r6.i >= 0) goto label65;
    XMLVM_SOURCE_POSITION("String.java", 421)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "count: "
    _r2.o = xmlvm_create_java_string_from_pool(172);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r6.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label65:;
    XMLVM_SOURCE_POSITION("String.java", 423)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    _r0.i = _r0.i - _r5.i;
    if (_r0.i >= _r6.i) goto label96;
    XMLVM_SOURCE_POSITION("String.java", 424)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT___(_r1.o);
    // "offset + count: "
    _r2.o = xmlvm_create_java_string_from_pool(173);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    _r2.i = _r5.i + _r6.i;
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r2.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label96:;
    XMLVM_SOURCE_POSITION("String.java", 426)
    if (_r7.i == 0) goto label105;
    XMLVM_SOURCE_POSITION("String.java", 428)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.value_ = _r4.o;
    XMLVM_SOURCE_POSITION("String.java", 429)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_ = _r5.i;
    label102:;
    XMLVM_SOURCE_POSITION("String.java", 437)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.count_ = _r6.i;
    XMLVM_SOURCE_POSITION("String.java", 438)
    XMLVM_EXIT_METHOD()
    return;
    label105:;
    XMLVM_SOURCE_POSITION("String.java", 433)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r6.i);
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 434)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r5.i, _r0.o, _r1.i, _r6.i);
    XMLVM_SOURCE_POSITION("String.java", 435)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_ = _r1.i;
    goto label102;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 446)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 447)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_String*) _r2.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 448)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.offset_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 449)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 450)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.o = n1;
    _r7.o = n2;
    _r4.i = 0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 456)
    XMLVM_CHECK_NPE(5)
    java_lang_Object___INIT___(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 457)
    if (_r6.o != JAVA_NULL) goto label15;
    XMLVM_SOURCE_POSITION("String.java", 458)
    // "null"
    _r6.o = xmlvm_create_java_string_from_pool(63);
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 460)
    if (_r7.o != JAVA_NULL) goto label19;
    XMLVM_SOURCE_POSITION("String.java", 461)
    // "null"
    _r7.o = xmlvm_create_java_string_from_pool(63);
    label19:;
    XMLVM_SOURCE_POSITION("String.java", 463)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 464)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 465)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 466)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("String.java", 467)
    XMLVM_CHECK_NPE(7)
    _r0.o = ((java_lang_String*) _r7.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r4.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("String.java", 468)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_String_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_String_java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.o = n2;
    _r8.o = n3;
    _r4.i = 0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 474)
    XMLVM_CHECK_NPE(5)
    java_lang_Object___INIT___(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 475)
    if (_r6.o != JAVA_NULL) goto label15;
    XMLVM_SOURCE_POSITION("String.java", 476)
    // "null"
    _r6.o = xmlvm_create_java_string_from_pool(63);
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 478)
    if (_r7.o != JAVA_NULL) goto label19;
    XMLVM_SOURCE_POSITION("String.java", 479)
    // "null"
    _r7.o = xmlvm_create_java_string_from_pool(63);
    label19:;
    XMLVM_SOURCE_POSITION("String.java", 481)
    if (_r8.o != JAVA_NULL) goto label23;
    XMLVM_SOURCE_POSITION("String.java", 482)
    // "null"
    _r8.o = xmlvm_create_java_string_from_pool(63);
    label23:;
    XMLVM_SOURCE_POSITION("String.java", 484)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 485)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 486)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 487)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("String.java", 488)
    XMLVM_CHECK_NPE(7)
    _r0.o = ((java_lang_String*) _r7.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r4.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("String.java", 489)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_lang_String*) _r8.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r4.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i + _r4.i;
    XMLVM_CHECK_NPE(8)
    _r4.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("String.java", 491)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_StringBuffer]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 500)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 501)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.offset_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 502)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w3113aaac27b1c17)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 503)

    
    // Red class access removed: java.lang.StringBuffer::getValue
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 504)

    
    // Red class access removed: java.lang.StringBuffer::length
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_lang_String*) _r1.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 505)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("String.java", 506)
    XMLVM_MEMCPY(curThread_w3113aaac27b1c17->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3113aaac27b1c17, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac27b1c17)
        XMLVM_CATCH_SPECIFIC(w3113aaac27b1c17,java_lang_Object,28)
    XMLVM_CATCH_END(w3113aaac27b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac27b1c17)
    label28:;
    XMLVM_TRY_BEGIN(w3113aaac27b1c19)
    // Begin try
    java_lang_Thread* curThread_w3113aaac27b1c19aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3113aaac27b1c19aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac27b1c19)
        XMLVM_CATCH_SPECIFIC(w3113aaac27b1c19,java_lang_Object,28)
    XMLVM_CATCH_END(w3113aaac27b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac27b1c19)
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____int_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____int_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.i = n2;
    _r8.i = n3;
    _r3.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 529)
    XMLVM_CHECK_NPE(5)
    java_lang_Object___INIT___(_r5.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset2_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.charset3_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 530)
    if (_r7.i < 0) goto label18;
    if (_r8.i < 0) goto label18;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r3.i = _r3.i - _r8.i;
    if (_r7.i <= _r3.i) goto label24;
    label18:;
    XMLVM_SOURCE_POSITION("String.java", 531)
    _r3.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(3)
    java_lang_IndexOutOfBoundsException___INIT___(_r3.o);
    XMLVM_THROW_CUSTOM(_r3.o)
    label24:;
    XMLVM_SOURCE_POSITION("String.java", 533)
    _r3.i = 0;
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_ = _r3.i;
    XMLVM_SOURCE_POSITION("String.java", 534)
    _r3.i = _r8.i * 2;
    XMLVM_CLASS_INIT(char)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_char, _r3.i);
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.value_ = _r3.o;
    XMLVM_SOURCE_POSITION("String.java", 535)
    _r1.i = _r7.i + _r8.i;
    _r0.i = 0;
    _r2 = _r7;
    label37:;
    XMLVM_SOURCE_POSITION("String.java", 537)
    if (_r2.i >= _r1.i) goto label51;
    XMLVM_SOURCE_POSITION("String.java", 538)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r2.i);
    _r3.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    _r3.i = java_lang_Character_toChars___int_char_1ARRAY_int(_r3.i, _r4.o, _r0.i);
    _r0.i = _r0.i + _r3.i;
    _r2.i = _r2.i + 1;
    goto label37;
    label51:;
    XMLVM_SOURCE_POSITION("String.java", 540)
    XMLVM_CHECK_NPE(5)
    ((java_lang_String*) _r5.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 541)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_StringBuilder(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_StringBuilder]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r0.o = JAVA_NULL;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 553)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset2_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.charset3_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 554)
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_ = _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 555)
    XMLVM_CHECK_NPE(4)
    _r0.i = java_lang_AbstractStringBuilder_length__(_r4.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 556)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(3)
    ((java_lang_String*) _r3.o)->fields.java_lang_String.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("String.java", 557)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(4)
    java_lang_AbstractStringBuilder_getChars___int_int_char_1ARRAY_int(_r4.o, _r2.i, _r0.i, _r1.o, _r2.i);
    XMLVM_SOURCE_POSITION("String.java", 558)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_String___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___INIT____java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.String", "<init>", "?")
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
    _r7.o = me;
    _r8.o = n1;
    _r9.i = n2;
    _r6.i = 0;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("String.java", 564)
    XMLVM_CHECK_NPE(7)
    java_lang_Object___INIT___(_r7.o);
    XMLVM_SOURCE_POSITION("String.java", 58)
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.charset_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 59)
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.charset2_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 60)
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.charset3_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 565)
    if (_r8.o != JAVA_NULL) goto label15;
    XMLVM_SOURCE_POSITION("String.java", 566)
    // "null"
    _r8.o = xmlvm_create_java_string_from_pool(63);
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 568)
    _r1.o = java_lang_String_valueOf___int(_r9.i);
    XMLVM_SOURCE_POSITION("String.java", 569)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(1)
    _r3.i = ((java_lang_String*) _r1.o)->fields.java_lang_String.count_;
    _r0.i = _r2.i + _r3.i;
    XMLVM_SOURCE_POSITION("String.java", 570)
    XMLVM_CLASS_INIT(char)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.value_ = _r2.o;
    XMLVM_SOURCE_POSITION("String.java", 571)
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.offset_ = _r6.i;
    XMLVM_SOURCE_POSITION("String.java", 572)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_lang_String*) _r8.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(7)
    _r4.o = ((java_lang_String*) _r7.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(8)
    _r5.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r4.o, _r6.i, _r5.i);
    XMLVM_SOURCE_POSITION("String.java", 573)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_lang_String*) _r1.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(1)
    _r3.i = ((java_lang_String*) _r1.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(7)
    _r4.o = ((java_lang_String*) _r7.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(8)
    _r5.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(1)
    _r6.i = ((java_lang_String*) _r1.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r4.o, _r5.i, _r6.i);
    XMLVM_SOURCE_POSITION("String.java", 574)
    XMLVM_CHECK_NPE(7)
    ((java_lang_String*) _r7.o)->fields.java_lang_String.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("String.java", 575)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_String_charAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_charAt___int]
    XMLVM_ENTER_METHOD("java.lang.String", "charAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 587)
    if (_r3.i < 0) goto label14;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.count_;
    if (_r3.i >= _r0.i) goto label14;
    XMLVM_SOURCE_POSITION("String.java", 588)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_String*) _r2.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.offset_;
    _r1.i = _r1.i + _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    XMLVM_SOURCE_POSITION("String.java", 590)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_String_compareValue___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_compareValue___char]
    XMLVM_ENTER_METHOD("java.lang.String", "compareValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 595)
    _r0.i = 128;
    if (_r2.i >= _r0.i) goto label18;
    XMLVM_SOURCE_POSITION("String.java", 596)
    _r0.i = 65;
    if (_r0.i > _r2.i) goto label16;
    _r0.i = 90;
    if (_r2.i > _r0.i) goto label16;
    XMLVM_SOURCE_POSITION("String.java", 597)
    _r0.i = _r2.i + 32;
    _r0.i = _r0.i & 0xffff;
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 601)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("String.java", 599)
    goto label15;
    label18:;
    _r0.i = java_lang_Character_toUpperCase___char(_r2.i);
    _r0.i = java_lang_Character_toLowerCase___char(_r0.i);
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_String_toLowerCase___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toLowerCase___char]
    XMLVM_ENTER_METHOD("java.lang.String", "toLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 606)
    _r0.i = 128;
    if (_r2.i >= _r0.i) goto label18;
    XMLVM_SOURCE_POSITION("String.java", 607)
    _r0.i = 65;
    if (_r0.i > _r2.i) goto label16;
    _r0.i = 90;
    if (_r2.i > _r0.i) goto label16;
    XMLVM_SOURCE_POSITION("String.java", 608)
    _r0.i = _r2.i + 32;
    _r0.i = _r0.i & 0xffff;
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 612)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("String.java", 610)
    goto label15;
    label18:;
    _r0.i = java_lang_Character_toLowerCase___char(_r2.i);
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_String_toUpperCase___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toUpperCase___char]
    XMLVM_ENTER_METHOD("java.lang.String", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 617)
    _r0.i = 128;
    if (_r2.i >= _r0.i) goto label20;
    XMLVM_SOURCE_POSITION("String.java", 618)
    _r0.i = 97;
    if (_r0.i > _r2.i) goto label18;
    _r0.i = 122;
    if (_r2.i > _r0.i) goto label18;
    XMLVM_SOURCE_POSITION("String.java", 619)
    _r0.i = 32;
    _r0.i = _r2.i - _r0.i;
    _r0.i = _r0.i & 0xffff;
    label17:;
    XMLVM_SOURCE_POSITION("String.java", 623)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("String.java", 621)
    goto label17;
    label20:;
    _r0.i = java_lang_Character_toUpperCase___char(_r2.i);
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_compareTo___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_compareTo___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "compareTo", "?")
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
    _r10.o = me;
    _r11.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 648)
    XMLVM_CHECK_NPE(10)
    _r1.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(11)
    _r3.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 649)
    XMLVM_CHECK_NPE(10)
    _r7.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(10)
    _r8.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(11)
    _r9.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r8.i >= _r9.i) goto label38;
    XMLVM_CHECK_NPE(10)
    _r8.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    label14:;
    _r0.i = _r7.i + _r8.i;
    XMLVM_SOURCE_POSITION("String.java", 650)
    XMLVM_CHECK_NPE(11)
    _r6.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    _r4 = _r3;
    _r2 = _r1;
    label20:;
    XMLVM_SOURCE_POSITION("String.java", 651)
    if (_r2.i >= _r0.i) goto label41;
    XMLVM_SOURCE_POSITION("String.java", 652)
    XMLVM_CHECK_NPE(10)
    _r7.o = ((java_lang_String*) _r10.o)->fields.java_lang_String.value_;
    _r1.i = _r2.i + 1;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r2.i);
    _r7.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r3.i = _r4.i + 1;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r4.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r5.i = _r7.i - _r8.i;
    if (_r5.i == 0) goto label49;
    _r7 = _r5;
    label37:;
    XMLVM_SOURCE_POSITION("String.java", 653)
    XMLVM_SOURCE_POSITION("String.java", 656)
    XMLVM_EXIT_METHOD()
    return _r7.i;
    label38:;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    goto label14;
    label41:;
    XMLVM_CHECK_NPE(10)
    _r7.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    _r7.i = _r7.i - _r8.i;
    _r3 = _r4;
    _r1 = _r2;
    goto label37;
    label49:;
    _r4 = _r3;
    _r2 = _r1;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_compareToIgnoreCase___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_compareToIgnoreCase___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "compareToIgnoreCase", "?")
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
    _r13.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 680)
    XMLVM_CHECK_NPE(12)
    _r3.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(13)
    _r5.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 681)
    XMLVM_CHECK_NPE(12)
    _r9.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(12)
    _r10.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(13)
    _r11.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.count_;
    if (_r10.i >= _r11.i) goto label37;
    XMLVM_CHECK_NPE(12)
    _r10.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    label14:;
    _r2.i = _r9.i + _r10.i;
    XMLVM_SOURCE_POSITION("String.java", 683)
    XMLVM_CHECK_NPE(13)
    _r8.o = ((java_lang_String*) _r13.o)->fields.java_lang_String.value_;
    _r6 = _r5;
    _r4 = _r3;
    label20:;
    XMLVM_SOURCE_POSITION("String.java", 684)
    if (_r4.i >= _r2.i) goto label54;
    XMLVM_SOURCE_POSITION("String.java", 685)
    XMLVM_CHECK_NPE(12)
    _r9.o = ((java_lang_String*) _r12.o)->fields.java_lang_String.value_;
    _r3.i = _r4.i + 1;
    XMLVM_CHECK_NPE(9)
    XMLVM_CHECK_ARRAY_BOUNDS(_r9.o, _r4.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r9.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r5.i = _r6.i + 1;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r6.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    if (_r0.i != _r1.i) goto label40;
    _r6 = _r5;
    _r4 = _r3;
    XMLVM_SOURCE_POSITION("String.java", 686)
    goto label20;
    label37:;
    XMLVM_CHECK_NPE(13)
    _r10.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.count_;
    goto label14;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 688)
    XMLVM_CHECK_NPE(12)
    _r0.i = java_lang_String_compareValue___char(_r12.o, _r0.i);
    XMLVM_SOURCE_POSITION("String.java", 689)
    XMLVM_CHECK_NPE(12)
    _r1.i = java_lang_String_compareValue___char(_r12.o, _r1.i);
    _r7.i = _r0.i - _r1.i;
    if (_r7.i == 0) goto label62;
    XMLVM_SOURCE_POSITION("String.java", 690)
    _r9 = _r7;
    label53:;
    XMLVM_SOURCE_POSITION("String.java", 691)
    XMLVM_SOURCE_POSITION("String.java", 694)
    XMLVM_EXIT_METHOD()
    return _r9.i;
    label54:;
    XMLVM_CHECK_NPE(12)
    _r9.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(13)
    _r10.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.count_;
    _r9.i = _r9.i - _r10.i;
    _r5 = _r6;
    _r3 = _r4;
    goto label53;
    label62:;
    _r6 = _r5;
    _r4 = _r3;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_concat___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_concat___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "concat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 706)
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    if (_r1.i != 0) goto label7;
    _r1 = _r6;
    label6:;
    XMLVM_SOURCE_POSITION("String.java", 707)
    XMLVM_SOURCE_POSITION("String.java", 716)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label7:;
    XMLVM_SOURCE_POSITION("String.java", 710)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r2.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 711)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    if (_r1.i <= 0) goto label27;
    XMLVM_SOURCE_POSITION("String.java", 712)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r5.i, _r3.i);
    label27:;
    XMLVM_SOURCE_POSITION("String.java", 714)
    XMLVM_CHECK_NPE(7)
    _r1.o = ((java_lang_String*) _r7.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(7)
    _r2.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(7)
    _r4.i = ((java_lang_String*) _r7.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    _r1.o = __NEW_java_lang_String();
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(1)
    java_lang_String___INIT____int_int_char_1ARRAY(_r1.o, _r5.i, _r2.i, _r0.o);
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_copyValueOf___char_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_copyValueOf___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "copyValueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 731)
    _r0.o = __NEW_java_lang_String();
    _r1.i = 0;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r3.o, _r1.i, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_copyValueOf___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_copyValueOf___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "copyValueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("String.java", 753)
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r1.o, _r2.i, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_defaultCharset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_defaultCharset__]
    XMLVM_ENTER_METHOD("java.lang.String", "defaultCharset", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("String.java", 757)
    _r1.o = java_lang_String_GET_DefaultCharset();
    if (_r1.o != JAVA_NULL) goto label37;
    XMLVM_SOURCE_POSITION("String.java", 758)
    _r1.o = __NEW_org_apache_harmony_luni_util_PriviAction();
    // "file.encoding"
    _r2.o = xmlvm_create_java_string_from_pool(174);
    // "ISO8859_1"
    _r3.o = xmlvm_create_java_string_from_pool(116);
    XMLVM_CHECK_NPE(1)
    org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String_java_lang_String(_r1.o, _r2.o, _r3.o);
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r1.o);
    _r0.o = _r0.o;
    XMLVM_TRY_BEGIN(w3113aaac40b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 764)
    _r1.o = java_nio_charset_Charset_forName___java_lang_String(_r0.o);
    java_lang_String_PUT_DefaultCharset( _r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac40b1c12)
    XMLVM_CATCH_END(w3113aaac40b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac40b1c12)
    label25:;
    XMLVM_SOURCE_POSITION("String.java", 771)
    _r1.o = java_lang_String_GET_DefaultCharset();
    if (_r1.o != JAVA_NULL) goto label37;
    XMLVM_SOURCE_POSITION("String.java", 772)
    // "ISO-8859-1"
    _r1.o = xmlvm_create_java_string_from_pool(175);
    _r1.o = java_nio_charset_Charset_forName___java_lang_String(_r1.o);
    java_lang_String_PUT_DefaultCharset( _r1.o);
    label37:;
    XMLVM_SOURCE_POSITION("String.java", 775)
    _r1.o = java_lang_String_GET_DefaultCharset();
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 767)
    java_lang_Thread* curThread_w3113aaac40b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3113aaac40b1c27->fields.java_lang_Thread.xmlvmException_;
    goto label25;
    label42:;
    XMLVM_SOURCE_POSITION("String.java", 765)
    java_lang_Thread* curThread_w3113aaac40b1c31 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3113aaac40b1c31->fields.java_lang_Thread.xmlvmException_;
    goto label25;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_endsWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_endsWith___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "endsWith", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 790)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    _r0.i = _r0.i - _r1.i;
    _r1.i = 0;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_String_regionMatches___int_java_lang_String_int_int(_r3.o, _r0.i, _r4.o, _r1.i, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.String", "equals", "?")
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
    _r10.o = me;
    _r11.o = n1;
    _r9.i = 1;
    _r8.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 806)
    if (_r11.o != _r10.o) goto label6;
    _r5 = _r9;
    label5:;
    XMLVM_SOURCE_POSITION("String.java", 807)
    XMLVM_SOURCE_POSITION("String.java", 823)
    XMLVM_EXIT_METHOD()
    return _r5.i;
    label6:;
    XMLVM_SOURCE_POSITION("String.java", 809)
    XMLVM_CLASS_INIT(java_lang_String)
    _r5.i = XMLVM_ISA(_r11.o, __CLASS_java_lang_String);
    if (_r5.i == 0) goto label60;
    XMLVM_SOURCE_POSITION("String.java", 810)
    _r0 = _r11;
    _r0.o = _r0.o;
    _r3 = _r0;
    XMLVM_SOURCE_POSITION("String.java", 811)
    XMLVM_CHECK_NPE(10)
    _r1.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.hashCode_;
    XMLVM_SOURCE_POSITION("String.java", 812)
    XMLVM_CHECK_NPE(3)
    _r4.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.hashCode_;
    XMLVM_SOURCE_POSITION("String.java", 813)
    XMLVM_CHECK_NPE(10)
    _r5.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(3)
    _r6.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    if (_r5.i != _r6.i) goto label30;
    if (_r1.i == _r4.i) goto label32;
    if (_r1.i == 0) goto label32;
    if (_r4.i == 0) goto label32;
    label30:;
    _r5 = _r8;
    XMLVM_SOURCE_POSITION("String.java", 814)
    goto label5;
    label32:;
    XMLVM_SOURCE_POSITION("String.java", 816)
    _r2.i = 0;
    label33:;
    XMLVM_CHECK_NPE(10)
    _r5.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    if (_r2.i >= _r5.i) goto label58;
    XMLVM_SOURCE_POSITION("String.java", 817)
    XMLVM_CHECK_NPE(10)
    _r5.o = ((java_lang_String*) _r10.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(10)
    _r6.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.offset_;
    _r6.i = _r6.i + _r2.i;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r6.i);
    _r5.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    XMLVM_CHECK_NPE(3)
    _r6.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    _r7.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_;
    _r7.i = _r7.i + _r2.i;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    _r6.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    if (_r5.i == _r6.i) goto label55;
    _r5 = _r8;
    XMLVM_SOURCE_POSITION("String.java", 818)
    goto label5;
    label55:;
    _r2.i = _r2.i + 1;
    goto label33;
    label58:;
    _r5 = _r9;
    XMLVM_SOURCE_POSITION("String.java", 821)
    goto label5;
    label60:;
    _r5 = _r8;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_equalsIgnoreCase___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_equalsIgnoreCase___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "equalsIgnoreCase", "?")
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
    _r13.o = n1;
    _r11.i = 1;
    _r10.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 836)
    if (_r13.o != _r12.o) goto label6;
    _r8 = _r11;
    label5:;
    XMLVM_SOURCE_POSITION("String.java", 837)
    XMLVM_SOURCE_POSITION("String.java", 855)
    XMLVM_EXIT_METHOD()
    return _r8.i;
    label6:;
    XMLVM_SOURCE_POSITION("String.java", 839)
    if (_r13.o == JAVA_NULL) goto label14;
    XMLVM_CHECK_NPE(12)
    _r8.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(13)
    _r9.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.count_;
    if (_r8.i == _r9.i) goto label16;
    label14:;
    _r8 = _r10;
    XMLVM_SOURCE_POSITION("String.java", 840)
    goto label5;
    label16:;
    XMLVM_SOURCE_POSITION("String.java", 843)
    XMLVM_CHECK_NPE(12)
    _r3.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(13)
    _r5.i = ((java_lang_String*) _r13.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 844)
    XMLVM_CHECK_NPE(12)
    _r8.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(12)
    _r9.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    _r2.i = _r8.i + _r9.i;
    XMLVM_SOURCE_POSITION("String.java", 846)
    XMLVM_CHECK_NPE(13)
    _r7.o = ((java_lang_String*) _r13.o)->fields.java_lang_String.value_;
    _r6 = _r5;
    _r4 = _r3;
    label30:;
    XMLVM_SOURCE_POSITION("String.java", 847)
    if (_r4.i >= _r2.i) goto label66;
    XMLVM_SOURCE_POSITION("String.java", 848)
    XMLVM_CHECK_NPE(12)
    _r8.o = ((java_lang_String*) _r12.o)->fields.java_lang_String.value_;
    _r3.i = _r4.i + 1;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r4.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r5.i = _r6.i + 1;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r6.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    if (_r0.i == _r1.i) goto label68;
    XMLVM_CHECK_NPE(12)
    _r8.i = java_lang_String_toUpperCase___char(_r12.o, _r0.i);
    XMLVM_CHECK_NPE(12)
    _r9.i = java_lang_String_toUpperCase___char(_r12.o, _r1.i);
    if (_r8.i == _r9.i) goto label68;
    XMLVM_CHECK_NPE(12)
    _r8.i = java_lang_String_toLowerCase___char(_r12.o, _r0.i);
    XMLVM_CHECK_NPE(12)
    _r9.i = java_lang_String_toLowerCase___char(_r12.o, _r1.i);
    if (_r8.i == _r9.i) goto label68;
    _r8 = _r10;
    XMLVM_SOURCE_POSITION("String.java", 852)
    goto label5;
    label66:;
    _r8 = _r11;
    goto label5;
    label68:;
    _r6 = _r5;
    _r4 = _r3;
    goto label30;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_getBytes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getBytes__]
    XMLVM_ENTER_METHOD("java.lang.String", "getBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    XMLVM_SOURCE_POSITION("String.java", 867)
    XMLVM_CHECK_NPE(6)
    _r2.o = java_lang_String_defaultCharset__(_r6.o);
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    _r3.o = java_nio_CharBuffer_wrap___char_1ARRAY_int_int(_r3.o, _r4.i, _r5.i);
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_charset_Charset_encode___java_nio_CharBuffer(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("String.java", 869)
    XMLVM_CHECK_NPE(0)
    _r2.i = java_nio_Buffer_limit__(_r0.o);
    XMLVM_CLASS_INIT(byte)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r2.i);
    XMLVM_SOURCE_POSITION("String.java", 870)
    XMLVM_CHECK_NPE(0)
    java_nio_ByteBuffer_get___byte_1ARRAY(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("String.java", 871)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

void java_lang_String_getBytes___int_int_byte_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getBytes___int_int_byte_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.String", "getBytes", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r4.o = me;
    _r5.i = n1;
    _r6.i = n2;
    _r7.o = n3;
    _r8.i = n4;
    XMLVM_SOURCE_POSITION("String.java", 895)
    if (_r5.i < 0) goto label39;
    if (_r5.i > _r6.i) goto label39;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r6.i > _r3.i) goto label39;
    XMLVM_SOURCE_POSITION("String.java", 896)
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r6.i = _r6.i + _r3.i;
    XMLVM_TRY_BEGIN(w3113aaac45b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 898)
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r1.i = _r3.i + _r5.i;
    _r2 = _r8;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac45b1c14)
    XMLVM_CATCH_END(w3113aaac45b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac45b1c14)
    label16:;
    XMLVM_TRY_BEGIN(w3113aaac45b1c16)
    // Begin try
    if (_r1.i >= _r6.i) { XMLVM_MEMCPY(curThread_w3113aaac45b1c16->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3113aaac45b1c16, sizeof(XMLVM_JMP_BUF)); goto label45; };
    XMLVM_SOURCE_POSITION("String.java", 899)
    _r8.i = _r2.i + 1;
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = (_r3.i << 24) >> 24;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r2.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac45b1c16)
    XMLVM_CATCH_END(w3113aaac45b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac45b1c16)
    _r1.i = _r1.i + 1;
    _r2 = _r8;
    goto label16;
    label31:;
    XMLVM_SOURCE_POSITION("String.java", 901)
    java_lang_Thread* curThread_w3113aaac45b1c22 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3113aaac45b1c22->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("String.java", 902)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label39:;
    XMLVM_SOURCE_POSITION("String.java", 905)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label45:;
    XMLVM_SOURCE_POSITION("String.java", 907)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_getBytes___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getBytes___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "getBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 919)
    XMLVM_CHECK_NPE(6)
    _r2.o = java_lang_String_getCharset___java_lang_String(_r6.o, _r7.o);
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    _r3.o = java_nio_CharBuffer_wrap___char_1ARRAY_int_int(_r3.o, _r4.i, _r5.i);
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_charset_Charset_encode___java_nio_CharBuffer(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("String.java", 921)
    XMLVM_CHECK_NPE(0)
    _r2.i = java_nio_Buffer_limit__(_r0.o);
    XMLVM_CLASS_INIT(byte)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r2.i);
    XMLVM_SOURCE_POSITION("String.java", 922)
    XMLVM_CHECK_NPE(0)
    java_nio_ByteBuffer_get___byte_1ARRAY(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("String.java", 923)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_getBytes___java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getBytes___java_nio_charset_Charset]
    XMLVM_ENTER_METHOD("java.lang.String", "getBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 945)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    _r2.o = java_nio_CharBuffer_wrap___char_1ARRAY_int_int(_r2.o, _r3.i, _r4.i);
    XMLVM_CHECK_NPE(6)
    _r0.o = java_nio_charset_Charset_encode___java_nio_CharBuffer(_r6.o, _r2.o);
    XMLVM_SOURCE_POSITION("String.java", 947)
    XMLVM_CHECK_NPE(0)
    _r2.i = java_nio_Buffer_limit__(_r0.o);
    XMLVM_CLASS_INIT(byte)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r2.i);
    XMLVM_SOURCE_POSITION("String.java", 948)
    XMLVM_CHECK_NPE(0)
    java_nio_ByteBuffer_get___byte_1ARRAY(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("String.java", 949)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_getCharset___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getCharset___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "getCharset", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 954)
    _r0.o = java_lang_String_GET_lastCharset();
    if (_r0.o == JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("String.java", 955)
    XMLVM_CHECK_NPE(0)
    _r2.o = java_nio_charset_Charset_name__(_r0.o);
    XMLVM_CHECK_NPE(4)
    _r2.i = java_lang_String_equalsIgnoreCase___java_lang_String(_r4.o, _r2.o);
    if (_r2.i != 0) goto label20;
    label14:;
    XMLVM_TRY_BEGIN(w3113aaac48b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 957)
    _r0.o = java_nio_charset_Charset_forName___java_lang_String(_r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac48b1c11)
    XMLVM_CATCH_END(w3113aaac48b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac48b1c11)
    XMLVM_SOURCE_POSITION("String.java", 965)
    java_lang_String_PUT_lastCharset( _r0.o);
    label20:;
    XMLVM_SOURCE_POSITION("String.java", 967)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label21:;
    XMLVM_SOURCE_POSITION("String.java", 958)
    java_lang_Thread* curThread_w3113aaac48b1c19 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3113aaac48b1c19->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("String.java", 959)

    
    // Red class access removed: java.io.UnsupportedEncodingException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::initCause
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label36:;
    XMLVM_SOURCE_POSITION("String.java", 961)
    java_lang_Thread* curThread_w3113aaac48b1c29 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3113aaac48b1c29->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("String.java", 962)

    
    // Red class access removed: java.io.UnsupportedEncodingException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::initCause
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.UnsupportedEncodingException::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

void java_lang_String_getChars___int_int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getChars___int_int_char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.String", "getChars", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r3.o = me;
    _r4.i = n1;
    _r5.i = n2;
    _r6.o = n3;
    _r7.i = n4;
    XMLVM_SOURCE_POSITION("String.java", 992)
    if (_r4.i < 0) goto label19;
    if (_r4.i > _r5.i) goto label19;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    if (_r5.i > _r0.i) goto label19;
    XMLVM_SOURCE_POSITION("String.java", 993)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_;
    _r1.i = _r1.i + _r4.i;
    _r2.i = _r5.i - _r4.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r6.o, _r7.i, _r2.i);
    XMLVM_SOURCE_POSITION("String.java", 997)
    XMLVM_EXIT_METHOD()
    return;
    label19:;
    XMLVM_SOURCE_POSITION("String.java", 995)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.String", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1001)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.hashCode_;
    if (_r2.i != 0) goto label34;
    XMLVM_SOURCE_POSITION("String.java", 1002)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r2.i != 0) goto label10;
    XMLVM_SOURCE_POSITION("String.java", 1003)
    _r2.i = 0;
    label9:;
    XMLVM_SOURCE_POSITION("String.java", 1011)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 1005)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1006)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    label13:;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r2.i = _r2.i + _r3.i;
    if (_r1.i >= _r2.i) goto label32;
    XMLVM_SOURCE_POSITION("String.java", 1007)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = _r0.i << 5;
    _r3.i = _r3.i - _r0.i;
    _r0.i = _r2.i + _r3.i;
    _r1.i = _r1.i + 1;
    goto label13;
    label32:;
    XMLVM_SOURCE_POSITION("String.java", 1009)
    XMLVM_CHECK_NPE(4)
    ((java_lang_String*) _r4.o)->fields.java_lang_String.hashCode_ = _r0.i;
    label34:;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.hashCode_;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_indexOf___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_indexOf___int]
    XMLVM_ENTER_METHOD("java.lang.String", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1025)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_indexOf___int_int(_r1.o, _r2.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_indexOf___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_indexOf___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.i = n2;
    _r3.i = 65535;
    XMLVM_SOURCE_POSITION("String.java", 1041)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r6.i >= _r2.i) goto label68;
    XMLVM_SOURCE_POSITION("String.java", 1042)
    if (_r6.i >= 0) goto label10;
    XMLVM_SOURCE_POSITION("String.java", 1043)
    _r6.i = 0;
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 1045)
    if (_r5.i < 0) goto label39;
    if (_r5.i > _r3.i) goto label39;
    XMLVM_SOURCE_POSITION("String.java", 1046)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r1.i = _r2.i + _r6.i;
    label18:;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    _r2.i = _r2.i + _r3.i;
    if (_r1.i >= _r2.i) goto label68;
    XMLVM_SOURCE_POSITION("String.java", 1047)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.i != _r5.i) goto label36;
    XMLVM_SOURCE_POSITION("String.java", 1048)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r2.i = _r1.i - _r2.i;
    label35:;
    XMLVM_SOURCE_POSITION("String.java", 1062)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label36:;
    _r1.i = _r1.i + 1;
    goto label18;
    label39:;
    XMLVM_SOURCE_POSITION("String.java", 1051)
    if (_r5.i <= _r3.i) goto label68;
    _r2.i = 1114111;
    if (_r5.i > _r2.i) goto label68;
    XMLVM_SOURCE_POSITION("String.java", 1052)
    _r1 = _r6;
    label47:;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r1.i >= _r2.i) goto label68;
    XMLVM_SOURCE_POSITION("String.java", 1053)
    XMLVM_CHECK_NPE(4)
    _r0.i = java_lang_String_codePointAt___int(_r4.o, _r1.i);
    if (_r0.i != _r5.i) goto label59;
    XMLVM_SOURCE_POSITION("String.java", 1054)
    _r2 = _r1;
    XMLVM_SOURCE_POSITION("String.java", 1055)
    goto label35;
    label59:;
    XMLVM_SOURCE_POSITION("String.java", 1056)
    _r2.i = 65536;
    if (_r0.i < _r2.i) goto label65;
    XMLVM_SOURCE_POSITION("String.java", 1057)
    _r1.i = _r1.i + 1;
    label65:;
    _r1.i = _r1.i + 1;
    goto label47;
    label68:;
    _r2.i = -1;
    goto label35;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_indexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_indexOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1078)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_indexOf___java_lang_String_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_indexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_indexOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.String", "indexOf", "?")
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
    _r11.o = me;
    _r12.o = n1;
    _r13.i = n2;
    _r10.i = -1;
    XMLVM_SOURCE_POSITION("String.java", 1096)
    if (_r13.i >= 0) goto label4;
    XMLVM_SOURCE_POSITION("String.java", 1097)
    _r13.i = 0;
    label4:;
    XMLVM_SOURCE_POSITION("String.java", 1099)
    XMLVM_CHECK_NPE(12)
    _r5.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    if (_r5.i <= 0) goto label64;
    XMLVM_SOURCE_POSITION("String.java", 1100)
    XMLVM_SOURCE_POSITION("String.java", 1101)
    _r8.i = _r5.i + _r13.i;
    XMLVM_CHECK_NPE(11)
    _r9.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r8.i <= _r9.i) goto label16;
    _r8 = _r10;
    label15:;
    XMLVM_SOURCE_POSITION("String.java", 1102)
    XMLVM_SOURCE_POSITION("String.java", 1123)
    XMLVM_EXIT_METHOD()
    return _r8.i;
    label16:;
    XMLVM_SOURCE_POSITION("String.java", 1104)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((java_lang_String*) _r12.o)->fields.java_lang_String.value_;
    XMLVM_SOURCE_POSITION("String.java", 1105)
    XMLVM_CHECK_NPE(12)
    _r6.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 1106)
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r6.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    _r0.i = _r6.i + _r5.i;
    label24:;
    XMLVM_SOURCE_POSITION("String.java", 1107)
    XMLVM_SOURCE_POSITION("String.java", 1109)
    XMLVM_CHECK_NPE(11)
    _r2.i = java_lang_String_indexOf___int_int(_r11.o, _r1.i, _r13.i);
    if (_r2.i == _r10.i) goto label36;
    XMLVM_SOURCE_POSITION("String.java", 1110)
    _r8.i = _r5.i + _r2.i;
    XMLVM_CHECK_NPE(11)
    _r9.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r8.i <= _r9.i) goto label38;
    label36:;
    _r8 = _r10;
    XMLVM_SOURCE_POSITION("String.java", 1111)
    goto label15;
    label38:;
    XMLVM_SOURCE_POSITION("String.java", 1113)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    _r3.i = _r8.i + _r2.i;
    _r4 = _r6;
    label43:;
    XMLVM_SOURCE_POSITION("String.java", 1114)
    _r4.i = _r4.i + 1;
    if (_r4.i >= _r0.i) goto label57;
    XMLVM_CHECK_NPE(11)
    _r8.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    _r3.i = _r3.i + 1;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r3.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r4.i);
    _r9.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    if (_r8.i == _r9.i) goto label43;
    label57:;
    XMLVM_SOURCE_POSITION("String.java", 1117)
    if (_r4.i != _r0.i) goto label61;
    _r8 = _r2;
    XMLVM_SOURCE_POSITION("String.java", 1118)
    goto label15;
    label61:;
    XMLVM_SOURCE_POSITION("String.java", 1120)
    _r13.i = _r2.i + 1;
    XMLVM_SOURCE_POSITION("String.java", 1121)
    goto label24;
    label64:;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r13.i >= _r8.i) goto label70;
    _r8 = _r13;
    goto label15;
    label70:;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_intern__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_intern__]
    XMLVM_ENTER_METHOD("java.lang.String", "intern", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1136)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_lastIndexOf___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_lastIndexOf___int]
    XMLVM_ENTER_METHOD("java.lang.String", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1150)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.count_;
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(2)
    _r0.i = java_lang_String_lastIndexOf___int_int(_r2.o, _r3.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_lastIndexOf___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_lastIndexOf___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.i = n1;
    _r7.i = n2;
    _r4.i = 65535;
    XMLVM_SOURCE_POSITION("String.java", 1166)
    if (_r7.i < 0) goto label67;
    XMLVM_SOURCE_POSITION("String.java", 1167)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    if (_r7.i < _r2.i) goto label14;
    XMLVM_SOURCE_POSITION("String.java", 1168)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    _r3.i = 1;
    _r7.i = _r2.i - _r3.i;
    label14:;
    XMLVM_SOURCE_POSITION("String.java", 1170)
    if (_r6.i < 0) goto label40;
    if (_r6.i > _r4.i) goto label40;
    XMLVM_SOURCE_POSITION("String.java", 1171)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    _r1.i = _r2.i + _r7.i;
    label22:;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    if (_r1.i < _r2.i) goto label67;
    XMLVM_SOURCE_POSITION("String.java", 1172)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.i != _r6.i) goto label37;
    XMLVM_SOURCE_POSITION("String.java", 1173)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    _r2.i = _r1.i - _r2.i;
    label36:;
    XMLVM_SOURCE_POSITION("String.java", 1187)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label37:;
    _r1.i = _r1.i + -1;
    goto label22;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 1176)
    if (_r6.i <= _r4.i) goto label67;
    _r2.i = 1114111;
    if (_r6.i > _r2.i) goto label67;
    XMLVM_SOURCE_POSITION("String.java", 1177)
    _r1 = _r7;
    label48:;
    if (_r1.i < 0) goto label67;
    XMLVM_SOURCE_POSITION("String.java", 1178)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_lang_String_codePointAt___int(_r5.o, _r1.i);
    if (_r0.i != _r6.i) goto label58;
    XMLVM_SOURCE_POSITION("String.java", 1179)
    _r2 = _r1;
    XMLVM_SOURCE_POSITION("String.java", 1180)
    goto label36;
    label58:;
    XMLVM_SOURCE_POSITION("String.java", 1181)
    _r2.i = 65536;
    if (_r0.i < _r2.i) goto label64;
    XMLVM_SOURCE_POSITION("String.java", 1182)
    _r1.i = _r1.i + -1;
    label64:;
    _r1.i = _r1.i + -1;
    goto label48;
    label67:;
    _r2.i = -1;
    goto label36;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_lastIndexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_lastIndexOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1204)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_String*) _r1.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_lastIndexOf___java_lang_String_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_lastIndexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_lastIndexOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.String", "lastIndexOf", "?")
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
    _r11.o = me;
    _r12.o = n1;
    _r13.i = n2;
    _r10.i = -1;
    XMLVM_SOURCE_POSITION("String.java", 1222)
    XMLVM_CHECK_NPE(12)
    _r5.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    XMLVM_SOURCE_POSITION("String.java", 1223)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r5.i > _r8.i) goto label72;
    if (_r13.i < 0) goto label72;
    XMLVM_SOURCE_POSITION("String.java", 1224)
    if (_r5.i <= 0) goto label63;
    XMLVM_SOURCE_POSITION("String.java", 1225)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    _r8.i = _r8.i - _r5.i;
    if (_r13.i <= _r8.i) goto label20;
    XMLVM_SOURCE_POSITION("String.java", 1226)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    _r13.i = _r8.i - _r5.i;
    label20:;
    XMLVM_SOURCE_POSITION("String.java", 1229)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((java_lang_String*) _r12.o)->fields.java_lang_String.value_;
    XMLVM_SOURCE_POSITION("String.java", 1230)
    XMLVM_CHECK_NPE(12)
    _r6.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 1231)
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r6.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    _r0.i = _r6.i + _r5.i;
    label28:;
    XMLVM_SOURCE_POSITION("String.java", 1232)
    XMLVM_SOURCE_POSITION("String.java", 1234)
    XMLVM_CHECK_NPE(11)
    _r2.i = java_lang_String_lastIndexOf___int_int(_r11.o, _r1.i, _r13.i);
    if (_r2.i != _r10.i) goto label36;
    XMLVM_SOURCE_POSITION("String.java", 1235)
    _r8 = _r10;
    label35:;
    XMLVM_SOURCE_POSITION("String.java", 1236)
    XMLVM_SOURCE_POSITION("String.java", 1250)
    XMLVM_EXIT_METHOD()
    return _r8.i;
    label36:;
    XMLVM_SOURCE_POSITION("String.java", 1238)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    _r3.i = _r8.i + _r2.i;
    _r4 = _r6;
    label41:;
    XMLVM_SOURCE_POSITION("String.java", 1239)
    _r4.i = _r4.i + 1;
    if (_r4.i >= _r0.i) goto label55;
    XMLVM_CHECK_NPE(11)
    _r8.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    _r3.i = _r3.i + 1;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r3.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r4.i);
    _r9.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    if (_r8.i == _r9.i) goto label41;
    label55:;
    XMLVM_SOURCE_POSITION("String.java", 1242)
    if (_r4.i != _r0.i) goto label59;
    _r8 = _r2;
    XMLVM_SOURCE_POSITION("String.java", 1243)
    goto label35;
    label59:;
    XMLVM_SOURCE_POSITION("String.java", 1245)
    _r8.i = 1;
    _r13.i = _r2.i - _r8.i;
    XMLVM_SOURCE_POSITION("String.java", 1246)
    goto label28;
    label63:;
    XMLVM_SOURCE_POSITION("String.java", 1248)
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r13.i >= _r8.i) goto label69;
    _r8 = _r13;
    goto label35;
    label69:;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    goto label35;
    label72:;
    _r8 = _r10;
    goto label35;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_length__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_length__]
    XMLVM_ENTER_METHOD("java.lang.String", "length", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1259)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_String*) _r1.o)->fields.java_lang_String.count_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_regionMatches___int_java_lang_String_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_regionMatches___int_java_lang_String_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "regionMatches", "?")
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
    _r8.o = me;
    _r9.i = n1;
    _r10.o = n2;
    _r11.i = n3;
    _r12.i = n4;
    _r7.i = 1;
    _r6.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1281)
    XMLVM_CHECK_NPE(10)
    _r3.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i - _r11.i;
    if (_r3.i < _r12.i) goto label9;
    if (_r11.i >= 0) goto label11;
    label9:;
    _r3 = _r6;
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 1282)
    XMLVM_SOURCE_POSITION("String.java", 1296)
    XMLVM_EXIT_METHOD()
    return _r3.i;
    label11:;
    XMLVM_SOURCE_POSITION("String.java", 1284)
    if (_r9.i < 0) goto label18;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i - _r9.i;
    if (_r3.i >= _r12.i) goto label20;
    label18:;
    _r3 = _r6;
    XMLVM_SOURCE_POSITION("String.java", 1285)
    goto label10;
    label20:;
    XMLVM_SOURCE_POSITION("String.java", 1287)
    if (_r12.i > 0) goto label24;
    _r3 = _r7;
    XMLVM_SOURCE_POSITION("String.java", 1288)
    goto label10;
    label24:;
    XMLVM_SOURCE_POSITION("String.java", 1290)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.offset_;
    _r1.i = _r3.i + _r9.i;
    XMLVM_CHECK_NPE(10)
    _r3.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.offset_;
    _r2.i = _r3.i + _r11.i;
    _r0.i = 0;
    label33:;
    XMLVM_SOURCE_POSITION("String.java", 1291)
    if (_r0.i >= _r12.i) goto label54;
    XMLVM_SOURCE_POSITION("String.java", 1292)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_lang_String*) _r8.o)->fields.java_lang_String.value_;
    _r4.i = _r1.i + _r0.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(10)
    _r4.o = ((java_lang_String*) _r10.o)->fields.java_lang_String.value_;
    _r5.i = _r2.i + _r0.i;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    if (_r3.i == _r4.i) goto label51;
    _r3 = _r6;
    XMLVM_SOURCE_POSITION("String.java", 1293)
    goto label10;
    label51:;
    _r0.i = _r0.i + 1;
    goto label33;
    label54:;
    _r3 = _r7;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_regionMatches___boolean_int_java_lang_String_int_int(JAVA_OBJECT me, JAVA_BOOLEAN n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_regionMatches___boolean_int_java_lang_String_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "regionMatches", "?")
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
    _r9.o = me;
    _r10.i = n1;
    _r11.i = n2;
    _r12.o = n3;
    _r13.i = n4;
    _r14.i = n5;
    _r8.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1321)
    if (_r10.i != 0) goto label8;
    XMLVM_SOURCE_POSITION("String.java", 1322)
    XMLVM_CHECK_NPE(9)
    _r6.i = java_lang_String_regionMatches___int_java_lang_String_int_int(_r9.o, _r11.i, _r12.o, _r13.i, _r14.i);
    label7:;
    XMLVM_SOURCE_POSITION("String.java", 1346)
    XMLVM_EXIT_METHOD()
    return _r6.i;
    label8:;
    XMLVM_SOURCE_POSITION("String.java", 1325)
    if (_r12.o == JAVA_NULL) goto label80;
    XMLVM_SOURCE_POSITION("String.java", 1326)
    if (_r11.i < 0) goto label17;
    XMLVM_CHECK_NPE(9)
    _r6.i = ((java_lang_String*) _r9.o)->fields.java_lang_String.count_;
    _r6.i = _r6.i - _r11.i;
    if (_r14.i <= _r6.i) goto label19;
    label17:;
    _r6 = _r8;
    XMLVM_SOURCE_POSITION("String.java", 1327)
    goto label7;
    label19:;
    XMLVM_SOURCE_POSITION("String.java", 1329)
    if (_r13.i < 0) goto label26;
    XMLVM_CHECK_NPE(12)
    _r6.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.count_;
    _r6.i = _r6.i - _r13.i;
    if (_r14.i <= _r6.i) goto label28;
    label26:;
    _r6 = _r8;
    XMLVM_SOURCE_POSITION("String.java", 1330)
    goto label7;
    label28:;
    XMLVM_SOURCE_POSITION("String.java", 1333)
    XMLVM_CHECK_NPE(9)
    _r6.i = ((java_lang_String*) _r9.o)->fields.java_lang_String.offset_;
    _r11.i = _r11.i + _r6.i;
    XMLVM_SOURCE_POSITION("String.java", 1334)
    XMLVM_CHECK_NPE(12)
    _r6.i = ((java_lang_String*) _r12.o)->fields.java_lang_String.offset_;
    _r13.i = _r13.i + _r6.i;
    _r2.i = _r11.i + _r14.i;
    XMLVM_SOURCE_POSITION("String.java", 1337)
    XMLVM_CHECK_NPE(12)
    _r4.o = ((java_lang_String*) _r12.o)->fields.java_lang_String.value_;
    _r3 = _r13;
    _r5 = _r11;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 1338)
    if (_r5.i >= _r2.i) goto label76;
    XMLVM_SOURCE_POSITION("String.java", 1339)
    XMLVM_CHECK_NPE(9)
    _r6.o = ((java_lang_String*) _r9.o)->fields.java_lang_String.value_;
    _r11.i = _r5.i + 1;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r5.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r13.i = _r3.i + 1;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    if (_r0.i == _r1.i) goto label86;
    XMLVM_CHECK_NPE(9)
    _r6.i = java_lang_String_toUpperCase___char(_r9.o, _r0.i);
    XMLVM_CHECK_NPE(9)
    _r7.i = java_lang_String_toUpperCase___char(_r9.o, _r1.i);
    if (_r6.i == _r7.i) goto label86;
    XMLVM_CHECK_NPE(9)
    _r6.i = java_lang_String_toLowerCase___char(_r9.o, _r0.i);
    XMLVM_CHECK_NPE(9)
    _r7.i = java_lang_String_toLowerCase___char(_r9.o, _r1.i);
    if (_r6.i == _r7.i) goto label86;
    _r6 = _r8;
    XMLVM_SOURCE_POSITION("String.java", 1343)
    goto label7;
    label76:;
    _r6.i = 1;
    _r13 = _r3;
    _r11 = _r5;
    goto label7;
    label80:;
    XMLVM_SOURCE_POSITION("String.java", 1348)
    _r6.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(6)
    java_lang_NullPointerException___INIT___(_r6.o);
    XMLVM_THROW_CUSTOM(_r6.o)
    label86:;
    _r3 = _r13;
    _r5 = _r11;
    goto label40;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_replace___char_char(JAVA_OBJECT me, JAVA_CHAR n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_replace___char_char]
    XMLVM_ENTER_METHOD("java.lang.String", "replace", "?")
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
    _r8.o = me;
    _r9.i = n1;
    _r10.i = n2;
    _r7.i = -1;
    _r6.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1362)
    XMLVM_CHECK_NPE(8)
    _r1.i = java_lang_String_indexOf___int_int(_r8.o, _r9.i, _r6.i);
    if (_r1.i != _r7.i) goto label10;
    XMLVM_SOURCE_POSITION("String.java", 1363)
    _r3 = _r8;
    label9:;
    XMLVM_SOURCE_POSITION("String.java", 1364)
    XMLVM_SOURCE_POSITION("String.java", 1372)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 1367)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r3.i);
    XMLVM_SOURCE_POSITION("String.java", 1368)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_lang_String*) _r8.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(8)
    _r4.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(8)
    _r5.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r4.i, _r0.o, _r6.i, _r5.i);
    label23:;
    XMLVM_SOURCE_POSITION("String.java", 1370)
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r10.i;
    XMLVM_SOURCE_POSITION("String.java", 1371)
    XMLVM_CHECK_NPE(8)
    _r1.i = java_lang_String_indexOf___int_int(_r8.o, _r9.i, _r2.i);
    if (_r1.i != _r7.i) goto label23;
    _r3.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(8)
    _r4.i = ((java_lang_String*) _r8.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(3)
    java_lang_String___INIT____int_int_char_1ARRAY(_r3.o, _r6.i, _r4.i, _r0.o);
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_replace___java_lang_CharSequence_java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_replace___java_lang_CharSequence_java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.String", "replace", "?")
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
    _r11.o = me;
    _r12.o = n1;
    _r13.o = n2;
    _r10.i = -1;
    XMLVM_SOURCE_POSITION("String.java", 1389)
    if (_r12.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("String.java", 1390)
    _r7.o = __NEW_java_lang_NullPointerException();
    // "target should not be null"
    _r8.o = xmlvm_create_java_string_from_pool(176);
    XMLVM_CHECK_NPE(7)
    java_lang_NullPointerException___INIT____java_lang_String(_r7.o, _r8.o);
    XMLVM_THROW_CUSTOM(_r7.o)
    label11:;
    XMLVM_SOURCE_POSITION("String.java", 1392)
    if (_r13.o != JAVA_NULL) goto label21;
    XMLVM_SOURCE_POSITION("String.java", 1393)
    _r7.o = __NEW_java_lang_NullPointerException();
    // "replacement should not be null"
    _r8.o = xmlvm_create_java_string_from_pool(177);
    XMLVM_CHECK_NPE(7)
    java_lang_NullPointerException___INIT____java_lang_String(_r7.o, _r8.o);
    XMLVM_THROW_CUSTOM(_r7.o)
    label21:;
    XMLVM_SOURCE_POSITION("String.java", 1395)
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(12)
    _r6.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r12.o)->tib->vtable[5])(_r12.o);
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1396)
    XMLVM_CHECK_NPE(11)
    _r2.i = java_lang_String_indexOf___java_lang_String_int(_r11.o, _r6.o, _r7.i);
    if (_r2.i != _r10.i) goto label34;
    XMLVM_SOURCE_POSITION("String.java", 1398)
    _r7 = _r11;
    label33:;
    XMLVM_SOURCE_POSITION("String.java", 1399)
    XMLVM_SOURCE_POSITION("String.java", 1426)
    XMLVM_EXIT_METHOD()
    return _r7.o;
    label34:;
    XMLVM_SOURCE_POSITION("String.java", 1401)
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(13)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r13.o)->tib->vtable[5])(_r13.o);
    XMLVM_SOURCE_POSITION("String.java", 1405)
    // ""
    _r7.o = xmlvm_create_java_string_from_pool(21);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r7.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r6.o);
    if (_r7.i == 0) goto label92;
    XMLVM_SOURCE_POSITION("String.java", 1406)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(11)
    _r7.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r8.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    XMLVM_CHECK_NPE(11)
    _r9.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    _r9.i = _r9.i + 1;
    _r8.i = _r8.i * _r9.i;
    _r7.i = _r7.i + _r8.i;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____int(_r0.o, _r7.i);
    XMLVM_SOURCE_POSITION("String.java", 1407)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    XMLVM_SOURCE_POSITION("String.java", 1408)
    _r1.i = 0;
    label67:;
    XMLVM_CHECK_NPE(11)
    _r7.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    if (_r1.i >= _r7.i) goto label87;
    XMLVM_SOURCE_POSITION("String.java", 1409)
    XMLVM_CHECK_NPE(11)
    _r7.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    _r8.i = _r8.i + _r1.i;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r8.i);
    _r7.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i];
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r7.i);
    XMLVM_SOURCE_POSITION("String.java", 1410)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    _r1.i = _r1.i + 1;
    goto label67;
    label87:;
    XMLVM_SOURCE_POSITION("String.java", 1412)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r7.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label33;
    label92:;
    XMLVM_SOURCE_POSITION("String.java", 1415)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(11)
    _r7.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r8.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    _r7.i = _r7.i + _r8.i;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____int(_r0.o, _r7.i);
    XMLVM_SOURCE_POSITION("String.java", 1416)
    XMLVM_CHECK_NPE(12)
    _r5.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r12.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r12.o);
    _r4.i = 0;
    label109:;
    XMLVM_SOURCE_POSITION("String.java", 1417)
    XMLVM_SOURCE_POSITION("String.java", 1419)
    XMLVM_CHECK_NPE(11)
    _r7.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    _r8.i = _r8.i + _r4.i;
    _r9.i = _r2.i - _r4.i;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___char_1ARRAY_int_int(_r0.o, _r7.o, _r8.i, _r9.i);
    XMLVM_SOURCE_POSITION("String.java", 1420)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    XMLVM_SOURCE_POSITION("String.java", 1421)
    _r4.i = _r2.i + _r5.i;
    XMLVM_SOURCE_POSITION("String.java", 1422)
    XMLVM_CHECK_NPE(11)
    _r2.i = java_lang_String_indexOf___java_lang_String_int(_r11.o, _r6.o, _r4.i);
    if (_r2.i != _r10.i) goto label109;
    XMLVM_SOURCE_POSITION("String.java", 1424)
    XMLVM_CHECK_NPE(11)
    _r7.o = ((java_lang_String*) _r11.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(11)
    _r8.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.offset_;
    _r8.i = _r8.i + _r4.i;
    XMLVM_CHECK_NPE(11)
    _r9.i = ((java_lang_String*) _r11.o)->fields.java_lang_String.count_;
    _r9.i = _r9.i - _r4.i;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___char_1ARRAY_int_int(_r0.o, _r7.o, _r8.i, _r9.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r7.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_startsWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_startsWith___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "startsWith", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1441)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_startsWith___java_lang_String_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_startsWith___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_startsWith___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.String", "startsWith", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 1458)
    _r0.i = 0;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    XMLVM_CHECK_NPE(2)
    _r0.i = java_lang_String_regionMatches___int_java_lang_String_int_int(_r2.o, _r4.i, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_substring___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_substring___int]
    XMLVM_ENTER_METHOD("java.lang.String", "substring", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1472)
    if (_r5.i != 0) goto label4;
    _r0 = _r4;
    label3:;
    XMLVM_SOURCE_POSITION("String.java", 1473)
    XMLVM_SOURCE_POSITION("String.java", 1476)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label4:;
    XMLVM_SOURCE_POSITION("String.java", 1475)
    if (_r5.i < 0) goto label24;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r5.i > _r0.i) goto label24;
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r1.i = _r1.i + _r5.i;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    _r2.i = _r2.i - _r5.i;
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r1.i, _r2.i, _r3.o);
    goto label3;
    label24:;
    XMLVM_SOURCE_POSITION("String.java", 1478)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_substring___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_substring___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "substring", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 1494)
    if (_r5.i != 0) goto label8;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r6.i != _r0.i) goto label8;
    _r0 = _r4;
    label7:;
    XMLVM_SOURCE_POSITION("String.java", 1495)
    XMLVM_SOURCE_POSITION("String.java", 1505)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label8:;
    XMLVM_SOURCE_POSITION("String.java", 1497)
    if (_r5.i >= 0) goto label16;
    XMLVM_SOURCE_POSITION("String.java", 1498)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("String.java", 1499)
    if (_r5.i <= _r6.i) goto label26;
    XMLVM_SOURCE_POSITION("String.java", 1500)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.i = _r6.i - _r5.i;

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label26:;
    XMLVM_SOURCE_POSITION("String.java", 1501)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r6.i <= _r0.i) goto label36;
    XMLVM_SOURCE_POSITION("String.java", 1502)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label36:;
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r1.i = _r1.i + _r5.i;
    _r2.i = _r6.i - _r5.i;
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r1.i, _r2.i, _r3.o);
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_toCharArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toCharArray__]
    XMLVM_ENTER_METHOD("java.lang.String", "toCharArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1514)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r1.i);
    XMLVM_SOURCE_POSITION("String.java", 1515)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    _r3.i = 0;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("String.java", 1516)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_toLowerCase__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toLowerCase__]
    XMLVM_ENTER_METHOD("java.lang.String", "toLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1527)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_toLowerCase___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_toLowerCase___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toLowerCase___java_util_Locale]
    XMLVM_ENTER_METHOD("java.lang.String", "toLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1541)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("String.java", 1542)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    label7:;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i + _r4.i;
    if (_r1.i >= _r3.i) goto label29;
    XMLVM_SOURCE_POSITION("String.java", 1543)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(5)
    _r3.i = java_lang_String_toLowerCaseImpl___int(_r5.o, _r3.i);
    _r3.i = _r3.i & 0xffff;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r3.i);
    _r1.i = _r1.i + 1;
    goto label7;
    label29:;
    XMLVM_SOURCE_POSITION("String.java", 1546)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_SOURCE_POSITION("String.java", 1547)
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r3.i == 0) goto label41;
    _r3 = _r5;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 1548)
    XMLVM_SOURCE_POSITION("String.java", 1550)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label41:;
    _r3 = _r2;
    goto label40;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_String_toLowerCaseImpl___int(JAVA_OBJECT me, JAVA_INT n1)]

JAVA_OBJECT java_lang_String_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toString__]
    XMLVM_ENTER_METHOD("java.lang.String", "toString", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1563)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_toUpperCase__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toUpperCase__]
    XMLVM_ENTER_METHOD("java.lang.String", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("String.java", 1574)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_toUpperCase___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_toUpperCase___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_toUpperCase___java_util_Locale]
    XMLVM_ENTER_METHOD("java.lang.String", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1588)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("String.java", 1589)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    label7:;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i + _r4.i;
    if (_r1.i >= _r3.i) goto label29;
    XMLVM_SOURCE_POSITION("String.java", 1590)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(5)
    _r3.i = java_lang_String_toUpperCaseImpl___int(_r5.o, _r3.i);
    _r3.i = _r3.i & 0xffff;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r3.i);
    _r1.i = _r1.i + 1;
    goto label7;
    label29:;
    XMLVM_SOURCE_POSITION("String.java", 1593)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_SOURCE_POSITION("String.java", 1594)
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r3.i == 0) goto label41;
    _r3 = _r5;
    label40:;
    XMLVM_SOURCE_POSITION("String.java", 1595)
    XMLVM_SOURCE_POSITION("String.java", 1597)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label41:;
    _r3 = _r2;
    goto label40;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_String_toUpperCaseImpl___int(JAVA_OBJECT me, JAVA_INT n1)]

JAVA_OBJECT java_lang_String_trim__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_trim__]
    XMLVM_ENTER_METHOD("java.lang.String", "trim", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    _r5.i = 32;
    XMLVM_SOURCE_POSITION("String.java", 1611)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.count_;
    _r3.i = _r3.i + _r4.i;
    _r4.i = 1;
    _r1.i = _r3.i - _r4.i;
    _r0 = _r1;
    label13:;
    XMLVM_SOURCE_POSITION("String.java", 1612)
    XMLVM_SOURCE_POSITION("String.java", 1613)
    if (_r2.i > _r0.i) goto label24;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r3.i > _r5.i) goto label24;
    XMLVM_SOURCE_POSITION("String.java", 1614)
    _r2.i = _r2.i + 1;
    goto label13;
    label24:;
    XMLVM_SOURCE_POSITION("String.java", 1616)
    if (_r0.i < _r2.i) goto label35;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r3.i > _r5.i) goto label35;
    XMLVM_SOURCE_POSITION("String.java", 1617)
    _r0.i = _r0.i + -1;
    goto label24;
    label35:;
    XMLVM_SOURCE_POSITION("String.java", 1619)
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_String*) _r6.o)->fields.java_lang_String.offset_;
    if (_r2.i != _r3.i) goto label43;
    if (_r0.i != _r1.i) goto label43;
    _r3 = _r6;
    label42:;
    XMLVM_SOURCE_POSITION("String.java", 1620)
    XMLVM_SOURCE_POSITION("String.java", 1622)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label43:;
    _r3.o = __NEW_java_lang_String();
    _r4.i = _r0.i - _r2.i;
    _r4.i = _r4.i + 1;
    XMLVM_CHECK_NPE(6)
    _r5.o = ((java_lang_String*) _r6.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(3)
    java_lang_String___INIT____int_int_char_1ARRAY(_r3.o, _r2.i, _r4.i, _r5.o);
    goto label42;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___char_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1637)
    _r0.o = __NEW_java_lang_String();
    _r1.i = 0;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r3.o, _r1.i, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("String.java", 1659)
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r1.o, _r2.i, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___char]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("String.java", 1671)
    _r1.i = 128;
    if (_r4.i >= _r1.i) goto label16;
    XMLVM_SOURCE_POSITION("String.java", 1672)
    _r0.o = __NEW_java_lang_String();
    _r1.o = java_lang_String_GET_ascii();
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r4.i, _r2.i, _r1.o);
    label13:;
    XMLVM_SOURCE_POSITION("String.java", 1676)
    XMLVM_CHECK_NPE(0)
    ((java_lang_String*) _r0.o)->fields.java_lang_String.hashCode_ = _r4.i;
    XMLVM_SOURCE_POSITION("String.java", 1677)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    XMLVM_SOURCE_POSITION("String.java", 1674)
    _r0.o = __NEW_java_lang_String();
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.i;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r3.i, _r2.i, _r1.o);
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___double(JAVA_DOUBLE n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___double]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.d = n1;
    XMLVM_SOURCE_POSITION("String.java", 1688)
    _r0.o = java_lang_Double_toString___double(_r1.d);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___float(JAVA_FLOAT n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___float]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.f = n1;
    XMLVM_SOURCE_POSITION("String.java", 1699)
    _r0.o = java_lang_Float_toString___float(_r1.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___int]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1710)
    _r0.o = java_lang_Integer_toString___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___long(JAVA_LONG n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___long]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.l = n1;
    XMLVM_SOURCE_POSITION("String.java", 1721)
    _r0.o = java_lang_Long_toString___long(_r1.l);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___java_lang_Object(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1734)
    if (_r1.o == JAVA_NULL) goto label7;
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[5])(_r1.o);
    label6:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_valueOf___boolean(JAVA_BOOLEAN n1)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_valueOf___boolean]
    XMLVM_ENTER_METHOD("java.lang.String", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1747)
    if (_r1.i == 0) goto label5;
    // "true"
    _r0.o = xmlvm_create_java_string_from_pool(4);
    label4:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label5:;
    // "false"
    _r0.o = xmlvm_create_java_string_from_pool(178);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_contentEquals___java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_contentEquals___java_lang_StringBuffer]
    XMLVM_ENTER_METHOD("java.lang.String", "contentEquals", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1764)
    java_lang_Object_acquireLockRecursive__(_r6.o);
    XMLVM_TRY_BEGIN(w3113aaac87b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 1765)

    
    // Red class access removed: java.lang.StringBuffer::length
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("String.java", 1766)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    if (_r1.i == _r0.i) { XMLVM_MEMCPY(curThread_w3113aaac87b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3113aaac87b1b6, sizeof(XMLVM_JMP_BUF)); goto label13; };
    XMLVM_SOURCE_POSITION("String.java", 1767)
    java_lang_Object_releaseLockRecursive__(_r6.o);
    _r1 = _r2;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac87b1b6)
        XMLVM_CATCH_SPECIFIC(w3113aaac87b1b6,java_lang_Object,31)
    XMLVM_CATCH_END(w3113aaac87b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac87b1b6)
    label12:;
    XMLVM_TRY_BEGIN(w3113aaac87b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("String.java", 1769)
    XMLVM_MEMCPY(curThread_w3113aaac87b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3113aaac87b1b8, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac87b1b8)
        XMLVM_CATCH_SPECIFIC(w3113aaac87b1b8,java_lang_Object,31)
    XMLVM_CATCH_END(w3113aaac87b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac87b1b8)
    label13:;
    XMLVM_TRY_BEGIN(w3113aaac87b1c10)
    // Begin try
    _r1.i = 0;
    _r2.o = __NEW_java_lang_String();
    _r3.i = 0;

    
    // Red class access removed: java.lang.StringBuffer::getValue
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_lang_String___INIT____int_int_char_1ARRAY(_r2.o, _r3.i, _r0.i, _r4.o);
    _r3.i = 0;
    XMLVM_CHECK_NPE(5)
    _r1.i = java_lang_String_regionMatches___int_java_lang_String_int_int(_r5.o, _r1.i, _r2.o, _r3.i, _r0.i);
    java_lang_Object_releaseLockRecursive__(_r6.o);
    { XMLVM_MEMCPY(curThread_w3113aaac87b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3113aaac87b1c10, sizeof(XMLVM_JMP_BUF)); goto label12; };
    XMLVM_SOURCE_POSITION("String.java", 1771)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac87b1c10)
        XMLVM_CATCH_SPECIFIC(w3113aaac87b1c10,java_lang_Object,31)
    XMLVM_CATCH_END(w3113aaac87b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac87b1c10)
    label31:;
    XMLVM_TRY_BEGIN(w3113aaac87b1c12)
    // Begin try
    java_lang_Thread* curThread_w3113aaac87b1c12aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3113aaac87b1c12aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3113aaac87b1c12)
        XMLVM_CATCH_SPECIFIC(w3113aaac87b1c12,java_lang_Object,31)
    XMLVM_CATCH_END(w3113aaac87b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3113aaac87b1c12)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_contentEquals___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_contentEquals___java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.String", "contentEquals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("String.java", 1784)
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r4.o);
    XMLVM_SOURCE_POSITION("String.java", 1786)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    if (_r0.i == _r1.i) goto label11;
    _r1 = _r2;
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 1787)
    XMLVM_SOURCE_POSITION("String.java", 1794)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label11:;
    XMLVM_SOURCE_POSITION("String.java", 1790)
    if (_r0.i != 0) goto label19;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    if (_r1.i != 0) goto label19;
    XMLVM_SOURCE_POSITION("String.java", 1791)
    _r1.i = 1;
    goto label10;
    label19:;
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[5])(_r4.o);
    XMLVM_CHECK_NPE(3)
    _r1.i = java_lang_String_regionMatches___int_java_lang_String_int_int(_r3.o, _r2.i, _r1.o, _r2.i, _r0.i);
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_matches___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_matches___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "matches", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1811)

    
    // Red class access removed: java.util.regex.Pattern::matches
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_replaceAll___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_replaceAll___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "replaceAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("String.java", 1830)

    
    // Red class access removed: java.util.regex.Pattern::compile
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Pattern::matcher
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Matcher::replaceAll
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_replaceFirst___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_replaceFirst___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "replaceFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("String.java", 1851)

    
    // Red class access removed: java.util.regex.Pattern::compile
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Pattern::matcher
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Matcher::replaceFirst
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_split___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_split___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.String", "split", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1870)

    
    // Red class access removed: java.util.regex.Pattern::compile
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Pattern::split
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_split___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_split___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.String", "split", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 1893)

    
    // Red class access removed: java.util.regex.Pattern::compile
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.regex.Pattern::split
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_subSequence___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "subSequence", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 1912)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int_int(_r1.o, _r2.i, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_codePointAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_codePointAt___int]
    XMLVM_ENTER_METHOD("java.lang.String", "codePointAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1929)
    if (_r5.i < 0) goto label6;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    if (_r5.i < _r1.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("String.java", 1930)
    _r1.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(1)
    java_lang_IndexOutOfBoundsException___INIT___(_r1.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    label12:;
    XMLVM_SOURCE_POSITION("String.java", 1932)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    _r0.i = _r5.i + _r1.i;
    XMLVM_SOURCE_POSITION("String.java", 1933)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_String*) _r4.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_String*) _r4.o)->fields.java_lang_String.count_;
    _r2.i = _r2.i + _r3.i;
    _r1.i = java_lang_Character_codePointAt___char_1ARRAY_int_int(_r1.o, _r0.i, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_codePointBefore___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_codePointBefore___int]
    XMLVM_ENTER_METHOD("java.lang.String", "codePointBefore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("String.java", 1950)
    _r1.i = 1;
    if (_r3.i < _r1.i) goto label7;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.count_;
    if (_r3.i <= _r1.i) goto label13;
    label7:;
    XMLVM_SOURCE_POSITION("String.java", 1951)
    _r1.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(1)
    java_lang_IndexOutOfBoundsException___INIT___(_r1.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    label13:;
    XMLVM_SOURCE_POSITION("String.java", 1953)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_lang_String*) _r2.o)->fields.java_lang_String.offset_;
    _r0.i = _r3.i + _r1.i;
    XMLVM_SOURCE_POSITION("String.java", 1954)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_String*) _r2.o)->fields.java_lang_String.value_;
    _r1.i = java_lang_Character_codePointBefore___char_1ARRAY_int(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_codePointCount___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_codePointCount___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "codePointCount", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.i = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 1974)
    if (_r4.i < 0) goto label8;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.count_;
    if (_r5.i > _r1.i) goto label8;
    if (_r4.i <= _r5.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("String.java", 1975)
    _r1.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(1)
    java_lang_IndexOutOfBoundsException___INIT___(_r1.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    label14:;
    XMLVM_SOURCE_POSITION("String.java", 1977)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_String*) _r3.o)->fields.java_lang_String.offset_;
    _r0.i = _r4.i + _r1.i;
    XMLVM_SOURCE_POSITION("String.java", 1978)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_String*) _r3.o)->fields.java_lang_String.value_;
    _r2.i = _r5.i - _r4.i;
    _r1.i = java_lang_Character_codePointCount___char_1ARRAY_int_int(_r1.o, _r0.i, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_contains___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_contains___java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.lang.String", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 1992)
    //java_lang_Object_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[5])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_indexOf___java_lang_String(_r1.o, _r0.o);
    if (_r0.i < 0) goto label12;
    _r0.i = 1;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    _r0.i = 0;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_offsetByCodePoints___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_offsetByCodePoints___int_int]
    XMLVM_ENTER_METHOD("java.lang.String", "offsetByCodePoints", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.i = n1;
    _r7.i = n2;
    XMLVM_SOURCE_POSITION("String.java", 2011)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    _r1.i = _r6.i + _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 2012)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_String*) _r5.o)->fields.java_lang_String.value_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.count_;
    _r0.i = java_lang_Character_offsetByCodePoints___char_1ARRAY_int_int_int_int(_r2.o, _r3.i, _r4.i, _r1.i, _r7.i);
    XMLVM_SOURCE_POSITION("String.java", 2014)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_String*) _r5.o)->fields.java_lang_String.offset_;
    _r2.i = _r0.i - _r2.i;
    XMLVM_EXIT_METHOD()
    return _r2.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_format___java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_format___java_lang_String_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "format", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("String.java", 2034)
    _r0.o = java_util_Locale_getDefault__();
    _r0.o = java_lang_String_format___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY(_r0.o, _r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_format___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_format___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.String", "format", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = n1;
    _r5.o = n2;
    _r6.o = n3;
    XMLVM_SOURCE_POSITION("String.java", 2063)
    if (_r5.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("String.java", 2064)
    _r2.o = __NEW_java_lang_NullPointerException();
    // "null format argument"
    _r3.o = xmlvm_create_java_string_from_pool(179);
    XMLVM_CHECK_NPE(2)
    java_lang_NullPointerException___INIT____java_lang_String(_r2.o, _r3.o);
    XMLVM_THROW_CUSTOM(_r2.o)
    label10:;
    XMLVM_SOURCE_POSITION("String.java", 2066)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[8])(_r5.o);
    if (_r6.o != JAVA_NULL) goto label38;
    _r3.i = 0;
    label17:;
    _r0.i = _r2.i + _r3.i;
    XMLVM_SOURCE_POSITION("String.java", 2068)

    
    // Red class access removed: java.util.Formatter::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r2.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r0.i);

    
    // Red class access removed: java.util.Formatter::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("String.java", 2069)

    
    // Red class access removed: java.util.Formatter::format
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Formatter::toString
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label38:;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r3.i = _r3.i * 10;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_indexOf___java_lang_String_java_lang_String_int_int_char(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_CHAR n5)
{
    XMLVM_CLASS_INIT(java_lang_String)
    //XMLVM_BEGIN_WRAPPER[java_lang_String_indexOf___java_lang_String_java_lang_String_int_int_char]
    XMLVM_ENTER_METHOD("java.lang.String", "indexOf", "?")
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
    _r9.o = n1;
    _r10.o = n2;
    _r11.i = n3;
    _r12.i = n4;
    _r13.i = n5;
    XMLVM_SOURCE_POSITION("String.java", 2084)
    XMLVM_CHECK_NPE(9)
    _r0.o = ((java_lang_String*) _r9.o)->fields.java_lang_String.value_;
    XMLVM_SOURCE_POSITION("String.java", 2085)
    XMLVM_CHECK_NPE(9)
    _r1.i = ((java_lang_String*) _r9.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 2086)
    XMLVM_CHECK_NPE(9)
    _r9.i = ((java_lang_String*) _r9.o)->fields.java_lang_String.count_;
    XMLVM_SOURCE_POSITION("String.java", 2087)
    XMLVM_CHECK_NPE(10)
    _r3.o = ((java_lang_String*) _r10.o)->fields.java_lang_String.value_;
    XMLVM_SOURCE_POSITION("String.java", 2088)
    XMLVM_CHECK_NPE(10)
    _r5.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.offset_;
    XMLVM_SOURCE_POSITION("String.java", 2089)
    XMLVM_CHECK_NPE(10)
    _r10.i = ((java_lang_String*) _r10.o)->fields.java_lang_String.count_;
    _r2.i = 1;
    _r4.i = _r10.i - _r2.i;
    _r9.i = _r9.i + _r1.i;
    _r10.i = _r1.i + _r4.i;
    label18:;
    XMLVM_SOURCE_POSITION("String.java", 2092)
    if (_r10.i >= _r9.i) goto label71;
    XMLVM_SOURCE_POSITION("String.java", 2093)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r10.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r10.i];
    if (_r13.i != _r2.i) goto label60;
    XMLVM_SOURCE_POSITION("String.java", 2094)
    _r2.i = 0;
    label25:;
    if (_r2.i >= _r4.i) goto label56;
    XMLVM_SOURCE_POSITION("String.java", 2095)
    _r6.i = _r2.i + _r5.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    _r6.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    _r7.i = _r10.i + _r2.i;
    _r7.i = _r7.i - _r4.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r7.i);
    _r7.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    if (_r6.i == _r7.i) goto label53;
    XMLVM_SOURCE_POSITION("String.java", 2097)
    _r6.i = 1;
    _r7.i = 1;
    XMLVM_SOURCE_POSITION("String.java", 2098)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r10.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r10.i];
    _r7.i = _r7.i << _r8.i;
    _r7.i = _r7.i & _r11.i;
    if (_r7.i != 0) goto label73;
    XMLVM_SOURCE_POSITION("String.java", 2099)
    _r2.i = _r2.i + _r6.i;
    label47:;
    XMLVM_SOURCE_POSITION("String.java", 2101)
    _r2.i = java_lang_Math_max___int_int(_r12.i, _r2.i);
    _r10.i = _r10.i + _r2.i;
    XMLVM_SOURCE_POSITION("String.java", 2102)
    goto label18;
    label53:;
    _r2.i = _r2.i + 1;
    goto label25;
    label56:;
    XMLVM_SOURCE_POSITION("String.java", 2105)
    _r9.i = _r10.i - _r4.i;
    _r9.i = _r9.i - _r1.i;
    label59:;
    XMLVM_SOURCE_POSITION("String.java", 2113)
    XMLVM_EXIT_METHOD()
    return _r9.i;
    label60:;
    XMLVM_SOURCE_POSITION("String.java", 2108)
    _r2.i = 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r10.i);
    _r6.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r10.i];
    _r2.i = _r2.i << _r6.i;
    _r2.i = _r2.i & _r11.i;
    if (_r2.i != 0) goto label68;
    XMLVM_SOURCE_POSITION("String.java", 2109)
    _r10.i = _r10.i + _r4.i;
    label68:;
    XMLVM_SOURCE_POSITION("String.java", 2111)
    _r10.i = _r10.i + 1;
    goto label18;
    label71:;
    _r9.i = -1;
    goto label59;
    label73:;
    _r2 = _r6;
    goto label47;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_String_getValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_getValue__]
    XMLVM_ENTER_METHOD("java.lang.String", "getValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("String.java", 2120)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_String*) _r1.o)->fields.java_lang_String.value_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_String_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_isEmpty__]
    XMLVM_ENTER_METHOD("java.lang.String", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("String.java", 2124)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r0.i != 0) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_String_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.String", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("String.java", 53)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_String_compareTo___java_lang_String(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_String___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_String___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.String", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("String.java", 140)
    _r1.o = __NEW_java_lang_String_CaseInsensitiveComparator();
    _r2.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    java_lang_String_CaseInsensitiveComparator___INIT____java_lang_String_1(_r1.o, _r2.o);
    java_lang_String_PUT_CASE_INSENSITIVE_ORDER( _r1.o);
    XMLVM_SOURCE_POSITION("String.java", 157)
    _r1.i = 128;
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r1.i);
    java_lang_String_PUT_ascii( _r1.o);
    XMLVM_SOURCE_POSITION("String.java", 158)
    _r0.i = 0;
    label15:;
    _r1.o = java_lang_String_GET_ascii();
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i >= _r1.i) goto label28;
    XMLVM_SOURCE_POSITION("String.java", 159)
    _r1.o = java_lang_String_GET_ascii();
    _r2.i = _r0.i & 0xffff;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    _r0.i = _r0.i + 1;
    goto label15;
    label28:;
    XMLVM_SOURCE_POSITION("String.java", 161)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

