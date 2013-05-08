#ifndef __JAVA_UTIL_HASHTABLE__
#define __JAVA_UTIL_HASHTABLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Hashtable 14
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
#include "java_util_Map.h"
// Super Class:
#include "java_util_Dictionary.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
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
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable_1
#define XMLVM_FORWARD_DECL_java_util_Hashtable_1
XMLVM_FORWARD_DECL(java_util_Hashtable_1)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable_2
#define XMLVM_FORWARD_DECL_java_util_Hashtable_2
XMLVM_FORWARD_DECL(java_util_Hashtable_2)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable_Entry
#define XMLVM_FORWARD_DECL_java_util_Hashtable_Entry
XMLVM_FORWARD_DECL(java_util_Hashtable_Entry)
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
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.util.Hashtable
XMLVM_DEFINE_CLASS(java_util_Hashtable, 20, XMLVM_ITABLE_SIZE_java_util_Hashtable)

extern JAVA_OBJECT __CLASS_java_util_Hashtable;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Hashtable \
    __INSTANCE_FIELDS_java_util_Dictionary; \
    struct { \
        JAVA_INT elementCount_; \
        JAVA_OBJECT elementData_; \
        JAVA_FLOAT loadFactor_; \
        JAVA_INT threshold_; \
        JAVA_INT firstSlot_; \
        JAVA_INT lastSlot_; \
        JAVA_INT modCount_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable \
    } java_util_Hashtable

struct java_util_Hashtable {
    __TIB_DEFINITION_java_util_Hashtable* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Hashtable;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable
#define XMLVM_FORWARD_DECL_java_util_Hashtable
typedef struct java_util_Hashtable java_util_Hashtable;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Hashtable 20
#define XMLVM_VTABLE_IDX_java_util_Hashtable_clear__ 13
#define XMLVM_VTABLE_IDX_java_util_Hashtable_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_Hashtable_containsKey___java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_Hashtable_containsValue___java_lang_Object 15
#define XMLVM_VTABLE_IDX_java_util_Hashtable_elements__ 6
#define XMLVM_VTABLE_IDX_java_util_Hashtable_entrySet__ 16
#define XMLVM_VTABLE_IDX_java_util_Hashtable_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_Hashtable_get___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_Hashtable_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_Hashtable_isEmpty__ 8
#define XMLVM_VTABLE_IDX_java_util_Hashtable_keys__ 9
#define XMLVM_VTABLE_IDX_java_util_Hashtable_keySet__ 17
#define XMLVM_VTABLE_IDX_java_util_Hashtable_put___java_lang_Object_java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Hashtable_putAll___java_util_Map 18
#define XMLVM_VTABLE_IDX_java_util_Hashtable_remove___java_lang_Object 11
#define XMLVM_VTABLE_IDX_java_util_Hashtable_size__ 12
#define XMLVM_VTABLE_IDX_java_util_Hashtable_toString__ 5
#define XMLVM_VTABLE_IDX_java_util_Hashtable_values__ 19

void __INIT_java_util_Hashtable();
void __INIT_IMPL_java_util_Hashtable();
void __DELETE_java_util_Hashtable(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Hashtable(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Hashtable();
JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable();
JAVA_LONG java_util_Hashtable_GET_serialVersionUID();
void java_util_Hashtable_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_util_Hashtable_GET_EMPTY_ENUMERATION();
void java_util_Hashtable_PUT_EMPTY_ENUMERATION(JAVA_OBJECT v);
JAVA_OBJECT java_util_Hashtable_GET_EMPTY_ITERATOR();
void java_util_Hashtable_PUT_EMPTY_ITERATOR(JAVA_OBJECT v);
void java_util_Hashtable___CLINIT_();
JAVA_OBJECT java_util_Hashtable_newEntry___java_lang_Object_java_lang_Object_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3);
void java_util_Hashtable___INIT___(JAVA_OBJECT me);
void java_util_Hashtable___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_Hashtable___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
void java_util_Hashtable___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Hashtable_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 13
void java_util_Hashtable_clear__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_Hashtable_clone__(JAVA_OBJECT me);
void java_util_Hashtable_computeMaxSize__(JAVA_OBJECT me);
JAVA_BOOLEAN java_util_Hashtable_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 14
JAVA_BOOLEAN java_util_Hashtable_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 15
JAVA_BOOLEAN java_util_Hashtable_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_OBJECT java_util_Hashtable_elements__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_OBJECT java_util_Hashtable_entrySet__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_Hashtable_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_OBJECT java_util_Hashtable_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Hashtable_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_util_Hashtable_hashCode__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_util_Hashtable_isEmpty__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_OBJECT java_util_Hashtable_keys__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_Hashtable_keySet__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_util_Hashtable_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 18
void java_util_Hashtable_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Hashtable_rehash__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_OBJECT java_util_Hashtable_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_INT java_util_Hashtable_size__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_util_Hashtable_toString__(JAVA_OBJECT me);
// Vtable index: 19
JAVA_OBJECT java_util_Hashtable_values__(JAVA_OBJECT me);
void java_util_Hashtable_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Hashtable_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Hashtable_access$0__();

#endif
