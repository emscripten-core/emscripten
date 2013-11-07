#ifndef __JAVA_UTIL_MAP__
#define __JAVA_UTIL_MAP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_util_Map_clear__ 0
#define XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object 1
#define XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object 2
#define XMLVM_ITABLE_IDX_java_util_Map_entrySet__ 3
#define XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object 4
#define XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object 5
#define XMLVM_ITABLE_IDX_java_util_Map_hashCode__ 6
#define XMLVM_ITABLE_IDX_java_util_Map_isEmpty__ 7
#define XMLVM_ITABLE_IDX_java_util_Map_keySet__ 8
#define XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object 10
#define XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map 9
#define XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object 11
#define XMLVM_ITABLE_IDX_java_util_Map_size__ 12
#define XMLVM_ITABLE_IDX_java_util_Map_values__ 13
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif

XMLVM_DEFINE_CLASS(java_util_Map, 0, 0)

extern JAVA_OBJECT __CLASS_java_util_Map;
extern JAVA_OBJECT __CLASS_java_util_Map_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Map_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Map_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_util_Map
#define XMLVM_FORWARD_DECL_java_util_Map
typedef struct java_util_Map java_util_Map;
#endif

void __INIT_java_util_Map();
void __INIT_IMPL_java_util_Map();

#endif
