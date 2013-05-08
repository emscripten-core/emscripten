#ifndef __JAVA_LANG_STRING__
#define __JAVA_LANG_STRING__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_String 8
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_CharSequence.h"
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
XMLVM_FORWARD_DECL(java_lang_Double)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Exception
#define XMLVM_FORWARD_DECL_java_lang_Exception
XMLVM_FORWARD_DECL(java_lang_Exception)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
XMLVM_FORWARD_DECL(java_lang_Float)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Math
#define XMLVM_FORWARD_DECL_java_lang_Math
XMLVM_FORWARD_DECL(java_lang_Math)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String_CaseInsensitiveComparator
#define XMLVM_FORWARD_DECL_java_lang_String_CaseInsensitiveComparator
XMLVM_FORWARD_DECL(java_lang_String_CaseInsensitiveComparator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ByteBuffer
XMLVM_FORWARD_DECL(java_nio_ByteBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_AccessController
#define XMLVM_FORWARD_DECL_java_security_AccessController
XMLVM_FORWARD_DECL(java_security_AccessController)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Comparator
#define XMLVM_FORWARD_DECL_java_util_Comparator
XMLVM_FORWARD_DECL(java_util_Comparator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Locale
#define XMLVM_FORWARD_DECL_java_util_Locale
XMLVM_FORWARD_DECL(java_util_Locale)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_PriviAction)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_niochar_charset_UTF_8
#define XMLVM_FORWARD_DECL_org_apache_harmony_niochar_charset_UTF_8
XMLVM_FORWARD_DECL(org_apache_harmony_niochar_charset_UTF_8)
#endif
// Class declarations for java.lang.String
XMLVM_DEFINE_CLASS(java_lang_String, 10, XMLVM_ITABLE_SIZE_java_lang_String)

extern JAVA_OBJECT __CLASS_java_lang_String;
extern JAVA_OBJECT __CLASS_java_lang_String_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_String_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_String_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_String
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_String \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT charset_; \
        JAVA_OBJECT charset2_; \
        JAVA_OBJECT charset3_; \
        JAVA_OBJECT value_; \
        JAVA_INT offset_; \
        JAVA_INT count_; \
        JAVA_INT hashCode_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_String \
    } java_lang_String

struct java_lang_String {
    __TIB_DEFINITION_java_lang_String* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_String;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
typedef struct java_lang_String java_lang_String;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_String 10
#define XMLVM_VTABLE_IDX_java_lang_String_charAt___int 6
#define XMLVM_VTABLE_IDX_java_lang_String_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_String_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_String_length__ 8
#define XMLVM_VTABLE_IDX_java_lang_String_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_String_subSequence___int_int 9
#define XMLVM_VTABLE_IDX_java_lang_String_compareTo___java_lang_Object 7

void __INIT_java_lang_String();
void __INIT_IMPL_java_lang_String();
void __DELETE_java_lang_String(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_String(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_String();
JAVA_OBJECT __NEW_INSTANCE_java_lang_String();
void xmlvm_init_native_java_lang_String();
JAVA_LONG java_lang_String_GET_serialVersionUID();
void java_lang_String_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_lang_String_GET_CASE_INSENSITIVE_ORDER();
void java_lang_String_PUT_CASE_INSENSITIVE_ORDER(JAVA_OBJECT v);
JAVA_OBJECT java_lang_String_GET_ascii();
void java_lang_String_PUT_ascii(JAVA_OBJECT v);
JAVA_OBJECT java_lang_String_GET_DefaultCharset();
void java_lang_String_PUT_DefaultCharset(JAVA_OBJECT v);
JAVA_OBJECT java_lang_String_GET_lastCharset();
void java_lang_String_PUT_lastCharset(JAVA_OBJECT v);
void java_lang_String___INIT___(JAVA_OBJECT me);
void java_lang_String___INIT____java_lang_String_char(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_CHAR n2);
void java_lang_String___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___INIT____byte_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_String___INIT____byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_lang_String___INIT____byte_1ARRAY_int_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_INT n4);
void java_lang_String___INIT____byte_1ARRAY_int_int_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_OBJECT n4);
void java_lang_String___INIT____byte_1ARRAY_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_String___INIT____char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___INIT____char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_lang_String___INIT____int_int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3);
void java_lang_String___INIT____char_1ARRAY_int_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4);
void java_lang_String___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_String___INIT____java_lang_String_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_lang_String___INIT____java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___INIT____int_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_lang_String___INIT____java_lang_StringBuilder(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 6
JAVA_CHAR java_lang_String_charAt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_CHAR java_lang_String_compareValue___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_CHAR java_lang_String_toLowerCase___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_CHAR java_lang_String_toUpperCase___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_INT java_lang_String_compareTo___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_compareToIgnoreCase___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_concat___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_copyValueOf___char_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_copyValueOf___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_lang_String_defaultCharset__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_String_endsWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 1
JAVA_BOOLEAN java_lang_String_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_String_equalsIgnoreCase___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_getBytes__(JAVA_OBJECT me);
void java_lang_String_getBytes___int_int_byte_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4);
JAVA_OBJECT java_lang_String_getBytes___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_getBytes___java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_getCharset___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String_getChars___int_int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4);
// Vtable index: 4
JAVA_INT java_lang_String_hashCode__(JAVA_OBJECT me);
JAVA_INT java_lang_String_indexOf___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_String_indexOf___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_String_indexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_indexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_String_intern__(JAVA_OBJECT me);
JAVA_INT java_lang_String_lastIndexOf___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_String_lastIndexOf___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_String_lastIndexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_lastIndexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 8
JAVA_INT java_lang_String_length__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_String_regionMatches___int_java_lang_String_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
JAVA_BOOLEAN java_lang_String_regionMatches___boolean_int_java_lang_String_int_int(JAVA_OBJECT me, JAVA_BOOLEAN n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
JAVA_OBJECT java_lang_String_replace___char_char(JAVA_OBJECT me, JAVA_CHAR n1, JAVA_CHAR n2);
JAVA_OBJECT java_lang_String_replace___java_lang_CharSequence_java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_BOOLEAN java_lang_String_startsWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_String_startsWith___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_String_substring___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_String_substring___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_String_toCharArray__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_String_toLowerCase__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_String_toLowerCase___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_toLowerCaseImpl___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 5
JAVA_OBJECT java_lang_String_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_String_toUpperCase__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_String_toUpperCase___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_toUpperCaseImpl___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_String_trim__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_String_valueOf___char_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_valueOf___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_lang_String_valueOf___char(JAVA_CHAR n1);
JAVA_OBJECT java_lang_String_valueOf___double(JAVA_DOUBLE n1);
JAVA_OBJECT java_lang_String_valueOf___float(JAVA_FLOAT n1);
JAVA_OBJECT java_lang_String_valueOf___int(JAVA_INT n1);
JAVA_OBJECT java_lang_String_valueOf___long(JAVA_LONG n1);
JAVA_OBJECT java_lang_String_valueOf___java_lang_Object(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_valueOf___boolean(JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_lang_String_contentEquals___java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_String_contentEquals___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_String_matches___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_replaceAll___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_String_replaceFirst___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_String_split___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_String_split___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 9
JAVA_OBJECT java_lang_String_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_String_codePointAt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_String_codePointBefore___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_String_codePointCount___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_BOOLEAN java_lang_String_contains___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_String_offsetByCodePoints___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_String_format___java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_String_format___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_INT java_lang_String_indexOf___java_lang_String_java_lang_String_int_int_char(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_CHAR n5);
JAVA_OBJECT java_lang_String_getValue__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_String_isEmpty__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_INT java_lang_String_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_String___CLINIT_();

#endif
