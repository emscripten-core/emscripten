#include "xmlvm.h"

#include "org_xmlvm_runtime_XMLVMOutputStream.h"

#define XMLVM_CURRENT_CLASS_NAME XMLVMOutputStream
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_XMLVMOutputStream

__TIB_DEFINITION_org_xmlvm_runtime_XMLVMOutputStream __TIB_org_xmlvm_runtime_XMLVMOutputStream = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_XMLVMOutputStream, // classInitializer
    "org.xmlvm.runtime.XMLVMOutputStream", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_OutputStream, // extends
    sizeof(org_xmlvm_runtime_XMLVMOutputStream), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_3ARRAY;
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

void __INIT_org_xmlvm_runtime_XMLVMOutputStream()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_XMLVMOutputStream);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_XMLVMOutputStream.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_XMLVMOutputStream);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_XMLVMOutputStream.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_XMLVMOutputStream.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_XMLVMOutputStream.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.XMLVMOutputStream")
        __INIT_IMPL_org_xmlvm_runtime_XMLVMOutputStream();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_XMLVMOutputStream()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_OutputStream)
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_XMLVMOutputStream;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_XMLVMOutputStream.vtable, __TIB_java_io_OutputStream.vtable, sizeof(__TIB_java_io_OutputStream.vtable));
    // Initialize vtable for this class
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.vtable[11] = (VTABLE_PTR) &org_xmlvm_runtime_XMLVMOutputStream_write___int;
    // Initialize interface information
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.numImplementedInterfaces = 2;
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Closeable)

    __TIB_org_xmlvm_runtime_XMLVMOutputStream.implementedInterfaces[0][0] = &__TIB_java_io_Closeable;

    XMLVM_CLASS_INIT(java_io_Flushable)

    __TIB_org_xmlvm_runtime_XMLVMOutputStream.implementedInterfaces[0][1] = &__TIB_java_io_Flushable;
    // Initialize itable for this class
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.itableBegin = &__TIB_org_xmlvm_runtime_XMLVMOutputStream.itable[0];
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.itable[XMLVM_ITABLE_IDX_java_io_Closeable_close__] = __TIB_org_xmlvm_runtime_XMLVMOutputStream.vtable[7];
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.itable[XMLVM_ITABLE_IDX_java_io_Flushable_flush__] = __TIB_org_xmlvm_runtime_XMLVMOutputStream.vtable[8];


    __TIB_org_xmlvm_runtime_XMLVMOutputStream.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_XMLVMOutputStream = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_XMLVMOutputStream);
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.clazz = __CLASS_org_xmlvm_runtime_XMLVMOutputStream;
    __TIB_org_xmlvm_runtime_XMLVMOutputStream.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_XMLVMOutputStream_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMOutputStream);
    __CLASS_org_xmlvm_runtime_XMLVMOutputStream_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMOutputStream_1ARRAY);
    __CLASS_org_xmlvm_runtime_XMLVMOutputStream_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMOutputStream_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_XMLVMOutputStream]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_XMLVMOutputStream.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_XMLVMOutputStream(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_XMLVMOutputStream]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMOutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_OutputStream(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMOutputStream]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMOutputStream()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_XMLVMOutputStream)
org_xmlvm_runtime_XMLVMOutputStream* me = (org_xmlvm_runtime_XMLVMOutputStream*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_XMLVMOutputStream));
    me->tib = &__TIB_org_xmlvm_runtime_XMLVMOutputStream;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMOutputStream(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_XMLVMOutputStream]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMOutputStream()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_xmlvm_runtime_XMLVMOutputStream();
    org_xmlvm_runtime_XMLVMOutputStream___INIT___(me);
    return me;
}

void org_xmlvm_runtime_XMLVMOutputStream___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMOutputStream___INIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMOutputStream", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("XMLVMOutputStream.java", 25)
    XMLVM_CHECK_NPE(0)
    java_io_OutputStream___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_xmlvm_runtime_XMLVMOutputStream_write___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMOutputStream_write___int]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMOutputStream", "write", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("XMLVMOutputStream.java", 34)
    org_xmlvm_runtime_XMLVMOutputStream_nativeWrite___int(_r1.i);
    XMLVM_SOURCE_POSITION("XMLVMOutputStream.java", 35)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_xmlvm_runtime_XMLVMOutputStream_nativeWrite___int(JAVA_INT n1)]

