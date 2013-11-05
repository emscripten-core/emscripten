#include "xmlvm.h"
#include "java_lang_ref_ReferenceQueue.h"

#include "java_lang_ref_Reference.h"

#define XMLVM_CURRENT_CLASS_NAME Reference
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ref_Reference

__TIB_DEFINITION_java_lang_ref_Reference __TIB_java_lang_ref_Reference = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ref_Reference, // classInitializer
    "java.lang.ref.Reference", // className
    "java.lang.ref", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_ref_Reference), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ref_Reference;
JAVA_OBJECT __CLASS_java_lang_ref_Reference_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_Reference_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ref_Reference_3ARRAY;
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

void __INIT_java_lang_ref_Reference()
{
    staticInitializerLock(&__TIB_java_lang_ref_Reference);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ref_Reference.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ref_Reference.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ref_Reference);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ref_Reference.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ref_Reference.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ref_Reference.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ref.Reference")
        __INIT_IMPL_java_lang_ref_Reference();
    }
}

void __INIT_IMPL_java_lang_ref_Reference()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_ref_Reference.newInstanceFunc = __NEW_INSTANCE_java_lang_ref_Reference;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ref_Reference.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_ref_Reference.numImplementedInterfaces = 0;
    __TIB_java_lang_ref_Reference.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_ref_Reference.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ref_Reference.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ref_Reference.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ref_Reference.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ref_Reference.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ref_Reference.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ref_Reference.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ref_Reference.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ref_Reference = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ref_Reference);
    __TIB_java_lang_ref_Reference.clazz = __CLASS_java_lang_ref_Reference;
    __TIB_java_lang_ref_Reference.baseType = JAVA_NULL;
    __CLASS_java_lang_ref_Reference_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_Reference);
    __CLASS_java_lang_ref_Reference_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_Reference_1ARRAY);
    __CLASS_java_lang_ref_Reference_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ref_Reference_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ref_Reference]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ref_Reference.classInitialized = 1;
}

void __DELETE_java_lang_ref_Reference(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ref_Reference]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ref_Reference(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ref_Reference*) me)->fields.java_lang_ref_Reference.obj_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ref_Reference]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ref_Reference()
{    XMLVM_CLASS_INIT(java_lang_ref_Reference)
java_lang_ref_Reference* me = (java_lang_ref_Reference*) XMLVM_MALLOC(sizeof(java_lang_ref_Reference));
    me->tib = &__TIB_java_lang_ref_Reference;
    __INIT_INSTANCE_MEMBERS_java_lang_ref_Reference(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ref_Reference]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_Reference()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_ref_Reference();
    java_lang_ref_Reference___INIT___(me);
    return me;
}

void java_lang_ref_Reference___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference___INIT___]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 48)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Reference.java", 49)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ref_Reference_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_clear__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "clear", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 56)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ref_Reference_enqueue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_enqueue__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "enqueue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 67)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ref_Reference_get__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_get__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 77)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ref_Reference*) _r1.o)->fields.java_lang_ref_Reference.obj_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ref_Reference_isEnqueued__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_isEnqueued__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "isEnqueued", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 87)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_ref_Reference_initReference___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_initReference___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "initReference", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Reference.java", 98)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ref_Reference*) _r0.o)->fields.java_lang_ref_Reference.obj_ = _r1.o;
    XMLVM_SOURCE_POSITION("Reference.java", 99)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ref_Reference_initReference___java_lang_Object_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_initReference___java_lang_Object_java_lang_ref_ReferenceQueue]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "initReference", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Reference.java", 110)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ref_Reference*) _r0.o)->fields.java_lang_ref_Reference.obj_ = _r1.o;
    XMLVM_SOURCE_POSITION("Reference.java", 111)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ref_Reference_enqueueImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_enqueueImpl__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "enqueueImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 120)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_ref_Reference_dequeue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ref_Reference_dequeue__]
    XMLVM_ENTER_METHOD("java.lang.ref.Reference", "dequeue", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Reference.java", 128)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

