#include "xmlvm.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_Throwable.h"
#include "java_util_Collection.h"
#include "java_util_Enumeration.h"
#include "java_util_Hashtable_1.h"
#include "java_util_Hashtable_2.h"
#include "java_util_Hashtable_Entry.h"
#include "java_util_Iterator.h"
#include "java_util_Map_Entry.h"
#include "java_util_Set.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_util_Hashtable.h"

#define XMLVM_CURRENT_CLASS_NAME Hashtable
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Hashtable

__TIB_DEFINITION_java_util_Hashtable __TIB_java_util_Hashtable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Hashtable, // classInitializer
    "java.util.Hashtable", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/util/Dictionary<TK;TV;>;Ljava/util/Map<TK;TV;>;Ljava/lang/Cloneable;Ljava/io/Serializable;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_Dictionary, // extends
    sizeof(java_util_Hashtable), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Hashtable;
JAVA_OBJECT __CLASS_java_util_Hashtable_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Hashtable_serialVersionUID;
static JAVA_OBJECT _STATIC_java_util_Hashtable_EMPTY_ENUMERATION;
static JAVA_OBJECT _STATIC_java_util_Hashtable_EMPTY_ITERATOR;

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

void __INIT_java_util_Hashtable()
{
    staticInitializerLock(&__TIB_java_util_Hashtable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Hashtable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Hashtable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Hashtable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Hashtable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Hashtable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Hashtable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Hashtable")
        __INIT_IMPL_java_util_Hashtable();
    }
}

void __INIT_IMPL_java_util_Hashtable()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_Dictionary)
    __TIB_java_util_Hashtable.newInstanceFunc = __NEW_INSTANCE_java_util_Hashtable;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Hashtable.vtable, __TIB_java_util_Dictionary.vtable, sizeof(__TIB_java_util_Dictionary.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Hashtable.vtable[13] = (VTABLE_PTR) &java_util_Hashtable_clear__;
    __TIB_java_util_Hashtable.vtable[0] = (VTABLE_PTR) &java_util_Hashtable_clone__;
    __TIB_java_util_Hashtable.vtable[14] = (VTABLE_PTR) &java_util_Hashtable_containsKey___java_lang_Object;
    __TIB_java_util_Hashtable.vtable[15] = (VTABLE_PTR) &java_util_Hashtable_containsValue___java_lang_Object;
    __TIB_java_util_Hashtable.vtable[6] = (VTABLE_PTR) &java_util_Hashtable_elements__;
    __TIB_java_util_Hashtable.vtable[16] = (VTABLE_PTR) &java_util_Hashtable_entrySet__;
    __TIB_java_util_Hashtable.vtable[1] = (VTABLE_PTR) &java_util_Hashtable_equals___java_lang_Object;
    __TIB_java_util_Hashtable.vtable[7] = (VTABLE_PTR) &java_util_Hashtable_get___java_lang_Object;
    __TIB_java_util_Hashtable.vtable[4] = (VTABLE_PTR) &java_util_Hashtable_hashCode__;
    __TIB_java_util_Hashtable.vtable[8] = (VTABLE_PTR) &java_util_Hashtable_isEmpty__;
    __TIB_java_util_Hashtable.vtable[9] = (VTABLE_PTR) &java_util_Hashtable_keys__;
    __TIB_java_util_Hashtable.vtable[17] = (VTABLE_PTR) &java_util_Hashtable_keySet__;
    __TIB_java_util_Hashtable.vtable[10] = (VTABLE_PTR) &java_util_Hashtable_put___java_lang_Object_java_lang_Object;
    __TIB_java_util_Hashtable.vtable[18] = (VTABLE_PTR) &java_util_Hashtable_putAll___java_util_Map;
    __TIB_java_util_Hashtable.vtable[11] = (VTABLE_PTR) &java_util_Hashtable_remove___java_lang_Object;
    __TIB_java_util_Hashtable.vtable[12] = (VTABLE_PTR) &java_util_Hashtable_size__;
    __TIB_java_util_Hashtable.vtable[5] = (VTABLE_PTR) &java_util_Hashtable_toString__;
    __TIB_java_util_Hashtable.vtable[19] = (VTABLE_PTR) &java_util_Hashtable_values__;
    // Initialize interface information
    __TIB_java_util_Hashtable.numImplementedInterfaces = 3;
    __TIB_java_util_Hashtable.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Hashtable.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_Hashtable.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_util_Map)

    __TIB_java_util_Hashtable.implementedInterfaces[0][2] = &__TIB_java_util_Map;
    // Initialize itable for this class
    __TIB_java_util_Hashtable.itableBegin = &__TIB_java_util_Hashtable.itable[0];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_clear__] = __TIB_java_util_Hashtable.vtable[13];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object] = __TIB_java_util_Hashtable.vtable[14];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object] = __TIB_java_util_Hashtable.vtable[15];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_entrySet__] = __TIB_java_util_Hashtable.vtable[16];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object] = __TIB_java_util_Hashtable.vtable[1];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object] = __TIB_java_util_Hashtable.vtable[7];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_hashCode__] = __TIB_java_util_Hashtable.vtable[4];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__] = __TIB_java_util_Hashtable.vtable[8];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_keySet__] = __TIB_java_util_Hashtable.vtable[17];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map] = __TIB_java_util_Hashtable.vtable[18];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object] = __TIB_java_util_Hashtable.vtable[10];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object] = __TIB_java_util_Hashtable.vtable[11];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_size__] = __TIB_java_util_Hashtable.vtable[12];
    __TIB_java_util_Hashtable.itable[XMLVM_ITABLE_IDX_java_util_Map_values__] = __TIB_java_util_Hashtable.vtable[19];

    _STATIC_java_util_Hashtable_serialVersionUID = 1421746759512286392;
    _STATIC_java_util_Hashtable_EMPTY_ENUMERATION = (java_util_Enumeration*) JAVA_NULL;
    _STATIC_java_util_Hashtable_EMPTY_ITERATOR = (java_util_Iterator*) JAVA_NULL;

    __TIB_java_util_Hashtable.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Hashtable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Hashtable.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Hashtable.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Hashtable.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Hashtable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Hashtable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Hashtable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Hashtable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Hashtable);
    __TIB_java_util_Hashtable.clazz = __CLASS_java_util_Hashtable;
    __TIB_java_util_Hashtable.baseType = JAVA_NULL;
    __CLASS_java_util_Hashtable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable);
    __CLASS_java_util_Hashtable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_1ARRAY);
    __CLASS_java_util_Hashtable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_2ARRAY);
    java_util_Hashtable___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Hashtable]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Hashtable.classInitialized = 1;
}

void __DELETE_java_util_Hashtable(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Hashtable]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Hashtable(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_Dictionary(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.elementCount_ = 0;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.elementData_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.loadFactor_ = 0;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.threshold_ = 0;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.firstSlot_ = 0;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.lastSlot_ = 0;
    ((java_util_Hashtable*) me)->fields.java_util_Hashtable.modCount_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Hashtable]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Hashtable()
{    XMLVM_CLASS_INIT(java_util_Hashtable)
java_util_Hashtable* me = (java_util_Hashtable*) XMLVM_MALLOC(sizeof(java_util_Hashtable));
    me->tib = &__TIB_java_util_Hashtable;
    __INIT_INSTANCE_MEMBERS_java_util_Hashtable(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Hashtable]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_Hashtable();
    java_util_Hashtable___INIT___(me);
    return me;
}

JAVA_LONG java_util_Hashtable_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
    return _STATIC_java_util_Hashtable_serialVersionUID;
}

void java_util_Hashtable_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
_STATIC_java_util_Hashtable_serialVersionUID = v;
}

JAVA_OBJECT java_util_Hashtable_GET_EMPTY_ENUMERATION()
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
    return _STATIC_java_util_Hashtable_EMPTY_ENUMERATION;
}

void java_util_Hashtable_PUT_EMPTY_ENUMERATION(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
_STATIC_java_util_Hashtable_EMPTY_ENUMERATION = v;
}

JAVA_OBJECT java_util_Hashtable_GET_EMPTY_ITERATOR()
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
    return _STATIC_java_util_Hashtable_EMPTY_ITERATOR;
}

void java_util_Hashtable_PUT_EMPTY_ITERATOR(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
_STATIC_java_util_Hashtable_EMPTY_ITERATOR = v;
}

void java_util_Hashtable___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable___CLINIT___]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Hashtable.java", 60)
    _r0.o = __NEW_java_util_Hashtable_1();
    XMLVM_CHECK_NPE(0)
    java_util_Hashtable_1___INIT___(_r0.o);
    java_util_Hashtable_PUT_EMPTY_ENUMERATION( _r0.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 70)
    _r0.o = __NEW_java_util_Hashtable_2();
    XMLVM_CHECK_NPE(0)
    java_util_Hashtable_2___INIT___(_r0.o);
    java_util_Hashtable_PUT_EMPTY_ITERATOR( _r0.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 41)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_newEntry___java_lang_Object_java_lang_Object_int(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_newEntry___java_lang_Object_java_lang_Object_int]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "newEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.o = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("Hashtable.java", 86)
    _r0.o = __NEW_java_util_Hashtable_Entry();
    XMLVM_CHECK_NPE(0)
    java_util_Hashtable_Entry___INIT____java_lang_Object_java_lang_Object(_r0.o, _r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable___INIT___]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 235)
    _r0.i = 11;
    XMLVM_CHECK_NPE(1)
    java_util_Hashtable___INIT____int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 236)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable___INIT____int]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 245)
    XMLVM_CHECK_NPE(1)
    java_util_Dictionary___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 56)
    _r0.i = -1;
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.lastSlot_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 246)
    if (_r2.i < 0) goto label35;
    XMLVM_SOURCE_POSITION("Hashtable.java", 247)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 248)
    if (_r2.i != 0) goto label33;
    _r0.i = 1;
    label14:;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Hashtable_newElementArray___int(_r1.o, _r0.i);
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementData_ = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 249)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.firstSlot_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 250)
    _r0.f = 0.75;
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.loadFactor_ = _r0.f;
    XMLVM_SOURCE_POSITION("Hashtable.java", 251)
    XMLVM_CHECK_NPE(1)
    java_util_Hashtable_computeMaxSize__(_r1.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 255)
    XMLVM_EXIT_METHOD()
    return;
    label33:;
    _r0 = _r2;
    goto label14;
    label35:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 253)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable___INIT____int_float]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("Hashtable.java", 266)
    XMLVM_CHECK_NPE(1)
    java_util_Dictionary___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 56)
    _r0.i = -1;
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.lastSlot_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 267)
    if (_r2.i < 0) goto label35;
    _r0.f = 0.0;
    _r0.i = _r3.f > _r0.f ? 1 : (_r3.f == _r0.f ? 0 : -1);
    if (_r0.i <= 0) goto label35;
    XMLVM_SOURCE_POSITION("Hashtable.java", 268)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 269)
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.firstSlot_ = _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 270)
    if (_r2.i != 0) goto label33;
    _r0.i = 1;
    label21:;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Hashtable_newElementArray___int(_r1.o, _r0.i);
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementData_ = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 271)
    XMLVM_CHECK_NPE(1)
    ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.loadFactor_ = _r3.f;
    XMLVM_SOURCE_POSITION("Hashtable.java", 272)
    XMLVM_CHECK_NPE(1)
    java_util_Hashtable_computeMaxSize__(_r1.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 276)
    XMLVM_EXIT_METHOD()
    return;
    label33:;
    _r0 = _r2;
    goto label21;
    label35:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 274)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable___INIT____java_util_Map]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 286)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r3.o);
    _r1.i = 6;
    if (_r0.i >= _r1.i) goto label16;
    _r0.i = 11;
    label9:;
    XMLVM_CHECK_NPE(2)
    java_util_Hashtable___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 287)
    //java_util_Hashtable_putAll___java_util_Map[18]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Hashtable*) _r2.o)->tib->vtable[18])(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 288)
    XMLVM_EXIT_METHOD()
    return;
    label16:;
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r3.o);
    _r0.i = _r0.i * 4;
    _r0.i = _r0.i / 3;
    _r0.i = _r0.i + 11;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_newElementArray___int]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "newElementArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 292)
    XMLVM_CLASS_INIT(java_util_Hashtable_Entry)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_util_Hashtable_Entry, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_clear__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "clear", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 303)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    _r0.i = 0;
    XMLVM_TRY_BEGIN(w2948aaac17b1b5)
    // Begin try
    XMLVM_CHECK_NPE(2)
    ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 304)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.elementData_;
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 305)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.modCount_ = _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac17b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac17b1b5,java_lang_Object,18)
    XMLVM_CATCH_END(w2948aaac17b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac17b1b5)
    XMLVM_SOURCE_POSITION("Hashtable.java", 306)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return;
    label18:;
    java_lang_Thread* curThread_w2948aaac17b1c10 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac17b1c10->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_clone__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 319)
    java_lang_Object_acquireLockRecursive__(_r4.o);
    XMLVM_TRY_BEGIN(w2948aaac18b1b4)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r0.o = java_lang_Object_clone__(_r4.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 320)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_Hashtable*) _r4.o)->fields.java_util_Hashtable.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CLASS_INIT(java_util_Hashtable_Entry)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_util_Hashtable_Entry, _r1.i);
    XMLVM_CHECK_NPE(0)
    ((java_util_Hashtable*) _r0.o)->fields.java_util_Hashtable.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 322)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_Hashtable*) _r4.o)->fields.java_util_Hashtable.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac18b1b4)
        XMLVM_CATCH_SPECIFIC(w2948aaac18b1b4,java_lang_Object,44)
    XMLVM_CATCH_END(w2948aaac18b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac18b1b4)
    label17:;
    _r2.i = _r1.i + -1;
    if (_r2.i >= 0) goto label23;
    label21:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 329)
    java_lang_Object_releaseLockRecursive__(_r4.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label23:;
    XMLVM_TRY_BEGIN(w2948aaac18b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 323)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_Hashtable*) _r4.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r1.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac18b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac18b1c13, sizeof(XMLVM_JMP_BUF)); goto label39; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 324)
    XMLVM_CHECK_NPE(0)
    _r3.o = ((java_util_Hashtable*) _r0.o)->fields.java_util_Hashtable.elementData_;
    //java_util_Hashtable_Entry_clone__[0]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable_Entry*) _r1.o)->tib->vtable[0])(_r1.o);
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r1.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac18b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac18b1c13,java_lang_Object,44)
    XMLVM_CATCH_END(w2948aaac18b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac18b1c13)
    label39:;
    _r1 = _r2;
    goto label17;
    label41:;
    java_lang_Thread* curThread_w2948aaac18b1c18 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac18b1c18->fields.java_lang_Thread.xmlvmException_;
    _r0.o = JAVA_NULL;
    goto label21;
    label44:;
    java_lang_Thread* curThread_w2948aaac18b1c22 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac18b1c22->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r4.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_computeMaxSize__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_computeMaxSize__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "computeMaxSize", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 334)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_CHECK_NPE(2)
    _r1.f = ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.loadFactor_;
    _r0.f = _r0.f * _r1.f;
    _r0.i = (JAVA_INT) _r0.f;
    XMLVM_CHECK_NPE(2)
    ((java_util_Hashtable*) _r2.o)->fields.java_util_Hashtable.threshold_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 335)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "contains", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 349)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    if (_r4.o != JAVA_NULL) goto label12;
    XMLVM_TRY_BEGIN(w2948aaac20b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 350)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac20b1b6)
        XMLVM_CATCH_SPECIFIC(w2948aaac20b1b6,java_lang_Object,9)
    XMLVM_CATCH_END(w2948aaac20b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac20b1b6)
    label9:;
    java_lang_Thread* curThread_w2948aaac20b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac20b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_TRY_BEGIN(w2948aaac20b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 353)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac20b1c12)
        XMLVM_CATCH_SPECIFIC(w2948aaac20b1c12,java_lang_Object,9)
    XMLVM_CATCH_END(w2948aaac20b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac20b1c12)
    label15:;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label22;
    XMLVM_SOURCE_POSITION("Hashtable.java", 362)
    _r0.i = 0;
    label20:;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    XMLVM_TRY_BEGIN(w2948aaac20b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 354)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac20b1c22)
        XMLVM_CATCH_SPECIFIC(w2948aaac20b1c22,java_lang_Object,9)
    XMLVM_CATCH_END(w2948aaac20b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac20b1c22)
    label26:;
    XMLVM_TRY_BEGIN(w2948aaac20b1c24)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 355)
    if (_r1.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac20b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac20b1c24, sizeof(XMLVM_JMP_BUF)); goto label15; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 356)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[1])(_r2.o, _r4.o);
    if (_r2.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac20b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac20b1c24, sizeof(XMLVM_JMP_BUF)); goto label38; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 357)
    _r0.i = 1;
    { XMLVM_MEMCPY(curThread_w2948aaac20b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac20b1c24, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 359)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac20b1c24)
        XMLVM_CATCH_SPECIFIC(w2948aaac20b1c24,java_lang_Object,9)
    XMLVM_CATCH_END(w2948aaac20b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac20b1c24)
    label38:;
    XMLVM_TRY_BEGIN(w2948aaac20b1c26)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_Hashtable_Entry*) _r1.o)->fields.java_util_Hashtable_Entry.next_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac20b1c26)
        XMLVM_CATCH_SPECIFIC(w2948aaac20b1c26,java_lang_Object,9)
    XMLVM_CATCH_END(w2948aaac20b1c26)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac20b1c26)
    goto label26;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_containsKey___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "containsKey", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 377)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w2948aaac21b1b5)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_Hashtable_getEntry___java_lang_Object(_r1.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac21b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac21b1b5,java_lang_Object,12)
    XMLVM_CATCH_END(w2948aaac21b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac21b1b5)
    if (_r0.o == JAVA_NULL) goto label10;
    _r0.i = 1;
    label8:;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 0;
    goto label8;
    label12:;
    java_lang_Thread* curThread_w2948aaac21b1c15 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac21b1c15->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_containsValue___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "containsValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 389)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_Hashtable_contains___java_lang_Object(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_elements__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_elements__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "elements", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 405)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w2948aaac23b1b4)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementCount_;
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w2948aaac23b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac23b1b4, sizeof(XMLVM_JMP_BUF)); goto label9; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 406)
    _r0.o = java_util_Hashtable_GET_EMPTY_ENUMERATION();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac23b1b4)
        XMLVM_CATCH_SPECIFIC(w2948aaac23b1b4,java_lang_Object,21)
    XMLVM_CATCH_END(w2948aaac23b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac23b1b4)
    label7:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 408)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    XMLVM_TRY_BEGIN(w2948aaac23b1c10)
    // Begin try

    
    // Red class access removed: java.util.Hashtable$HashEnumIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$3::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$3::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 412)
    _r2.i = 1;

    
    // Red class access removed: java.util.Hashtable$HashEnumIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac23b1c10)
        XMLVM_CATCH_SPECIFIC(w2948aaac23b1c10,java_lang_Object,21)
    XMLVM_CATCH_END(w2948aaac23b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac23b1c10)
    goto label7;
    label21:;
    java_lang_Thread* curThread_w2948aaac23b1c13 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac23b1c13->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_entrySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_entrySet__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "entrySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 424)

    
    // Red class access removed: java.util.Collections$SynchronizedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 425)

    
    // Red class access removed: java.util.Hashtable$4::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$4::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "equals", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Hashtable.java", 481)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    if (_r5.o != _r6.o) goto label8;
    _r0 = _r4;
    label6:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 482)
    XMLVM_SOURCE_POSITION("Hashtable.java", 498)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_TRY_BEGIN(w2948aaac25b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 484)
    XMLVM_CLASS_INIT(java_util_Map)
    _r0.i = XMLVM_ISA(_r6.o, __CLASS_java_util_Map);
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac25b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac25b1c15, sizeof(XMLVM_JMP_BUF)); goto label60; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 485)
    _r6.o = _r6.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 486)
    //java_util_Hashtable_size__[12]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[12])(_r5.o);
    XMLVM_CHECK_NPE(6)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r6.o);
    if (_r0.i == _r1.i) { XMLVM_MEMCPY(curThread_w2948aaac25b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac25b1c15, sizeof(XMLVM_JMP_BUF)); goto label26; };
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Hashtable.java", 487)
    { XMLVM_MEMCPY(curThread_w2948aaac25b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac25b1c15, sizeof(XMLVM_JMP_BUF)); goto label6; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 490)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac25b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac25b1c15,java_lang_Object,62)
    XMLVM_CATCH_END(w2948aaac25b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac25b1c15)
    label26:;
    XMLVM_TRY_BEGIN(w2948aaac25b1c17)
    // Begin try
    //java_util_Hashtable_entrySet__[16]
    XMLVM_CHECK_NPE(5)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[16])(_r5.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 491)
    XMLVM_CHECK_NPE(6)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r6.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_entrySet__])(_r6.o);
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac25b1c17)
        XMLVM_CATCH_SPECIFIC(w2948aaac25b1c17,java_lang_Object,62)
    XMLVM_CATCH_END(w2948aaac25b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac25b1c17)
    label38:;
    XMLVM_TRY_BEGIN(w2948aaac25b1c19)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r2.o);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w2948aaac25b1c19->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac25b1c19, sizeof(XMLVM_JMP_BUF)); goto label46; };
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("Hashtable.java", 496)
    { XMLVM_MEMCPY(curThread_w2948aaac25b1c19->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac25b1c19, sizeof(XMLVM_JMP_BUF)); goto label6; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac25b1c19)
        XMLVM_CATCH_SPECIFIC(w2948aaac25b1c19,java_lang_Object,62)
    XMLVM_CATCH_END(w2948aaac25b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac25b1c19)
    label46:;
    XMLVM_TRY_BEGIN(w2948aaac25b1c21)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 492)
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_contains___java_lang_Object])(_r1.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac25b1c21)
        XMLVM_CATCH_SPECIFIC(w2948aaac25b1c21,java_lang_Object,62)
    XMLVM_CATCH_END(w2948aaac25b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac25b1c21)
    if (_r0.i != 0) goto label38;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Hashtable.java", 493)
    goto label6;
    label60:;
    _r0 = _r3;
    goto label6;
    label62:;
    java_lang_Thread* curThread_w2948aaac25b1c30 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac25b1c30->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_get___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "get", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 513)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w2948aaac26b1b5)
    // Begin try
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[4])(_r4.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 514)
    _r1.i = 2147483647;
    _r1.i = _r1.i & _r0.i;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 515)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac26b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac26b1b5,java_lang_Object,34)
    XMLVM_CATCH_END(w2948aaac26b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac26b1b5)
    label17:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 516)
    if (_r1.o != JAVA_NULL) goto label22;
    XMLVM_SOURCE_POSITION("Hashtable.java", 522)
    _r0.o = JAVA_NULL;
    label20:;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label22:;
    XMLVM_TRY_BEGIN(w2948aaac26b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 517)
    XMLVM_CHECK_NPE(1)
    _r2.i = java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(_r1.o, _r4.o, _r0.i);
    if (_r2.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac26b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac26b1c15, sizeof(XMLVM_JMP_BUF)); goto label31; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 518)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;
    { XMLVM_MEMCPY(curThread_w2948aaac26b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac26b1c15, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 520)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac26b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac26b1c15,java_lang_Object,34)
    XMLVM_CATCH_END(w2948aaac26b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac26b1c15)
    label31:;
    XMLVM_TRY_BEGIN(w2948aaac26b1c17)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_Hashtable_Entry*) _r1.o)->fields.java_util_Hashtable_Entry.next_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac26b1c17)
        XMLVM_CATCH_SPECIFIC(w2948aaac26b1c17,java_lang_Object,34)
    XMLVM_CATCH_END(w2948aaac26b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac26b1c17)
    goto label17;
    label34:;
    java_lang_Thread* curThread_w2948aaac26b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac26b1c20->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_getEntry___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "getEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 526)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[4])(_r4.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 527)
    _r1.i = 2147483647;
    _r1.i = _r1.i & _r0.i;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 528)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    label16:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 529)
    if (_r1.o != JAVA_NULL) goto label20;
    XMLVM_SOURCE_POSITION("Hashtable.java", 535)
    _r0.o = JAVA_NULL;
    label19:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label20:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 530)
    XMLVM_CHECK_NPE(1)
    _r2.i = java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(_r1.o, _r4.o, _r0.i);
    if (_r2.i == 0) goto label28;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 531)
    goto label19;
    label28:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 533)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_Hashtable_Entry*) _r1.o)->fields.java_util_Hashtable_Entry.next_;
    goto label16;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Hashtable_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_hashCode__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "hashCode", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.o = me;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("Hashtable.java", 540)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    XMLVM_TRY_BEGIN(w2948aaac28b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 541)
    //java_util_Hashtable_entrySet__[16]
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[16])(_r5.o);
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    _r2 = _r4;
    XMLVM_SOURCE_POSITION("Hashtable.java", 542)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac28b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac28b1b5,java_lang_Object,57)
    XMLVM_CATCH_END(w2948aaac28b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac28b1b5)
    label11:;
    XMLVM_TRY_BEGIN(w2948aaac28b1b7)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac28b1b7)
        XMLVM_CATCH_SPECIFIC(w2948aaac28b1b7,java_lang_Object,57)
    XMLVM_CATCH_END(w2948aaac28b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac28b1b7)
    if (_r0.i != 0) goto label19;
    XMLVM_SOURCE_POSITION("Hashtable.java", 556)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label19:;
    XMLVM_TRY_BEGIN(w2948aaac28b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 543)
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 544)
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 545)
    if (_r3.o == _r5.o) { XMLVM_MEMCPY(curThread_w2948aaac28b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac28b1c13, sizeof(XMLVM_JMP_BUF)); goto label11; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 548)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 549)
    if (_r0.o == _r5.o) { XMLVM_MEMCPY(curThread_w2948aaac28b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac28b1c13, sizeof(XMLVM_JMP_BUF)); goto label11; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 552)
    if (_r3.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac28b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac28b1c13, sizeof(XMLVM_JMP_BUF)); goto label53; };
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[4])(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac28b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac28b1c13,java_lang_Object,57)
    XMLVM_CATCH_END(w2948aaac28b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac28b1c13)
    label43:;
    XMLVM_TRY_BEGIN(w2948aaac28b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 553)
    if (_r0.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac28b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac28b1c15, sizeof(XMLVM_JMP_BUF)); goto label55; };
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[4])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac28b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac28b1c15,java_lang_Object,57)
    XMLVM_CATCH_END(w2948aaac28b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac28b1c15)
    label49:;
    _r0.i = _r0.i ^ _r3.i;
    _r0.i = _r0.i + _r2.i;
    _r2 = _r0;
    XMLVM_SOURCE_POSITION("Hashtable.java", 554)
    goto label11;
    label53:;
    _r3 = _r4;
    goto label43;
    label55:;
    _r0 = _r4;
    goto label49;
    label57:;
    java_lang_Thread* curThread_w2948aaac28b1c29 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac28b1c29->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "isEmpty", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 568)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w2948aaac29b1b4)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementCount_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac29b1b4)
        XMLVM_CATCH_SPECIFIC(w2948aaac29b1b4,java_lang_Object,10)
    XMLVM_CATCH_END(w2948aaac29b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac29b1b4)
    if (_r0.i != 0) goto label8;
    _r0.i = 1;
    label6:;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label6;
    label10:;
    java_lang_Thread* curThread_w2948aaac29b1c14 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac29b1c14->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_keys__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_keys__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "keys", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 584)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w2948aaac30b1b4)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementCount_;
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w2948aaac30b1b4->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac30b1b4, sizeof(XMLVM_JMP_BUF)); goto label9; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 585)
    _r0.o = java_util_Hashtable_GET_EMPTY_ENUMERATION();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac30b1b4)
        XMLVM_CATCH_SPECIFIC(w2948aaac30b1b4,java_lang_Object,21)
    XMLVM_CATCH_END(w2948aaac30b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac30b1b4)
    label7:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 587)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    XMLVM_TRY_BEGIN(w2948aaac30b1c10)
    // Begin try

    
    // Red class access removed: java.util.Hashtable$HashEnumIterator::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$5::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$5::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 591)
    _r2.i = 1;

    
    // Red class access removed: java.util.Hashtable$HashEnumIterator::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac30b1c10)
        XMLVM_CATCH_SPECIFIC(w2948aaac30b1c10,java_lang_Object,21)
    XMLVM_CATCH_END(w2948aaac30b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac30b1c10)
    goto label7;
    label21:;
    java_lang_Thread* curThread_w2948aaac30b1c13 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac30b1c13->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_keySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_keySet__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "keySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 602)

    
    // Red class access removed: java.util.Collections$SynchronizedSet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$6::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$6::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedSet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_put___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "put", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r5.o = me;
    _r6.o = n1;
    _r7.o = n2;
    _r4.i = 2147483647;
    XMLVM_SOURCE_POSITION("Hashtable.java", 737)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    if (_r6.o == JAVA_NULL) goto label97;
    if (_r7.o == JAVA_NULL) goto label97;
    XMLVM_TRY_BEGIN(w2948aaac32b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 738)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[4])(_r6.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 739)
    _r1.i = _r0.i & _r4.i;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 740)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1b9)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1b9,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1b9)
    label22:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 741)
    if (_r2.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac32b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c11, sizeof(XMLVM_JMP_BUF)); goto label30; };
    XMLVM_CHECK_NPE(2)
    _r3.i = java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(_r2.o, _r6.o, _r0.i);
    if (_r3.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac32b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c11, sizeof(XMLVM_JMP_BUF)); goto label86; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c11)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c11,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c11)
    label30:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 744)
    if (_r2.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac32b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c13, sizeof(XMLVM_JMP_BUF)); goto label89; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 745)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.modCount_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.modCount_ = _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 746)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementCount_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementCount_ = _r2.i;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.threshold_;
    if (_r2.i <= _r3.i) { XMLVM_MEMCPY(curThread_w2948aaac32b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c13, sizeof(XMLVM_JMP_BUF)); goto label57; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 747)
    XMLVM_CHECK_NPE(5)
    java_util_Hashtable_rehash__(_r5.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 748)
    _r1.i = _r0.i & _r4.i;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c13,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c13)
    label57:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 750)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.firstSlot_;
    if (_r1.i >= _r2.i) { XMLVM_MEMCPY(curThread_w2948aaac32b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c15, sizeof(XMLVM_JMP_BUF)); goto label63; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 751)
    XMLVM_CHECK_NPE(5)
    ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.firstSlot_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c15,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c15)
    label63:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c17)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 753)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.lastSlot_;
    if (_r1.i <= _r2.i) { XMLVM_MEMCPY(curThread_w2948aaac32b1c17->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c17, sizeof(XMLVM_JMP_BUF)); goto label69; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 754)
    XMLVM_CHECK_NPE(5)
    ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.lastSlot_ = _r1.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c17)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c17,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c17)
    label69:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c19)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 756)
    _r0.o = java_util_Hashtable_newEntry___java_lang_Object_java_lang_Object_int(_r6.o, _r7.o, _r0.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 757)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(0)
    ((java_util_Hashtable_Entry*) _r0.o)->fields.java_util_Hashtable_Entry.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 758)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c19)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c19,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c19)
    XMLVM_SOURCE_POSITION("Hashtable.java", 759)
    _r0.o = JAVA_NULL;
    label84:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 763)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label86:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 742)
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    { XMLVM_MEMCPY(curThread_w2948aaac32b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac32b1c27, sizeof(XMLVM_JMP_BUF)); goto label22; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 761)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c27)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c27,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c27)
    label89:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c29)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;
    XMLVM_SOURCE_POSITION("Hashtable.java", 762)
    XMLVM_CHECK_NPE(2)
    ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_ = _r7.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c29)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c29,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c29)
    goto label84;
    label94:;
    java_lang_Thread* curThread_w2948aaac32b1c32 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac32b1c32->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label97:;
    XMLVM_TRY_BEGIN(w2948aaac32b1c36)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 765)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac32b1c36)
        XMLVM_CATCH_SPECIFIC(w2948aaac32b1c36,java_lang_Object,94)
    XMLVM_CATCH_END(w2948aaac32b1c36)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac32b1c36)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_putAll___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "putAll", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 775)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w2948aaac33b1b5)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_entrySet__])(_r4.o);
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac33b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac33b1b5,java_lang_Object,35)
    XMLVM_CATCH_END(w2948aaac33b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac33b1b5)
    label9:;
    XMLVM_TRY_BEGIN(w2948aaac33b1b7)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac33b1b7)
        XMLVM_CATCH_SPECIFIC(w2948aaac33b1b7,java_lang_Object,35)
    XMLVM_CATCH_END(w2948aaac33b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac33b1b7)
    if (_r0.i != 0) goto label17;
    XMLVM_SOURCE_POSITION("Hashtable.java", 778)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return;
    label17:;
    XMLVM_TRY_BEGIN(w2948aaac33b1c13)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 776)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    //java_util_Hashtable_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(3)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Hashtable*) _r3.o)->tib->vtable[10])(_r3.o, _r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac33b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac33b1c13,java_lang_Object,35)
    XMLVM_CATCH_END(w2948aaac33b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac33b1c13)
    goto label9;
    label35:;
    java_lang_Thread* curThread_w2948aaac33b1c16 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac33b1c16->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_rehash__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_rehash__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "rehash", "?")
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
    _r10.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 785)
    XMLVM_CHECK_NPE(10)
    _r0.o = ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r0.i << 1;
    _r0.i = _r0.i + 1;
    if (_r0.i != 0) goto label10;
    XMLVM_SOURCE_POSITION("Hashtable.java", 786)
    XMLVM_SOURCE_POSITION("Hashtable.java", 787)
    _r0.i = 1;
    label10:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 790)
    _r1.i = -1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 791)
    XMLVM_CHECK_NPE(10)
    _r2.o = java_util_Hashtable_newElementArray___int(_r10.o, _r0.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 792)
    XMLVM_CHECK_NPE(10)
    _r3.i = ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.lastSlot_;
    _r3.i = _r3.i + 1;
    _r4 = _r0;
    _r9 = _r1;
    _r1 = _r3;
    _r3 = _r9;
    label23:;
    _r1.i = _r1.i + -1;
    XMLVM_CHECK_NPE(10)
    _r5.i = ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.firstSlot_;
    if (_r1.i >= _r5.i) goto label39;
    XMLVM_SOURCE_POSITION("Hashtable.java", 808)
    XMLVM_CHECK_NPE(10)
    ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.firstSlot_ = _r4.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 809)
    XMLVM_CHECK_NPE(10)
    ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.lastSlot_ = _r3.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 810)
    XMLVM_CHECK_NPE(10)
    ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.elementData_ = _r2.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 811)
    XMLVM_CHECK_NPE(10)
    java_util_Hashtable_computeMaxSize__(_r10.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 812)
    XMLVM_EXIT_METHOD()
    return;
    label39:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 793)
    XMLVM_CHECK_NPE(10)
    _r5.o = ((java_util_Hashtable*) _r10.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r1.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r9 = _r5;
    _r5 = _r4;
    _r4 = _r3;
    _r3 = _r9;
    label47:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 794)
    if (_r3.o != JAVA_NULL) goto label52;
    _r3 = _r4;
    _r4 = _r5;
    goto label23;
    label52:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 795)
    XMLVM_CHECK_NPE(3)
    _r6.i = java_util_Hashtable_Entry_getKeyHash__(_r3.o);
    _r7.i = 2147483647;
    _r6.i = _r6.i & _r7.i;
    _r6.i = _r6.i % _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 796)
    if (_r6.i >= _r5.i) goto label64;
    _r5 = _r6;
    label64:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 797)
    XMLVM_SOURCE_POSITION("Hashtable.java", 799)
    if (_r6.i <= _r4.i) goto label67;
    _r4 = _r6;
    label67:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 800)
    XMLVM_SOURCE_POSITION("Hashtable.java", 802)
    XMLVM_CHECK_NPE(3)
    _r7.o = ((java_util_Hashtable_Entry*) _r3.o)->fields.java_util_Hashtable_Entry.next_;
    XMLVM_SOURCE_POSITION("Hashtable.java", 803)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    _r8.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    XMLVM_CHECK_NPE(3)
    ((java_util_Hashtable_Entry*) _r3.o)->fields.java_util_Hashtable_Entry.next_ = _r8.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 804)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    _r3 = _r7;
    XMLVM_SOURCE_POSITION("Hashtable.java", 805)
    goto label47;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "remove", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r7.o = me;
    _r8.o = n1;
    _r5.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Hashtable.java", 827)
    java_lang_Object_acquireLockRecursive__(_r7.o);
    XMLVM_TRY_BEGIN(w2948aaac35b1b6)
    // Begin try
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(8)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r8.o)->tib->vtable[4])(_r8.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 828)
    _r1.i = 2147483647;
    _r1.i = _r1.i & _r0.i;
    XMLVM_CHECK_NPE(7)
    _r2.o = ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 830)
    XMLVM_CHECK_NPE(7)
    _r2.o = ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3 = _r5;
    XMLVM_SOURCE_POSITION("Hashtable.java", 831)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1b6)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1b6,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1b6)
    label19:;
    XMLVM_TRY_BEGIN(w2948aaac35b1b8)
    // Begin try
    if (_r2.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac35b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac35b1b8, sizeof(XMLVM_JMP_BUF)); goto label27; };
    XMLVM_CHECK_NPE(2)
    _r4.i = java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(_r2.o, _r8.o, _r0.i);
    if (_r4.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac35b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac35b1b8, sizeof(XMLVM_JMP_BUF)); goto label56; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1b8)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1b8,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1b8)
    label27:;
    XMLVM_TRY_BEGIN(w2948aaac35b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 835)
    if (_r2.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac35b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac35b1c10, sizeof(XMLVM_JMP_BUF)); goto label70; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 836)
    XMLVM_CHECK_NPE(7)
    _r0.i = ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(7)
    ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 837)
    if (_r3.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac35b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac35b1c10, sizeof(XMLVM_JMP_BUF)); goto label62; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 838)
    XMLVM_CHECK_NPE(7)
    _r0.o = ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1c10)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1c10,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1c10)
    label43:;
    XMLVM_TRY_BEGIN(w2948aaac35b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 842)
    XMLVM_CHECK_NPE(7)
    _r0.i = ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.elementCount_;
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(7)
    ((java_util_Hashtable*) _r7.o)->fields.java_util_Hashtable.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 843)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;
    XMLVM_SOURCE_POSITION("Hashtable.java", 844)
    _r1.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_ = _r1.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1c12)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1c12,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1c12)
    label54:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 847)
    java_lang_Object_releaseLockRecursive__(_r7.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label56:;
    XMLVM_TRY_BEGIN(w2948aaac35b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 833)
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    _r6 = _r3;
    _r3 = _r2;
    _r2 = _r6;
    { XMLVM_MEMCPY(curThread_w2948aaac35b1c18->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac35b1c18, sizeof(XMLVM_JMP_BUF)); goto label19; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 840)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1c18)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1c18,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1c18)
    label62:;
    XMLVM_TRY_BEGIN(w2948aaac35b1c20)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    XMLVM_CHECK_NPE(3)
    ((java_util_Hashtable_Entry*) _r3.o)->fields.java_util_Hashtable_Entry.next_ = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac35b1c20)
        XMLVM_CATCH_SPECIFIC(w2948aaac35b1c20,java_lang_Object,67)
    XMLVM_CATCH_END(w2948aaac35b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac35b1c20)
    goto label43;
    label67:;
    java_lang_Thread* curThread_w2948aaac35b1c23 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac35b1c23->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r7.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label70:;
    _r0 = _r5;
    goto label54;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_Hashtable_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_size__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "size", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 859)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w2948aaac36b1b4)
    // Begin try
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_Hashtable*) _r1.o)->fields.java_util_Hashtable.elementCount_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac36b1b4)
        XMLVM_CATCH_SPECIFIC(w2948aaac36b1b4,java_lang_Object,5)
    XMLVM_CATCH_END(w2948aaac36b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac36b1b4)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label5:;
    java_lang_Thread* curThread_w2948aaac36b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac36b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_toString__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "toString", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.o = me;
    // "luni.04"
    _r0.o = xmlvm_create_java_string_from_pool(169);
    // ")"
    _r0.o = xmlvm_create_java_string_from_pool(23);
    // "("
    _r0.o = xmlvm_create_java_string_from_pool(85);
    XMLVM_SOURCE_POSITION("Hashtable.java", 869)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    XMLVM_TRY_BEGIN(w2948aaac37b1b7)
    // Begin try
    //java_util_Hashtable_isEmpty__[8]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[8])(_r5.o);
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w2948aaac37b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1b7, sizeof(XMLVM_JMP_BUF)); goto label17; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 870)
    // "\173\175"
    _r0.o = xmlvm_create_java_string_from_pool(6);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1b7)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1b7,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1b7)
    label15:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 900)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label17:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 873)
    _r0.o = __NEW_java_lang_StringBuilder();
    //java_util_Hashtable_size__[12]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[12])(_r5.o);
    _r1.i = _r1.i * 28;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 874)
    _r1.i = 123;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 875)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.lastSlot_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c13,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c13)
    label35:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c15)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.firstSlot_;
    if (_r1.i >= _r2.i) { XMLVM_MEMCPY(curThread_w2948aaac37b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c15, sizeof(XMLVM_JMP_BUF)); goto label62; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 896)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementCount_;
    if (_r1.i <= 0) { XMLVM_MEMCPY(curThread_w2948aaac37b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c15, sizeof(XMLVM_JMP_BUF)); goto label52; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 897)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_AbstractStringBuilder_length__(_r0.o);
    _r2.i = 2;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_setLength___int(_r0.o, _r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c15,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c15)
    label52:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c17)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 899)
    _r1.i = 125;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    { XMLVM_MEMCPY(curThread_w2948aaac37b1c17->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c17, sizeof(XMLVM_JMP_BUF)); goto label15; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 876)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c17)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c17,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c17)
    label62:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c19)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_Hashtable*) _r5.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c19)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c19,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c19)
    label66:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c21)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 877)
    if (_r2.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac37b1c21->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c21, sizeof(XMLVM_JMP_BUF)); goto label71; };
    _r1.i = _r1.i + -1;
    { XMLVM_MEMCPY(curThread_w2948aaac37b1c21->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c21, sizeof(XMLVM_JMP_BUF)); goto label35; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 878)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c21)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c21,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c21)
    label71:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c23)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.key_;
    if (_r3.o == _r5.o) { XMLVM_MEMCPY(curThread_w2948aaac37b1c23->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c23, sizeof(XMLVM_JMP_BUF)); goto label102; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 879)
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.key_;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c23)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c23,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c23)
    label80:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c25)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 884)
    _r3.i = 61;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r3.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 885)
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;
    if (_r3.o == _r5.o) { XMLVM_MEMCPY(curThread_w2948aaac37b1c25->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c25, sizeof(XMLVM_JMP_BUF)); goto label136; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 886)
    XMLVM_CHECK_NPE(2)
    _r3.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c25)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c25,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c25)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c25)
    label94:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 891)
    // ", "
    _r3.o = xmlvm_create_java_string_from_pool(7);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 892)
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_;
    { XMLVM_MEMCPY(curThread_w2948aaac37b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac37b1c27, sizeof(XMLVM_JMP_BUF)); goto label66; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 882)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c27)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c27,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c27)
    label102:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c29)
    // Begin try
    _r3.o = __NEW_java_lang_StringBuilder();
    // "("
    _r4.o = xmlvm_create_java_string_from_pool(85);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____java_lang_String(_r3.o, _r4.o);
    // "luni.04"
    _r4.o = xmlvm_create_java_string_from_pool(169);
    _r4.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r4.o);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    // ")"
    _r4.o = xmlvm_create_java_string_from_pool(23);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c29)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c29,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c29)
    goto label80;
    label133:;
    java_lang_Thread* curThread_w2948aaac37b1c32 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac37b1c32->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label136:;
    XMLVM_TRY_BEGIN(w2948aaac37b1c36)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 889)
    _r3.o = __NEW_java_lang_StringBuilder();
    // "("
    _r4.o = xmlvm_create_java_string_from_pool(85);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____java_lang_String(_r3.o, _r4.o);
    // "luni.04"
    _r4.o = xmlvm_create_java_string_from_pool(169);
    _r4.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r4.o);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    // ")"
    _r4.o = xmlvm_create_java_string_from_pool(23);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac37b1c36)
        XMLVM_CATCH_SPECIFIC(w2948aaac37b1c36,java_lang_Object,133)
    XMLVM_CATCH_END(w2948aaac37b1c36)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac37b1c36)
    goto label94;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_values__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_values__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "values", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 911)

    
    // Red class access removed: java.util.Collections$SynchronizedCollection::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 912)

    
    // Red class access removed: java.util.Hashtable$7::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Hashtable$7::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Collections$SynchronizedCollection::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "writeObject", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Hashtable.java", 942)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w2948aaac39b1b5)
    // Begin try

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 943)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 944)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementCount_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 945)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac39b1b5)
        XMLVM_CATCH_SPECIFIC(w2948aaac39b1b5,java_lang_Object,43)
    XMLVM_CATCH_END(w2948aaac39b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac39b1b5)
    label18:;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label24;
    XMLVM_SOURCE_POSITION("Hashtable.java", 953)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return;
    label24:;
    XMLVM_TRY_BEGIN(w2948aaac39b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 946)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_Hashtable*) _r3.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac39b1c13)
        XMLVM_CATCH_SPECIFIC(w2948aaac39b1c13,java_lang_Object,43)
    XMLVM_CATCH_END(w2948aaac39b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac39b1c13)
    label28:;
    XMLVM_TRY_BEGIN(w2948aaac39b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("Hashtable.java", 947)
    if (_r1.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w2948aaac39b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w2948aaac39b1c15, sizeof(XMLVM_JMP_BUF)); goto label18; };
    XMLVM_SOURCE_POSITION("Hashtable.java", 948)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.key_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 949)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 950)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_Hashtable_Entry*) _r1.o)->fields.java_util_Hashtable_Entry.next_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2948aaac39b1c15)
        XMLVM_CATCH_SPECIFIC(w2948aaac39b1c15,java_lang_Object,43)
    XMLVM_CATCH_END(w2948aaac39b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w2948aaac39b1c15)
    goto label28;
    label43:;
    java_lang_Thread* curThread_w2948aaac39b1c18 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2948aaac39b1c18->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "readObject", "?")
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
    XMLVM_SOURCE_POSITION("Hashtable.java", 958)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 959)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 960)
    XMLVM_CHECK_NPE(6)
    _r1.o = java_util_Hashtable_newElementArray___int(_r6.o, _r0.i);
    XMLVM_CHECK_NPE(6)
    ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 961)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(6)
    ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.elementCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 962)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.elementCount_;
    label21:;
    _r1.i = _r1.i + -1;
    if (_r1.i >= 0) goto label26;
    XMLVM_SOURCE_POSITION("Hashtable.java", 976)
    XMLVM_EXIT_METHOD()
    return;
    label26:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 963)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Hashtable.java", 964)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(2)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[4])(_r2.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 965)
    _r4.i = 2147483647;
    _r4.i = _r4.i & _r3.i;
    _r4.i = _r4.i % _r0.i;
    XMLVM_SOURCE_POSITION("Hashtable.java", 966)
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.firstSlot_;
    if (_r4.i >= _r5.i) goto label45;
    XMLVM_SOURCE_POSITION("Hashtable.java", 967)
    XMLVM_CHECK_NPE(6)
    ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.firstSlot_ = _r4.i;
    label45:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 969)
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.lastSlot_;
    if (_r4.i <= _r5.i) goto label51;
    XMLVM_SOURCE_POSITION("Hashtable.java", 970)
    XMLVM_CHECK_NPE(6)
    ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.lastSlot_ = _r4.i;
    label51:;
    XMLVM_SOURCE_POSITION("Hashtable.java", 972)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    _r2.o = java_util_Hashtable_newEntry___java_lang_Object_java_lang_Object_int(_r2.o, _r5.o, _r3.i);
    XMLVM_SOURCE_POSITION("Hashtable.java", 973)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(2)
    ((java_util_Hashtable_Entry*) _r2.o)->fields.java_util_Hashtable_Entry.next_ = _r3.o;
    XMLVM_SOURCE_POSITION("Hashtable.java", 974)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_Hashtable*) _r6.o)->fields.java_util_Hashtable.elementData_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r2.o;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_access$0__()
{
    XMLVM_CLASS_INIT(java_util_Hashtable)
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_access$0__]
    XMLVM_ENTER_METHOD("java.util.Hashtable", "access$0", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Hashtable.java", 70)
    _r0.o = java_util_Hashtable_GET_EMPTY_ITERATOR();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

