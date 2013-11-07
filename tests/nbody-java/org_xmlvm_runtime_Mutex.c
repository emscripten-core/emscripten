#include "xmlvm.h"
#include "java_util_LinkedList.h"
#include "java_util_Queue.h"

#include "org_xmlvm_runtime_Mutex.h"

#define XMLVM_CURRENT_CLASS_NAME Mutex
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_Mutex

__TIB_DEFINITION_org_xmlvm_runtime_Mutex __TIB_org_xmlvm_runtime_Mutex = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_Mutex, // classInitializer
    "org.xmlvm.runtime.Mutex", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_xmlvm_runtime_Mutex), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_org_xmlvm_runtime_Mutex_finalizableNativeMutexList;

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

void __INIT_org_xmlvm_runtime_Mutex()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_Mutex);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_Mutex.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_Mutex.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_Mutex);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_Mutex.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_Mutex.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_Mutex.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.Mutex")
        __INIT_IMPL_org_xmlvm_runtime_Mutex();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_Mutex()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_xmlvm_runtime_Mutex.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_Mutex;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_Mutex.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    xmlvm_init_native_org_xmlvm_runtime_Mutex();
    // Initialize interface information
    __TIB_org_xmlvm_runtime_Mutex.numImplementedInterfaces = 0;
    __TIB_org_xmlvm_runtime_Mutex.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_org_xmlvm_runtime_Mutex_finalizableNativeMutexList = (java_util_Queue*) JAVA_NULL;

    __TIB_org_xmlvm_runtime_Mutex.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_Mutex.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_Mutex.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_Mutex.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_Mutex.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_Mutex.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_Mutex.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_Mutex.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_Mutex = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_Mutex);
    __TIB_org_xmlvm_runtime_Mutex.clazz = __CLASS_org_xmlvm_runtime_Mutex;
    __TIB_org_xmlvm_runtime_Mutex.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_Mutex_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Mutex);
    __CLASS_org_xmlvm_runtime_Mutex_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Mutex_1ARRAY);
    __CLASS_org_xmlvm_runtime_Mutex_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_Mutex_2ARRAY);
    org_xmlvm_runtime_Mutex___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_Mutex]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_Mutex.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_Mutex(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_Mutex]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Mutex(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_xmlvm_runtime_Mutex*) me)->fields.org_xmlvm_runtime_Mutex.nativeMutex_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Mutex]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_Mutex()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_Mutex)
org_xmlvm_runtime_Mutex* me = (org_xmlvm_runtime_Mutex*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_Mutex));
    me->tib = &__TIB_org_xmlvm_runtime_Mutex;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Mutex(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_Mutex]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_Mutex()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_xmlvm_runtime_Mutex();
    org_xmlvm_runtime_Mutex___INIT___(me);
    return me;
}

JAVA_OBJECT org_xmlvm_runtime_Mutex_GET_finalizableNativeMutexList()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_Mutex)
    return _STATIC_org_xmlvm_runtime_Mutex_finalizableNativeMutexList;
}

void org_xmlvm_runtime_Mutex_PUT_finalizableNativeMutexList(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_Mutex)
_STATIC_org_xmlvm_runtime_Mutex_finalizableNativeMutexList = v;
}

void org_xmlvm_runtime_Mutex_addNativeMutexToFinalizerQueue___java_lang_Object(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_Mutex)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Mutex_addNativeMutexToFinalizerQueue___java_lang_Object]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Mutex", "addNativeMutexToFinalizerQueue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Mutex.java", 63)
    _r0.o = org_xmlvm_runtime_Mutex_GET_finalizableNativeMutexList();
    XMLVM_CHECK_NPE(0)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Queue_add___java_lang_Object])(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Mutex.java", 64)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_Mutex_destroyFinalizableNativeMutexes__()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_Mutex)
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Mutex_destroyFinalizableNativeMutexes__]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Mutex", "destroyFinalizableNativeMutexes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Mutex.java", 74)
    _r0.o = JAVA_NULL;
    label1:;
    XMLVM_SOURCE_POSITION("Mutex.java", 75)
    _r1.o = org_xmlvm_runtime_Mutex_GET_finalizableNativeMutexList();
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Queue_poll__])(_r1.o);
    if (_r0.o == JAVA_NULL) goto label13;
    XMLVM_SOURCE_POSITION("Mutex.java", 76)
    org_xmlvm_runtime_Mutex_destroyNativeMutex___java_lang_Object(_r0.o);
    goto label1;
    label13:;
    XMLVM_SOURCE_POSITION("Mutex.java", 78)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_Mutex_destroyNativeMutex___java_lang_Object(JAVA_OBJECT n1)]

void org_xmlvm_runtime_Mutex___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Mutex___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Mutex", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Mutex.java", 92)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Mutex.java", 93)
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex_initNativeInstance__(_r0.o);
    XMLVM_SOURCE_POSITION("Mutex.java", 94)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_Mutex_initNativeInstance__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void org_xmlvm_runtime_Mutex_lock__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void org_xmlvm_runtime_Mutex_unlock__(JAVA_OBJECT me)]

void org_xmlvm_runtime_Mutex___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_Mutex___CLINIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.Mutex", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Mutex.java", 48)
    _r0.o = __NEW_java_util_LinkedList();
    XMLVM_CHECK_NPE(0)
    java_util_LinkedList___INIT___(_r0.o);
    org_xmlvm_runtime_Mutex_PUT_finalizableNativeMutexList( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

