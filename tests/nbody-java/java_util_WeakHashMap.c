#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_ref_Reference.h"
#include "java_lang_ref_ReferenceQueue.h"
#include "java_util_Collection.h"
#include "java_util_Set.h"
#include "java_util_WeakHashMap_Entry.h"

#include "java_util_WeakHashMap.h"

#define XMLVM_CURRENT_CLASS_NAME WeakHashMap
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_WeakHashMap

__TIB_DEFINITION_java_util_WeakHashMap __TIB_java_util_WeakHashMap = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_WeakHashMap, // classInitializer
    "java.util.WeakHashMap", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<K:Ljava/lang/Object;V:Ljava/lang/Object;>Ljava/util/AbstractMap<TK;TV;>;Ljava/util/Map<TK;TV;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractMap, // extends
    sizeof(java_util_WeakHashMap), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_WeakHashMap;
JAVA_OBJECT __CLASS_java_util_WeakHashMap_1ARRAY;
JAVA_OBJECT __CLASS_java_util_WeakHashMap_2ARRAY;
JAVA_OBJECT __CLASS_java_util_WeakHashMap_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_util_WeakHashMap_DEFAULT_SIZE;

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

void __INIT_java_util_WeakHashMap()
{
    staticInitializerLock(&__TIB_java_util_WeakHashMap);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_WeakHashMap.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_WeakHashMap.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_WeakHashMap);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_WeakHashMap.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_WeakHashMap.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_WeakHashMap.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.WeakHashMap")
        __INIT_IMPL_java_util_WeakHashMap();
    }
}

void __INIT_IMPL_java_util_WeakHashMap()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractMap)
    __TIB_java_util_WeakHashMap.newInstanceFunc = __NEW_INSTANCE_java_util_WeakHashMap;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_WeakHashMap.vtable, __TIB_java_util_AbstractMap.vtable, sizeof(__TIB_java_util_AbstractMap.vtable));
    // Initialize vtable for this class
    __TIB_java_util_WeakHashMap.vtable[6] = (VTABLE_PTR) &java_util_WeakHashMap_clear__;
    __TIB_java_util_WeakHashMap.vtable[7] = (VTABLE_PTR) &java_util_WeakHashMap_containsKey___java_lang_Object;
    __TIB_java_util_WeakHashMap.vtable[9] = (VTABLE_PTR) &java_util_WeakHashMap_entrySet__;
    __TIB_java_util_WeakHashMap.vtable[12] = (VTABLE_PTR) &java_util_WeakHashMap_keySet__;
    __TIB_java_util_WeakHashMap.vtable[17] = (VTABLE_PTR) &java_util_WeakHashMap_values__;
    __TIB_java_util_WeakHashMap.vtable[10] = (VTABLE_PTR) &java_util_WeakHashMap_get___java_lang_Object;
    __TIB_java_util_WeakHashMap.vtable[8] = (VTABLE_PTR) &java_util_WeakHashMap_containsValue___java_lang_Object;
    __TIB_java_util_WeakHashMap.vtable[11] = (VTABLE_PTR) &java_util_WeakHashMap_isEmpty__;
    __TIB_java_util_WeakHashMap.vtable[14] = (VTABLE_PTR) &java_util_WeakHashMap_put___java_lang_Object_java_lang_Object;
    __TIB_java_util_WeakHashMap.vtable[13] = (VTABLE_PTR) &java_util_WeakHashMap_putAll___java_util_Map;
    __TIB_java_util_WeakHashMap.vtable[15] = (VTABLE_PTR) &java_util_WeakHashMap_remove___java_lang_Object;
    __TIB_java_util_WeakHashMap.vtable[16] = (VTABLE_PTR) &java_util_WeakHashMap_size__;
    // Initialize interface information
    __TIB_java_util_WeakHashMap.numImplementedInterfaces = 1;
    __TIB_java_util_WeakHashMap.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_util_Map)

    __TIB_java_util_WeakHashMap.implementedInterfaces[0][0] = &__TIB_java_util_Map;
    // Initialize itable for this class
    __TIB_java_util_WeakHashMap.itableBegin = &__TIB_java_util_WeakHashMap.itable[0];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_clear__] = __TIB_java_util_WeakHashMap.vtable[6];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[7];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[8];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_entrySet__] = __TIB_java_util_WeakHashMap.vtable[9];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[1];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[10];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_hashCode__] = __TIB_java_util_WeakHashMap.vtable[4];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__] = __TIB_java_util_WeakHashMap.vtable[11];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_keySet__] = __TIB_java_util_WeakHashMap.vtable[12];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map] = __TIB_java_util_WeakHashMap.vtable[13];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[14];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object] = __TIB_java_util_WeakHashMap.vtable[15];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_size__] = __TIB_java_util_WeakHashMap.vtable[16];
    __TIB_java_util_WeakHashMap.itable[XMLVM_ITABLE_IDX_java_util_Map_values__] = __TIB_java_util_WeakHashMap.vtable[17];

    _STATIC_java_util_WeakHashMap_DEFAULT_SIZE = 16;

    __TIB_java_util_WeakHashMap.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_WeakHashMap.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_WeakHashMap.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_WeakHashMap.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_WeakHashMap.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_WeakHashMap.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_WeakHashMap.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_WeakHashMap.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_WeakHashMap = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_WeakHashMap);
    __TIB_java_util_WeakHashMap.clazz = __CLASS_java_util_WeakHashMap;
    __TIB_java_util_WeakHashMap.baseType = JAVA_NULL;
    __CLASS_java_util_WeakHashMap_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_WeakHashMap);
    __CLASS_java_util_WeakHashMap_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_WeakHashMap_1ARRAY);
    __CLASS_java_util_WeakHashMap_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_WeakHashMap_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_WeakHashMap]
    //XMLVM_END_WRAPPER

    __TIB_java_util_WeakHashMap.classInitialized = 1;
}

void __DELETE_java_util_WeakHashMap(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_WeakHashMap]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_WeakHashMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractMap(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.referenceQueue_ = (java_lang_ref_ReferenceQueue*) JAVA_NULL;
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.elementCount_ = 0;
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.elementData_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.loadFactor_ = 0;
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.threshold_ = 0;
    ((java_util_WeakHashMap*) me)->fields.java_util_WeakHashMap.modCount_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_WeakHashMap]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_WeakHashMap()
{    XMLVM_CLASS_INIT(java_util_WeakHashMap)
java_util_WeakHashMap* me = (java_util_WeakHashMap*) XMLVM_MALLOC(sizeof(java_util_WeakHashMap));
    me->tib = &__TIB_java_util_WeakHashMap;
    __INIT_INSTANCE_MEMBERS_java_util_WeakHashMap(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_WeakHashMap]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_WeakHashMap()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_WeakHashMap();
    java_util_WeakHashMap___INIT___(me);
    return me;
}

JAVA_INT java_util_WeakHashMap_GET_DEFAULT_SIZE()
{
    XMLVM_CLASS_INIT(java_util_WeakHashMap)
    return _STATIC_java_util_WeakHashMap_DEFAULT_SIZE;
}

void java_util_WeakHashMap_PUT_DEFAULT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_WeakHashMap)
_STATIC_java_util_WeakHashMap_DEFAULT_SIZE = v;
}

JAVA_OBJECT java_util_WeakHashMap_newEntryArray___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_util_WeakHashMap)
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_newEntryArray___int]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "newEntryArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 53)
    XMLVM_CLASS_INIT(java_util_WeakHashMap_Entry)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_util_WeakHashMap_Entry, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap___INIT___]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 188)
    _r0.i = 16;
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap___INIT____int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 189)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap___INIT____int]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 200)
    XMLVM_CHECK_NPE(1)
    java_util_AbstractMap___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 201)
    if (_r2.i < 0) goto label34;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 202)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 203)
    if (_r2.i != 0) goto label32;
    _r0.i = 1;
    label11:;
    _r0.o = java_util_WeakHashMap_newEntryArray___int(_r0.i);
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.elementData_ = _r0.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 204)
    _r0.i = 7500;
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.loadFactor_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 205)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_computeMaxSize__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 206)
    _r0.o = __NEW_java_lang_ref_ReferenceQueue();
    XMLVM_CHECK_NPE(0)
    java_lang_ref_ReferenceQueue___INIT___(_r0.o);
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.referenceQueue_ = _r0.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 210)
    XMLVM_EXIT_METHOD()
    return;
    label32:;
    _r0 = _r2;
    goto label11;
    label34:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 208)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap___INIT____int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap___INIT____int_float]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.f = n2;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 224)
    XMLVM_CHECK_NPE(1)
    java_util_AbstractMap___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 225)
    if (_r2.i < 0) goto label42;
    _r0.f = 0.0;
    _r0.i = _r3.f > _r0.f ? 1 : (_r3.f == _r0.f ? 0 : -1);
    if (_r0.i <= 0) goto label42;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 226)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 227)
    if (_r2.i != 0) goto label40;
    _r0.i = 1;
    label16:;
    _r0.o = java_util_WeakHashMap_newEntryArray___int(_r0.i);
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.elementData_ = _r0.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 228)
    _r0.f = 10000.0;
    _r0.f = _r0.f * _r3.f;
    _r0.i = (JAVA_INT) _r0.f;
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.loadFactor_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 229)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_computeMaxSize__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 230)
    _r0.o = __NEW_java_lang_ref_ReferenceQueue();
    XMLVM_CHECK_NPE(0)
    java_lang_ref_ReferenceQueue___INIT___(_r0.o);
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.referenceQueue_ = _r0.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 234)
    XMLVM_EXIT_METHOD()
    return;
    label40:;
    _r0 = _r2;
    goto label16;
    label42:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 232)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap___INIT____java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap___INIT____java_util_Map]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 244)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r3.o);
    _r1.i = 6;
    if (_r0.i >= _r1.i) goto label16;
    _r0.i = 11;
    label9:;
    XMLVM_CHECK_NPE(2)
    java_util_WeakHashMap___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 245)
    XMLVM_CHECK_NPE(2)
    java_util_WeakHashMap_putAllImpl___java_util_Map(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 246)
    XMLVM_EXIT_METHOD()
    return;
    label16:;
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_size__])(_r3.o);
    _r0.i = _r0.i * 2;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_clear__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 256)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.elementCount_;
    if (_r0.i <= 0) goto label27;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 257)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 258)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 259)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.modCount_ = _r0.i;
    label19:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 260)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_WeakHashMap*) _r2.o)->fields.java_util_WeakHashMap.referenceQueue_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_ref_ReferenceQueue_poll__(_r0.o);
    if (_r0.o != JAVA_NULL) goto label19;
    label27:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 264)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_computeMaxSize__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_computeMaxSize__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "computeMaxSize", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 267)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.l = (JAVA_LONG) _r0.i;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.loadFactor_;
    _r2.l = (JAVA_LONG) _r2.i;
    _r0.l = _r0.l * _r2.l;
    _r2.l = 10000;
    _r0.l = _r0.l / _r2.l;
    _r0.i = (JAVA_INT) _r0.l;
    XMLVM_CHECK_NPE(4)
    ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.threshold_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 268)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_WeakHashMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_containsKey___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "containsKey", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 280)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_WeakHashMap_getEntry___java_lang_Object(_r1.o, _r2.o);
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

JAVA_OBJECT java_util_WeakHashMap_entrySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_entrySet__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "entrySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 293)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_poll__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 294)

    
    // Red class access removed: java.util.WeakHashMap$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.WeakHashMap$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_keySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_keySet__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "keySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 351)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_poll__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 352)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_;
    if (_r0.o != JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 353)

    
    // Red class access removed: java.util.WeakHashMap$2::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.WeakHashMap$2::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_ = _r0.o;
    label14:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 408)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.keySet_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_values__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_values__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "values", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 432)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_poll__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 433)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_;
    if (_r0.o != JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 434)

    
    // Red class access removed: java.util.WeakHashMap$3::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.WeakHashMap$3::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_ = _r0.o;
    label14:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 460)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_AbstractMap*) _r1.o)->fields.java_util_AbstractMap.valuesCollection_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_get___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 473)
    XMLVM_CHECK_NPE(3)
    java_util_WeakHashMap_poll__(_r3.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 474)
    if (_r4.o == JAVA_NULL) goto label42;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 475)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[4])(_r4.o);
    _r1.i = 2147483647;
    _r0.i = _r0.i & _r1.i;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i % _r1.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 476)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label22:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 477)
    if (_r0.o != JAVA_NULL) goto label26;
    _r0 = _r2;
    label25:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 483)
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 492)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label26:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 478)
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_ref_Reference_get__(_r0.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i == 0) goto label39;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 479)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.value_;
    goto label25;
    label39:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 481)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label22;
    label42:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 485)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = 0;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    label47:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 486)
    if (_r0.o != JAVA_NULL) goto label51;
    _r0 = _r2;
    goto label25;
    label51:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 487)
    XMLVM_CHECK_NPE(0)
    _r1.i = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r1.i == 0) goto label58;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 488)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.value_;
    goto label25;
    label58:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 490)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label47;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_getEntry___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_getEntry___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "getEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 496)
    XMLVM_CHECK_NPE(3)
    java_util_WeakHashMap_poll__(_r3.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 497)
    if (_r4.o == JAVA_NULL) goto label39;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 498)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[4])(_r4.o);
    _r1.i = 2147483647;
    _r0.i = _r0.i & _r1.i;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i % _r1.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 499)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label22:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 500)
    if (_r0.o != JAVA_NULL) goto label26;
    _r0 = _r2;
    label25:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 506)
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 515)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label26:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 501)
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_ref_Reference_get__(_r0.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i != 0) goto label25;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 504)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label22;
    label39:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 508)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_WeakHashMap*) _r3.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = 0;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    label44:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 509)
    if (_r0.o != JAVA_NULL) goto label48;
    _r0 = _r2;
    goto label25;
    label48:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 510)
    XMLVM_CHECK_NPE(0)
    _r1.i = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r1.i != 0) goto label25;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 513)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label44;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_WeakHashMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_containsValue___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "containsValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 528)
    XMLVM_CHECK_NPE(4)
    java_util_WeakHashMap_poll__(_r4.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 529)
    if (_r5.o == JAVA_NULL) goto label44;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 530)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    label9:;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label15;
    label13:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 553)
    _r0.i = 0;
    label14:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 531)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label19:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 532)
    if (_r1.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 533)
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_ref_Reference_get__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 534)
    if (_r2.o != JAVA_NULL) goto label31;
    XMLVM_CHECK_NPE(1)
    _r2.i = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r2.i == 0) goto label41;
    label31:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 535)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.value_;
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r2.i == 0) goto label41;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 536)
    goto label14;
    label41:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 538)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label19;
    label44:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 542)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    label47:;
    _r0.i = _r0.i + -1;
    if (_r0.i < 0) goto label13;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 543)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_WeakHashMap*) _r4.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label55:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 544)
    if (_r1.o == JAVA_NULL) goto label47;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 545)
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_ref_Reference_get__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 546)
    if (_r2.o != JAVA_NULL) goto label67;
    XMLVM_CHECK_NPE(1)
    _r2.i = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r2.i == 0) goto label73;
    label67:;
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.value_;
    if (_r2.o != JAVA_NULL) goto label73;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 547)
    goto label14;
    label73:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 549)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label55;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_WeakHashMap_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 563)
    //java_util_WeakHashMap_size__[16]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_WeakHashMap*) _r1.o)->tib->vtable[16])(_r1.o);
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

void java_util_WeakHashMap_poll__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_poll__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "poll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    label0:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 569)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.referenceQueue_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_ref_ReferenceQueue_poll__(_r0.o);
    _r0.o = _r0.o;
    if (_r0.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 572)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 570)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_removeEntry___java_util_WeakHashMap_Entry(_r1.o, _r0.o);
    goto label0;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_removeEntry___java_util_WeakHashMap_Entry(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_removeEntry___java_util_WeakHashMap_Entry]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "removeEntry", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 575)
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 576)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_WeakHashMap_Entry*) _r6.o)->fields.java_util_WeakHashMap_Entry.hash_;
    _r2.i = 2147483647;
    _r1.i = _r1.i & _r2.i;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.elementData_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = _r1.i % _r2.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 577)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    label15:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 580)
    if (_r2.o != JAVA_NULL) goto label18;
    label17:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 594)
    XMLVM_EXIT_METHOD()
    return;
    label18:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 581)
    if (_r6.o != _r2.o) goto label46;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 582)
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.modCount_;
    _r3.i = _r3.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.modCount_ = _r3.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 583)
    if (_r0.o != JAVA_NULL) goto label41;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 584)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_util_WeakHashMap_Entry*) _r2.o)->fields.java_util_WeakHashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    label34:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 588)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.elementCount_;
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(5)
    ((java_util_WeakHashMap*) _r5.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    goto label17;
    label41:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 586)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r2.o)->fields.java_util_WeakHashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_ = _r1.o;
    goto label34;
    label46:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 592)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r2.o)->fields.java_util_WeakHashMap_Entry.next_;
    _r4 = _r2;
    _r2 = _r0;
    _r0 = _r4;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_put___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_put___java_lang_Object_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = me;
    _r7.o = n1;
    _r8.o = n2;
    _r4.i = 2147483647;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 608)
    XMLVM_CHECK_NPE(6)
    java_util_WeakHashMap_poll__(_r6.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 611)
    if (_r7.o == JAVA_NULL) goto label83;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 612)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[4])(_r7.o);
    _r0.i = _r0.i & _r4.i;
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i % _r1.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 613)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    label22:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 614)
    if (_r1.o == JAVA_NULL) goto label116;
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_ref_Reference_get__(_r1.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[1])(_r7.o, _r2.o);
    if (_r2.i == 0) goto label80;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    label37:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 623)
    if (_r0.o != JAVA_NULL) goto label108;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 624)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 625)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.threshold_;
    if (_r0.i <= _r2.i) goto label114;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 626)
    XMLVM_CHECK_NPE(6)
    java_util_WeakHashMap_rehash__(_r6.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 627)
    if (_r7.o != JAVA_NULL) goto label98;
    _r0 = _r3;
    label61:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 630)
    _r1.o = __NEW_java_util_WeakHashMap_Entry();
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.referenceQueue_;
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_Entry___INIT____java_lang_Object_java_lang_Object_java_lang_ref_ReferenceQueue(_r1.o, _r7.o, _r8.o, _r2.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 631)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(1)
    ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_ = _r2.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 632)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 633)
    _r0.o = JAVA_NULL;
    label79:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 637)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label80:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 615)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label22;
    label83:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 618)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    label87:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 619)
    if (_r0.o == JAVA_NULL) goto label93;
    XMLVM_CHECK_NPE(0)
    _r1.i = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r1.i == 0) goto label95;
    label93:;
    _r1 = _r3;
    goto label37;
    label95:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 620)
    XMLVM_CHECK_NPE(0)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_;
    goto label87;
    label98:;
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[4])(_r7.o);
    _r0.i = _r0.i & _r4.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 628)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i % _r1.i;
    goto label61;
    label108:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 635)
    XMLVM_CHECK_NPE(0)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.value_;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 636)
    XMLVM_CHECK_NPE(0)
    ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.value_ = _r8.o;
    _r0 = _r1;
    goto label79;
    label114:;
    _r0 = _r1;
    goto label61;
    label116:;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    goto label37;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_rehash__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_rehash__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "rehash", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r8.o = me;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 641)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_WeakHashMap*) _r8.o)->fields.java_util_WeakHashMap.elementData_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r0.i << 1;
    if (_r0.i != 0) goto label9;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 642)
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 643)
    _r0.i = 1;
    label9:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 645)
    _r1.o = java_util_WeakHashMap_newEntryArray___int(_r0.i);
    _r2 = _r7;
    label14:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 646)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_WeakHashMap*) _r8.o)->fields.java_util_WeakHashMap.elementData_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r2.i < _r3.i) goto label25;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 657)
    XMLVM_CHECK_NPE(8)
    ((java_util_WeakHashMap*) _r8.o)->fields.java_util_WeakHashMap.elementData_ = _r1.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 658)
    XMLVM_CHECK_NPE(8)
    java_util_WeakHashMap_computeMaxSize__(_r8.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 659)
    XMLVM_EXIT_METHOD()
    return;
    label25:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 647)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_WeakHashMap*) _r8.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    label29:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 648)
    if (_r3.o != JAVA_NULL) goto label34;
    _r2.i = _r2.i + 1;
    goto label14;
    label34:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 649)
    XMLVM_CHECK_NPE(3)
    _r4.i = ((java_util_WeakHashMap_Entry*) _r3.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r4.i == 0) goto label49;
    _r4 = _r7;
    label39:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 651)
    XMLVM_CHECK_NPE(3)
    _r5.o = ((java_util_WeakHashMap_Entry*) _r3.o)->fields.java_util_WeakHashMap_Entry.next_;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 652)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_CHECK_NPE(3)
    ((java_util_WeakHashMap_Entry*) _r3.o)->fields.java_util_WeakHashMap_Entry.next_ = _r6.o;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 653)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r3.o;
    _r3 = _r5;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 654)
    goto label29;
    label49:;
    XMLVM_CHECK_NPE(3)
    _r4.i = ((java_util_WeakHashMap_Entry*) _r3.o)->fields.java_util_WeakHashMap_Entry.hash_;
    _r5.i = 2147483647;
    _r4.i = _r4.i & _r5.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 650)
    _r4.i = _r4.i % _r0.i;
    goto label39;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_putAll___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_putAll___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "putAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 673)
    XMLVM_CHECK_NPE(0)
    java_util_WeakHashMap_putAllImpl___java_util_Map(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 674)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_WeakHashMap_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "remove", "?")
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
    _r4.o = JAVA_NULL;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 686)
    XMLVM_CHECK_NPE(6)
    java_util_WeakHashMap_poll__(_r6.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 689)
    if (_r7.o == JAVA_NULL) goto label72;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 690)
    //java_lang_Object_hashCode__[4]
    XMLVM_CHECK_NPE(7)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[4])(_r7.o);
    _r1.i = 2147483647;
    _r0.i = _r0.i & _r1.i;
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i % _r1.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 691)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r2 = _r1;
    _r1 = _r4;
    label25:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 692)
    if (_r2.o == JAVA_NULL) goto label99;
    XMLVM_CHECK_NPE(2)
    _r3.o = java_lang_ref_Reference_get__(_r2.o);
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r7.o)->tib->vtable[1])(_r7.o, _r3.o);
    if (_r3.i == 0) goto label66;
    _r5 = _r1;
    _r1 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    label41:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 703)
    if (_r1.o == JAVA_NULL) goto label97;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 704)
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.modCount_;
    _r3.i = _r3.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.modCount_ = _r3.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 705)
    if (_r0.o != JAVA_NULL) goto label92;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 706)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(1)
    _r3.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.o;
    label57:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 710)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementCount_;
    _r2.i = 1;
    _r0.i = _r0.i - _r2.i;
    XMLVM_CHECK_NPE(6)
    ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 711)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.value_;
    label65:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 713)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label66:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 694)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_WeakHashMap_Entry*) _r2.o)->fields.java_util_WeakHashMap_Entry.next_;
    _r5 = _r2;
    _r2 = _r1;
    _r1 = _r5;
    goto label25;
    label72:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 697)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_WeakHashMap*) _r6.o)->fields.java_util_WeakHashMap.elementData_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r1 = _r0;
    _r0 = _r4;
    label78:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 698)
    if (_r1.o == JAVA_NULL) goto label84;
    XMLVM_CHECK_NPE(1)
    _r2.i = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.isNull_;
    if (_r2.i == 0) goto label86;
    label84:;
    _r2 = _r3;
    goto label41;
    label86:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 700)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    goto label78;
    label92:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 708)
    XMLVM_CHECK_NPE(1)
    _r2.o = ((java_util_WeakHashMap_Entry*) _r1.o)->fields.java_util_WeakHashMap_Entry.next_;
    XMLVM_CHECK_NPE(0)
    ((java_util_WeakHashMap_Entry*) _r0.o)->fields.java_util_WeakHashMap_Entry.next_ = _r2.o;
    goto label57;
    label97:;
    _r0 = _r4;
    goto label65;
    label99:;
    _r5 = _r1;
    _r1 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    goto label41;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_WeakHashMap_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_size__]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 723)
    XMLVM_CHECK_NPE(1)
    java_util_WeakHashMap_poll__(_r1.o);
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 724)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_WeakHashMap*) _r1.o)->fields.java_util_WeakHashMap.elementCount_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_util_WeakHashMap_putAllImpl___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_WeakHashMap_putAllImpl___java_util_Map]
    XMLVM_ENTER_METHOD("java.util.WeakHashMap", "putAllImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 728)
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_entrySet__])(_r2.o);
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 729)
    XMLVM_CHECK_NPE(1)
    java_util_AbstractMap_putAll___java_util_Map(_r1.o, _r2.o);
    label9:;
    XMLVM_SOURCE_POSITION("WeakHashMap.java", 731)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

