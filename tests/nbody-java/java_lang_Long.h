#ifndef __JAVA_LANG_LONG__
#define __JAVA_LANG_LONG__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Long 8
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
#ifndef XMLVM_FORWARD_DECL_java_lang_Long_valueOfCache
#define XMLVM_FORWARD_DECL_java_lang_Long_valueOfCache
XMLVM_FORWARD_DECL(java_lang_Long_valueOfCache)
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
// Class declarations for java.lang.Long
XMLVM_DEFINE_CLASS(java_lang_Long, 13, XMLVM_ITABLE_SIZE_java_lang_Long)

extern JAVA_OBJECT __CLASS_java_lang_Long;
extern JAVA_OBJECT __CLASS_java_lang_Long_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Long_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Long_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Long
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Long \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_LONG value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Long \
    } java_lang_Long

struct java_lang_Long {
    __TIB_DEFINITION_java_lang_Long* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Long;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
typedef struct java_lang_Long java_lang_Long;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Long 13
#define XMLVM_VTABLE_IDX_java_lang_Long_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Long_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Long_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Long_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Long_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Long_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Long_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Long_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Long_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Long_compareTo___java_lang_Object 12

void __INIT_java_lang_Long();
void __INIT_IMPL_java_lang_Long();
void __DELETE_java_lang_Long(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Long(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Long();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Long();
JAVA_LONG java_lang_Long_GET_serialVersionUID();
void java_lang_Long_PUT_serialVersionUID(JAVA_LONG v);
JAVA_LONG java_lang_Long_GET_MAX_VALUE();
void java_lang_Long_PUT_MAX_VALUE(JAVA_LONG v);
JAVA_LONG java_lang_Long_GET_MIN_VALUE();
void java_lang_Long_PUT_MIN_VALUE(JAVA_LONG v);
JAVA_OBJECT java_lang_Long_GET_TYPE();
void java_lang_Long_PUT_TYPE(JAVA_OBJECT v);
JAVA_INT java_lang_Long_GET_SIZE();
void java_lang_Long_PUT_SIZE(JAVA_INT v);
void java_lang_Long___CLINIT_();
void java_lang_Long___INIT____long(JAVA_OBJECT me, JAVA_LONG n1);
void java_lang_Long___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Long_byteValue__(JAVA_OBJECT me);
JAVA_INT java_lang_Long_compareTo___java_lang_Long(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Long_decode___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 7
JAVA_DOUBLE java_lang_Long_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Long_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Long_floatValue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Long_getLong___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Long_getLong___java_lang_String_long(JAVA_OBJECT n1, JAVA_LONG n2);
JAVA_OBJECT java_lang_Long_getLong___java_lang_String_java_lang_Long(JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 4
JAVA_INT java_lang_Long_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Long_intValue__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_LONG java_lang_Long_longValue__(JAVA_OBJECT me);
JAVA_LONG java_lang_Long_parseLong___java_lang_String(JAVA_OBJECT n1);
JAVA_LONG java_lang_Long_parseLong___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_LONG java_lang_Long_parse___java_lang_String_int_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_BOOLEAN n4);
// Vtable index: 11
JAVA_SHORT java_lang_Long_shortValue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Long_toBinaryString___long(JAVA_LONG n1);
JAVA_OBJECT java_lang_Long_toHexString___long(JAVA_LONG n1);
JAVA_OBJECT java_lang_Long_toOctalString___long(JAVA_LONG n1);
// Vtable index: 5
JAVA_OBJECT java_lang_Long_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Long_toString___long(JAVA_LONG n1);
JAVA_OBJECT java_lang_Long_toString___long_int(JAVA_LONG n1, JAVA_INT n2);
JAVA_OBJECT java_lang_Long_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Long_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_LONG java_lang_Long_highestOneBit___long(JAVA_LONG n1);
JAVA_LONG java_lang_Long_lowestOneBit___long(JAVA_LONG n1);
JAVA_INT java_lang_Long_numberOfLeadingZeros___long(JAVA_LONG n1);
JAVA_INT java_lang_Long_numberOfTrailingZeros___long(JAVA_LONG n1);
JAVA_INT java_lang_Long_bitCount___long(JAVA_LONG n1);
JAVA_LONG java_lang_Long_rotateLeft___long_int(JAVA_LONG n1, JAVA_INT n2);
JAVA_LONG java_lang_Long_rotateRight___long_int(JAVA_LONG n1, JAVA_INT n2);
JAVA_LONG java_lang_Long_reverseBytes___long(JAVA_LONG n1);
JAVA_LONG java_lang_Long_reverse___long(JAVA_LONG n1);
JAVA_INT java_lang_Long_signum___long(JAVA_LONG n1);
JAVA_OBJECT java_lang_Long_valueOf___long(JAVA_LONG n1);
// Vtable index: 12
JAVA_INT java_lang_Long_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
