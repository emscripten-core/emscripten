#include "xmlvm.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_util_Collections.h"
#include "java_util_List.h"

#include "java_util_Collections_EmptyList.h"

#define XMLVM_CURRENT_CLASS_NAME Collections_EmptyList
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Collections_EmptyList

__TIB_DEFINITION_java_util_Collections_EmptyList __TIB_java_util_Collections_EmptyList = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Collections_EmptyList, // classInitializer
    "java.util.Collections$EmptyList", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractList, // extends
    sizeof(java_util_Collections_EmptyList), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Collections_EmptyList;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyList_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Collections_EmptyList_serialVersionUID;

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

void __INIT_java_util_Collections_EmptyList()
{
    staticInitializerLock(&__TIB_java_util_Collections_EmptyList);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Collections_EmptyList.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Collections_EmptyList.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Collections_EmptyList);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Collections_EmptyList.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Collections_EmptyList.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Collections_EmptyList.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Collections$EmptyList")
        __INIT_IMPL_java_util_Collections_EmptyList();
    }
}

void __INIT_IMPL_java_util_Collections_EmptyList()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractList)
    __TIB_java_util_Collections_EmptyList.newInstanceFunc = __NEW_INSTANCE_java_util_Collections_EmptyList;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Collections_EmptyList.vtable, __TIB_java_util_AbstractList.vtable, sizeof(__TIB_java_util_AbstractList.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Collections_EmptyList.vtable[10] = (VTABLE_PTR) &java_util_Collections_EmptyList_contains___java_lang_Object;
    __TIB_java_util_Collections_EmptyList.vtable[16] = (VTABLE_PTR) &java_util_Collections_EmptyList_size__;
    __TIB_java_util_Collections_EmptyList.vtable[21] = (VTABLE_PTR) &java_util_Collections_EmptyList_get___int;
    // Initialize interface information
    __TIB_java_util_Collections_EmptyList.numImplementedInterfaces = 5;
    __TIB_java_util_Collections_EmptyList.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 5);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Collections_EmptyList.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_Collections_EmptyList.implementedInterfaces[0][1] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_Collections_EmptyList.implementedInterfaces[0][2] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_List)

    __TIB_java_util_Collections_EmptyList.implementedInterfaces[0][3] = &__TIB_java_util_List;

    XMLVM_CLASS_INIT(java_util_RandomAccess)

    __TIB_java_util_Collections_EmptyList.implementedInterfaces[0][4] = &__TIB_java_util_RandomAccess;
    // Initialize itable for this class
    __TIB_java_util_Collections_EmptyList.itableBegin = &__TIB_java_util_Collections_EmptyList.itable[0];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_Collections_EmptyList.vtable[12];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[6];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[7];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_Collections_EmptyList.vtable[8];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[9];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[10];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[1];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_Collections_EmptyList.vtable[4];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_Collections_EmptyList.vtable[11];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_Collections_EmptyList.vtable[12];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[13];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[14];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[15];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_Collections_EmptyList.vtable[16];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_Collections_EmptyList.vtable[17];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_Collections_EmptyList.vtable[18];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___int_java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[19];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[6];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_add___int_java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[20];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_add___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[7];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_clear__] = __TIB_java_util_Collections_EmptyList.vtable[8];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_containsAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[9];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_contains___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[10];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_equals___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[1];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_get___int] = __TIB_java_util_Collections_EmptyList.vtable[21];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_hashCode__] = __TIB_java_util_Collections_EmptyList.vtable[4];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[22];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_isEmpty__] = __TIB_java_util_Collections_EmptyList.vtable[11];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_iterator__] = __TIB_java_util_Collections_EmptyList.vtable[12];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_lastIndexOf___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[23];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator__] = __TIB_java_util_Collections_EmptyList.vtable[24];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator___int] = __TIB_java_util_Collections_EmptyList.vtable[25];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_removeAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[13];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___int] = __TIB_java_util_Collections_EmptyList.vtable[27];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[14];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_retainAll___java_util_Collection] = __TIB_java_util_Collections_EmptyList.vtable[15];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object] = __TIB_java_util_Collections_EmptyList.vtable[28];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_size__] = __TIB_java_util_Collections_EmptyList.vtable[16];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_subList___int_int] = __TIB_java_util_Collections_EmptyList.vtable[29];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray__] = __TIB_java_util_Collections_EmptyList.vtable[17];
    __TIB_java_util_Collections_EmptyList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_Collections_EmptyList.vtable[18];

    _STATIC_java_util_Collections_EmptyList_serialVersionUID = 8842843931221139166;

    __TIB_java_util_Collections_EmptyList.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Collections_EmptyList.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptyList.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Collections_EmptyList.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Collections_EmptyList.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptyList.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Collections_EmptyList.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Collections_EmptyList.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Collections_EmptyList = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Collections_EmptyList);
    __TIB_java_util_Collections_EmptyList.clazz = __CLASS_java_util_Collections_EmptyList;
    __TIB_java_util_Collections_EmptyList.baseType = JAVA_NULL;
    __CLASS_java_util_Collections_EmptyList_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyList);
    __CLASS_java_util_Collections_EmptyList_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyList_1ARRAY);
    __CLASS_java_util_Collections_EmptyList_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyList_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Collections_EmptyList]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Collections_EmptyList.classInitialized = 1;
}

void __DELETE_java_util_Collections_EmptyList(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Collections_EmptyList]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractList(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyList]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Collections_EmptyList()
{    XMLVM_CLASS_INIT(java_util_Collections_EmptyList)
java_util_Collections_EmptyList* me = (java_util_Collections_EmptyList*) XMLVM_MALLOC(sizeof(java_util_Collections_EmptyList));
    me->tib = &__TIB_java_util_Collections_EmptyList;
    __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyList(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Collections_EmptyList]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptyList()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_util_Collections_EmptyList_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptyList)
    return _STATIC_java_util_Collections_EmptyList_serialVersionUID;
}

void java_util_Collections_EmptyList_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptyList)
_STATIC_java_util_Collections_EmptyList_serialVersionUID = v;
}

void java_util_Collections_EmptyList___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList___INIT___]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 74)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractList___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_EmptyList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 80)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_EmptyList_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList_size__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 85)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyList_get___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList_get___int]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 90)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyList_readResolve__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList_readResolve__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "readResolve", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 94)
    _r0.o = java_util_Collections_GET_EMPTY_LIST();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_EmptyList___INIT____java_util_Collections_EmptyList(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyList___INIT____java_util_Collections_EmptyList]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 74)
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptyList___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

