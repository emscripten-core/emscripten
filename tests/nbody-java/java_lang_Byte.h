#ifndef __JAVA_LANG_BYTE__
#define __JAVA_LANG_BYTE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Byte 8
// Implemented interfaces:
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Number.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.Byte
XMLVM_DEFINE_CLASS(java_lang_Byte, 13, XMLVM_ITABLE_SIZE_java_lang_Byte)

extern JAVA_OBJECT __CLASS_java_lang_Byte;
extern JAVA_OBJECT __CLASS_java_lang_Byte_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Byte_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Byte_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Byte
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Byte \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_BYTE value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Byte \
    } java_lang_Byte

struct java_lang_Byte {
    __TIB_DEFINITION_java_lang_Byte* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Byte;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Byte
#define XMLVM_FORWARD_DECL_java_lang_Byte
typedef struct java_lang_Byte java_lang_Byte;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Byte 13
#define XMLVM_VTABLE_IDX_java_lang_Byte_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Byte_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Byte_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Byte_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Byte_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Byte_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Byte_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Byte_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Byte_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Byte_compareTo___java_lang_Object 12

void __INIT_java_lang_Byte();
void __INIT_IMPL_java_lang_Byte();
void __DELETE_java_lang_Byte(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Byte(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Byte();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Byte();
JAVA_LONG java_lang_Byte_GET_serialVersionUID();
void java_lang_Byte_PUT_serialVersionUID(JAVA_LONG v);
JAVA_BYTE java_lang_Byte_GET_MAX_VALUE();
void java_lang_Byte_PUT_MAX_VALUE(JAVA_BYTE v);
JAVA_BYTE java_lang_Byte_GET_MIN_VALUE();
void java_lang_Byte_PUT_MIN_VALUE(JAVA_BYTE v);
JAVA_INT java_lang_Byte_GET_SIZE();
void java_lang_Byte_PUT_SIZE(JAVA_INT v);
JAVA_OBJECT java_lang_Byte_GET_TYPE();
void java_lang_Byte_PUT_TYPE(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Byte_GET_CACHE();
void java_lang_Byte_PUT_CACHE(JAVA_OBJECT v);
void java_lang_Byte___CLINIT_();
void java_lang_Byte___INIT____byte(JAVA_OBJECT me, JAVA_BYTE n1);
void java_lang_Byte___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Byte_byteValue__(JAVA_OBJECT me);
JAVA_INT java_lang_Byte_compareTo___java_lang_Byte(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Byte_decode___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 7
JAVA_DOUBLE java_lang_Byte_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Byte_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Byte_floatValue__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_Byte_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Byte_intValue__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_LONG java_lang_Byte_longValue__(JAVA_OBJECT me);
JAVA_BYTE java_lang_Byte_parseByte___java_lang_String(JAVA_OBJECT n1);
JAVA_BYTE java_lang_Byte_parseByte___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 11
JAVA_SHORT java_lang_Byte_shortValue__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Byte_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Byte_toString___byte(JAVA_BYTE n1);
JAVA_OBJECT java_lang_Byte_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Byte_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_Byte_valueOf___byte(JAVA_BYTE n1);
// Vtable index: 12
JAVA_INT java_lang_Byte_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
