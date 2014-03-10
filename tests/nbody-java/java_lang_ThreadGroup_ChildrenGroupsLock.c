#include "xmlvm.h"
#include "java_lang_ThreadGroup.h"

#include "java_lang_ThreadGroup_ChildrenGroupsLock.h"

#define XMLVM_CURRENT_CLASS_NAME ThreadGroup_ChildrenGroupsLock
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ThreadGroup_ChildrenGroupsLock

__TIB_DEFINITION_java_lang_ThreadGroup_ChildrenGroupsLock __TIB_java_lang_ThreadGroup_ChildrenGroupsLock = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ThreadGroup_ChildrenGroupsLock, // classInitializer
    "java.lang.ThreadGroup$ChildrenGroupsLock", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_ThreadGroup_ChildrenGroupsLock), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_3ARRAY;
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

void __INIT_java_lang_ThreadGroup_ChildrenGroupsLock()
{
    staticInitializerLock(&__TIB_java_lang_ThreadGroup_ChildrenGroupsLock);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ThreadGroup_ChildrenGroupsLock);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ThreadGroup_ChildrenGroupsLock.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ThreadGroup_ChildrenGroupsLock.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ThreadGroup$ChildrenGroupsLock")
        __INIT_IMPL_java_lang_ThreadGroup_ChildrenGroupsLock();
    }
}

void __INIT_IMPL_java_lang_ThreadGroup_ChildrenGroupsLock()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.newInstanceFunc = __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenGroupsLock;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ThreadGroup_ChildrenGroupsLock.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.numImplementedInterfaces = 0;
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ThreadGroup_ChildrenGroupsLock);
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.clazz = __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock;
    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.baseType = JAVA_NULL;
    __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenGroupsLock);
    __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_1ARRAY);
    __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ThreadGroup_ChildrenGroupsLock]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ThreadGroup_ChildrenGroupsLock.classInitialized = 1;
}

void __DELETE_java_lang_ThreadGroup_ChildrenGroupsLock(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ThreadGroup_ChildrenGroupsLock]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenGroupsLock(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ThreadGroup_ChildrenGroupsLock*) me)->fields.java_lang_ThreadGroup_ChildrenGroupsLock.this_0_ = (java_lang_ThreadGroup*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenGroupsLock]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ThreadGroup_ChildrenGroupsLock()
{    XMLVM_CLASS_INIT(java_lang_ThreadGroup_ChildrenGroupsLock)
java_lang_ThreadGroup_ChildrenGroupsLock* me = (java_lang_ThreadGroup_ChildrenGroupsLock*) XMLVM_MALLOC(sizeof(java_lang_ThreadGroup_ChildrenGroupsLock));
    me->tib = &__TIB_java_lang_ThreadGroup_ChildrenGroupsLock;
    __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenGroupsLock(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ThreadGroup_ChildrenGroupsLock]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenGroupsLock()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup$ChildrenGroupsLock", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 67)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ThreadGroup_ChildrenGroupsLock*) _r0.o)->fields.java_lang_ThreadGroup_ChildrenGroupsLock.this_0_ = _r1.o;
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup$ChildrenGroupsLock", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 67)
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup(_r0.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

