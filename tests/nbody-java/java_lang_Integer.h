#ifndef __JAVA_LANG_INTEGER__
#define __JAVA_LANG_INTEGER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Integer 8
// Implemented interfaces:
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Number.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
// Class declarations for java.lang.Integer
XMLVM_DEFINE_CLASS(java_lang_Integer, 13, XMLVM_ITABLE_SIZE_java_lang_Integer)

extern JAVA_OBJECT __CLASS_java_lang_Integer;
extern JAVA_OBJECT __CLASS_java_lang_Integer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Integer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Integer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Integer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Integer \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_INT value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Integer \
    } java_lang_Integer

struct java_lang_Integer {
    __TIB_DEFINITION_java_lang_Integer* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Integer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
typedef struct java_lang_Integer java_lang_Integer;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Integer 13
#define XMLVM_VTABLE_IDX_java_lang_Integer_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Integer_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Integer_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Integer_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Integer_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Integer_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Integer_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Integer_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Integer_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Integer_compareTo___java_lang_Object 12

void __INIT_java_lang_Integer();
void __INIT_IMPL_java_lang_Integer();
void __DELETE_java_lang_Integer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Integer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Integer();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Integer();
JAVA_LONG java_lang_Integer_GET_serialVersionUID();
void java_lang_Integer_PUT_serialVersionUID(JAVA_LONG v);
JAVA_INT java_lang_Integer_GET_MAX_VALUE();
void java_lang_Integer_PUT_MAX_VALUE(JAVA_INT v);
JAVA_INT java_lang_Integer_GET_MIN_VALUE();
void java_lang_Integer_PUT_MIN_VALUE(JAVA_INT v);
JAVA_INT java_lang_Integer_GET_SIZE();
void java_lang_Integer_PUT_SIZE(JAVA_INT v);
JAVA_OBJECT java_lang_Integer_GET_decimalScale();
void java_lang_Integer_PUT_decimalScale(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Integer_GET_TYPE();
void java_lang_Integer_PUT_TYPE(JAVA_OBJECT v);
void java_lang_Integer___CLINIT_();
void java_lang_Integer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_Integer___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Integer_byteValue__(JAVA_OBJECT me);
JAVA_INT java_lang_Integer_compareTo___java_lang_Integer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Integer_decode___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 7
JAVA_DOUBLE java_lang_Integer_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Integer_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Integer_floatValue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Integer_getInteger___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Integer_getInteger___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_Integer_getInteger___java_lang_String_java_lang_Integer(JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 4
JAVA_INT java_lang_Integer_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Integer_intValue__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_LONG java_lang_Integer_longValue__(JAVA_OBJECT me);
JAVA_INT java_lang_Integer_parseInt___java_lang_String(JAVA_OBJECT n1);
JAVA_INT java_lang_Integer_parseInt___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_INT java_lang_Integer_parse___java_lang_String_int_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4);
// Vtable index: 11
JAVA_SHORT java_lang_Integer_shortValue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Integer_toBinaryString___int(JAVA_INT n1);
JAVA_OBJECT java_lang_Integer_toHexString___int(JAVA_INT n1);
JAVA_OBJECT java_lang_Integer_toOctalString___int(JAVA_INT n1);
// Vtable index: 5
JAVA_OBJECT java_lang_Integer_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Integer_toString___int(JAVA_INT n1);
JAVA_OBJECT java_lang_Integer_toString___int_int(JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_Integer_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Integer_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_INT java_lang_Integer_highestOneBit___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_lowestOneBit___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_numberOfLeadingZeros___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_numberOfTrailingZeros___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_bitCount___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_rotateLeft___int_int(JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_Integer_rotateRight___int_int(JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_Integer_reverseBytes___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_reverse___int(JAVA_INT n1);
JAVA_INT java_lang_Integer_signum___int(JAVA_INT n1);
JAVA_OBJECT java_lang_Integer_valueOf___int(JAVA_INT n1);
// Vtable index: 12
JAVA_INT java_lang_Integer_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
