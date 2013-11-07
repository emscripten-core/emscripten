#ifndef __ORG_APACHE_HARMONY_LUNI_UTIL_NUMBERCONVERTER__
#define __ORG_APACHE_HARMONY_LUNI_UTIL_NUMBERCONVERTER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_apache_harmony_luni_util_NumberConverter 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
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
#ifndef XMLVM_FORWARD_DECL_java_lang_Math
#define XMLVM_FORWARD_DECL_java_lang_Math
XMLVM_FORWARD_DECL(java_lang_Math)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for org.apache.harmony.luni.util.NumberConverter
XMLVM_DEFINE_CLASS(org_apache_harmony_luni_util_NumberConverter, 6, XMLVM_ITABLE_SIZE_org_apache_harmony_luni_util_NumberConverter)

extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_1ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_2ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_NumberConverter_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_util_NumberConverter
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_apache_harmony_luni_util_NumberConverter \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_INT setCount_; \
        JAVA_INT getCount_; \
        JAVA_OBJECT uArray_; \
        JAVA_INT firstK_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_util_NumberConverter \
    } org_apache_harmony_luni_util_NumberConverter

struct org_apache_harmony_luni_util_NumberConverter {
    __TIB_DEFINITION_org_apache_harmony_luni_util_NumberConverter* tib;
    struct {
        __INSTANCE_FIELDS_org_apache_harmony_luni_util_NumberConverter;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_NumberConverter
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_NumberConverter
typedef struct org_apache_harmony_luni_util_NumberConverter org_apache_harmony_luni_util_NumberConverter;
#endif

#define XMLVM_VTABLE_SIZE_org_apache_harmony_luni_util_NumberConverter 6

void __INIT_org_apache_harmony_luni_util_NumberConverter();
void __INIT_IMPL_org_apache_harmony_luni_util_NumberConverter();
void __DELETE_org_apache_harmony_luni_util_NumberConverter(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_NumberConverter(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_apache_harmony_luni_util_NumberConverter();
JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_util_NumberConverter();
void xmlvm_init_native_org_apache_harmony_luni_util_NumberConverter();
JAVA_DOUBLE org_apache_harmony_luni_util_NumberConverter_GET_invLogOfTenBaseTwo();
void org_apache_harmony_luni_util_NumberConverter_PUT_invLogOfTenBaseTwo(JAVA_DOUBLE v);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_GET_TEN_TO_THE();
void org_apache_harmony_luni_util_NumberConverter_PUT_TEN_TO_THE(JAVA_OBJECT v);
void org_apache_harmony_luni_util_NumberConverter___CLINIT_();
void org_apache_harmony_luni_util_NumberConverter___INIT___(JAVA_OBJECT me);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_getConverter__();
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convert___double(JAVA_DOUBLE n1);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convert___float(JAVA_FLOAT n1);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convertD___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_convertF___float(JAVA_OBJECT me, JAVA_FLOAT n1);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_freeFormatExponential__(JAVA_OBJECT me);
JAVA_OBJECT org_apache_harmony_luni_util_NumberConverter_freeFormat__(JAVA_OBJECT me);
void org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4, JAVA_INT n5);
void org_apache_harmony_luni_util_NumberConverter_longDigitGenerator___long_int_boolean_boolean_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4, JAVA_INT n5);

#endif
