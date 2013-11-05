#ifndef __JAVA_UTIL_WEAKHASHMAP_ENTRY__
#define __JAVA_UTIL_WEAKHASHMAP_ENTRY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_WeakHashMap_Entry 5
// Implemented interfaces:
#include "java_util_Map_Entry.h"
// Super Class:
#include "java_lang_ref_WeakReference.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
#define XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
XMLVM_FORWARD_DECL(java_lang_ref_ReferenceQueue)
#endif
// Class declarations for java.util.WeakHashMap$Entry
XMLVM_DEFINE_CLASS(java_util_WeakHashMap_Entry, 9, XMLVM_ITABLE_SIZE_java_util_WeakHashMap_Entry)

extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_Entry;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_Entry_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_Entry_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_WeakHashMap_Entry_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_WeakHashMap_Entry
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_WeakHashMap_Entry \
    __INSTANCE_FIELDS_java_lang_ref_WeakReference; \
    struct { \
        JAVA_INT hash_; \
        JAVA_BOOLEAN isNull_; \
        JAVA_OBJECT value_; \
        JAVA_OBJECT next_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_WeakHashMap_Entry \
    } java_util_WeakHashMap_Entry

struct java_util_WeakHashMap_Entry {
    __TIB_DEFINITION_java_util_WeakHashMap_Entry* tib;
    struct {
        __INSTANCE_FIELDS_java_util_WeakHashMap_Entry;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_WeakHashMap_Entry
#define XMLVM_FORWARD_DECL_java_util_WeakHashMap_Entry
typedef struct java_util_WeakHashMap_Entry java_util_WeakHashMap_Entry;
#endif

#define XMLVM_VTABLE_SIZE_java_util_WeakHashMap_Entry 9
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_getKey__ 6
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_getValue__ 7
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_setValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_WeakHashMap_Entry_toString__ 5

void __INIT_java_util_WeakHashMap_Entry();
void __INIT_IMPL_java_util_WeakHashMap_Entry();
void __DELETE_java_util_WeakHashMap_Entry(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_WeakHashMap_Entry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_WeakHashMap_Entry();
JAVA_OBJECT __NEW_INSTANCE_java_util_WeakHashMap_Entry();
void java_util_WeakHashMap_Entry___INIT____java_lang_Object_java_lang_Object_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
// Vtable index: 6
JAVA_OBJECT java_util_WeakHashMap_Entry_getKey__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_util_WeakHashMap_Entry_getValue__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_OBJECT java_util_WeakHashMap_Entry_setValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 1
JAVA_BOOLEAN java_util_WeakHashMap_Entry_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_util_WeakHashMap_Entry_hashCode__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_util_WeakHashMap_Entry_toString__(JAVA_OBJECT me);

#endif
