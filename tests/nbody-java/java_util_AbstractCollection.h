#ifndef __JAVA_UTIL_ABSTRACTCOLLECTION__
#define __JAVA_UTIL_ABSTRACTCOLLECTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_AbstractCollection 16
// Implemented interfaces:
#include "java_util_Collection.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Array
#define XMLVM_FORWARD_DECL_java_lang_reflect_Array
XMLVM_FORWARD_DECL(java_lang_reflect_Array)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
// Class declarations for java.util.AbstractCollection
XMLVM_DEFINE_CLASS(java_util_AbstractCollection, 19, XMLVM_ITABLE_SIZE_java_util_AbstractCollection)

extern JAVA_OBJECT __CLASS_java_util_AbstractCollection;
extern JAVA_OBJECT __CLASS_java_util_AbstractCollection_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractCollection_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractCollection_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractCollection
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_AbstractCollection \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractCollection \
    } java_util_AbstractCollection

struct java_util_AbstractCollection {
    __TIB_DEFINITION_java_util_AbstractCollection* tib;
    struct {
        __INSTANCE_FIELDS_java_util_AbstractCollection;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_AbstractCollection
#define XMLVM_FORWARD_DECL_java_util_AbstractCollection
typedef struct java_util_AbstractCollection java_util_AbstractCollection;
#endif

#define XMLVM_VTABLE_SIZE_java_util_AbstractCollection 19
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_add___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_addAll___java_util_Collection 6
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_clear__ 8
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_containsAll___java_util_Collection 9
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_iterator__ 12
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_remove___java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_removeAll___java_util_Collection 13
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_retainAll___java_util_Collection 15
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_size__ 16
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_toArray__ 17
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_toArray___java_lang_Object_1ARRAY 18
#define XMLVM_VTABLE_IDX_java_util_AbstractCollection_toString__ 5

void __INIT_java_util_AbstractCollection();
void __INIT_IMPL_java_util_AbstractCollection();
void __DELETE_java_util_AbstractCollection(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_AbstractCollection(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_AbstractCollection();
JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractCollection();
void java_util_AbstractCollection___INIT___(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_AbstractCollection_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_BOOLEAN java_util_AbstractCollection_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
void java_util_AbstractCollection_clear__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_AbstractCollection_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_BOOLEAN java_util_AbstractCollection_containsAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 11
JAVA_BOOLEAN java_util_AbstractCollection_isEmpty__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_AbstractCollection_iterator__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_BOOLEAN java_util_AbstractCollection_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 13
JAVA_BOOLEAN java_util_AbstractCollection_removeAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 15
JAVA_BOOLEAN java_util_AbstractCollection_retainAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_AbstractCollection_size__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_AbstractCollection_toArray__(JAVA_OBJECT me);
// Vtable index: 18
JAVA_OBJECT java_util_AbstractCollection_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_util_AbstractCollection_toString__(JAVA_OBJECT me);

#endif
