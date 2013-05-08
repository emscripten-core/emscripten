#include "xmlvm.h"
#include "java_lang_RuntimePermission.h"
#include "java_lang_SecurityManager.h"
#include "java_lang_String.h"
#include "java_lang_System.h"
#include "java_nio_charset_Charset.h"
#include "java_util_Iterator.h"

#include "java_nio_charset_spi_CharsetProvider.h"

#define XMLVM_CURRENT_CLASS_NAME CharsetProvider
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_charset_spi_CharsetProvider

__TIB_DEFINITION_java_nio_charset_spi_CharsetProvider __TIB_java_nio_charset_spi_CharsetProvider = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_charset_spi_CharsetProvider, // classInitializer
    "java.nio.charset.spi.CharsetProvider", // className
    "java.nio.charset.spi", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_nio_charset_spi_CharsetProvider), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_charset_spi_CharsetProvider;
JAVA_OBJECT __CLASS_java_nio_charset_spi_CharsetProvider_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_spi_CharsetProvider_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_spi_CharsetProvider_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_nio_charset_spi_CharsetProvider_CONSTRUCT_PERM;

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

void __INIT_java_nio_charset_spi_CharsetProvider()
{
    staticInitializerLock(&__TIB_java_nio_charset_spi_CharsetProvider);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_charset_spi_CharsetProvider.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_charset_spi_CharsetProvider.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_charset_spi_CharsetProvider);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_charset_spi_CharsetProvider.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_charset_spi_CharsetProvider.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_charset_spi_CharsetProvider.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.charset.spi.CharsetProvider")
        __INIT_IMPL_java_nio_charset_spi_CharsetProvider();
    }
}

void __INIT_IMPL_java_nio_charset_spi_CharsetProvider()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_nio_charset_spi_CharsetProvider.newInstanceFunc = __NEW_INSTANCE_java_nio_charset_spi_CharsetProvider;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_charset_spi_CharsetProvider.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_nio_charset_spi_CharsetProvider.numImplementedInterfaces = 0;
    __TIB_java_nio_charset_spi_CharsetProvider.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_nio_charset_spi_CharsetProvider_CONSTRUCT_PERM = (java_lang_RuntimePermission*) JAVA_NULL;

    __TIB_java_nio_charset_spi_CharsetProvider.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_charset_spi_CharsetProvider.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_charset_spi_CharsetProvider.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_charset_spi_CharsetProvider.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_charset_spi_CharsetProvider.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_charset_spi_CharsetProvider.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_charset_spi_CharsetProvider.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_charset_spi_CharsetProvider.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_charset_spi_CharsetProvider = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_charset_spi_CharsetProvider);
    __TIB_java_nio_charset_spi_CharsetProvider.clazz = __CLASS_java_nio_charset_spi_CharsetProvider;
    __TIB_java_nio_charset_spi_CharsetProvider.baseType = JAVA_NULL;
    __CLASS_java_nio_charset_spi_CharsetProvider_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_spi_CharsetProvider);
    __CLASS_java_nio_charset_spi_CharsetProvider_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_spi_CharsetProvider_1ARRAY);
    __CLASS_java_nio_charset_spi_CharsetProvider_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_spi_CharsetProvider_2ARRAY);
    java_nio_charset_spi_CharsetProvider___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_charset_spi_CharsetProvider]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_charset_spi_CharsetProvider.classInitialized = 1;
}

void __DELETE_java_nio_charset_spi_CharsetProvider(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_charset_spi_CharsetProvider]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_charset_spi_CharsetProvider(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_charset_spi_CharsetProvider]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_charset_spi_CharsetProvider()
{    XMLVM_CLASS_INIT(java_nio_charset_spi_CharsetProvider)
java_nio_charset_spi_CharsetProvider* me = (java_nio_charset_spi_CharsetProvider*) XMLVM_MALLOC(sizeof(java_nio_charset_spi_CharsetProvider));
    me->tib = &__TIB_java_nio_charset_spi_CharsetProvider;
    __INIT_INSTANCE_MEMBERS_java_nio_charset_spi_CharsetProvider(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_charset_spi_CharsetProvider]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_spi_CharsetProvider()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_nio_charset_spi_CharsetProvider();
    java_nio_charset_spi_CharsetProvider___INIT___(me);
    return me;
}

JAVA_OBJECT java_nio_charset_spi_CharsetProvider_GET_CONSTRUCT_PERM()
{
    XMLVM_CLASS_INIT(java_nio_charset_spi_CharsetProvider)
    return _STATIC_java_nio_charset_spi_CharsetProvider_CONSTRUCT_PERM;
}

void java_nio_charset_spi_CharsetProvider_PUT_CONSTRUCT_PERM(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_spi_CharsetProvider)
_STATIC_java_nio_charset_spi_CharsetProvider_CONSTRUCT_PERM = v;
}

void java_nio_charset_spi_CharsetProvider___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_spi_CharsetProvider___CLINIT___]
    XMLVM_ENTER_METHOD("java.nio.charset.spi.CharsetProvider", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 28)
    _r0.o = __NEW_java_lang_RuntimePermission();
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 29)
    // "charsetProvider"
    _r1.o = xmlvm_create_java_string_from_pool(59);
    XMLVM_CHECK_NPE(0)
    java_lang_RuntimePermission___INIT____java_lang_String(_r0.o, _r1.o);
    java_nio_charset_spi_CharsetProvider_PUT_CONSTRUCT_PERM( _r0.o);
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 25)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_spi_CharsetProvider___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_spi_CharsetProvider___INIT___]
    XMLVM_ENTER_METHOD("java.nio.charset.spi.CharsetProvider", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 38)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 39)
    _r0.o = java_lang_System_getSecurityManager__();
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 40)
    if (_r0.o == JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 41)
    _r1.o = java_nio_charset_spi_CharsetProvider_GET_CONSTRUCT_PERM();
    XMLVM_CHECK_NPE(0)
    java_lang_SecurityManager_checkPermission___java_security_Permission(_r0.o, _r1.o);
    label14:;
    XMLVM_SOURCE_POSITION("CharsetProvider.java", 42)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

