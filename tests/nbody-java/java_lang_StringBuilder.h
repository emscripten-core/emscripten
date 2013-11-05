#ifndef __JAVA_LANG_STRINGBUILDER__
#define __JAVA_LANG_STRINGBUILDER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_StringBuilder 7
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Appendable.h"
#include "java_lang_CharSequence.h"
// Super Class:
#include "java_lang_AbstractStringBuilder.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
XMLVM_FORWARD_DECL(java_lang_Double)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
XMLVM_FORWARD_DECL(java_lang_Float)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.StringBuilder
XMLVM_DEFINE_CLASS(java_lang_StringBuilder, 9, XMLVM_ITABLE_SIZE_java_lang_StringBuilder)

extern JAVA_OBJECT __CLASS_java_lang_StringBuilder;
extern JAVA_OBJECT __CLASS_java_lang_StringBuilder_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_StringBuilder_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_StringBuilder_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_StringBuilder
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_StringBuilder \
    __INSTANCE_FIELDS_java_lang_AbstractStringBuilder; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_StringBuilder \
    } java_lang_StringBuilder

struct java_lang_StringBuilder {
    __TIB_DEFINITION_java_lang_StringBuilder* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_StringBuilder;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
typedef struct java_lang_StringBuilder java_lang_StringBuilder;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_StringBuilder 9
#define XMLVM_VTABLE_IDX_java_lang_StringBuilder_append___char 6
#define XMLVM_VTABLE_IDX_java_lang_StringBuilder_append___java_lang_CharSequence 7
#define XMLVM_VTABLE_IDX_java_lang_StringBuilder_append___java_lang_CharSequence_int_int 8
#define XMLVM_VTABLE_IDX_java_lang_StringBuilder_toString__ 5

void __INIT_java_lang_StringBuilder();
void __INIT_IMPL_java_lang_StringBuilder();
void __DELETE_java_lang_StringBuilder(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_StringBuilder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_StringBuilder();
JAVA_OBJECT __NEW_INSTANCE_java_lang_StringBuilder();
JAVA_LONG java_lang_StringBuilder_GET_serialVersionUID();
void java_lang_StringBuilder_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_StringBuilder___INIT___(JAVA_OBJECT me);
void java_lang_StringBuilder___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_StringBuilder___INIT____java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_StringBuilder___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StringBuilder_append___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
// Vtable index: 6
JAVA_OBJECT java_lang_StringBuilder_append___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_OBJECT java_lang_StringBuilder_append___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_StringBuilder_append___long(JAVA_OBJECT me, JAVA_LONG n1);
JAVA_OBJECT java_lang_StringBuilder_append___float(JAVA_OBJECT me, JAVA_FLOAT n1);
JAVA_OBJECT java_lang_StringBuilder_append___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
JAVA_OBJECT java_lang_StringBuilder_append___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StringBuilder_append___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StringBuilder_append___java_lang_StringBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StringBuilder_append___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StringBuilder_append___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 7
JAVA_OBJECT java_lang_StringBuilder_append___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_OBJECT java_lang_StringBuilder_append___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_lang_StringBuilder_appendCodePoint___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_StringBuilder_delete___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_StringBuilder_deleteCharAt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_StringBuilder_insert___int_boolean(JAVA_OBJECT me, JAVA_INT n1, JAVA_BOOLEAN n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_long(JAVA_OBJECT me, JAVA_INT n1, JAVA_LONG n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_double(JAVA_OBJECT me, JAVA_INT n1, JAVA_DOUBLE n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_CharSequence(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_StringBuilder_insert___int_java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
JAVA_OBJECT java_lang_StringBuilder_replace___int_int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_lang_StringBuilder_reverse__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_StringBuilder_toString__(JAVA_OBJECT me);
void java_lang_StringBuilder_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_StringBuilder_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
