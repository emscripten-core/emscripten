#include "xmlvm.h"

#include "java_util_LinkedList_Link.h"

#define XMLVM_CURRENT_CLASS_NAME LinkedList_Link
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_LinkedList_Link

__TIB_DEFINITION_java_util_LinkedList_Link __TIB_java_util_LinkedList_Link = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_LinkedList_Link, // classInitializer
    "java.util.LinkedList$Link", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<ET:Ljava/lang/Object;>Ljava/lang/Object;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_LinkedList_Link), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_LinkedList_Link;
JAVA_OBJECT __CLASS_java_util_LinkedList_Link_1ARRAY;
JAVA_OBJECT __CLASS_java_util_LinkedList_Link_2ARRAY;
JAVA_OBJECT __CLASS_java_util_LinkedList_Link_3ARRAY;
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

void __INIT_java_util_LinkedList_Link()
{
    staticInitializerLock(&__TIB_java_util_LinkedList_Link);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_LinkedList_Link.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_LinkedList_Link.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_LinkedList_Link);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_LinkedList_Link.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_LinkedList_Link.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_LinkedList_Link.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.LinkedList$Link")
        __INIT_IMPL_java_util_LinkedList_Link();
    }
}

void __INIT_IMPL_java_util_LinkedList_Link()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_LinkedList_Link.newInstanceFunc = __NEW_INSTANCE_java_util_LinkedList_Link;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_LinkedList_Link.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_util_LinkedList_Link.numImplementedInterfaces = 0;
    __TIB_java_util_LinkedList_Link.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_util_LinkedList_Link.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_LinkedList_Link.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_LinkedList_Link.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_LinkedList_Link.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_LinkedList_Link.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_LinkedList_Link.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_LinkedList_Link.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_LinkedList_Link.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_LinkedList_Link = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_LinkedList_Link);
    __TIB_java_util_LinkedList_Link.clazz = __CLASS_java_util_LinkedList_Link;
    __TIB_java_util_LinkedList_Link.baseType = JAVA_NULL;
    __CLASS_java_util_LinkedList_Link_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList_Link);
    __CLASS_java_util_LinkedList_Link_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList_Link_1ARRAY);
    __CLASS_java_util_LinkedList_Link_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_LinkedList_Link_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_LinkedList_Link]
    //XMLVM_END_WRAPPER

    __TIB_java_util_LinkedList_Link.classInitialized = 1;
}

void __DELETE_java_util_LinkedList_Link(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_LinkedList_Link]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_LinkedList_Link(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_LinkedList_Link*) me)->fields.java_util_LinkedList_Link.data_ = (java_lang_Object*) JAVA_NULL;
    ((java_util_LinkedList_Link*) me)->fields.java_util_LinkedList_Link.previous_ = (java_util_LinkedList_Link*) JAVA_NULL;
    ((java_util_LinkedList_Link*) me)->fields.java_util_LinkedList_Link.next_ = (java_util_LinkedList_Link*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_LinkedList_Link]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_LinkedList_Link()
{    XMLVM_CLASS_INIT(java_util_LinkedList_Link)
java_util_LinkedList_Link* me = (java_util_LinkedList_Link*) XMLVM_MALLOC(sizeof(java_util_LinkedList_Link));
    me->tib = &__TIB_java_util_LinkedList_Link;
    __INIT_INSTANCE_MEMBERS_java_util_LinkedList_Link(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_LinkedList_Link]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_LinkedList_Link()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_LinkedList_Link___INIT____java_lang_Object_java_util_LinkedList_Link_java_util_LinkedList_Link]
    XMLVM_ENTER_METHOD("java.util.LinkedList$Link", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("LinkedList.java", 48)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("LinkedList.java", 49)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.data_ = _r1.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 50)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.previous_ = _r2.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 51)
    XMLVM_CHECK_NPE(0)
    ((java_util_LinkedList_Link*) _r0.o)->fields.java_util_LinkedList_Link.next_ = _r3.o;
    XMLVM_SOURCE_POSITION("LinkedList.java", 52)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

