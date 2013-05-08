#ifndef __JAVA_UTIL_ABSTRACTSET__
#define __JAVA_UTIL_ABSTRACTSET__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_AbstractSet 31
// Implemented interfaces:
#include "java_util_Set.h"
// Super Class:
#include "java_util_AbstractCollection.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
// Class declarations for java.util.AbstractSet
XMLVM_DEFINE_CLASS(java_util_AbstractSet, 19, XMLVM_ITABLE_SIZE_java_util_AbstractSet)

extern JAVA_OBJECT __CLASS_java_util_AbstractSet;
extern JAVA_OBJECT __CLASS_java_util_AbstractSet_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractSet_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractSet_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractSet
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_AbstractSet \
    __INSTANCE_FIELDS_java_util_AbstractCollection; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractSet \
    } java_util_AbstractSet

struct java_util_AbstractSet {
    __TIB_DEFINITION_java_util_AbstractSet* tib;
    struct {
        __INSTANCE_FIELDS_java_util_AbstractSet;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_AbstractSet
#define XMLVM_FORWARD_DECL_java_util_AbstractSet
typedef struct java_util_AbstractSet java_util_AbstractSet;
#endif

#define XMLVM_VTABLE_SIZE_java_util_AbstractSet 19
#define XMLVM_VTABLE_IDX_java_util_AbstractSet_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_AbstractSet_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_AbstractSet_removeAll___java_util_Collection 13

void __INIT_java_util_AbstractSet();
void __INIT_IMPL_java_util_AbstractSet();
void __DELETE_java_util_AbstractSet(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_AbstractSet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_AbstractSet();
JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractSet();
void java_util_AbstractSet___INIT___(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_AbstractSet_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_util_AbstractSet_hashCode__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_BOOLEAN java_util_AbstractSet_removeAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
