#ifndef __JAVA_UTIL_LOCALE__
#define __JAVA_UTIL_LOCALE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Locale 0
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
XMLVM_FORWARD_DECL(java_io_ObjectStreamField)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
XMLVM_FORWARD_DECL(org_xmlvm_runtime_XMLVMUtil)
#endif
// Class declarations for java.util.Locale
XMLVM_DEFINE_CLASS(java_util_Locale, 6, XMLVM_ITABLE_SIZE_java_util_Locale)

extern JAVA_OBJECT __CLASS_java_util_Locale;
extern JAVA_OBJECT __CLASS_java_util_Locale_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Locale_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Locale_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Locale
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Locale \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT countryCode_; \
        JAVA_OBJECT languageCode_; \
        JAVA_OBJECT variantCode_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Locale \
    } java_util_Locale

struct java_util_Locale {
    __TIB_DEFINITION_java_util_Locale* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Locale;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Locale
#define XMLVM_FORWARD_DECL_java_util_Locale
typedef struct java_util_Locale java_util_Locale;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Locale 6
#define XMLVM_VTABLE_IDX_java_util_Locale_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_Locale_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_Locale_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_Locale_toString__ 5

void __INIT_java_util_Locale();
void __INIT_IMPL_java_util_Locale();
void __DELETE_java_util_Locale(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Locale(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Locale();
JAVA_OBJECT __NEW_INSTANCE_java_util_Locale();
JAVA_LONG java_util_Locale_GET_serialVersionUID();
void java_util_Locale_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_util_Locale_GET_defaultLocale();
void java_util_Locale_PUT_defaultLocale(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_CANADA();
void java_util_Locale_PUT_CANADA(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_CANADA_FRENCH();
void java_util_Locale_PUT_CANADA_FRENCH(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_CHINA();
void java_util_Locale_PUT_CHINA(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_CHINESE();
void java_util_Locale_PUT_CHINESE(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_ENGLISH();
void java_util_Locale_PUT_ENGLISH(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_FRANCE();
void java_util_Locale_PUT_FRANCE(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_FRENCH();
void java_util_Locale_PUT_FRENCH(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_GERMAN();
void java_util_Locale_PUT_GERMAN(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_GERMANY();
void java_util_Locale_PUT_GERMANY(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_ITALIAN();
void java_util_Locale_PUT_ITALIAN(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_ITALY();
void java_util_Locale_PUT_ITALY(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_JAPAN();
void java_util_Locale_PUT_JAPAN(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_JAPANESE();
void java_util_Locale_PUT_JAPANESE(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_KOREA();
void java_util_Locale_PUT_KOREA(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_KOREAN();
void java_util_Locale_PUT_KOREAN(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_PRC();
void java_util_Locale_PUT_PRC(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_SIMPLIFIED_CHINESE();
void java_util_Locale_PUT_SIMPLIFIED_CHINESE(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_TAIWAN();
void java_util_Locale_PUT_TAIWAN(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_TRADITIONAL_CHINESE();
void java_util_Locale_PUT_TRADITIONAL_CHINESE(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_UK();
void java_util_Locale_PUT_UK(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_US();
void java_util_Locale_PUT_US(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_ROOT();
void java_util_Locale_PUT_ROOT(JAVA_OBJECT v);
JAVA_OBJECT java_util_Locale_GET_serialPersistentFields();
void java_util_Locale_PUT_serialPersistentFields(JAVA_OBJECT v);
void java_util_Locale_initNativeLayer__();
void java_util_Locale___INIT___(JAVA_OBJECT me);
void java_util_Locale___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Locale___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Locale___INIT____java_lang_String_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
// Vtable index: 0
JAVA_OBJECT java_util_Locale_clone__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_Locale_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Locale_getAvailableLocales__();
JAVA_OBJECT java_util_Locale_getCountry__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getDefault__();
JAVA_OBJECT java_util_Locale_getDisplayCountry__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getDisplayCountry___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Locale_getDisplayLanguage__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getDisplayLanguage___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Locale_getDisplayName__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getDisplayName___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Locale_getDisplayVariant__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getDisplayVariant___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Locale_getISO3Country__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getISO3Language__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getISOCountries__();
JAVA_OBJECT java_util_Locale_getISOLanguages__();
JAVA_OBJECT java_util_Locale_getLanguage__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Locale_getVariant__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_util_Locale_hashCode__(JAVA_OBJECT me);
void java_util_Locale_setDefault___java_util_Locale(JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_util_Locale_toString__(JAVA_OBJECT me);
void java_util_Locale_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Locale_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Locale___CLINIT_();

#endif
