#ifndef __JAVA_UTIL_HASHMAP__
#define __JAVA_UTIL_HASHMAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_HashMap 14
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
#include "java_util_Map.h"
// Super Class:
#include "java_util_AbstractMap.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap_Entry
#define XMLVM_FORWARD_DECL_java_util_HashMap_Entry
XMLVM_FORWARD_DECL(java_util_HashMap_Entry)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Map_Entry
#define XMLVM_FORWARD_DECL_java_util_Map_Entry
XMLVM_FORWARD_DECL(java_util_Map_Entry)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
// Class declarations for java.util.HashMap
XMLVM_DEFINE_CLASS(java_util_HashMap, 18, XMLVM_ITABLE_SIZE_java_util_HashMap)

extern JAVA_OBJECT __CLASS_java_util_HashMap;
extern JAVA_OBJECT __CLASS_java_util_HashMap_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashMap_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_HashMap_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_HashMap
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_HashMap \
    __INSTANCE_FIELDS_java_util_AbstractMap; \
    struct { \
        JAVA_INT elementCount_; \
        JAVA_OBJECT elementData_; \
        JAVA_INT modCount_; \
        JAVA_FLOAT loadFactor_; \
        JAVA_INT threshold_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_HashMap \
    } java_util_HashMap

struct java_util_HashMap {
    __TIB_DEFINITION_java_util_HashMap* tib;
    struct {
        __INSTANCE_FIELDS_java_util_HashMap;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap
#define XMLVM_FORWARD_DECL_java_util_HashMap
typedef struct java_util_HashMap java_util_HashMap;
#endif

#define XMLVM_VTABLE_SIZE_java_util_HashMap 18
#define XMLVM_VTABLE_IDX_java_util_HashMap_clear__ 6
#define XMLVM_VTABLE_IDX_java_util_HashMap_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_HashMap_containsKey___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_HashMap_containsValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_HashMap_entrySet__ 9
#define XMLVM_VTABLE_IDX_java_util_HashMap_get___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_HashMap_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_HashMap_keySet__ 12
#define XMLVM_VTABLE_IDX_java_util_HashMap_put___java_lang_Object_java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_HashMap_putAll___java_util_Map 13
#define XMLVM_VTABLE_IDX_java_util_HashMap_remove___java_lang_Object 15
#define XMLVM_VTABLE_IDX_java_util_HashMap_size__ 16
#define XMLVM_VTABLE_IDX_java_util_HashMap_values__ 17

void __INIT_java_util_HashMap();
void __INIT_IMPL_java_util_HashMap();
void __DELETE_java_util_HashMap(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_HashMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_HashMap();
JAVA_OBJECT __NEW_INSTANCE_java_util_HashMap();
JAVA_LONG java_util_HashMap_GET_serialVersionUID();
void java_util_HashMap_PUT_serialVersionUID(JAVA_LONG v);
JAVA_INT java_util_HashMap_GET_DEFAULT_SIZE();
void java_util_HashMap_PUT_DEFAULT_SIZE(JAVA_INT v);
JAVA_OBJECT java_util_HashMap_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_HashMap___INIT___(JAVA_OBJECT me);
void java_util_HashMap___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_util_HashMap_calculateCapacity___int(JAVA_INT n1);
void java_util_HashMap___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
void java_util_HashMap___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
void java_util_HashMap_clear__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_HashMap_clone__(JAVA_OBJECT me);
void java_util_HashMap_computeThreshold__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_HashMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_HashMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_HashMap_entrySet__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_util_HashMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_HashMap_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_HashMap_findNonNullKeyEntry___java_lang_Object_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_util_HashMap_findNullKeyEntry__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_util_HashMap_isEmpty__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_HashMap_keySet__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_util_HashMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_util_HashMap_putImpl___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_util_HashMap_createEntry___java_lang_Object_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_util_HashMap_createHashedEntry___java_lang_Object_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 13
void java_util_HashMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashMap_putAllImpl___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashMap_rehash___int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_HashMap_rehash__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_OBJECT java_util_HashMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashMap_removeEntry___java_util_HashMap_Entry(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_HashMap_removeEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_HashMap_size__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_HashMap_values__(JAVA_OBJECT me);
void java_util_HashMap_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_HashMap_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_util_HashMap_computeHashCode___java_lang_Object(JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_HashMap_areEqualKeys___java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_BOOLEAN java_util_HashMap_areEqualValues___java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2);

#endif
