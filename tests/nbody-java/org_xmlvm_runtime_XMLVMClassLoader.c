#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"

#include "org_xmlvm_runtime_XMLVMClassLoader.h"

#define XMLVM_CURRENT_CLASS_NAME XMLVMClassLoader
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_XMLVMClassLoader

__TIB_DEFINITION_org_xmlvm_runtime_XMLVMClassLoader __TIB_org_xmlvm_runtime_XMLVMClassLoader = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_XMLVMClassLoader, // classInitializer
    "org.xmlvm.runtime.XMLVMClassLoader", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_ClassLoader, // extends
    sizeof(org_xmlvm_runtime_XMLVMClassLoader), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_3ARRAY;
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

void __INIT_org_xmlvm_runtime_XMLVMClassLoader()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_XMLVMClassLoader);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_XMLVMClassLoader.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_XMLVMClassLoader);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_XMLVMClassLoader.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_XMLVMClassLoader.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_XMLVMClassLoader.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.XMLVMClassLoader")
        __INIT_IMPL_org_xmlvm_runtime_XMLVMClassLoader();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_XMLVMClassLoader()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_ClassLoader)
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_XMLVMClassLoader;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_XMLVMClassLoader.vtable, __TIB_java_lang_ClassLoader.vtable, sizeof(__TIB_java_lang_ClassLoader.vtable));
    // Initialize vtable for this class
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.vtable[6] = (VTABLE_PTR) &org_xmlvm_runtime_XMLVMClassLoader_findClass___java_lang_String;
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.vtable[7] = (VTABLE_PTR) &org_xmlvm_runtime_XMLVMClassLoader_findResource___java_lang_String;
    // Initialize interface information
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.numImplementedInterfaces = 0;
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_org_xmlvm_runtime_XMLVMClassLoader.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_XMLVMClassLoader = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_XMLVMClassLoader);
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.clazz = __CLASS_org_xmlvm_runtime_XMLVMClassLoader;
    __TIB_org_xmlvm_runtime_XMLVMClassLoader.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_XMLVMClassLoader_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMClassLoader);
    __CLASS_org_xmlvm_runtime_XMLVMClassLoader_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMClassLoader_1ARRAY);
    __CLASS_org_xmlvm_runtime_XMLVMClassLoader_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMClassLoader_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_XMLVMClassLoader]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_XMLVMClassLoader.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_XMLVMClassLoader(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_XMLVMClassLoader]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMClassLoader(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_ClassLoader(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMClassLoader]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMClassLoader()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_XMLVMClassLoader)
org_xmlvm_runtime_XMLVMClassLoader* me = (org_xmlvm_runtime_XMLVMClassLoader*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_XMLVMClassLoader));
    me->tib = &__TIB_org_xmlvm_runtime_XMLVMClassLoader;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMClassLoader(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_XMLVMClassLoader]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMClassLoader()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_xmlvm_runtime_XMLVMClassLoader();
    org_xmlvm_runtime_XMLVMClassLoader___INIT___(me);
    return me;
}

void org_xmlvm_runtime_XMLVMClassLoader___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMClassLoader___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMClassLoader", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("XMLVMClassLoader.java", 28)
    XMLVM_CHECK_NPE(0)
    java_lang_ClassLoader___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMClassLoader_findClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMClassLoader_findClass___java_lang_String]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMClassLoader", "findClass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("XMLVMClassLoader.java", 35)
    _r0.o = java_lang_Class_forName___java_lang_String(_r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMClassLoader_findResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMClassLoader_findResource___java_lang_String]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMClassLoader", "findResource", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("XMLVMClassLoader.java", 44)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r1.o)->tib->vtable[3])(_r1.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getResource___java_lang_String(_r0.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

