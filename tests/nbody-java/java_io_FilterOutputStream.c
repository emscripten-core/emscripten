#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"
#include "java_lang_Throwable.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_io_FilterOutputStream.h"

#define XMLVM_CURRENT_CLASS_NAME FilterOutputStream
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_FilterOutputStream

__TIB_DEFINITION_java_io_FilterOutputStream __TIB_java_io_FilterOutputStream = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_FilterOutputStream, // classInitializer
    "java.io.FilterOutputStream", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_io_OutputStream, // extends
    sizeof(java_io_FilterOutputStream), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_FilterOutputStream;
JAVA_OBJECT __CLASS_java_io_FilterOutputStream_1ARRAY;
JAVA_OBJECT __CLASS_java_io_FilterOutputStream_2ARRAY;
JAVA_OBJECT __CLASS_java_io_FilterOutputStream_3ARRAY;
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

void __INIT_java_io_FilterOutputStream()
{
    staticInitializerLock(&__TIB_java_io_FilterOutputStream);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_FilterOutputStream.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_FilterOutputStream.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_FilterOutputStream);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_FilterOutputStream.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_FilterOutputStream.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_FilterOutputStream.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.FilterOutputStream")
        __INIT_IMPL_java_io_FilterOutputStream();
    }
}

void __INIT_IMPL_java_io_FilterOutputStream()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_io_OutputStream)
    __TIB_java_io_FilterOutputStream.newInstanceFunc = __NEW_INSTANCE_java_io_FilterOutputStream;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_FilterOutputStream.vtable, __TIB_java_io_OutputStream.vtable, sizeof(__TIB_java_io_OutputStream.vtable));
    // Initialize vtable for this class
    __TIB_java_io_FilterOutputStream.vtable[7] = (VTABLE_PTR) &java_io_FilterOutputStream_close__;
    __TIB_java_io_FilterOutputStream.vtable[8] = (VTABLE_PTR) &java_io_FilterOutputStream_flush__;
    __TIB_java_io_FilterOutputStream.vtable[9] = (VTABLE_PTR) &java_io_FilterOutputStream_write___byte_1ARRAY;
    __TIB_java_io_FilterOutputStream.vtable[10] = (VTABLE_PTR) &java_io_FilterOutputStream_write___byte_1ARRAY_int_int;
    __TIB_java_io_FilterOutputStream.vtable[11] = (VTABLE_PTR) &java_io_FilterOutputStream_write___int;
    // Initialize interface information
    __TIB_java_io_FilterOutputStream.numImplementedInterfaces = 2;
    __TIB_java_io_FilterOutputStream.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Closeable)

    __TIB_java_io_FilterOutputStream.implementedInterfaces[0][0] = &__TIB_java_io_Closeable;

    XMLVM_CLASS_INIT(java_io_Flushable)

    __TIB_java_io_FilterOutputStream.implementedInterfaces[0][1] = &__TIB_java_io_Flushable;
    // Initialize itable for this class
    __TIB_java_io_FilterOutputStream.itableBegin = &__TIB_java_io_FilterOutputStream.itable[0];
    __TIB_java_io_FilterOutputStream.itable[XMLVM_ITABLE_IDX_java_io_Closeable_close__] = __TIB_java_io_FilterOutputStream.vtable[7];
    __TIB_java_io_FilterOutputStream.itable[XMLVM_ITABLE_IDX_java_io_Flushable_flush__] = __TIB_java_io_FilterOutputStream.vtable[8];


    __TIB_java_io_FilterOutputStream.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_FilterOutputStream.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_FilterOutputStream.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_FilterOutputStream.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_FilterOutputStream.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_FilterOutputStream.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_FilterOutputStream.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_FilterOutputStream.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_FilterOutputStream = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_FilterOutputStream);
    __TIB_java_io_FilterOutputStream.clazz = __CLASS_java_io_FilterOutputStream;
    __TIB_java_io_FilterOutputStream.baseType = JAVA_NULL;
    __CLASS_java_io_FilterOutputStream_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FilterOutputStream);
    __CLASS_java_io_FilterOutputStream_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FilterOutputStream_1ARRAY);
    __CLASS_java_io_FilterOutputStream_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_FilterOutputStream_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_FilterOutputStream]
    //XMLVM_END_WRAPPER

    __TIB_java_io_FilterOutputStream.classInitialized = 1;
}

void __DELETE_java_io_FilterOutputStream(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_FilterOutputStream]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_FilterOutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_io_OutputStream(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_io_FilterOutputStream*) me)->fields.java_io_FilterOutputStream.out_ = (java_io_OutputStream*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_FilterOutputStream]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_FilterOutputStream()
{    XMLVM_CLASS_INIT(java_io_FilterOutputStream)
java_io_FilterOutputStream* me = (java_io_FilterOutputStream*) XMLVM_MALLOC(sizeof(java_io_FilterOutputStream));
    me->tib = &__TIB_java_io_FilterOutputStream;
    __INIT_INSTANCE_MEMBERS_java_io_FilterOutputStream(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_FilterOutputStream]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_FilterOutputStream()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_io_FilterOutputStream___INIT____java_io_OutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream___INIT____java_io_OutputStream]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 47)
    XMLVM_CHECK_NPE(0)
    java_io_OutputStream___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 48)
    XMLVM_CHECK_NPE(0)
    ((java_io_FilterOutputStream*) _r0.o)->fields.java_io_FilterOutputStream.out_ = _r1.o;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 49)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_FilterOutputStream_close__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream_close__]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "close", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 59)
    _r0.o = JAVA_NULL;
    XMLVM_TRY_BEGIN(w4463aaab2b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 61)
    //java_io_FilterOutputStream_flush__[8]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT)) ((java_io_FilterOutputStream*) _r2.o)->tib->vtable[8])(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4463aaab2b1b4)
        XMLVM_CATCH_SPECIFIC(w4463aaab2b1b4,java_lang_Throwable,20)
    XMLVM_CATCH_END(w4463aaab2b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w4463aaab2b1b4)
    label4:;
    XMLVM_TRY_BEGIN(w4463aaab2b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 67)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_io_FilterOutputStream*) _r2.o)->fields.java_io_FilterOutputStream.out_;
    //java_io_OutputStream_close__[7]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT)) ((java_io_OutputStream*) _r1.o)->tib->vtable[7])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4463aaab2b1b6)
        XMLVM_CATCH_SPECIFIC(w4463aaab2b1b6,java_lang_Throwable,15)
    XMLVM_CATCH_END(w4463aaab2b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w4463aaab2b1b6)
    label9:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 74)
    if (_r0.o == JAVA_NULL) goto label14;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 75)

    
    // Red class access removed: org.apache.harmony.luni.util.SneakyThrow::sneakyThrow
    XMLVM_RED_CLASS_DEPENDENCY();
    label14:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 77)
    XMLVM_EXIT_METHOD()
    return;
    label15:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 68)
    java_lang_Thread* curThread_w4463aaab2b1c17 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4463aaab2b1c17->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 69)
    if (_r0.o != JAVA_NULL) goto label9;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 70)
    goto label9;
    label20:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 62)
    java_lang_Thread* curThread_w4463aaab2b1c25 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4463aaab2b1c25->fields.java_lang_Thread.xmlvmException_;
    goto label4;
    //XMLVM_END_WRAPPER
}

void java_io_FilterOutputStream_flush__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream_flush__]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "flush", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 88)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_io_FilterOutputStream*) _r1.o)->fields.java_io_FilterOutputStream.out_;
    //java_io_OutputStream_flush__[8]
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT)) ((java_io_OutputStream*) _r0.o)->tib->vtable[8])(_r0.o);
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 89)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_FilterOutputStream_write___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream_write___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "write", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 103)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_io_FilterOutputStream_write___byte_1ARRAY_int_int[10]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_io_FilterOutputStream*) _r2.o)->tib->vtable[10])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 104)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_FilterOutputStream_write___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream_write___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "write", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 126)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r4.i > _r0.i) goto label5;
    if (_r4.i >= 0) goto label17;
    label5:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 128)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.12"
    _r1.o = xmlvm_create_java_string_from_pool(61);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r1.o, _r4.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 130)
    if (_r5.i < 0) goto label23;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r0.i = _r0.i - _r4.i;
    if (_r5.i <= _r0.i) goto label35;
    label23:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 132)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.18"
    _r1.o = xmlvm_create_java_string_from_pool(62);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r1.o, _r5.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label35:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 134)
    _r0.i = 0;
    label36:;
    if (_r0.i < _r5.i) goto label39;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 139)
    XMLVM_EXIT_METHOD()
    return;
    label39:;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 137)
    _r1.i = _r4.i + _r0.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    //java_io_FilterOutputStream_write___int[11]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_INT)) ((java_io_FilterOutputStream*) _r2.o)->tib->vtable[11])(_r2.o, _r1.i);
    _r0.i = _r0.i + 1;
    goto label36;
    //XMLVM_END_WRAPPER
}

void java_io_FilterOutputStream_write___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_FilterOutputStream_write___int]
    XMLVM_ENTER_METHOD("java.io.FilterOutputStream", "write", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 152)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_io_FilterOutputStream*) _r1.o)->fields.java_io_FilterOutputStream.out_;
    //java_io_OutputStream_write___int[11]
    XMLVM_CHECK_NPE(0)
    (*(void (*)(JAVA_OBJECT, JAVA_INT)) ((java_io_OutputStream*) _r0.o)->tib->vtable[11])(_r0.o, _r2.i);
    XMLVM_SOURCE_POSITION("FilterOutputStream.java", 153)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

