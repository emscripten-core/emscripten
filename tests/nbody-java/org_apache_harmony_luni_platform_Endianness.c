#include "xmlvm.h"
#include "java_lang_String.h"

#include "org_apache_harmony_luni_platform_Endianness.h"

#define XMLVM_CURRENT_CLASS_NAME Endianness
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_luni_platform_Endianness

__TIB_DEFINITION_org_apache_harmony_luni_platform_Endianness __TIB_org_apache_harmony_luni_platform_Endianness = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_platform_Endianness, // classInitializer
    "org.apache.harmony.luni.platform.Endianness", // className
    "org.apache.harmony.luni.platform", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_apache_harmony_luni_platform_Endianness), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_org_apache_harmony_luni_platform_Endianness_BIG_ENDIAN;
static JAVA_OBJECT _STATIC_org_apache_harmony_luni_platform_Endianness_LITTLE_ENDIAN;

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

void __INIT_org_apache_harmony_luni_platform_Endianness()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_platform_Endianness);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_platform_Endianness.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_platform_Endianness.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_platform_Endianness);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_platform_Endianness.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_platform_Endianness.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_platform_Endianness.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.platform.Endianness")
        __INIT_IMPL_org_apache_harmony_luni_platform_Endianness();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_platform_Endianness()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_apache_harmony_luni_platform_Endianness.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_luni_platform_Endianness;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_luni_platform_Endianness.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_luni_platform_Endianness.vtable[5] = (VTABLE_PTR) &org_apache_harmony_luni_platform_Endianness_toString__;
    // Initialize interface information
    __TIB_org_apache_harmony_luni_platform_Endianness.numImplementedInterfaces = 0;
    __TIB_org_apache_harmony_luni_platform_Endianness.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_org_apache_harmony_luni_platform_Endianness_BIG_ENDIAN = (org_apache_harmony_luni_platform_Endianness*) JAVA_NULL;
    _STATIC_org_apache_harmony_luni_platform_Endianness_LITTLE_ENDIAN = (org_apache_harmony_luni_platform_Endianness*) JAVA_NULL;

    __TIB_org_apache_harmony_luni_platform_Endianness.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_Endianness.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_platform_Endianness.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_luni_platform_Endianness.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_Endianness.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_platform_Endianness.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_platform_Endianness.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_Endianness.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_luni_platform_Endianness = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_platform_Endianness);
    __TIB_org_apache_harmony_luni_platform_Endianness.clazz = __CLASS_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_Endianness.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_platform_Endianness_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_Endianness);
    __CLASS_org_apache_harmony_luni_platform_Endianness_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_Endianness_1ARRAY);
    __CLASS_org_apache_harmony_luni_platform_Endianness_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_Endianness_2ARRAY);
    org_apache_harmony_luni_platform_Endianness___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_luni_platform_Endianness]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_luni_platform_Endianness.classInitialized = 1;
}

void __DELETE_org_apache_harmony_luni_platform_Endianness(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_luni_platform_Endianness]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_luni_platform_Endianness*) me)->fields.org_apache_harmony_luni_platform_Endianness.displayName_ = (java_lang_String*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_Endianness]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_luni_platform_Endianness()
{    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_Endianness)
org_apache_harmony_luni_platform_Endianness* me = (org_apache_harmony_luni_platform_Endianness*) XMLVM_MALLOC(sizeof(org_apache_harmony_luni_platform_Endianness));
    me->tib = &__TIB_org_apache_harmony_luni_platform_Endianness;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_Endianness(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_luni_platform_Endianness]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_platform_Endianness()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_Endianness)
    return _STATIC_org_apache_harmony_luni_platform_Endianness_BIG_ENDIAN;
}

void org_apache_harmony_luni_platform_Endianness_PUT_BIG_ENDIAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_Endianness)
_STATIC_org_apache_harmony_luni_platform_Endianness_BIG_ENDIAN = v;
}

JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_GET_LITTLE_ENDIAN()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_Endianness)
    return _STATIC_org_apache_harmony_luni_platform_Endianness_LITTLE_ENDIAN;
}

void org_apache_harmony_luni_platform_Endianness_PUT_LITTLE_ENDIAN(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_Endianness)
_STATIC_org_apache_harmony_luni_platform_Endianness_LITTLE_ENDIAN = v;
}

void org_apache_harmony_luni_platform_Endianness___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_Endianness___CLINIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.Endianness", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Endianness.java", 28)
    _r0.o = __NEW_org_apache_harmony_luni_platform_Endianness();
    // "BIG_ENDIAN"
    _r1.o = xmlvm_create_java_string_from_pool(727);
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_Endianness___INIT____java_lang_String(_r0.o, _r1.o);
    org_apache_harmony_luni_platform_Endianness_PUT_BIG_ENDIAN( _r0.o);
    XMLVM_SOURCE_POSITION("Endianness.java", 33)
    _r0.o = __NEW_org_apache_harmony_luni_platform_Endianness();
    XMLVM_SOURCE_POSITION("Endianness.java", 34)
    // "LITTLE_ENDIAN"
    _r1.o = xmlvm_create_java_string_from_pool(728);
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_Endianness___INIT____java_lang_String(_r0.o, _r1.o);
    org_apache_harmony_luni_platform_Endianness_PUT_LITTLE_ENDIAN( _r0.o);
    XMLVM_SOURCE_POSITION("Endianness.java", 24)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_platform_Endianness___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_Endianness___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.Endianness", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Endianness.java", 43)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Endianness.java", 44)
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_luni_platform_Endianness*) _r0.o)->fields.org_apache_harmony_luni_platform_Endianness.displayName_ = _r1.o;
    XMLVM_SOURCE_POSITION("Endianness.java", 45)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_Endianness_toString__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.Endianness", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Endianness.java", 53)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((org_apache_harmony_luni_platform_Endianness*) _r1.o)->fields.org_apache_harmony_luni_platform_Endianness.displayName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

