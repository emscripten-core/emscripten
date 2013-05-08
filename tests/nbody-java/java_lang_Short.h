#ifndef __JAVA_LANG_SHORT__
#define __JAVA_LANG_SHORT__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Short 8
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
// Class declarations for java.lang.Short
XMLVM_DEFINE_CLASS(java_lang_Short, 13, XMLVM_ITABLE_SIZE_java_lang_Short)

extern JAVA_OBJECT __CLASS_java_lang_Short;
extern JAVA_OBJECT __CLASS_java_lang_Short_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Short_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Short_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Short
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Short \
    __INSTANCE_FIELDS_java_lang_Number; \
    struct { \
        JAVA_SHORT value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Short \
    } java_lang_Short

struct java_lang_Short {
    __TIB_DEFINITION_java_lang_Short* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Short;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Short
#define XMLVM_FORWARD_DECL_java_lang_Short
typedef struct java_lang_Short java_lang_Short;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Short 13
#define XMLVM_VTABLE_IDX_java_lang_Short_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Short_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Short_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Short_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Short_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Short_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Short_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Short_shortValue__ 11
#define XMLVM_VTABLE_IDX_java_lang_Short_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Short_compareTo___java_lang_Object 12

void __INIT_java_lang_Short();
void __INIT_IMPL_java_lang_Short();
void __DELETE_java_lang_Short(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Short(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Short();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Short();
JAVA_LONG java_lang_Short_GET_serialVersionUID();
void java_lang_Short_PUT_serialVersionUID(JAVA_LONG v);
JAVA_SHORT java_lang_Short_GET_MAX_VALUE();
void java_lang_Short_PUT_MAX_VALUE(JAVA_SHORT v);
JAVA_SHORT java_lang_Short_GET_MIN_VALUE();
void java_lang_Short_PUT_MIN_VALUE(JAVA_SHORT v);
JAVA_INT java_lang_Short_GET_SIZE();
void java_lang_Short_PUT_SIZE(JAVA_INT v);
JAVA_OBJECT java_lang_Short_GET_TYPE();
void java_lang_Short_PUT_TYPE(JAVA_OBJECT v);
void java_lang_Short___CLINIT_();
void java_lang_Short___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Short___INIT____short(JAVA_OBJECT me, JAVA_SHORT n1);
// Vtable index: 6
JAVA_BYTE java_lang_Short_byteValue__(JAVA_OBJECT me);
JAVA_INT java_lang_Short_compareTo___java_lang_Short(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Short_decode___java_lang_String(JAVA_OBJECT n1);
// Vtable index: 7
JAVA_DOUBLE java_lang_Short_doubleValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Short_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_FLOAT java_lang_Short_floatValue__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_Short_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Short_intValue__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_LONG java_lang_Short_longValue__(JAVA_OBJECT me);
JAVA_SHORT java_lang_Short_parseShort___java_lang_String(JAVA_OBJECT n1);
JAVA_SHORT java_lang_Short_parseShort___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 11
JAVA_SHORT java_lang_Short_shortValue__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Short_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Short_toString___short(JAVA_SHORT n1);
JAVA_OBJECT java_lang_Short_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Short_valueOf___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_SHORT java_lang_Short_reverseBytes___short(JAVA_SHORT n1);
JAVA_OBJECT java_lang_Short_valueOf___short(JAVA_SHORT n1);
// Vtable index: 12
JAVA_INT java_lang_Short_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
