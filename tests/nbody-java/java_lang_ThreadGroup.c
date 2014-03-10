#include "xmlvm.h"
#include "java_io_PrintStream.h"
#include "java_lang_Class.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_SecurityManager.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Thread.h"
#include "java_lang_ThreadGroup_ChildrenGroupsLock.h"
#include "java_lang_ThreadGroup_ChildrenThreadsLock.h"
#include "java_lang_Throwable.h"

#include "java_lang_ThreadGroup.h"

#define XMLVM_CURRENT_CLASS_NAME ThreadGroup
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_ThreadGroup

__TIB_DEFINITION_java_lang_ThreadGroup __TIB_java_lang_ThreadGroup = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_ThreadGroup, // classInitializer
    "java.lang.ThreadGroup", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_ThreadGroup), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_ThreadGroup;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_ThreadGroup_3ARRAY;
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

void __INIT_java_lang_ThreadGroup()
{
    staticInitializerLock(&__TIB_java_lang_ThreadGroup);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_ThreadGroup.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_ThreadGroup.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_ThreadGroup);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_ThreadGroup.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_ThreadGroup.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_ThreadGroup.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.ThreadGroup")
        __INIT_IMPL_java_lang_ThreadGroup();
    }
}

void __INIT_IMPL_java_lang_ThreadGroup()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_ThreadGroup.newInstanceFunc = __NEW_INSTANCE_java_lang_ThreadGroup;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_ThreadGroup.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_ThreadGroup.vtable[5] = (VTABLE_PTR) &java_lang_ThreadGroup_toString__;
    __TIB_java_lang_ThreadGroup.vtable[6] = (VTABLE_PTR) &java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable;
    // Initialize interface information
    __TIB_java_lang_ThreadGroup.numImplementedInterfaces = 1;
    __TIB_java_lang_ThreadGroup.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Thread_UncaughtExceptionHandler)

    __TIB_java_lang_ThreadGroup.implementedInterfaces[0][0] = &__TIB_java_lang_Thread_UncaughtExceptionHandler;
    // Initialize itable for this class
    __TIB_java_lang_ThreadGroup.itableBegin = &__TIB_java_lang_ThreadGroup.itable[0];
    __TIB_java_lang_ThreadGroup.itable[XMLVM_ITABLE_IDX_java_lang_Thread_UncaughtExceptionHandler_uncaughtException___java_lang_Thread_java_lang_Throwable] = __TIB_java_lang_ThreadGroup.vtable[6];


    __TIB_java_lang_ThreadGroup.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_ThreadGroup.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_ThreadGroup.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_ThreadGroup.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_ThreadGroup.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_ThreadGroup.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_ThreadGroup.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_ThreadGroup = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_ThreadGroup);
    __TIB_java_lang_ThreadGroup.clazz = __CLASS_java_lang_ThreadGroup;
    __TIB_java_lang_ThreadGroup.baseType = JAVA_NULL;
    __CLASS_java_lang_ThreadGroup_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup);
    __CLASS_java_lang_ThreadGroup_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_1ARRAY);
    __CLASS_java_lang_ThreadGroup_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_ThreadGroup_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_ThreadGroup]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_ThreadGroup.classInitialized = 1;
}

void __DELETE_java_lang_ThreadGroup(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_ThreadGroup]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.name_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.maxPriority_ = 0;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.parent_ = (java_lang_ThreadGroup*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.numThreads_ = 0;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.childrenThreads_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.numGroups_ = 0;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.childrenGroups_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.childrenGroupsLock_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.childrenThreadsLock_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.isDaemon_ = 0;
    ((java_lang_ThreadGroup*) me)->fields.java_lang_ThreadGroup.isDestroyed_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_ThreadGroup()
{    XMLVM_CLASS_INIT(java_lang_ThreadGroup)
java_lang_ThreadGroup* me = (java_lang_ThreadGroup*) XMLVM_MALLOC(sizeof(java_lang_ThreadGroup));
    me->tib = &__TIB_java_lang_ThreadGroup;
    __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_ThreadGroup]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_ThreadGroup___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup___INIT___]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 84)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 50)
    _r0.i = 10;
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.maxPriority_ = _r0.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 58)
    _r0.i = 5;
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Thread, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreads_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 64)
    _r0.i = 3;
    XMLVM_CLASS_INIT(java_lang_ThreadGroup)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_ThreadGroup, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroups_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 68)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenGroupsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 72)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenThreadsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 85)
    // "system"
    _r0.o = xmlvm_create_java_string_from_pool(662);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.name_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 86)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 102)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Thread_getThreadGroup__(_r0.o);
    XMLVM_CHECK_NPE(1)
    java_lang_ThreadGroup___INIT____java_lang_ThreadGroup_java_lang_String(_r1.o, _r0.o, _r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 103)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup___INIT____java_lang_ThreadGroup_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup___INIT____java_lang_ThreadGroup_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 120)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 50)
    _r0.i = 10;
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.maxPriority_ = _r0.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 58)
    _r0.i = 5;
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Thread, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreads_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 64)
    _r0.i = 3;
    XMLVM_CLASS_INIT(java_lang_ThreadGroup)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_ThreadGroup, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroups_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 68)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenGroupsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 72)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenThreadsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 121)
    _r0.o = java_lang_Thread_currentThread__();
    if (_r0.o == JAVA_NULL) goto label49;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 124)
    if (_r3.o != JAVA_NULL) goto label46;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 130)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label46:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 132)
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_checkAccess__(_r3.o);
    label49:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 136)
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.name_ = _r4.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 137)
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_setParent___java_lang_ThreadGroup(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 138)
    if (_r3.o == JAVA_NULL) goto label73;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 139)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_ThreadGroup_getMaxPriority__(_r3.o);
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_setMaxPriority___int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 140)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_ThreadGroup_isDaemon__(_r3.o);
    if (_r0.i == 0) goto label73;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 141)
    _r0.i = 1;
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_setDaemon___boolean(_r2.o, _r0.i);
    label73:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 144)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup___INIT____java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    _r1.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 149)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 50)
    _r0.i = 10;
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.maxPriority_ = _r0.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 58)
    _r0.i = 5;
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Thread, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreads_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 64)
    _r0.i = 3;
    XMLVM_CLASS_INIT(java_lang_ThreadGroup)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_ThreadGroup, _r0.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroups_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 68)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenGroupsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 72)
    _r0.o = __NEW_java_lang_ThreadGroup_ChildrenThreadsLock();
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(_r0.o, _r2.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 150)
    // "main"
    _r0.o = xmlvm_create_java_string_from_pool(663);
    XMLVM_CHECK_NPE(2)
    ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.name_ = _r0.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 151)
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_setParent___java_lang_ThreadGroup(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 152)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_activeCount__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_activeCount__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "activeCount", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 161)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.numThreads_;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 163)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 164)
    _r1.i = 0;
    label6:;
    XMLVM_TRY_BEGIN(w3933aaac15b1c10)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r1.i >= _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac15b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac15b1c10, sizeof(XMLVM_JMP_BUF)); goto label22; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 165)
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(3)
    _r3.i = java_lang_ThreadGroup_activeCount__(_r3.o);
    _r0.i = _r0.i + _r3.i;
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac15b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac15b1c10, sizeof(XMLVM_JMP_BUF)); goto label6; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 167)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac15b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac15b1c10,java_lang_Object,24)
    XMLVM_CATCH_END(w3933aaac15b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac15b1c10)
    label22:;
    XMLVM_TRY_BEGIN(w3933aaac15b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 168)
    XMLVM_MEMCPY(curThread_w3933aaac15b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac15b1c12, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac15b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac15b1c12,java_lang_Object,24)
    XMLVM_CATCH_END(w3933aaac15b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac15b1c12)
    label24:;
    XMLVM_TRY_BEGIN(w3933aaac15b1c14)
    // Begin try
    java_lang_Thread* curThread_w3933aaac15b1c14aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac15b1c14aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac15b1c14)
        XMLVM_CATCH_SPECIFIC(w3933aaac15b1c14,java_lang_Object,24)
    XMLVM_CATCH_END(w3933aaac15b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac15b1c14)
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_activeGroupCount__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_activeGroupCount__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "activeGroupCount", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 178)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 180)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 181)
    _r1.i = 0;
    label5:;
    XMLVM_TRY_BEGIN(w3933aaac16b1c10)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r1.i >= _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac16b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac16b1c10, sizeof(XMLVM_JMP_BUF)); goto label23; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 183)
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(3)
    _r3.i = java_lang_ThreadGroup_activeGroupCount__(_r3.o);
    _r3.i = _r3.i + 1;
    _r0.i = _r0.i + _r3.i;
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac16b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac16b1c10, sizeof(XMLVM_JMP_BUF)); goto label5; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 185)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac16b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac16b1c10,java_lang_Object,25)
    XMLVM_CATCH_END(w3933aaac16b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac16b1c10)
    label23:;
    XMLVM_TRY_BEGIN(w3933aaac16b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 186)
    XMLVM_MEMCPY(curThread_w3933aaac16b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac16b1c12, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r0.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac16b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac16b1c12,java_lang_Object,25)
    XMLVM_CATCH_END(w3933aaac16b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac16b1c12)
    label25:;
    XMLVM_TRY_BEGIN(w3933aaac16b1c14)
    // Begin try
    java_lang_Thread* curThread_w3933aaac16b1c14aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac16b1c14aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac16b1c14)
        XMLVM_CATCH_SPECIFIC(w3933aaac16b1c14,java_lang_Object,25)
    XMLVM_CATCH_END(w3933aaac16b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac16b1c14)
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_add___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_add___java_lang_Thread]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "add", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 202)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w3933aaac17b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 203)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.isDestroyed_;
    if (_r2.i != 0) { XMLVM_MEMCPY(curThread_w3933aaac17b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac17b1b6, sizeof(XMLVM_JMP_BUF)); goto label56; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 204)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r2.i != _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac17b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac17b1b6, sizeof(XMLVM_JMP_BUF)); goto label42; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 205)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r2.i = _r2.i * 2;
    XMLVM_CLASS_INIT(java_lang_Thread)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Thread, _r2.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 206)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    _r3.i = 0;
    _r4.i = 0;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r0.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 207)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_ = _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r7.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 208)
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_ = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac17b1b6)
        XMLVM_CATCH_SPECIFIC(w3933aaac17b1b6,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac17b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac17b1b6)
    label40:;
    XMLVM_TRY_BEGIN(w3933aaac17b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 215)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 216)
    XMLVM_MEMCPY(curThread_w3933aaac17b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac17b1b8, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac17b1b8)
        XMLVM_CATCH_SPECIFIC(w3933aaac17b1b8,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac17b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac17b1b8)
    label42:;
    XMLVM_TRY_BEGIN(w3933aaac17b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 210)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    _r4.i = _r3.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_ = _r4.i;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r7.o;
    { XMLVM_MEMCPY(curThread_w3933aaac17b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac17b1c10, sizeof(XMLVM_JMP_BUF)); goto label40; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac17b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac17b1c10,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac17b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac17b1c10)
    label53:;
    XMLVM_TRY_BEGIN(w3933aaac17b1c12)
    // Begin try
    java_lang_Thread* curThread_w3933aaac17b1c12aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac17b1c12aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac17b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac17b1c12,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac17b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac17b1c12)
    XMLVM_THROW_CUSTOM(_r2.o)
    label56:;
    XMLVM_TRY_BEGIN(w3933aaac17b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 213)

    
    // Red class access removed: java.lang.IllegalThreadStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalThreadStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r2.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac17b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac17b1c15,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac17b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac17b1c15)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_add___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_add___java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "add", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 227)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w3933aaac18b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 228)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.isDestroyed_;
    if (_r2.i != 0) { XMLVM_MEMCPY(curThread_w3933aaac18b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac18b1b6, sizeof(XMLVM_JMP_BUF)); goto label56; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 229)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r2.i != _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac18b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac18b1b6, sizeof(XMLVM_JMP_BUF)); goto label42; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 230)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r2.i = _r2.i * 2;
    XMLVM_CLASS_INIT(java_lang_ThreadGroup)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_ThreadGroup, _r2.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 231)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r3.i = 0;
    _r4.i = 0;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r0.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 232)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_ = _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r7.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 233)
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_ = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac18b1b6)
        XMLVM_CATCH_SPECIFIC(w3933aaac18b1b6,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac18b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac18b1b6)
    label40:;
    XMLVM_TRY_BEGIN(w3933aaac18b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 240)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 241)
    XMLVM_MEMCPY(curThread_w3933aaac18b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac18b1b8, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac18b1b8)
        XMLVM_CATCH_SPECIFIC(w3933aaac18b1b8,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac18b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac18b1b8)
    label42:;
    XMLVM_TRY_BEGIN(w3933aaac18b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 235)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r4.i = _r3.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_ = _r4.i;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r7.o;
    { XMLVM_MEMCPY(curThread_w3933aaac18b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac18b1c10, sizeof(XMLVM_JMP_BUF)); goto label40; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac18b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac18b1c10,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac18b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac18b1c10)
    label53:;
    XMLVM_TRY_BEGIN(w3933aaac18b1c12)
    // Begin try
    java_lang_Thread* curThread_w3933aaac18b1c12aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac18b1c12aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac18b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac18b1c12,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac18b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac18b1c12)
    XMLVM_THROW_CUSTOM(_r2.o)
    label56:;
    XMLVM_TRY_BEGIN(w3933aaac18b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 238)

    
    // Red class access removed: java.lang.IllegalThreadStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalThreadStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r2.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac18b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac18b1c15,java_lang_Object,53)
    XMLVM_CATCH_END(w3933aaac18b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac18b1c15)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_allowThreadSuspension___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_allowThreadSuspension___boolean]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "allowThreadSuspension", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 256)
    _r0.i = 1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_checkAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_checkAccess__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "checkAccess", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 268)
    _r0.o = java_lang_System_getSecurityManager__();
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 269)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 270)
    XMLVM_CHECK_NPE(0)
    java_lang_SecurityManager_checkAccess___java_lang_ThreadGroup(_r0.o, _r1.o);
    label9:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 272)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_destroy__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_destroy__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "destroy", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 288)
    XMLVM_CHECK_NPE(6)
    java_lang_ThreadGroup_checkAccess__(_r6.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 291)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w3933aaac21b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 292)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1b7)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1b7,java_lang_Object,43)
    XMLVM_CATCH_END(w3933aaac21b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1b7)
    XMLVM_TRY_BEGIN(w3933aaac21b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 293)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r0.i = 0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1b8)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1b8,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac21b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1b8)
    label12:;
    XMLVM_TRY_BEGIN(w3933aaac21b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 295)
    if (_r0.i >= _r1.i) { XMLVM_MEMCPY(curThread_w3933aaac21b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac21b1c10, sizeof(XMLVM_JMP_BUF)); goto label25; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 299)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r5.i = 0;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    XMLVM_CHECK_NPE(4)
    java_lang_ThreadGroup_destroy__(_r4.o);
    _r0.i = _r0.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac21b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac21b1c10, sizeof(XMLVM_JMP_BUF)); goto label12; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 302)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c10,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac21b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c10)
    label25:;
    XMLVM_TRY_BEGIN(w3933aaac21b1c12)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.parent_;
    if (_r4.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w3933aaac21b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac21b1c12, sizeof(XMLVM_JMP_BUF)); goto label34; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 303)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.parent_;
    XMLVM_CHECK_NPE(4)
    java_lang_ThreadGroup_remove___java_lang_ThreadGroup(_r4.o, _r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c12,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac21b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c12)
    label34:;
    XMLVM_TRY_BEGIN(w3933aaac21b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 308)
    _r4.i = 1;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.isDestroyed_ = _r4.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 309)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c14)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c14,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac21b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c14)
    XMLVM_TRY_BEGIN(w3933aaac21b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 310)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c15,java_lang_Object,43)
    XMLVM_CATCH_END(w3933aaac21b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c15)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 311)
    XMLVM_EXIT_METHOD()
    return;
    label40:;
    java_lang_Thread* curThread_w3933aaac21b1c19 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac21b1c19->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac21b1c20)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c20)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c20,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac21b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c20)
    XMLVM_TRY_BEGIN(w3933aaac21b1c21)
    // Begin try
    XMLVM_THROW_CUSTOM(_r4.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c21)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c21,java_lang_Object,43)
    XMLVM_CATCH_END(w3933aaac21b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c21)
    label43:;
    XMLVM_TRY_BEGIN(w3933aaac21b1c23)
    // Begin try
    java_lang_Thread* curThread_w3933aaac21b1c23aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac21b1c23aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac21b1c23)
        XMLVM_CATCH_SPECIFIC(w3933aaac21b1c23,java_lang_Object,43)
    XMLVM_CATCH_END(w3933aaac21b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac21b1c23)
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_destroyIfEmptyDaemon__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_destroyIfEmptyDaemon__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "destroyIfEmptyDaemon", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 323)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w3933aaac22b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 324)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.isDaemon_;
    if (_r1.i == 0) { XMLVM_MEMCPY(curThread_w3933aaac22b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac22b1b5, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.isDestroyed_;
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w3933aaac22b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac22b1b5, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w3933aaac22b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac22b1b5, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 325)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1b5)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1b5,java_lang_Object,31)
    XMLVM_CATCH_END(w3933aaac22b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1b5)
    XMLVM_TRY_BEGIN(w3933aaac22b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 326)
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r2.i != 0) { XMLVM_MEMCPY(curThread_w3933aaac22b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac22b1b6, sizeof(XMLVM_JMP_BUF)); goto label25; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 327)
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_destroy__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1b6)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1b6,java_lang_Object,28)
    XMLVM_CATCH_END(w3933aaac22b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1b6)
    label25:;
    XMLVM_TRY_BEGIN(w3933aaac22b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 329)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1b8)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1b8,java_lang_Object,28)
    XMLVM_CATCH_END(w3933aaac22b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1b8)
    label26:;
    XMLVM_TRY_BEGIN(w3933aaac22b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 331)
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1c10,java_lang_Object,31)
    XMLVM_CATCH_END(w3933aaac22b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1c10)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 332)
    XMLVM_EXIT_METHOD()
    return;
    label28:;
    java_lang_Thread* curThread_w3933aaac22b1c14 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac22b1c14->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac22b1c15)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1c15,java_lang_Object,28)
    XMLVM_CATCH_END(w3933aaac22b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1c15)
    XMLVM_TRY_BEGIN(w3933aaac22b1c16)
    // Begin try
    XMLVM_THROW_CUSTOM(_r2.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1c16)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1c16,java_lang_Object,31)
    XMLVM_CATCH_END(w3933aaac22b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1c16)
    label31:;
    XMLVM_TRY_BEGIN(w3933aaac22b1c18)
    // Begin try
    java_lang_Thread* curThread_w3933aaac22b1c18aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w3933aaac22b1c18aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac22b1c18)
        XMLVM_CATCH_SPECIFIC(w3933aaac22b1c18,java_lang_Object,31)
    XMLVM_CATCH_END(w3933aaac22b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac22b1c18)
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "enumerate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 343)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY_boolean(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY_boolean]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "enumerate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 357)
    _r0.i = 0;
    _r1.i = 1;
    XMLVM_CHECK_NPE(2)
    _r0.i = java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean(_r2.o, _r3.o, _r4.i, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "enumerate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 370)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY_boolean(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY_boolean]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "enumerate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.i = n2;
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 385)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean(_r1.o, _r2.o, _r3.i, _r0.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_INT n3, JAVA_BOOLEAN n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "enumerateGeneric", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    _r6.o = me;
    _r7.o = n1;
    _r8.i = n2;
    _r9.i = n3;
    _r10.i = n4;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 407)
    XMLVM_CHECK_NPE(6)
    java_lang_ThreadGroup_checkAccess__(_r6.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 409)
    if (_r10.i == 0) goto label44;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    _r4.o = _r4.o;
    _r2 = _r4;
    label10:;
    if (_r10.i == 0) goto label50;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 411)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    _r3 = _r4;
    label15:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 413)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 414)
    if (_r10.i == 0) goto label54;
    XMLVM_TRY_BEGIN(w3933aaac27b1c23)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c23)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c23,java_lang_Object,84)
    XMLVM_CATCH_END(w3933aaac27b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c23)
    _r1 = _r4;
    label21:;
    _r0 = _r9;
    label22:;
    _r1.i = _r1.i + -1;
    if (_r1.i < 0) goto label66;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 415)
    if (_r10.i == 0) goto label38;
    XMLVM_TRY_BEGIN(w3933aaac27b1c32)
    // Begin try
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r4.o = _r4.o;
    XMLVM_CHECK_NPE(4)
    _r4.i = java_lang_Thread_isAlive__(_r4.o);
    if (_r4.i == 0) { XMLVM_MEMCPY(curThread_w3933aaac27b1c32->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c32, sizeof(XMLVM_JMP_BUF)); goto label22; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c32)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c32,java_lang_Object,104)
    XMLVM_CATCH_END(w3933aaac27b1c32)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c32)
    label38:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c34)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 416)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    if (_r0.i < _r4.i) { XMLVM_MEMCPY(curThread_w3933aaac27b1c34->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c34, sizeof(XMLVM_JMP_BUF)); goto label58; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 417)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c34)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c34,java_lang_Object,104)
    XMLVM_CATCH_END(w3933aaac27b1c34)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c34)
    _r9 = _r0;
    label43:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 435)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label44:;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r4.o = _r4.o;
    _r2 = _r4;
    goto label10;
    label50:;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    _r3 = _r4;
    goto label15;
    label54:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c49)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r1 = _r4;
    { XMLVM_MEMCPY(curThread_w3933aaac27b1c49->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c49, sizeof(XMLVM_JMP_BUF)); goto label21; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 419)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c49)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c49,java_lang_Object,84)
    XMLVM_CATCH_END(w3933aaac27b1c49)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c49)
    label58:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c51)
    // Begin try
    _r9.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r4.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c51)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c51,java_lang_Object,84)
    XMLVM_CATCH_END(w3933aaac27b1c51)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c51)
    _r0 = _r9;
    goto label22;
    label66:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c55)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 422)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c55)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c55,java_lang_Object,104)
    XMLVM_CATCH_END(w3933aaac27b1c55)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c55)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 424)
    if (_r8.i == 0) goto label107;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 425)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r4.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 426)
    _r1.i = 0;
    _r9 = _r0;
    label74:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c65)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r1.i >= _r5.i) { XMLVM_MEMCPY(curThread_w3933aaac27b1c65->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c65, sizeof(XMLVM_JMP_BUF)); goto label98; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 427)
    _r5.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    if (_r9.i < _r5.i) { XMLVM_MEMCPY(curThread_w3933aaac27b1c65->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c65, sizeof(XMLVM_JMP_BUF)); goto label87; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 428)
    java_lang_Object_releaseLockRecursive__(_r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c65)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c65,java_lang_Object,101)
    XMLVM_CATCH_END(w3933aaac27b1c65)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c65)
    _r0 = _r9;
    goto label43;
    label84:;
    java_lang_Thread* curThread_w3933aaac27b1c69 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac27b1c69->fields.java_lang_Thread.xmlvmException_;
    label85:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c71)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c71)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c71,java_lang_Object,84)
    XMLVM_CATCH_END(w3933aaac27b1c71)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c71)
    XMLVM_THROW_CUSTOM(_r4.o)
    label87:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c74)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 430)
    XMLVM_CHECK_NPE(6)
    _r5.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r1.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(5)
    _r9.i = java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean(_r5.o, _r7.o, _r8.i, _r9.i, _r10.i);
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac27b1c74->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c74, sizeof(XMLVM_JMP_BUF)); goto label74; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 433)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c74)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c74,java_lang_Object,101)
    XMLVM_CATCH_END(w3933aaac27b1c74)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c74)
    label98:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c76)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c76)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c76,java_lang_Object,101)
    XMLVM_CATCH_END(w3933aaac27b1c76)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c76)
    label99:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c78)
    // Begin try
    _r0 = _r9;
    { XMLVM_MEMCPY(curThread_w3933aaac27b1c78->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac27b1c78, sizeof(XMLVM_JMP_BUF)); goto label43; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c78)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c78,java_lang_Object,101)
    XMLVM_CATCH_END(w3933aaac27b1c78)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c78)
    label101:;
    XMLVM_TRY_BEGIN(w3933aaac27b1c80)
    // Begin try
    java_lang_Thread* curThread_w3933aaac27b1c80aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r5.o = curThread_w3933aaac27b1c80aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac27b1c80)
        XMLVM_CATCH_SPECIFIC(w3933aaac27b1c80,java_lang_Object,101)
    XMLVM_CATCH_END(w3933aaac27b1c80)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac27b1c80)
    XMLVM_THROW_CUSTOM(_r5.o)
    label104:;
    java_lang_Thread* curThread_w3933aaac27b1c83 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac27b1c83->fields.java_lang_Thread.xmlvmException_;
    _r9 = _r0;
    goto label85;
    label107:;
    _r9 = _r0;
    goto label99;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_ThreadGroup_getMaxPriority__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_getMaxPriority__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "getMaxPriority", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 446)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.maxPriority_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ThreadGroup_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_getName__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "getName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 455)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.name_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ThreadGroup_getParent__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_getParent__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "getParent", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 466)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.parent_;
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 467)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.parent_;
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_checkAccess__(_r0.o);
    label9:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 469)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.parent_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_interrupt__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_interrupt__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "interrupt", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 482)
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_checkAccess__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 484)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 485)
    _r0.i = 0;
    label7:;
    XMLVM_TRY_BEGIN(w3933aaac31b1c10)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac31b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac31b1c10, sizeof(XMLVM_JMP_BUF)); goto label21; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 486)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_interrupt__(_r2.o);
    _r0.i = _r0.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac31b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac31b1c10, sizeof(XMLVM_JMP_BUF)); goto label7; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 488)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c10,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac31b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c10)
    label21:;
    XMLVM_TRY_BEGIN(w3933aaac31b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c12,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac31b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c12)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 490)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 491)
    _r0.i = 0;
    label26:;
    XMLVM_TRY_BEGIN(w3933aaac31b1c19)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac31b1c19->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac31b1c19, sizeof(XMLVM_JMP_BUF)); goto label43; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 492)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_interrupt__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c19)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c19,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac31b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c19)
    _r0.i = _r0.i + 1;
    goto label26;
    label40:;
    java_lang_Thread* curThread_w3933aaac31b1c23 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac31b1c23->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac31b1c24)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c24)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c24,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac31b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c24)
    XMLVM_THROW_CUSTOM(_r2.o)
    label43:;
    XMLVM_TRY_BEGIN(w3933aaac31b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 494)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 495)
    XMLVM_MEMCPY(curThread_w3933aaac31b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac31b1c27, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c27)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c27,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac31b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c27)
    label45:;
    XMLVM_TRY_BEGIN(w3933aaac31b1c29)
    // Begin try
    java_lang_Thread* curThread_w3933aaac31b1c29aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac31b1c29aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac31b1c29)
        XMLVM_CATCH_SPECIFIC(w3933aaac31b1c29,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac31b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac31b1c29)
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_isDaemon__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_isDaemon__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "isDaemon", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 506)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.isDaemon_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_isDestroyed__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_isDestroyed__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "isDestroyed", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 517)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.isDestroyed_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_list__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_list__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "list", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 528)
    _r0.o = java_lang_System_GET_out();
    XMLVM_CHECK_NPE(0)
    java_io_PrintStream_println__(_r0.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 529)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    java_lang_ThreadGroup_list___int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 530)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_list___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_list___int]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "list", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r5.o = me;
    _r6.i = n1;
    // "    "
    _r4.o = xmlvm_create_java_string_from_pool(664);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 541)
    _r0.i = 0;
    label3:;
    if (_r0.i >= _r6.i) goto label15;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 542)
    _r2.o = java_lang_System_GET_out();
    // "    "
    _r3.o = xmlvm_create_java_string_from_pool(664);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r4.o);
    _r0.i = _r0.i + 1;
    goto label3;
    label15:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 546)
    _r2.o = java_lang_System_GET_out();
    //java_lang_ThreadGroup_toString__[5]
    XMLVM_CHECK_NPE(5)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_ThreadGroup*) _r5.o)->tib->vtable[5])(_r5.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 549)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 550)
    _r0.i = 0;
    label28:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c25)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r0.i >= _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac35b1c25->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c25, sizeof(XMLVM_JMP_BUF)); goto label57; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 552)
    _r1.i = 0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c25)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c25,java_lang_Object,78)
    XMLVM_CATCH_END(w3933aaac35b1c25)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c25)
    label33:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c27)
    // Begin try
    if (_r1.i > _r6.i) { XMLVM_MEMCPY(curThread_w3933aaac35b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c27, sizeof(XMLVM_JMP_BUF)); goto label45; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 553)
    _r3.o = java_lang_System_GET_out();
    // "    "
    _r4.o = xmlvm_create_java_string_from_pool(664);
    XMLVM_CHECK_NPE(3)
    java_io_PrintStream_print___java_lang_String(_r3.o, _r4.o);
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac35b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c27, sizeof(XMLVM_JMP_BUF)); goto label33; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 555)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c27)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c27,java_lang_Object,78)
    XMLVM_CATCH_END(w3933aaac35b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c27)
    label45:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c29)
    // Begin try
    _r3.o = java_lang_System_GET_out();
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(3)
    java_io_PrintStream_println___java_lang_Object(_r3.o, _r4.o);
    _r0.i = _r0.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac35b1c29->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c29, sizeof(XMLVM_JMP_BUF)); goto label28; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 557)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c29)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c29,java_lang_Object,78)
    XMLVM_CATCH_END(w3933aaac35b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c29)
    label57:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c31)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c31)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c31,java_lang_Object,78)
    XMLVM_CATCH_END(w3933aaac35b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c31)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 558)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 559)
    _r0.i = 0;
    label62:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c38)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r0.i >= _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac35b1c38->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c38, sizeof(XMLVM_JMP_BUF)); goto label81; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 560)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r4.i = _r6.i + 1;
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_list___int(_r3.o, _r4.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c38)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c38,java_lang_Object,83)
    XMLVM_CATCH_END(w3933aaac35b1c38)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c38)
    _r0.i = _r0.i + 1;
    goto label62;
    label78:;
    java_lang_Thread* curThread_w3933aaac35b1c42 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac35b1c42->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac35b1c43)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c43)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c43,java_lang_Object,78)
    XMLVM_CATCH_END(w3933aaac35b1c43)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c43)
    XMLVM_THROW_CUSTOM(_r3.o)
    label81:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c46)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 562)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 563)
    XMLVM_MEMCPY(curThread_w3933aaac35b1c46->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac35b1c46, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c46)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c46,java_lang_Object,83)
    XMLVM_CATCH_END(w3933aaac35b1c46)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c46)
    label83:;
    XMLVM_TRY_BEGIN(w3933aaac35b1c48)
    // Begin try
    java_lang_Thread* curThread_w3933aaac35b1c48aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac35b1c48aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac35b1c48)
        XMLVM_CATCH_SPECIFIC(w3933aaac35b1c48,java_lang_Object,83)
    XMLVM_CATCH_END(w3933aaac35b1c48)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac35b1c48)
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_parentOf___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_parentOf___java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "parentOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    label0:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 575)
    if (_r2.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 576)
    if (_r1.o != _r2.o) goto label6;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 577)
    _r0.i = 1;
    label5:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 581)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 579)
    XMLVM_CHECK_NPE(2)
    _r2.o = ((java_lang_ThreadGroup*) _r2.o)->fields.java_lang_ThreadGroup.parent_;
    goto label0;
    label9:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_remove___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_remove___java_lang_Thread]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "remove", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 593)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 594)
    _r0.i = 0;
    label4:;
    XMLVM_TRY_BEGIN(w3933aaac37b1b9)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac37b1b9->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac37b1b9, sizeof(XMLVM_JMP_BUF)); goto label43; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 595)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_lang_Thread_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Thread*) _r2.o)->tib->vtable[1])(_r2.o, _r7.o);
    if (_r2.i == 0) { XMLVM_MEMCPY(curThread_w3933aaac37b1b9->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac37b1b9, sizeof(XMLVM_JMP_BUF)); goto label48; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 596)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    _r3.i = 1;
    _r2.i = _r2.i - _r3.i;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_ = _r2.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 597)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    _r3.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    _r5.i = _r5.i - _r0.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r4.o, _r0.i, _r5.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 600)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numThreads_;
    _r4.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac37b1b9)
        XMLVM_CATCH_SPECIFIC(w3933aaac37b1b9,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac37b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac37b1b9)
    label43:;
    XMLVM_TRY_BEGIN(w3933aaac37b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 604)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac37b1c11)
        XMLVM_CATCH_SPECIFIC(w3933aaac37b1c11,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac37b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac37b1c11)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 605)
    XMLVM_CHECK_NPE(6)
    java_lang_ThreadGroup_destroyIfEmptyDaemon__(_r6.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 606)
    XMLVM_EXIT_METHOD()
    return;
    label48:;
    _r0.i = _r0.i + 1;
    goto label4;
    label51:;
    java_lang_Thread* curThread_w3933aaac37b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac37b1c20->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac37b1c21)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac37b1c21)
        XMLVM_CATCH_SPECIFIC(w3933aaac37b1c21,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac37b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac37b1c21)
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_remove___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_remove___java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "remove", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 617)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 618)
    _r0.i = 0;
    label4:;
    XMLVM_TRY_BEGIN(w3933aaac38b1b9)
    // Begin try
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac38b1b9->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac38b1b9, sizeof(XMLVM_JMP_BUF)); goto label43; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 619)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[1])(_r2.o, _r7.o);
    if (_r2.i == 0) { XMLVM_MEMCPY(curThread_w3933aaac38b1b9->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac38b1b9, sizeof(XMLVM_JMP_BUF)); goto label48; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 620)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r3.i = 1;
    _r2.i = _r2.i - _r3.i;
    XMLVM_CHECK_NPE(6)
    ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_ = _r2.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 621)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    _r3.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r5.i = _r5.i - _r0.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r4.o, _r0.i, _r5.i);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 622)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_lang_ThreadGroup*) _r6.o)->fields.java_lang_ThreadGroup.numGroups_;
    _r4.o = JAVA_NULL;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac38b1b9)
        XMLVM_CATCH_SPECIFIC(w3933aaac38b1b9,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac38b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac38b1b9)
    label43:;
    XMLVM_TRY_BEGIN(w3933aaac38b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 626)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac38b1c11)
        XMLVM_CATCH_SPECIFIC(w3933aaac38b1c11,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac38b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac38b1c11)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 627)
    XMLVM_CHECK_NPE(6)
    java_lang_ThreadGroup_destroyIfEmptyDaemon__(_r6.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 628)
    XMLVM_EXIT_METHOD()
    return;
    label48:;
    _r0.i = _r0.i + 1;
    goto label4;
    label51:;
    java_lang_Thread* curThread_w3933aaac38b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac38b1c20->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac38b1c21)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac38b1c21)
        XMLVM_CATCH_SPECIFIC(w3933aaac38b1c21,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac38b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac38b1c21)
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_resume__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_resume__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "resume", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 645)
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_checkAccess__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 647)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 648)
    _r0.i = 0;
    label7:;
    XMLVM_TRY_BEGIN(w3933aaac39b1c10)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac39b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac39b1c10, sizeof(XMLVM_JMP_BUF)); goto label21; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 649)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(2)
    java_lang_Thread_resume__(_r2.o);
    _r0.i = _r0.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac39b1c10->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac39b1c10, sizeof(XMLVM_JMP_BUF)); goto label7; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 651)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c10)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c10,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac39b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c10)
    label21:;
    XMLVM_TRY_BEGIN(w3933aaac39b1c12)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c12)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c12,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac39b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c12)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 653)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 654)
    _r0.i = 0;
    label26:;
    XMLVM_TRY_BEGIN(w3933aaac39b1c19)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r0.i >= _r2.i) { XMLVM_MEMCPY(curThread_w3933aaac39b1c19->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac39b1c19, sizeof(XMLVM_JMP_BUF)); goto label43; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 655)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_ThreadGroup*) _r3.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(2)
    java_lang_ThreadGroup_resume__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c19)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c19,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac39b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c19)
    _r0.i = _r0.i + 1;
    goto label26;
    label40:;
    java_lang_Thread* curThread_w3933aaac39b1c23 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac39b1c23->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w3933aaac39b1c24)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c24)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c24,java_lang_Object,40)
    XMLVM_CATCH_END(w3933aaac39b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c24)
    XMLVM_THROW_CUSTOM(_r2.o)
    label43:;
    XMLVM_TRY_BEGIN(w3933aaac39b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 657)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 658)
    XMLVM_MEMCPY(curThread_w3933aaac39b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac39b1c27, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c27)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c27,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac39b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c27)
    label45:;
    XMLVM_TRY_BEGIN(w3933aaac39b1c29)
    // Begin try
    java_lang_Thread* curThread_w3933aaac39b1c29aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w3933aaac39b1c29aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac39b1c29)
        XMLVM_CATCH_SPECIFIC(w3933aaac39b1c29,java_lang_Object,45)
    XMLVM_CATCH_END(w3933aaac39b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac39b1c29)
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_setDaemon___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_setDaemon___boolean]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "setDaemon", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 674)
    XMLVM_CHECK_NPE(0)
    java_lang_ThreadGroup_checkAccess__(_r0.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 675)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ThreadGroup*) _r0.o)->fields.java_lang_ThreadGroup.isDaemon_ = _r1.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 676)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_setMaxPriority___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_setMaxPriority___int]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "setMaxPriority", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 696)
    XMLVM_CHECK_NPE(4)
    java_lang_ThreadGroup_checkAccess__(_r4.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 698)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.maxPriority_;
    if (_r5.i > _r2.i) goto label50;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 699)
    _r2.i = 1;
    if (_r5.i >= _r2.i) goto label11;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 700)
    _r5.i = 1;
    label11:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 703)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.parent_;
    if (_r2.o != JAVA_NULL) goto label39;
    _r1 = _r5;
    label16:;
    if (_r1.i > _r5.i) goto label47;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 704)
    _r2 = _r1;
    label19:;
    XMLVM_CHECK_NPE(4)
    ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.maxPriority_ = _r2.i;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 706)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 708)
    _r0.i = 0;
    label25:;
    XMLVM_TRY_BEGIN(w3933aaac41b1c30)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r0.i >= _r3.i) { XMLVM_MEMCPY(curThread_w3933aaac41b1c30->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac41b1c30, sizeof(XMLVM_JMP_BUF)); goto label49; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 709)
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(3)
    java_lang_ThreadGroup_setMaxPriority___int(_r3.o, _r5.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac41b1c30)
        XMLVM_CATCH_SPECIFIC(w3933aaac41b1c30,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac41b1c30)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac41b1c30)
    _r0.i = _r0.i + 1;
    goto label25;
    label39:;
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_ThreadGroup*) _r4.o)->fields.java_lang_ThreadGroup.parent_;
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_ThreadGroup_getMaxPriority__(_r2.o);
    _r1 = _r2;
    goto label16;
    label47:;
    _r2 = _r5;
    goto label19;
    label49:;
    XMLVM_TRY_BEGIN(w3933aaac41b1c42)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 711)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac41b1c42)
        XMLVM_CATCH_SPECIFIC(w3933aaac41b1c42,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac41b1c42)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac41b1c42)
    label50:;
    XMLVM_TRY_BEGIN(w3933aaac41b1c44)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 713)
    XMLVM_MEMCPY(curThread_w3933aaac41b1c44->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac41b1c44, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac41b1c44)
        XMLVM_CATCH_SPECIFIC(w3933aaac41b1c44,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac41b1c44)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac41b1c44)
    label51:;
    XMLVM_TRY_BEGIN(w3933aaac41b1c46)
    // Begin try
    java_lang_Thread* curThread_w3933aaac41b1c46aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3933aaac41b1c46aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac41b1c46)
        XMLVM_CATCH_SPECIFIC(w3933aaac41b1c46,java_lang_Object,51)
    XMLVM_CATCH_END(w3933aaac41b1c46)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac41b1c46)
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_setParent___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_setParent___java_lang_ThreadGroup]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "setParent", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 727)
    if (_r1.o == JAVA_NULL) goto label5;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 728)
    XMLVM_CHECK_NPE(1)
    java_lang_ThreadGroup_add___java_lang_ThreadGroup(_r1.o, _r0.o);
    label5:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 730)
    XMLVM_CHECK_NPE(0)
    ((java_lang_ThreadGroup*) _r0.o)->fields.java_lang_ThreadGroup.parent_ = _r1.o;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 731)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_stop__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_stop__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "stop", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 748)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_ThreadGroup_stopHelper__(_r1.o);
    if (_r0.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 749)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_stop__(_r0.o);
    label13:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 751)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_stopHelper__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_stopHelper__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "stopHelper", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 759)
    XMLVM_CHECK_NPE(5)
    java_lang_ThreadGroup_checkAccess__(_r5.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 761)
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 763)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w3933aaac44b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 764)
    _r0.o = java_lang_Thread_currentThread__();
    _r1.i = 0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1b9)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1b9,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1b9)
    label12:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 765)
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w3933aaac44b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c11, sizeof(XMLVM_JMP_BUF)); goto label37; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 766)
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r4.o != _r0.o) { XMLVM_MEMCPY(curThread_w3933aaac44b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c11, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 767)
    _r2.i = 1;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c11)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c11,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c11)
    label23:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c13)
    // Begin try
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac44b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c13, sizeof(XMLVM_JMP_BUF)); goto label12; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 769)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c13)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c13,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c13)
    label26:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c15)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(4)
    java_lang_Thread_stop__(_r4.o);
    { XMLVM_MEMCPY(curThread_w3933aaac44b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c15, sizeof(XMLVM_JMP_BUF)); goto label23; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 772)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c15,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c15)
    label34:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c17)
    // Begin try
    java_lang_Thread* curThread_w3933aaac44b1c17aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac44b1c17aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c17)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c17,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c17)
    XMLVM_THROW_CUSTOM(_r4.o)
    label37:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c20)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c20)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c20,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac44b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c20)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 774)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 775)
    _r1.i = 0;
    label42:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c27)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w3933aaac44b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c27, sizeof(XMLVM_JMP_BUF)); goto label58; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 776)
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(4)
    _r4.i = java_lang_ThreadGroup_stopHelper__(_r4.o);
    _r2.i = _r2.i | _r4.i;
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac44b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c27, sizeof(XMLVM_JMP_BUF)); goto label42; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 778)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c27)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c27,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac44b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c27)
    label58:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c29)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 779)
    XMLVM_MEMCPY(curThread_w3933aaac44b1c29->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac44b1c29, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r2.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c29)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c29,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac44b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c29)
    label60:;
    XMLVM_TRY_BEGIN(w3933aaac44b1c31)
    // Begin try
    java_lang_Thread* curThread_w3933aaac44b1c31aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac44b1c31aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac44b1c31)
        XMLVM_CATCH_SPECIFIC(w3933aaac44b1c31,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac44b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac44b1c31)
    XMLVM_THROW_CUSTOM(_r4.o)
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_suspend__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_suspend__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "suspend", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 797)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_ThreadGroup_suspendHelper__(_r1.o);
    if (_r0.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 798)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_suspend__(_r0.o);
    label13:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 800)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_ThreadGroup_suspendHelper__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_suspendHelper__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "suspendHelper", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 808)
    XMLVM_CHECK_NPE(5)
    java_lang_ThreadGroup_checkAccess__(_r5.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 810)
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 812)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreadsLock_;
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_TRY_BEGIN(w3933aaac46b1b9)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 813)
    _r0.o = java_lang_Thread_currentThread__();
    _r1.i = 0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1b9)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1b9,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1b9)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1b9)
    label12:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c11)
    // Begin try
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 814)
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numThreads_;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w3933aaac46b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c11, sizeof(XMLVM_JMP_BUF)); goto label37; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 815)
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r4.o != _r0.o) { XMLVM_MEMCPY(curThread_w3933aaac46b1c11->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c11, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 816)
    _r2.i = 1;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c11)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c11,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1c11)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c11)
    label23:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c13)
    // Begin try
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac46b1c13->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c13, sizeof(XMLVM_JMP_BUF)); goto label12; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 818)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c13)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c13,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c13)
    label26:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c15)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenThreads_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(4)
    java_lang_Thread_suspend__(_r4.o);
    { XMLVM_MEMCPY(curThread_w3933aaac46b1c15->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c15, sizeof(XMLVM_JMP_BUF)); goto label23; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 821)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c15)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c15,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c15)
    label34:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c17)
    // Begin try
    java_lang_Thread* curThread_w3933aaac46b1c17aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac46b1c17aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c17)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c17,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c17)
    XMLVM_THROW_CUSTOM(_r4.o)
    label37:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c20)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c20)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c20,java_lang_Object,34)
    XMLVM_CATCH_END(w3933aaac46b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c20)
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 823)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroupsLock_;
    java_lang_Object_acquireLockRecursive__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 824)
    _r1.i = 0;
    label42:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c27)
    // Begin try
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.numGroups_;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w3933aaac46b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c27, sizeof(XMLVM_JMP_BUF)); goto label58; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 825)
    XMLVM_CHECK_NPE(5)
    _r4.o = ((java_lang_ThreadGroup*) _r5.o)->fields.java_lang_ThreadGroup.childrenGroups_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(4)
    _r4.i = java_lang_ThreadGroup_suspendHelper__(_r4.o);
    _r2.i = _r2.i | _r4.i;
    _r1.i = _r1.i + 1;
    { XMLVM_MEMCPY(curThread_w3933aaac46b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c27, sizeof(XMLVM_JMP_BUF)); goto label42; };
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 827)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c27)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c27,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac46b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c27)
    label58:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c29)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 828)
    XMLVM_MEMCPY(curThread_w3933aaac46b1c29->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w3933aaac46b1c29, sizeof(XMLVM_JMP_BUF));
    XMLVM_EXIT_METHOD()
    return _r2.i;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c29)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c29,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac46b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c29)
    label60:;
    XMLVM_TRY_BEGIN(w3933aaac46b1c31)
    // Begin try
    java_lang_Thread* curThread_w3933aaac46b1c31aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w3933aaac46b1c31aa->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3933aaac46b1c31)
        XMLVM_CATCH_SPECIFIC(w3933aaac46b1c31,java_lang_Object,60)
    XMLVM_CATCH_END(w3933aaac46b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w3933aaac46b1c31)
    XMLVM_THROW_CUSTOM(_r4.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_ThreadGroup_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_toString__]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 839)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[3])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_Class_getName__(_r1.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "[name="
    _r1.o = xmlvm_create_java_string_from_pool(665);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_ThreadGroup_getName__(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // ",maxpri="
    _r1.o = xmlvm_create_java_string_from_pool(666);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    _r1.i = java_lang_ThreadGroup_getMaxPriority__(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    // "]"
    _r1.o = xmlvm_create_java_string_from_pool(183);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.lang.ThreadGroup", "uncaughtException", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 858)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.parent_;
    if (_r0.o == JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 859)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_ThreadGroup*) _r1.o)->fields.java_lang_ThreadGroup.parent_;
    //java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable[6]
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_ThreadGroup*) _r0.o)->tib->vtable[6])(_r0.o, _r2.o, _r3.o);
    label9:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 864)
    XMLVM_EXIT_METHOD()
    return;
    label10:;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 860)

    
    // Red class access removed: java.lang.ThreadDeath::instance-of
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r0.i != 0) goto label9;
    XMLVM_SOURCE_POSITION("ThreadGroup.java", 862)
    _r0.o = java_lang_System_GET_err();
    XMLVM_CHECK_NPE(3)
    java_lang_Throwable_printStackTrace___java_io_PrintStream(_r3.o, _r0.o);
    goto label9;
    //XMLVM_END_WRAPPER
}

