#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_util_Collection.h"
#include "java_util_Collections.h"
#include "java_util_List.h"
#include "java_util_Map.h"
#include "java_util_Set.h"

#include "java_util_Collections_EmptyMap.h"

#define XMLVM_CURRENT_CLASS_NAME Collections_EmptyMap
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Collections_EmptyMap

__TIB_DEFINITION_java_util_Collections_EmptyMap __TIB_java_util_Collections_EmptyMap = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Collections_EmptyMap, // classInitializer
    "java.util.Collections$EmptyMap", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractMap, // extends
    sizeof(java_util_Collections_EmptyMap), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Collections_EmptyMap_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Collections_EmptyMap_serialVersionUID;

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

void __INIT_java_util_Collections_EmptyMap()
{
    staticInitializerLock(&__TIB_java_util_Collections_EmptyMap);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Collections_EmptyMap.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Collections_EmptyMap.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Collections_EmptyMap);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Collections_EmptyMap.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Collections_EmptyMap.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Collections_EmptyMap.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Collections$EmptyMap")
        __INIT_IMPL_java_util_Collections_EmptyMap();
    }
}

void __INIT_IMPL_java_util_Collections_EmptyMap()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractMap)
    __TIB_java_util_Collections_EmptyMap.newInstanceFunc = __NEW_INSTANCE_java_util_Collections_EmptyMap;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Collections_EmptyMap.vtable, __TIB_java_util_AbstractMap.vtable, sizeof(__TIB_java_util_AbstractMap.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Collections_EmptyMap.vtable[7] = (VTABLE_PTR) &java_util_Collections_EmptyMap_containsKey___java_lang_Object;
    __TIB_java_util_Collections_EmptyMap.vtable[8] = (VTABLE_PTR) &java_util_Collections_EmptyMap_containsValue___java_lang_Object;
    __TIB_java_util_Collections_EmptyMap.vtable[9] = (VTABLE_PTR) &java_util_Collections_EmptyMap_entrySet__;
    __TIB_java_util_Collections_EmptyMap.vtable[10] = (VTABLE_PTR) &java_util_Collections_EmptyMap_get___java_lang_Object;
    __TIB_java_util_Collections_EmptyMap.vtable[12] = (VTABLE_PTR) &java_util_Collections_EmptyMap_keySet__;
    __TIB_java_util_Collections_EmptyMap.vtable[17] = (VTABLE_PTR) &java_util_Collections_EmptyMap_values__;
    // Initialize interface information
    __TIB_java_util_Collections_EmptyMap.numImplementedInterfaces = 2;
    __TIB_java_util_Collections_EmptyMap.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Collections_EmptyMap.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_util_Map)

    __TIB_java_util_Collections_EmptyMap.implementedInterfaces[0][1] = &__TIB_java_util_Map;
    // Initialize itable for this class
    __TIB_java_util_Collections_EmptyMap.itableBegin = &__TIB_java_util_Collections_EmptyMap.itable[0];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_clear__] = __TIB_java_util_Collections_EmptyMap.vtable[6];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[7];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[8];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_entrySet__] = __TIB_java_util_Collections_EmptyMap.vtable[9];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[1];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[10];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_hashCode__] = __TIB_java_util_Collections_EmptyMap.vtable[4];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__] = __TIB_java_util_Collections_EmptyMap.vtable[11];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_keySet__] = __TIB_java_util_Collections_EmptyMap.vtable[12];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map] = __TIB_java_util_Collections_EmptyMap.vtable[13];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[14];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object] = __TIB_java_util_Collections_EmptyMap.vtable[15];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_size__] = __TIB_java_util_Collections_EmptyMap.vtable[16];
    __TIB_java_util_Collections_EmptyMap.itable[XMLVM_ITABLE_IDX_java_util_Map_values__] = __TIB_java_util_Collections_EmptyMap.vtable[17];

    _STATIC_java_util_Collections_EmptyMap_serialVersionUID = 6428348081105594320;

    __TIB_java_util_Collections_EmptyMap.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Collections_EmptyMap.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptyMap.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Collections_EmptyMap.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Collections_EmptyMap.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Collections_EmptyMap.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Collections_EmptyMap.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Collections_EmptyMap.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Collections_EmptyMap = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Collections_EmptyMap);
    __TIB_java_util_Collections_EmptyMap.clazz = __CLASS_java_util_Collections_EmptyMap;
    __TIB_java_util_Collections_EmptyMap.baseType = JAVA_NULL;
    __CLASS_java_util_Collections_EmptyMap_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyMap);
    __CLASS_java_util_Collections_EmptyMap_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyMap_1ARRAY);
    __CLASS_java_util_Collections_EmptyMap_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Collections_EmptyMap_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Collections_EmptyMap]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Collections_EmptyMap.classInitialized = 1;
}

void __DELETE_java_util_Collections_EmptyMap(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Collections_EmptyMap]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyMap(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractMap(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyMap]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Collections_EmptyMap()
{    XMLVM_CLASS_INIT(java_util_Collections_EmptyMap)
java_util_Collections_EmptyMap* me = (java_util_Collections_EmptyMap*) XMLVM_MALLOC(sizeof(java_util_Collections_EmptyMap));
    me->tib = &__TIB_java_util_Collections_EmptyMap;
    __INIT_INSTANCE_MEMBERS_java_util_Collections_EmptyMap(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Collections_EmptyMap]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Collections_EmptyMap()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_util_Collections_EmptyMap_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptyMap)
    return _STATIC_java_util_Collections_EmptyMap_serialVersionUID;
}

void java_util_Collections_EmptyMap_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Collections_EmptyMap)
_STATIC_java_util_Collections_EmptyMap_serialVersionUID = v;
}

void java_util_Collections_EmptyMap___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap___INIT___]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 136)
    XMLVM_CHECK_NPE(0)
    java_util_AbstractMap___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_EmptyMap_containsKey___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_containsKey___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "containsKey", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 142)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Collections_EmptyMap_containsValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_containsValue___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "containsValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 147)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyMap_entrySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_entrySet__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "entrySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 152)
    _r0.o = java_util_Collections_GET_EMPTY_SET();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyMap_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_get___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 157)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyMap_keySet__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_keySet__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "keySet", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 162)
    _r0.o = java_util_Collections_GET_EMPTY_SET();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyMap_values__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_values__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "values", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 167)
    _r0.o = java_util_Collections_GET_EMPTY_LIST();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Collections_EmptyMap_readResolve__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap_readResolve__]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "readResolve", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Collections.java", 171)
    _r0.o = java_util_Collections_GET_EMPTY_MAP();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Collections_EmptyMap___INIT____java_util_Collections_EmptyMap(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Collections_EmptyMap___INIT____java_util_Collections_EmptyMap]
    XMLVM_ENTER_METHOD("java.util.Collections$EmptyMap", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Collections.java", 136)
    XMLVM_CHECK_NPE(0)
    java_util_Collections_EmptyMap___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

