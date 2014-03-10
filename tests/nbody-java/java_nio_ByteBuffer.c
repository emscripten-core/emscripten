#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_nio_BufferFactory.h"
#include "java_nio_CharBuffer.h"
#include "org_apache_harmony_luni_platform_Endianness.h"

#include "java_nio_ByteBuffer.h"

#define XMLVM_CURRENT_CLASS_NAME ByteBuffer
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_ByteBuffer

__TIB_DEFINITION_java_nio_ByteBuffer __TIB_java_nio_ByteBuffer = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_ByteBuffer, // classInitializer
    "java.nio.ByteBuffer", // className
    "java.nio", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/nio/Buffer;Ljava/lang/Comparable<Ljava/nio/ByteBuffer;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_Buffer, // extends
    sizeof(java_nio_ByteBuffer), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_ByteBuffer;
JAVA_OBJECT __CLASS_java_nio_ByteBuffer_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_ByteBuffer_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_ByteBuffer_3ARRAY;
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

void __INIT_java_nio_ByteBuffer()
{
    staticInitializerLock(&__TIB_java_nio_ByteBuffer);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_ByteBuffer.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_ByteBuffer.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_ByteBuffer);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_ByteBuffer.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_ByteBuffer.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_ByteBuffer.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.ByteBuffer")
        __INIT_IMPL_java_nio_ByteBuffer();
    }
}

void __INIT_IMPL_java_nio_ByteBuffer()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_Buffer)
    __TIB_java_nio_ByteBuffer.newInstanceFunc = __NEW_INSTANCE_java_nio_ByteBuffer;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_ByteBuffer.vtable, __TIB_java_nio_Buffer.vtable, sizeof(__TIB_java_nio_Buffer.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_ByteBuffer.vtable[7] = (VTABLE_PTR) &java_nio_ByteBuffer_array__;
    __TIB_java_nio_ByteBuffer.vtable[6] = (VTABLE_PTR) &java_nio_ByteBuffer_arrayOffset__;
    __TIB_java_nio_ByteBuffer.vtable[1] = (VTABLE_PTR) &java_nio_ByteBuffer_equals___java_lang_Object;
    __TIB_java_nio_ByteBuffer.vtable[34] = (VTABLE_PTR) &java_nio_ByteBuffer_get___byte_1ARRAY_int_int;
    __TIB_java_nio_ByteBuffer.vtable[8] = (VTABLE_PTR) &java_nio_ByteBuffer_hasArray__;
    __TIB_java_nio_ByteBuffer.vtable[4] = (VTABLE_PTR) &java_nio_ByteBuffer_hashCode__;
    __TIB_java_nio_ByteBuffer.vtable[52] = (VTABLE_PTR) &java_nio_ByteBuffer_put___byte_1ARRAY_int_int;
    __TIB_java_nio_ByteBuffer.vtable[5] = (VTABLE_PTR) &java_nio_ByteBuffer_toString__;
    __TIB_java_nio_ByteBuffer.vtable[19] = (VTABLE_PTR) &java_nio_ByteBuffer_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_nio_ByteBuffer.numImplementedInterfaces = 1;
    __TIB_java_nio_ByteBuffer.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_nio_ByteBuffer.implementedInterfaces[0][0] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_nio_ByteBuffer.itableBegin = &__TIB_java_nio_ByteBuffer.itable[0];
    __TIB_java_nio_ByteBuffer.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_nio_ByteBuffer.vtable[19];


    __TIB_java_nio_ByteBuffer.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_ByteBuffer.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_ByteBuffer.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_ByteBuffer.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_ByteBuffer.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_ByteBuffer.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_ByteBuffer.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_ByteBuffer.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_ByteBuffer = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_ByteBuffer);
    __TIB_java_nio_ByteBuffer.clazz = __CLASS_java_nio_ByteBuffer;
    __TIB_java_nio_ByteBuffer.baseType = JAVA_NULL;
    __CLASS_java_nio_ByteBuffer_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ByteBuffer);
    __CLASS_java_nio_ByteBuffer_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ByteBuffer_1ARRAY);
    __CLASS_java_nio_ByteBuffer_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_ByteBuffer_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_ByteBuffer]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_ByteBuffer.classInitialized = 1;
}

void __DELETE_java_nio_ByteBuffer(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_ByteBuffer]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_ByteBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_Buffer(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_ByteBuffer*) me)->fields.java_nio_ByteBuffer.order_ = (org_apache_harmony_luni_platform_Endianness*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_ByteBuffer]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_ByteBuffer()
{    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
java_nio_ByteBuffer* me = (java_nio_ByteBuffer*) XMLVM_MALLOC(sizeof(java_nio_ByteBuffer));
    me->tib = &__TIB_java_nio_ByteBuffer;
    __INIT_INSTANCE_MEMBERS_java_nio_ByteBuffer(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_ByteBuffer]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_ByteBuffer()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_nio_ByteBuffer_allocate___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_allocate___int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "allocate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 49)
    if (_r1.i >= 0) goto label8;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 50)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 52)
    _r0.o = java_nio_BufferFactory_newByteBuffer___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_allocateDirect___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_allocateDirect___int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "allocateDirect", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 65)
    if (_r1.i >= 0) goto label8;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 66)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 68)
    _r0.o = java_nio_BufferFactory_newDirectByteBuffer___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_wrap___byte_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_wrap___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "wrap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 82)
    _r0.o = java_nio_BufferFactory_newByteBuffer___byte_1ARRAY(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_wrap___byte_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_wrap___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "wrap", "?")
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
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 104)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 105)
    if (_r6.i < 0) goto label13;
    if (_r7.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r6.i;
    _r3.l = (JAVA_LONG) _r7.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 106)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 109)
    _r0.o = java_nio_BufferFactory_newByteBuffer___byte_1ARRAY(_r5.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 110)
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.position_ = _r6.i;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 111)
    _r1.i = _r6.i + _r7.i;
    XMLVM_CHECK_NPE(0)
    ((java_nio_Buffer*) _r0.o)->fields.java_nio_Buffer.limit_ = _r1.i;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 113)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_ByteBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer___INIT____int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 128)
    XMLVM_CHECK_NPE(1)
    java_nio_Buffer___INIT____int(_r1.o, _r2.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 119)
    _r0.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    XMLVM_CHECK_NPE(1)
    ((java_nio_ByteBuffer*) _r1.o)->fields.java_nio_ByteBuffer.order_ = _r0.o;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 129)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_array__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_array__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "array", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 141)
    //java_nio_ByteBuffer_protectedArray__[37]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r1.o)->tib->vtable[37])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_ByteBuffer_arrayOffset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_arrayOffset__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "arrayOffset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 158)
    //java_nio_ByteBuffer_protectedArrayOffset__[36]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r1.o)->tib->vtable[36])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_ByteBuffer_compareTo___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_compareTo___java_nio_ByteBuffer]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "compareTo", "?")
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
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 306)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    if (_r0.i >= _r1.i) goto label33;
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    label14:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 308)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 309)
    XMLVM_CHECK_NPE(7)
    _r2.i = ((java_nio_Buffer*) _r7.o)->fields.java_nio_Buffer.position_;
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    label21:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 311)
    if (_r2.i > 0) goto label38;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 321)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    _r0.i = _r0.i - _r1.i;
    label32:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label33:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 307)
    XMLVM_CHECK_NPE(7)
    _r0.i = java_nio_Buffer_remaining__(_r7.o);
    goto label14;
    label38:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 312)
    //java_nio_ByteBuffer_get___int[35]
    XMLVM_CHECK_NPE(6)
    _r3.i = (*(JAVA_BYTE (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_ByteBuffer*) _r6.o)->tib->vtable[35])(_r6.o, _r1.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 313)
    //java_nio_ByteBuffer_get___int[35]
    XMLVM_CHECK_NPE(7)
    _r4.i = (*(JAVA_BYTE (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_ByteBuffer*) _r7.o)->tib->vtable[35])(_r7.o, _r0.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 314)
    if (_r3.i == _r4.i) goto label54;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 315)
    if (_r3.i >= _r4.i) goto label52;
    _r0.i = -1;
    goto label32;
    label52:;
    _r0.i = 1;
    goto label32;
    label54:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 317)
    _r1.i = _r1.i + 1;
    _r0.i = _r0.i + 1;
    _r2.i = _r2.i + -1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 319)
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_ByteBuffer_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "equals", "?")
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
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 353)
    XMLVM_CLASS_INIT(java_nio_ByteBuffer)
    _r0.i = XMLVM_ISA(_r7.o, __CLASS_java_nio_ByteBuffer);
    if (_r0.i != 0) goto label8;
    _r0 = _r4;
    label7:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 354)
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 369)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 356)
    _r7.o = _r7.o;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 358)
    XMLVM_CHECK_NPE(6)
    _r0.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(7)
    _r1.i = java_nio_Buffer_remaining__(_r7.o);
    if (_r0.i == _r1.i) goto label22;
    _r0 = _r4;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 359)
    goto label7;
    label22:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 362)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 363)
    XMLVM_CHECK_NPE(7)
    _r1.i = ((java_nio_Buffer*) _r7.o)->fields.java_nio_Buffer.position_;
    _r2 = _r0;
    _r0 = _r5;
    label28:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 365)
    if (_r0.i == 0) goto label7;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_nio_Buffer*) _r6.o)->fields.java_nio_Buffer.limit_;
    if (_r2.i >= _r3.i) goto label7;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 366)
    _r0.i = _r2.i + 1;
    //java_nio_ByteBuffer_get___int[35]
    XMLVM_CHECK_NPE(6)
    _r2.i = (*(JAVA_BYTE (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_ByteBuffer*) _r6.o)->tib->vtable[35])(_r6.o, _r2.i);
    _r3.i = _r1.i + 1;
    //java_nio_ByteBuffer_get___int[35]
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_BYTE (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_ByteBuffer*) _r7.o)->tib->vtable[35])(_r7.o, _r1.i);
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

JAVA_OBJECT java_nio_ByteBuffer_get___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_get___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 395)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_nio_ByteBuffer_get___byte_1ARRAY_int_int[34]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_nio_ByteBuffer*) _r2.o)->tib->vtable[34])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_get___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_get___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "get", "?")
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
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 418)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 419)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r7.i;
    _r3.l = (JAVA_LONG) _r8.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 420)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 423)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 424)

    
    // Red class access removed: java.nio.BufferUnderflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferUnderflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    _r0 = _r7;
    label32:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 426)
    _r1.i = _r7.i + _r8.i;
    if (_r0.i < _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 429)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    label37:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 427)
    //java_nio_ByteBuffer_get__[33]
    XMLVM_CHECK_NPE(5)
    _r1.i = (*(JAVA_BYTE (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r5.o)->tib->vtable[33])(_r5.o);
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    _r0.i = _r0.i + 1;
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_ByteBuffer_hasArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_hasArray__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "hasArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 621)
    //java_nio_ByteBuffer_protectedHasArray__[38]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r1.o)->tib->vtable[38])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_ByteBuffer_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_hashCode__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 632)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.position_;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 633)
    _r1.i = 0;
    _r3 = _r1;
    _r1 = _r0;
    _r0 = _r3;
    label6:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 634)
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_nio_Buffer*) _r4.o)->fields.java_nio_Buffer.limit_;
    if (_r1.i < _r2.i) goto label11;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 637)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 635)
    _r2.i = _r1.i + 1;
    //java_nio_ByteBuffer_get___int[35]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BYTE (*)(JAVA_OBJECT, JAVA_INT)) ((java_nio_ByteBuffer*) _r4.o)->tib->vtable[35])(_r4.o, _r1.i);
    _r0.i = _r0.i + _r1.i;
    _r1 = _r2;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_order__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_order__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "order", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 658)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_ByteBuffer*) _r2.o)->fields.java_nio_ByteBuffer.order_;
    _r1.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    if (_r0.o != _r1.o) goto label9;

    
    // Red class access removed: java.nio.ByteOrder,java.nio.ByteOrder::BIG_ENDIAN
    XMLVM_RED_CLASS_DEPENDENCY();
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label9:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 659)

    
    // Red class access removed: java.nio.ByteOrder,java.nio.ByteOrder::LITTLE_ENDIAN
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_order___java_nio_ByteOrder(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_order___java_nio_ByteOrder]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "order", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 672)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_ByteBuffer_orderImpl___java_nio_ByteOrder(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_orderImpl___java_nio_ByteOrder(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_orderImpl___java_nio_ByteOrder]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "orderImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 676)

    
    // Red class access removed: java.nio.ByteOrder,java.nio.ByteOrder::BIG_ENDIAN
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r2.o != _r0.o) goto label9;
    _r0.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    label6:;
    XMLVM_CHECK_NPE(1)
    ((java_nio_ByteBuffer*) _r1.o)->fields.java_nio_ByteBuffer.order_ = _r0.o;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 678)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label9:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 677)
    _r0.o = org_apache_harmony_luni_platform_Endianness_GET_LITTLE_ENDIAN();
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_put___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_put___byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 732)
    _r0.i = 0;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    //java_nio_ByteBuffer_put___byte_1ARRAY_int_int[52]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_INT, JAVA_INT)) ((java_nio_ByteBuffer*) _r2.o)->tib->vtable[52])(_r2.o, _r3.o, _r0.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_put___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_put___byte_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "put", "?")
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
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 757)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 758)
    if (_r7.i < 0) goto label13;
    if (_r8.i < 0) goto label13;
    _r1.l = (JAVA_LONG) _r7.i;
    _r3.l = (JAVA_LONG) _r8.i;
    _r1.l = _r1.l + _r3.l;
    _r3.l = (JAVA_LONG) _r0.i;
    _r0.i = _r1.l > _r3.l ? 1 : (_r1.l == _r3.l ? 0 : -1);
    if (_r0.i <= 0) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 759)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 762)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_nio_Buffer_remaining__(_r5.o);
    if (_r8.i <= _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 763)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label31:;
    _r0 = _r7;
    label32:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 765)
    _r1.i = _r7.i + _r8.i;
    if (_r0.i < _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 768)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    label37:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 766)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r5.o)->tib->vtable[51])(_r5.o, _r1.i);
    _r0.i = _r0.i + 1;
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_put___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_put___java_nio_ByteBuffer]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "put", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 788)
    if (_r3.o != _r2.o) goto label8;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 789)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 791)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_Buffer_remaining__(_r3.o);
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_remaining__(_r2.o);
    if (_r0.i <= _r1.i) goto label24;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 792)

    
    // Red class access removed: java.nio.BufferOverflowException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.BufferOverflowException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 794)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_Buffer_remaining__(_r3.o);
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 795)
    XMLVM_CHECK_NPE(3)
    java_nio_ByteBuffer_get___byte_1ARRAY(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 796)
    XMLVM_CHECK_NPE(2)
    java_nio_ByteBuffer_put___byte_1ARRAY(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 797)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_ByteBuffer_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_toString__]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1050)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1051)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[3])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_Class_getName__(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1052)
    // ", status: capacity="
    _r1.o = xmlvm_create_java_string_from_pool(719);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1053)
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_capacity__(_r2.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1054)
    // " position="
    _r1.o = xmlvm_create_java_string_from_pool(720);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1055)
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_position__(_r2.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1056)
    // " limit="
    _r1.o = xmlvm_create_java_string_from_pool(721);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1057)
    XMLVM_CHECK_NPE(2)
    _r1.i = java_nio_Buffer_limit__(_r2.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___int(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1058)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_ByteBuffer_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_ByteBuffer_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.ByteBuffer", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ByteBuffer.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_nio_ByteBuffer_compareTo___java_nio_ByteBuffer(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

