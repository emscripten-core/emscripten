#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Comparable.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_util_ArrayList.h"
#include "java_util_Collection.h"
#include "java_util_Collections_EmptyList.h"
#include "java_util_Collections_EmptyMap.h"
#include "java_util_Collections_EmptySet.h"
#include "java_util_Collections_SynchronizedMap.h"
#include "java_util_Comparator.h"
#include "java_util_Deque.h"
#include "java_util_Enumeration.h"
#include "java_util_Iterator.h"
#include "java_util_List.h"
#include "java_util_Map.h"
#include "java_util_Queue.h"
#include "java_util_RandomAccess.h"
#include "java_util_Set.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_util_Collections.h"

#define XMLVM_CURRENT_CLASS_NAME Collections
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Collections

__TIB_DEFINITION_java_util_Collections __TIB_java_util_Collections = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Collections, // classInitializer
    "java.util.Collections", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_Collections), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Collections;
JAVA_OBJECT __CLASS_java_util_Collections_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_util_Collections_EMPTY_LIST;
static JAVA_OBJECT _STATIC_java_util_Collections_EMPTY_SET;
static JAVA_OBJECT _STATIC_java_util_Collections_EMPTY_MAP;

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

void __INIT_java_util_Collections()
{
    staticInitializerLock(&__TIB_java_util_Collections);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Collections.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Collections.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Collections);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Collections.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Collections.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Collections.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Collections")
        __INIT_IMPL_java_util_Collections();
    }
}

void __INIT_IMPL_java_util_Collections()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_Collections.newInstanceFunc = __NEW_INSTANCE_java_util_Collections;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Collections.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_util_Collections.numImplementedInterfaces = 0;
    __TIB_java_util_Collections.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_util_Collections_EMPTY_LIST = (java_util_List*) JAVA_NULL;
    _STATIC_java_util_Collections_EMPTY_SET = (java_util_Set*) JAVA_NULL;
    _STATIC_java_util_Collections_EMPTY_MAP = (java_util_Map*) JAVA_NULL;

    __TIB_java_util_Collections.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Collections.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Collections.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Collections.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Collections.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Collections.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Collections.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Collections.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Collections = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Collections);
    __TIB_java_util_Collections.clazz = __CLASS_java_util_Collections;
    __TIB_java_util_Collections.baseType = JAVA_NULL;
    __CLASS_java_util_Collections_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections);
    __CLASS_java_util_Collections_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_1ARRAY);
    __CLASS_java_util_Collections_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_2ARRAY);
    java_util_Collections___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Collections]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Collections.classInitialized = 1;
}

void __DELETE_java_util_Collections(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Collections]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Collections(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Collections]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Collections()
{    XMLVM_CLASS_INIT(java_util_Collections)
java_util_Collections* me = (java_util_Collections*) XMLVM_MALLOC(sizeof(java_util_Collections));
    me->tib = &__TIB_java_util_Collections;
    __INIT_INSTANCE_MEMBERS_java_util_Collections(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Collections]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Collections()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_util_Collections_GET_EMPTY_LIST()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    return _STATIC_java_util_Collections_EMPTY_LIST;
}

void java_util_Collections_PUT_EMPTY_LIST(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Collections)
_STATIC_java_util_Collections_EMPTY_LIST = v;
}

JAVA_OBJECT java_util_Collections_GET_EMPTY_SET()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    return _STATIC_java_util_Collections_EMPTY_SET;
}

void java_util_Collections_PUT_EMPTY_SET(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Collections)
_STATIC_java_util_Collections_EMPTY_SET = v;
}

JAVA_OBJECT java_util_Collections_GET_EMPTY_MAP()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    return _STATIC_java_util_Collections_EMPTY_MAP;
}

void java_util_Collections_PUT_EMPTY_MAP(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Collections)
_STATIC_java_util_Collections_EMPTY_MAP = v;
}

void java_util_Collections___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections___CLINIT___]
    XMLVM_ENTER_METHOD("java.util.Collections", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Collections.java", 179)
    _r0.o = __NEW_java_util_Collections_EmptyList();
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptyList___INIT____java_util_Collections_EmptyList(_r0.o, _r1.o);
    java_util_Collections_PUT_EMPTY_LIST( _r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 185)
    _r0.o = __NEW_java_util_Collections_EmptySet();
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptySet___INIT____java_util_Collections_EmptySet(_r0.o, _r1.o);
    java_util_Collections_PUT_EMPTY_SET( _r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 191)
    _r0.o = __NEW_java_util_Collections_EmptyMap();
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptyMap___INIT____java_util_Collections_EmptyMap(_r0.o, _r1.o);
    java_util_Collections_PUT_EMPTY_MAP( _r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 36)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Collections___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections___INIT___]
    XMLVM_ENTER_METHOD("java.util.Collections", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 1465)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1467)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_binarySearch___java_util_List_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_binarySearch___java_util_List_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "binarySearch", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r7.o = n1;
    _r8.o = n2;
    _r3.i = -1;
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("Collections.java", 1489)
    if (_r7.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("Collections.java", 1490)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label10:;
    XMLVM_SOURCE_POSITION("Collections.java", 1492)
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_isEmpty__])(_r7.o);
    if (_r0.i == 0) goto label18;
    _r0 = _r3;
    label17:;
    XMLVM_SOURCE_POSITION("Collections.java", 1493)
    XMLVM_SOURCE_POSITION("Collections.java", 1522)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    XMLVM_SOURCE_POSITION("Collections.java", 1497)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r7.o, __CLASS_java_util_RandomAccess);
    if (_r0.i != 0) goto label66;
    XMLVM_SOURCE_POSITION("Collections.java", 1498)
    XMLVM_CHECK_NPE(7)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r7.o);
    label26:;
    XMLVM_SOURCE_POSITION("Collections.java", 1499)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i != 0) goto label39;
    XMLVM_SOURCE_POSITION("Collections.java", 1508)
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r7.o);
    _r0.i = -_r0.i;
    _r0.i = _r0.i - _r4.i;
    goto label17;
    label39:;
    XMLVM_SOURCE_POSITION("Collections.java", 1501)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object])(_r0.o, _r8.o);
    _r0.i = -_r0.i;
    if (_r0.i > 0) goto label26;
    XMLVM_SOURCE_POSITION("Collections.java", 1502)
    if (_r0.i != 0) goto label59;
    XMLVM_SOURCE_POSITION("Collections.java", 1503)

    
    // Red class access removed: java.util.ListIterator::previousIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label17;
    label59:;
    XMLVM_SOURCE_POSITION("Collections.java", 1505)

    
    // Red class access removed: java.util.ListIterator::previousIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = -_r0.i;
    _r0.i = _r0.i - _r4.i;
    goto label17;
    label66:;
    XMLVM_SOURCE_POSITION("Collections.java", 1511)
    _r0.i = 0;
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r7.o);
    _r2.i = _r1.i - _r4.i;
    _r5 = _r3;
    _r3 = _r0;
    _r0 = _r5;
    _r6 = _r1;
    _r1 = _r2;
    _r2 = _r6;
    label79:;
    XMLVM_SOURCE_POSITION("Collections.java", 1512)
    if (_r3.i <= _r1.i) goto label88;
    _r1.i = -_r2.i;
    if (_r0.i >= 0) goto label115;
    _r0 = _r4;
    label85:;
    _r0.i = _r1.i - _r0.i;
    goto label17;
    label88:;
    XMLVM_SOURCE_POSITION("Collections.java", 1513)
    _r0.i = _r3.i + _r1.i;
    _r2.i = _r0.i >> 1;
    XMLVM_SOURCE_POSITION("Collections.java", 1514)
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r7.o, _r2.i);
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object])(_r0.o, _r8.o);
    _r0.i = -_r0.i;
    if (_r0.i <= 0) goto label108;
    XMLVM_SOURCE_POSITION("Collections.java", 1515)
    _r3.i = _r2.i + 1;
    goto label79;
    label108:;
    XMLVM_SOURCE_POSITION("Collections.java", 1516)
    if (_r0.i != 0) goto label112;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Collections.java", 1517)
    goto label17;
    label112:;
    XMLVM_SOURCE_POSITION("Collections.java", 1519)
    _r1.i = _r2.i - _r4.i;
    goto label79;
    label115:;
    _r0.i = 2;
    goto label85;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_binarySearch___java_util_List_java_lang_Object_java_util_Comparator(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_binarySearch___java_util_List_java_lang_Object_java_util_Comparator]
    XMLVM_ENTER_METHOD("java.util.Collections", "binarySearch", "?")
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
    _r7.o = n1;
    _r8.o = n2;
    _r9.o = n3;
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("Collections.java", 1549)
    if (_r9.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 1550)
    _r0.i = java_util_Collections_binarySearch___java_util_List_java_lang_Object(_r7.o, _r8.o);
    label7:;
    XMLVM_SOURCE_POSITION("Collections.java", 1578)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 1553)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r7.o, __CLASS_java_util_RandomAccess);
    if (_r0.i != 0) goto label54;
    XMLVM_SOURCE_POSITION("Collections.java", 1554)
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r7.o);
    label16:;
    XMLVM_SOURCE_POSITION("Collections.java", 1555)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r1.i != 0) goto label29;
    XMLVM_SOURCE_POSITION("Collections.java", 1564)
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r7.o);
    _r0.i = -_r0.i;
    _r0.i = _r0.i - _r4.i;
    goto label7;
    label29:;
    XMLVM_SOURCE_POSITION("Collections.java", 1557)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(9)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r9.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Comparator_compare___java_lang_Object_java_lang_Object])(_r9.o, _r1.o, _r8.o);
    _r1.i = -_r1.i;
    if (_r1.i > 0) goto label16;
    XMLVM_SOURCE_POSITION("Collections.java", 1558)
    if (_r1.i != 0) goto label47;
    XMLVM_SOURCE_POSITION("Collections.java", 1559)

    
    // Red class access removed: java.util.ListIterator::previousIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label7;
    label47:;
    XMLVM_SOURCE_POSITION("Collections.java", 1561)

    
    // Red class access removed: java.util.ListIterator::previousIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = -_r0.i;
    _r0.i = _r0.i - _r4.i;
    goto label7;
    label54:;
    XMLVM_SOURCE_POSITION("Collections.java", 1567)
    _r0.i = 0;
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r7.o);
    _r2.i = _r1.i - _r4.i;
    _r3.i = -1;
    _r5 = _r3;
    _r3 = _r0;
    _r0 = _r5;
    _r6 = _r1;
    _r1 = _r2;
    _r2 = _r6;
    label68:;
    XMLVM_SOURCE_POSITION("Collections.java", 1568)
    if (_r3.i <= _r1.i) goto label77;
    _r1.i = -_r2.i;
    if (_r0.i >= 0) goto label106;
    _r0 = _r4;
    label74:;
    _r0.i = _r1.i - _r0.i;
    goto label7;
    label77:;
    XMLVM_SOURCE_POSITION("Collections.java", 1569)
    _r0.i = _r3.i + _r1.i;
    _r0.i = _r0.i >> 1;
    XMLVM_SOURCE_POSITION("Collections.java", 1570)
    XMLVM_CHECK_NPE(7)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r7.o, _r0.i);
    XMLVM_CHECK_NPE(9)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r9.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Comparator_compare___java_lang_Object_java_lang_Object])(_r9.o, _r2.o, _r8.o);
    _r2.i = -_r2.i;
    if (_r2.i <= 0) goto label98;
    XMLVM_SOURCE_POSITION("Collections.java", 1571)
    _r3.i = _r0.i + 1;
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    goto label68;
    label98:;
    XMLVM_SOURCE_POSITION("Collections.java", 1572)
    if (_r2.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 1575)
    _r1.i = _r0.i - _r4.i;
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    goto label68;
    label106:;
    _r0.i = 2;
    goto label74;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_copy___java_util_List_java_util_List(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_copy___java_util_List_java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "copy", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r4.o = n1;
    _r5.o = n2;
    // "luni.38"
    _r3.o = xmlvm_create_java_string_from_pool(810);
    XMLVM_SOURCE_POSITION("Collections.java", 1599)
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r4.o);
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r5.o);
    if (_r0.i >= _r1.i) goto label28;
    XMLVM_SOURCE_POSITION("Collections.java", 1601)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.38"
    _r1.o = xmlvm_create_java_string_from_pool(810);
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r5.o);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r3.o, _r1.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_SOURCE_POSITION("Collections.java", 1603)
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_iterator__])(_r5.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1604)
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r4.o);
    label36:;
    XMLVM_SOURCE_POSITION("Collections.java", 1605)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label43;
    XMLVM_SOURCE_POSITION("Collections.java", 1614)
    XMLVM_EXIT_METHOD()
    return;
    label43:;
    XMLVM_TRY_BEGIN(w5845aaab7b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Collections.java", 1607)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w5845aaab7b1c27)
    XMLVM_CATCH_END(w5845aaab7b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w5845aaab7b1c27)
    XMLVM_SOURCE_POSITION("Collections.java", 1612)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label36;
    label54:;
    XMLVM_SOURCE_POSITION("Collections.java", 1610)
    java_lang_Thread* curThread_w5845aaab7b1c34 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w5845aaab7b1c34->fields.java_lang_Thread.xmlvmException_;

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.38"
    _r1.o = xmlvm_create_java_string_from_pool(810);
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r5.o);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r3.o, _r1.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_enumeration___java_util_Collection(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_enumeration___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "enumeration", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1624)
    XMLVM_SOURCE_POSITION("Collections.java", 1625)

    
    // Red class access removed: java.util.Collections$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_fill___java_util_List_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_fill___java_util_List_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "fill", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1649)
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r2.o);
    label4:;
    XMLVM_SOURCE_POSITION("Collections.java", 1650)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r1.i != 0) goto label11;
    XMLVM_SOURCE_POSITION("Collections.java", 1654)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    XMLVM_SOURCE_POSITION("Collections.java", 1651)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1652)

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_max___java_util_Collection(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_max___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1669)
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r5.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1670)
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 1671)
    XMLVM_CHECK_NPE(2)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r2.o);
    if (_r3.i != 0) goto label15;
    XMLVM_SOURCE_POSITION("Collections.java", 1677)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label15:;
    XMLVM_SOURCE_POSITION("Collections.java", 1672)
    XMLVM_CHECK_NPE(2)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1673)
    _r0 = _r1;
    _r0.o = _r0.o;
    _r5 = _r0;
    XMLVM_CHECK_NPE(5)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object])(_r5.o, _r3.o);
    if (_r4.i >= 0) goto label8;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("Collections.java", 1674)
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_max___java_util_Collection_java_util_Comparator(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_max___java_util_Collection_java_util_Comparator]
    XMLVM_ENTER_METHOD("java.util.Collections", "max", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1695)
    if (_r5.o != JAVA_NULL) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 1697)
    _r0.o = java_util_Collections_max___java_util_Collection(_r4.o);
    label6:;
    XMLVM_SOURCE_POSITION("Collections.java", 1709)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_SOURCE_POSITION("Collections.java", 1701)
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r4.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1702)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    label15:;
    XMLVM_SOURCE_POSITION("Collections.java", 1703)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label23;
    _r0 = _r1;
    goto label6;
    label23:;
    XMLVM_SOURCE_POSITION("Collections.java", 1704)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1705)
    XMLVM_CHECK_NPE(5)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Comparator_compare___java_lang_Object_java_lang_Object])(_r5.o, _r1.o, _r2.o);
    if (_r3.i >= 0) goto label15;
    _r1 = _r2;
    XMLVM_SOURCE_POSITION("Collections.java", 1706)
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_min___java_util_Collection(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_min___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1725)
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r5.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1726)
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 1727)
    XMLVM_CHECK_NPE(2)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r2.o);
    if (_r3.i != 0) goto label15;
    XMLVM_SOURCE_POSITION("Collections.java", 1733)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label15:;
    XMLVM_SOURCE_POSITION("Collections.java", 1728)
    XMLVM_CHECK_NPE(2)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1729)
    _r0 = _r1;
    _r0.o = _r0.o;
    _r5 = _r0;
    XMLVM_CHECK_NPE(5)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object])(_r5.o, _r3.o);
    if (_r4.i <= 0) goto label8;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("Collections.java", 1730)
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_min___java_util_Collection_java_util_Comparator(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_min___java_util_Collection_java_util_Comparator]
    XMLVM_ENTER_METHOD("java.util.Collections", "min", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1751)
    if (_r5.o != JAVA_NULL) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 1753)
    _r0.o = java_util_Collections_min___java_util_Collection(_r4.o);
    label6:;
    XMLVM_SOURCE_POSITION("Collections.java", 1765)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_SOURCE_POSITION("Collections.java", 1757)
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r4.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1758)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    label15:;
    XMLVM_SOURCE_POSITION("Collections.java", 1759)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label23;
    _r0 = _r1;
    goto label6;
    label23:;
    XMLVM_SOURCE_POSITION("Collections.java", 1760)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1761)
    XMLVM_CHECK_NPE(5)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Comparator_compare___java_lang_Object_java_lang_Object])(_r5.o, _r1.o, _r2.o);
    if (_r3.i <= 0) goto label15;
    _r1 = _r2;
    XMLVM_SOURCE_POSITION("Collections.java", 1762)
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_nCopies___int_java_lang_Object(JAVA_INT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_nCopies___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "nCopies", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.i = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1781)

    
    // Red class access removed: java.util.Collections$CopiesList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CopiesList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_reverse___java_util_List(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_reverse___java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "reverse", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1795)
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r6.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1796)
    XMLVM_CHECK_NPE(6)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r6.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1798)
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator___int])(_r6.o, _r0.i);
    XMLVM_SOURCE_POSITION("Collections.java", 1799)
    _r3.i = 0;
    label13:;
    _r4.i = _r0.i / 2;
    if (_r3.i < _r4.i) goto label18;
    XMLVM_SOURCE_POSITION("Collections.java", 1805)
    XMLVM_EXIT_METHOD()
    return;
    label18:;
    XMLVM_SOURCE_POSITION("Collections.java", 1800)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1801)

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1802)

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1803)

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    _r3.i = _r3.i + 1;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_reverseOrder__()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_reverseOrder__]
    XMLVM_ENTER_METHOD("java.util.Collections", "reverseOrder", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Collections.java", 1818)

    
    // Red class access removed: java.util.Collections$ReverseComparator::access$0
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_reverseOrder___java_util_Comparator(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_reverseOrder___java_util_Comparator]
    XMLVM_ENTER_METHOD("java.util.Collections", "reverseOrder", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1836)
    if (_r1.o != JAVA_NULL) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 1837)
    _r0.o = java_util_Collections_reverseOrder__();
    label6:;
    XMLVM_SOURCE_POSITION("Collections.java", 1842)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_SOURCE_POSITION("Collections.java", 1839)

    
    // Red class access removed: java.util.Collections$ReverseComparatorWithComparator::instance-of
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i == 0) goto label18;
    XMLVM_SOURCE_POSITION("Collections.java", 1840)

    
    // Red class access removed: java.util.Collections$ReverseComparatorWithComparator::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$ReverseComparatorWithComparator::access$0
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label6;
    label18:;

    
    // Red class access removed: java.util.Collections$ReverseComparatorWithComparator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$ReverseComparatorWithComparator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label6;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_shuffle___java_util_List(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_shuffle___java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "shuffle", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1855)

    
    // Red class access removed: java.util.Random::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Random::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    java_util_Collections_shuffle___java_util_List_java_util_Random(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1856)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_shuffle___java_util_List_java_util_Random(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_shuffle___java_util_List_java_util_Random]
    XMLVM_ENTER_METHOD("java.util.Collections", "shuffle", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = n1;
    _r6.o = n2;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Collections.java", 1871)
    XMLVM_SOURCE_POSITION("Collections.java", 1873)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r5.o, __CLASS_java_util_RandomAccess);
    if (_r0.i == 0) goto label33;
    XMLVM_SOURCE_POSITION("Collections.java", 1874)
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r5.o);
    _r0.i = _r0.i - _r2.i;
    label10:;
    if (_r0.i > 0) goto label13;
    label12:;
    XMLVM_SOURCE_POSITION("Collections.java", 1894)
    XMLVM_EXIT_METHOD()
    return;
    label13:;
    XMLVM_SOURCE_POSITION("Collections.java", 1875)
    _r1.i = _r0.i + 1;

    
    // Red class access removed: java.util.Random::nextInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1876)
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r5.o, _r1.i);
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r5.o, _r0.i, _r2.o);
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r5.o, _r1.i, _r2.o);
    _r0.i = _r0.i + -1;
    goto label10;
    label33:;
    XMLVM_SOURCE_POSITION("Collections.java", 1879)
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_toArray__])(_r5.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1880)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r1.i = _r1.i - _r2.i;
    label39:;
    if (_r1.i > 0) goto label64;
    XMLVM_SOURCE_POSITION("Collections.java", 1887)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 1888)
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r5.o);
    label46:;
    XMLVM_SOURCE_POSITION("Collections.java", 1889)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r3.i == 0) goto label12;
    XMLVM_SOURCE_POSITION("Collections.java", 1890)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1891)
    _r3.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1 = _r3;
    goto label46;
    label64:;
    XMLVM_SOURCE_POSITION("Collections.java", 1881)
    _r2.i = _r1.i + 1;

    
    // Red class access removed: java.util.Random::nextInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1882)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("Collections.java", 1883)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.o;
    XMLVM_SOURCE_POSITION("Collections.java", 1884)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.o;
    _r1.i = _r1.i + -1;
    goto label39;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_singleton___java_lang_Object(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_singleton___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "singleton", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1905)

    
    // Red class access removed: java.util.Collections$SingletonSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SingletonSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_singletonList___java_lang_Object(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_singletonList___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "singletonList", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1917)

    
    // Red class access removed: java.util.Collections$SingletonList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SingletonList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_singletonMap___java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_singletonMap___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "singletonMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1931)

    
    // Red class access removed: java.util.Collections$SingletonMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SingletonMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_sort___java_util_List(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_sort___java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "sort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 1946)
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_toArray__])(_r4.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1947)

    
    // Red class access removed: java.util.Arrays::sort
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1948)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 1949)
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r4.o);
    label12:;
    XMLVM_SOURCE_POSITION("Collections.java", 1950)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r3.i != 0) goto label19;
    XMLVM_SOURCE_POSITION("Collections.java", 1954)
    XMLVM_EXIT_METHOD()
    return;
    label19:;
    XMLVM_SOURCE_POSITION("Collections.java", 1951)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1952)
    _r3.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r4.o = _r4.o;

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1 = _r3;
    goto label12;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_sort___java_util_List_java_util_Comparator(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_sort___java_util_List_java_util_Comparator]
    XMLVM_ENTER_METHOD("java.util.Collections", "sort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 1970)
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r4.o);
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r0.i);
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_toArray___java_lang_Object_1ARRAY])(_r4.o, _r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 1971)

    
    // Red class access removed: java.util.Arrays::sort
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1972)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 1973)
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator__])(_r4.o);
    label18:;
    XMLVM_SOURCE_POSITION("Collections.java", 1974)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r3.i != 0) goto label25;
    XMLVM_SOURCE_POSITION("Collections.java", 1978)
    XMLVM_EXIT_METHOD()
    return;
    label25:;
    XMLVM_SOURCE_POSITION("Collections.java", 1975)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 1976)
    _r3.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];

    
    // Red class access removed: java.util.ListIterator::set
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1 = _r3;
    goto label18;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_swap___java_util_List_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_swap___java_util_List_int_int]
    XMLVM_ENTER_METHOD("java.util.Collections", "swap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("Collections.java", 1998)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 1999)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2001)
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r1.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2002)
    if (_r2.i < 0) goto label20;
    if (_r2.i >= _r0.i) goto label20;
    if (_r3.i < 0) goto label20;
    if (_r3.i < _r0.i) goto label26;
    label20:;
    XMLVM_SOURCE_POSITION("Collections.java", 2003)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label26:;
    XMLVM_SOURCE_POSITION("Collections.java", 2005)
    if (_r2.i != _r3.i) goto label29;
    label28:;
    XMLVM_SOURCE_POSITION("Collections.java", 2010)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_SOURCE_POSITION("Collections.java", 2009)
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r1.o, _r3.i);
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r1.o, _r2.i, _r0.o);
    XMLVM_CHECK_NPE(1)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r1.o, _r3.i, _r0.o);
    goto label28;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_replaceAll___java_util_List_java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_replaceAll___java_util_List_java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "replaceAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = n1;
    _r4.o = n2;
    _r5.o = n3;
    XMLVM_SOURCE_POSITION("Collections.java", 2031)
    _r0.i = 0;
    label1:;
    XMLVM_SOURCE_POSITION("Collections.java", 2033)
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object])(_r3.o, _r4.o);
    _r2.i = -1;
    if (_r1.i > _r2.i) goto label9;
    XMLVM_SOURCE_POSITION("Collections.java", 2037)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("Collections.java", 2034)
    _r0.i = 1;
    XMLVM_SOURCE_POSITION("Collections.java", 2035)
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r3.o, _r1.i, _r5.o);
    goto label1;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_rotate___java_util_List_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_rotate___java_util_List_int]
    XMLVM_ENTER_METHOD("java.util.Collections", "rotate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = n1;
    _r7.i = n2;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 2055)
    XMLVM_SOURCE_POSITION("Collections.java", 2056)
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r6.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2059)
    if (_r0.i != 0) goto label8;
    label7:;
    XMLVM_SOURCE_POSITION("Collections.java", 2096)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2065)
    if (_r7.i <= 0) goto label48;
    XMLVM_SOURCE_POSITION("Collections.java", 2066)
    _r1.i = _r7.i % _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("Collections.java", 2071)
    if (_r1.i == 0) goto label7;
    if (_r1.i == _r0.i) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 2075)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r2.i = XMLVM_ISA(_r6.o, __CLASS_java_util_RandomAccess);
    if (_r2.i == 0) goto label55;
    XMLVM_SOURCE_POSITION("Collections.java", 2078)
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r6.o, _r3.i);
    _r4 = _r3;
    _r5 = _r2;
    _r2 = _r3;
    label27:;
    XMLVM_SOURCE_POSITION("Collections.java", 2080)
    if (_r2.i >= _r0.i) goto label7;
    XMLVM_SOURCE_POSITION("Collections.java", 2081)
    _r4.i = _r4.i + _r1.i;
    _r4.i = _r4.i % _r0.i;
    XMLVM_SOURCE_POSITION("Collections.java", 2082)
    XMLVM_CHECK_NPE(6)
    _r5.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object])(_r6.o, _r4.i, _r5.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2083)
    if (_r4.i != _r3.i) goto label45;
    XMLVM_SOURCE_POSITION("Collections.java", 2084)
    _r3.i = _r3.i + 1;
    XMLVM_SOURCE_POSITION("Collections.java", 2085)
    XMLVM_CHECK_NPE(6)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r6.o, _r3.i);
    _r5 = _r4;
    _r4 = _r3;
    label45:;
    _r2.i = _r2.i + 1;
    goto label27;
    label48:;
    XMLVM_SOURCE_POSITION("Collections.java", 2068)
    _r1.i = _r7.i % _r0.i;
    _r1.i = _r1.i * -1;
    _r1.i = _r0.i - _r1.i;
    goto label12;
    label55:;
    XMLVM_SOURCE_POSITION("Collections.java", 2089)
    _r1.i = _r0.i - _r1.i;
    _r1.i = _r1.i % _r0.i;
    XMLVM_SOURCE_POSITION("Collections.java", 2090)
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_subList___int_int])(_r6.o, _r3.i, _r1.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2091)
    XMLVM_CHECK_NPE(6)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_subList___int_int])(_r6.o, _r1.i, _r0.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2092)
    java_util_Collections_reverse___java_util_List(_r2.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2093)
    java_util_Collections_reverse___java_util_List(_r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2094)
    java_util_Collections_reverse___java_util_List(_r6.o);
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_indexOfSubList___java_util_List_java_util_List(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_indexOfSubList___java_util_List_java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "indexOfSubList", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    _r11.o = n1;
    _r12.o = n2;
    _r10.i = 1;
    _r9.i = 0;
    _r8.i = -1;
    XMLVM_SOURCE_POSITION("Collections.java", 2112)
    XMLVM_CHECK_NPE(11)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r11.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2113)
    XMLVM_CHECK_NPE(12)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r12.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r12.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2115)
    if (_r1.i <= _r0.i) goto label15;
    _r0 = _r8;
    label14:;
    XMLVM_SOURCE_POSITION("Collections.java", 2116)
    XMLVM_SOURCE_POSITION("Collections.java", 2162)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("Collections.java", 2119)
    if (_r1.i != 0) goto label19;
    _r0 = _r9;
    XMLVM_SOURCE_POSITION("Collections.java", 2120)
    goto label14;
    label19:;
    XMLVM_SOURCE_POSITION("Collections.java", 2124)
    XMLVM_CHECK_NPE(12)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r12.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r12.o, _r9.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2125)
    XMLVM_CHECK_NPE(11)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object])(_r11.o, _r2.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2126)
    if (_r3.i != _r8.i) goto label70;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Collections.java", 2127)
    goto label14;
    label31:;
    XMLVM_SOURCE_POSITION("Collections.java", 2131)
    XMLVM_CHECK_NPE(11)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator___int])(_r11.o, _r3.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2133)
    if (_r2.o != JAVA_NULL) goto label58;

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r5.o != JAVA_NULL) goto label68;
    label43:;
    XMLVM_SOURCE_POSITION("Collections.java", 2138)
    XMLVM_CHECK_NPE(12)
    _r5.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r12.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator___int])(_r12.o, _r10.i);
    label47:;
    XMLVM_SOURCE_POSITION("Collections.java", 2140)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.i != 0) goto label78;
    _r4 = _r9;
    label54:;
    XMLVM_SOURCE_POSITION("Collections.java", 2153)
    if (_r4.i != 0) goto label68;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Collections.java", 2154)
    goto label14;
    label58:;
    XMLVM_SOURCE_POSITION("Collections.java", 2134)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r5.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[1])(_r2.o, _r5.o);
    if (_r5.i != 0) goto label43;
    label68:;
    XMLVM_SOURCE_POSITION("Collections.java", 2160)
    _r3.i = _r3.i + 1;
    label70:;
    XMLVM_SOURCE_POSITION("Collections.java", 2130)
    if (_r3.i >= _r0.i) goto label76;
    _r4.i = _r0.i - _r3.i;
    if (_r4.i >= _r1.i) goto label31;
    label76:;
    _r0 = _r8;
    goto label14;
    label78:;
    XMLVM_SOURCE_POSITION("Collections.java", 2141)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 2142)

    
    // Red class access removed: java.util.ListIterator::hasNext
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r7.i != 0) goto label90;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Collections.java", 2143)
    goto label14;
    label90:;
    XMLVM_SOURCE_POSITION("Collections.java", 2145)
    if (_r6.o != JAVA_NULL) goto label100;

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.o == JAVA_NULL) goto label47;
    label98:;
    _r4 = _r10;
    XMLVM_SOURCE_POSITION("Collections.java", 2148)
    goto label54;
    label100:;
    XMLVM_SOURCE_POSITION("Collections.java", 2146)

    
    // Red class access removed: java.util.ListIterator::next
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(6)
    _r6.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[1])(_r6.o, _r7.o);
    if (_r6.i != 0) goto label47;
    goto label98;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_lastIndexOfSubList___java_util_List_java_util_List(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_lastIndexOfSubList___java_util_List_java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "lastIndexOfSubList", "?")
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
    XMLVMElem _r11;
    _r10.o = n1;
    _r11.o = n2;
    _r9.i = 1;
    _r8.i = -1;
    XMLVM_SOURCE_POSITION("Collections.java", 2179)
    XMLVM_CHECK_NPE(11)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r11.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2180)
    XMLVM_CHECK_NPE(10)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r10.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_size__])(_r10.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2182)
    if (_r0.i <= _r1.i) goto label14;
    _r0 = _r8;
    label13:;
    XMLVM_SOURCE_POSITION("Collections.java", 2183)
    XMLVM_SOURCE_POSITION("Collections.java", 2226)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    XMLVM_SOURCE_POSITION("Collections.java", 2186)
    if (_r0.i != 0) goto label18;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Collections.java", 2187)
    goto label13;
    label18:;
    XMLVM_SOURCE_POSITION("Collections.java", 2191)
    _r1.i = _r0.i - _r9.i;
    XMLVM_CHECK_NPE(11)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_get___int])(_r11.o, _r1.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2192)
    XMLVM_CHECK_NPE(10)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r10.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_lastIndexOf___java_lang_Object])(_r10.o, _r1.o);
    label28:;
    XMLVM_SOURCE_POSITION("Collections.java", 2194)
    if (_r2.i <= _r8.i) goto label34;
    _r3.i = _r2.i + 1;
    if (_r3.i >= _r0.i) goto label36;
    label34:;
    _r0 = _r8;
    goto label13;
    label36:;
    XMLVM_SOURCE_POSITION("Collections.java", 2195)
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(10)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r10.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator___int])(_r10.o, _r3.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2197)
    if (_r1.o != JAVA_NULL) goto label71;

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r4.o != JAVA_NULL) goto label81;
    label50:;
    XMLVM_SOURCE_POSITION("Collections.java", 2202)
    _r4.i = _r0.i - _r9.i;
    XMLVM_CHECK_NPE(11)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r11.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_List_listIterator___int])(_r11.o, _r4.i);
    XMLVM_SOURCE_POSITION("Collections.java", 2203)
    _r5.i = 0;
    label57:;
    XMLVM_SOURCE_POSITION("Collections.java", 2204)

    
    // Red class access removed: java.util.ListIterator::hasPrevious
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.i != 0) goto label84;
    _r4 = _r5;
    label64:;
    XMLVM_SOURCE_POSITION("Collections.java", 2217)
    if (_r4.i != 0) goto label81;
    XMLVM_SOURCE_POSITION("Collections.java", 2218)

    
    // Red class access removed: java.util.ListIterator::nextIndex
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label13;
    label71:;
    XMLVM_SOURCE_POSITION("Collections.java", 2198)

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(1)
    _r4.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[1])(_r1.o, _r4.o);
    if (_r4.i != 0) goto label50;
    label81:;
    XMLVM_SOURCE_POSITION("Collections.java", 2224)
    _r2.i = _r2.i + -1;
    goto label28;
    label84:;
    XMLVM_SOURCE_POSITION("Collections.java", 2205)

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 2206)

    
    // Red class access removed: java.util.ListIterator::hasPrevious
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r7.i != 0) goto label96;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Collections.java", 2207)
    goto label13;
    label96:;
    XMLVM_SOURCE_POSITION("Collections.java", 2209)
    if (_r6.o != JAVA_NULL) goto label106;

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r6.o == JAVA_NULL) goto label57;
    label104:;
    _r4 = _r9;
    XMLVM_SOURCE_POSITION("Collections.java", 2212)
    goto label64;
    label106:;
    XMLVM_SOURCE_POSITION("Collections.java", 2210)

    
    // Red class access removed: java.util.ListIterator::previous
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(6)
    _r6.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[1])(_r6.o, _r7.o);
    if (_r6.i != 0) goto label57;
    goto label104;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_list___java_util_Enumeration(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_list___java_util_Enumeration]
    XMLVM_ENTER_METHOD("java.util.Collections", "list", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2239)
    _r0.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(0)
    java_util_ArrayList___INIT___(_r0.o);
    label5:;
    XMLVM_SOURCE_POSITION("Collections.java", 2240)
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r2.o);
    if (_r1.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("Collections.java", 2243)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label12:;
    XMLVM_SOURCE_POSITION("Collections.java", 2241)
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r2.o);
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r0.o)->tib->vtable[7])(_r0.o, _r1.o);
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedCollection___java_util_Collection(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedCollection___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedCollection", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2256)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2257)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2259)

    
    // Red class access removed: java.util.Collections$SynchronizedCollection::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedCollection::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedList___java_util_List(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedList___java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedList", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2271)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2272)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2274)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r1.o, __CLASS_java_util_RandomAccess);
    if (_r0.i == 0) goto label18;
    XMLVM_SOURCE_POSITION("Collections.java", 2275)

    
    // Red class access removed: java.util.Collections$SynchronizedRandomAccessList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedRandomAccessList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label17:;
    XMLVM_SOURCE_POSITION("Collections.java", 2277)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label18:;

    
    // Red class access removed: java.util.Collections$SynchronizedList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedMap___java_util_Map(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedMap___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2289)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2290)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2292)
    _r0.o = __NEW_java_util_Collections_SynchronizedMap();
    XMLVM_CHECK_NPE(0)
    java_util_Collections_SynchronizedMap___INIT____java_util_Map(_r0.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedSet___java_util_Set(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedSet___java_util_Set]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2304)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2305)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2307)

    
    // Red class access removed: java.util.Collections$SynchronizedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedSortedMap___java_util_SortedMap(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedSortedMap___java_util_SortedMap]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedSortedMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2320)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2321)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2323)

    
    // Red class access removed: java.util.Collections$SynchronizedSortedMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedSortedMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_synchronizedSortedSet___java_util_SortedSet(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_synchronizedSortedSet___java_util_SortedSet]
    XMLVM_ENTER_METHOD("java.util.Collections", "synchronizedSortedSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2335)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2336)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2338)

    
    // Red class access removed: java.util.Collections$SynchronizedSortedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedSortedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableCollection___java_util_Collection(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableCollection___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableCollection", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2353)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2354)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2356)

    
    // Red class access removed: java.util.Collections$UnmodifiableCollection::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableCollection::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableList___java_util_List(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableList___java_util_List]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableList", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2370)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2371)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2373)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r1.o, __CLASS_java_util_RandomAccess);
    if (_r0.i == 0) goto label18;
    XMLVM_SOURCE_POSITION("Collections.java", 2374)

    
    // Red class access removed: java.util.Collections$UnmodifiableRandomAccessList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableRandomAccessList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label17:;
    XMLVM_SOURCE_POSITION("Collections.java", 2376)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label18:;

    
    // Red class access removed: java.util.Collections$UnmodifiableList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableMap___java_util_Map(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableMap___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2391)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2392)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2394)

    
    // Red class access removed: java.util.Collections$UnmodifiableMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableSet___java_util_Set(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableSet___java_util_Set]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2408)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2409)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2411)

    
    // Red class access removed: java.util.Collections$UnmodifiableSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableSortedMap___java_util_SortedMap(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableSortedMap___java_util_SortedMap]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableSortedMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2426)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2427)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2429)

    
    // Red class access removed: java.util.Collections$UnmodifiableSortedMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableSortedMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_unmodifiableSortedSet___java_util_SortedSet(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_unmodifiableSortedSet___java_util_SortedSet]
    XMLVM_ENTER_METHOD("java.util.Collections", "unmodifiableSortedSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2442)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Collections.java", 2443)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2445)

    
    // Red class access removed: java.util.Collections$UnmodifiableSortedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$UnmodifiableSortedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_frequency___java_util_Collection_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_frequency___java_util_Collection_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections", "frequency", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.o = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 2463)
    if (_r3.o != JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Collections.java", 2464)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label9:;
    XMLVM_SOURCE_POSITION("Collections.java", 2466)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__])(_r3.o);
    if (_r0.i == 0) goto label17;
    _r0 = _r1;
    label16:;
    XMLVM_SOURCE_POSITION("Collections.java", 2467)
    XMLVM_SOURCE_POSITION("Collections.java", 2477)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label17:;
    XMLVM_SOURCE_POSITION("Collections.java", 2470)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r3.o);
    label21:;
    XMLVM_SOURCE_POSITION("Collections.java", 2471)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label29;
    _r0 = _r1;
    goto label16;
    label29:;
    XMLVM_SOURCE_POSITION("Collections.java", 2472)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_SOURCE_POSITION("Collections.java", 2473)
    if (_r4.o != JAVA_NULL) goto label40;
    if (_r2.o != JAVA_NULL) goto label21;
    label37:;
    XMLVM_SOURCE_POSITION("Collections.java", 2474)
    _r1.i = _r1.i + 1;
    goto label21;
    label40:;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r2.o);
    if (_r2.i == 0) goto label21;
    goto label37;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_emptyList__()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_emptyList__]
    XMLVM_ENTER_METHOD("java.util.Collections", "emptyList", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Collections.java", 2489)
    _r0.o = java_util_Collections_GET_EMPTY_LIST();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_emptySet__()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_emptySet__]
    XMLVM_ENTER_METHOD("java.util.Collections", "emptySet", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Collections.java", 2501)
    _r0.o = java_util_Collections_GET_EMPTY_SET();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_emptyMap__()
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_emptyMap__]
    XMLVM_ENTER_METHOD("java.util.Collections", "emptyMap", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Collections.java", 2513)
    _r0.o = java_util_Collections_GET_EMPTY_MAP();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedCollection___java_util_Collection_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedCollection___java_util_Collection_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedCollection", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2530)

    
    // Red class access removed: java.util.Collections$CheckedCollection::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedCollection::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedMap___java_util_Map_java_lang_Class_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedMap___java_util_Map_java_lang_Class_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = n1;
    _r7.o = n2;
    _r8.o = n3;
    _r4.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Collections.java", 2549)

    
    // Red class access removed: java.util.Collections$CheckedMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1 = _r6;
    _r2 = _r7;
    _r3 = _r8;
    _r5 = _r4;

    
    // Red class access removed: java.util.Collections$CheckedMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedList___java_util_List_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedList___java_util_List_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedList", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2565)
    XMLVM_CLASS_INIT(java_util_RandomAccess)
    _r0.i = XMLVM_ISA(_r1.o, __CLASS_java_util_RandomAccess);
    if (_r0.i == 0) goto label10;
    XMLVM_SOURCE_POSITION("Collections.java", 2566)

    
    // Red class access removed: java.util.Collections$CheckedRandomAccessList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedRandomAccessList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    label9:;
    XMLVM_SOURCE_POSITION("Collections.java", 2568)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;

    
    // Red class access removed: java.util.Collections$CheckedList::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedList::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedSet___java_util_Set_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedSet___java_util_Set_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2584)

    
    // Red class access removed: java.util.Collections$CheckedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedSortedMap___java_util_SortedMap_java_lang_Class_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedSortedMap___java_util_SortedMap_java_lang_Class_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedSortedMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("Collections.java", 2603)

    
    // Red class access removed: java.util.Collections$CheckedSortedMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedSortedMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkedSortedSet___java_util_SortedSet_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkedSortedSet___java_util_SortedSet_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkedSortedSet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2620)

    
    // Red class access removed: java.util.Collections$CheckedSortedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$CheckedSortedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_addAll___java_util_Collection_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_addAll___java_util_Collection_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.util.Collections", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.o = n2;
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("Collections.java", 2642)
    _r1 = _r0;
    label2:;
    XMLVM_SOURCE_POSITION("Collections.java", 2643)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r0.i < _r2.i) goto label6;
    XMLVM_SOURCE_POSITION("Collections.java", 2646)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label6:;
    XMLVM_SOURCE_POSITION("Collections.java", 2644)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object])(_r3.o, _r2.o);
    _r1.i = _r1.i | _r2.i;
    _r0.i = _r0.i + 1;
    goto label2;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_disjoint___java_util_Collection_java_util_Collection(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_disjoint___java_util_Collection_java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.Collections", "disjoint", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2662)
    XMLVM_CLASS_INIT(java_util_Set)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_util_Set);
    if (_r0.i == 0) goto label8;
    XMLVM_CLASS_INIT(java_util_Set)
    _r0.i = XMLVM_ISA(_r4.o, __CLASS_java_util_Set);
    if (_r0.i == 0) goto label18;
    label8:;
    XMLVM_SOURCE_POSITION("Collections.java", 2663)
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r4.o);
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r3.o);
    if (_r0.i <= _r1.i) goto label44;
    label18:;
    _r0 = _r3;
    _r1 = _r4;
    label20:;
    XMLVM_SOURCE_POSITION("Collections.java", 2666)
    XMLVM_SOURCE_POSITION("Collections.java", 2668)
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r1.o);
    label24:;
    XMLVM_SOURCE_POSITION("Collections.java", 2669)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i != 0) goto label32;
    XMLVM_SOURCE_POSITION("Collections.java", 2674)
    _r0.i = 1;
    label31:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label32:;
    XMLVM_SOURCE_POSITION("Collections.java", 2670)
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object])(_r0.o, _r2.o);
    if (_r2.i == 0) goto label24;
    XMLVM_SOURCE_POSITION("Collections.java", 2671)
    _r0.i = 0;
    goto label31;
    label44:;
    _r0 = _r4;
    _r1 = _r3;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_checkType___java_lang_Object_java_lang_Class(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_checkType___java_lang_Object_java_lang_Class]
    XMLVM_ENTER_METHOD("java.util.Collections", "checkType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("Collections.java", 2688)
    if (_r3.o == JAVA_NULL) goto label24;
    XMLVM_CHECK_NPE(4)
    _r0.i = java_lang_Class_isInstance___java_lang_Object(_r4.o, _r3.o);
    if (_r0.i != 0) goto label24;
    XMLVM_SOURCE_POSITION("Collections.java", 2691)

    
    // Red class access removed: java.lang.ClassCastException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Collections.java", 2692)
    // "luni.05"
    _r1.o = xmlvm_create_java_string_from_pool(811);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(3)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[3])(_r3.o);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r4.o);

    
    // Red class access removed: java.lang.ClassCastException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("Collections.java", 2694)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_newSetFromMap___java_util_Map(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_newSetFromMap___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.Collections", "newSetFromMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2711)
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__])(_r1.o);
    if (_r0.i == 0) goto label12;
    XMLVM_SOURCE_POSITION("Collections.java", 2712)

    
    // Red class access removed: java.util.Collections$SetFromMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SetFromMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label12:;
    XMLVM_SOURCE_POSITION("Collections.java", 2714)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_asLifoQueue___java_util_Deque(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_Collections)
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_asLifoQueue___java_util_Deque]
    XMLVM_ENTER_METHOD("java.util.Collections", "asLifoQueue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 2729)

    
    // Red class access removed: java.util.Collections$AsLIFOQueue::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$AsLIFOQueue::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

