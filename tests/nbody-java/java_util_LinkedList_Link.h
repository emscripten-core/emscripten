#ifndef __JAVA_UTIL_LINKEDLIST_LINK__
#define __JAVA_UTIL_LINKEDLIST_LINK__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_LinkedList_Link 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for java.util.LinkedList$Link
XMLVM_DEFINE_CLASS(java_util_LinkedList_Link, 6, XMLVM_ITABLE_SIZE_java_util_LinkedList_Link)

extern JAVA_OBJECT __CLASS_java_util_LinkedList_Link;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_Link_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_Link_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_Link_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_LinkedList_Link
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_LinkedList_Link \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT data_; \
        JAVA_OBJECT previous_; \
        JAVA_OBJECT next_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_LinkedList_Link \
    } java_util_LinkedList_Link

struct java_util_LinkedList_Link {
    __TIB_DEFINITION_java_util_LinkedList_Link* tib;
    struct {
        __INSTANCE_FIELDS_java_util_LinkedList_Link;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_LinkedList_Link
#define XMLVM_FORWARD_DECL_java_util_LinkedList_Link
typedef struct java_util_LinkedList_Link java_util_LinkedList_Link;
#endif

#define XMLVM_VTABLE_SIZE_java_util_LinkedList_Link 6

void __INIT_java_util_LinkedList_Link();
void __INIT_IMPL_java_util_LinkedList_Link();
void __DELETE_java_util_LinkedList_Link(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_LinkedList_Link(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_LinkedList_Link();
JAVA_OBJECT __NEW_INSTANCE_java_util_LinkedList_Link();
void java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);

#endif
