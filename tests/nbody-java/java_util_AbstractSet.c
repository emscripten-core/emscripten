#include "xmlvm.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_util_Collection.h"
#include "java_util_Iterator.h"

#include "java_util_AbstractSet.h"

#define XMLVM_CURRENT_CLASS_NAME AbstractSet
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_AbstractSet

__TIB_DEFINITION_java_util_AbstractSet __TIB_java_util_AbstractSet = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_AbstractSet, // classInitializer
    "java.util.AbstractSet", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/util/AbstractCollection<TE;>;Ljava/util/Set<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractCollection, // extends
    sizeof(java_util_AbstractSet), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_AbstractSet;
JAVA_OBJECT __CLASS_java_util_AbstractSet_1ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractSet_2ARRAY;
JAVA_OBJECT __CLASS_java_util_AbstractSet_3ARRAY;
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

void __INIT_java_util_AbstractSet()
{
    staticInitializerLock(&__TIB_java_util_AbstractSet);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_AbstractSet.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_AbstractSet.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_AbstractSet);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_AbstractSet.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_AbstractSet.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_AbstractSet.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.AbstractSet")
        __INIT_IMPL_java_util_AbstractSet();
    }
}

void __INIT_IMPL_java_util_AbstractSet()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractCollection)
    __TIB_java_util_AbstractSet.newInstanceFunc = __NEW_INSTANCE_java_util_AbstractSet;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_AbstractSet.vtable, __TIB_java_util_AbstractCollection.vtable, sizeof(__TIB_java_util_AbstractCollection.vtable));
    // Initialize vtable for this class
    __TIB_java_util_AbstractSet.vtable[1] = (VTABLE_PTR) &java_util_AbstractSet_equals___java_lang_Object;
    __TIB_java_util_AbstractSet.vtable[4] = (VTABLE_PTR) &java_util_AbstractSet_hashCode__;
    __TIB_java_util_AbstractSet.vtable[13] = (VTABLE_PTR) &java_util_AbstractSet_removeAll___java_util_Collection;
    // Initialize interface information
    __TIB_java_util_AbstractSet.numImplementedInterfaces = 3;
    __TIB_java_util_AbstractSet.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_AbstractSet.implementedInterfaces[0][0] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_AbstractSet.implementedInterfaces[0][1] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_Set)

    __TIB_java_util_AbstractSet.implementedInterfaces[0][2] = &__TIB_java_util_Set;
    // Initialize itable for this class
    __TIB_java_util_AbstractSet.itableBegin = &__TIB_java_util_AbstractSet.itable[0];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_AbstractSet.vtable[12];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[6];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[7];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_AbstractSet.vtable[8];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[9];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[10];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[1];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_AbstractSet.vtable[4];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_AbstractSet.vtable[11];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_AbstractSet.vtable[12];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[13];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[14];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[15];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_AbstractSet.vtable[16];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_AbstractSet.vtable[17];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_AbstractSet.vtable[18];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_addAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[6];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_add___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[7];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_clear__] = __TIB_java_util_AbstractSet.vtable[8];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_containsAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[9];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_contains___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[10];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_equals___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[1];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_hashCode__] = __TIB_java_util_AbstractSet.vtable[4];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_isEmpty__] = __TIB_java_util_AbstractSet.vtable[11];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_iterator__] = __TIB_java_util_AbstractSet.vtable[12];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_removeAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[13];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_remove___java_lang_Object] = __TIB_java_util_AbstractSet.vtable[14];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_retainAll___java_util_Collection] = __TIB_java_util_AbstractSet.vtable[15];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_size__] = __TIB_java_util_AbstractSet.vtable[16];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray__] = __TIB_java_util_AbstractSet.vtable[17];
    __TIB_java_util_AbstractSet.itable[XMLVM_ITABLE_IDX_java_util_Set_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_AbstractSet.vtable[18];


    __TIB_java_util_AbstractSet.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_AbstractSet.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_AbstractSet.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_AbstractSet.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_AbstractSet.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_AbstractSet.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_AbstractSet.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_AbstractSet.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_AbstractSet = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_AbstractSet);
    __TIB_java_util_AbstractSet.clazz = __CLASS_java_util_AbstractSet;
    __TIB_java_util_AbstractSet.baseType = JAVA_NULL;
    __CLASS_java_util_AbstractSet_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSet);
    __CLASS_java_util_AbstractSet_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSet_1ARRAY);
    __CLASS_java_util_AbstractSet_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_AbstractSet_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_AbstractSet]
    //XMLVM_END_WRAPPER

    __TIB_java_util_AbstractSet.classInitialized = 1;
}

void __DELETE_java_util_AbstractSet(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_AbstractSet]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_AbstractSet(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractCollection(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_AbstractSet]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_AbstractSet()
{    XMLVM_CLASS_INIT(java_util_AbstractSet)
java_util_AbstractSet* me = (java_util_AbstractSet*) XMLVM_MALLOC(sizeof(java_util_AbstractSet));
    me->tib = &__TIB_java_util_AbstractSet;
    __INIT_INSTANCE_MEMBERS_java_util_AbstractSet(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_AbstractSet]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_AbstractSet()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_AbstractSet();
    java_util_AbstractSet___INIT___(me);
    return me;
}

void java_util_AbstractSet___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSet___INIT___]
    XMLVM_ENTER_METHOD("java.util.AbstractSet", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 34)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractCollection___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractSet.java", 35)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractSet_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSet_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.AbstractSet", "equals", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 50)
    if (_r4.o != _r5.o) goto label6;
    _r0 = _r3;
    label5:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 51)
    XMLVM_SOURCE_POSITION("AbstractSet.java", 64)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 53)
    XMLVM_CLASS_INIT(java_util_Set)
    _r0.i = XMLVM_ISA(_r5.o, __CLASS_java_util_Set);
    if (_r0.i == 0) goto label38;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 54)
    _r5.o = _r5.o;
    XMLVM_TRY_BEGIN(w1612aaab1b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("AbstractSet.java", 57)
    //java_util_AbstractSet_size__[16]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractSet*) _r4.o)->tib->vtable[16])(_r4.o);
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_size__])(_r5.o);
    if (_r0.i != _r1.i) { XMLVM_MEMCPY(curThread_w1612aaab1b1c18->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1612aaab1b1c18, sizeof(XMLVM_JMP_BUF)); goto label30; };
    //java_util_AbstractSet_containsAll___java_util_Collection[9]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_AbstractSet*) _r4.o)->tib->vtable[9])(_r4.o, _r5.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1612aaab1b1c18)
        XMLVM_CATCH_SPECIFIC(w1612aaab1b1c18,java_lang_NullPointerException,32)
    XMLVM_CATCH_END(w1612aaab1b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w1612aaab1b1c18)
    if (_r0.i == 0) goto label30;
    _r0 = _r3;
    goto label5;
    label30:;
    _r0 = _r2;
    goto label5;
    label32:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 59)
    java_lang_Thread* curThread_w1612aaab1b1c27 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1612aaab1b1c27->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r2;
    goto label5;
    label35:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 61)
    java_lang_Thread* curThread_w1612aaab1b1c32 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1612aaab1b1c32->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r2;
    goto label5;
    label38:;
    _r0 = _r2;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_AbstractSet_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSet_hashCode__]
    XMLVM_ENTER_METHOD("java.util.AbstractSet", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 77)
    XMLVM_SOURCE_POSITION("AbstractSet.java", 78)
    //java_util_AbstractSet_iterator__[12]
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractSet*) _r4.o)->tib->vtable[12])(_r4.o);
    _r1 = _r3;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 79)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label13;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 83)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label13:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 80)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractSet.java", 81)
    if (_r2.o != JAVA_NULL) goto label22;
    _r2 = _r3;
    label20:;
    _r1.i = _r1.i + _r2.i;
    goto label6;
    label22:;
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[4])(_r2.o);
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_AbstractSet_removeAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_AbstractSet_removeAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.AbstractSet", "removeAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 99)
    XMLVM_SOURCE_POSITION("AbstractSet.java", 100)
    //java_util_AbstractSet_size__[16]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_AbstractSet*) _r5.o)->tib->vtable[16])(_r5.o);
    XMLVM_CHECK_NPE(6)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_size__])(_r6.o);
    if (_r0.i > _r1.i) goto label40;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 101)
    //java_util_AbstractSet_iterator__[12]
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_AbstractSet*) _r5.o)->tib->vtable[12])(_r5.o);
    _r1 = _r3;
    label17:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 102)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label25;
    _r0 = _r1;
    label24:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 114)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label25:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 103)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    XMLVM_CHECK_NPE(6)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object])(_r6.o, _r2.o);
    if (_r2.i == 0) goto label17;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 104)
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_remove__])(_r0.o);
    _r1 = _r4;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 105)
    goto label17;
    label40:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 109)
    XMLVM_CHECK_NPE(6)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_iterator__])(_r6.o);
    _r1 = _r3;
    label45:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 110)
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r2.i != 0) goto label53;
    _r0 = _r1;
    goto label24;
    label53:;
    XMLVM_SOURCE_POSITION("AbstractSet.java", 111)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    //java_util_AbstractSet_remove___java_lang_Object[14]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_AbstractSet*) _r5.o)->tib->vtable[14])(_r5.o, _r2.o);
    if (_r2.i != 0) goto label67;
    if (_r1.i != 0) goto label67;
    _r1 = _r3;
    goto label45;
    label67:;
    _r1 = _r4;
    goto label45;
    //XMLVM_END_WRAPPER
}

