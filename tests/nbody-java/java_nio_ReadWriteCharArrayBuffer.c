#include "xmlvm.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_lang_System.h"
#include "java_nio_CharBuffer.h"

#include "java_nio_ReadWriteCharArrayBuffer.h"

#define XMLVM_CURRENT_CLASS_NAME ReadWriteCharArrayBuffer
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_ReadWriteCharArrayBuffer

__TIB_DEFINITION_java_nio_ReadWriteCharArrayBuffer __TIB_java_nio_ReadWriteCharArrayBuffer = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_ReadWriteCharArrayBuffer, // classInitializer
    "java.nio.ReadWriteCharArrayBuffer", // className
    "java.nio", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_CharArrayBuffer, // extends
    sizeof(java_nio_ReadWriteCharArrayBuffer), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer;
JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_3ARRAY;
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

void __INIT_java_nio_ReadWriteCharArrayBuffer()
{
    staticInitializerLock(&__TIB_java_nio_ReadWriteCharArrayBuffer);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_ReadWriteCharArrayBuffer.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_ReadWriteCharArrayBuffer.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_ReadWriteCharArrayBuffer);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_ReadWriteCharArrayBuffer.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_ReadWriteCharArrayBuffer.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_ReadWriteCharArrayBuffer.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.ReadWriteCharArrayBuffer")
        __INIT_IMPL_java_nio_ReadWriteCharArrayBuffer();
    }
}

void __INIT_IMPL_java_nio_ReadWriteCharArrayBuffer()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_CharArrayBuffer)
    __TIB_java_nio_ReadWriteCharArrayBuffer.newInstanceFunc = __NEW_INSTANCE_java_nio_ReadWriteCharArrayBuffer;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_ReadWriteCharArrayBuffer.vtable, __TIB_java_nio_CharArrayBuffer.vtable, sizeof(__TIB_java_nio_CharArrayBuffer.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[14] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_asReadOnlyBuffer__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[16] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_compact__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[18] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_duplicate__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[10] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_isReadOnly__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[25] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_protectedArray__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[24] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_protectedArrayOffset__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[26] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_protectedHasArray__;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[27] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_put___char;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[29] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_put___int_char;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[28] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_put___char_1ARRAY_int_int;
    __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[31] = (VTABLE_PTR) &java_nio_ReadWriteCharArrayBuffer_slice__;
    // Initialize interface information
    __TIB_java_nio_ReadWriteCharArrayBuffer.numImplementedInterfaces = 4;
    __TIB_java_nio_ReadWriteCharArrayBuffer.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 4);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Appendable)

    __TIB_java_nio_ReadWriteCharArrayBuffer.implementedInterfaces[0][0] = &__TIB_java_lang_Appendable;

    XMLVM_CLASS_INIT(java_lang_CharSequence)

    __TIB_java_nio_ReadWriteCharArrayBuffer.implementedInterfaces[0][1] = &__TIB_java_lang_CharSequence;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_nio_ReadWriteCharArrayBuffer.implementedInterfaces[0][2] = &__TIB_java_lang_Comparable;

    XMLVM_CLASS_INIT(java_lang_Readable)

    __TIB_java_nio_ReadWriteCharArrayBuffer.implementedInterfaces[0][3] = &__TIB_java_lang_Readable;
    // Initialize itable for this class
    __TIB_java_nio_ReadWriteCharArrayBuffer.itableBegin = &__TIB_java_nio_ReadWriteCharArrayBuffer.itable[0];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___char] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[11];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[12];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence_int_int] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[13];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[15];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[22];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[32];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[5];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[17];
    __TIB_java_nio_ReadWriteCharArrayBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Readable_read___java_nio_CharBuffer] = __TIB_java_nio_ReadWriteCharArrayBuffer.vtable[30];


    __TIB_java_nio_ReadWriteCharArrayBuffer.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_ReadWriteCharArrayBuffer.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_ReadWriteCharArrayBuffer.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_ReadWriteCharArrayBuffer.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_ReadWriteCharArrayBuffer.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_ReadWriteCharArrayBuffer.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_ReadWriteCharArrayBuffer.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_ReadWriteCharArrayBuffer.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_ReadWriteCharArrayBuffer = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_ReadWriteCharArrayBuffer);
    __TIB_java_nio_ReadWriteCharArrayBuffer.clazz = __CLASS_java_nio_ReadWriteCharArrayBuffer;
    __TIB_java_nio_ReadWriteCharArrayBuffer.baseType = JAVA_NULL;
    __CLASS_java_nio_ReadWriteCharArrayBuffer_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ReadWriteCharArrayBuffer);
    __CLASS_java_nio_ReadWriteCharArrayBuffer_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ReadWriteCharArrayBuffer_1ARRAY);
    __CLASS_java_nio_ReadWriteCharArrayBuffer_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ReadWriteCharArrayBuffer_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_ReadWriteCharArrayBuffer]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_ReadWriteCharArrayBuffer.classInitialized = 1;
}

void __DELETE_java_nio_ReadWriteCharArrayBuffer(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_ReadWriteCharArrayBuffer]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_ReadWriteCharArrayBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_CharArrayBuffer(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_ReadWriteCharArrayBuffer]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_ReadWriteCharArrayBuffer()
{    XMLVM_CLASS_INIT(java_nio_ReadWriteCharArrayBuffer)
java_nio_ReadWriteCharArrayBuffer* me = (java_nio_ReadWriteCharArrayBuffer*) XMLVM_MALLOC(sizeof(java_nio_ReadWriteCharArrayBuffer));
    me->tib = &__TIB_java_nio_ReadWriteCharArrayBuffer;
    __INIT_INSTANCE_MEMBERS_java_nio_ReadWriteCharArrayBuffer(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_ReadWriteCharArrayBuffer]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_ReadWriteCharArrayBuffer()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_copy___java_nio_CharArrayBuffer_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_nio_ReadWriteCharArrayBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_copy___java_nio_CharArrayBuffer_int]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "copy", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 34)
    _r0.o = __NEW_java_nio_ReadWriteCharArrayBuffer();
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 35)
    XMLVM_CHECK_NPE(4)
    _r1.i = java_nio_Buffer_capacity__(_r4.o);
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_nio_CharArrayBuffer*) _r4.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_nio_CharArrayBuffer*) _r4.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_CHECK_NPE(0)
    java_nio_ReadWriteCharArrayBuffer___INIT____int_char_1ARRAY_int(_r0.o, _r1.i, _r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 36)
    XMLVM_CHECK_NPE(4)
    _r1.i = java_nio_Buffer_limit__(_r4.o);
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.limit_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 37)
    XMLVM_CHECK_NPE(4)
    _r1.i = java_nio_Buffer_position__(_r4.o);
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.position_ = _r1.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 38)
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.mark_ = _r5.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 39)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_ReadWriteCharArrayBuffer___INIT____char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer___INIT____char_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 43)
    XMLVM_CHECK_NPE(0)
    java_nio_CharArrayBuffer___INIT____char_1ARRAY(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 44)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_ReadWriteCharArrayBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer___INIT____int]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 47)
    XMLVM_CHECK_NPE(0)
    java_nio_CharArrayBuffer___INIT____int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 48)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_ReadWriteCharArrayBuffer___INIT____int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer___INIT____int_char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.i = n1;
    _r2.o = n2;
    _r3.i = n3;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 51)
    XMLVM_CHECK_NPE(0)
    java_nio_CharArrayBuffer___INIT____int_char_1ARRAY_int(_r0.o, _r1.i, _r2.o, _r3.i);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 52)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_asReadOnlyBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_asReadOnlyBuffer__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "asReadOnlyBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 56)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_nio_Buffer*) _r1.o)->fields.java_nio_Buffer.mark_;

    
    // Red class access removed: java.nio.ReadOnlyCharArrayBuffer::copy
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_compact__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_compact__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "compact", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 61)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.offset_;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 62)
    XMLVM_CHECK_NPE(5)
    _r4.i = java_nio_Buffer_remaining__(_r5.o);
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 63)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.limit_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(5)
    ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 64)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.capacity_;
    XMLVM_CHECK_NPE(5)
    ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.limit_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 65)
    _r0.i = -1;
    XMLVM_CHECK_NPE(5)
    ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.mark_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 66)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_duplicate__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_duplicate__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "duplicate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 71)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_nio_Buffer*) _r1.o)->fields.java_nio_Buffer.mark_;
    _r0.o = java_nio_ReadWriteCharArrayBuffer_copy___java_nio_CharArrayBuffer_int(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_ReadWriteCharArrayBuffer_isReadOnly__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_isReadOnly__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "isReadOnly", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 76)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_protectedArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_protectedArray__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "protectedArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 81)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_CharArrayBuffer*) _r1.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_ReadWriteCharArrayBuffer_protectedArrayOffset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_protectedArrayOffset__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "protectedArrayOffset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 86)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_nio_CharArrayBuffer*) _r1.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_ReadWriteCharArrayBuffer_protectedHasArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_protectedHasArray__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "protectedHasArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 91)
    _r0.i = 1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_put___char]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 96)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i != _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 97)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 99)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_nio_CharArrayBuffer*) _r4.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_nio_CharArrayBuffer*) _r4.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_ = _r3.i;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 100)
    XMLVM_EXIT_METHOD()
    return _r4.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_put___int_char]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.i = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 105)
    if (_r3.i < 0) goto label6;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r3.i < _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 106)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 108)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_CharArrayBuffer*) _r2.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_CharArrayBuffer*) _r2.o)->fields.java_nio_CharArrayBuffer.offset_;
    _r1.i = _r1.i + _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 109)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_put___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "put", "?")
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
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 114)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 115)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r8.i;
    _r3.l = (JAVA_LONG) _r7.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 116)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 118)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 119)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 121)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r1.i = _r1.i + _r2.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r6.o, _r7.i, _r0.o, _r1.i, _r8.i);
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 122)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + _r8.i;
    XMLVM_CHECK_NPE(5)
    ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 123)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_slice__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ReadWriteCharArrayBuffer_slice__]
    XMLVM_ENTER_METHOD("java.nio.ReadWriteCharArrayBuffer", "slice", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 128)
    _r0.o = __NEW_java_nio_ReadWriteCharArrayBuffer();
    XMLVM_CHECK_NPE(5)
    _r1.i = java_nio_Buffer_remaining__(_r5.o);
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.backingArray_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_nio_CharArrayBuffer*) _r5.o)->fields.java_nio_CharArrayBuffer.offset_;
    XMLVM_SOURCE_POSITION("ReadWriteCharArrayBuffer.java", 129)
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r3.i = _r3.i + _r4.i;
    XMLVM_CHECK_NPE(0)
    java_nio_ReadWriteCharArrayBuffer___INIT____int_char_1ARRAY_int(_r0.o, _r1.i, _r2.o, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

