#ifndef __JAVA_UTIL_COLLECTIONS_EMPTYMAP__
#define __JAVA_UTIL_COLLECTIONS_EMPTYMAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Collections_EmptyMap 14
// Implemented interfaces:
#include "java_io_Serializable.h"
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
#ifndef XMLVM_FORWARD_DECL_java_util_Collections
#define XMLVM_FORWARD_DECL_java_util_Collections
XMLVM_FORWARD_DECL(java_util_Collections)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_List
#define XMLVM_FORWARD_DECL_java_util_List
XMLVM_FORWARD_DECL(java_util_List)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Map
#define XMLVM_FORWARD_DECL_java_util_Map
XMLVM_FORWARD_DECL(java_util_Map)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
// Class declarations for java.util.Collections$EmptyMap
XMLVM_DEFINE_CLASS(java_util_Collections_EmptyMap, 18, XMLVM_ITABLE_SIZE_java_util_Collections_EmptyMap)

extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptyMap
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Collections_EmptyMap \
    __INSTANCE_FIELDS_java_util_AbstractMap; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptyMap \
    } java_util_Collections_EmptyMap

struct java_util_Collections_EmptyMap {
    __TIB_DEFINITION_java_util_Collections_EmptyMap* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Collections_EmptyMap;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Collections_EmptyMap
#define XMLVM_FORWARD_DECL_java_util_Collections_EmptyMap
typedef struct java_util_Collections_EmptyMap java_util_Collections_EmptyMap;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Collections_EmptyMap 18
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_containsKey___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_containsValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_entrySet__ 9
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_get___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_keySet__ 12
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyMap_values__ 17

void __INIT_java_util_Collections_EmptyMap();
void __INIT_IMPL_java_util_Collections_EmptyMap();
void __DELETE_java_util_Collections_EmptyMap(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Collections_EmptyMap();
JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptyMap();
JAVA_LONG java_util_Collections_EmptyMap_GET_serialVersionUID();
void java_util_Collections_EmptyMap_PUT_serialVersionUID(JAVA_LONG v);
void java_util_Collections_EmptyMap___INIT___(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_Collections_EmptyMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_Collections_EmptyMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_Collections_EmptyMap_entrySet__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_util_Collections_EmptyMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_OBJECT java_util_Collections_EmptyMap_keySet__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_Collections_EmptyMap_values__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Collections_EmptyMap_readResolve__(JAVA_OBJECT me);
void java_util_Collections_EmptyMap___INIT____java_util_Collections_EmptyMap(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
