#include "xmlvm.h"
#include "java_lang_ThreadGroup.h"

#include "java_lang_ThreadGroup_ChildrenThreadsLock.h"

#define XMLVM_CURRENT_CLASS_NAME ThreadGroup_ChildrenThreadsLock
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ThreadGroup_ChildrenThreadsLock

__TIB_DEFINITION_java_lang_ThreadGroup_ChildrenThreadsLock __TIB_java_lang_ThreadGroup_ChildrenThreadsLock = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ThreadGroup_ChildrenThreadsLock, // classInitializer
    "java.lang.ThreadGroup$ChildrenThreadsLock", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_ThreadGroup_ChildrenThreadsLock), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_3ARRAY;
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

void __INIT_java_lang_ThreadGroup_ChildrenThreadsLock()
{
    staticInitializerLock(&__TIB_java_lang_ThreadGroup_ChildrenThreadsLock);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ThreadGroup_ChildrenThreadsLock);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ThreadGroup_ChildrenThreadsLock.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ThreadGroup_ChildrenThreadsLock.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ThreadGroup$ChildrenThreadsLock")
        __INIT_IMPL_java_lang_ThreadGroup_ChildrenThreadsLock();
    }
}

void __INIT_IMPL_java_lang_ThreadGroup_ChildrenThreadsLock()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.newInstanceFunc = __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenThreadsLock;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ThreadGroup_ChildrenThreadsLock.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.numImplementedInterfaces = 0;
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ThreadGroup_ChildrenThreadsLock);
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.clazz = __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock;
    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.baseType = JAVA_NULL;
    __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenThreadsLock);
    __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_1ARRAY);
    __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ThreadGroup_ChildrenThreadsLock]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ThreadGroup_ChildrenThreadsLock.classInitialized = 1;
}

void __DELETE_java_lang_ThreadGroup_ChildrenThreadsLock(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ThreadGroup_ChildrenThreadsLock]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenThreadsLock(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ThreadGroup_ChildrenThreadsLock*) me)->fields.java_lang_ThreadGroup_ChildrenThreadsLock.this_0_ = (java_lang_ThreadGroup*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenThreadsLock]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ThreadGroup_ChildrenThreadsLock()
{    XMLVM_CLASS_INIT(java_lang_ThreadGroup_ChildrenThreadsLock)
java_lang_ThreadGroup_ChildrenThreadsLock* me = (java_lang_ThreadGroup_ChildrenThreadsLock*) XMLVM_MALLOC(sizeof(java_lang_ThreadGroup_ChildrenThreadsLock));
    me->tib = &__TIB_java_lang_ThreadGroup_ChildrenThreadsLock;
    __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenThreadsLock(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ThreadGroup_ChildrenThreadsLock]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenThreadsLock()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup$ChildrenThreadsLock", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 71)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ThreadGroup_ChildrenThreadsLock*) _r0.o)->fields.java_lang_ThreadGroup_ChildrenThreadsLock.this_0_ = _r1.o;
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup$ChildrenThreadsLock", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 71)
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup(_r0.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

