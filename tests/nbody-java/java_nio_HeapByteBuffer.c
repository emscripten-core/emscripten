#include "xmlvm.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_lang_System.h"
#include "java_nio_CharBuffer.h"
#include "org_apache_harmony_luni_platform_Endianness.h"

#include "java_nio_HeapByteBuffer.h"

#define XMLVM_CURRENT_CLASS_NAME HeapByteBuffer
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_HeapByteBuffer

__TIB_DEFINITION_java_nio_HeapByteBuffer __TIB_java_nio_HeapByteBuffer = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_HeapByteBuffer, // classInitializer
    "java.nio.HeapByteBuffer", // className
    "java.nio", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_ByteBuffer, // extends
    sizeof(java_nio_HeapByteBuffer), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer;
JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_3ARRAY;
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

void __INIT_java_nio_HeapByteBuffer()
{
    staticInitializerLock(&__TIB_java_nio_HeapByteBuffer);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_HeapByteBuffer.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_HeapByteBuffer.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_HeapByteBuffer);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_HeapByteBuffer.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_HeapByteBuffer.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_HeapByteBuffer.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.HeapByteBuffer")
        __INIT_IMPL_java_nio_HeapByteBuffer();
    }
}

void __INIT_IMPL_java_nio_HeapByteBuffer()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    __TIB_java_nio_HeapByteBuffer.newInstanceFunc = __NEW_INSTANCE_java_nio_HeapByteBuffer;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_HeapByteBuffer.vtable, __TIB_java_nio_ByteBuffer.vtable, sizeof(__TIB_java_nio_ByteBuffer.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_HeapByteBuffer.vtable[34] = (VTABLE_PTR) &java_nio_HeapByteBuffer_get___byte_1ARRAY_int_int;
    __TIB_java_nio_HeapByteBuffer.vtable[33] = (VTABLE_PTR) &java_nio_HeapByteBuffer_get__;
    __TIB_java_nio_HeapByteBuffer.vtable[35] = (VTABLE_PTR) &java_nio_HeapByteBuffer_get___int;
    __TIB_java_nio_HeapByteBuffer.vtable[23] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getDouble__;
    __TIB_java_nio_HeapByteBuffer.vtable[24] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getDouble___int;
    __TIB_java_nio_HeapByteBuffer.vtable[25] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getFloat__;
    __TIB_java_nio_HeapByteBuffer.vtable[26] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getFloat___int;
    __TIB_java_nio_HeapByteBuffer.vtable[27] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getInt__;
    __TIB_java_nio_HeapByteBuffer.vtable[28] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getInt___int;
    __TIB_java_nio_HeapByteBuffer.vtable[29] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getLong__;
    __TIB_java_nio_HeapByteBuffer.vtable[30] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getLong___int;
    __TIB_java_nio_HeapByteBuffer.vtable[31] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getShort__;
    __TIB_java_nio_HeapByteBuffer.vtable[32] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getShort___int;
    __TIB_java_nio_HeapByteBuffer.vtable[9] = (VTABLE_PTR) &java_nio_HeapByteBuffer_isDirect__;
    __TIB_java_nio_HeapByteBuffer.vtable[11] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asCharBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[12] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asDoubleBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[13] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asFloatBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[14] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asIntBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[15] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asLongBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[17] = (VTABLE_PTR) &java_nio_HeapByteBuffer_asShortBuffer__;
    __TIB_java_nio_HeapByteBuffer.vtable[21] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getChar__;
    __TIB_java_nio_HeapByteBuffer.vtable[22] = (VTABLE_PTR) &java_nio_HeapByteBuffer_getChar___int;
    __TIB_java_nio_HeapByteBuffer.vtable[39] = (VTABLE_PTR) &java_nio_HeapByteBuffer_putChar___char;
    __TIB_java_nio_HeapByteBuffer.vtable[40] = (VTABLE_PTR) &java_nio_HeapByteBuffer_putChar___int_char;
    // Initialize interface information
    __TIB_java_nio_HeapByteBuffer.numImplementedInterfaces = 1;
    __TIB_java_nio_HeapByteBuffer.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_nio_HeapByteBuffer.implementedInterfaces[0][0] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_nio_HeapByteBuffer.itableBegin = &__TIB_java_nio_HeapByteBuffer.itable[0];
    __TIB_java_nio_HeapByteBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_nio_HeapByteBuffer.vtable[19];


    __TIB_java_nio_HeapByteBuffer.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_HeapByteBuffer.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_HeapByteBuffer.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_HeapByteBuffer.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_HeapByteBuffer.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_HeapByteBuffer.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_HeapByteBuffer.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_HeapByteBuffer.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_HeapByteBuffer = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_HeapByteBuffer);
    __TIB_java_nio_HeapByteBuffer.clazz = __CLASS_java_nio_HeapByteBuffer;
    __TIB_java_nio_HeapByteBuffer.baseType = JAVA_NULL;
    __CLASS_java_nio_HeapByteBuffer_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_HeapByteBuffer);
    __CLASS_java_nio_HeapByteBuffer_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_HeapByteBuffer_1ARRAY);
    __CLASS_java_nio_HeapByteBuffer_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_HeapByteBuffer_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_HeapByteBuffer]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_HeapByteBuffer.classInitialized = 1;
}

void __DELETE_java_nio_HeapByteBuffer(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_HeapByteBuffer]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_HeapByteBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_ByteBuffer(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_HeapByteBuffer*) me)->fields.java_nio_HeapByteBuffer.backingArray_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_nio_HeapByteBuffer*) me)->fields.java_nio_HeapByteBuffer.offset_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_HeapByteBuffer]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_HeapByteBuffer()
{    XMLVM_CLASS_INIT(java_nio_HeapByteBuffer)
java_nio_HeapByteBuffer* me = (java_nio_HeapByteBuffer*) XMLVM_MALLOC(sizeof(java_nio_HeapByteBuffer));
    me->tib = &__TIB_java_nio_HeapByteBuffer;
    __INIT_INSTANCE_MEMBERS_java_nio_HeapByteBuffer(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_HeapByteBuffer]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_HeapByteBuffer()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_nio_HeapByteBuffer___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer___INIT____byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 41)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    java_nio_HeapByteBuffer___INIT____byte_1ARRAY_int_int(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 42)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_HeapByteBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer___INIT____int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 45)
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r3.i);
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    java_nio_HeapByteBuffer___INIT____byte_1ARRAY_int_int(_r2.o, _r0.o, _r3.i, _r1.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 46)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_HeapByteBuffer___INIT____byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer___INIT____byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "<init>", "?")
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
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 49)
    XMLVM_CHECK_NPE(2)
    java_nio_ByteBuffer___INIT____int(_r2.o, _r4.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 50)
    XMLVM_CHECK_NPE(2)
    ((java_nio_HeapByteBuffer*) _r2.o)->fields.java_nio_HeapByteBuffer.backingArray_ = _r3.o;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 51)
    XMLVM_CHECK_NPE(2)
    ((java_nio_HeapByteBuffer*) _r2.o)->fields.java_nio_HeapByteBuffer.offset_ = _r5.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 53)
    _r0.i = _r5.i + _r4.i;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r0.i <= _r1.i) goto label18;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 54)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label18:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 56)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_get___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_get___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "get", "?")
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
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 67)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 68)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r7.i;
    _r3.l = (JAVA_LONG) _r8.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 69)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 71)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 72)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 74)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_nio_HeapByteBuffer*) _r5.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_nio_HeapByteBuffer*) _r5.o)->fields.java_nio_HeapByteBuffer.offset_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r1.i = _r1.i + _r2.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r6.o, _r7.i, _r8.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 75)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + _r8.i;
    XMLVM_CHECK_NPE(5)
    ((java_nio_Buffer*) _r5.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 76)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_nio_HeapByteBuffer_get__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_get__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 81)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i != _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 82)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 84)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.offset_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_ = _r3.i;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_nio_HeapByteBuffer_get___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_get___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 89)
    if (_r3.i < 0) goto label6;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r3.i < _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 90)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 92)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_HeapByteBuffer*) _r2.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_HeapByteBuffer*) _r2.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r1.i = _r1.i + _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_nio_HeapByteBuffer_getDouble__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getDouble__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getDouble", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 97)
    //java_nio_HeapByteBuffer_getLong__[29]
    XMLVM_CHECK_NPE(2)
    _r0.l = (*(JAVA_LONG (*)(JAVA_OBJECT)) ((java_nio_HeapByteBuffer*) _r2.o)->tib->vtable[29])(_r2.o);
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_nio_HeapByteBuffer_getDouble___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getDouble___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getDouble", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 102)
    //java_nio_HeapByteBuffer_getLong___int[30]
    XMLVM_CHECK_NPE(2)
    _r0.l = (*(JAVA_LONG (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_HeapByteBuffer*) _r2.o)->tib->vtable[30])(_r2.o, _r3.i);
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_nio_HeapByteBuffer_getFloat__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getFloat__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getFloat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 107)
    //java_nio_HeapByteBuffer_getInt__[27]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[27])(_r1.o);
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_nio_HeapByteBuffer_getFloat___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getFloat___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getFloat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 112)
    //java_nio_HeapByteBuffer_getInt___int[28]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[28])(_r1.o, _r2.i);
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_HeapByteBuffer_getInt__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getInt__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 117)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + 4;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 118)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 119)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 121)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_HeapByteBuffer_loadInt___int(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 122)
    XMLVM_CHECK_NPE(2)
    ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 123)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_HeapByteBuffer_getInt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getInt___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 128)
    if (_r3.i < 0) goto label8;
    _r0.i = _r3.i + 4;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 129)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 131)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_nio_HeapByteBuffer_loadInt___int(_r2.o, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_nio_HeapByteBuffer_getLong__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getLong__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 136)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + 8;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 137)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 138)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 140)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(3)
    _r1.l = java_nio_HeapByteBuffer_loadLong___int(_r3.o, _r1.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 141)
    XMLVM_CHECK_NPE(3)
    ((java_nio_Buffer*) _r3.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 142)
    XMLVM_EXIT_METHOD()
    return _r1.l;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_nio_HeapByteBuffer_getLong___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getLong___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 147)
    if (_r3.i < 0) goto label8;
    _r0.i = _r3.i + 8;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 148)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 150)
    XMLVM_CHECK_NPE(2)
    _r0.l = java_nio_HeapByteBuffer_loadLong___int(_r2.o, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_nio_HeapByteBuffer_getShort__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getShort__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 155)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_;
    _r0.i = _r0.i + 2;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 156)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 157)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 159)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_;
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_HeapByteBuffer_loadShort___int(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 160)
    XMLVM_CHECK_NPE(2)
    ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 161)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_nio_HeapByteBuffer_getShort___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getShort___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 166)
    if (_r3.i < 0) goto label8;
    _r0.i = _r3.i + 2;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_nio_Buffer*) _r2.o)->fields.java_nio_Buffer.limit_;
    if (_r0.i <= _r1.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 167)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 169)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_nio_HeapByteBuffer_loadShort___int(_r2.o, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_HeapByteBuffer_isDirect__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_isDirect__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "isDirect", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 174)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_HeapByteBuffer_loadInt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_loadInt___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "loadInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.i = n1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 178)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_nio_HeapByteBuffer*) _r5.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r6.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 180)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_nio_ByteBuffer*) _r5.o)->fields.java_nio_ByteBuffer.order_;
    _r2.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r1.o != _r2.o) goto label31;
    _r1 = _r3;
    _r2 = _r3;
    label12:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 181)
    _r3.i = 4;
    if (_r1.i < _r3.i) goto label17;
    _r0 = _r2;
    label16:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 191)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label17:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 182)
    _r2.i = _r2.i << 8;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 183)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_nio_HeapByteBuffer*) _r5.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r4.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r3.i = _r3.i & 255;
    _r2.i = _r2.i | _r3.i;
    _r1.i = _r1.i + 1;
    goto label12;
    label31:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 186)
    _r1.i = 3;
    _r2 = _r3;
    label33:;
    if (_r1.i >= 0) goto label37;
    _r0 = _r2;
    goto label16;
    label37:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 187)
    _r2.i = _r2.i << 8;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 188)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_nio_HeapByteBuffer*) _r5.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r4.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r3.i = _r3.i & 255;
    _r2.i = _r2.i | _r3.i;
    _r1.i = _r1.i + -1;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_nio_HeapByteBuffer_loadLong___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_loadLong___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "loadLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    _r10.o = me;
    _r11.i = n1;
    _r6.i = 8;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 195)
    XMLVM_CHECK_NPE(10)
    _r0.i = ((java_nio_HeapByteBuffer*) _r10.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r11.i;
    _r1.l = 0;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 197)
    XMLVM_CHECK_NPE(10)
    _r3.o = ((java_nio_ByteBuffer*) _r10.o)->fields.java_nio_ByteBuffer.order_;
    _r4.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r3.o != _r4.o) goto label36;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 198)
    _r3.i = 0;
    _r7 = _r3;
    _r8 = _r1;
    _r2 = _r8;
    _r1 = _r7;
    label18:;
    if (_r1.i < _r6.i) goto label22;
    _r0 = _r2;
    label21:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 208)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label22:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 199)
    _r2.l = _r2.l << (0x3f & _r6.l);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 200)
    XMLVM_CHECK_NPE(10)
    _r4.o = ((java_nio_HeapByteBuffer*) _r10.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r5.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r4.i = _r4.i & 255;
    _r4.l = (JAVA_LONG) _r4.i;
    _r2.l = _r2.l | _r4.l;
    _r1.i = _r1.i + 1;
    goto label18;
    label36:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 203)
    _r3.i = 7;
    _r7 = _r3;
    _r8 = _r1;
    _r2 = _r8;
    _r1 = _r7;
    label41:;
    if (_r1.i >= 0) goto label45;
    _r0 = _r2;
    goto label21;
    label45:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 204)
    _r2.l = _r2.l << (0x3f & _r6.l);
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 205)
    XMLVM_CHECK_NPE(10)
    _r4.o = ((java_nio_HeapByteBuffer*) _r10.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r5.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r4.i = _r4.i & 255;
    _r4.l = (JAVA_LONG) _r4.i;
    _r2.l = _r2.l | _r4.l;
    _r1.i = _r1.i + -1;
    goto label41;
    //XMLVM_END_WRAPPER
}

JAVA_SHORT java_nio_HeapByteBuffer_loadShort___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_loadShort___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "loadShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 212)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_nio_HeapByteBuffer*) _r3.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r4.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 214)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_ByteBuffer*) _r3.o)->fields.java_nio_ByteBuffer.order_;
    _r2.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r1.o != _r2.o) goto label27;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 215)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_HeapByteBuffer*) _r3.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r1.i = _r1.i << 8;
    _r1.i = (_r1.i << 16) >> 16;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 216)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_nio_HeapByteBuffer*) _r3.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r0.i = _r0.i & 255;
    _r0.i = _r0.i | _r1.i;
    _r0.i = (_r0.i << 16) >> 16;
    label26:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 221)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label27:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 218)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_HeapByteBuffer*) _r3.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r2.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r1.i = _r1.i << 8;
    _r1.i = (_r1.i << 16) >> 16;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 219)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_nio_HeapByteBuffer*) _r3.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r0.i = _r0.i & 255;
    _r0.i = _r0.i | _r1.i;
    _r0.i = (_r0.i << 16) >> 16;
    goto label26;
    //XMLVM_END_WRAPPER
}

void java_nio_HeapByteBuffer_store___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_store___int_int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "store", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    _r7.o = me;
    _r8.i = n1;
    _r9.i = n2;
    _r6.i = 3;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 225)
    XMLVM_CHECK_NPE(7)
    _r0.i = ((java_nio_HeapByteBuffer*) _r7.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r8.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 226)
    XMLVM_CHECK_NPE(7)
    _r1.o = ((java_nio_ByteBuffer*) _r7.o)->fields.java_nio_ByteBuffer.order_;
    _r2.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r1.o != _r2.o) goto label29;
    _r1 = _r6;
    _r2 = _r9;
    label12:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 227)
    if (_r1.i >= 0) goto label15;
    label14:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 237)
    XMLVM_EXIT_METHOD()
    return;
    label15:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 228)
    XMLVM_CHECK_NPE(7)
    _r3.o = ((java_nio_HeapByteBuffer*) _r7.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r4.i = _r0.i + _r1.i;
    _r5.i = _r2.i & 255;
    _r5.i = (_r5.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 229)
    _r2.i = _r2.i >> 8;
    _r1.i = _r1.i + -1;
    goto label12;
    label29:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 232)
    _r1.i = 0;
    _r2 = _r9;
    label31:;
    if (_r1.i > _r6.i) goto label14;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 233)
    XMLVM_CHECK_NPE(7)
    _r3.o = ((java_nio_HeapByteBuffer*) _r7.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r4.i = _r0.i + _r1.i;
    _r5.i = _r2.i & 255;
    _r5.i = (_r5.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 234)
    _r2.i = _r2.i >> 8;
    _r1.i = _r1.i + 1;
    goto label31;
    //XMLVM_END_WRAPPER
}

void java_nio_HeapByteBuffer_store___int_long(JAVA_OBJECT me, JAVA_INT n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_store___int_long]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "store", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    XMLVMElem _r15;
    _r12.o = me;
    _r13.i = n1;
    _r14.l = n2;
    _r10.l = 255;
    _r9.i = 8;
    _r8.i = 7;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 240)
    XMLVM_CHECK_NPE(12)
    _r0.i = ((java_nio_HeapByteBuffer*) _r12.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r13.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 241)
    XMLVM_CHECK_NPE(12)
    _r1.o = ((java_nio_ByteBuffer*) _r12.o)->fields.java_nio_ByteBuffer.order_;
    _r2.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r1.o != _r2.o) goto label33;
    _r1 = _r8;
    _r2 = _r14;
    label16:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 242)
    if (_r1.i >= 0) goto label19;
    label18:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 252)
    XMLVM_EXIT_METHOD()
    return;
    label19:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 243)
    XMLVM_CHECK_NPE(12)
    _r4.o = ((java_nio_HeapByteBuffer*) _r12.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r5.i = _r0.i + _r1.i;
    _r6.l = _r2.l & _r10.l;
    _r6.i = (JAVA_INT) _r6.l;
    _r6.i = (_r6.i << 24) >> 24;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r6.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 244)
    _r2.l = _r2.l >> (0x3f & _r9.l);
    _r1.i = _r1.i + -1;
    goto label16;
    label33:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 247)
    _r1.i = 0;
    _r2 = _r14;
    label35:;
    if (_r1.i > _r8.i) goto label18;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 248)
    XMLVM_CHECK_NPE(12)
    _r4.o = ((java_nio_HeapByteBuffer*) _r12.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r5.i = _r0.i + _r1.i;
    _r6.l = _r2.l & _r10.l;
    _r6.i = (JAVA_INT) _r6.l;
    _r6.i = (_r6.i << 24) >> 24;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r6.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 249)
    _r2.l = _r2.l >> (0x3f & _r9.l);
    _r1.i = _r1.i + 1;
    goto label35;
    //XMLVM_END_WRAPPER
}

void java_nio_HeapByteBuffer_store___int_short(JAVA_OBJECT me, JAVA_INT n1, JAVA_SHORT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_store___int_short]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "store", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.i = n2;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 255)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.offset_;
    _r0.i = _r0.i + _r5.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 256)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_ByteBuffer*) _r4.o)->fields.java_nio_ByteBuffer.order_;
    _r2.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r1.o != _r2.o) goto label28;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 257)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r2.i = _r6.i >> 8;
    _r2.i = _r2.i & 255;
    _r2.i = (_r2.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 258)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r0.i = _r0.i + 1;
    _r2.i = _r6.i & 255;
    _r2.i = (_r2.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    label27:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 263)
    XMLVM_EXIT_METHOD()
    return;
    label28:;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 260)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r2.i = _r0.i + 1;
    _r3.i = _r6.i >> 8;
    _r3.i = _r3.i & 255;
    _r3.i = (_r3.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 261)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_HeapByteBuffer*) _r4.o)->fields.java_nio_HeapByteBuffer.backingArray_;
    _r2.i = _r6.i & 255;
    _r2.i = (_r2.i << 24) >> 24;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.i;
    goto label27;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asCharBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asCharBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asCharBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 267)

    
    // Red class access removed: java.nio.CharToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asDoubleBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asDoubleBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asDoubleBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 272)

    
    // Red class access removed: java.nio.DoubleToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asFloatBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asFloatBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asFloatBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 277)

    
    // Red class access removed: java.nio.FloatToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asIntBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asIntBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asIntBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 282)

    
    // Red class access removed: java.nio.IntToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asLongBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asLongBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asLongBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 287)

    
    // Red class access removed: java.nio.LongToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_asShortBuffer__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_asShortBuffer__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "asShortBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 292)

    
    // Red class access removed: java.nio.ShortToByteBufferAdapter::wrap
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_nio_HeapByteBuffer_getChar__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getChar__]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 297)
    //java_nio_HeapByteBuffer_getShort__[31]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_SHORT (*)(JAVA_OBJECT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[31])(_r1.o);
    _r0.i = _r0.i & 0xffff;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_nio_HeapByteBuffer_getChar___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_getChar___int]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "getChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 302)
    //java_nio_HeapByteBuffer_getShort___int[32]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_SHORT (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[32])(_r1.o, _r2.i);
    _r0.i = _r0.i & 0xffff;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_putChar___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_putChar___char]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "putChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 307)
    _r0.i = (_r2.i << 16) >> 16;
    //java_nio_HeapByteBuffer_putShort___short[50]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_SHORT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[50])(_r1.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_HeapByteBuffer_putChar___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_HeapByteBuffer_putChar___int_char]
    XMLVM_ENTER_METHOD("java.nio.HeapByteBuffer", "putChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("HeapByteBuffer.java", 312)
    _r0.i = (_r3.i << 16) >> 16;
    //java_nio_HeapByteBuffer_putShort___int_short[49]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_SHORT)) ((java_nio_HeapByteBuffer*) _r1.o)->tib->vtable[49])(_r1.o, _r2.i, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

