#include "xmlvm.h"

#include "org_xmlvm_runtime_RedTypeMarker.h"

#define XMLVM_CURRENT_CLASS_NAME RedTypeMarker
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_RedTypeMarker

__TIB_DEFINITION_org_xmlvm_runtime_RedTypeMarker __TIB_org_xmlvm_runtime_RedTypeMarker = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_RedTypeMarker, // classInitializer
    "org.xmlvm.runtime.RedTypeMarker", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_xmlvm_runtime_RedTypeMarker), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_3ARRAY;
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

void __INIT_org_xmlvm_runtime_RedTypeMarker()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_RedTypeMarker);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_RedTypeMarker.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_RedTypeMarker.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_RedTypeMarker);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_RedTypeMarker.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_RedTypeMarker.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_RedTypeMarker.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.RedTypeMarker")
        __INIT_IMPL_org_xmlvm_runtime_RedTypeMarker();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_RedTypeMarker()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_xmlvm_runtime_RedTypeMarker.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_RedTypeMarker;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_RedTypeMarker.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_org_xmlvm_runtime_RedTypeMarker.numImplementedInterfaces = 0;
    __TIB_org_xmlvm_runtime_RedTypeMarker.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_org_xmlvm_runtime_RedTypeMarker.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_RedTypeMarker.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_RedTypeMarker.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_RedTypeMarker.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_RedTypeMarker.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_RedTypeMarker.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_RedTypeMarker.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_RedTypeMarker.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_RedTypeMarker = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_RedTypeMarker);
    __TIB_org_xmlvm_runtime_RedTypeMarker.clazz = __CLASS_org_xmlvm_runtime_RedTypeMarker;
    __TIB_org_xmlvm_runtime_RedTypeMarker.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_RedTypeMarker_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_RedTypeMarker);
    __CLASS_org_xmlvm_runtime_RedTypeMarker_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_RedTypeMarker_1ARRAY);
    __CLASS_org_xmlvm_runtime_RedTypeMarker_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_RedTypeMarker_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_RedTypeMarker]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_RedTypeMarker.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_RedTypeMarker(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_RedTypeMarker]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_RedTypeMarker(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_RedTypeMarker]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_RedTypeMarker()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_RedTypeMarker)
org_xmlvm_runtime_RedTypeMarker* me = (org_xmlvm_runtime_RedTypeMarker*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_RedTypeMarker));
    me->tib = &__TIB_org_xmlvm_runtime_RedTypeMarker;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_RedTypeMarker(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_RedTypeMarker]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_RedTypeMarker()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void org_xmlvm_runtime_RedTypeMarker___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_RedTypeMarker___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.RedTypeMarker", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("RedTypeMarker.java", 28)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("RedTypeMarker.java", 29)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

