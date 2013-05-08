#include "xmlvm.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"

#include "java_lang_reflect_Modifier.h"

#define XMLVM_CURRENT_CLASS_NAME Modifier
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_reflect_Modifier

__TIB_DEFINITION_java_lang_reflect_Modifier __TIB_java_lang_reflect_Modifier = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_Modifier, // classInitializer
    "java.lang.reflect.Modifier", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_reflect_Modifier), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_reflect_Modifier;
JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_lang_reflect_Modifier_PUBLIC;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_PRIVATE;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_PROTECTED;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_STATIC;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_FINAL;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_SYNCHRONIZED;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_VOLATILE;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_TRANSIENT;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_NATIVE;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_INTERFACE;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_ABSTRACT;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_STRICT;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_BRIDGE;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_VARARGS;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_SYNTHETIC;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_ANNOTATION;
static JAVA_INT _STATIC_java_lang_reflect_Modifier_ENUM;

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

void __INIT_java_lang_reflect_Modifier()
{
    staticInitializerLock(&__TIB_java_lang_reflect_Modifier);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_Modifier.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_Modifier.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_Modifier);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_Modifier.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_Modifier.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_Modifier.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.Modifier")
        __INIT_IMPL_java_lang_reflect_Modifier();
    }
}

void __INIT_IMPL_java_lang_reflect_Modifier()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_reflect_Modifier.newInstanceFunc = __NEW_INSTANCE_java_lang_reflect_Modifier;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_reflect_Modifier.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_reflect_Modifier.numImplementedInterfaces = 0;
    __TIB_java_lang_reflect_Modifier.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_reflect_Modifier_PUBLIC = 1;
    _STATIC_java_lang_reflect_Modifier_PRIVATE = 2;
    _STATIC_java_lang_reflect_Modifier_PROTECTED = 4;
    _STATIC_java_lang_reflect_Modifier_STATIC = 8;
    _STATIC_java_lang_reflect_Modifier_FINAL = 16;
    _STATIC_java_lang_reflect_Modifier_SYNCHRONIZED = 32;
    _STATIC_java_lang_reflect_Modifier_VOLATILE = 64;
    _STATIC_java_lang_reflect_Modifier_TRANSIENT = 128;
    _STATIC_java_lang_reflect_Modifier_NATIVE = 256;
    _STATIC_java_lang_reflect_Modifier_INTERFACE = 512;
    _STATIC_java_lang_reflect_Modifier_ABSTRACT = 1024;
    _STATIC_java_lang_reflect_Modifier_STRICT = 2048;
    _STATIC_java_lang_reflect_Modifier_BRIDGE = 64;
    _STATIC_java_lang_reflect_Modifier_VARARGS = 128;
    _STATIC_java_lang_reflect_Modifier_SYNTHETIC = 4096;
    _STATIC_java_lang_reflect_Modifier_ANNOTATION = 8192;
    _STATIC_java_lang_reflect_Modifier_ENUM = 16384;

    __TIB_java_lang_reflect_Modifier.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_Modifier.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_reflect_Modifier.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_reflect_Modifier.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_reflect_Modifier.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_reflect_Modifier.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_Modifier.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_Modifier.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_reflect_Modifier = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_Modifier);
    __TIB_java_lang_reflect_Modifier.clazz = __CLASS_java_lang_reflect_Modifier;
    __TIB_java_lang_reflect_Modifier.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_Modifier_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Modifier);
    __CLASS_java_lang_reflect_Modifier_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Modifier_1ARRAY);
    __CLASS_java_lang_reflect_Modifier_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Modifier_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_reflect_Modifier]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_reflect_Modifier.classInitialized = 1;
}

void __DELETE_java_lang_reflect_Modifier(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_reflect_Modifier]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Modifier(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_reflect_Modifier]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_reflect_Modifier()
{    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
java_lang_reflect_Modifier* me = (java_lang_reflect_Modifier*) XMLVM_MALLOC(sizeof(java_lang_reflect_Modifier));
    me->tib = &__TIB_java_lang_reflect_Modifier;
    __INIT_INSTANCE_MEMBERS_java_lang_reflect_Modifier(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_reflect_Modifier]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Modifier()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_reflect_Modifier();
    java_lang_reflect_Modifier___INIT___(me);
    return me;
}

JAVA_INT java_lang_reflect_Modifier_GET_PUBLIC()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_PUBLIC;
}

void java_lang_reflect_Modifier_PUT_PUBLIC(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_PUBLIC = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_PRIVATE()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_PRIVATE;
}

void java_lang_reflect_Modifier_PUT_PRIVATE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_PRIVATE = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_PROTECTED()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_PROTECTED;
}

void java_lang_reflect_Modifier_PUT_PROTECTED(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_PROTECTED = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_STATIC()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_STATIC;
}

void java_lang_reflect_Modifier_PUT_STATIC(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_STATIC = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_FINAL()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_FINAL;
}

void java_lang_reflect_Modifier_PUT_FINAL(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_FINAL = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_SYNCHRONIZED()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_SYNCHRONIZED;
}

void java_lang_reflect_Modifier_PUT_SYNCHRONIZED(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_SYNCHRONIZED = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_VOLATILE()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_VOLATILE;
}

void java_lang_reflect_Modifier_PUT_VOLATILE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_VOLATILE = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_TRANSIENT()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_TRANSIENT;
}

void java_lang_reflect_Modifier_PUT_TRANSIENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_TRANSIENT = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_NATIVE()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_NATIVE;
}

void java_lang_reflect_Modifier_PUT_NATIVE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_NATIVE = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_INTERFACE()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_INTERFACE;
}

void java_lang_reflect_Modifier_PUT_INTERFACE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_INTERFACE = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_ABSTRACT()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_ABSTRACT;
}

void java_lang_reflect_Modifier_PUT_ABSTRACT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_ABSTRACT = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_STRICT()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_STRICT;
}

void java_lang_reflect_Modifier_PUT_STRICT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_STRICT = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_BRIDGE()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_BRIDGE;
}

void java_lang_reflect_Modifier_PUT_BRIDGE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_BRIDGE = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_VARARGS()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_VARARGS;
}

void java_lang_reflect_Modifier_PUT_VARARGS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_VARARGS = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_SYNTHETIC()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_SYNTHETIC;
}

void java_lang_reflect_Modifier_PUT_SYNTHETIC(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_SYNTHETIC = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_ANNOTATION()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_ANNOTATION;
}

void java_lang_reflect_Modifier_PUT_ANNOTATION(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_ANNOTATION = v;
}

JAVA_INT java_lang_reflect_Modifier_GET_ENUM()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    return _STATIC_java_lang_reflect_Modifier_ENUM;
}

void java_lang_reflect_Modifier_PUT_ENUM(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
_STATIC_java_lang_reflect_Modifier_ENUM = v;
}

void java_lang_reflect_Modifier___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier___INIT___]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Modifier.java", 110)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Modifier.java", 111)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isAbstract___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isAbstract___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isAbstract", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 123)
    _r0.i = _r1.i & 1024;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isFinal___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isFinal___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isFinal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 136)
    _r0.i = _r1.i & 16;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isInterface___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isInterface___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isInterface", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 149)
    _r0.i = _r1.i & 512;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isNative___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isNative___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isNative", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 162)
    _r0.i = _r1.i & 256;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isPrivate___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isPrivate___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isPrivate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 175)
    _r0.i = _r1.i & 2;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isProtected___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isProtected___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isProtected", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 188)
    _r0.i = _r1.i & 4;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isPublic___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isPublic___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isPublic", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 201)
    _r0.i = _r1.i & 1;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isStatic___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isStatic___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isStatic", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 214)
    _r0.i = _r1.i & 8;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isStrict___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isStrict___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isStrict", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 227)
    _r0.i = _r1.i & 2048;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isSynchronized___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isSynchronized___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isSynchronized", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 240)
    _r0.i = _r1.i & 32;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isTransient___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isTransient___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isTransient", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 253)
    _r0.i = _r1.i & 128;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_Modifier_isVolatile___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_isVolatile___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "isVolatile", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 266)
    _r0.i = _r1.i & 64;
    if (_r0.i == 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_Modifier_toString___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_reflect_Modifier)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Modifier_toString___int]
    XMLVM_ENTER_METHOD("java.lang.reflect.Modifier", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("Modifier.java", 282)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Modifier.java", 284)
    _r1.i = java_lang_reflect_Modifier_isPublic___int(_r3.i);
    if (_r1.i == 0) goto label16;
    XMLVM_SOURCE_POSITION("Modifier.java", 285)
    // "public "
    _r1.o = xmlvm_create_java_string_from_pool(650);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label16:;
    XMLVM_SOURCE_POSITION("Modifier.java", 287)
    _r1.i = java_lang_reflect_Modifier_isProtected___int(_r3.i);
    if (_r1.i == 0) goto label27;
    XMLVM_SOURCE_POSITION("Modifier.java", 288)
    // "protected "
    _r1.o = xmlvm_create_java_string_from_pool(651);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label27:;
    XMLVM_SOURCE_POSITION("Modifier.java", 290)
    _r1.i = java_lang_reflect_Modifier_isPrivate___int(_r3.i);
    if (_r1.i == 0) goto label38;
    XMLVM_SOURCE_POSITION("Modifier.java", 291)
    // "private "
    _r1.o = xmlvm_create_java_string_from_pool(652);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label38:;
    XMLVM_SOURCE_POSITION("Modifier.java", 293)
    _r1.i = java_lang_reflect_Modifier_isAbstract___int(_r3.i);
    if (_r1.i == 0) goto label49;
    XMLVM_SOURCE_POSITION("Modifier.java", 294)
    // "abstract "
    _r1.o = xmlvm_create_java_string_from_pool(653);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label49:;
    XMLVM_SOURCE_POSITION("Modifier.java", 296)
    _r1.i = java_lang_reflect_Modifier_isStatic___int(_r3.i);
    if (_r1.i == 0) goto label60;
    XMLVM_SOURCE_POSITION("Modifier.java", 297)
    // "static "
    _r1.o = xmlvm_create_java_string_from_pool(654);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label60:;
    XMLVM_SOURCE_POSITION("Modifier.java", 299)
    _r1.i = java_lang_reflect_Modifier_isFinal___int(_r3.i);
    if (_r1.i == 0) goto label71;
    XMLVM_SOURCE_POSITION("Modifier.java", 300)
    // "final "
    _r1.o = xmlvm_create_java_string_from_pool(655);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label71:;
    XMLVM_SOURCE_POSITION("Modifier.java", 302)
    _r1.i = java_lang_reflect_Modifier_isTransient___int(_r3.i);
    if (_r1.i == 0) goto label82;
    XMLVM_SOURCE_POSITION("Modifier.java", 303)
    // "transient "
    _r1.o = xmlvm_create_java_string_from_pool(656);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label82:;
    XMLVM_SOURCE_POSITION("Modifier.java", 305)
    _r1.i = java_lang_reflect_Modifier_isVolatile___int(_r3.i);
    if (_r1.i == 0) goto label93;
    XMLVM_SOURCE_POSITION("Modifier.java", 306)
    // "volatile "
    _r1.o = xmlvm_create_java_string_from_pool(657);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label93:;
    XMLVM_SOURCE_POSITION("Modifier.java", 308)
    _r1.i = java_lang_reflect_Modifier_isSynchronized___int(_r3.i);
    if (_r1.i == 0) goto label104;
    XMLVM_SOURCE_POSITION("Modifier.java", 309)
    // "synchronized "
    _r1.o = xmlvm_create_java_string_from_pool(658);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label104:;
    XMLVM_SOURCE_POSITION("Modifier.java", 311)
    _r1.i = java_lang_reflect_Modifier_isNative___int(_r3.i);
    if (_r1.i == 0) goto label115;
    XMLVM_SOURCE_POSITION("Modifier.java", 312)
    // "native "
    _r1.o = xmlvm_create_java_string_from_pool(659);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label115:;
    XMLVM_SOURCE_POSITION("Modifier.java", 314)
    _r1.i = java_lang_reflect_Modifier_isStrict___int(_r3.i);
    if (_r1.i == 0) goto label126;
    XMLVM_SOURCE_POSITION("Modifier.java", 315)
    // "strictfp "
    _r1.o = xmlvm_create_java_string_from_pool(660);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label126:;
    XMLVM_SOURCE_POSITION("Modifier.java", 317)
    _r1.i = java_lang_reflect_Modifier_isInterface___int(_r3.i);
    if (_r1.i == 0) goto label137;
    XMLVM_SOURCE_POSITION("Modifier.java", 318)
    // "interface "
    _r1.o = xmlvm_create_java_string_from_pool(661);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label137:;
    XMLVM_SOURCE_POSITION("Modifier.java", 320)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_AbstractStringBuilder_length__(_r0.o);
    if (_r1.i != 0) goto label146;
    XMLVM_SOURCE_POSITION("Modifier.java", 321)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label145:;
    XMLVM_SOURCE_POSITION("Modifier.java", 324)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label146:;
    XMLVM_SOURCE_POSITION("Modifier.java", 323)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_AbstractStringBuilder_length__(_r0.o);
    _r2.i = 1;
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(0)
    java_lang_AbstractStringBuilder_setLength___int(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label145;
    //XMLVM_END_WRAPPER
}

