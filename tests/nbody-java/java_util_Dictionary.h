#ifndef __JAVA_UTIL_DICTIONARY__
#define __JAVA_UTIL_DICTIONARY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Dictionary 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
// Class declarations for java.util.Dictionary
XMLVM_DEFINE_CLASS(java_util_Dictionary, 13, XMLVM_ITABLE_SIZE_java_util_Dictionary)

extern JAVA_OBJECT __CLASS_java_util_Dictionary;
extern JAVA_OBJECT __CLASS_java_util_Dictionary_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Dictionary_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Dictionary_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Dictionary
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Dictionary \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Dictionary \
    } java_util_Dictionary

struct java_util_Dictionary {
    __TIB_DEFINITION_java_util_Dictionary* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Dictionary;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Dictionary
#define XMLVM_FORWARD_DECL_java_util_Dictionary
typedef struct java_util_Dictionary java_util_Dictionary;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Dictionary 13
#define XMLVM_VTABLE_IDX_java_util_Dictionary_elements__ 6
#define XMLVM_VTABLE_IDX_java_util_Dictionary_get___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_Dictionary_isEmpty__ 8
#define XMLVM_VTABLE_IDX_java_util_Dictionary_keys__ 9
#define XMLVM_VTABLE_IDX_java_util_Dictionary_put___java_lang_Object_java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Dictionary_remove___java_lang_Object 11
#define XMLVM_VTABLE_IDX_java_util_Dictionary_size__ 12

void __INIT_java_util_Dictionary();
void __INIT_IMPL_java_util_Dictionary();
void __DELETE_java_util_Dictionary(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Dictionary(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Dictionary();
JAVA_OBJECT __NEW_INSTANCE_java_util_Dictionary();
void java_util_Dictionary___INIT___(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT java_util_Dictionary_elements__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_util_Dictionary_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_Dictionary_isEmpty__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_OBJECT java_util_Dictionary_keys__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_util_Dictionary_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 11
JAVA_OBJECT java_util_Dictionary_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_INT java_util_Dictionary_size__(JAVA_OBJECT me);

#endif
