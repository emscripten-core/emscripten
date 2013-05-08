#include "xmlvm.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"

#include "java_util_Hashtable_Entry.h"

#define XMLVM_CURRENT_CLASS_NAME Hashtable_Entry
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Hashtable_Entry

__TIB_DEFINITION_java_util_Hashtable_Entry __TIB_java_util_Hashtable_Entry = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Hashtable_Entry, // classInitializer
    "java.util.Hashtable$Entry", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/util/MapEntry<TK;TV;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_MapEntry, // extends
    sizeof(java_util_Hashtable_Entry), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Hashtable_Entry;
JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_3ARRAY;
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

void __INIT_java_util_Hashtable_Entry()
{
    staticInitializerLock(&__TIB_java_util_Hashtable_Entry);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Hashtable_Entry.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Hashtable_Entry.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Hashtable_Entry);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Hashtable_Entry.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Hashtable_Entry.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Hashtable_Entry.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Hashtable$Entry")
        __INIT_IMPL_java_util_Hashtable_Entry();
    }
}

void __INIT_IMPL_java_util_Hashtable_Entry()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_MapEntry)
    __TIB_java_util_Hashtable_Entry.newInstanceFunc = __NEW_INSTANCE_java_util_Hashtable_Entry;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Hashtable_Entry.vtable, __TIB_java_util_MapEntry.vtable, sizeof(__TIB_java_util_MapEntry.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Hashtable_Entry.vtable[0] = (VTABLE_PTR) &java_util_Hashtable_Entry_clone__;
    __TIB_java_util_Hashtable_Entry.vtable[8] = (VTABLE_PTR) &java_util_Hashtable_Entry_setValue___java_lang_Object;
    __TIB_java_util_Hashtable_Entry.vtable[5] = (VTABLE_PTR) &java_util_Hashtable_Entry_toString__;
    // Initialize interface information
    __TIB_java_util_Hashtable_Entry.numImplementedInterfaces = 2;
    __TIB_java_util_Hashtable_Entry.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_Hashtable_Entry.implementedInterfaces[0][0] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_util_Map_Entry)

    __TIB_java_util_Hashtable_Entry.implementedInterfaces[0][1] = &__TIB_java_util_Map_Entry;
    // Initialize itable for this class
    __TIB_java_util_Hashtable_Entry.itableBegin = &__TIB_java_util_Hashtable_Entry.itable[0];
    __TIB_java_util_Hashtable_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_equals___java_lang_Object] = __TIB_java_util_Hashtable_Entry.vtable[1];
    __TIB_java_util_Hashtable_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__] = __TIB_java_util_Hashtable_Entry.vtable[6];
    __TIB_java_util_Hashtable_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__] = __TIB_java_util_Hashtable_Entry.vtable[7];
    __TIB_java_util_Hashtable_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_hashCode__] = __TIB_java_util_Hashtable_Entry.vtable[4];
    __TIB_java_util_Hashtable_Entry.itable[XMLVM_ITABLE_IDX_java_util_Map_Entry_setValue___java_lang_Object] = __TIB_java_util_Hashtable_Entry.vtable[8];


    __TIB_java_util_Hashtable_Entry.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Hashtable_Entry.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Hashtable_Entry.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Hashtable_Entry.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Hashtable_Entry.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Hashtable_Entry.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Hashtable_Entry.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Hashtable_Entry.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Hashtable_Entry = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Hashtable_Entry);
    __TIB_java_util_Hashtable_Entry.clazz = __CLASS_java_util_Hashtable_Entry;
    __TIB_java_util_Hashtable_Entry.baseType = JAVA_NULL;
    __CLASS_java_util_Hashtable_Entry_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_Entry);
    __CLASS_java_util_Hashtable_Entry_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_Entry_1ARRAY);
    __CLASS_java_util_Hashtable_Entry_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_Entry_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Hashtable_Entry]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Hashtable_Entry.classInitialized = 1;
}

void __DELETE_java_util_Hashtable_Entry(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Hashtable_Entry]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Hashtable_Entry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_MapEntry(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_Hashtable_Entry*) me)->fields.java_util_Hashtable_Entry.next_ = (java_util_Hashtable_Entry*) JAVA_NULL;
    ((java_util_Hashtable_Entry*) me)->fields.java_util_Hashtable_Entry.hashcode_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Hashtable_Entry]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Hashtable_Entry()
{    XMLVM_CLASS_INIT(java_util_Hashtable_Entry)
java_util_Hashtable_Entry* me = (java_util_Hashtable_Entry*) XMLVM_MALLOC(sizeof(java_util_Hashtable_Entry));
    me->tib = &__TIB_java_util_Hashtable_Entry;
    __INIT_INSTANCE_MEMBERS_java_util_Hashtable_Entry(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Hashtable_Entry]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable_Entry()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_util_Hashtable_Entry___INIT____java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry___INIT____java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Hashtable.java", 95)
    XMLVM_CHECK_NPE(1)
    java_util_MapEntry___INIT____java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 96)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[4])(_r2.o);
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable_Entry*) _r1.o)->fields.java_util_Hashtable_Entry.hashcode_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 97)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_Entry_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry_clone__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "clone", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 102)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_MapEntry_clone__(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 103)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    if (_r1.o == JAVA_NULL) goto label20;
    XMLVM_SOURCE_POSITION("Hashtable.java", 104)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    //java_util_Hashtable_Entry_clone__[0]
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable_Entry*) _r1.o)->tib->vtable[0])(_r1.o);
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(0)
    ((java_util_Hashtable_Entry*) _r0.o)->fields.java_util_Hashtable_Entry.next_ = _r2.o;
    label20:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 106)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_Entry_setValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry_setValue___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "setValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 111)
    if (_r2.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Hashtable.java", 112)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 114)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;
    XMLVM_SOURCE_POSITION("Hashtable.java", 115)
    XMLVM_CHECK_NPE(1)
    ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_ = _r2.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 116)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Hashtable_Entry_getKeyHash__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry_getKeyHash__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "getKeyHash", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 120)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.key_;
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry_equalsKey___java_lang_Object_int]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "equalsKey", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("Hashtable.java", 124)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.hashcode_;
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[4])(_r3.o);
    if (_r0.i != _r1.i) goto label18;
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.key_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[1])(_r0.o, _r3.o);
    if (_r0.i == 0) goto label18;
    _r0.i = 1;
    label17:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    _r0.i = 0;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_Entry_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_Entry_toString__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$Entry", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 129)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.key_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r1.o);
    // "="
    _r1.o = xmlvm_create_java_string_from_pool(648);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

