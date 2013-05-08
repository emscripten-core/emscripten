#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_util_HashMap.h"

#include "java_util_HashMap_Entry.h"

#define XMLVM_CURRENT_CLASS_NAME HashMap_Entry
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_HashMap_Entry

__TIB_DEFINITION_java_util_HashMap_Entry __TIB_java_util_HashMap_Entry = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_HashMap_Entry, // classInitializer
    "java.util.HashMap$Entry", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/util/MapEntry<TK;TV;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_MapEntry, // extends
    sizeof(java_util_HashMap_Entry), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_HashMap_Entry;
JAVA_OBJECT __CLASS_java_util_HashMap_Entry_1ARRAY;
JAVA_OBJECT __CLASS_java_util_HashMap_Entry_2ARRAY;
JAVA_OBJECT __CLASS_java_util_HashMap_Entry_3ARRAY;
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

void __INIT_java_util_HashMap_Entry()
{
    staticInitializerLock(&__TIB_java_util_HashMap_Entry);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_HashMap_Entry.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_HashMap_Entry.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_HashMap_Entry);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_HashMap_Entry.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_HashMap_Entry.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_HashMap_Entry.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.HashMap$Entry")
        __INIT_IMPL_java_util_HashMap_Entry();
    }
}

void __INIT_IMPL_java_util_HashMap_Entry()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_MapEntry)
    __TIB_java_util_HashMap_Entry.newInstanceFunc = __NEW_INSTANCE_java_util_HashMap_Entry;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_HashMap_Entry.vtable, __TIB_java_util_MapEntry.vtable, sizeof(__TIB_java_util_MapEntry.vtable));
    // Initialize vtable for this class
    __TIB_java_util_HashMap_Entry.vtable[0] = (VTABLE_PTR) &java_util_HashMap_Entry_clone__;
    // Initialize interface information
    __TIB_java_util_HashMap_Entry.numImplementedInterfaces = 2;
    __TIB_java_util_HashMap_Entry.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_HashMap_Entry.implementedInterfaces[0][0] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_util_Map_Entry)

    __TIB_java_util_HashMap_Entry.implementedInterfaces[0][1] = &__TIB_java_util_Map_Entry;
    // Initialize itable for this class
    __TIB_java_util_HashMap_Entry.itableBegin = &__TIB_java_util_HashMap_Entry.itable[0];
    __TIB_java_util_HashMap_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_equals___java_lang_Object] = __TIB_java_util_HashMap_Entry.vtable[1];
    __TIB_java_util_HashMap_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__] = __TIB_java_util_HashMap_Entry.vtable[6];
    __TIB_java_util_HashMap_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__] = __TIB_java_util_HashMap_Entry.vtable[7];
    __TIB_java_util_HashMap_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_hashCode__] = __TIB_java_util_HashMap_Entry.vtable[4];
    __TIB_java_util_HashMap_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_setValue___java_lang_Object] = __TIB_java_util_HashMap_Entry.vtable[8];


    __TIB_java_util_HashMap_Entry.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_HashMap_Entry.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_HashMap_Entry.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_HashMap_Entry.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_HashMap_Entry.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_HashMap_Entry.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_HashMap_Entry.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_HashMap_Entry.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_HashMap_Entry = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_HashMap_Entry);
    __TIB_java_util_HashMap_Entry.clazz = __CLASS_java_util_HashMap_Entry;
    __TIB_java_util_HashMap_Entry.baseType = JAVA_NULL;
    __CLASS_java_util_HashMap_Entry_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap_Entry);
    __CLASS_java_util_HashMap_Entry_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap_Entry_1ARRAY);
    __CLASS_java_util_HashMap_Entry_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap_Entry_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_HashMap_Entry]
    //XMLVM_END_WRAPPER

    __TIB_java_util_HashMap_Entry.classInitialized = 1;
}

void __DELETE_java_util_HashMap_Entry(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_HashMap_Entry]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_HashMap_Entry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_MapEntry(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_HashMap_Entry*) me)->fields.java_util_HashMap_Entry.origKeyHash_ = 0;
    ((java_util_HashMap_Entry*) me)->fields.java_util_HashMap_Entry.next_ = (java_util_HashMap_Entry*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_HashMap_Entry]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_HashMap_Entry()
{    XMLVM_CLASS_INIT(java_util_HashMap_Entry)
java_util_HashMap_Entry* me = (java_util_HashMap_Entry*) XMLVM_MALLOC(sizeof(java_util_HashMap_Entry));
    me->tib = &__TIB_java_util_HashMap_Entry;
    __INIT_INSTANCE_MEMBERS_java_util_HashMap_Entry(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_HashMap_Entry]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_HashMap_Entry()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_util_HashMap_Entry___INIT____java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_Entry___INIT____java_lang_Object_int]
    XMLVM_ENTER_METHOD("java.util.HashMap$Entry", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("HashMap.java", 74)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    java_util_MapEntry___INIT____java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 75)
    XMLVM_CHECK_NPE(1)
    ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.origKeyHash_ = _r3.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 76)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_Entry___INIT____java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_Entry___INIT____java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap$Entry", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("HashMap.java", 79)
    XMLVM_CHECK_NPE(1)
    java_util_MapEntry___INIT____java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 80)
    if (_r2.o != JAVA_NULL) goto label9;
    _r0.i = 0;
    label6:;
    XMLVM_CHECK_NPE(1)
    ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.origKeyHash_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 81)
    XMLVM_EXIT_METHOD()
    return;
    label9:;
    _r0.i = java_util_HashMap_computeHashCode___java_lang_Object(_r2.o);
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_Entry_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_Entry_clone__]
    XMLVM_ENTER_METHOD("java.util.HashMap$Entry", "clone", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 86)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_MapEntry_clone__(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 87)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap_Entry*) _r2.o)->fields.java_util_HashMap_Entry.next_;
    if (_r1.o == JAVA_NULL) goto label20;
    XMLVM_SOURCE_POSITION("HashMap.java", 88)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap_Entry*) _r2.o)->fields.java_util_HashMap_Entry.next_;
    //java_util_HashMap_Entry_clone__[0]
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_HashMap_Entry*) _r1.o)->tib->vtable[0])(_r1.o);
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_ = _r2.o;
    label20:;
    XMLVM_SOURCE_POSITION("HashMap.java", 90)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

