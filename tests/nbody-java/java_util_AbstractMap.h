#ifndef __JAVA_UTIL_ABSTRACTMAP__
#define __JAVA_UTIL_ABSTRACTMAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_AbstractMap 14
// Implemented interfaces:
#include "java_util_Map.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
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
// Class declarations for java.util.AbstractMap
XMLVM_DEFINE_CLASS(java_util_AbstractMap, 18, XMLVM_ITABLE_SIZE_java_util_AbstractMap)

extern JAVA_OBJECT __CLASS_java_util_AbstractMap;
extern JAVA_OBJECT __CLASS_java_util_AbstractMap_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractMap_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractMap_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractMap
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_AbstractMap \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT keySet_; \
        JAVA_OBJECT valuesCollection_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractMap \
    } java_util_AbstractMap

struct java_util_AbstractMap {
    __TIB_DEFINITION_java_util_AbstractMap* tib;
    struct {
        __INSTANCE_FIELDS_java_util_AbstractMap;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_AbstractMap
#define XMLVM_FORWARD_DECL_java_util_AbstractMap
typedef struct java_util_AbstractMap java_util_AbstractMap;
#endif

#define XMLVM_VTABLE_SIZE_java_util_AbstractMap 18
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_clear__ 6
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_containsKey___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_containsValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_entrySet__ 9
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_get___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_keySet__ 12
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_put___java_lang_Object_java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_putAll___java_util_Map 13
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_remove___java_lang_Object 15
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_size__ 16
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_toString__ 5
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_values__ 17
#define XMLVM_VTABLE_IDX_java_util_AbstractMap_clone__ 0

void __INIT_java_util_AbstractMap();
void __INIT_IMPL_java_util_AbstractMap();
void __DELETE_java_util_AbstractMap(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_AbstractMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_AbstractMap();
JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractMap();
void java_util_AbstractMap___INIT___(JAVA_OBJECT me);
// Vtable index: 6
void java_util_AbstractMap_clear__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_AbstractMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_AbstractMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_AbstractMap_entrySet__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_AbstractMap_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 10
JAVA_OBJECT java_util_AbstractMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_util_AbstractMap_hashCode__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_util_AbstractMap_isEmpty__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_AbstractMap_keySet__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_util_AbstractMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 13
void java_util_AbstractMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 15
JAVA_OBJECT java_util_AbstractMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_AbstractMap_size__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_util_AbstractMap_toString__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_AbstractMap_values__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_AbstractMap_clone__(JAVA_OBJECT me);

#endif
