#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_util_Collection.h"
#include "java_util_HashMap_Entry.h"
#include "java_util_Iterator.h"
#include "java_util_Map_Entry.h"
#include "java_util_Set.h"

#include "java_util_HashMap.h"

#define XMLVM_CURRENT_CLASS_NAME HashMap
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_HashMap

__TIB_DEFINITION_java_util_HashMap __TIB_java_util_HashMap = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_HashMap, // classInitializer
    "java.util.HashMap", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/util/AbstractMap<TK;TV;>;Ljava/util/Map<TK;TV;>;Ljava/lang/Cloneable;Ljava/io/Serializable;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractMap, // extends
    sizeof(java_util_HashMap), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_HashMap;
JAVA_OBJECT __CLASS_java_util_HashMap_1ARRAY;
JAVA_OBJECT __CLASS_java_util_HashMap_2ARRAY;
JAVA_OBJECT __CLASS_java_util_HashMap_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_HashMap_serialVersionUID;
static JAVA_INT _STATIC_java_util_HashMap_DEFAULT_SIZE;

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

void __INIT_java_util_HashMap()
{
    staticInitializerLock(&__TIB_java_util_HashMap);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_HashMap.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_HashMap.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_HashMap);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_HashMap.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_HashMap.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_HashMap.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.HashMap")
        __INIT_IMPL_java_util_HashMap();
    }
}

void __INIT_IMPL_java_util_HashMap()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractMap)
    __TIB_java_util_HashMap.newInstanceFunc = __NEW_INSTANCE_java_util_HashMap;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_HashMap.vtable, __TIB_java_util_AbstractMap.vtable, sizeof(__TIB_java_util_AbstractMap.vtable));
    // Initialize vtable for this class
    __TIB_java_util_HashMap.vtable[6] = (VTABLE_PTR) &java_util_HashMap_clear__;
    __TIB_java_util_HashMap.vtable[0] = (VTABLE_PTR) &java_util_HashMap_clone__;
    __TIB_java_util_HashMap.vtable[7] = (VTABLE_PTR) &java_util_HashMap_containsKey___java_lang_Object;
    __TIB_java_util_HashMap.vtable[8] = (VTABLE_PTR) &java_util_HashMap_containsValue___java_lang_Object;
    __TIB_java_util_HashMap.vtable[9] = (VTABLE_PTR) &java_util_HashMap_entrySet__;
    __TIB_java_util_HashMap.vtable[10] = (VTABLE_PTR) &java_util_HashMap_get___java_lang_Object;
    __TIB_java_util_HashMap.vtable[11] = (VTABLE_PTR) &java_util_HashMap_isEmpty__;
    __TIB_java_util_HashMap.vtable[12] = (VTABLE_PTR) &java_util_HashMap_keySet__;
    __TIB_java_util_HashMap.vtable[14] = (VTABLE_PTR) &java_util_HashMap_put___java_lang_Object_java_lang_Object;
    __TIB_java_util_HashMap.vtable[13] = (VTABLE_PTR) &java_util_HashMap_putAll___java_util_Map;
    __TIB_java_util_HashMap.vtable[15] = (VTABLE_PTR) &java_util_HashMap_remove___java_lang_Object;
    __TIB_java_util_HashMap.vtable[16] = (VTABLE_PTR) &java_util_HashMap_size__;
    __TIB_java_util_HashMap.vtable[17] = (VTABLE_PTR) &java_util_HashMap_values__;
    // Initialize interface information
    __TIB_java_util_HashMap.numImplementedInterfaces = 3;
    __TIB_java_util_HashMap.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_HashMap.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_HashMap.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_util_Map)

    __TIB_java_util_HashMap.implementedInterfaces[0][2] = &__TIB_java_util_Map;
    // Initialize itable for this class
    __TIB_java_util_HashMap.itableBegin = &__TIB_java_util_HashMap.itable[0];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_clear__] = __TIB_java_util_HashMap.vtable[6];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object] = __TIB_java_util_HashMap.vtable[7];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object] = __TIB_java_util_HashMap.vtable[8];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_entrySet__] = __TIB_java_util_HashMap.vtable[9];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object] = __TIB_java_util_HashMap.vtable[1];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object] = __TIB_java_util_HashMap.vtable[10];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_hashCode__] = __TIB_java_util_HashMap.vtable[4];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__] = __TIB_java_util_HashMap.vtable[11];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_keySet__] = __TIB_java_util_HashMap.vtable[12];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map] = __TIB_java_util_HashMap.vtable[13];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object] = __TIB_java_util_HashMap.vtable[14];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object] = __TIB_java_util_HashMap.vtable[15];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_size__] = __TIB_java_util_HashMap.vtable[16];
    __TIB_java_util_HashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_values__] = __TIB_java_util_HashMap.vtable[17];

    _STATIC_java_util_HashMap_serialVersionUID = 362498820763181265;
    _STATIC_java_util_HashMap_DEFAULT_SIZE = 16;

    __TIB_java_util_HashMap.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_HashMap.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_HashMap.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_HashMap.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_HashMap.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_HashMap.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_HashMap.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_HashMap.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_HashMap = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_HashMap);
    __TIB_java_util_HashMap.clazz = __CLASS_java_util_HashMap;
    __TIB_java_util_HashMap.baseType = JAVA_NULL;
    __CLASS_java_util_HashMap_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap);
    __CLASS_java_util_HashMap_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap_1ARRAY);
    __CLASS_java_util_HashMap_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_HashMap_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_HashMap]
    //XMLVM_END_WRAPPER

    __TIB_java_util_HashMap.classInitialized = 1;
}

void __DELETE_java_util_HashMap(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_HashMap]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_HashMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractMap(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_HashMap*) me)->fields.java_util_HashMap.elementCount_ = 0;
    ((java_util_HashMap*) me)->fields.java_util_HashMap.elementData_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_util_HashMap*) me)->fields.java_util_HashMap.modCount_ = 0;
    ((java_util_HashMap*) me)->fields.java_util_HashMap.loadFactor_ = 0;
    ((java_util_HashMap*) me)->fields.java_util_HashMap.threshold_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_HashMap]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_HashMap()
{    XMLVM_CLASS_INIT(java_util_HashMap)
java_util_HashMap* me = (java_util_HashMap*) XMLVM_MALLOC(sizeof(java_util_HashMap));
    me->tib = &__TIB_java_util_HashMap;
    __INIT_INSTANCE_MEMBERS_java_util_HashMap(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_HashMap]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_HashMap()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_HashMap();
    java_util_HashMap___INIT___(me);
    return me;
}

JAVA_LONG java_util_HashMap_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    return _STATIC_java_util_HashMap_serialVersionUID;
}

void java_util_HashMap_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
_STATIC_java_util_HashMap_serialVersionUID = v;
}

JAVA_INT java_util_HashMap_GET_DEFAULT_SIZE()
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    return _STATIC_java_util_HashMap_DEFAULT_SIZE;
}

void java_util_HashMap_PUT_DEFAULT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
_STATIC_java_util_HashMap_DEFAULT_SIZE = v;
}

JAVA_OBJECT java_util_HashMap_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_newElementArray___int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "newElementArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 268)
    XMLVM_CLASS_INIT(java_util_HashMap_Entry)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_util_HashMap_Entry, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap___INIT___]
    XMLVM_ENTER_METHOD("java.util.HashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 275)
    _r0.i = 16;
    XMLVM_CHECK_NPE(1)
    java_util_HashMap___INIT____int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 276)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap___INIT____int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 287)
    _r0.f = 0.75;
    XMLVM_CHECK_NPE(1)
    java_util_HashMap___INIT____int_float(_r1.o, _r2.i, _r0.f);
    XMLVM_SOURCE_POSITION("HashMap.java", 288)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_HashMap_calculateCapacity___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_calculateCapacity___int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "calculateCapacity", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    _r0.i = 1073741824;
    XMLVM_SOURCE_POSITION("HashMap.java", 299)
    if (_r2.i < _r0.i) goto label5;
    label4:;
    XMLVM_SOURCE_POSITION("HashMap.java", 311)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label5:;
    XMLVM_SOURCE_POSITION("HashMap.java", 302)
    if (_r2.i != 0) goto label10;
    XMLVM_SOURCE_POSITION("HashMap.java", 303)
    _r0.i = 16;
    goto label4;
    label10:;
    XMLVM_SOURCE_POSITION("HashMap.java", 305)
    _r0.i = _r2.i + -1;
    _r1.i = _r0.i >> 1;
    _r0.i = _r0.i | _r1.i;
    _r1.i = _r0.i >> 2;
    _r0.i = _r0.i | _r1.i;
    _r1.i = _r0.i >> 4;
    _r0.i = _r0.i | _r1.i;
    _r1.i = _r0.i >> 8;
    _r0.i = _r0.i | _r1.i;
    _r1.i = _r0.i >> 16;
    _r0.i = _r0.i | _r1.i;
    _r0.i = _r0.i + 1;
    goto label4;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap___INIT____int_float]
    XMLVM_ENTER_METHOD("java.util.HashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.i = n1;
    _r4.f = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("HashMap.java", 326)
    XMLVM_CHECK_NPE(2)
    java_util_AbstractMap___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 48)
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.modCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 327)
    if (_r3.i < 0) goto label31;
    _r0.f = 0.0;
    _r0.i = _r4.f > _r0.f ? 1 : (_r4.f == _r0.f ? 0 : -1);
    if (_r0.i <= 0) goto label31;
    XMLVM_SOURCE_POSITION("HashMap.java", 328)
    _r0.i = java_util_HashMap_calculateCapacity___int(_r3.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 329)
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 330)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_HashMap_newElementArray___int(_r2.o, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_ = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 331)
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.loadFactor_ = _r4.f;
    XMLVM_SOURCE_POSITION("HashMap.java", 332)
    XMLVM_CHECK_NPE(2)
    java_util_HashMap_computeThreshold__(_r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 336)
    XMLVM_EXIT_METHOD()
    return;
    label31:;
    XMLVM_SOURCE_POSITION("HashMap.java", 334)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_HashMap___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap___INIT____java_util_Map]
    XMLVM_ENTER_METHOD("java.util.HashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 346)
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r2.o);
    _r0.i = java_util_HashMap_calculateCapacity___int(_r0.i);
    XMLVM_CHECK_NPE(1)
    java_util_HashMap___INIT____int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 347)
    XMLVM_CHECK_NPE(1)
    java_util_HashMap_putAllImpl___java_util_Map(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 348)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_clear__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 358)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementCount_;
    if (_r0.i <= 0) goto label19;
    XMLVM_SOURCE_POSITION("HashMap.java", 359)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 360)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 361)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.modCount_ = _r0.i;
    label19:;
    XMLVM_SOURCE_POSITION("HashMap.java", 363)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_clone__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_TRY_BEGIN(w2219aaac14b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("HashMap.java", 374)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_AbstractMap_clone__(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 375)
    _r1.i = 0;
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.elementCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 376)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(2)
    _r1.o = java_util_HashMap_newElementArray___int(_r2.o, _r1.i);
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap*) _r0.o)->fields.java_util_HashMap.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 377)
    //java_util_HashMap_putAll___java_util_Map[13]
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[13])(_r0.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2219aaac14b1b2)
    XMLVM_CATCH_END(w2219aaac14b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w2219aaac14b1b2)
    label21:;
    XMLVM_SOURCE_POSITION("HashMap.java", 381)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label22:;
    java_lang_Thread* curThread_w2219aaac14b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2219aaac14b1b7->fields.java_lang_Thread.xmlvmException_;
    _r0.o = JAVA_NULL;
    goto label21;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_computeThreshold__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_computeThreshold__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "computeThreshold", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 389)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_CHECK_NPE(2)
    _r1.f = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.loadFactor_;
    _r0.f = _r0.f * _r1.f;
    _r0.i = (JAVA_INT) _r0.f;
    XMLVM_CHECK_NPE(2)
    ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.threshold_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 390)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_containsKey___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "containsKey", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 402)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_HashMap_getEntry___java_lang_Object(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 403)
    if (_r0.o == JAVA_NULL) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_containsValue___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "containsValue", "?")
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
    XMLVM_SOURCE_POSITION("HashMap.java", 416)
    if (_r6.o == JAVA_NULL) goto label34;
    _r0 = _r3;
    label5:;
    XMLVM_SOURCE_POSITION("HashMap.java", 417)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i < _r1.i) goto label12;
    label10:;
    _r0 = _r3;
    label11:;
    XMLVM_SOURCE_POSITION("HashMap.java", 437)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("HashMap.java", 418)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label16:;
    XMLVM_SOURCE_POSITION("HashMap.java", 419)
    if (_r1.o != JAVA_NULL) goto label21;
    _r0.i = _r0.i + 1;
    goto label5;
    label21:;
    XMLVM_SOURCE_POSITION("HashMap.java", 420)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;
    _r2.i = java_util_HashMap_areEqualValues___java_lang_Object_java_lang_Object(_r6.o, _r2.o);
    if (_r2.i == 0) goto label31;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("HashMap.java", 421)
    goto label11;
    label31:;
    XMLVM_SOURCE_POSITION("HashMap.java", 423)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    goto label16;
    label34:;
    _r0 = _r3;
    label35:;
    XMLVM_SOURCE_POSITION("HashMap.java", 427)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i >= _r1.i) goto label10;
    XMLVM_SOURCE_POSITION("HashMap.java", 428)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label44:;
    XMLVM_SOURCE_POSITION("HashMap.java", 429)
    if (_r1.o != JAVA_NULL) goto label49;
    _r0.i = _r0.i + 1;
    goto label35;
    label49:;
    XMLVM_SOURCE_POSITION("HashMap.java", 430)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.value_;
    if (_r2.o != JAVA_NULL) goto label55;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("HashMap.java", 431)
    goto label11;
    label55:;
    XMLVM_SOURCE_POSITION("HashMap.java", 433)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    goto label44;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_entrySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_entrySet__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "entrySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 449)

    
    // Red class access removed: java.util.HashMap$HashMapEntrySet::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.HashMap$HashMapEntrySet::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_get___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 462)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_HashMap_getEntry___java_lang_Object(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 463)
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("HashMap.java", 464)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.value_;
    label8:;
    XMLVM_SOURCE_POSITION("HashMap.java", 466)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    _r0.o = JAVA_NULL;
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_getEntry___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "getEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 471)
    if (_r4.o != JAVA_NULL) goto label7;
    XMLVM_SOURCE_POSITION("HashMap.java", 472)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashMap_findNullKeyEntry__(_r3.o);
    label6:;
    XMLVM_SOURCE_POSITION("HashMap.java", 478)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label7:;
    XMLVM_SOURCE_POSITION("HashMap.java", 474)
    _r0.i = java_util_HashMap_computeHashCode___java_lang_Object(_r4.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 475)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    _r1.i = _r1.i & _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 476)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashMap_findNonNullKeyEntry___java_lang_Object_int_int(_r3.o, _r4.o, _r1.i, _r0.i);
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_findNonNullKeyEntry___java_lang_Object_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_findNonNullKeyEntry___java_lang_Object_int_int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "findNonNullKeyEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("HashMap.java", 482)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    label4:;
    XMLVM_SOURCE_POSITION("HashMap.java", 483)
    if (_r0.o == JAVA_NULL) goto label18;
    XMLVM_SOURCE_POSITION("HashMap.java", 484)
    XMLVM_CHECK_NPE(0)
    _r1.i = ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.origKeyHash_;
    if (_r1.i != _r5.i) goto label19;
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.key_;
    _r1.i = java_util_HashMap_areEqualKeys___java_lang_Object_java_lang_Object(_r3.o, _r1.o);
    if (_r1.i == 0) goto label19;
    label18:;
    XMLVM_SOURCE_POSITION("HashMap.java", 487)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label19:;
    XMLVM_SOURCE_POSITION("HashMap.java", 485)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_findNullKeyEntry__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_findNullKeyEntry__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "findNullKeyEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 491)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    _r1.i = 0;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    label5:;
    XMLVM_SOURCE_POSITION("HashMap.java", 492)
    if (_r0.o == JAVA_NULL) goto label11;
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.key_;
    if (_r1.o != JAVA_NULL) goto label12;
    label11:;
    XMLVM_SOURCE_POSITION("HashMap.java", 494)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label12:;
    XMLVM_SOURCE_POSITION("HashMap.java", 493)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashMap_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 506)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_HashMap*) _r1.o)->fields.java_util_HashMap.elementCount_;
    if (_r0.i != 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_keySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_keySet__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "keySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 518)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_;
    if (_r0.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("HashMap.java", 519)

    
    // Red class access removed: java.util.HashMap$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.HashMap$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_ = _r0.o;
    label11:;
    XMLVM_SOURCE_POSITION("HashMap.java", 547)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_put___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("HashMap.java", 562)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_HashMap_putImpl___java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_putImpl___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_putImpl___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "putImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.o = n2;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("HashMap.java", 567)
    if (_r4.o != JAVA_NULL) goto label38;
    XMLVM_SOURCE_POSITION("HashMap.java", 568)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashMap_findNullKeyEntry__(_r3.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 569)
    if (_r0.o != JAVA_NULL) goto label33;
    XMLVM_SOURCE_POSITION("HashMap.java", 570)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 571)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashMap_createHashedEntry___java_lang_Object_int_int(_r3.o, _r0.o, _r1.i, _r1.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 572)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementCount_ = _r1.i;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.threshold_;
    if (_r1.i <= _r2.i) goto label33;
    XMLVM_SOURCE_POSITION("HashMap.java", 573)
    XMLVM_CHECK_NPE(3)
    java_util_HashMap_rehash__(_r3.o);
    label33:;
    XMLVM_SOURCE_POSITION("HashMap.java", 589)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.value_;
    XMLVM_SOURCE_POSITION("HashMap.java", 590)
    XMLVM_CHECK_NPE(0)
    ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.value_ = _r5.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 591)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label38:;
    XMLVM_SOURCE_POSITION("HashMap.java", 577)
    _r0.i = java_util_HashMap_computeHashCode___java_lang_Object(_r4.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 578)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    _r1.i = _r1.i & _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 579)
    XMLVM_CHECK_NPE(3)
    _r2.o = java_util_HashMap_findNonNullKeyEntry___java_lang_Object_int_int(_r3.o, _r4.o, _r1.i, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 580)
    if (_r2.o != JAVA_NULL) goto label78;
    XMLVM_SOURCE_POSITION("HashMap.java", 581)
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.modCount_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.modCount_ = _r2.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 582)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_HashMap_createHashedEntry___java_lang_Object_int_int(_r3.o, _r4.o, _r1.i, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 583)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementCount_ = _r1.i;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.threshold_;
    if (_r1.i <= _r2.i) goto label33;
    XMLVM_SOURCE_POSITION("HashMap.java", 584)
    XMLVM_CHECK_NPE(3)
    java_util_HashMap_rehash__(_r3.o);
    goto label33;
    label78:;
    _r0 = _r2;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_createEntry___java_lang_Object_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_createEntry___java_lang_Object_int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "createEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    _r5.o = n3;
    XMLVM_SOURCE_POSITION("HashMap.java", 595)
    _r0.o = __NEW_java_util_HashMap_Entry();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap_Entry___INIT____java_lang_Object_java_lang_Object(_r0.o, _r3.o, _r5.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 596)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 597)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 598)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_createHashedEntry___java_lang_Object_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_createHashedEntry___java_lang_Object_int_int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "createHashedEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("HashMap.java", 602)
    _r0.o = __NEW_java_util_HashMap_Entry();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap_Entry___INIT____java_lang_Object_int(_r0.o, _r3.o, _r5.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 603)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 604)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 605)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_putAll___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.HashMap", "putAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 620)
    XMLVM_CHECK_NPE(2)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__])(_r2.o);
    if (_r0.i != 0) goto label9;
    XMLVM_SOURCE_POSITION("HashMap.java", 621)
    XMLVM_CHECK_NPE(1)
    java_util_HashMap_putAllImpl___java_util_Map(_r1.o, _r2.o);
    label9:;
    XMLVM_SOURCE_POSITION("HashMap.java", 623)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_putAllImpl___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_putAllImpl___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.HashMap", "putAllImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 626)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.elementCount_;
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r4.o);
    _r0.i = _r0.i + _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 627)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_HashMap*) _r3.o)->fields.java_util_HashMap.threshold_;
    if (_r0.i <= _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("HashMap.java", 628)
    XMLVM_CHECK_NPE(3)
    java_util_HashMap_rehash___int(_r3.o, _r0.i);
    label14:;
    XMLVM_SOURCE_POSITION("HashMap.java", 630)
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_entrySet__])(_r4.o);
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    label22:;
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r1.o);
    if (_r0.i != 0) goto label29;
    XMLVM_SOURCE_POSITION("HashMap.java", 633)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r1.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 631)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    XMLVM_CHECK_NPE(3)
    java_util_HashMap_putImpl___java_lang_Object_java_lang_Object(_r3.o, _r2.o, _r0.o);
    goto label22;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_rehash___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_rehash___int]
    XMLVM_ENTER_METHOD("java.util.HashMap", "rehash", "?")
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
    _r8.o = me;
    _r9.i = n1;
    _r7.i = 1;
    XMLVM_SOURCE_POSITION("HashMap.java", 636)
    if (_r9.i != 0) goto label24;
    _r0 = _r7;
    label4:;
    _r0.i = java_util_HashMap_calculateCapacity___int(_r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 638)
    XMLVM_CHECK_NPE(8)
    _r1.o = java_util_HashMap_newElementArray___int(_r8.o, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 639)
    _r2.i = 0;
    label13:;
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r2.i < _r3.i) goto label27;
    XMLVM_SOURCE_POSITION("HashMap.java", 650)
    XMLVM_CHECK_NPE(8)
    ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 651)
    XMLVM_CHECK_NPE(8)
    java_util_HashMap_computeThreshold__(_r8.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 652)
    XMLVM_EXIT_METHOD()
    return;
    label24:;
    _r0.i = _r9.i << 1;
    goto label4;
    label27:;
    XMLVM_SOURCE_POSITION("HashMap.java", 640)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("HashMap.java", 641)
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    _r5.o = JAVA_NULL;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r5.o;
    label36:;
    XMLVM_SOURCE_POSITION("HashMap.java", 642)
    if (_r3.o != JAVA_NULL) goto label41;
    _r2.i = _r2.i + 1;
    goto label13;
    label41:;
    XMLVM_SOURCE_POSITION("HashMap.java", 643)
    XMLVM_CHECK_NPE(3)
    _r4.i = ((java_util_HashMap_Entry*) _r3.o)->fields.java_util_HashMap_Entry.origKeyHash_;
    _r5.i = _r0.i - _r7.i;
    _r4.i = _r4.i & _r5.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 644)
    XMLVM_CHECK_NPE(3)
    _r5.o = ((java_util_HashMap_Entry*) _r3.o)->fields.java_util_HashMap_Entry.next_;
    XMLVM_SOURCE_POSITION("HashMap.java", 645)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(3)
    ((java_util_HashMap_Entry*) _r3.o)->fields.java_util_HashMap_Entry.next_ = _r6.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 646)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r3.o;
    _r3 = _r5;
    XMLVM_SOURCE_POSITION("HashMap.java", 647)
    goto label36;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_rehash__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_rehash__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "rehash", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 655)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashMap*) _r1.o)->fields.java_util_HashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(1)
    java_util_HashMap_rehash___int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("HashMap.java", 656)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 668)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_HashMap_removeEntry___java_lang_Object(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 669)
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("HashMap.java", 670)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_MapEntry*) _r0.o)->fields.java_util_MapEntry.value_;
    label8:;
    XMLVM_SOURCE_POSITION("HashMap.java", 672)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    _r0.o = JAVA_NULL;
    goto label8;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_removeEntry___java_util_HashMap_Entry(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_removeEntry___java_util_HashMap_Entry]
    XMLVM_ENTER_METHOD("java.util.HashMap", "removeEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("HashMap.java", 680)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_HashMap_Entry*) _r5.o)->fields.java_util_HashMap_Entry.origKeyHash_;
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r1.i = _r1.i - _r3.i;
    _r0.i = _r0.i & _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 681)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("HashMap.java", 682)
    if (_r1.o != _r5.o) goto label43;
    XMLVM_SOURCE_POSITION("HashMap.java", 683)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_HashMap_Entry*) _r5.o)->fields.java_util_HashMap_Entry.next_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.o;
    label20:;
    XMLVM_SOURCE_POSITION("HashMap.java", 691)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 692)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.elementCount_;
    _r0.i = _r0.i - _r3.i;
    XMLVM_CHECK_NPE(4)
    ((java_util_HashMap*) _r4.o)->fields.java_util_HashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 693)
    XMLVM_EXIT_METHOD()
    return;
    label32:;
    XMLVM_SOURCE_POSITION("HashMap.java", 686)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_;
    label34:;
    XMLVM_SOURCE_POSITION("HashMap.java", 685)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_;
    if (_r1.o != _r5.o) goto label32;
    XMLVM_SOURCE_POSITION("HashMap.java", 688)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_HashMap_Entry*) _r5.o)->fields.java_util_HashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_ = _r1.o;
    goto label20;
    label43:;
    _r0 = _r1;
    goto label34;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_removeEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_removeEntry___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "removeEntry", "?")
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
    _r8.o = me;
    _r9.o = n1;
    _r6.o = JAVA_NULL;
    _r5.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("HashMap.java", 696)
    XMLVM_SOURCE_POSITION("HashMap.java", 699)
    if (_r9.o == JAVA_NULL) goto label47;
    XMLVM_SOURCE_POSITION("HashMap.java", 700)
    _r0.i = java_util_HashMap_computeHashCode___java_lang_Object(_r9.o);
    XMLVM_SOURCE_POSITION("HashMap.java", 701)
    XMLVM_CHECK_NPE(8)
    _r1.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r1.i = _r1.i - _r5.i;
    _r1.i = _r1.i & _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 702)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3 = _r2;
    _r2 = _r6;
    label20:;
    XMLVM_SOURCE_POSITION("HashMap.java", 703)
    if (_r3.o == JAVA_NULL) goto label93;
    XMLVM_CHECK_NPE(3)
    _r4.i = ((java_util_HashMap_Entry*) _r3.o)->fields.java_util_HashMap_Entry.origKeyHash_;
    if (_r4.i != _r0.i) goto label41;
    XMLVM_CHECK_NPE(3)
    _r4.o = ((java_util_MapEntry*) _r3.o)->fields.java_util_MapEntry.key_;
    _r4.i = java_util_HashMap_areEqualKeys___java_lang_Object_java_lang_Object(_r9.o, _r4.o);
    if (_r4.i == 0) goto label41;
    _r0 = _r2;
    _r2 = _r1;
    _r1 = _r3;
    label37:;
    XMLVM_SOURCE_POSITION("HashMap.java", 714)
    if (_r1.o != JAVA_NULL) goto label67;
    _r0 = _r6;
    label40:;
    XMLVM_SOURCE_POSITION("HashMap.java", 715)
    XMLVM_SOURCE_POSITION("HashMap.java", 724)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label41:;
    XMLVM_SOURCE_POSITION("HashMap.java", 705)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_HashMap_Entry*) _r3.o)->fields.java_util_HashMap_Entry.next_;
    _r7 = _r3;
    _r3 = _r2;
    _r2 = _r7;
    goto label20;
    label47:;
    XMLVM_SOURCE_POSITION("HashMap.java", 708)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r1 = _r0;
    _r0 = _r6;
    label53:;
    XMLVM_SOURCE_POSITION("HashMap.java", 709)
    if (_r1.o == JAVA_NULL) goto label59;
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_MapEntry*) _r1.o)->fields.java_util_MapEntry.key_;
    if (_r2.o != JAVA_NULL) goto label61;
    label59:;
    _r2 = _r3;
    goto label37;
    label61:;
    XMLVM_SOURCE_POSITION("HashMap.java", 711)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    _r7 = _r1;
    _r1 = _r0;
    _r0 = _r7;
    goto label53;
    label67:;
    XMLVM_SOURCE_POSITION("HashMap.java", 717)
    if (_r0.o != JAVA_NULL) goto label88;
    XMLVM_SOURCE_POSITION("HashMap.java", 718)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    _r3.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.o;
    label75:;
    XMLVM_SOURCE_POSITION("HashMap.java", 722)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 723)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementCount_;
    _r0.i = _r0.i - _r5.i;
    XMLVM_CHECK_NPE(8)
    ((java_util_HashMap*) _r8.o)->fields.java_util_HashMap.elementCount_ = _r0.i;
    _r0 = _r1;
    goto label40;
    label88:;
    XMLVM_SOURCE_POSITION("HashMap.java", 720)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_HashMap_Entry*) _r1.o)->fields.java_util_HashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    ((java_util_HashMap_Entry*) _r0.o)->fields.java_util_HashMap_Entry.next_ = _r2.o;
    goto label75;
    label93:;
    _r0 = _r2;
    _r2 = _r1;
    _r1 = _r3;
    goto label37;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_HashMap_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_size__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 734)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_HashMap*) _r1.o)->fields.java_util_HashMap.elementCount_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_HashMap_values__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_values__]
    XMLVM_ENTER_METHOD("java.util.HashMap", "values", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HashMap.java", 758)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_;
    if (_r0.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("HashMap.java", 759)

    
    // Red class access removed: java.util.HashMap$2::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.HashMap$2::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_ = _r0.o;
    label11:;
    XMLVM_SOURCE_POSITION("HashMap.java", 781)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.HashMap", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 785)

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 786)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 787)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_HashMap*) _r2.o)->fields.java_util_HashMap.elementCount_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 788)
    //java_util_HashMap_entrySet__[9]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_HashMap*) _r2.o)->tib->vtable[9])(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    label22:;
    XMLVM_SOURCE_POSITION("HashMap.java", 789)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label29;
    XMLVM_SOURCE_POSITION("HashMap.java", 795)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_SOURCE_POSITION("HashMap.java", 790)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    _r2.o = _r2.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 791)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.key_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 792)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_MapEntry*) _r2.o)->fields.java_util_MapEntry.value_;

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 793)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_HashMap_Entry*) _r2.o)->fields.java_util_HashMap_Entry.next_;
    goto label22;
    //XMLVM_END_WRAPPER
}

void java_util_HashMap_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.HashMap", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 800)

    
    // Red class access removed: java.io.ObjectInputStream::defaultReadObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 801)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 802)
    XMLVM_CHECK_NPE(5)
    _r1.o = java_util_HashMap_newElementArray___int(_r5.o, _r0.i);
    XMLVM_CHECK_NPE(5)
    ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("HashMap.java", 803)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(5)
    ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("HashMap.java", 804)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_HashMap*) _r5.o)->fields.java_util_HashMap.elementCount_;
    label21:;
    _r1.i = _r1.i + -1;
    if (_r1.i >= 0) goto label26;
    XMLVM_SOURCE_POSITION("HashMap.java", 809)
    XMLVM_EXIT_METHOD()
    return;
    label26:;
    XMLVM_SOURCE_POSITION("HashMap.java", 805)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("HashMap.java", 806)
    if (_r2.o != JAVA_NULL) goto label41;
    _r3.i = 0;
    label33:;
    XMLVM_SOURCE_POSITION("HashMap.java", 807)

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(5)
    java_util_HashMap_createEntry___java_lang_Object_int_java_lang_Object(_r5.o, _r2.o, _r3.i, _r4.o);
    goto label21;
    label41:;
    _r3.i = java_util_HashMap_computeHashCode___java_lang_Object(_r2.o);
    _r4.i = 1;
    _r4.i = _r0.i - _r4.i;
    _r3.i = _r3.i & _r4.i;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_HashMap_computeHashCode___java_lang_Object(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_computeHashCode___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "computeHashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("HashMap.java", 815)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[4])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashMap_areEqualKeys___java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_areEqualKeys___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "areEqualKeys", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("HashMap.java", 819)
    if (_r1.o == _r2.o) goto label10;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[1])(_r1.o, _r2.o);
    if (_r0.i != 0) goto label10;
    _r0.i = 0;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 1;
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_HashMap_areEqualValues___java_lang_Object_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_util_HashMap)
    //XMLVM_BEGIN_WRAPPER[java_util_HashMap_areEqualValues___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.HashMap", "areEqualValues", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("HashMap.java", 823)
    if (_r1.o == _r2.o) goto label10;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[1])(_r1.o, _r2.o);
    if (_r0.i != 0) goto label10;
    _r0.i = 0;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 1;
    goto label9;
    //XMLVM_END_WRAPPER
}

