#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Exception.h"
#include "java_lang_String.h"
#include "java_lang_reflect_Constructor.h"
#include "org_apache_harmony_niochar_CharsetProviderImpl.h"

#include "org_apache_harmony_niochar_CharsetProviderImpl_1.h"

#define XMLVM_CURRENT_CLASS_NAME CharsetProviderImpl_1
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_niochar_CharsetProviderImpl_1

__TIB_DEFINITION_org_apache_harmony_niochar_CharsetProviderImpl_1 __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1 = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_niochar_CharsetProviderImpl_1, // classInitializer
    "org.apache.harmony.niochar.CharsetProviderImpl$1", // className
    "org.apache.harmony.niochar", // package
    "org.apache.harmony.niochar.CharsetProviderImpl", // enclosingClassName
    "charsetForName:(Ljava/lang/String;)Ljava/nio/charset/Charset;", // enclosingMethodName
    "Ljava/lang/Object;Ljava/security/PrivilegedAction<Ljava/lang/Object;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_apache_harmony_niochar_CharsetProviderImpl_1), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_3ARRAY;
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

void __INIT_org_apache_harmony_niochar_CharsetProviderImpl_1()
{
    staticInitializerLock(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.niochar.CharsetProviderImpl$1")
        __INIT_IMPL_org_apache_harmony_niochar_CharsetProviderImpl_1();
    }
}

void __INIT_IMPL_org_apache_harmony_niochar_CharsetProviderImpl_1()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_niochar_CharsetProviderImpl_1;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.vtable[6] = (VTABLE_PTR) &org_apache_harmony_niochar_CharsetProviderImpl_1_run__;
    // Initialize interface information
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.numImplementedInterfaces = 1;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_security_PrivilegedAction)

    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.implementedInterfaces[0][0] = &__TIB_java_security_PrivilegedAction;
    // Initialize itable for this class
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.itableBegin = &__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.itable[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.itable[XMLVM_ITABLE_IDX_java_security_PrivilegedAction_run__] = __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.vtable[6];


    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1 = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.clazz = __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_1ARRAY);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_niochar_CharsetProviderImpl_1]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_niochar_CharsetProviderImpl_1.classInitialized = 1;
}

void __DELETE_org_apache_harmony_niochar_CharsetProviderImpl_1(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_niochar_CharsetProviderImpl_1]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl_1(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_className_ = (java_lang_String*) JAVA_NULL;
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_canonicalName_ = (java_lang_String*) JAVA_NULL;
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_aliases_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.this_0_ = (org_apache_harmony_niochar_CharsetProviderImpl*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl_1]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_niochar_CharsetProviderImpl_1()
{    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl_1)
org_apache_harmony_niochar_CharsetProviderImpl_1* me = (org_apache_harmony_niochar_CharsetProviderImpl_1*) XMLVM_MALLOC(sizeof(org_apache_harmony_niochar_CharsetProviderImpl_1));
    me->tib = &__TIB_org_apache_harmony_niochar_CharsetProviderImpl_1;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl_1(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_niochar_CharsetProviderImpl_1]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_niochar_CharsetProviderImpl_1()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void org_apache_harmony_niochar_CharsetProviderImpl_1___INIT____org_apache_harmony_niochar_CharsetProviderImpl_java_lang_String_java_lang_String_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_1___INIT____org_apache_harmony_niochar_CharsetProviderImpl_java_lang_String_java_lang_String_java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl$1", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    _r4.o = n4;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 191)
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r0.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.this_0_ = _r1.o;
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r0.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_className_ = _r2.o;
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r0.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_canonicalName_ = _r3.o;
    XMLVM_CHECK_NPE(0)
    ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r0.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_aliases_ = _r4.o;
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_1_run__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_1_run__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl$1", "run", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r6.o = me;
    XMLVM_TRY_BEGIN(w3623aaab5b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 194)
    XMLVM_CHECK_NPE(6)
    _r3.o = ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r6.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_className_;
    _r0.o = java_lang_Class_forName___java_lang_String(_r3.o);
    _r3.i = 2;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 195)
    XMLVM_CLASS_INIT(java_lang_Class)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Class, _r3.i);
    _r4.i = 0;
    XMLVM_CLASS_INIT(java_lang_String)
    _r5.o = __CLASS_java_lang_String;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 1;
    XMLVM_CLASS_INIT(java_lang_String)
    _r5.o = __CLASS_java_lang_String_1ARRAY;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getConstructor___java_lang_Class_1ARRAY(_r0.o, _r3.o);
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 198)
    XMLVM_CHECK_NPE(1)
    java_lang_reflect_AccessibleObject_setAccessible___boolean(_r1.o, _r3.i);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 199)
    _r3.i = 2;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r3.i);
    _r4.i = 0;
    XMLVM_CHECK_NPE(6)
    _r5.o = ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r6.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_canonicalName_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 1;
    XMLVM_CHECK_NPE(6)
    _r5.o = ((org_apache_harmony_niochar_CharsetProviderImpl_1*) _r6.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl_1.val_aliases_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(1)
    _r3.o = java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY(_r1.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3623aaab5b1b2)
        XMLVM_CATCH_SPECIFIC(w3623aaab5b1b2,java_lang_Exception,45)
    XMLVM_CATCH_END(w3623aaab5b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w3623aaab5b1b2)
    label44:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 202)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label45:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 201)
    java_lang_Thread* curThread_w3623aaab5b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r3.o = curThread_w3623aaab5b1b8->fields.java_lang_Thread.xmlvmException_;
    _r2 = _r3;
    _r3.o = JAVA_NULL;
    goto label44;
    //XMLVM_END_WRAPPER
}

