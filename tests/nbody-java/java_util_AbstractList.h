#ifndef __JAVA_UTIL_ABSTRACTLIST__
#define __JAVA_UTIL_ABSTRACTLIST__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_AbstractList 78
// Implemented interfaces:
#include "java_util_List.h"
// Super Class:
#include "java_util_AbstractCollection.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_RandomAccess
#define XMLVM_FORWARD_DECL_java_util_RandomAccess
XMLVM_FORWARD_DECL(java_util_RandomAccess)
#endif
// Class declarations for java.util.AbstractList
XMLVM_DEFINE_CLASS(java_util_AbstractList, 30, XMLVM_ITABLE_SIZE_java_util_AbstractList)

extern JAVA_OBJECT __CLASS_java_util_AbstractList;
extern JAVA_OBJECT __CLASS_java_util_AbstractList_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractList_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractList_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractList
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_AbstractList \
    __INSTANCE_FIELDS_java_util_AbstractCollection; \
    struct { \
        JAVA_INT modCount_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractList \
    } java_util_AbstractList

struct java_util_AbstractList {
    __TIB_DEFINITION_java_util_AbstractList* tib;
    struct {
        __INSTANCE_FIELDS_java_util_AbstractList;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_AbstractList
#define XMLVM_FORWARD_DECL_java_util_AbstractList
typedef struct java_util_AbstractList java_util_AbstractList;
#endif

#define XMLVM_VTABLE_SIZE_java_util_AbstractList 30
#define XMLVM_VTABLE_IDX_java_util_AbstractList_add___int_java_lang_Object 20
#define XMLVM_VTABLE_IDX_java_util_AbstractList_add___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_AbstractList_addAll___int_java_util_Collection 19
#define XMLVM_VTABLE_IDX_java_util_AbstractList_clear__ 8
#define XMLVM_VTABLE_IDX_java_util_AbstractList_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_AbstractList_get___int 21
#define XMLVM_VTABLE_IDX_java_util_AbstractList_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_AbstractList_indexOf___java_lang_Object 22
#define XMLVM_VTABLE_IDX_java_util_AbstractList_iterator__ 12
#define XMLVM_VTABLE_IDX_java_util_AbstractList_lastIndexOf___java_lang_Object 23
#define XMLVM_VTABLE_IDX_java_util_AbstractList_listIterator__ 24
#define XMLVM_VTABLE_IDX_java_util_AbstractList_listIterator___int 25
#define XMLVM_VTABLE_IDX_java_util_AbstractList_remove___int 27
#define XMLVM_VTABLE_IDX_java_util_AbstractList_removeRange___int_int 26
#define XMLVM_VTABLE_IDX_java_util_AbstractList_set___int_java_lang_Object 28
#define XMLVM_VTABLE_IDX_java_util_AbstractList_subList___int_int 29

void __INIT_java_util_AbstractList();
void __INIT_IMPL_java_util_AbstractList();
void __DELETE_java_util_AbstractList(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_AbstractList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_AbstractList();
JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractList();
void java_util_AbstractList___INIT___(JAVA_OBJECT me);
// Vtable index: 20
void java_util_AbstractList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 7
JAVA_BOOLEAN java_util_AbstractList_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 19
JAVA_BOOLEAN java_util_AbstractList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 8
void java_util_AbstractList_clear__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_AbstractList_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 21
JAVA_OBJECT java_util_AbstractList_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 4
JAVA_INT java_util_AbstractList_hashCode__(JAVA_OBJECT me);
// Vtable index: 22
JAVA_INT java_util_AbstractList_indexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_OBJECT java_util_AbstractList_iterator__(JAVA_OBJECT me);
// Vtable index: 23
JAVA_INT java_util_AbstractList_lastIndexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 24
JAVA_OBJECT java_util_AbstractList_listIterator__(JAVA_OBJECT me);
// Vtable index: 25
JAVA_OBJECT java_util_AbstractList_listIterator___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 27
JAVA_OBJECT java_util_AbstractList_remove___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 26
void java_util_AbstractList_removeRange___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 28
JAVA_OBJECT java_util_AbstractList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 29
JAVA_OBJECT java_util_AbstractList_subList___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);

#endif
