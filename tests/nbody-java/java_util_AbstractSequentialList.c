#include "xmlvm.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_util_Collection.h"
#include "java_util_Iterator.h"

#include "java_util_AbstractSequentialList.h"

#define XMLVM_CURRENT_CLASS_NAME AbstractSequentialList
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_AbstractSequentialList

__TIB_DEFINITION_java_util_AbstractSequentialList __TIB_java_util_AbstractSequentialList = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_AbstractSequentialList, // classInitializer
    "java.util.AbstractSequentialList", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/util/AbstractList<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractList, // extends
    sizeof(java_util_AbstractSequentialList), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_AbstractSequentialList;
JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_1ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_2ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractSequentialList_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION


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

void __INIT_java_util_AbstractSequentialList()
{
    staticInitializerLock(&__TIB_java_util_AbstractSequentialList);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_AbstractSequentialList.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_AbstractSequentialList.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_AbstractSequentialList);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_AbstractSequentialList.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_AbstractSequentialList.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_AbstractSequentialList.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.AbstractSequentialList")
        __INIT_IMPL_java_util_AbstractSequentialList();
    }
}

void __INIT_IMPL_java_util_AbstractSequentialList()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractList)
    __TIB_java_util_AbstractSequentialList.newInstanceFunc = __NEW_INSTANCE_java_util_AbstractSequentialList;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_AbstractSequentialList.vtable, __TIB_java_util_AbstractList.vtable, sizeof(__TIB_java_util_AbstractList.vtable));
    // Initialize vtable for this class
    __TIB_java_util_AbstractSequentialList.vtable[20] = (VTABLE_PTR) &java_util_AbstractSequentialList_add___int_java_lang_Object;
    __TIB_java_util_AbstractSequentialList.vtable[19] = (VTABLE_PTR) &java_util_AbstractSequentialList_addAll___int_java_util_Collection;
    __TIB_java_util_AbstractSequentialList.vtable[21] = (VTABLE_PTR) &java_util_AbstractSequentialList_get___int;
    __TIB_java_util_AbstractSequentialList.vtable[12] = (VTABLE_PTR) &java_util_AbstractSequentialList_iterator__;
    __TIB_java_util_AbstractSequentialList.vtable[27] = (VTABLE_PTR) &java_util_AbstractSequentialList_remove___int;
    __TIB_java_util_AbstractSequentialList.vtable[28] = (VTABLE_PTR) &java_util_AbstractSequentialList_set___int_java_lang_Object;
    // Initialize interface information
    __TIB_java_util_AbstractSequentialList.numImplementedInterfaces = 3;
    __TIB_java_util_AbstractSequentialList.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_AbstractSequentialList.implementedInterfaces[0][0] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_AbstractSequentialList.implementedInterfaces[0][1] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_List)

    __TIB_java_util_AbstractSequentialList.implementedInterfaces[0][2] = &__TIB_java_util_List;
    // Initialize itable for this class
    __TIB_java_util_AbstractSequentialList.itableBegin = &__TIB_java_util_AbstractSequentialList.itable[0];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_AbstractSequentialList.vtable[12];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[6];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[7];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_AbstractSequentialList.vtable[8];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[9];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[10];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[1];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_AbstractSequentialList.vtable[4];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_AbstractSequentialList.vtable[11];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_AbstractSequentialList.vtable[12];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[13];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[14];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[15];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_AbstractSequentialList.vtable[16];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_AbstractSequentialList.vtable[17];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_AbstractSequentialList.vtable[18];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___int_java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[19];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[6];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_add___int_java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[20];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_add___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[7];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_clear__] = __TIB_java_util_AbstractSequentialList.vtable[8];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_containsAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[9];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_contains___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[10];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_equals___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[1];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_get___int] = __TIB_java_util_AbstractSequentialList.vtable[21];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_hashCode__] = __TIB_java_util_AbstractSequentialList.vtable[4];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[22];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_isEmpty__] = __TIB_java_util_AbstractSequentialList.vtable[11];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_iterator__] = __TIB_java_util_AbstractSequentialList.vtable[12];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_lastIndexOf___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[23];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator__] = __TIB_java_util_AbstractSequentialList.vtable[24];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator___int] = __TIB_java_util_AbstractSequentialList.vtable[25];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_removeAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[13];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___int] = __TIB_java_util_AbstractSequentialList.vtable[27];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[14];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_retainAll___java_util_Collection] = __TIB_java_util_AbstractSequentialList.vtable[15];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object] = __TIB_java_util_AbstractSequentialList.vtable[28];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_size__] = __TIB_java_util_AbstractSequentialList.vtable[16];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_subList___int_int] = __TIB_java_util_AbstractSequentialList.vtable[29];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray__] = __TIB_java_util_AbstractSequentialList.vtable[17];
    __TIB_java_util_AbstractSequentialList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_AbstractSequentialList.vtable[18];


    __TIB_java_util_AbstractSequentialList.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_AbstractSequentialList.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_AbstractSequentialList.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_AbstractSequentialList.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_AbstractSequentialList.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_AbstractSequentialList.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_AbstractSequentialList.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_AbstractSequentialList.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_AbstractSequentialList = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_AbstractSequentialList);
    __TIB_java_util_AbstractSequentialList.clazz = __CLASS_java_util_AbstractSequentialList;
    __TIB_java_util_AbstractSequentialList.baseType = JAVA_NULL;
    __CLASS_java_util_AbstractSequentialList_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSequentialList);
    __CLASS_java_util_AbstractSequentialList_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSequentialList_1ARRAY);
    __CLASS_java_util_AbstractSequentialList_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSequentialList_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_AbstractSequentialList]
    //XMLVM_END_WRAPPER

    __TIB_java_util_AbstractSequentialList.classInitialized = 1;
}

void __DELETE_java_util_AbstractSequentialList(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_AbstractSequentialList]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_AbstractSequentialList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractList(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_AbstractSequentialList]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_AbstractSequentialList()
{    XMLVM_CLASS_INIT(java_util_AbstractSequentialList)
java_util_AbstractSequentialList* me = (java_util_AbstractSequentialList*) XMLVM_MALLOC(sizeof(java_util_AbstractSequentialList));
    me->tib = &__TIB_java_util_AbstractSequentialList;
    __INIT_INSTANCE_MEMBERS_java_util_AbstractSequentialList(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_AbstractSequentialList]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractSequentialList()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_AbstractSequentialList();
    java_util_AbstractSequentialList___INIT___(me);
    return me;
}

void java_util_AbstractSequentialList___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList___INIT___]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 33)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractList___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 34)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_AbstractSequentialList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_add___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 38)
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r1.o)->tib->vtable[25])(_r1.o, _r2.i);

    
    // Red class access removed: java.util.ListIterator::add
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 39)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractSequentialList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_addAll___int_java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "addAll", "?")
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
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 43)
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r4.o)->tib->vtable[25])(_r4.o, _r5.i);
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 44)
    XMLVM_CHECK_NPE(6)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r6.o);
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 45)

    
    // Red class access removed: java.util.ListIterator::nextIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    label12:;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 46)
    XMLVM_CHECK_NPE(1)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r3.i != 0) goto label26;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 49)

    
    // Red class access removed: java.util.ListIterator::nextIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r2.i == _r0.i) goto label34;
    _r0.i = 1;
    label25:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label26:;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 47)
    XMLVM_CHECK_NPE(1)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);

    
    // Red class access removed: java.util.ListIterator::add
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label12;
    label34:;
    _r0.i = 0;
    goto label25;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractSequentialList_get___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_get___int]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "get", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_TRY_BEGIN(w3823aaab3b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 55)
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r1.o)->tib->vtable[25])(_r1.o, _r2.i);

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3823aaab3b1b3)
    XMLVM_CATCH_END(w3823aaab3b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w3823aaab3b1b3)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 57)
    java_lang_Thread* curThread_w3823aaab3b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3823aaab3b1b7->fields.java_lang_Thread.xmlvmException_;
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractSequentialList_iterator__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_iterator__]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "iterator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 63)
    _r0.i = 0;
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r1.o)->tib->vtable[25])(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractSequentialList_remove___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_remove___int]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "remove", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_TRY_BEGIN(w3823aaab6b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 72)
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r2.o)->tib->vtable[25])(_r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 73)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 74)

    
    // Red class access removed: java.util.ListIterator::remove
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3823aaab6b1b3)
    XMLVM_CATCH_END(w3823aaab6b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w3823aaab6b1b3)
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 75)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 77)
    java_lang_Thread* curThread_w3823aaab6b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3823aaab6b1b8->fields.java_lang_Thread.xmlvmException_;
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractSequentialList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSequentialList_set___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractSequentialList", "set", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.i = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 83)
    //java_util_AbstractSequentialList_listIterator___int[25]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_AbstractSequentialList*) _r2.o)->tib->vtable[25])(_r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 84)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r1.i != 0) goto label16;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 85)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 87)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 88)

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("AbstractSequentialList.java", 89)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

