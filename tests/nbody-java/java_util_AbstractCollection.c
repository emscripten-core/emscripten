#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_reflect_Array.h"
#include "java_util_Iterator.h"

#include "java_util_AbstractCollection.h"

#define XMLVM_CURRENT_CLASS_NAME AbstractCollection
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_AbstractCollection

__TIB_DEFINITION_java_util_AbstractCollection __TIB_java_util_AbstractCollection = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_AbstractCollection, // classInitializer
    "java.util.AbstractCollection", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/lang/Object;Ljava/util/Collection<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_AbstractCollection), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_AbstractCollection;
JAVA_OBJECT __CLASS_java_util_AbstractCollection_1ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractCollection_2ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractCollection_3ARRAY;
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

void __INIT_java_util_AbstractCollection()
{
    staticInitializerLock(&__TIB_java_util_AbstractCollection);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_AbstractCollection.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_AbstractCollection.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_AbstractCollection);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_AbstractCollection.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_AbstractCollection.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_AbstractCollection.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.AbstractCollection")
        __INIT_IMPL_java_util_AbstractCollection();
    }
}

void __INIT_IMPL_java_util_AbstractCollection()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_AbstractCollection.newInstanceFunc = __NEW_INSTANCE_java_util_AbstractCollection;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_AbstractCollection.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_util_AbstractCollection.vtable[7] = (VTABLE_PTR) &java_util_AbstractCollection_add___java_lang_Object;
    __TIB_java_util_AbstractCollection.vtable[6] = (VTABLE_PTR) &java_util_AbstractCollection_addAll___java_util_Collection;
    __TIB_java_util_AbstractCollection.vtable[8] = (VTABLE_PTR) &java_util_AbstractCollection_clear__;
    __TIB_java_util_AbstractCollection.vtable[10] = (VTABLE_PTR) &java_util_AbstractCollection_contains___java_lang_Object;
    __TIB_java_util_AbstractCollection.vtable[9] = (VTABLE_PTR) &java_util_AbstractCollection_containsAll___java_util_Collection;
    __TIB_java_util_AbstractCollection.vtable[11] = (VTABLE_PTR) &java_util_AbstractCollection_isEmpty__;
    __TIB_java_util_AbstractCollection.vtable[14] = (VTABLE_PTR) &java_util_AbstractCollection_remove___java_lang_Object;
    __TIB_java_util_AbstractCollection.vtable[13] = (VTABLE_PTR) &java_util_AbstractCollection_removeAll___java_util_Collection;
    __TIB_java_util_AbstractCollection.vtable[15] = (VTABLE_PTR) &java_util_AbstractCollection_retainAll___java_util_Collection;
    __TIB_java_util_AbstractCollection.vtable[17] = (VTABLE_PTR) &java_util_AbstractCollection_toArray__;
    __TIB_java_util_AbstractCollection.vtable[18] = (VTABLE_PTR) &java_util_AbstractCollection_toArray___java_lang_Object_1ARRAY;
    __TIB_java_util_AbstractCollection.vtable[5] = (VTABLE_PTR) &java_util_AbstractCollection_toString__;
    // Initialize interface information
    __TIB_java_util_AbstractCollection.numImplementedInterfaces = 2;
    __TIB_java_util_AbstractCollection.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_AbstractCollection.implementedInterfaces[0][0] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_AbstractCollection.implementedInterfaces[0][1] = &__TIB_java_util_Collection;
    // Initialize itable for this class
    __TIB_java_util_AbstractCollection.itableBegin = &__TIB_java_util_AbstractCollection.itable[0];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_AbstractCollection.vtable[12];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_AbstractCollection.vtable[6];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_AbstractCollection.vtable[7];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_AbstractCollection.vtable[8];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_AbstractCollection.vtable[9];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_AbstractCollection.vtable[10];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_AbstractCollection.vtable[1];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_AbstractCollection.vtable[4];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_AbstractCollection.vtable[11];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_AbstractCollection.vtable[12];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_AbstractCollection.vtable[13];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_AbstractCollection.vtable[14];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_AbstractCollection.vtable[15];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_AbstractCollection.vtable[16];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_AbstractCollection.vtable[17];
    __TIB_java_util_AbstractCollection.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_AbstractCollection.vtable[18];


    __TIB_java_util_AbstractCollection.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_AbstractCollection.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_AbstractCollection.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_AbstractCollection.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_AbstractCollection.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_AbstractCollection.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_AbstractCollection.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_AbstractCollection.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_AbstractCollection = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_AbstractCollection);
    __TIB_java_util_AbstractCollection.clazz = __CLASS_java_util_AbstractCollection;
    __TIB_java_util_AbstractCollection.baseType = JAVA_NULL;
    __CLASS_java_util_AbstractCollection_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractCollection);
    __CLASS_java_util_AbstractCollection_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractCollection_1ARRAY);
    __CLASS_java_util_AbstractCollection_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractCollection_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_AbstractCollection]
    //XMLVM_END_WRAPPER

    __TIB_java_util_AbstractCollection.classInitialized = 1;
}

void __DELETE_java_util_AbstractCollection(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_AbstractCollection]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_AbstractCollection(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_AbstractCollection]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_AbstractCollection()
{    XMLVM_CLASS_INIT(java_util_AbstractCollection)
java_util_AbstractCollection* me = (java_util_AbstractCollection*) XMLVM_MALLOC(sizeof(java_util_AbstractCollection));
    me->tib = &__TIB_java_util_AbstractCollection;
    __INIT_INSTANCE_MEMBERS_java_util_AbstractCollection(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_AbstractCollection]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractCollection()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_AbstractCollection();
    java_util_AbstractCollection___INIT___(me);
    return me;
}

void java_util_AbstractCollection___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection___INIT___]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 37)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 38)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_add___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 41)

    
    // Red class access removed: java.lang.UnsupportedOperationException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.UnsupportedOperationException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_addAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 74)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 75)
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r4.o);
    label5:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 76)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 81)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 77)
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    //java_util_AbstractCollection_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[7])(_r3.o, _r2.o);
    if (_r2.i == 0) goto label5;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 78)
    _r0.i = 1;
    goto label5;
    //XMLVM_END_WRAPPER
}

void java_util_AbstractCollection_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_clear__]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 101)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r2.o)->tib->vtable[12])(_r2.o);
    label4:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 102)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label11;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 106)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 103)
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 104)
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r0.o);
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 128)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[12])(_r3.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 129)
    if (_r4.o == JAVA_NULL) goto label35;
    label7:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 130)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label15;
    label13:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 142)
    _r0.i = 0;
    label14:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 131)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i == 0) goto label7;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 132)
    goto label14;
    label27:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 137)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    if (_r1.o != JAVA_NULL) goto label35;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 138)
    goto label14;
    label35:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 136)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label27;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_containsAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_containsAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "containsAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 166)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r3.o);
    label4:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 167)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 172)
    _r0.i = 1;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 168)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    //java_util_AbstractCollection_contains___java_lang_Object[10]
    XMLVM_CHECK_NPE(2)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_AbstractCollection*) _r2.o)->tib->vtable[10])(_r2.o, _r1.o);
    if (_r1.i != 0) goto label4;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 169)
    _r0.i = 0;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 185)
    //java_util_AbstractCollection_size__[16]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r1.o)->tib->vtable[16])(_r1.o);
    if (_r0.i != 0) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 226)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[12])(_r3.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 227)
    if (_r4.o == JAVA_NULL) goto label41;
    label7:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 228)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label15;
    label13:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 242)
    _r0.i = 0;
    label14:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 229)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i == 0) goto label7;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 230)
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r0.o);
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 231)
    goto label14;
    label30:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 236)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    if (_r1.o != JAVA_NULL) goto label41;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 237)
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r0.o);
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 238)
    goto label14;
    label41:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 235)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label30;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_removeAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_removeAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "removeAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 275)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 276)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[12])(_r3.o);
    label5:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 277)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 283)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 278)
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object])(_r4.o, _r2.o);
    if (_r2.i == 0) goto label5;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 279)
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r1.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 280)
    _r0.i = 1;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractCollection_retainAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_retainAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "retainAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 316)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 317)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[12])(_r3.o);
    label5:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 318)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 324)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 319)
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object])(_r4.o, _r2.o);
    if (_r2.i != 0) goto label5;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 320)
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r1.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 321)
    _r0.i = 1;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractCollection_toArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_toArray__]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 340)
    //java_util_AbstractCollection_size__[16]
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r6.o)->tib->vtable[16])(_r6.o);
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 341)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r6.o)->tib->vtable[12])(_r6.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 342)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r0.i);
    label11:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 343)
    if (_r1.i < _r0.i) goto label14;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 346)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label14:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 344)
    _r4.i = _r1.i + 1;
    XMLVM_CHECK_NPE(2)
    _r5.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.o;
    _r1 = _r4;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractCollection_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_toArray___java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 351)
    //java_util_AbstractCollection_size__[16]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r4.o)->tib->vtable[16])(_r4.o);
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 352)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r0.i <= _r2.i) goto label51;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 353)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[3])(_r5.o);
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_Class_getComponentType__(_r2.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 354)
    _r0.o = java_lang_reflect_Array_newInstance___java_lang_Class_int(_r2.o, _r0.i);
    _r0.o = _r0.o;
    label22:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 356)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r4.o)->tib->vtable[12])(_r4.o);
    label26:;
    XMLVM_CHECK_NPE(2)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r2.o);
    if (_r3.i != 0) goto label39;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 359)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i >= _r2.i) goto label38;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 360)
    _r2.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    label38:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 362)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label39:;
    XMLVM_CHECK_NPE(2)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    _r4.o = _r4.o;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 357)
    _r3.i = _r1.i + 1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.o;
    _r1 = _r3;
    goto label26;
    label51:;
    _r0 = _r5;
    goto label22;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_AbstractCollection_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractCollection_toString__]
    XMLVM_ENTER_METHOD("java.util.AbstractCollection", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 374)
    //java_util_AbstractCollection_isEmpty__[11]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[11])(_r3.o);
    if (_r0.i == 0) goto label9;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 375)
    // "[]"
    _r0.o = xmlvm_create_java_string_from_pool(95);
    label8:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 393)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 378)
    _r0.o = __NEW_java_lang_StringBuilder();
    //java_util_AbstractCollection_size__[16]
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[16])(_r3.o);
    _r1.i = _r1.i * 16;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 379)
    _r1.i = 91;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 380)
    //java_util_AbstractCollection_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractCollection*) _r3.o)->tib->vtable[12])(_r3.o);
    label29:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 381)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i != 0) goto label45;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 392)
    _r1.i = 93;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label8;
    label45:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 382)
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 383)
    if (_r2.o == _r3.o) goto label66;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 384)
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r2.o);
    label54:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 388)
    XMLVM_CHECK_NPE(1)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r2.i == 0) goto label29;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 389)
    // ", "
    _r2.o = xmlvm_create_java_string_from_pool(7);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    goto label29;
    label66:;
    XMLVM_SOURCE_POSITION("AbstractCollection.java", 386)
    // "(this Collection)"
    _r2.o = xmlvm_create_java_string_from_pool(96);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r2.o);
    goto label54;
    //XMLVM_END_WRAPPER
}

