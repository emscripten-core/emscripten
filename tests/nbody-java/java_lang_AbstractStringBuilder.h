#ifndef __JAVA_LANG_ABSTRACTSTRINGBUILDER__
#define __JAVA_LANG_ABSTRACTSTRINGBUILDER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_AbstractStringBuilder 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_InvalidObjectException
#define XMLVM_FORWARD_DECL_java_io_InvalidObjectException
XMLVM_FORWARD_DECL(java_io_InvalidObjectException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
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
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.lang.AbstractStringBuilder
XMLVM_DEFINE_CLASS(java_lang_AbstractStringBuilder, 6, XMLVM_ITABLE_SIZE_java_lang_AbstractStringBuilder)

extern JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder;
extern JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_AbstractStringBuilder
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_AbstractStringBuilder \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT value_; \
        JAVA_INT count_; \
        JAVA_BOOLEAN shared_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_AbstractStringBuilder \
    } java_lang_AbstractStringBuilder

struct java_lang_AbstractStringBuilder {
    __TIB_DEFINITION_java_lang_AbstractStringBuilder* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_AbstractStringBuilder;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_AbstractStringBuilder
#define XMLVM_FORWARD_DECL_java_lang_AbstractStringBuilder
typedef struct java_lang_AbstractStringBuilder java_lang_AbstractStringBuilder;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_AbstractStringBuilder 6
#define XMLVM_VTABLE_IDX_java_lang_AbstractStringBuilder_toString__ 5

void __INIT_java_lang_AbstractStringBuilder();
void __INIT_IMPL_java_lang_AbstractStringBuilder();
void __DELETE_java_lang_AbstractStringBuilder(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_AbstractStringBuilder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_AbstractStringBuilder();
JAVA_OBJECT __NEW_INSTANCE_java_lang_AbstractStringBuilder();
JAVA_INT java_lang_AbstractStringBuilder_GET_INITIAL_CAPACITY();
void java_lang_AbstractStringBuilder_PUT_INITIAL_CAPACITY(JAVA_INT v);
JAVA_OBJECT java_lang_AbstractStringBuilder_getValue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_AbstractStringBuilder_shareValue__(JAVA_OBJECT me);
void java_lang_AbstractStringBuilder_set___char_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_AbstractStringBuilder___INIT___(JAVA_OBJECT me);
void java_lang_AbstractStringBuilder___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_AbstractStringBuilder___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_AbstractStringBuilder_enlargeBuffer___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_AbstractStringBuilder_appendNull__(JAVA_OBJECT me);
void java_lang_AbstractStringBuilder_append0___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_AbstractStringBuilder_append0___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_lang_AbstractStringBuilder_append0___char(JAVA_OBJECT me, JAVA_CHAR n1);
void java_lang_AbstractStringBuilder_append0___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_AbstractStringBuilder_append0___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_INT java_lang_AbstractStringBuilder_capacity__(JAVA_OBJECT me);
JAVA_CHAR java_lang_AbstractStringBuilder_charAt___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_AbstractStringBuilder_delete0___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
void java_lang_AbstractStringBuilder_deleteCharAt0___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_AbstractStringBuilder_ensureCapacity___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_AbstractStringBuilder_getChars___int_int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4);
void java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
void java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
void java_lang_AbstractStringBuilder_insert0___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
void java_lang_AbstractStringBuilder_insert0___int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
void java_lang_AbstractStringBuilder_insert0___int_java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
JAVA_INT java_lang_AbstractStringBuilder_length__(JAVA_OBJECT me);
void java_lang_AbstractStringBuilder_move___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
void java_lang_AbstractStringBuilder_replace0___int_int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3);
void java_lang_AbstractStringBuilder_reverse0__(JAVA_OBJECT me);
void java_lang_AbstractStringBuilder_setCharAt___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
void java_lang_AbstractStringBuilder_setLength___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_AbstractStringBuilder_substring___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_AbstractStringBuilder_substring___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 5
JAVA_OBJECT java_lang_AbstractStringBuilder_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_AbstractStringBuilder_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_AbstractStringBuilder_indexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_AbstractStringBuilder_indexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
JAVA_INT java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_AbstractStringBuilder_trimToSize__(JAVA_OBJECT me);
JAVA_INT java_lang_AbstractStringBuilder_codePointAt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_AbstractStringBuilder_codePointBefore___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_AbstractStringBuilder_codePointCount___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_AbstractStringBuilder_offsetByCodePoints___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);

#endif
