#ifndef __JAVA_LANG_FLOAT__
#define __JAVA_LANG_FLOAT__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Float 8
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
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_FloatingPointParser
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_FloatingPointParser
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_FloatingPointParser)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_NumberConverter
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_NumberConverter
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_NumberConverter)
#endif
// Class declarations for java.lang.Float
XMLVM_DEFINE_CLASS(java_lang_Float, 13, XMLVM_ITABLE_SIZE_java_lang_Float)

extern JAVA_OBJECT __CLASS_java_lang_Float;
extern JAVA_OBJECT __CLASS_java_lang_Float_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Float_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Float_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Float
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Float \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_FLOAT value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Float \
    } java_lang_Float

struct java_lang_Float {
    __TIB_DEFINITION_java_lang_Float* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Float;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
typedef struct java_lang_Float java_lang_Float;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Float 13
#define XMLVM_VTABLE_IDX_java_lang_Float_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Float_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Float_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Float_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Float_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Float_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Float_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Float_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Float_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Float_compareTo___java_lang_Object 12

void __INIT_java_lang_Float();
void __INIT_IMPL_java_lang_Float();
void __DELETE_java_lang_Float(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Float(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Float();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Float();
JAVA_LONG java_lang_Float_GET_serialVersionUID();
void java_lang_Float_PUT_serialVersionUID(JAVA_LONG v);
JAVA_FLOAT java_lang_Float_GET_MAX_VALUE();
void java_lang_Float_PUT_MAX_VALUE(JAVA_FLOAT v);
JAVA_FLOAT java_lang_Float_GET_MIN_VALUE();
void java_lang_Float_PUT_MIN_VALUE(JAVA_FLOAT v);
JAVA_FLOAT java_lang_Float_GET_MIN_NORMAL();
void java_lang_Float_PUT_MIN_NORMAL(JAVA_FLOAT v);
JAVA_FLOAT java_lang_Float_GET_NaN();
void java_lang_Float_PUT_NaN(JAVA_FLOAT v);
JAVA_FLOAT java_lang_Float_GET_POSITIVE_INFINITY();
void java_lang_Float_PUT_POSITIVE_INFINITY(JAVA_FLOAT v);
JAVA_FLOAT java_lang_Float_GET_NEGATIVE_INFINITY();
void java_lang_Float_PUT_NEGATIVE_INFINITY(JAVA_FLOAT v);
JAVA_OBJECT java_lang_Float_GET_TYPE();
void java_lang_Float_PUT_TYPE(JAVA_OBJECT v);
JAVA_INT java_lang_Float_GET_MAX_EXPONENT();
void java_lang_Float_PUT_MAX_EXPONENT(JAVA_INT v);
JAVA_INT java_lang_Float_GET_MIN_EXPONENT();
void java_lang_Float_PUT_MIN_EXPONENT(JAVA_INT v);
JAVA_INT java_lang_Float_GET_SIZE();
void java_lang_Float_PUT_SIZE(JAVA_INT v);
void java_lang_Float___CLINIT_();
void java_lang_Float___INIT____float(JAVA_OBJECT me, JAVA_FLOAT n1);
void java_lang_Float___INIT____double(JAVA_OBJECT me, JAVA_DOUBLE n1);
void java_lang_Float___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Float_compareTo___java_lang_Float(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Float_byteValue__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_DOUBLE java_lang_Float_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Float_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Float_floatToIntBits___float(JAVA_FLOAT n1);
JAVA_INT java_lang_Float_floatToRawIntBits___float(JAVA_FLOAT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Float_floatValue__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_Float_hashCode__(JAVA_OBJECT me);
JAVA_FLOAT java_lang_Float_intBitsToFloat___int(JAVA_INT n1);
// Vtable index: 9
JAVA_INT java_lang_Float_intValue__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Float_isInfinite__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Float_isInfinite___float(JAVA_FLOAT n1);
JAVA_BOOLEAN java_lang_Float_isNaN__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Float_isNaN___float(JAVA_FLOAT n1);
// Vtable index: 10
JAVA_LONG java_lang_Float_longValue__(JAVA_OBJECT me);
JAVA_FLOAT java_lang_Float_parseFloat___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 11
JAVA_SHORT java_lang_Float_shortValue__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Float_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Float_toString___float(JAVA_FLOAT n1);
JAVA_OBJECT java_lang_Float_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_INT java_lang_Float_compare___float_float(JAVA_FLOAT n1, JAVA_FLOAT n2);
JAVA_OBJECT java_lang_Float_valueOf___float(JAVA_FLOAT n1);
JAVA_OBJECT java_lang_Float_toHexString___float(JAVA_FLOAT n1);
// Vtable index: 12
JAVA_INT java_lang_Float_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
