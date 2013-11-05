#ifndef __JAVA_UTIL_COLLECTIONS_SYNCHRONIZEDMAP__
#define __JAVA_UTIL_COLLECTIONS_SYNCHRONIZEDMAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Collections_SynchronizedMap 14
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_util_Map.h"
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
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
// Class declarations for java.util.Collections$SynchronizedMap
XMLVM_DEFINE_CLASS(java_util_Collections_SynchronizedMap, 18, XMLVM_ITABLE_SIZE_java_util_Collections_SynchronizedMap)

extern JAVA_OBJECT __CLASS_java_util_Collections_SynchronizedMap;
extern JAVA_OBJECT __CLASS_java_util_Collections_SynchronizedMap_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_SynchronizedMap_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_SynchronizedMap_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_SynchronizedMap
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Collections_SynchronizedMap \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT m_; \
        JAVA_OBJECT mutex_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_SynchronizedMap \
    } java_util_Collections_SynchronizedMap

struct java_util_Collections_SynchronizedMap {
    __TIB_DEFINITION_java_util_Collections_SynchronizedMap* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Collections_SynchronizedMap;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Collections_SynchronizedMap
#define XMLVM_FORWARD_DECL_java_util_Collections_SynchronizedMap
typedef struct java_util_Collections_SynchronizedMap java_util_Collections_SynchronizedMap;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Collections_SynchronizedMap 18
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_clear__ 6
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_containsKey___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_containsValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_entrySet__ 9
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_get___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_keySet__ 12
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_put___java_lang_Object_java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_putAll___java_util_Map 13
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_remove___java_lang_Object 15
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_size__ 16
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_values__ 17
#define XMLVM_VTABLE_IDX_java_util_Collections_SynchronizedMap_toString__ 5

void __INIT_java_util_Collections_SynchronizedMap();
void __INIT_IMPL_java_util_Collections_SynchronizedMap();
void __DELETE_java_util_Collections_SynchronizedMap(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Collections_SynchronizedMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Collections_SynchronizedMap();
JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_SynchronizedMap();
JAVA_LONG java_util_Collections_SynchronizedMap_GET_serialVersionUID();
void java_util_Collections_SynchronizedMap_PUT_serialVersionUID(JAVA_LONG v);
void java_util_Collections_SynchronizedMap___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Collections_SynchronizedMap___INIT____java_util_Map_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 6
void java_util_Collections_SynchronizedMap_clear__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_BOOLEAN java_util_Collections_SynchronizedMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
JAVA_BOOLEAN java_util_Collections_SynchronizedMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_Collections_SynchronizedMap_entrySet__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_Collections_SynchronizedMap_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 10
JAVA_OBJECT java_util_Collections_SynchronizedMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_util_Collections_SynchronizedMap_hashCode__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_util_Collections_SynchronizedMap_isEmpty__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_util_Collections_SynchronizedMap_keySet__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_util_Collections_SynchronizedMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 13
void java_util_Collections_SynchronizedMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 15
JAVA_OBJECT java_util_Collections_SynchronizedMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_Collections_SynchronizedMap_size__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_Collections_SynchronizedMap_values__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_util_Collections_SynchronizedMap_toString__(JAVA_OBJECT me);
void java_util_Collections_SynchronizedMap_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
