#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_lang_reflect_Array.h"
#include "java_util_ArrayList.h"
#include "java_util_Collection.h"
#include "java_util_Iterator.h"
#include "java_util_LinkedList_Link.h"

#include "java_util_LinkedList.h"

#define XMLVM_CURRENT_CLASS_NAME LinkedList
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_LinkedList

__TIB_DEFINITION_java_util_LinkedList __TIB_java_util_LinkedList = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_LinkedList, // classInitializer
    "java.util.LinkedList", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/util/AbstractSequentialList<TE;>;Ljava/util/List<TE;>;Ljava/util/Deque<TE;>;Ljava/lang/Cloneable;Ljava/io/Serializable;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractSequentialList, // extends
    sizeof(java_util_LinkedList), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_LinkedList;
JAVA_OBJECT __CLASS_java_util_LinkedList_1ARRAY;
JAVA_OBJECT __CLASS_java_util_LinkedList_2ARRAY;
JAVA_OBJECT __CLASS_java_util_LinkedList_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_LinkedList_serialVersionUID;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_java_util_LinkedList()
{
    staticInitializerLock(&__TIB_java_util_LinkedList);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_LinkedList.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_LinkedList.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_LinkedList);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_LinkedList.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_LinkedList.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_LinkedList.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.LinkedList")
        __INIT_IMPL_java_util_LinkedList();
    }
}

void __INIT_IMPL_java_util_LinkedList()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractSequentialList)
    __TIB_java_util_LinkedList.newInstanceFunc = __NEW_INSTANCE_java_util_LinkedList;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_LinkedList.vtable, __TIB_java_util_AbstractSequentialList.vtable, sizeof(__TIB_java_util_AbstractSequentialList.vtable));
    // Initialize vtable for this class
    __TIB_java_util_LinkedList.vtable[20] = (VTABLE_PTR) &java_util_LinkedList_add___int_java_lang_Object;
    __TIB_java_util_LinkedList.vtable[7] = (VTABLE_PTR) &java_util_LinkedList_add___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[19] = (VTABLE_PTR) &java_util_LinkedList_addAll___int_java_util_Collection;
    __TIB_java_util_LinkedList.vtable[6] = (VTABLE_PTR) &java_util_LinkedList_addAll___java_util_Collection;
    __TIB_java_util_LinkedList.vtable[30] = (VTABLE_PTR) &java_util_LinkedList_addFirst___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[31] = (VTABLE_PTR) &java_util_LinkedList_addLast___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[8] = (VTABLE_PTR) &java_util_LinkedList_clear__;
    __TIB_java_util_LinkedList.vtable[0] = (VTABLE_PTR) &java_util_LinkedList_clone__;
    __TIB_java_util_LinkedList.vtable[10] = (VTABLE_PTR) &java_util_LinkedList_contains___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[21] = (VTABLE_PTR) &java_util_LinkedList_get___int;
    __TIB_java_util_LinkedList.vtable[34] = (VTABLE_PTR) &java_util_LinkedList_getFirst__;
    __TIB_java_util_LinkedList.vtable[35] = (VTABLE_PTR) &java_util_LinkedList_getLast__;
    __TIB_java_util_LinkedList.vtable[22] = (VTABLE_PTR) &java_util_LinkedList_indexOf___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[23] = (VTABLE_PTR) &java_util_LinkedList_lastIndexOf___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[25] = (VTABLE_PTR) &java_util_LinkedList_listIterator___int;
    __TIB_java_util_LinkedList.vtable[27] = (VTABLE_PTR) &java_util_LinkedList_remove___int;
    __TIB_java_util_LinkedList.vtable[14] = (VTABLE_PTR) &java_util_LinkedList_remove___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[48] = (VTABLE_PTR) &java_util_LinkedList_removeFirst__;
    __TIB_java_util_LinkedList.vtable[50] = (VTABLE_PTR) &java_util_LinkedList_removeLast__;
    __TIB_java_util_LinkedList.vtable[32] = (VTABLE_PTR) &java_util_LinkedList_descendingIterator__;
    __TIB_java_util_LinkedList.vtable[36] = (VTABLE_PTR) &java_util_LinkedList_offerFirst___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[37] = (VTABLE_PTR) &java_util_LinkedList_offerLast___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[39] = (VTABLE_PTR) &java_util_LinkedList_peekFirst__;
    __TIB_java_util_LinkedList.vtable[40] = (VTABLE_PTR) &java_util_LinkedList_peekLast__;
    __TIB_java_util_LinkedList.vtable[42] = (VTABLE_PTR) &java_util_LinkedList_pollFirst__;
    __TIB_java_util_LinkedList.vtable[43] = (VTABLE_PTR) &java_util_LinkedList_pollLast__;
    __TIB_java_util_LinkedList.vtable[45] = (VTABLE_PTR) &java_util_LinkedList_pop__;
    __TIB_java_util_LinkedList.vtable[46] = (VTABLE_PTR) &java_util_LinkedList_push___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[47] = (VTABLE_PTR) &java_util_LinkedList_removeFirstOccurrence___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[49] = (VTABLE_PTR) &java_util_LinkedList_removeLastOccurrence___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[28] = (VTABLE_PTR) &java_util_LinkedList_set___int_java_lang_Object;
    __TIB_java_util_LinkedList.vtable[16] = (VTABLE_PTR) &java_util_LinkedList_size__;
    __TIB_java_util_LinkedList.vtable[38] = (VTABLE_PTR) &java_util_LinkedList_offer___java_lang_Object;
    __TIB_java_util_LinkedList.vtable[44] = (VTABLE_PTR) &java_util_LinkedList_poll__;
    __TIB_java_util_LinkedList.vtable[51] = (VTABLE_PTR) &java_util_LinkedList_remove__;
    __TIB_java_util_LinkedList.vtable[41] = (VTABLE_PTR) &java_util_LinkedList_peek__;
    __TIB_java_util_LinkedList.vtable[33] = (VTABLE_PTR) &java_util_LinkedList_element__;
    __TIB_java_util_LinkedList.vtable[17] = (VTABLE_PTR) &java_util_LinkedList_toArray__;
    __TIB_java_util_LinkedList.vtable[18] = (VTABLE_PTR) &java_util_LinkedList_toArray___java_lang_Object_1ARRAY;
    // Initialize interface information
    __TIB_java_util_LinkedList.numImplementedInterfaces = 7;
    __TIB_java_util_LinkedList.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 7);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_LinkedList.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_LinkedList.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_LinkedList.implementedInterfaces[0][2] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_LinkedList.implementedInterfaces[0][3] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_Deque)

    __TIB_java_util_LinkedList.implementedInterfaces[0][4] = &__TIB_java_util_Deque;

    XMLVM_CLASS_INIT(java_util_List)

    __TIB_java_util_LinkedList.implementedInterfaces[0][5] = &__TIB_java_util_List;

    XMLVM_CLASS_INIT(java_util_Queue)

    __TIB_java_util_LinkedList.implementedInterfaces[0][6] = &__TIB_java_util_Queue;
    // Initialize itable for this class
    __TIB_java_util_LinkedList.itableBegin = &__TIB_java_util_LinkedList.itable[0];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_LinkedList.vtable[12];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[6];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_LinkedList.vtable[7];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_LinkedList.vtable[8];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[9];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_LinkedList.vtable[10];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_LinkedList.vtable[1];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_LinkedList.vtable[4];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_LinkedList.vtable[11];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_LinkedList.vtable[12];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[13];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_LinkedList.vtable[14];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[15];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_LinkedList.vtable[16];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_LinkedList.vtable[17];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_LinkedList.vtable[18];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_addAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[6];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_addFirst___java_lang_Object] = __TIB_java_util_LinkedList.vtable[30];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_addLast___java_lang_Object] = __TIB_java_util_LinkedList.vtable[31];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_add___java_lang_Object] = __TIB_java_util_LinkedList.vtable[7];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_clear__] = __TIB_java_util_LinkedList.vtable[8];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_containsAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[9];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_contains___java_lang_Object] = __TIB_java_util_LinkedList.vtable[10];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_descendingIterator__] = __TIB_java_util_LinkedList.vtable[32];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_element__] = __TIB_java_util_LinkedList.vtable[33];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_equals___java_lang_Object] = __TIB_java_util_LinkedList.vtable[1];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_getFirst__] = __TIB_java_util_LinkedList.vtable[34];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_getLast__] = __TIB_java_util_LinkedList.vtable[35];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_hashCode__] = __TIB_java_util_LinkedList.vtable[4];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_isEmpty__] = __TIB_java_util_LinkedList.vtable[11];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_iterator__] = __TIB_java_util_LinkedList.vtable[12];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_offerFirst___java_lang_Object] = __TIB_java_util_LinkedList.vtable[36];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_offerLast___java_lang_Object] = __TIB_java_util_LinkedList.vtable[37];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_offer___java_lang_Object] = __TIB_java_util_LinkedList.vtable[38];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_peekFirst__] = __TIB_java_util_LinkedList.vtable[39];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_peekLast__] = __TIB_java_util_LinkedList.vtable[40];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_peek__] = __TIB_java_util_LinkedList.vtable[41];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_pollFirst__] = __TIB_java_util_LinkedList.vtable[42];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_pollLast__] = __TIB_java_util_LinkedList.vtable[43];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_poll__] = __TIB_java_util_LinkedList.vtable[44];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_pop__] = __TIB_java_util_LinkedList.vtable[45];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_push___java_lang_Object] = __TIB_java_util_LinkedList.vtable[46];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_removeAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[13];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_removeFirstOccurrence___java_lang_Object] = __TIB_java_util_LinkedList.vtable[47];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_removeFirst__] = __TIB_java_util_LinkedList.vtable[48];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_removeLastOccurrence___java_lang_Object] = __TIB_java_util_LinkedList.vtable[49];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_removeLast__] = __TIB_java_util_LinkedList.vtable[50];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_remove__] = __TIB_java_util_LinkedList.vtable[51];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_remove___java_lang_Object] = __TIB_java_util_LinkedList.vtable[14];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_retainAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[15];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_size__] = __TIB_java_util_LinkedList.vtable[16];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_toArray__] = __TIB_java_util_LinkedList.vtable[17];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Deque_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_LinkedList.vtable[18];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___int_java_util_Collection] = __TIB_java_util_LinkedList.vtable[19];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[6];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_add___int_java_lang_Object] = __TIB_java_util_LinkedList.vtable[20];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_add___java_lang_Object] = __TIB_java_util_LinkedList.vtable[7];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_clear__] = __TIB_java_util_LinkedList.vtable[8];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_containsAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[9];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_contains___java_lang_Object] = __TIB_java_util_LinkedList.vtable[10];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_equals___java_lang_Object] = __TIB_java_util_LinkedList.vtable[1];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_get___int] = __TIB_java_util_LinkedList.vtable[21];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_hashCode__] = __TIB_java_util_LinkedList.vtable[4];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object] = __TIB_java_util_LinkedList.vtable[22];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_isEmpty__] = __TIB_java_util_LinkedList.vtable[11];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_iterator__] = __TIB_java_util_LinkedList.vtable[12];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_lastIndexOf___java_lang_Object] = __TIB_java_util_LinkedList.vtable[23];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator__] = __TIB_java_util_LinkedList.vtable[24];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator___int] = __TIB_java_util_LinkedList.vtable[25];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_removeAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[13];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___int] = __TIB_java_util_LinkedList.vtable[27];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___java_lang_Object] = __TIB_java_util_LinkedList.vtable[14];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_retainAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[15];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object] = __TIB_java_util_LinkedList.vtable[28];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_size__] = __TIB_java_util_LinkedList.vtable[16];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_subList___int_int] = __TIB_java_util_LinkedList.vtable[29];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray__] = __TIB_java_util_LinkedList.vtable[17];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_LinkedList.vtable[18];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_addAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[6];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_add___java_lang_Object] = __TIB_java_util_LinkedList.vtable[7];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_clear__] = __TIB_java_util_LinkedList.vtable[8];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_containsAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[9];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_contains___java_lang_Object] = __TIB_java_util_LinkedList.vtable[10];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_element__] = __TIB_java_util_LinkedList.vtable[33];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_equals___java_lang_Object] = __TIB_java_util_LinkedList.vtable[1];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_hashCode__] = __TIB_java_util_LinkedList.vtable[4];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_isEmpty__] = __TIB_java_util_LinkedList.vtable[11];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_iterator__] = __TIB_java_util_LinkedList.vtable[12];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_offer___java_lang_Object] = __TIB_java_util_LinkedList.vtable[38];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_peek__] = __TIB_java_util_LinkedList.vtable[41];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_poll__] = __TIB_java_util_LinkedList.vtable[44];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_removeAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[13];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_remove__] = __TIB_java_util_LinkedList.vtable[51];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_remove___java_lang_Object] = __TIB_java_util_LinkedList.vtable[14];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_retainAll___java_util_Collection] = __TIB_java_util_LinkedList.vtable[15];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_size__] = __TIB_java_util_LinkedList.vtable[16];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_toArray__] = __TIB_java_util_LinkedList.vtable[17];
    __TIB_java_util_LinkedList.itable[XMLVM_ITABLE_IDX_java_util_Queue_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_LinkedList.vtable[18];

    _STATIC_java_util_LinkedList_serialVersionUID = 876323262645176354;

    __TIB_java_util_LinkedList.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_LinkedList.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_LinkedList.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_LinkedList.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_LinkedList.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_LinkedList.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_LinkedList.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_LinkedList.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_LinkedList = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_LinkedList);
    __TIB_java_util_LinkedList.clazz = __CLASS_java_util_LinkedList;
    __TIB_java_util_LinkedList.baseType = JAVA_NULL;
    __CLASS_java_util_LinkedList_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList);
    __CLASS_java_util_LinkedList_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList_1ARRAY);
    __CLASS_java_util_LinkedList_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_LinkedList]
    //XMLVM_END_WRAPPER

    __TIB_java_util_LinkedList.classInitialized = 1;
}

void __DELETE_java_util_LinkedList(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_LinkedList]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_LinkedList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractSequentialList(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_LinkedList*) me)->fields.java_util_LinkedList.size_ = 0;
    ((java_util_LinkedList*) me)->fields.java_util_LinkedList.voidLink_ = (java_util_LinkedList_Link*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_LinkedList]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_LinkedList()
{    XMLVM_CLASS_INIT(java_util_LinkedList)
java_util_LinkedList* me = (java_util_LinkedList*) XMLVM_MALLOC(sizeof(java_util_LinkedList));
    me->tib = &__TIB_java_util_LinkedList;
    __INIT_INSTANCE_MEMBERS_java_util_LinkedList(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_LinkedList]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_LinkedList()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_LinkedList();
    java_util_LinkedList___INIT___(me);
    return me;
}

JAVA_LONG java_util_LinkedList_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_LinkedList)
    return _STATIC_java_util_LinkedList_serialVersionUID;
}

void java_util_LinkedList_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_LinkedList)
_STATIC_java_util_LinkedList_serialVersionUID = v;
}

void java_util_LinkedList___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList___INIT___]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("LinkedList.java", 239)
    XMLVM_CHECK_NPE(2)
    java_util_AbstractSequentialList___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 39)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 240)
    _r0.o = __NEW_java_util_LinkedList_Link();
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r0.o, _r1.o, _r1.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_ = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 241)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 242)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 243)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList___INIT____java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 255)
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 256)
    //java_util_LinkedList_addAll___java_util_Collection[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_LinkedList*) _r0.o)->tib->vtable[6])(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 257)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_add___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.o = n2;
    XMLVM_SOURCE_POSITION("LinkedList.java", 274)
    if (_r5.i < 0) goto label64;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    if (_r5.i > _r0.i) goto label64;
    XMLVM_SOURCE_POSITION("LinkedList.java", 275)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 276)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r1.i = _r1.i / 2;
    if (_r5.i >= _r1.i) goto label50;
    XMLVM_SOURCE_POSITION("LinkedList.java", 277)
    _r1.i = 0;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label18:;
    if (_r0.i <= _r5.i) goto label45;
    _r0 = _r1;
    label21:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 285)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 286)
    _r2.o = __NEW_java_util_LinkedList_Link();
    XMLVM_CHECK_NPE(2)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r2.o, _r6.o, _r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 287)
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 288)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 289)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 290)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 294)
    XMLVM_EXIT_METHOD()
    return;
    label45:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 278)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    _r0.i = _r0.i + 1;
    goto label18;
    label50:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 281)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label55:;
    if (_r0.i > _r5.i) goto label59;
    _r0 = _r1;
    goto label21;
    label59:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 282)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_;
    _r0.i = _r0.i + -1;
    goto label55;
    label64:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 292)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_add___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 305)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_addLastImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_addLastImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addLastImpl___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addLastImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 309)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 310)
    _r1.o = __NEW_java_util_LinkedList_Link();
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r1.o, _r4.o, _r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 311)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 312)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 313)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 314)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 315)
    _r0.i = 1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addAll___int_java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    _r8.o = me;
    _r9.i = n1;
    _r10.o = n2;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("LinkedList.java", 338)
    if (_r9.i < 0) goto label7;
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.size_;
    if (_r9.i <= _r0.i) goto label13;
    label7:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 339)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label13:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 341)
    XMLVM_CHECK_NPE(10)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r10.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r10.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 342)
    if (_r1.i != 0) goto label21;
    _r0 = _r4;
    label20:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 343)
    XMLVM_SOURCE_POSITION("LinkedList.java", 368)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label21:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 345)
    if (_r10.o != _r8.o) goto label70;
    XMLVM_SOURCE_POSITION("LinkedList.java", 346)
    _r0.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(0)
    java_util_ArrayList___INIT____java_util_Collection(_r0.o, _r10.o);
    label28:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 348)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 349)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.size_;
    _r3.i = _r3.i / 2;
    if (_r9.i >= _r3.i) goto label77;
    _r3 = _r2;
    _r2 = _r4;
    label38:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 350)
    if (_r2.i < _r9.i) goto label72;
    _r2 = _r3;
    label41:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 358)
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 359)
    XMLVM_CHECK_NPE(0)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r0.o);
    label47:;
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r4.o);
    if (_r0.i != 0) goto label91;
    XMLVM_SOURCE_POSITION("LinkedList.java", 364)
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r3.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 365)
    XMLVM_CHECK_NPE(3)
    ((java_util_LinkedList_Link*) _r3.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 366)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 367)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    _r0.i = 1;
    goto label20;
    label70:;
    _r0 = _r10;
    goto label28;
    label72:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 351)
    XMLVM_CHECK_NPE(3)
    _r3.o = ((java_util_LinkedList_Link*) _r3.o)->fields.java_util_LinkedList_Link.next_;
    _r2.i = _r2.i + 1;
    goto label38;
    label77:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 354)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_LinkedList*) _r8.o)->fields.java_util_LinkedList.size_;
    _r7 = _r3;
    _r3 = _r2;
    _r2 = _r7;
    label82:;
    if (_r2.i >= _r9.i) goto label86;
    _r2 = _r3;
    goto label41;
    label86:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 355)
    XMLVM_CHECK_NPE(3)
    _r3.o = ((java_util_LinkedList_Link*) _r3.o)->fields.java_util_LinkedList_Link.previous_;
    _r2.i = _r2.i + -1;
    goto label82;
    label91:;
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r4.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 360)
    _r5.o = __NEW_java_util_LinkedList_Link();
    _r6.o = JAVA_NULL;
    XMLVM_CHECK_NPE(5)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r5.o, _r0.o, _r2.o, _r6.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 361)
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r5.o;
    _r2 = _r5;
    XMLVM_SOURCE_POSITION("LinkedList.java", 362)
    goto label47;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 381)
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r7.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 382)
    if (_r1.i != 0) goto label8;
    XMLVM_SOURCE_POSITION("LinkedList.java", 383)
    _r0.i = 0;
    label7:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 398)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 385)
    if (_r7.o != _r6.o) goto label50;
    XMLVM_SOURCE_POSITION("LinkedList.java", 386)
    _r0.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(0)
    java_util_ArrayList___INIT____java_util_Collection(_r0.o, _r7.o);
    label15:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 388)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 389)
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r0.o);
    label23:;
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r3.o);
    if (_r0.i != 0) goto label52;
    XMLVM_SOURCE_POSITION("LinkedList.java", 394)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 395)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 396)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(6)
    ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 397)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    _r0.i = 1;
    goto label7;
    label50:;
    _r0 = _r7;
    goto label15;
    label52:;
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r3.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 390)
    _r4.o = __NEW_java_util_LinkedList_Link();
    _r5.o = JAVA_NULL;
    XMLVM_CHECK_NPE(4)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r4.o, _r0.o, _r2.o, _r5.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 391)
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r4.o;
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("LinkedList.java", 392)
    goto label23;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_addFirst___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addFirst___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 408)
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList_addFirstImpl___java_lang_Object(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 409)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_addFirstImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addFirstImpl___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addFirstImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 412)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 413)
    _r1.o = __NEW_java_util_LinkedList_Link();
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r1.o, _r4.o, _r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 414)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 415)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 416)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 417)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 418)
    _r0.i = 1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_addLast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_addLast___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "addLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 428)
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList_addLastImpl___java_lang_Object(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 429)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_clear__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 439)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.size_;
    if (_r0.i <= 0) goto label25;
    XMLVM_SOURCE_POSITION("LinkedList.java", 440)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 441)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 442)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 443)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_AbstractList*) _r2.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    ((java_util_AbstractList*) _r2.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    label25:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 445)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_clone__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    _r5.o = JAVA_NULL;
    XMLVM_TRY_BEGIN(w921aaac14b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("LinkedList.java", 458)
    XMLVM_CHECK_NPE(6)
    _r0.o = java_lang_Object_clone__(_r6.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 459)
    _r1.i = 0;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 460)
    _r1.o = __NEW_java_util_LinkedList_Link();
    _r2.o = JAVA_NULL;
    _r3.o = JAVA_NULL;
    _r4.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r1.o, _r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.voidLink_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 461)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 462)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList*) _r0.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 463)
    //java_util_LinkedList_addAll___java_util_Collection[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_LinkedList*) _r0.o)->tib->vtable[6])(_r0.o, _r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w921aaac14b1b3)
    XMLVM_CATCH_END(w921aaac14b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w921aaac14b1b3)
    label35:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 466)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label36:;
    java_lang_Thread* curThread_w921aaac14b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w921aaac14b1b8->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r5;
    goto label35;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 480)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 481)
    if (_r4.o == JAVA_NULL) goto label34;
    label7:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 482)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r1.o) goto label13;
    label11:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 496)
    _r0.i = 0;
    label12:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label13:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 483)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i == 0) goto label23;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("LinkedList.java", 484)
    goto label12;
    label23:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 486)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    goto label7;
    label26:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 490)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    if (_r1.o != JAVA_NULL) goto label32;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("LinkedList.java", 491)
    goto label12;
    label32:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 493)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    label34:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 489)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r1.o) goto label26;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_get___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_get___int]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 501)
    if (_r4.i < 0) goto label43;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    if (_r4.i >= _r0.i) goto label43;
    XMLVM_SOURCE_POSITION("LinkedList.java", 502)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 503)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r1.i = _r1.i / 2;
    if (_r4.i >= _r1.i) goto label29;
    XMLVM_SOURCE_POSITION("LinkedList.java", 504)
    _r1.i = 0;
    _r2 = _r1;
    _r1 = _r0;
    _r0 = _r2;
    label18:;
    if (_r0.i <= _r4.i) goto label24;
    _r0 = _r1;
    label21:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 512)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label24:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 505)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    _r0.i = _r0.i + 1;
    goto label18;
    label29:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 508)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r2 = _r1;
    _r1 = _r0;
    _r0 = _r2;
    label34:;
    if (_r0.i > _r4.i) goto label38;
    _r0 = _r1;
    goto label21;
    label38:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 509)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_;
    _r0.i = _r0.i + -1;
    goto label34;
    label43:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 514)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_getFirst__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_getFirst__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "getFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 525)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_getFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_getFirstImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_getFirstImpl__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "getFirstImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 529)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 530)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o == _r1.o) goto label11;
    XMLVM_SOURCE_POSITION("LinkedList.java", 531)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label11:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 533)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_getLast__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_getLast__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "getLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 544)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 545)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o == _r1.o) goto label11;
    XMLVM_SOURCE_POSITION("LinkedList.java", 546)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label11:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 548)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_LinkedList_indexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_indexOf___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 553)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("LinkedList.java", 554)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 555)
    if (_r5.o == JAVA_NULL) goto label46;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label10:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 556)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r2.o) goto label16;
    label14:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 572)
    _r0.i = -1;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 557)
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r2.i == 0) goto label26;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 558)
    goto label15;
    label26:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 560)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 561)
    _r1.i = _r1.i + 1;
    goto label10;
    label31:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 565)
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    if (_r2.o != JAVA_NULL) goto label37;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 566)
    goto label15;
    label37:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 568)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 569)
    _r1.i = _r1.i + 1;
    label41:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 564)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r2.o) goto label31;
    goto label14;
    label46:;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    goto label41;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_LinkedList_lastIndexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_lastIndexOf___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 586)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 587)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 588)
    if (_r5.o == JAVA_NULL) goto label47;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label11:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 589)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r2.o) goto label17;
    label15:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 605)
    _r0.i = -1;
    label16:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label17:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 590)
    _r1.i = _r1.i + -1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 591)
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r2.i == 0) goto label29;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 592)
    goto label16;
    label29:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 594)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    goto label11;
    label32:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 598)
    _r1.i = _r1.i + -1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 599)
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    if (_r2.o != JAVA_NULL) goto label40;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 600)
    goto label16;
    label40:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 602)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    label42:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 597)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r2.o) goto label32;
    goto label15;
    label47:;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    goto label42;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_listIterator___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_listIterator___int]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "listIterator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 622)

    
    // Red class access removed: java.util.LinkedList$LinkIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.LinkedList$LinkIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_remove___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_remove___int]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 636)
    if (_r5.i < 0) goto label63;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    if (_r5.i >= _r0.i) goto label63;
    XMLVM_SOURCE_POSITION("LinkedList.java", 637)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 638)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r1.i = _r1.i / 2;
    if (_r5.i >= _r1.i) goto label49;
    XMLVM_SOURCE_POSITION("LinkedList.java", 639)
    _r1.i = 0;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label18:;
    if (_r0.i <= _r5.i) goto label44;
    _r0 = _r1;
    label21:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 647)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 648)
    XMLVM_CHECK_NPE(0)
    _r2.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 649)
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 650)
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 651)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(4)
    ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 652)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 653)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label44:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 640)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    _r0.i = _r0.i + 1;
    goto label18;
    label49:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 643)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_LinkedList*) _r4.o)->fields.java_util_LinkedList.size_;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label54:;
    if (_r0.i > _r5.i) goto label58;
    _r0 = _r1;
    goto label21;
    label58:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 644)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_;
    _r0.i = _r0.i + -1;
    goto label54;
    label63:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 655)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 660)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_removeFirstOccurrenceImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_removeFirst__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeFirst__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 671)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_removeFirstImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeFirstImpl__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeFirstImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 675)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 676)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o == _r1.o) goto label33;
    XMLVM_SOURCE_POSITION("LinkedList.java", 677)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 678)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 679)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 680)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(3)
    ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 681)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 682)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label33:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 684)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_removeLast__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeLast__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 695)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeLastImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_removeLastImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeLastImpl__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeLastImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 699)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 700)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o == _r1.o) goto label33;
    XMLVM_SOURCE_POSITION("LinkedList.java", 701)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 702)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 703)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 704)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(3)
    ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 705)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_AbstractList*) _r3.o)->fields.java_util_AbstractList.modCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 706)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label33:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 708)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_descendingIterator__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_descendingIterator__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "descendingIterator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 718)

    
    // Red class access removed: java.util.LinkedList$ReverseLinkIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.LinkedList$ReverseLinkIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_offerFirst___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_offerFirst___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "offerFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 728)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_addFirstImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_offerLast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_offerLast___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "offerLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 738)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_addLastImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_peekFirst__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_peekFirst__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "peekFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 748)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_peekFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_peekLast__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_peekLast__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "peekLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 758)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 759)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r1.o) goto label10;
    _r0.o = JAVA_NULL;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_pollFirst__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_pollFirst__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "pollFirst", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 769)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_LinkedList*) _r1.o)->fields.java_util_LinkedList.size_;
    if (_r0.i != 0) goto label6;
    _r0.o = JAVA_NULL;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label6:;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeFirstImpl__(_r1.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_pollLast__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_pollLast__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "pollLast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 779)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_LinkedList*) _r1.o)->fields.java_util_LinkedList.size_;
    if (_r0.i != 0) goto label6;
    _r0.o = JAVA_NULL;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label6:;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeLastImpl__(_r1.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_pop__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_pop__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "pop", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 789)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_push___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_push___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "push", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 799)
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList_addFirstImpl___java_lang_Object(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 800)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_removeFirstOccurrence___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeFirstOccurrence___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeFirstOccurrence", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 809)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_removeFirstOccurrenceImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_removeLastOccurrence___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeLastOccurrence___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeLastOccurrence", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 819)

    
    // Red class access removed: java.util.LinkedList$ReverseLinkIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.LinkedList$ReverseLinkIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("LinkedList.java", 820)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_removeOneOccurrence___java_lang_Object_java_util_Iterator(_r1.o, _r2.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_removeFirstOccurrenceImpl___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeFirstOccurrenceImpl___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeFirstOccurrenceImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 824)

    
    // Red class access removed: java.util.LinkedList$LinkIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.i = 0;

    
    // Red class access removed: java.util.LinkedList$LinkIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("LinkedList.java", 825)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_util_LinkedList_removeOneOccurrence___java_lang_Object_java_util_Iterator(_r2.o, _r3.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_removeOneOccurrence___java_lang_Object_java_util_Iterator(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_removeOneOccurrence___java_lang_Object_java_util_Iterator]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "removeOneOccurrence", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    label0:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 829)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r3.o);
    if (_r0.i != 0) goto label8;
    XMLVM_SOURCE_POSITION("LinkedList.java", 836)
    _r0.i = 0;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 830)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r3.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 831)
    if (_r2.o != JAVA_NULL) goto label21;
    if (_r0.o != JAVA_NULL) goto label0;
    label16:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 832)
    XMLVM_CHECK_NPE(3)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r3.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 833)
    _r0.i = 1;
    goto label7;
    label21:;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[1])(_r2.o, _r0.o);
    if (_r0.i == 0) goto label0;
    goto label16;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_set___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "set", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.i = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("LinkedList.java", 857)
    if (_r4.i < 0) goto label45;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    if (_r4.i >= _r0.i) goto label45;
    XMLVM_SOURCE_POSITION("LinkedList.java", 858)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 859)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r1.i = _r1.i / 2;
    if (_r4.i >= _r1.i) goto label31;
    XMLVM_SOURCE_POSITION("LinkedList.java", 860)
    _r1.i = 0;
    _r2 = _r1;
    _r1 = _r0;
    _r0 = _r2;
    label18:;
    if (_r0.i <= _r4.i) goto label26;
    _r0 = _r1;
    label21:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 868)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 869)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_ = _r5.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 870)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label26:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 861)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    _r0.i = _r0.i + 1;
    goto label18;
    label31:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 864)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_LinkedList*) _r3.o)->fields.java_util_LinkedList.size_;
    _r2 = _r1;
    _r1 = _r0;
    _r0 = _r2;
    label36:;
    if (_r0.i > _r4.i) goto label40;
    _r0 = _r1;
    goto label21;
    label40:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 865)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.previous_;
    _r0.i = _r0.i + -1;
    goto label36;
    label45:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 872)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_LinkedList_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_size__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 882)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_LinkedList*) _r1.o)->fields.java_util_LinkedList.size_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_LinkedList_offer___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_offer___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "offer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 886)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_LinkedList_addLastImpl___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_poll__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_poll__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "poll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 890)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_LinkedList*) _r1.o)->fields.java_util_LinkedList.size_;
    if (_r0.i != 0) goto label6;
    _r0.o = JAVA_NULL;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label6:;
    //java_util_LinkedList_removeFirst__[48]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_LinkedList*) _r1.o)->tib->vtable[48])(_r1.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_remove__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_remove__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 894)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_removeFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_peek__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_peek__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "peek", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 898)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_peekFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_peekFirstImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_peekFirstImpl__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "peekFirstImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 902)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 903)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r1.o) goto label10;
    _r0.o = JAVA_NULL;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_element__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_element__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "element", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 907)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_LinkedList_getFirstImpl__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_toArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_toArray__]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    XMLVM_SOURCE_POSITION("LinkedList.java", 918)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("LinkedList.java", 919)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r1.i);
    XMLVM_SOURCE_POSITION("LinkedList.java", 920)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_;
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    label12:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 921)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    if (_r0.o != _r3.o) goto label17;
    XMLVM_SOURCE_POSITION("LinkedList.java", 925)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label17:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 922)
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(0)
    _r4.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r4.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 923)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_;
    _r2 = _r3;
    goto label12;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_LinkedList_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_toArray___java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 946)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("LinkedList.java", 947)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    if (_r0.i <= _r2.i) goto label50;
    XMLVM_SOURCE_POSITION("LinkedList.java", 948)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[3])(_r7.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 949)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_;
    _r0.o = java_lang_reflect_Array_newInstance___java_lang_Class_int(_r0.o, _r2.i);
    _r0.o = _r0.o;
    label22:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 951)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_LinkedList_Link*) _r2.o)->fields.java_util_LinkedList_Link.next_;
    _r5 = _r2;
    _r2 = _r1;
    _r1 = _r5;
    label29:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 952)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    if (_r1.o != _r3.o) goto label40;
    XMLVM_SOURCE_POSITION("LinkedList.java", 956)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r2.i >= _r1.i) goto label39;
    XMLVM_SOURCE_POSITION("LinkedList.java", 957)
    _r1.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r1.o;
    label39:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 959)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label40:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 953)
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(1)
    _r4.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.data_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r4.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 954)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_;
    _r2 = _r3;
    goto label29;
    label50:;
    _r0 = _r7;
    goto label22;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("LinkedList.java", 963)

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("LinkedList.java", 964)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_LinkedList*) _r2.o)->fields.java_util_LinkedList.size_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("LinkedList.java", 965)
    //java_util_LinkedList_iterator__[12]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_LinkedList*) _r2.o)->tib->vtable[12])(_r2.o);
    label12:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 966)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label19;
    XMLVM_SOURCE_POSITION("LinkedList.java", 969)
    XMLVM_EXIT_METHOD()
    return;
    label19:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 967)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label12;
    //XMLVM_END_WRAPPER
}

void java_util_LinkedList_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.LinkedList", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r4.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("LinkedList.java", 974)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("LinkedList.java", 975)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(6)
    ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("LinkedList.java", 976)
    _r0.o = __NEW_java_util_LinkedList_Link();
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r0.o, _r4.o, _r4.o, _r4.o);
    XMLVM_CHECK_NPE(6)
    ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_ = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 977)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_SOURCE_POSITION("LinkedList.java", 978)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.size_;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    label24:;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label37;
    XMLVM_SOURCE_POSITION("LinkedList.java", 983)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r0.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 984)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_LinkedList*) _r6.o)->fields.java_util_LinkedList.voidLink_;
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 985)
    XMLVM_EXIT_METHOD()
    return;
    label37:;
    XMLVM_SOURCE_POSITION("LinkedList.java", 979)
    _r2.o = __NEW_java_util_LinkedList_Link();

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(2)
    java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(_r2.o, _r3.o, _r1.o, _r4.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 980)
    XMLVM_CHECK_NPE(1)
    ((java_util_LinkedList_Link*) _r1.o)->fields.java_util_LinkedList_Link.next_ = _r2.o;
    _r1 = _r2;
    XMLVM_SOURCE_POSITION("LinkedList.java", 981)
    goto label24;
    //XMLVM_END_WRAPPER
}

