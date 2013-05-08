#ifndef __JAVA_UTIL_LINKEDLIST__
#define __JAVA_UTIL_LINKEDLIST__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_LinkedList 98
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
#include "java_util_Deque.h"
#include "java_util_List.h"
// Super Class:
#include "java_util_AbstractSequentialList.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Array
#define XMLVM_FORWARD_DECL_java_lang_reflect_Array
XMLVM_FORWARD_DECL(java_lang_reflect_Array)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_ArrayList
#define XMLVM_FORWARD_DECL_java_util_ArrayList
XMLVM_FORWARD_DECL(java_util_ArrayList)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_LinkedList_Link
#define XMLVM_FORWARD_DECL_java_util_LinkedList_Link
XMLVM_FORWARD_DECL(java_util_LinkedList_Link)
#endif
// Class declarations for java.util.LinkedList
XMLVM_DEFINE_CLASS(java_util_LinkedList, 52, XMLVM_ITABLE_SIZE_java_util_LinkedList)

extern JAVA_OBJECT __CLASS_java_util_LinkedList;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_LinkedList_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_LinkedList
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_LinkedList \
    __INSTANCE_FIELDS_java_util_AbstractSequentialList; \
    struct { \
        JAVA_INT size_; \
        JAVA_OBJECT voidLink_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_LinkedList \
    } java_util_LinkedList

struct java_util_LinkedList {
    __TIB_DEFINITION_java_util_LinkedList* tib;
    struct {
        __INSTANCE_FIELDS_java_util_LinkedList;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_LinkedList
#define XMLVM_FORWARD_DECL_java_util_LinkedList
typedef struct java_util_LinkedList java_util_LinkedList;
#endif

#define XMLVM_VTABLE_SIZE_java_util_LinkedList 52
#define XMLVM_VTABLE_IDX_java_util_LinkedList_add___int_java_lang_Object 20
#define XMLVM_VTABLE_IDX_java_util_LinkedList_add___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_LinkedList_addAll___int_java_util_Collection 19
#define XMLVM_VTABLE_IDX_java_util_LinkedList_addAll___java_util_Collection 6
#define XMLVM_VTABLE_IDX_java_util_LinkedList_addFirst___java_lang_Object 30
#define XMLVM_VTABLE_IDX_java_util_LinkedList_addLast___java_lang_Object 31
#define XMLVM_VTABLE_IDX_java_util_LinkedList_clear__ 8
#define XMLVM_VTABLE_IDX_java_util_LinkedList_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_LinkedList_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_LinkedList_get___int 21
#define XMLVM_VTABLE_IDX_java_util_LinkedList_getFirst__ 34
#define XMLVM_VTABLE_IDX_java_util_LinkedList_getLast__ 35
#define XMLVM_VTABLE_IDX_java_util_LinkedList_indexOf___java_lang_Object 22
#define XMLVM_VTABLE_IDX_java_util_LinkedList_lastIndexOf___java_lang_Object 23
#define XMLVM_VTABLE_IDX_java_util_LinkedList_listIterator___int 25
#define XMLVM_VTABLE_IDX_java_util_LinkedList_remove___int 27
#define XMLVM_VTABLE_IDX_java_util_LinkedList_remove___java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_LinkedList_removeFirst__ 48
#define XMLVM_VTABLE_IDX_java_util_LinkedList_removeLast__ 50
#define XMLVM_VTABLE_IDX_java_util_LinkedList_descendingIterator__ 32
#define XMLVM_VTABLE_IDX_java_util_LinkedList_offerFirst___java_lang_Object 36
#define XMLVM_VTABLE_IDX_java_util_LinkedList_offerLast___java_lang_Object 37
#define XMLVM_VTABLE_IDX_java_util_LinkedList_peekFirst__ 39
#define XMLVM_VTABLE_IDX_java_util_LinkedList_peekLast__ 40
#define XMLVM_VTABLE_IDX_java_util_LinkedList_pollFirst__ 42
#define XMLVM_VTABLE_IDX_java_util_LinkedList_pollLast__ 43
#define XMLVM_VTABLE_IDX_java_util_LinkedList_pop__ 45
#define XMLVM_VTABLE_IDX_java_util_LinkedList_push___java_lang_Object 46
#define XMLVM_VTABLE_IDX_java_util_LinkedList_removeFirstOccurrence___java_lang_Object 47
#define XMLVM_VTABLE_IDX_java_util_LinkedList_removeLastOccurrence___java_lang_Object 49
#define XMLVM_VTABLE_IDX_java_util_LinkedList_set___int_java_lang_Object 28
#define XMLVM_VTABLE_IDX_java_util_LinkedList_size__ 16
#define XMLVM_VTABLE_IDX_java_util_LinkedList_offer___java_lang_Object 38
#define XMLVM_VTABLE_IDX_java_util_LinkedList_poll__ 44
#define XMLVM_VTABLE_IDX_java_util_LinkedList_remove__ 51
#define XMLVM_VTABLE_IDX_java_util_LinkedList_peek__ 41
#define XMLVM_VTABLE_IDX_java_util_LinkedList_element__ 33
#define XMLVM_VTABLE_IDX_java_util_LinkedList_toArray__ 17
#define XMLVM_VTABLE_IDX_java_util_LinkedList_toArray___java_lang_Object_1ARRAY 18

void __INIT_java_util_LinkedList();
void __INIT_IMPL_java_util_LinkedList();
void __DELETE_java_util_LinkedList(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_LinkedList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_LinkedList();
JAVA_OBJECT __NEW_INSTANCE_java_util_LinkedList();
JAVA_LONG java_util_LinkedList_GET_serialVersionUID();
void java_util_LinkedList_PUT_serialVersionUID(JAVA_LONG v);
void java_util_LinkedList___INIT___(JAVA_OBJECT me);
void java_util_LinkedList___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 20
void java_util_LinkedList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 7
JAVA_BOOLEAN java_util_LinkedList_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_LinkedList_addLastImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 19
JAVA_BOOLEAN java_util_LinkedList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 6
JAVA_BOOLEAN java_util_LinkedList_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 30
void java_util_LinkedList_addFirst___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_LinkedList_addFirstImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 31
void java_util_LinkedList_addLast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
void java_util_LinkedList_clear__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_LinkedList_clone__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_LinkedList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 21
JAVA_OBJECT java_util_LinkedList_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 34
JAVA_OBJECT java_util_LinkedList_getFirst__(JAVA_OBJECT me);
JAVA_OBJECT java_util_LinkedList_getFirstImpl__(JAVA_OBJECT me);
// Vtable index: 35
JAVA_OBJECT java_util_LinkedList_getLast__(JAVA_OBJECT me);
// Vtable index: 22
JAVA_INT java_util_LinkedList_indexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 23
JAVA_INT java_util_LinkedList_lastIndexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 25
JAVA_OBJECT java_util_LinkedList_listIterator___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 27
JAVA_OBJECT java_util_LinkedList_remove___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 14
JAVA_BOOLEAN java_util_LinkedList_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 48
JAVA_OBJECT java_util_LinkedList_removeFirst__(JAVA_OBJECT me);
JAVA_OBJECT java_util_LinkedList_removeFirstImpl__(JAVA_OBJECT me);
// Vtable index: 50
JAVA_OBJECT java_util_LinkedList_removeLast__(JAVA_OBJECT me);
JAVA_OBJECT java_util_LinkedList_removeLastImpl__(JAVA_OBJECT me);
// Vtable index: 32
JAVA_OBJECT java_util_LinkedList_descendingIterator__(JAVA_OBJECT me);
// Vtable index: 36
JAVA_BOOLEAN java_util_LinkedList_offerFirst___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 37
JAVA_BOOLEAN java_util_LinkedList_offerLast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 39
JAVA_OBJECT java_util_LinkedList_peekFirst__(JAVA_OBJECT me);
// Vtable index: 40
JAVA_OBJECT java_util_LinkedList_peekLast__(JAVA_OBJECT me);
// Vtable index: 42
JAVA_OBJECT java_util_LinkedList_pollFirst__(JAVA_OBJECT me);
// Vtable index: 43
JAVA_OBJECT java_util_LinkedList_pollLast__(JAVA_OBJECT me);
// Vtable index: 45
JAVA_OBJECT java_util_LinkedList_pop__(JAVA_OBJECT me);
// Vtable index: 46
void java_util_LinkedList_push___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 47
JAVA_BOOLEAN java_util_LinkedList_removeFirstOccurrence___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 49
JAVA_BOOLEAN java_util_LinkedList_removeLastOccurrence___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_LinkedList_removeFirstOccurrenceImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_LinkedList_removeOneOccurrence___java_lang_Object_java_util_Iterator(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 28
JAVA_OBJECT java_util_LinkedList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 16
JAVA_INT java_util_LinkedList_size__(JAVA_OBJECT me);
// Vtable index: 38
JAVA_BOOLEAN java_util_LinkedList_offer___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 44
JAVA_OBJECT java_util_LinkedList_poll__(JAVA_OBJECT me);
// Vtable index: 51
JAVA_OBJECT java_util_LinkedList_remove__(JAVA_OBJECT me);
// Vtable index: 41
JAVA_OBJECT java_util_LinkedList_peek__(JAVA_OBJECT me);
JAVA_OBJECT java_util_LinkedList_peekFirstImpl__(JAVA_OBJECT me);
// Vtable index: 33
JAVA_OBJECT java_util_LinkedList_element__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_LinkedList_toArray__(JAVA_OBJECT me);
// Vtable index: 18
JAVA_OBJECT java_util_LinkedList_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_LinkedList_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_LinkedList_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
