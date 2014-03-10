#include "xmlvm.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_String.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_io_OutputStream.h"

#define XMLVM_CURRENT_CLASS_NAME OutputStream
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_OutputStream

__TIB_DEFINITION_java_io_OutputStream __TIB_java_io_OutputStream = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_OutputStream, // classInitializer
    "java.io.OutputStream", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_io_OutputStream), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_OutputStream;
JAVA_OBJECT __CLASS_java_io_OutputStream_1ARRAY;
JAVA_OBJECT __CLASS_java_io_OutputStream_2ARRAY;
JAVA_OBJECT __CLASS_java_io_OutputStream_3ARRAY;
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

void __INIT_java_io_OutputStream()
{
    staticInitializerLock(&__TIB_java_io_OutputStream);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_OutputStream.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_OutputStream.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_OutputStream);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_OutputStream.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_OutputStream.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_OutputStream.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.OutputStream")
        __INIT_IMPL_java_io_OutputStream();
    }
}

void __INIT_IMPL_java_io_OutputStream()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_io_OutputStream.newInstanceFunc = __NEW_INSTANCE_java_io_OutputStream;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_OutputStream.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_io_OutputStream.vtable[7] = (VTABLE_PTR) &java_io_OutputStream_close__;
    __TIB_java_io_OutputStream.vtable[8] = (VTABLE_PTR) &java_io_OutputStream_flush__;
    __TIB_java_io_OutputStream.vtable[9] = (VTABLE_PTR) &java_io_OutputStream_write___byte_1ARRAY;
    __TIB_java_io_OutputStream.vtable[10] = (VTABLE_PTR) &java_io_OutputStream_write___byte_1ARRAY_int_int;
    __TIB_java_io_OutputStream.vtable[6] = (VTABLE_PTR) &java_io_OutputStream_checkError__;
    // Initialize interface information
    __TIB_java_io_OutputStream.numImplementedInterfaces = 2;
    __TIB_java_io_OutputStream.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Closeable)

    __TIB_java_io_OutputStream.implementedInterfaces[0][0] = &__TIB_java_io_Closeable;

    XMLVM_CLASS_INIT(java_io_Flushable)

    __TIB_java_io_OutputStream.implementedInterfaces[0][1] = &__TIB_java_io_Flushable;
    // Initialize itable for this class
    __TIB_java_io_OutputStream.itableBegin = &__TIB_java_io_OutputStream.itable[0];
    __TIB_java_io_OutputStream.itable[XMLVM_ITABLE_IDX_java_io_Closeable_close__] = __TIB_java_io_OutputStream.vtable[7];
    __TIB_java_io_OutputStream.itable[XMLVM_ITABLE_IDX_java_io_Flushable_flush__] = __TIB_java_io_OutputStream.vtable[8];


    __TIB_java_io_OutputStream.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_OutputStream.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_OutputStream.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_OutputStream.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_OutputStream.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_OutputStream.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_OutputStream.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_OutputStream.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_OutputStream = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_OutputStream);
    __TIB_java_io_OutputStream.clazz = __CLASS_java_io_OutputStream;
    __TIB_java_io_OutputStream.baseType = JAVA_NULL;
    __CLASS_java_io_OutputStream_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_OutputStream);
    __CLASS_java_io_OutputStream_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_OutputStream_1ARRAY);
    __CLASS_java_io_OutputStream_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_OutputStream_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_OutputStream]
    //XMLVM_END_WRAPPER

    __TIB_java_io_OutputStream.classInitialized = 1;
}

void __DELETE_java_io_OutputStream(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_OutputStream]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_OutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_OutputStream]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_OutputStream()
{    XMLVM_CLASS_INIT(java_io_OutputStream)
java_io_OutputStream* me = (java_io_OutputStream*) XMLVM_MALLOC(sizeof(java_io_OutputStream));
    me->tib = &__TIB_java_io_OutputStream;
    __INIT_INSTANCE_MEMBERS_java_io_OutputStream(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_OutputStream]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_OutputStream()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_io_OutputStream();
    java_io_OutputStream___INIT___(me);
    return me;
}

void java_io_OutputStream___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream___INIT___]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("OutputStream.java", 44)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("OutputStream.java", 45)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_OutputStream_close__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream_close__]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "close", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("OutputStream.java", 56)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_OutputStream_flush__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream_flush__]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "flush", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("OutputStream.java", 67)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_OutputStream_write___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream_write___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "write", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("OutputStream.java", 79)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_io_OutputStream_write___byte_1ARRAY_int_int[10]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_io_OutputStream*) _r2.o)->tib->vtable[10])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("OutputStream.java", 80)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_OutputStream_write___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream_write___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "write", "?")
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
    XMLVM_SOURCE_POSITION("OutputStream.java", 102)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r4.i > _r0.i) goto label11;
    if (_r4.i < 0) goto label11;
    if (_r5.i < 0) goto label11;
    XMLVM_SOURCE_POSITION("OutputStream.java", 103)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r0.i = _r0.i - _r4.i;
    if (_r5.i <= _r0.i) goto label23;
    label11:;
    XMLVM_SOURCE_POSITION("OutputStream.java", 104)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    // "luni.13"
    _r1.o = xmlvm_create_java_string_from_pool(828);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    _r0 = _r4;
    label24:;
    XMLVM_SOURCE_POSITION("OutputStream.java", 106)
    _r1.i = _r4.i + _r5.i;
    if (_r0.i < _r1.i) goto label29;
    XMLVM_SOURCE_POSITION("OutputStream.java", 109)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_SOURCE_POSITION("OutputStream.java", 107)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_io_OutputStream_write___int[11]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_INT)) ((java_io_OutputStream*) _r2.o)->tib->vtable[11])(_r2.o, _r1.i);
    _r0.i = _r0.i + 1;
    goto label24;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_io_OutputStream_checkError__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_OutputStream_checkError__]
    XMLVM_ENTER_METHOD("java.io.OutputStream", "checkError", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("OutputStream.java", 127)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

