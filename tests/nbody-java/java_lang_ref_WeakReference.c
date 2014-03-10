#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_ref_ReferenceQueue.h"

#include "java_lang_ref_WeakReference.h"

#define XMLVM_CURRENT_CLASS_NAME WeakReference
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ref_WeakReference

__TIB_DEFINITION_java_lang_ref_WeakReference __TIB_java_lang_ref_WeakReference = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ref_WeakReference, // classInitializer
    "java.lang.ref.WeakReference", // className
    "java.lang.ref", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/ref/Reference<TT;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_ref_Reference, // extends
    sizeof(java_lang_ref_WeakReference), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ref_WeakReference;
JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_3ARRAY;
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

void __INIT_java_lang_ref_WeakReference()
{
    staticInitializerLock(&__TIB_java_lang_ref_WeakReference);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ref_WeakReference.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ref_WeakReference.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ref_WeakReference);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ref_WeakReference.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ref_WeakReference.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ref_WeakReference.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ref.WeakReference")
        __INIT_IMPL_java_lang_ref_WeakReference();
    }
}

void __INIT_IMPL_java_lang_ref_WeakReference()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_ref_Reference)
    __TIB_java_lang_ref_WeakReference.newInstanceFunc = __NEW_INSTANCE_java_lang_ref_WeakReference;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ref_WeakReference.vtable, __TIB_java_lang_ref_Reference.vtable, sizeof(__TIB_java_lang_ref_Reference.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_ref_WeakReference.numImplementedInterfaces = 0;
    __TIB_java_lang_ref_WeakReference.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_ref_WeakReference.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ref_WeakReference.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ref_WeakReference.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ref_WeakReference.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ref_WeakReference.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ref_WeakReference.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ref_WeakReference.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ref_WeakReference.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ref_WeakReference = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ref_WeakReference);
    __TIB_java_lang_ref_WeakReference.clazz = __CLASS_java_lang_ref_WeakReference;
    __TIB_java_lang_ref_WeakReference.baseType = JAVA_NULL;
    __CLASS_java_lang_ref_WeakReference_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_WeakReference);
    __CLASS_java_lang_ref_WeakReference_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_WeakReference_1ARRAY);
    __CLASS_java_lang_ref_WeakReference_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_WeakReference_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ref_WeakReference]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ref_WeakReference.classInitialized = 1;
}

void __DELETE_java_lang_ref_WeakReference(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ref_WeakReference]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ref_WeakReference(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_ref_Reference(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ref_WeakReference]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ref_WeakReference()
{    XMLVM_CLASS_INIT(java_lang_ref_WeakReference)
java_lang_ref_WeakReference* me = (java_lang_ref_WeakReference*) XMLVM_MALLOC(sizeof(java_lang_ref_WeakReference));
    me->tib = &__TIB_java_lang_ref_WeakReference;
    __INIT_INSTANCE_MEMBERS_java_lang_ref_WeakReference(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ref_WeakReference]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_WeakReference()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_ref_WeakReference___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_WeakReference___INIT____java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.ref.WeakReference", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("WeakReference.java", 82)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("WeakReference.java", 83)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference_initReference___java_lang_Object(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("WeakReference.java", 84)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ref_WeakReference___INIT____java_lang_Object_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_WeakReference___INIT____java_lang_Object_java_lang_ref_ReferenceQueue]
    XMLVM_ENTER_METHOD("java.lang.ref.WeakReference", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("WeakReference.java", 96)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("WeakReference.java", 97)
    XMLVM_CHECK_NPE(0)
    java_lang_ref_Reference_initReference___java_lang_Object_java_lang_ref_ReferenceQueue(_r0.o, _r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("WeakReference.java", 98)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

