#include "xmlvm.h"
#include "java_lang_Thread.h"
#include "java_util_ArrayList.h"
#include "java_util_List.h"
#include "org_xmlvm_runtime_Mutex.h"

#include "java_lang_Object_AddedMembers.h"

#define XMLVM_CURRENT_CLASS_NAME Object_AddedMembers
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Object_AddedMembers

__TIB_DEFINITION_java_lang_Object_AddedMembers __TIB_java_lang_Object_AddedMembers = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Object_AddedMembers, // classInitializer
    "java.lang.Object$AddedMembers", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Object_AddedMembers), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers;
JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_3ARRAY;
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

void __INIT_java_lang_Object_AddedMembers()
{
    staticInitializerLock(&__TIB_java_lang_Object_AddedMembers);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Object_AddedMembers.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Object_AddedMembers.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Object_AddedMembers);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Object_AddedMembers.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Object_AddedMembers.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Object_AddedMembers.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Object$AddedMembers")
        __INIT_IMPL_java_lang_Object_AddedMembers();
    }
}

void __INIT_IMPL_java_lang_Object_AddedMembers()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Object_AddedMembers.newInstanceFunc = __NEW_INSTANCE_java_lang_Object_AddedMembers;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Object_AddedMembers.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_Object_AddedMembers.numImplementedInterfaces = 0;
    __TIB_java_lang_Object_AddedMembers.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_Object_AddedMembers.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Object_AddedMembers.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Object_AddedMembers.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Object_AddedMembers.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Object_AddedMembers.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Object_AddedMembers.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Object_AddedMembers.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Object_AddedMembers.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Object_AddedMembers = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Object_AddedMembers);
    __TIB_java_lang_Object_AddedMembers.clazz = __CLASS_java_lang_Object_AddedMembers;
    __TIB_java_lang_Object_AddedMembers.baseType = JAVA_NULL;
    __CLASS_java_lang_Object_AddedMembers_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Object_AddedMembers);
    __CLASS_java_lang_Object_AddedMembers_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Object_AddedMembers_1ARRAY);
    __CLASS_java_lang_Object_AddedMembers_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Object_AddedMembers_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Object_AddedMembers]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Object_AddedMembers.classInitialized = 1;
}

void __DELETE_java_lang_Object_AddedMembers(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Object_AddedMembers]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Object_AddedMembers(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Object_AddedMembers*) me)->fields.java_lang_Object_AddedMembers.recursiveLocks_ = 0;
    ((java_lang_Object_AddedMembers*) me)->fields.java_lang_Object_AddedMembers.owningThread_ = (java_lang_Thread*) JAVA_NULL;
    ((java_lang_Object_AddedMembers*) me)->fields.java_lang_Object_AddedMembers.instanceMutex_ = (org_xmlvm_runtime_Mutex*) JAVA_NULL;
    ((java_lang_Object_AddedMembers*) me)->fields.java_lang_Object_AddedMembers.waitingConditions_ = (java_util_List*) JAVA_NULL;
    ((java_lang_Object_AddedMembers*) me)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Object_AddedMembers]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Object_AddedMembers()
{    XMLVM_CLASS_INIT(java_lang_Object_AddedMembers)
java_lang_Object_AddedMembers* me = (java_lang_Object_AddedMembers*) XMLVM_MALLOC(sizeof(java_lang_Object_AddedMembers));
    me->tib = &__TIB_java_lang_Object_AddedMembers;
    __INIT_INSTANCE_MEMBERS_java_lang_Object_AddedMembers(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Object_AddedMembers]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Object_AddedMembers()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_Object_AddedMembers();
    java_lang_Object_AddedMembers___INIT___(me);
    return me;
}

void java_lang_Object_AddedMembers___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Object_AddedMembers___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Object$AddedMembers", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Object.java", 369)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("Object.java", 356)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.recursiveLocks_ = _r1.i;
    XMLVM_SOURCE_POSITION("Object.java", 365)
    _r0.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(0)
    java_util_ArrayList___INIT____int(_r0.o, _r1.i);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.waitingConditions_ = _r0.o;
    XMLVM_SOURCE_POSITION("Object.java", 367)
    _r0.i = -1;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_ = _r0.i;
    XMLVM_SOURCE_POSITION("Object.java", 370)
    _r0.o = __NEW_org_xmlvm_runtime_Mutex();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_Mutex___INIT___(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.instanceMutex_ = _r0.o;
    XMLVM_SOURCE_POSITION("Object.java", 371)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Object_AddedMembers_access$000___java_lang_Object_AddedMembers(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Object_AddedMembers)
    //XMLVM_BEGIN_WRAPPER[java_lang_Object_AddedMembers_access$000___java_lang_Object_AddedMembers]
    XMLVM_ENTER_METHOD("java.lang.Object$AddedMembers", "access$000", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Object.java", 354)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Object_AddedMembers*) _r1.o)->fields.java_lang_Object_AddedMembers.waitingConditions_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Object_AddedMembers_access$100___java_lang_Object_AddedMembers(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Object_AddedMembers)
    //XMLVM_BEGIN_WRAPPER[java_lang_Object_AddedMembers_access$100___java_lang_Object_AddedMembers]
    XMLVM_ENTER_METHOD("java.lang.Object$AddedMembers", "access$100", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Object.java", 354)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Object_AddedMembers*) _r1.o)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Object_AddedMembers_access$110___java_lang_Object_AddedMembers(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Object_AddedMembers)
    //XMLVM_BEGIN_WRAPPER[java_lang_Object_AddedMembers_access$110___java_lang_Object_AddedMembers]
    XMLVM_ENTER_METHOD("java.lang.Object$AddedMembers", "access$110", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Object.java", 354)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_;
    _r1.i = 1;
    _r1.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(2)
    ((java_lang_Object_AddedMembers*) _r2.o)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_ = _r1.i;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Object_AddedMembers_access$102___java_lang_Object_AddedMembers_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Object_AddedMembers)
    //XMLVM_BEGIN_WRAPPER[java_lang_Object_AddedMembers_access$102___java_lang_Object_AddedMembers_int]
    XMLVM_ENTER_METHOD("java.lang.Object$AddedMembers", "access$102", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = n1;
    _r1.i = n2;
    XMLVM_SOURCE_POSITION("Object.java", 354)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Object_AddedMembers*) _r0.o)->fields.java_lang_Object_AddedMembers.notifyAllMaxIndex_ = _r1.i;
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

