#ifndef __JAVA_UTIL_HASHSET__
#define __JAVA_UTIL_HASHSET__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_HashSet 31
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
#include "java_util_Set.h"
// Super Class:
#include "java_util_AbstractSet.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap
#define XMLVM_FORWARD_DECL_java_util_HashMap
XMLVM_FORWARD_DECL(java_util_HashMap)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap_Entry
#define XMLVM_FORWARD_DECL_java_util_HashMap_Entry
XMLVM_FORWARD_DECL(java_util_HashMap_Entry)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
// Class declarations for java.util.HashSet
XMLVM_DEFINE_CLASS(java_util_HashSet, 19, XMLVM_ITABLE_SIZE_java_util_HashSet)

extern JAVA_OBJECT __CLASS_java_util_HashSet;
extern JAVA_OBJECT __CLASS_java_util_HashSet_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashSet_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashSet_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_HashSet
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_HashSet \
    __INSTANCE_FIELDS_java_util_AbstractSet; \
    struct { \
        JAVA_OBJECT backingMap_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_HashSet \
    } java_util_HashSet

struct java_util_HashSet {
    __TIB_DEFINITION_java_util_HashSet* tib;
    struct {
        __INSTANCE_FIELDS_java_util_HashSet;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_HashSet
#define XMLVM_FORWARD_DECL_java_util_HashSet
typedef struct java_util_HashSet java_util_HashSet;
#endif

#define XMLVM_VTABLE_SIZE_java_util_HashSet 19
#define XMLVM_VTABLE_IDX_java_util_HashSet_add___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_HashSet_clear__ 8
#define XMLVM_VTABLE_IDX_java_util_HashSet_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_HashSet_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_HashSet_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_HashSet_iterator__ 12
#define XMLVM_VTABLE_IDX_java_util_HashSet_remove___java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_HashSet_size__ 16

void __INIT_java_util_HashSet();
void __INIT_IMPL_java_util_HashSet();
void __DELETE_java_util_HashSet(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_HashSet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_HashSet();
JAVA_OBJECT __NEW_INSTANCE_java_util_HashSet();
JAVA_LONG java_util_HashSet_GET_serialVersionUID();
void java_util_HashSet_PUT_serialVersionUID(JAVA_LONG v);
void java_util_HashSet___INIT___(JAVA_OBJECT me);
void java_util_HashSet___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_HashSet___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
void java_util_HashSet___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashSet___INIT____java_util_HashMap(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_BOOLEAN java_util_HashSet_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
void java_util_HashSet_clear__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_HashSet_clone__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_HashSet_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 11
JAVA_BOOLEAN java_util_HashSet_isEmpty__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_HashSet_iterator__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_BOOLEAN java_util_HashSet_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_HashSet_size__(JAVA_OBJECT me);
void java_util_HashSet_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashSet_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_HashSet_createBackingMap___int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);

#endif
