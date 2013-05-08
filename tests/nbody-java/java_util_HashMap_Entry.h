#ifndef __JAVA_UTIL_HASHMAP_ENTRY__
#define __JAVA_UTIL_HASHMAP_ENTRY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_HashMap_Entry 5
// Implemented interfaces:
// Super Class:
#include "java_util_MapEntry.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap
#define XMLVM_FORWARD_DECL_java_util_HashMap
XMLVM_FORWARD_DECL(java_util_HashMap)
#endif
// Class declarations for java.util.HashMap$Entry
XMLVM_DEFINE_CLASS(java_util_HashMap_Entry, 9, XMLVM_ITABLE_SIZE_java_util_HashMap_Entry)

extern JAVA_OBJECT __CLASS_java_util_HashMap_Entry;
extern JAVA_OBJECT __CLASS_java_util_HashMap_Entry_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashMap_Entry_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashMap_Entry_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_HashMap_Entry
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_HashMap_Entry \
    __INSTANCE_FIELDS_java_util_MapEntry; \
    struct { \
        JAVA_INT origKeyHash_; \
        JAVA_OBJECT next_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_HashMap_Entry \
    } java_util_HashMap_Entry

struct java_util_HashMap_Entry {
    __TIB_DEFINITION_java_util_HashMap_Entry* tib;
    struct {
        __INSTANCE_FIELDS_java_util_HashMap_Entry;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap_Entry
#define XMLVM_FORWARD_DECL_java_util_HashMap_Entry
typedef struct java_util_HashMap_Entry java_util_HashMap_Entry;
#endif

#define XMLVM_VTABLE_SIZE_java_util_HashMap_Entry 9
#define XMLVM_VTABLE_IDX_java_util_HashMap_Entry_clone__ 0

void __INIT_java_util_HashMap_Entry();
void __INIT_IMPL_java_util_HashMap_Entry();
void __DELETE_java_util_HashMap_Entry(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_HashMap_Entry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_HashMap_Entry();
JAVA_OBJECT __NEW_INSTANCE_java_util_HashMap_Entry();
void java_util_HashMap_Entry___INIT____java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_util_HashMap_Entry___INIT____java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 0
JAVA_OBJECT java_util_HashMap_Entry_clone__(JAVA_OBJECT me);

#endif
