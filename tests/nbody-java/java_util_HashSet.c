#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_util_Collection.h"
#include "java_util_HashMap.h"
#include "java_util_HashMap_Entry.h"
#include "java_util_Iterator.h"

#include "java_util_HashSet.h"

#define XMLVM_CURRENT_CLASS_NAME HashSet
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_HashSet

__TIB_DEFINITION_java_util_HashSet __TIB_java_util_HashSet = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_HashSet, // classInitializer
    "java.util.HashSet", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/util/AbstractSet<TE;>;Ljava/util/Set<TE;>;Ljava/lang/Cloneable;Ljava/io/Serializable;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractSet, // extends
    sizeof(java_util_HashSet), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_HashSet;
JAVA_OBJECT __CLASS_java_util_HashSet_1ARRAY;
JAVA_OBJECT __CLASS_java_util_HashSet_2ARRAY;
JAVA_OBJECT __CLASS_java_util_HashSet_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_HashSet_serialVersionUID;

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

void __INIT_java_util_HashSet()
{
    staticInitializerLock(&__TIB_java_util_HashSet);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_HashSet.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_HashSet.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_HashSet);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_HashSet.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_HashSet.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_HashSet.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.HashSet")
        __INIT_IMPL_java_util_HashSet();
    }
}

void __INIT_IMPL_java_util_HashSet()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractSet)
    __TIB_java_util_HashSet.newInstanceFunc = __NEW_INSTANCE_java_util_HashSet;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_HashSet.vtable, __TIB_java_util_AbstractSet.vtable, sizeof(__TIB_java_util_AbstractSet.vtable));
    // Initialize vtable for this class
    __TIB_java_util_HashSet.vtable[7] = (VTABLE_PTR) &java_util_HashSet_add___java_lang_Object;
    __TIB_java_util_HashSet.vtable[8] = (VTABLE_PTR) &java_util_HashSet_clear__;
    __TIB_java_util_HashSet.vtable[0] = (VTABLE_PTR) &java_util_HashSet_clone__;
    __TIB_java_util_HashSet.vtable[10] = (VTABLE_PTR) &java_util_HashSet_contains___java_lang_Object;
    __TIB_java_util_HashSet.vtable[11] = (VTABLE_PTR) &java_util_HashSet_isEmpty__;
    __TIB_java_util_HashSet.vtable[12] = (VTABLE_PTR) &java_util_HashSet_iterator__;
    __TIB_java_util_HashSet.vtable[14] = (VTABLE_PTR) &java_util_HashSet_remove___java_lang_Object;
    __TIB_java_util_HashSet.vtable[16] = (VTABLE_PTR) &java_util_HashSet_size__;
    // Initialize interface information
    __TIB_java_util_HashSet.numImplementedInterfaces = 5;
    __TIB_java_util_HashSet.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 5);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_HashSet.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_HashSet.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_HashSet.implementedInterfaces[0][2] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_HashSet.implementedInterfaces[0][3] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_Set)

    __TIB_java_util_HashSet.implementedInterfaces[0][4] = &__TIB_java_util_Set;
    // Initialize itable for this class
    __TIB_java_util_HashSet.itableBegin = &__TIB_java_util_HashSet.itable[0];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_HashSet.vtable[12];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[6];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_HashSet.vtable[7];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_HashSet.vtable[8];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[9];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_HashSet.vtable[10];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_HashSet.vtable[1];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_HashSet.vtable[4];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_HashSet.vtable[11];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_HashSet.vtable[12];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[13];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_HashSet.vtable[14];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[15];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_HashSet.vtable[16];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_HashSet.vtable[17];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_HashSet.vtable[18];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_addAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[6];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_add___java_lang_Object] = __TIB_java_util_HashSet.vtable[7];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_clear__] = __TIB_java_util_HashSet.vtable[8];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_containsAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[9];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_contains___java_lang_Object] = __TIB_java_util_HashSet.vtable[10];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_equals___java_lang_Object] = __TIB_java_util_HashSet.vtable[1];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_hashCode__] = __TIB_java_util_HashSet.vtable[4];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_isEmpty__] = __TIB_java_util_HashSet.vtable[11];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_iterator__] = __TIB_java_util_HashSet.vtable[12];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_removeAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[13];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_remove___java_lang_Object] = __TIB_java_util_HashSet.vtable[14];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_retainAll___java_util_Collection] = __TIB_java_util_HashSet.vtable[15];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_size__] = __TIB_java_util_HashSet.vtable[16];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray__] = __TIB_java_util_HashSet.vtable[17];
    __TIB_java_util_HashSet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_HashSet.vtable[18];

    _STATIC_java_util_HashSet_serialVersionUID = -5024744406713321676;

    __TIB_java_util_HashSet.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_HashSet.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_HashSet.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_HashSet.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_HashSet.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_HashSet.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_HashSet.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_HashSet.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_HashSet = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_HashSet);
    __TIB_java_util_HashSet.clazz = __CLASS_java_util_HashSet;
    __TIB_java_util_HashSet.baseType = JAVA_NULL;
    __CLASS_java_util_HashSet_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashSet);
    __CLASS_java_util_HashSet_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashSet_1ARRAY);
    __CLASS_java_util_HashSet_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashSet_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_HashSet]
    //XMLVM_END_WRAPPER

    __TIB_java_util_HashSet.classInitialized = 1;
}

void __DELETE_java_util_HashSet(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_HashSet]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_HashSet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractSet(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_HashSet*) me)->fields.java_util_HashSet.backingMap_ = (java_util_HashMap*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_HashSet]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_HashSet()
{    XMLVM_CLASS_INIT(java_util_HashSet)
java_util_HashSet* me = (java_util_HashSet*) XMLVM_MALLOC(sizeof(java_util_HashSet));
    me->tib = &__TIB_java_util_HashSet;
    __INIT_INSTANCE_MEMBERS_java_util_HashSet(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_HashSet]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_HashSet()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_HashSet();
    java_util_HashSet___INIT___(me);
    return me;
}

JAVA_LONG java_util_HashSet_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_HashSet)
    return _STATIC_java_util_HashSet_serialVersionUID;
}

void java_util_HashSet_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_HashSet)
_STATIC_java_util_HashSet_serialVersionUID = v;
}

void java_util_HashSet___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet___INIT___]
    XMLVM_ENTER_METHOD("java.util.HashSet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashSet.java", 40)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    XMLVM_CHECK_NPE(1)
    java_util_HashSet___INIT____java_util_HashMap(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 41)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet___INIT____int]
    XMLVM_ENTER_METHOD("java.util.HashSet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 50)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT____int(_r0.o, _r2.i);
    XMLVM_CHECK_NPE(1)
    java_util_HashSet___INIT____java_util_HashMap(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 51)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet___INIT____int_float]
    XMLVM_ENTER_METHOD("java.util.HashSet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("HashSet.java", 63)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT____int_float(_r0.o, _r2.i, _r3.f);
    XMLVM_CHECK_NPE(1)
    java_util_HashSet___INIT____java_util_HashMap(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 64)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet___INIT____java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.HashSet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 74)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r4.o);
    _r2.i = 6;
    if (_r1.i >= _r2.i) goto label28;
    _r1.i = 11;
    label11:;
    XMLVM_SOURCE_POSITION("HashSet.java", 75)
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT____int(_r0.o, _r1.i);
    XMLVM_CHECK_NPE(3)
    java_util_HashSet___INIT____java_util_HashMap(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 76)
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r4.o);
    label21:;
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r0.i != 0) goto label35;
    XMLVM_SOURCE_POSITION("HashSet.java", 79)
    XMLVM_EXIT_METHOD()
    return;
    label28:;
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r4.o);
    _r1.i = _r1.i * 2;
    goto label11;
    label35:;
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("HashSet.java", 77)
    //java_util_HashSet_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashSet*) _r3.o)->tib->vtable[7])(_r3.o, _r0.o);
    goto label21;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet___INIT____java_util_HashMap(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet___INIT____java_util_HashMap]
    XMLVM_ENTER_METHOD("java.util.HashSet", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 81)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractSet___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 82)
    XMLVM_CHECK_NPE(0)
    ((java_util_HashSet*) _r0.o)->fields.java_util_HashSet.backingMap_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashSet.java", 83)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashSet_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_add___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashSet", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 95)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[14])(_r0.o, _r2.o, _r1.o);
    if (_r0.o != JAVA_NULL) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 0;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_clear__]
    XMLVM_ENTER_METHOD("java.util.HashSet", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashSet.java", 106)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_clear__[6]
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[6])(_r0.o);
    XMLVM_SOURCE_POSITION("HashSet.java", 107)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashSet_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_clone__]
    XMLVM_ENTER_METHOD("java.util.HashSet", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_TRY_BEGIN(w6279aaab9b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("HashSet.java", 120)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_Object_clone__(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("HashSet.java", 121)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashSet*) _r2.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_clone__[0]
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r1.o)->tib->vtable[0])(_r1.o);
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(0)
    ((java_util_HashSet*) _r0.o)->fields.java_util_HashSet.backingMap_ = _r2.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6279aaab9b1b2)
    XMLVM_CATCH_END(w6279aaab9b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w6279aaab9b1b2)
    label16:;
    XMLVM_SOURCE_POSITION("HashSet.java", 124)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label17:;
    java_lang_Thread* curThread_w6279aaab9b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w6279aaab9b1b7->fields.java_lang_Thread.xmlvmException_;
    _r0.o = JAVA_NULL;
    goto label16;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashSet_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashSet", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 138)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_containsKey___java_lang_Object[7]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[7])(_r0.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashSet_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.HashSet", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashSet.java", 150)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_isEmpty__[11]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[11])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashSet_iterator__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_iterator__]
    XMLVM_ENTER_METHOD("java.util.HashSet", "iterator", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashSet.java", 161)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_keySet__[12]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[12])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashSet_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashSet", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 173)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_remove___java_lang_Object[15]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[15])(_r0.o, _r2.o);
    if (_r0.o == JAVA_NULL) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 0;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_HashSet_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_size__]
    XMLVM_ENTER_METHOD("java.util.HashSet", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashSet.java", 183)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashSet*) _r1.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_size__[16]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[16])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.HashSet", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 187)

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 188)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 189)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    XMLVM_CHECK_NPE(0)
    _r0.f = ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.loadFactor_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeFloat
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 190)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    XMLVM_CHECK_NPE(0)
    _r0.i = ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.elementCount_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 191)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    label30:;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label35;
    XMLVM_SOURCE_POSITION("HashSet.java", 198)
    XMLVM_EXIT_METHOD()
    return;
    label35:;
    XMLVM_SOURCE_POSITION("HashSet.java", 192)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_HashMap*) _r1.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label41:;
    XMLVM_SOURCE_POSITION("HashSet.java", 193)
    if (_r1.o == JAVA_NULL) goto label30;
    XMLVM_SOURCE_POSITION("HashSet.java", 194)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.key_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 195)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    goto label41;
    //XMLVM_END_WRAPPER
}

void java_util_HashSet_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.HashSet", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("HashSet.java", 203)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 204)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 205)

    
    // Red class access removed: java.io.ObjectInputStream::readFloat
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 206)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashSet_createBackingMap___int_float(_r3.o, _r0.i, _r1.f);
    XMLVM_CHECK_NPE(3)
    ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_ = _r0.o;
    XMLVM_SOURCE_POSITION("HashSet.java", 207)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    label21:;
    XMLVM_SOURCE_POSITION("HashSet.java", 208)
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label26;
    XMLVM_SOURCE_POSITION("HashSet.java", 212)
    XMLVM_EXIT_METHOD()
    return;
    label26:;
    XMLVM_SOURCE_POSITION("HashSet.java", 209)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashSet.java", 210)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_HashSet*) _r3.o)->fields.java_util_HashSet.backingMap_;
    //java_util_HashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r2.o)->tib->vtable[14])(_r2.o, _r1.o, _r3.o);
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashSet_createBackingMap___int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashSet_createBackingMap___int_float]
    XMLVM_ENTER_METHOD("java.util.HashSet", "createBackingMap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("HashSet.java", 215)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT____int_float(_r0.o, _r2.i, _r3.f);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

