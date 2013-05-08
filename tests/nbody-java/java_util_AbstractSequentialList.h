#ifndef __JAVA_UTIL_ABSTRACTSEQUENTIALLIST__
#define __JAVA_UTIL_ABSTRACTSEQUENTIALLIST__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_AbstractSequentialList 78
// Implemented interfaces:
// Super Class:
#include "java_util_AbstractList.h"

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
// Class declarations for java.util.AbstractSequentialList
XMLVM_DEFINE_CLASS(java_util_AbstractSequentialList, 30, XMLVM_ITABLE_SIZE_java_util_AbstractSequentialList)

extern JAVA_OBJECT __CLASS_java_util_AbstractSequentialList;
extern JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractSequentialList
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_AbstractSequentialList \
    __INSTANCE_FIELDS_java_util_AbstractList; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_AbstractSequentialList \
    } java_util_AbstractSequentialList

struct java_util_AbstractSequentialList {
    __TIB_DEFINITION_java_util_AbstractSequentialList* tib;
    struct {
        __INSTANCE_FIELDS_java_util_AbstractSequentialList;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_AbstractSequentialList
#define XMLVM_FORWARD_DECL_java_util_AbstractSequentialList
typedef struct java_util_AbstractSequentialList java_util_AbstractSequentialList;
#endif

#define XMLVM_VTABLE_SIZE_java_util_AbstractSequentialList 30
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_add___int_java_lang_Object 20
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_addAll___int_java_util_Collection 19
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_get___int 21
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_iterator__ 12
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_listIterator___int 25
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_remove___int 27
#define XMLVM_VTABLE_IDX_java_util_AbstractSequentialList_set___int_java_lang_Object 28

void __INIT_java_util_AbstractSequentialList();
void __INIT_IMPL_java_util_AbstractSequentialList();
void __DELETE_java_util_AbstractSequentialList(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_AbstractSequentialList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_AbstractSequentialList();
JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractSequentialList();
void java_util_AbstractSequentialList___INIT___(JAVA_OBJECT me);
// Vtable index: 20
void java_util_AbstractSequentialList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 19
JAVA_BOOLEAN java_util_AbstractSequentialList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 21
JAVA_OBJECT java_util_AbstractSequentialList_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 12
JAVA_OBJECT java_util_AbstractSequentialList_iterator__(JAVA_OBJECT me);
// Vtable index: 25
JAVA_OBJECT java_util_AbstractSequentialList_listIterator___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 27
JAVA_OBJECT java_util_AbstractSequentialList_remove___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 28
JAVA_OBJECT java_util_AbstractSequentialList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);

#endif
