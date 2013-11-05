#ifndef __JAVA_UTIL_COLLECTIONS_EMPTYSET__
#define __JAVA_UTIL_COLLECTIONS_EMPTYSET__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Collections_EmptySet 31
// Implemented interfaces:
#include "java_io_Serializable.h"
// Super Class:
#include "java_util_AbstractSet.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collections
#define XMLVM_FORWARD_DECL_java_util_Collections
XMLVM_FORWARD_DECL(java_util_Collections)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
// Class declarations for java.util.Collections$EmptySet
XMLVM_DEFINE_CLASS(java_util_Collections_EmptySet, 19, XMLVM_ITABLE_SIZE_java_util_Collections_EmptySet)

extern JAVA_OBJECT __CLASS_java_util_Collections_EmptySet;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptySet
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Collections_EmptySet \
    __INSTANCE_FIELDS_java_util_AbstractSet; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptySet \
    } java_util_Collections_EmptySet

struct java_util_Collections_EmptySet {
    __TIB_DEFINITION_java_util_Collections_EmptySet* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Collections_EmptySet;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Collections_EmptySet
#define XMLVM_FORWARD_DECL_java_util_Collections_EmptySet
typedef struct java_util_Collections_EmptySet java_util_Collections_EmptySet;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Collections_EmptySet 19
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptySet_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptySet_size__ 16
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptySet_iterator__ 12

void __INIT_java_util_Collections_EmptySet();
void __INIT_IMPL_java_util_Collections_EmptySet();
void __DELETE_java_util_Collections_EmptySet(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptySet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Collections_EmptySet();
JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptySet();
JAVA_LONG java_util_Collections_EmptySet_GET_serialVersionUID();
void java_util_Collections_EmptySet_PUT_serialVersionUID(JAVA_LONG v);
void java_util_Collections_EmptySet___INIT___(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_Collections_EmptySet_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_Collections_EmptySet_size__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_Collections_EmptySet_iterator__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Collections_EmptySet_readResolve__(JAVA_OBJECT me);
void java_util_Collections_EmptySet___INIT____java_util_Collections_EmptySet(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
