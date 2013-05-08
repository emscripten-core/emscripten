#ifndef __JAVA_LANG_NUMBER__
#define __JAVA_LANG_NUMBER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Number 0
// Implemented interfaces:
#include "java_io_Serializable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for java.lang.Number
XMLVM_DEFINE_CLASS(java_lang_Number, 12, XMLVM_ITABLE_SIZE_java_lang_Number)

extern JAVA_OBJECT __CLASS_java_lang_Number;
extern JAVA_OBJECT __CLASS_java_lang_Number_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Number_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Number_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Number
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Number \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Number \
    } java_lang_Number

struct java_lang_Number {
    __TIB_DEFINITION_java_lang_Number* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Number;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Number
#define XMLVM_FORWARD_DECL_java_lang_Number
typedef struct java_lang_Number java_lang_Number;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Number 12
#define XMLVM_VTABLE_IDX_java_lang_Number_byteValue__ 6
#define XMLVM_VTABLE_IDX_java_lang_Number_doubleValue__ 7
#define XMLVM_VTABLE_IDX_java_lang_Number_floatValue__ 8
#define XMLVM_VTABLE_IDX_java_lang_Number_intValue__ 9
#define XMLVM_VTABLE_IDX_java_lang_Number_longValue__ 10
#define XMLVM_VTABLE_IDX_java_lang_Number_shortValue__ 11

void __INIT_java_lang_Number();
void __INIT_IMPL_java_lang_Number();
void __DELETE_java_lang_Number(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Number(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Number();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Number();
JAVA_LONG java_lang_Number_GET_serialVersionUID();
void java_lang_Number_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_Number___INIT___(JAVA_OBJECT me);
// Vtable index: 6
JAVA_BYTE java_lang_Number_byteValue__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_DOUBLE java_lang_Number_doubleValue__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_FLOAT java_lang_Number_floatValue__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_Number_intValue__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_LONG java_lang_Number_longValue__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_SHORT java_lang_Number_shortValue__(JAVA_OBJECT me);

#endif
