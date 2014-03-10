#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_util_Collections.h"
#include "java_util_Iterator.h"
#include "java_util_Set.h"

#include "java_util_Collections_EmptySet.h"

#define XMLVM_CURRENT_CLASS_NAME Collections_EmptySet
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Collections_EmptySet

__TIB_DEFINITION_java_util_Collections_EmptySet __TIB_java_util_Collections_EmptySet = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Collections_EmptySet, // classInitializer
    "java.util.Collections$EmptySet", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractSet, // extends
    sizeof(java_util_Collections_EmptySet), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Collections_EmptySet;
JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptySet_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Collections_EmptySet_serialVersionUID;

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

void __INIT_java_util_Collections_EmptySet()
{
    staticInitializerLock(&__TIB_java_util_Collections_EmptySet);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Collections_EmptySet.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Collections_EmptySet.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Collections_EmptySet);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Collections_EmptySet.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Collections_EmptySet.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Collections_EmptySet.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Collections$EmptySet")
        __INIT_IMPL_java_util_Collections_EmptySet();
    }
}

void __INIT_IMPL_java_util_Collections_EmptySet()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractSet)
    __TIB_java_util_Collections_EmptySet.newInstanceFunc = __NEW_INSTANCE_java_util_Collections_EmptySet;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Collections_EmptySet.vtable, __TIB_java_util_AbstractSet.vtable, sizeof(__TIB_java_util_AbstractSet.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Collections_EmptySet.vtable[10] = (VTABLE_PTR) &java_util_Collections_EmptySet_contains___java_lang_Object;
    __TIB_java_util_Collections_EmptySet.vtable[16] = (VTABLE_PTR) &java_util_Collections_EmptySet_size__;
    __TIB_java_util_Collections_EmptySet.vtable[12] = (VTABLE_PTR) &java_util_Collections_EmptySet_iterator__;
    // Initialize interface information
    __TIB_java_util_Collections_EmptySet.numImplementedInterfaces = 4;
    __TIB_java_util_Collections_EmptySet.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 4);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Collections_EmptySet.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_Collections_EmptySet.implementedInterfaces[0][1] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_Collections_EmptySet.implementedInterfaces[0][2] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_Set)

    __TIB_java_util_Collections_EmptySet.implementedInterfaces[0][3] = &__TIB_java_util_Set;
    // Initialize itable for this class
    __TIB_java_util_Collections_EmptySet.itableBegin = &__TIB_java_util_Collections_EmptySet.itable[0];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_Collections_EmptySet.vtable[12];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[6];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[7];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_Collections_EmptySet.vtable[8];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[9];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[10];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[1];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_Collections_EmptySet.vtable[4];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_Collections_EmptySet.vtable[11];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_Collections_EmptySet.vtable[12];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[13];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[14];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[15];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_Collections_EmptySet.vtable[16];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_Collections_EmptySet.vtable[17];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_Collections_EmptySet.vtable[18];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_addAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[6];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_add___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[7];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_clear__] = __TIB_java_util_Collections_EmptySet.vtable[8];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_containsAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[9];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_contains___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[10];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_equals___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[1];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_hashCode__] = __TIB_java_util_Collections_EmptySet.vtable[4];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_isEmpty__] = __TIB_java_util_Collections_EmptySet.vtable[11];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_iterator__] = __TIB_java_util_Collections_EmptySet.vtable[12];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_removeAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[13];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_remove___java_lang_Object] = __TIB_java_util_Collections_EmptySet.vtable[14];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_retainAll___java_util_Collection] = __TIB_java_util_Collections_EmptySet.vtable[15];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_size__] = __TIB_java_util_Collections_EmptySet.vtable[16];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray__] = __TIB_java_util_Collections_EmptySet.vtable[17];
    __TIB_java_util_Collections_EmptySet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_Collections_EmptySet.vtable[18];

    _STATIC_java_util_Collections_EmptySet_serialVersionUID = 1582296315990362920;

    __TIB_java_util_Collections_EmptySet.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Collections_EmptySet.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptySet.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Collections_EmptySet.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Collections_EmptySet.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptySet.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Collections_EmptySet.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Collections_EmptySet.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Collections_EmptySet = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Collections_EmptySet);
    __TIB_java_util_Collections_EmptySet.clazz = __CLASS_java_util_Collections_EmptySet;
    __TIB_java_util_Collections_EmptySet.baseType = JAVA_NULL;
    __CLASS_java_util_Collections_EmptySet_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptySet);
    __CLASS_java_util_Collections_EmptySet_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptySet_1ARRAY);
    __CLASS_java_util_Collections_EmptySet_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptySet_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Collections_EmptySet]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Collections_EmptySet.classInitialized = 1;
}

void __DELETE_java_util_Collections_EmptySet(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Collections_EmptySet]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptySet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractSet(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Collections_EmptySet]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Collections_EmptySet()
{    XMLVM_CLASS_INIT(java_util_Collections_EmptySet)
java_util_Collections_EmptySet* me = (java_util_Collections_EmptySet*) XMLVM_MALLOC(sizeof(java_util_Collections_EmptySet));
    me->tib = &__TIB_java_util_Collections_EmptySet;
    __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptySet(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Collections_EmptySet]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptySet()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_util_Collections_EmptySet_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptySet)
    return _STATIC_java_util_Collections_EmptySet_serialVersionUID;
}

void java_util_Collections_EmptySet_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptySet)
_STATIC_java_util_Collections_EmptySet_serialVersionUID = v;
}

void java_util_Collections_EmptySet___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet___INIT___]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 99)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractSet___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_EmptySet_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 105)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Collections_EmptySet_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet_size__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 110)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptySet_iterator__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet_iterator__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "iterator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 115)

    
    // Red class access removed: java.util.Collections$EmptySet$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$EmptySet$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptySet_readResolve__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet_readResolve__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "readResolve", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 131)
    _r0.o = java_util_Collections_GET_EMPTY_SET();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_EmptySet___INIT____java_util_Collections_EmptySet(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptySet___INIT____java_util_Collections_EmptySet]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptySet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 99)
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptySet___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

