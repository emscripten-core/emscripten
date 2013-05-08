#include "xmlvm.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Math.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_nio_BufferFactory.h"

#include "java_nio_CharBuffer.h"

#define XMLVM_CURRENT_CLASS_NAME CharBuffer
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_CharBuffer

__TIB_DEFINITION_java_nio_CharBuffer __TIB_java_nio_CharBuffer = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_CharBuffer, // classInitializer
    "java.nio.CharBuffer", // className
    "java.nio", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/nio/Buffer;Ljava/lang/Comparable<Ljava/nio/CharBuffer;>;Ljava/lang/CharSequence;Ljava/lang/Appendable;Ljava/lang/Readable;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_Buffer, // extends
    sizeof(java_nio_CharBuffer), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_CharBuffer;
JAVA_OBJECT __CLASS_java_nio_CharBuffer_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_CharBuffer_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_CharBuffer_3ARRAY;
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

void __INIT_java_nio_CharBuffer()
{
    staticInitializerLock(&__TIB_java_nio_CharBuffer);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_CharBuffer.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_CharBuffer.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_CharBuffer);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_CharBuffer.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_CharBuffer.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_CharBuffer.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.CharBuffer")
        __INIT_IMPL_java_nio_CharBuffer();
    }
}

void __INIT_IMPL_java_nio_CharBuffer()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_Buffer)
    __TIB_java_nio_CharBuffer.newInstanceFunc = __NEW_INSTANCE_java_nio_CharBuffer;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_CharBuffer.vtable, __TIB_java_nio_Buffer.vtable, sizeof(__TIB_java_nio_Buffer.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_CharBuffer.vtable[7] = (VTABLE_PTR) &java_nio_CharBuffer_array__;
    __TIB_java_nio_CharBuffer.vtable[6] = (VTABLE_PTR) &java_nio_CharBuffer_arrayOffset__;
    __TIB_java_nio_CharBuffer.vtable[15] = (VTABLE_PTR) &java_nio_CharBuffer_charAt___int;
    __TIB_java_nio_CharBuffer.vtable[1] = (VTABLE_PTR) &java_nio_CharBuffer_equals___java_lang_Object;
    __TIB_java_nio_CharBuffer.vtable[20] = (VTABLE_PTR) &java_nio_CharBuffer_get___char_1ARRAY_int_int;
    __TIB_java_nio_CharBuffer.vtable[8] = (VTABLE_PTR) &java_nio_CharBuffer_hasArray__;
    __TIB_java_nio_CharBuffer.vtable[4] = (VTABLE_PTR) &java_nio_CharBuffer_hashCode__;
    __TIB_java_nio_CharBuffer.vtable[22] = (VTABLE_PTR) &java_nio_CharBuffer_length__;
    __TIB_java_nio_CharBuffer.vtable[28] = (VTABLE_PTR) &java_nio_CharBuffer_put___char_1ARRAY_int_int;
    __TIB_java_nio_CharBuffer.vtable[5] = (VTABLE_PTR) &java_nio_CharBuffer_toString__;
    __TIB_java_nio_CharBuffer.vtable[11] = (VTABLE_PTR) &java_nio_CharBuffer_append___char;
    __TIB_java_nio_CharBuffer.vtable[12] = (VTABLE_PTR) &java_nio_CharBuffer_append___java_lang_CharSequence;
    __TIB_java_nio_CharBuffer.vtable[13] = (VTABLE_PTR) &java_nio_CharBuffer_append___java_lang_CharSequence_int_int;
    __TIB_java_nio_CharBuffer.vtable[30] = (VTABLE_PTR) &java_nio_CharBuffer_read___java_nio_CharBuffer;
    __TIB_java_nio_CharBuffer.vtable[17] = (VTABLE_PTR) &java_nio_CharBuffer_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_nio_CharBuffer.numImplementedInterfaces = 4;
    __TIB_java_nio_CharBuffer.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 4);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Appendable)

    __TIB_java_nio_CharBuffer.implementedInterfaces[0][0] = &__TIB_java_lang_Appendable;

    XMLVM_CLASS_INIT(java_lang_CharSequence)

    __TIB_java_nio_CharBuffer.implementedInterfaces[0][1] = &__TIB_java_lang_CharSequence;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_nio_CharBuffer.implementedInterfaces[0][2] = &__TIB_java_lang_Comparable;

    XMLVM_CLASS_INIT(java_lang_Readable)

    __TIB_java_nio_CharBuffer.implementedInterfaces[0][3] = &__TIB_java_lang_Readable;
    // Initialize itable for this class
    __TIB_java_nio_CharBuffer.itableBegin = &__TIB_java_nio_CharBuffer.itable[0];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___char] = __TIB_java_nio_CharBuffer.vtable[11];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence] = __TIB_java_nio_CharBuffer.vtable[12];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence_int_int] = __TIB_java_nio_CharBuffer.vtable[13];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int] = __TIB_java_nio_CharBuffer.vtable[15];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__] = __TIB_java_nio_CharBuffer.vtable[22];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int] = __TIB_java_nio_CharBuffer.vtable[32];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__] = __TIB_java_nio_CharBuffer.vtable[5];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_nio_CharBuffer.vtable[17];
    __TIB_java_nio_CharBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Readable_read___java_nio_CharBuffer] = __TIB_java_nio_CharBuffer.vtable[30];


    __TIB_java_nio_CharBuffer.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_CharBuffer.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_CharBuffer.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_CharBuffer.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_CharBuffer.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_CharBuffer.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_CharBuffer.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_CharBuffer.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_CharBuffer = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_CharBuffer);
    __TIB_java_nio_CharBuffer.clazz = __CLASS_java_nio_CharBuffer;
    __TIB_java_nio_CharBuffer.baseType = JAVA_NULL;
    __CLASS_java_nio_CharBuffer_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_CharBuffer);
    __CLASS_java_nio_CharBuffer_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_CharBuffer_1ARRAY);
    __CLASS_java_nio_CharBuffer_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_CharBuffer_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_CharBuffer]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_CharBuffer.classInitialized = 1;
}

void __DELETE_java_nio_CharBuffer(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_CharBuffer]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_CharBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_Buffer(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_CharBuffer]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_CharBuffer()
{    XMLVM_CLASS_INIT(java_nio_CharBuffer)
java_nio_CharBuffer* me = (java_nio_CharBuffer*) XMLVM_MALLOC(sizeof(java_nio_CharBuffer));
    me->tib = &__TIB_java_nio_CharBuffer;
    __INIT_INSTANCE_MEMBERS_java_nio_CharBuffer(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_CharBuffer]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_CharBuffer()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_nio_CharBuffer_allocate___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_allocate___int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "allocate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 50)
    if (_r1.i >= 0) goto label8;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 51)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 53)
    _r0.o = java_nio_BufferFactory_newCharBuffer___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_wrap___char_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_wrap___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "wrap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 67)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r0.o = java_nio_CharBuffer_wrap___char_1ARRAY_int_int(_r2.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_wrap___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_wrap___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "wrap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = n1;
    _r6.i = n2;
    _r7.i = n3;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 89)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    XMLVM_SOURCE_POSITION("CharBuffer.java", 90)
    if (_r6.i < 0) goto label13;
    if (_r7.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r6.i;
    _r3.l = (JAVA_LONG) _r7.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 91)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 94)
    _r0.o = java_nio_BufferFactory_newCharBuffer___char_1ARRAY(_r5.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 95)
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.position_ = _r6.i;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 96)
    _r1.i = _r6.i + _r7.i;
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.limit_ = _r1.i;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 98)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_wrap___java_lang_CharSequence(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_wrap___java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "wrap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 112)
    _r0.o = java_nio_BufferFactory_newCharBuffer___java_lang_CharSequence(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_wrap___java_lang_CharSequence_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_wrap___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "wrap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = n1;
    _r2.i = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 135)
    if (_r1.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 136)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 138)
    if (_r2.i < 0) goto label18;
    if (_r3.i < _r2.i) goto label18;
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r1.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r1.o);
    if (_r3.i <= _r0.i) goto label24;
    label18:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 139)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 142)
    _r0.o = java_nio_BufferFactory_newCharBuffer___java_lang_CharSequence(_r1.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 143)
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.position_ = _r2.i;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 144)
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.limit_ = _r3.i;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 145)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_CharBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer___INIT____int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 155)
    XMLVM_CHECK_NPE(0)
    java_nio_Buffer___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 156)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_array__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_array__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "array", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 168)
    //java_nio_CharBuffer_protectedArray__[25]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r1.o)->tib->vtable[25])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_arrayOffset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_arrayOffset__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "arrayOffset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 185)
    //java_nio_CharBuffer_protectedArrayOffset__[24]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r1.o)->tib->vtable[24])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_nio_CharBuffer_charAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_charAt___int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "charAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 217)
    if (_r2.i < 0) goto label8;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_nio_Buffer_remaining__(_r1.o);
    if (_r2.i < _r0.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 218)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 220)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_nio_Buffer*) _r1.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + _r2.i;
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r1.o)->tib->vtable[21])(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_compareTo___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_compareTo___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 249)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    if (_r0.i >= _r1.i) goto label33;
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    label14:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 251)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 252)
    XMLVM_CHECK_NPE(7)
    _r2.i = ((java_nio_Buffer*) _r7.o)->fields.java_nio_Buffer.position_;
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    label21:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 254)
    if (_r2.i > 0) goto label38;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 264)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    _r0.i = _r0.i - _r1.i;
    label32:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label33:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 250)
    XMLVM_CHECK_NPE(7)
    _r0.i = java_nio_Buffer_remaining__(_r7.o);
    goto label14;
    label38:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 255)
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(6)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r6.o)->tib->vtable[21])(_r6.o, _r1.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 256)
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(7)
    _r4.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r7.o)->tib->vtable[21])(_r7.o, _r0.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 257)
    if (_r3.i == _r4.i) goto label54;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 258)
    if (_r3.i >= _r4.i) goto label52;
    _r0.i = -1;
    goto label32;
    label52:;
    _r0.i = 1;
    goto label32;
    label54:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 260)
    _r1.i = _r1.i + 1;
    _r0.i = _r0.i + 1;
    _r2.i = _r2.i + -1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 262)
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_CharBuffer_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r5.i = 1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 296)
    XMLVM_CLASS_INIT(java_nio_CharBuffer)
    _r0.i = XMLVM_ISA(_r7.o, __CLASS_java_nio_CharBuffer);
    if (_r0.i != 0) goto label8;
    _r0 = _r4;
    label7:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 297)
    XMLVM_SOURCE_POSITION("CharBuffer.java", 312)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 299)
    _r7.o = _r7.o;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 301)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    if (_r0.i == _r1.i) goto label22;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 302)
    goto label7;
    label22:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 305)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 306)
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_nio_Buffer*) _r7.o)->fields.java_nio_Buffer.position_;
    _r2 = _r0;
    _r0 = _r5;
    label28:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 308)
    if (_r0.i == 0) goto label7;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.limit_;
    if (_r2.i >= _r3.i) goto label7;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 309)
    _r0.i = _r2.i + 1;
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(6)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r6.o)->tib->vtable[21])(_r6.o, _r2.i);
    _r3.i = _r1.i + 1;
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r7.o)->tib->vtable[21])(_r7.o, _r1.i);
    if (_r2.i != _r1.i) goto label53;
    _r1 = _r5;
    label49:;
    _r2 = _r0;
    _r0 = _r1;
    _r1 = _r3;
    goto label28;
    label53:;
    _r1 = _r4;
    goto label49;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_get___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_get___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 338)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_nio_CharBuffer_get___char_1ARRAY_int_int[20]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_nio_CharBuffer*) _r2.o)->tib->vtable[20])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_get___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_get___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.i = n2;
    _r8.i = n3;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 361)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("CharBuffer.java", 362)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r7.i;
    _r3.l = (JAVA_LONG) _r8.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 363)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 366)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 367)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    _r0 = _r7;
    label32:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 369)
    _r1.i = _r7.i + _r8.i;
    if (_r0.i < _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 372)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    label37:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 370)
    //java_nio_CharBuffer_get__[19]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r5.o)->tib->vtable[19])(_r5.o);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    _r0.i = _r0.i + 1;
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_CharBuffer_hasArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_hasArray__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "hasArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 393)
    //java_nio_CharBuffer_protectedHasArray__[26]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r1.o)->tib->vtable[26])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_hashCode__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 404)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 405)
    _r1.i = 0;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label6:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 406)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.limit_;
    if (_r1.i < _r2.i) goto label11;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 409)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 407)
    _r2.i = _r1.i + 1;
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r4.o)->tib->vtable[21])(_r4.o, _r1.i);
    _r0.i = _r0.i + _r1.i;
    _r1 = _r2;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_length__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_length__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "length", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 430)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_nio_Buffer_remaining__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_put___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_put___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 496)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_nio_CharBuffer_put___char_1ARRAY_int_int[28]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_nio_CharBuffer*) _r2.o)->tib->vtable[28])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_put___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_put___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.i = n2;
    _r8.i = n3;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 521)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("CharBuffer.java", 522)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r7.i;
    _r3.l = (JAVA_LONG) _r8.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 523)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 526)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 527)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    _r0 = _r7;
    label32:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 529)
    _r1.i = _r7.i + _r8.i;
    if (_r0.i < _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 532)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    label37:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 530)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_nio_CharBuffer_put___char[27]
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_nio_CharBuffer*) _r5.o)->tib->vtable[27])(_r5.o, _r1.i);
    _r0.i = _r0.i + 1;
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_put___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_put___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 552)
    if (_r3.o != _r2.o) goto label8;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 553)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 555)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_Buffer_remaining__(_r3.o);
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_remaining__(_r2.o);
    if (_r0.i <= _r1.i) goto label24;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 556)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 559)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_Buffer_remaining__(_r3.o);
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 560)
    XMLVM_CHECK_NPE(3)
    java_nio_CharBuffer_get___char_1ARRAY(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 561)
    XMLVM_CHECK_NPE(2)
    java_nio_CharBuffer_put___char_1ARRAY(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 562)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_put___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_put___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 597)
    _r0.i = 0;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_CharBuffer_put___java_lang_String_int_int(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_put___java_lang_String_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_put___java_lang_String_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "put", "?")
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
    XMLVM_SOURCE_POSITION("CharBuffer.java", 621)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 622)
    if (_r4.i < 0) goto label10;
    if (_r5.i < _r4.i) goto label10;
    if (_r5.i <= _r0.i) goto label16;
    label10:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 623)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 626)
    _r0.i = _r5.i - _r4.i;
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_remaining__(_r2.o);
    if (_r0.i <= _r1.i) goto label30;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 627)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label30:;
    _r0 = _r4;
    label31:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 629)
    if (_r0.i < _r5.i) goto label34;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 632)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label34:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 630)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r0.i);
    //java_nio_CharBuffer_put___char[27]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_nio_CharBuffer*) _r2.o)->tib->vtable[27])(_r2.o, _r1.i);
    _r0.i = _r0.i + 1;
    goto label31;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_toString__]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 689)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 690)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.position_;
    label7:;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.limit_;
    if (_r1.i < _r2.i) goto label16;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 693)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 691)
    //java_nio_CharBuffer_get___int[21]
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_CharBuffer*) _r3.o)->tib->vtable[21])(_r3.o, _r1.i);
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r2.i);
    _r1.i = _r1.i + 1;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_append___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_append___char]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 709)
    //java_nio_CharBuffer_put___char[27]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_nio_CharBuffer*) _r1.o)->tib->vtable[27])(_r1.o, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_append___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_append___java_lang_CharSequence]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "append", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 730)
    if (_r2.o == JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 731)
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r2.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_CharBuffer_put___java_lang_String(_r1.o, _r0.o);
    label10:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 733)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label11:;
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_CharBuffer_put___java_lang_String(_r1.o, _r0.o);
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_CharBuffer_append___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_append___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "append", "?")
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
    XMLVM_SOURCE_POSITION("CharBuffer.java", 757)
    if (_r3.o != JAVA_NULL) goto label25;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 758)
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label4:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 760)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int])(_r0.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 761)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r0.o);
    if (_r1.i <= 0) goto label23;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 762)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r0.o);
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_CharBuffer_put___java_lang_String(_r2.o, _r0.o);
    label22:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 764)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label23:;
    _r0 = _r2;
    goto label22;
    label25:;
    _r0 = _r3;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_read___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_read___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "read", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    _r1.i = -1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 785)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_nio_Buffer_remaining__(_r2.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 786)
    if (_r3.o != _r2.o) goto label17;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 787)
    if (_r0.i != 0) goto label11;
    _r0 = _r1;
    label10:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 788)
    XMLVM_SOURCE_POSITION("CharBuffer.java", 801)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 790)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 792)
    if (_r0.i != 0) goto label33;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 793)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= 0) goto label31;
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_Buffer_remaining__(_r3.o);
    if (_r0.i != 0) goto label31;
    _r0.i = 0;
    goto label10;
    label31:;
    _r0 = _r1;
    goto label10;
    label33:;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 795)
    XMLVM_CHECK_NPE(3)
    _r1.i = java_nio_Buffer_remaining__(_r3.o);
    _r0.i = java_lang_Math_min___int_int(_r1.i, _r0.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 796)
    if (_r0.i <= 0) goto label10;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 797)
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 798)
    XMLVM_CHECK_NPE(2)
    java_nio_CharBuffer_get___char_1ARRAY(_r2.o, _r1.o);
    XMLVM_SOURCE_POSITION("CharBuffer.java", 799)
    XMLVM_CHECK_NPE(3)
    java_nio_CharBuffer_put___char_1ARRAY(_r3.o, _r1.o);
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_CharBuffer_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_CharBuffer_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.CharBuffer", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharBuffer.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_nio_CharBuffer_compareTo___java_nio_CharBuffer(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

