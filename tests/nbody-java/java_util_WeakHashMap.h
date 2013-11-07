#ifndef __JAVA_UTIL_WEAKHASHMAP__
#define __JAVA_UTIL_WEAKHASHMAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_WeakHashMap 14
// Implemented interfaces:
#include "java_util_Map.h"
// Super Class:
#include "java_util_AbstractMap.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_Reference
#define XMLVM_FORWARD_DECL_java_lang_ref_Reference
XMLVM_FORWARD_DECL(java_lang_ref_Reference)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
#define XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
XMLVM_FORWARD_DECL(java_lang_ref_ReferenceQueue)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_WeakHashMap_Entry
#define XMLVM_FORWARD_DECL_java_util_WeakHashMap_Entry
XMLVM_FORWARD_DECL(java_util_WeakHashMap_Entry)
#endif
// Class declarations for java.util.WeakHashMap
XMLVM_DEFINE_CLASS(java_util_WeakHashMap, 18, XMLVM_ITABLE_SIZE_java_util_WeakHashMap)

extern JAVA_OBJECT __CLASS_java_util_WeakHashMap;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_WeakHashMap
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_WeakHashMap \
    __INSTANCE_FIELDS_java_util_AbstractMap; \
    struct { \
        JAVA_OBJECT referenceQueue_; \
        JAVA_INT elementCount_; \
        JAVA_OBJECT elementData_; \
        JAVA_INT loadFactor_; \
        JAVA_INT threshold_; \
        JAVA_INT modCount_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_WeakHashMap \
    } java_util_WeakHashMap

struct java_util_WeakHashMap {
    __TIB_DEFINITION_java_util_WeakHashMap* tib;
    struct {
        __INSTANCE_FIELDS_java_util_WeakHashMap;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_WeakHashMap
#define XMLVM_FORWARD_DECL_java_util_WeakHashMap
typedef struct java_util_WeakHashMap java_util_WeakHashMap;
#endif

#define XMLVM_VTABLE_SIZE_java_util_WeakHashMap 18
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_clear__ 6
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_containsKey___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_entrySet__ 9
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_keySet__ 12
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_values__ 17
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_get___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_containsValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_put___java_lang_Object_java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_putAll___java_util_Map 13
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_remove___java_lang_Object 15
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_size__ 16

void __INIT_java_util_WeakHashMap();
void __INIT_IMPL_java_util_WeakHashMap();
void __DELETE_java_util_WeakHashMap(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_WeakHashMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_WeakHashMap();
JAVA_OBJECT __NEW_INSTANCE_java_util_WeakHashMap();
JAVA_INT java_util_WeakHashMap_GET_DEFAULT_SIZE();
void java_util_WeakHashMap_PUT_DEFAULT_SIZE(JAVA_INT v);
JAVA_OBJECT java_util_WeakHashMap_newEntryArray___int(JAVA_INT n1);
void java_util_WeakHashMap___INIT___(JAVA_OBJECT me);
void java_util_WeakHashMap___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_WeakHashMap___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
void java_util_WeakHashMap___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
void java_util_WeakHashMap_clear__(JAVA_OBJECT me);
void java_util_WeakHashMap_computeMaxSize__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_WeakHashMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_WeakHashMap_entrySet__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_WeakHashMap_keySet__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_WeakHashMap_values__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_util_WeakHashMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_WeakHashMap_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_WeakHashMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 11
JAVA_BOOLEAN java_util_WeakHashMap_isEmpty__(JAVA_OBJECT me);
void java_util_WeakHashMap_poll__(JAVA_OBJECT me);
void java_util_WeakHashMap_removeEntry___java_util_WeakHashMap_Entry(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 14
JAVA_OBJECT java_util_WeakHashMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_WeakHashMap_rehash__(JAVA_OBJECT me);
// Vtable index: 13
void java_util_WeakHashMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 15
JAVA_OBJECT java_util_WeakHashMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_WeakHashMap_size__(JAVA_OBJECT me);
void java_util_WeakHashMap_putAllImpl___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
