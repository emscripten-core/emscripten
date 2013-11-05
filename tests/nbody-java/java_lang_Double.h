#ifndef __JAVA_LANG_DOUBLE__
#define __JAVA_LANG_DOUBLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Double 8
// Implemented interfaces:
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Number.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
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
// Class declarations for java.lang.Double
XMLVM_DEFINE_CLASS(java_lang_Double, 13, XMLVM_ITABLE_SIZE_java_lang_Double)

extern JAVA_OBJECT __CLASS_java_lang_Double;
extern JAVA_OBJECT __CLASS_java_lang_Double_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Double_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Double_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Double
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Double \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_DOUBLE value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Double \
    } java_lang_Double

struct java_lang_Double {
    __TIB_DEFINITION_java_lang_Double* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Double;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
typedef struct java_lang_Double java_lang_Double;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Double 13
#define XMLVM_VTABLE_IDX_java_lang_Double_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Double_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Double_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Double_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Double_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Double_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Double_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Double_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Double_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Double_compareTo___java_lang_Object 12

void __INIT_java_lang_Double();
void __INIT_IMPL_java_lang_Double();
void __DELETE_java_lang_Double(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Double(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Double();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Double();
JAVA_LONG java_lang_Double_GET_serialVersionUID();
void java_lang_Double_PUT_serialVersionUID(JAVA_LONG v);
JAVA_DOUBLE java_lang_Double_GET_MAX_VALUE();
void java_lang_Double_PUT_MAX_VALUE(JAVA_DOUBLE v);
JAVA_DOUBLE java_lang_Double_GET_MIN_VALUE();
void java_lang_Double_PUT_MIN_VALUE(JAVA_DOUBLE v);
JAVA_DOUBLE java_lang_Double_GET_MIN_NORMAL();
void java_lang_Double_PUT_MIN_NORMAL(JAVA_DOUBLE v);
JAVA_DOUBLE java_lang_Double_GET_NaN();
void java_lang_Double_PUT_NaN(JAVA_DOUBLE v);
JAVA_DOUBLE java_lang_Double_GET_POSITIVE_INFINITY();
void java_lang_Double_PUT_POSITIVE_INFINITY(JAVA_DOUBLE v);
JAVA_INT java_lang_Double_GET_MAX_EXPONENT();
void java_lang_Double_PUT_MAX_EXPONENT(JAVA_INT v);
JAVA_INT java_lang_Double_GET_MIN_EXPONENT();
void java_lang_Double_PUT_MIN_EXPONENT(JAVA_INT v);
JAVA_DOUBLE java_lang_Double_GET_NEGATIVE_INFINITY();
void java_lang_Double_PUT_NEGATIVE_INFINITY(JAVA_DOUBLE v);
JAVA_OBJECT java_lang_Double_GET_TYPE();
void java_lang_Double_PUT_TYPE(JAVA_OBJECT v);
JAVA_INT java_lang_Double_GET_SIZE();
void java_lang_Double_PUT_SIZE(JAVA_INT v);
void java_lang_Double___CLINIT_();
void java_lang_Double___INIT____double(JAVA_OBJECT me, JAVA_DOUBLE n1);
void java_lang_Double___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Double_compareTo___java_lang_Double(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Double_byteValue__(JAVA_OBJECT me);
JAVA_LONG java_lang_Double_doubleToLongBits___double(JAVA_DOUBLE n1);
JAVA_LONG java_lang_Double_doubleToRawLongBits___double(JAVA_DOUBLE n1);
// Vtable index: 7
JAVA_DOUBLE java_lang_Double_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Double_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Double_floatValue__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_Double_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Double_intValue__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Double_isInfinite__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Double_isInfinite___double(JAVA_DOUBLE n1);
JAVA_BOOLEAN java_lang_Double_isNaN__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Double_isNaN___double(JAVA_DOUBLE n1);
JAVA_DOUBLE java_lang_Double_longBitsToDouble___long(JAVA_LONG n1);
// Vtable index: 10
JAVA_LONG java_lang_Double_longValue__(JAVA_OBJECT me);
JAVA_DOUBLE java_lang_Double_parseDouble___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 11
JAVA_SHORT java_lang_Double_shortValue__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Double_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Double_toString___double(JAVA_DOUBLE n1);
JAVA_OBJECT java_lang_Double_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_INT java_lang_Double_compare___double_double(JAVA_DOUBLE n1, JAVA_DOUBLE n2);
JAVA_OBJECT java_lang_Double_valueOf___double(JAVA_DOUBLE n1);
JAVA_OBJECT java_lang_Double_toHexString___double(JAVA_DOUBLE n1);
// Vtable index: 12
JAVA_INT java_lang_Double_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
