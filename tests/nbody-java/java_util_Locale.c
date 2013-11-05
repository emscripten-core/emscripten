#include "xmlvm.h"
#include "java_io_ObjectStreamField.h"
#include "java_lang_Class.h"
#include "java_lang_Integer.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_Throwable.h"
#include "org_xmlvm_runtime_XMLVMUtil.h"

#include "java_util_Locale.h"

#define XMLVM_CURRENT_CLASS_NAME Locale
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Locale

__TIB_DEFINITION_java_util_Locale __TIB_java_util_Locale = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Locale, // classInitializer
    "java.util.Locale", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_Locale), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Locale;
JAVA_OBJECT __CLASS_java_util_Locale_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Locale_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Locale_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Locale_serialVersionUID;
static JAVA_OBJECT _STATIC_java_util_Locale_defaultLocale;
static JAVA_OBJECT _STATIC_java_util_Locale_CANADA;
static JAVA_OBJECT _STATIC_java_util_Locale_CANADA_FRENCH;
static JAVA_OBJECT _STATIC_java_util_Locale_CHINA;
static JAVA_OBJECT _STATIC_java_util_Locale_CHINESE;
static JAVA_OBJECT _STATIC_java_util_Locale_ENGLISH;
static JAVA_OBJECT _STATIC_java_util_Locale_FRANCE;
static JAVA_OBJECT _STATIC_java_util_Locale_FRENCH;
static JAVA_OBJECT _STATIC_java_util_Locale_GERMAN;
static JAVA_OBJECT _STATIC_java_util_Locale_GERMANY;
static JAVA_OBJECT _STATIC_java_util_Locale_ITALIAN;
static JAVA_OBJECT _STATIC_java_util_Locale_ITALY;
static JAVA_OBJECT _STATIC_java_util_Locale_JAPAN;
static JAVA_OBJECT _STATIC_java_util_Locale_JAPANESE;
static JAVA_OBJECT _STATIC_java_util_Locale_KOREA;
static JAVA_OBJECT _STATIC_java_util_Locale_KOREAN;
static JAVA_OBJECT _STATIC_java_util_Locale_PRC;
static JAVA_OBJECT _STATIC_java_util_Locale_SIMPLIFIED_CHINESE;
static JAVA_OBJECT _STATIC_java_util_Locale_TAIWAN;
static JAVA_OBJECT _STATIC_java_util_Locale_TRADITIONAL_CHINESE;
static JAVA_OBJECT _STATIC_java_util_Locale_UK;
static JAVA_OBJECT _STATIC_java_util_Locale_US;
static JAVA_OBJECT _STATIC_java_util_Locale_ROOT;
static JAVA_OBJECT _STATIC_java_util_Locale_serialPersistentFields;

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

void __INIT_java_util_Locale()
{
    staticInitializerLock(&__TIB_java_util_Locale);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Locale.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Locale.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Locale);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Locale.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Locale.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Locale.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Locale")
        __INIT_IMPL_java_util_Locale();
    }
}

void __INIT_IMPL_java_util_Locale()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_Locale.newInstanceFunc = __NEW_INSTANCE_java_util_Locale;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Locale.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Locale.vtable[0] = (VTABLE_PTR) &java_util_Locale_clone__;
    __TIB_java_util_Locale.vtable[1] = (VTABLE_PTR) &java_util_Locale_equals___java_lang_Object;
    __TIB_java_util_Locale.vtable[4] = (VTABLE_PTR) &java_util_Locale_hashCode__;
    __TIB_java_util_Locale.vtable[5] = (VTABLE_PTR) &java_util_Locale_toString__;
    // Initialize interface information
    __TIB_java_util_Locale.numImplementedInterfaces = 2;
    __TIB_java_util_Locale.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Locale.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_Locale.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;
    // Initialize itable for this class
    __TIB_java_util_Locale.itableBegin = &__TIB_java_util_Locale.itable[0];

    _STATIC_java_util_Locale_serialVersionUID = 9149081749638150636;
    _STATIC_java_util_Locale_defaultLocale = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_CANADA = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_CANADA_FRENCH = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_CHINA = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_CHINESE = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_ENGLISH = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_FRANCE = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_FRENCH = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_GERMAN = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_GERMANY = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_ITALIAN = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_ITALY = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_JAPAN = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_JAPANESE = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_KOREA = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_KOREAN = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_PRC = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_SIMPLIFIED_CHINESE = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_TAIWAN = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_TRADITIONAL_CHINESE = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_UK = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_US = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_ROOT = (java_util_Locale*) JAVA_NULL;
    _STATIC_java_util_Locale_serialPersistentFields = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_util_Locale.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Locale.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Locale.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Locale.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Locale.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Locale.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Locale.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Locale.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Locale = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Locale);
    __TIB_java_util_Locale.clazz = __CLASS_java_util_Locale;
    __TIB_java_util_Locale.baseType = JAVA_NULL;
    __CLASS_java_util_Locale_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Locale);
    __CLASS_java_util_Locale_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Locale_1ARRAY);
    __CLASS_java_util_Locale_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Locale_2ARRAY);
    java_util_Locale___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Locale]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Locale.classInitialized = 1;
}

void __DELETE_java_util_Locale(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Locale]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Locale(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_Locale*) me)->fields.java_util_Locale.countryCode_ = (java_lang_String*) JAVA_NULL;
    ((java_util_Locale*) me)->fields.java_util_Locale.languageCode_ = (java_lang_String*) JAVA_NULL;
    ((java_util_Locale*) me)->fields.java_util_Locale.variantCode_ = (java_lang_String*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Locale]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Locale()
{    XMLVM_CLASS_INIT(java_util_Locale)
java_util_Locale* me = (java_util_Locale*) XMLVM_MALLOC(sizeof(java_util_Locale));
    me->tib = &__TIB_java_util_Locale;
    __INIT_INSTANCE_MEMBERS_java_util_Locale(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Locale]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Locale()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_util_Locale_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_serialVersionUID;
}

void java_util_Locale_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_serialVersionUID = v;
}

JAVA_OBJECT java_util_Locale_GET_defaultLocale()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_defaultLocale;
}

void java_util_Locale_PUT_defaultLocale(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_defaultLocale = v;
}

JAVA_OBJECT java_util_Locale_GET_CANADA()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_CANADA;
}

void java_util_Locale_PUT_CANADA(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_CANADA = v;
}

JAVA_OBJECT java_util_Locale_GET_CANADA_FRENCH()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_CANADA_FRENCH;
}

void java_util_Locale_PUT_CANADA_FRENCH(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_CANADA_FRENCH = v;
}

JAVA_OBJECT java_util_Locale_GET_CHINA()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_CHINA;
}

void java_util_Locale_PUT_CHINA(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_CHINA = v;
}

JAVA_OBJECT java_util_Locale_GET_CHINESE()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_CHINESE;
}

void java_util_Locale_PUT_CHINESE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_CHINESE = v;
}

JAVA_OBJECT java_util_Locale_GET_ENGLISH()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_ENGLISH;
}

void java_util_Locale_PUT_ENGLISH(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_ENGLISH = v;
}

JAVA_OBJECT java_util_Locale_GET_FRANCE()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_FRANCE;
}

void java_util_Locale_PUT_FRANCE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_FRANCE = v;
}

JAVA_OBJECT java_util_Locale_GET_FRENCH()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_FRENCH;
}

void java_util_Locale_PUT_FRENCH(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_FRENCH = v;
}

JAVA_OBJECT java_util_Locale_GET_GERMAN()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_GERMAN;
}

void java_util_Locale_PUT_GERMAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_GERMAN = v;
}

JAVA_OBJECT java_util_Locale_GET_GERMANY()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_GERMANY;
}

void java_util_Locale_PUT_GERMANY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_GERMANY = v;
}

JAVA_OBJECT java_util_Locale_GET_ITALIAN()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_ITALIAN;
}

void java_util_Locale_PUT_ITALIAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_ITALIAN = v;
}

JAVA_OBJECT java_util_Locale_GET_ITALY()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_ITALY;
}

void java_util_Locale_PUT_ITALY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_ITALY = v;
}

JAVA_OBJECT java_util_Locale_GET_JAPAN()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_JAPAN;
}

void java_util_Locale_PUT_JAPAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_JAPAN = v;
}

JAVA_OBJECT java_util_Locale_GET_JAPANESE()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_JAPANESE;
}

void java_util_Locale_PUT_JAPANESE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_JAPANESE = v;
}

JAVA_OBJECT java_util_Locale_GET_KOREA()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_KOREA;
}

void java_util_Locale_PUT_KOREA(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_KOREA = v;
}

JAVA_OBJECT java_util_Locale_GET_KOREAN()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_KOREAN;
}

void java_util_Locale_PUT_KOREAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_KOREAN = v;
}

JAVA_OBJECT java_util_Locale_GET_PRC()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_PRC;
}

void java_util_Locale_PUT_PRC(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_PRC = v;
}

JAVA_OBJECT java_util_Locale_GET_SIMPLIFIED_CHINESE()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_SIMPLIFIED_CHINESE;
}

void java_util_Locale_PUT_SIMPLIFIED_CHINESE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_SIMPLIFIED_CHINESE = v;
}

JAVA_OBJECT java_util_Locale_GET_TAIWAN()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_TAIWAN;
}

void java_util_Locale_PUT_TAIWAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_TAIWAN = v;
}

JAVA_OBJECT java_util_Locale_GET_TRADITIONAL_CHINESE()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_TRADITIONAL_CHINESE;
}

void java_util_Locale_PUT_TRADITIONAL_CHINESE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_TRADITIONAL_CHINESE = v;
}

JAVA_OBJECT java_util_Locale_GET_UK()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_UK;
}

void java_util_Locale_PUT_UK(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_UK = v;
}

JAVA_OBJECT java_util_Locale_GET_US()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_US;
}

void java_util_Locale_PUT_US(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_US = v;
}

JAVA_OBJECT java_util_Locale_GET_ROOT()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_ROOT;
}

void java_util_Locale_PUT_ROOT(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_ROOT = v;
}

JAVA_OBJECT java_util_Locale_GET_serialPersistentFields()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    return _STATIC_java_util_Locale_serialPersistentFields;
}

void java_util_Locale_PUT_serialPersistentFields(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Locale)
_STATIC_java_util_Locale_serialPersistentFields = v;
}

//XMLVM_NATIVE[void java_util_Locale_initNativeLayer__()]

void java_util_Locale___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale___INIT___]
    XMLVM_ENTER_METHOD("java.util.Locale", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 190)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Locale.java", 191)
    // "en"
    _r0.o = xmlvm_create_java_string_from_pool(19);
    XMLVM_CHECK_NPE(1)
    ((java_util_Locale*) _r1.o)->fields.java_util_Locale.languageCode_ = _r0.o;
    XMLVM_SOURCE_POSITION("Locale.java", 192)
    // "US"
    _r0.o = xmlvm_create_java_string_from_pool(20);
    XMLVM_CHECK_NPE(1)
    ((java_util_Locale*) _r1.o)->fields.java_util_Locale.countryCode_ = _r0.o;
    XMLVM_SOURCE_POSITION("Locale.java", 193)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(1)
    ((java_util_Locale*) _r1.o)->fields.java_util_Locale.variantCode_ = _r0.o;
    XMLVM_SOURCE_POSITION("Locale.java", 194)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Locale___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Locale", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("Locale.java", 203)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(2)
    java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String(_r2.o, _r3.o, _r1.o, _r1.o);
    XMLVM_SOURCE_POSITION("Locale.java", 204)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Locale___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Locale", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Locale.java", 215)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(1)
    java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String(_r1.o, _r2.o, _r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 216)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Locale", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r5.o = n3;
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("Locale.java", 232)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Locale.java", 233)
    if (_r3.o == JAVA_NULL) goto label11;
    if (_r4.o == JAVA_NULL) goto label11;
    if (_r5.o != JAVA_NULL) goto label17;
    label11:;
    XMLVM_SOURCE_POSITION("Locale.java", 234)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("Locale.java", 236)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    if (_r0.i != 0) goto label40;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    if (_r0.i != 0) goto label40;
    XMLVM_SOURCE_POSITION("Locale.java", 237)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.languageCode_ = _r1.o;
    XMLVM_SOURCE_POSITION("Locale.java", 238)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.countryCode_ = _r1.o;
    XMLVM_SOURCE_POSITION("Locale.java", 239)
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.variantCode_ = _r5.o;
    label39:;
    XMLVM_SOURCE_POSITION("Locale.java", 262)
    XMLVM_EXIT_METHOD()
    return;
    label40:;
    XMLVM_SOURCE_POSITION("Locale.java", 259)
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.languageCode_ = _r3.o;
    XMLVM_SOURCE_POSITION("Locale.java", 260)
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.countryCode_ = _r4.o;
    XMLVM_SOURCE_POSITION("Locale.java", 261)
    XMLVM_CHECK_NPE(2)
    ((java_util_Locale*) _r2.o)->fields.java_util_Locale.variantCode_ = _r5.o;
    goto label39;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_clone__]
    XMLVM_ENTER_METHOD("java.util.Locale", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_TRY_BEGIN(w406aaac33b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("Locale.java", 274)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_Object_clone__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w406aaac33b1b2)
    XMLVM_CATCH_END(w406aaac33b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w406aaac33b1b2)
    label4:;
    XMLVM_SOURCE_POSITION("Locale.java", 276)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label5:;
    XMLVM_SOURCE_POSITION("Locale.java", 275)
    java_lang_Thread* curThread_w406aaac33b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w406aaac33b1b8->fields.java_lang_Thread.xmlvmException_;
    _r1.o = JAVA_NULL;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Locale_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Locale", "equals", "?")
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
    _r5.i = 1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("Locale.java", 293)
    if (_r7.o != _r6.o) goto label6;
    _r2 = _r5;
    label5:;
    XMLVM_SOURCE_POSITION("Locale.java", 294)
    XMLVM_SOURCE_POSITION("Locale.java", 302)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label6:;
    XMLVM_SOURCE_POSITION("Locale.java", 296)
    XMLVM_CLASS_INIT(java_util_Locale)
    _r2.i = XMLVM_ISA(_r7.o, __CLASS_java_util_Locale);
    if (_r2.i == 0) goto label48;
    XMLVM_SOURCE_POSITION("Locale.java", 297)
    _r0 = _r7;
    _r0.o = _r0.o;
    _r1 = _r0;
    XMLVM_SOURCE_POSITION("Locale.java", 298)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_Locale*) _r6.o)->fields.java_util_Locale.languageCode_;
    XMLVM_CHECK_NPE(1)
    _r3.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.languageCode_;
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[1])(_r2.o, _r3.o);
    if (_r2.i == 0) goto label46;
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_Locale*) _r6.o)->fields.java_util_Locale.countryCode_;
    XMLVM_CHECK_NPE(1)
    _r3.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.countryCode_;
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[1])(_r2.o, _r3.o);
    if (_r2.i == 0) goto label46;
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_Locale*) _r6.o)->fields.java_util_Locale.variantCode_;
    XMLVM_CHECK_NPE(1)
    _r3.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.variantCode_;
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[1])(_r2.o, _r3.o);
    if (_r2.i == 0) goto label46;
    _r2 = _r5;
    goto label5;
    label46:;
    _r2 = _r4;
    goto label5;
    label48:;
    _r2 = _r4;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getAvailableLocales__()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getAvailableLocales__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getAvailableLocales", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Locale.java", 312)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 313)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getCountry__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getCountry__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getCountry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 329)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.countryCode_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDefault__()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDefault__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDefault", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Locale.java", 338)
    _r0.o = java_util_Locale_GET_defaultLocale();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayCountry__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayCountry__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayCountry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 349)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Locale_getDisplayCountry___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayCountry___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayCountry___java_util_Locale]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayCountry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Locale.java", 362)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 363)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayLanguage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayLanguage__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayLanguage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 375)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Locale_getDisplayLanguage___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayLanguage___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayLanguage___java_util_Locale]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayLanguage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Locale.java", 388)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 389)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayName__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 400)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Locale_getDisplayName___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayName___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayName___java_util_Locale]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r3.i = 1;
    // " ("
    _r4.o = xmlvm_create_java_string_from_pool(22);
    XMLVM_SOURCE_POSITION("Locale.java", 412)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Locale.java", 413)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 414)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Locale*) _r5.o)->fields.java_util_Locale.languageCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r2.i <= 0) goto label26;
    XMLVM_SOURCE_POSITION("Locale.java", 415)
    XMLVM_CHECK_NPE(5)
    _r2.o = java_util_Locale_getDisplayLanguage___java_util_Locale(_r5.o, _r6.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("Locale.java", 416)
    _r1.i = _r1.i + 1;
    label26:;
    XMLVM_SOURCE_POSITION("Locale.java", 418)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Locale*) _r5.o)->fields.java_util_Locale.countryCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r2.i <= 0) goto label50;
    XMLVM_SOURCE_POSITION("Locale.java", 419)
    if (_r1.i != _r3.i) goto label41;
    XMLVM_SOURCE_POSITION("Locale.java", 420)
    // " ("
    _r2.o = xmlvm_create_java_string_from_pool(22);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r4.o);
    label41:;
    XMLVM_SOURCE_POSITION("Locale.java", 422)
    XMLVM_CHECK_NPE(5)
    _r2.o = java_util_Locale_getDisplayCountry___java_util_Locale(_r5.o, _r6.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("Locale.java", 423)
    _r1.i = _r1.i + 1;
    label50:;
    XMLVM_SOURCE_POSITION("Locale.java", 425)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Locale*) _r5.o)->fields.java_util_Locale.variantCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r2.i <= 0) goto label74;
    XMLVM_SOURCE_POSITION("Locale.java", 426)
    if (_r1.i != _r3.i) goto label86;
    XMLVM_SOURCE_POSITION("Locale.java", 427)
    // " ("
    _r2.o = xmlvm_create_java_string_from_pool(22);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r4.o);
    label65:;
    XMLVM_SOURCE_POSITION("Locale.java", 431)
    XMLVM_CHECK_NPE(5)
    _r2.o = java_util_Locale_getDisplayVariant___java_util_Locale(_r5.o, _r6.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("Locale.java", 432)
    _r1.i = _r1.i + 1;
    label74:;
    XMLVM_SOURCE_POSITION("Locale.java", 434)
    if (_r1.i <= _r3.i) goto label81;
    XMLVM_SOURCE_POSITION("Locale.java", 435)
    // ")"
    _r2.o = xmlvm_create_java_string_from_pool(23);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    label81:;
    XMLVM_SOURCE_POSITION("Locale.java", 437)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label86:;
    XMLVM_SOURCE_POSITION("Locale.java", 428)
    _r2.i = 2;
    if (_r1.i != _r2.i) goto label65;
    XMLVM_SOURCE_POSITION("Locale.java", 429)
    // ","
    _r2.o = xmlvm_create_java_string_from_pool(24);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    goto label65;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayVariant__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayVariant__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayVariant", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 448)
    _r0.o = java_util_Locale_getDefault__();
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Locale_getDisplayVariant___java_util_Locale(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getDisplayVariant___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getDisplayVariant___java_util_Locale]
    XMLVM_ENTER_METHOD("java.util.Locale", "getDisplayVariant", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Locale.java", 461)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 462)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getISO3Country__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getISO3Country__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getISO3Country", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 475)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 476)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getISO3Language__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getISO3Language__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getISO3Language", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 489)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 490)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getISOCountries__()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getISOCountries__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getISOCountries", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Locale.java", 501)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 502)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getISOLanguages__()
{
    XMLVM_CLASS_INIT(java_util_Locale)
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getISOLanguages__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getISOLanguages", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Locale.java", 513)
    org_xmlvm_runtime_XMLVMUtil_notImplemented__();
    XMLVM_SOURCE_POSITION("Locale.java", 514)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getLanguage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getLanguage__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getLanguage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 525)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.languageCode_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_getVariant__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_getVariant__]
    XMLVM_ENTER_METHOD("java.util.Locale", "getVariant", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 535)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_Locale*) _r1.o)->fields.java_util_Locale.variantCode_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Locale_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_hashCode__]
    XMLVM_ENTER_METHOD("java.util.Locale", "hashCode", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Locale.java", 547)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w406aaac52b1b4)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Locale*) _r2.o)->fields.java_util_Locale.countryCode_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Locale*) _r2.o)->fields.java_util_Locale.languageCode_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[4])(_r1.o);
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Locale*) _r2.o)->fields.java_util_Locale.variantCode_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[4])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w406aaac52b1b4)
        XMLVM_CATCH_SPECIFIC(w406aaac52b1b4,java_lang_Object,23)
    XMLVM_CATCH_END(w406aaac52b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w406aaac52b1b4)
    _r0.i = _r0.i + _r1.i;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label23:;
    java_lang_Thread* curThread_w406aaac52b1b9 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w406aaac52b1b9->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Locale_setDefault___java_util_Locale(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Locale)
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_setDefault___java_util_Locale]
    XMLVM_ENTER_METHOD("java.util.Locale", "setDefault", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Locale.java", 561)
    XMLVM_CLASS_INIT(java_util_Locale)
    _r0.o = __CLASS_java_util_Locale;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    if (_r2.o == JAVA_NULL) goto label9;
    XMLVM_TRY_BEGIN(w406aaac53b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Locale.java", 566)
    java_util_Locale_PUT_defaultLocale( _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w406aaac53b1b6)
        XMLVM_CATCH_SPECIFIC(w406aaac53b1b6,java_lang_Object,15)
    XMLVM_CATCH_END(w406aaac53b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w406aaac53b1b6)
    XMLVM_SOURCE_POSITION("Locale.java", 570)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    label9:;
    XMLVM_TRY_BEGIN(w406aaac53b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("Locale.java", 568)
    _r1.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(1)
    java_lang_NullPointerException___INIT___(_r1.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w406aaac53b1c11)
        XMLVM_CATCH_SPECIFIC(w406aaac53b1c11,java_lang_Object,15)
    XMLVM_CATCH_END(w406aaac53b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w406aaac53b1c11)
    label15:;
    java_lang_Thread* curThread_w406aaac53b1c13 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w406aaac53b1c13->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Locale_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_toString__]
    XMLVM_ENTER_METHOD("java.util.Locale", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    _r2.i = 95;
    XMLVM_SOURCE_POSITION("Locale.java", 587)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 588)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.languageCode_;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Locale.java", 589)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.countryCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r1.i <= 0) goto label28;
    XMLVM_SOURCE_POSITION("Locale.java", 590)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r2.i);
    XMLVM_SOURCE_POSITION("Locale.java", 591)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.countryCode_;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label28:;
    XMLVM_SOURCE_POSITION("Locale.java", 593)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.variantCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r1.i <= 0) goto label60;
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_AbstractStringBuilder_length__(_r0.o);
    if (_r1.i <= 0) goto label60;
    XMLVM_SOURCE_POSITION("Locale.java", 594)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.countryCode_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    if (_r1.i != 0) goto label65;
    XMLVM_SOURCE_POSITION("Locale.java", 595)
    // "__"
    _r1.o = xmlvm_create_java_string_from_pool(25);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label55:;
    XMLVM_SOURCE_POSITION("Locale.java", 599)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.variantCode_;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label60:;
    XMLVM_SOURCE_POSITION("Locale.java", 601)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label65:;
    XMLVM_SOURCE_POSITION("Locale.java", 597)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r2.i);
    goto label55;
    //XMLVM_END_WRAPPER
}

void java_util_Locale_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.Locale", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Locale.java", 611)

    
    // Red class access removed: java.io.ObjectOutputStream::putFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 612)
    // "country"
    _r1.o = xmlvm_create_java_string_from_pool(26);
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.countryCode_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 613)
    // "hashcode"
    _r1.o = xmlvm_create_java_string_from_pool(27);
    _r2.i = -1;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 614)
    // "language"
    _r1.o = xmlvm_create_java_string_from_pool(28);
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.languageCode_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 615)
    // "variant"
    _r1.o = xmlvm_create_java_string_from_pool(29);
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Locale*) _r3.o)->fields.java_util_Locale.variantCode_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 616)

    
    // Red class access removed: java.io.ObjectOutputStream::writeFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 617)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Locale_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.Locale", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    // ""
    _r3.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("Locale.java", 621)

    
    // Red class access removed: java.io.ObjectInputStream::readFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Locale.java", 622)
    // "country"
    _r1.o = xmlvm_create_java_string_from_pool(26);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(4)
    ((java_util_Locale*) _r4.o)->fields.java_util_Locale.countryCode_ = _r1.o;
    XMLVM_SOURCE_POSITION("Locale.java", 623)
    // "language"
    _r1.o = xmlvm_create_java_string_from_pool(28);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(4)
    ((java_util_Locale*) _r4.o)->fields.java_util_Locale.languageCode_ = _r1.o;
    XMLVM_SOURCE_POSITION("Locale.java", 624)
    // "variant"
    _r1.o = xmlvm_create_java_string_from_pool(29);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(4)
    ((java_util_Locale*) _r4.o)->fields.java_util_Locale.variantCode_ = _r1.o;
    XMLVM_SOURCE_POSITION("Locale.java", 625)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Locale___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_util_Locale___CLINIT___]
    XMLVM_ENTER_METHOD("java.util.Locale", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    // "fr"
    _r2.o = xmlvm_create_java_string_from_pool(30);
    // "CN"
    _r6.o = xmlvm_create_java_string_from_pool(31);
    // "en"
    _r5.o = xmlvm_create_java_string_from_pool(19);
    // "zh"
    _r4.o = xmlvm_create_java_string_from_pool(32);
    // ""
    _r3.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_SOURCE_POSITION("Locale.java", 53)
    java_util_Locale_initNativeLayer__();
    XMLVM_SOURCE_POSITION("Locale.java", 59)
    _r0.o = __NEW_java_util_Locale();
    // "en"
    _r1.o = xmlvm_create_java_string_from_pool(19);
    // "CA"
    _r1.o = xmlvm_create_java_string_from_pool(33);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r5.o, _r1.o);
    java_util_Locale_PUT_CANADA( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 64)
    _r0.o = __NEW_java_util_Locale();
    // "fr"
    _r1.o = xmlvm_create_java_string_from_pool(30);
    // "CA"
    _r1.o = xmlvm_create_java_string_from_pool(33);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r2.o, _r1.o);
    java_util_Locale_PUT_CANADA_FRENCH( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 69)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // "CN"
    _r1.o = xmlvm_create_java_string_from_pool(31);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r6.o);
    java_util_Locale_PUT_CHINA( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 74)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r3.o);
    java_util_Locale_PUT_CHINESE( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 79)
    _r0.o = __NEW_java_util_Locale();
    // "en"
    _r1.o = xmlvm_create_java_string_from_pool(19);
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r5.o, _r3.o);
    java_util_Locale_PUT_ENGLISH( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 84)
    _r0.o = __NEW_java_util_Locale();
    // "fr"
    _r1.o = xmlvm_create_java_string_from_pool(30);
    // "FR"
    _r1.o = xmlvm_create_java_string_from_pool(34);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r2.o, _r1.o);
    java_util_Locale_PUT_FRANCE( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 89)
    _r0.o = __NEW_java_util_Locale();
    // "fr"
    _r1.o = xmlvm_create_java_string_from_pool(30);
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r2.o, _r3.o);
    java_util_Locale_PUT_FRENCH( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 94)
    _r0.o = __NEW_java_util_Locale();
    // "de"
    _r1.o = xmlvm_create_java_string_from_pool(35);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r3.o);
    java_util_Locale_PUT_GERMAN( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 99)
    _r0.o = __NEW_java_util_Locale();
    // "de"
    _r1.o = xmlvm_create_java_string_from_pool(35);
    // "DE"
    _r2.o = xmlvm_create_java_string_from_pool(36);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_util_Locale_PUT_GERMANY( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 104)
    _r0.o = __NEW_java_util_Locale();
    // "it"
    _r1.o = xmlvm_create_java_string_from_pool(37);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r3.o);
    java_util_Locale_PUT_ITALIAN( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 109)
    _r0.o = __NEW_java_util_Locale();
    // "it"
    _r1.o = xmlvm_create_java_string_from_pool(37);
    // "IT"
    _r2.o = xmlvm_create_java_string_from_pool(38);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_util_Locale_PUT_ITALY( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 114)
    _r0.o = __NEW_java_util_Locale();
    // "ja"
    _r1.o = xmlvm_create_java_string_from_pool(39);
    // "JP"
    _r2.o = xmlvm_create_java_string_from_pool(40);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_util_Locale_PUT_JAPAN( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 119)
    _r0.o = __NEW_java_util_Locale();
    // "ja"
    _r1.o = xmlvm_create_java_string_from_pool(39);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r3.o);
    java_util_Locale_PUT_JAPANESE( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 124)
    _r0.o = __NEW_java_util_Locale();
    // "ko"
    _r1.o = xmlvm_create_java_string_from_pool(41);
    // "KR"
    _r2.o = xmlvm_create_java_string_from_pool(42);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    java_util_Locale_PUT_KOREA( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 129)
    _r0.o = __NEW_java_util_Locale();
    // "ko"
    _r1.o = xmlvm_create_java_string_from_pool(41);
    // ""
    _r2.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r1.o, _r3.o);
    java_util_Locale_PUT_KOREAN( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 134)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // "CN"
    _r1.o = xmlvm_create_java_string_from_pool(31);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r6.o);
    java_util_Locale_PUT_PRC( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 139)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // "CN"
    _r1.o = xmlvm_create_java_string_from_pool(31);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r6.o);
    java_util_Locale_PUT_SIMPLIFIED_CHINESE( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 144)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // "TW"
    _r1.o = xmlvm_create_java_string_from_pool(43);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r1.o);
    java_util_Locale_PUT_TAIWAN( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 149)
    _r0.o = __NEW_java_util_Locale();
    // "zh"
    _r1.o = xmlvm_create_java_string_from_pool(32);
    // "TW"
    _r1.o = xmlvm_create_java_string_from_pool(43);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r4.o, _r1.o);
    java_util_Locale_PUT_TRADITIONAL_CHINESE( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 154)
    _r0.o = __NEW_java_util_Locale();
    // "en"
    _r1.o = xmlvm_create_java_string_from_pool(19);
    // "GB"
    _r1.o = xmlvm_create_java_string_from_pool(44);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r5.o, _r1.o);
    java_util_Locale_PUT_UK( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 159)
    _r0.o = __NEW_java_util_Locale();
    // "en"
    _r1.o = xmlvm_create_java_string_from_pool(19);
    // "US"
    _r1.o = xmlvm_create_java_string_from_pool(20);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String(_r0.o, _r5.o, _r1.o);
    java_util_Locale_PUT_US( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 165)
    _r0.o = __NEW_java_util_Locale();
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    // ""
    _r1.o = xmlvm_create_java_string_from_pool(21);
    XMLVM_CHECK_NPE(0)
    java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String(_r0.o, _r3.o, _r3.o, _r3.o);
    java_util_Locale_PUT_ROOT( _r0.o);
    XMLVM_SOURCE_POSITION("Locale.java", 604)
    _r0.i = 4;
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_io_ObjectStreamField, _r0.i);
    _r1.i = 0;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "country"
    _r3.o = xmlvm_create_java_string_from_pool(26);
    XMLVM_CLASS_INIT(java_lang_String)
    _r4.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 1;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "hashcode"
    _r3.o = xmlvm_create_java_string_from_pool(27);
    _r4.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 2;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "language"
    _r3.o = xmlvm_create_java_string_from_pool(28);
    XMLVM_CLASS_INIT(java_lang_String)
    _r4.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 3;
    _r2.o = __NEW_java_io_ObjectStreamField();
    // "variant"
    _r3.o = xmlvm_create_java_string_from_pool(29);
    XMLVM_CLASS_INIT(java_lang_String)
    _r4.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    java_util_Locale_PUT_serialPersistentFields( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

