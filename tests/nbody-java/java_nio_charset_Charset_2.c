#include "xmlvm.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_Thread.h"
#include "java_nio_charset_Charset.h"

#include "java_nio_charset_Charset_2.h"

#define XMLVM_CURRENT_CLASS_NAME Charset_2
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_charset_Charset_2

__TIB_DEFINITION_java_nio_charset_Charset_2 __TIB_java_nio_charset_Charset_2 = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_charset_Charset_2, // classInitializer
    "java.nio.charset.Charset$2", // className
    "java.nio.charset", // package
    "java.nio.charset.Charset", // enclosingClassName
    "getContextClassLoader:()Ljava/lang/ClassLoader;", // enclosingMethodName
    "Ljava/lang/Object;Ljava/security/PrivilegedAction<Ljava/lang/ClassLoader;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_nio_charset_Charset_2), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_charset_Charset_2;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_3ARRAY;
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

void __INIT_java_nio_charset_Charset_2()
{
    staticInitializerLock(&__TIB_java_nio_charset_Charset_2);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_charset_Charset_2.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_charset_Charset_2.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_charset_Charset_2);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_charset_Charset_2.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_charset_Charset_2.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_charset_Charset_2.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.charset.Charset$2")
        __INIT_IMPL_java_nio_charset_Charset_2();
    }
}

void __INIT_IMPL_java_nio_charset_Charset_2()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_nio_charset_Charset_2.newInstanceFunc = __NEW_INSTANCE_java_nio_charset_Charset_2;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_charset_Charset_2.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_charset_Charset_2.vtable[6] = (VTABLE_PTR) &java_nio_charset_Charset_2_run__;
    // Initialize interface information
    __TIB_java_nio_charset_Charset_2.numImplementedInterfaces = 1;
    __TIB_java_nio_charset_Charset_2.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_security_PrivilegedAction)

    __TIB_java_nio_charset_Charset_2.implementedInterfaces[0][0] = &__TIB_java_security_PrivilegedAction;
    // Initialize itable for this class
    __TIB_java_nio_charset_Charset_2.itableBegin = &__TIB_java_nio_charset_Charset_2.itable[0];
    __TIB_java_nio_charset_Charset_2.itable[XMLVM_ITABLE_IDX_java_security_PrivilegedAction_run__] = __TIB_java_nio_charset_Charset_2.vtable[6];


    __TIB_java_nio_charset_Charset_2.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_charset_Charset_2.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_charset_Charset_2.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_charset_Charset_2.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_charset_Charset_2.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_charset_Charset_2.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_charset_Charset_2.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_charset_Charset_2.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_charset_Charset_2 = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_charset_Charset_2);
    __TIB_java_nio_charset_Charset_2.clazz = __CLASS_java_nio_charset_Charset_2;
    __TIB_java_nio_charset_Charset_2.baseType = JAVA_NULL;
    __CLASS_java_nio_charset_Charset_2_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset_2);
    __CLASS_java_nio_charset_Charset_2_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset_2_1ARRAY);
    __CLASS_java_nio_charset_Charset_2_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset_2_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_charset_Charset_2]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_charset_Charset_2.classInitialized = 1;
}

void __DELETE_java_nio_charset_Charset_2(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_charset_Charset_2]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset_2(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_charset_Charset_2*) me)->fields.java_nio_charset_Charset_2.val_t_ = (java_lang_Thread*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_charset_Charset_2]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_charset_Charset_2()
{    XMLVM_CLASS_INIT(java_nio_charset_Charset_2)
java_nio_charset_Charset_2* me = (java_nio_charset_Charset_2*) XMLVM_MALLOC(sizeof(java_nio_charset_Charset_2));
    me->tib = &__TIB_java_nio_charset_Charset_2;
    __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset_2(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_charset_Charset_2]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_Charset_2()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_nio_charset_Charset_2___INIT____java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_2___INIT____java_lang_Thread]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset$2", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 1)
    XMLVM_CHECK_NPE(0)
    ((java_nio_charset_Charset_2*) _r0.o)->fields.java_nio_charset_Charset_2.val_t_ = _r1.o;
    XMLVM_SOURCE_POSITION("Charset.java", 209)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_2_run__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_2_run__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset$2", "run", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 211)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset_2*) _r1.o)->fields.java_nio_charset_Charset_2.val_t_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Thread_getContextClassLoader__(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

