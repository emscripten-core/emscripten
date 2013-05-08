#ifndef __JAVA_UTIL_HASHTABLE_2__
#define __JAVA_UTIL_HASHTABLE_2__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Hashtable_2 3
// Implemented interfaces:
#include "java_util_Iterator.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable
#define XMLVM_FORWARD_DECL_java_util_Hashtable
XMLVM_FORWARD_DECL(java_util_Hashtable)
#endif
// Class declarations for java.util.Hashtable$2
XMLVM_DEFINE_CLASS(java_util_Hashtable_2, 9, XMLVM_ITABLE_SIZE_java_util_Hashtable_2)

extern JAVA_OBJECT __CLASS_java_util_Hashtable_2;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_2_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_2_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_2_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable_2
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Hashtable_2 \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable_2 \
    } java_util_Hashtable_2

struct java_util_Hashtable_2 {
    __TIB_DEFINITION_java_util_Hashtable_2* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Hashtable_2;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable_2
#define XMLVM_FORWARD_DECL_java_util_Hashtable_2
typedef struct java_util_Hashtable_2 java_util_Hashtable_2;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Hashtable_2 9
#define XMLVM_VTABLE_IDX_java_util_Hashtable_2_hasNext__ 6
#define XMLVM_VTABLE_IDX_java_util_Hashtable_2_next__ 7
#define XMLVM_VTABLE_IDX_java_util_Hashtable_2_remove__ 8

void __INIT_java_util_Hashtable_2();
void __INIT_IMPL_java_util_Hashtable_2();
void __DELETE_java_util_Hashtable_2(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Hashtable_2(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Hashtable_2();
JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable_2();
void java_util_Hashtable_2___INIT___(JAVA_OBJECT me);
// Vtable index: 6
JAVA_BOOLEAN java_util_Hashtable_2_hasNext__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_util_Hashtable_2_next__(JAVA_OBJECT me);
// Vtable index: 8
void java_util_Hashtable_2_remove__(JAVA_OBJECT me);

#endif
