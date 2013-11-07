#ifndef __JAVA_UTIL_COLLECTIONS_EMPTYLIST__
#define __JAVA_UTIL_COLLECTIONS_EMPTYLIST__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Collections_EmptyList 78
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_util_RandomAccess.h"
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
#ifndef XMLVM_FORWARD_DECL_java_util_Collections
#define XMLVM_FORWARD_DECL_java_util_Collections
XMLVM_FORWARD_DECL(java_util_Collections)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_List
#define XMLVM_FORWARD_DECL_java_util_List
XMLVM_FORWARD_DECL(java_util_List)
#endif
// Class declarations for java.util.Collections$EmptyList
XMLVM_DEFINE_CLASS(java_util_Collections_EmptyList, 30, XMLVM_ITABLE_SIZE_java_util_Collections_EmptyList)

extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyList;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptyList
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Collections_EmptyList \
    __INSTANCE_FIELDS_java_util_AbstractList; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Collections_EmptyList \
    } java_util_Collections_EmptyList

struct java_util_Collections_EmptyList {
    __TIB_DEFINITION_java_util_Collections_EmptyList* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Collections_EmptyList;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Collections_EmptyList
#define XMLVM_FORWARD_DECL_java_util_Collections_EmptyList
typedef struct java_util_Collections_EmptyList java_util_Collections_EmptyList;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Collections_EmptyList 30
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyList_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyList_size__ 16
#define XMLVM_VTABLE_IDX_java_util_Collections_EmptyList_get___int 21

void __INIT_java_util_Collections_EmptyList();
void __INIT_IMPL_java_util_Collections_EmptyList();
void __DELETE_java_util_Collections_EmptyList(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Collections_EmptyList();
JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptyList();
JAVA_LONG java_util_Collections_EmptyList_GET_serialVersionUID();
void java_util_Collections_EmptyList_PUT_serialVersionUID(JAVA_LONG v);
void java_util_Collections_EmptyList___INIT___(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_Collections_EmptyList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 16
JAVA_INT java_util_Collections_EmptyList_size__(JAVA_OBJECT me);
// Vtable index: 21
JAVA_OBJECT java_util_Collections_EmptyList_get___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_util_Collections_EmptyList_readResolve__(JAVA_OBJECT me);
void java_util_Collections_EmptyList___INIT____java_util_Collections_EmptyList(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
