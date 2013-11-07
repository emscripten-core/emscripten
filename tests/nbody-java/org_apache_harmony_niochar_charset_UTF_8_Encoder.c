#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Object.h"
#include "java_nio_ByteBuffer.h"
#include "java_nio_CharBuffer.h"
#include "java_nio_charset_Charset.h"
#include "java_nio_charset_CoderResult.h"
#include "org_apache_harmony_niochar_charset_UTF_8.h"

#include "org_apache_harmony_niochar_charset_UTF_8_Encoder.h"

#define XMLVM_CURRENT_CLASS_NAME UTF_8_Encoder
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_niochar_charset_UTF_8_Encoder

__TIB_DEFINITION_org_apache_harmony_niochar_charset_UTF_8_Encoder __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_niochar_charset_UTF_8_Encoder, // classInitializer
    "org.apache.harmony.niochar.charset.UTF_8$Encoder", // className
    "org.apache.harmony.niochar.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_charset_CharsetEncoder, // extends
    sizeof(org_apache_harmony_niochar_charset_UTF_8_Encoder), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_3ARRAY;
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

void __INIT_org_apache_harmony_niochar_charset_UTF_8_Encoder()
{
    staticInitializerLock(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.niochar.charset.UTF_8$Encoder")
        __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8_Encoder();
    }
}

void __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8_Encoder()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_charset_CharsetEncoder)
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8_Encoder;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.vtable, __TIB_java_nio_charset_CharsetEncoder.vtable, sizeof(__TIB_java_nio_charset_CharsetEncoder.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.vtable[6] = (VTABLE_PTR) &org_apache_harmony_niochar_charset_UTF_8_Encoder_encodeLoop___java_nio_CharBuffer_java_nio_ByteBuffer;
    // Initialize interface information
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.numImplementedInterfaces = 0;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.clazz = __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_1ARRAY);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Encoder_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_niochar_charset_UTF_8_Encoder]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder.classInitialized = 1;
}

void __DELETE_org_apache_harmony_niochar_charset_UTF_8_Encoder(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_niochar_charset_UTF_8_Encoder]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Encoder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetEncoder(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_niochar_charset_UTF_8_Encoder*) me)->fields.org_apache_harmony_niochar_charset_UTF_8_Encoder.this_0_ = (org_apache_harmony_niochar_charset_UTF_8*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Encoder]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_niochar_charset_UTF_8_Encoder()
{    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8_Encoder)
org_apache_harmony_niochar_charset_UTF_8_Encoder* me = (org_apache_harmony_niochar_charset_UTF_8_Encoder*) XMLVM_MALLOC(sizeof(org_apache_harmony_niochar_charset_UTF_8_Encoder));
    me->tib = &__TIB_org_apache_harmony_niochar_charset_UTF_8_Encoder;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Encoder(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_niochar_charset_UTF_8_Encoder]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8_Encoder()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Encoder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("UTF_8.java", 217)
    XMLVM_CHECK_NPE(2)
    ((org_apache_harmony_niochar_charset_UTF_8_Encoder*) _r2.o)->fields.org_apache_harmony_niochar_charset_UTF_8_Encoder.this_0_ = _r3.o;
    XMLVM_SOURCE_POSITION("UTF_8.java", 216)
    _r0.f = 1.1;
    _r1.f = 4.0;
    XMLVM_CHECK_NPE(2)
    java_nio_charset_CharsetEncoder___INIT____java_nio_charset_Charset_float_float(_r2.o, _r4.o, _r0.f, _r1.f);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_Encoder_encodeLoop___java_nio_CharBuffer_java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Encoder_encodeLoop___java_nio_CharBuffer_java_nio_ByteBuffer]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Encoder", "encodeLoop", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    volatile XMLVMElem _r10;
    volatile XMLVMElem _r11;
    volatile XMLVMElem _r12;
    volatile XMLVMElem _r13;
    volatile XMLVMElem _r14;
    _r12.o = me;
    _r13.o = n1;
    _r14.o = n2;
    XMLVM_SOURCE_POSITION("UTF_8.java", 220)
    XMLVM_CHECK_NPE(14)
    _r0.i = java_nio_Buffer_remaining__(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 221)
    XMLVM_CHECK_NPE(13)
    _r1.i = java_nio_Buffer_position__(_r13.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 222)
    XMLVM_CHECK_NPE(13)
    _r2.i = java_nio_Buffer_limit__(_r13.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 223)
    //java_nio_CharBuffer_hasArray__[8]
    XMLVM_CHECK_NPE(13)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r13.o)->tib->vtable[8])(_r13.o);
    if (_r3.i == 0) goto label589;
    //java_nio_ByteBuffer_hasArray__[8]
    XMLVM_CHECK_NPE(14)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[8])(_r14.o);
    if (_r3.i == 0) goto label589;
    XMLVM_SOURCE_POSITION("UTF_8.java", 227)
    //java_nio_ByteBuffer_array__[7]
    XMLVM_CHECK_NPE(14)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[7])(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 228)
    //java_nio_CharBuffer_array__[7]
    XMLVM_CHECK_NPE(13)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r13.o)->tib->vtable[7])(_r13.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 229)
    XMLVM_CHECK_NPE(14)
    _r5.i = java_nio_Buffer_position__(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 230)
    XMLVM_CHECK_NPE(13)
    _r6.i = java_nio_Buffer_remaining__(_r13.o);
    _r7 = _r0;
    _r0 = _r5;
    _r5 = _r1;
    label43:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 231)
    _r8.i = _r1.i + _r6.i;
    if (_r5.i < _r8.i) goto label58;
    XMLVM_SOURCE_POSITION("UTF_8.java", 317)
    if (_r6.i == 0) goto label55;
    XMLVM_SOURCE_POSITION("UTF_8.java", 318)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 319)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    label55:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 401)
    _r0.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    label57:;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label58:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 232)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r9.i = 65535;
    _r8.i = _r8.i & _r9.i;
    _r9.i = 127;
    if (_r8.i > _r9.i) goto label104;
    XMLVM_SOURCE_POSITION("UTF_8.java", 234)
    XMLVM_SOURCE_POSITION("UTF_8.java", 235)
    _r9.i = 1;
    if (_r7.i >= _r9.i) goto label80;
    XMLVM_SOURCE_POSITION("UTF_8.java", 236)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 237)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 238)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label57;
    label80:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 240)
    _r9.i = _r0.i + 1;
    _r8.i = _r8.i & 255;
    _r8.i = (_r8.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r8.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 241)
    _r0.i = _r7.i + -1;
    _r7 = _r0;
    _r0 = _r9;
    label91:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 310)
    if (_r7.i != 0) goto label327;
    XMLVM_SOURCE_POSITION("UTF_8.java", 311)
    _r1.i = _r5.i + 1;
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r1.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 312)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 313)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label57;
    label104:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 242)
    _r9.i = 2047;
    if (_r8.i > _r9.i) goto label143;
    XMLVM_SOURCE_POSITION("UTF_8.java", 244)
    _r9.i = 2;
    if (_r7.i >= _r9.i) goto label120;
    XMLVM_SOURCE_POSITION("UTF_8.java", 245)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 246)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 247)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label57;
    label120:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 249)
    _r9.i = _r0.i + 1;
    _r10.i = _r8.i >> 6;
    _r10.i = _r10.i & 31;
    _r10.i = _r10.i + 192;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 250)
    _r0.i = _r9.i + 1;
    _r8.i = _r8.i & 63;
    _r8.i = _r8.i + 128;
    _r8.i = (_r8.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r8.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 251)
    _r7.i = _r7.i + -2;
    goto label91;
    label143:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 253)
    _r9.i = 55296;
    if (_r8.i < _r9.i) goto label277;
    _r9.i = 57343;
    if (_r8.i > _r9.i) goto label277;
    XMLVM_SOURCE_POSITION("UTF_8.java", 256)
    _r9.i = _r5.i + 1;
    if (_r2.i > _r9.i) goto label166;
    XMLVM_SOURCE_POSITION("UTF_8.java", 257)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 258)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 259)
    _r0.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    goto label57;
    label166:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 262)
    _r9.i = 4;
    if (_r7.i >= _r9.i) goto label178;
    XMLVM_SOURCE_POSITION("UTF_8.java", 263)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 264)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 265)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label57;
    label178:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 269)
    _r9.i = 56320;
    if (_r8.i < _r9.i) goto label196;
    XMLVM_SOURCE_POSITION("UTF_8.java", 270)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 271)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 272)
    _r0.i = 1;
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r0.i);
    goto label57;
    label196:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 275)
    _r9.i = _r5.i + 1;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r9.i);
    _r9.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i];
    _r10.i = 65535;
    _r9.i = _r9.i & _r10.i;
    _r10.i = 56320;
    if (_r9.i >= _r10.i) goto label222;
    XMLVM_SOURCE_POSITION("UTF_8.java", 278)
    XMLVM_SOURCE_POSITION("UTF_8.java", 279)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 280)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 281)
    _r0.i = 1;
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r0.i);
    goto label57;
    label222:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 289)
    _r8.i = _r8.i << 10;
    _r8.i = _r8.i + _r9.i;
    _r9.i = -56613888;
    _r8.i = _r8.i + _r9.i;
    _r9.i = _r0.i + 1;
    _r10.i = _r8.i >> 18;
    _r10.i = _r10.i & 7;
    _r10.i = _r10.i + 240;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_SOURCE_POSITION("UTF_8.java", 291)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 292)
    _r0.i = _r9.i + 1;
    _r10.i = _r8.i >> 12;
    _r10.i = _r10.i & 63;
    _r10.i = _r10.i + 128;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 293)
    _r9.i = _r0.i + 1;
    _r10.i = _r8.i >> 6;
    _r10.i = _r10.i & 63;
    _r10.i = _r10.i + 128;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 294)
    _r0.i = _r9.i + 1;
    _r8.i = _r8.i & 63;
    _r8.i = _r8.i + 128;
    _r8.i = (_r8.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r8.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 295)
    _r7.i = _r7.i + -4;
    _r5.i = _r5.i + 1;
    XMLVM_SOURCE_POSITION("UTF_8.java", 296)
    goto label91;
    label277:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 300)
    _r9.i = 3;
    if (_r7.i >= _r9.i) goto label290;
    XMLVM_SOURCE_POSITION("UTF_8.java", 301)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r5.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 302)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 303)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label57;
    label290:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 305)
    _r9.i = _r0.i + 1;
    _r10.i = _r8.i >> 12;
    _r10.i = _r10.i & 15;
    _r10.i = _r10.i + 224;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 306)
    _r0.i = _r9.i + 1;
    _r10.i = _r8.i >> 6;
    _r10.i = _r10.i & 63;
    _r10.i = _r10.i + 128;
    _r10.i = (_r10.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r10.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 307)
    _r9.i = _r0.i + 1;
    _r8.i = _r8.i & 63;
    _r8.i = _r8.i + 128;
    _r8.i = (_r8.i << 24) >> 24;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r8.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 308)
    _r0.i = _r7.i + -3;
    _r7 = _r0;
    _r0 = _r9;
    goto label91;
    label327:;
    _r5.i = _r5.i + 1;
    goto label43;
    label331:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 324)
    if (_r1.i != 0) goto label341;
    XMLVM_TRY_BEGIN(w6023aaab2b1d248)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 325)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d248)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d248,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d248)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d248)
    XMLVM_SOURCE_POSITION("UTF_8.java", 397)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label341:;
    XMLVM_TRY_BEGIN(w6023aaab2b1d254)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 328)
    //java_nio_CharBuffer_get__[19]
    XMLVM_CHECK_NPE(13)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r13.o)->tib->vtable[19])(_r13.o);
    _r4.i = 65535;
    _r3.i = _r3.i & _r4.i;
    _r4.i = 127;
    if (_r3.i > _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d254->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d254, sizeof(XMLVM_JMP_BUF)); goto label381; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 330)
    XMLVM_SOURCE_POSITION("UTF_8.java", 332)
    _r4.i = 1;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d254->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d254, sizeof(XMLVM_JMP_BUF)); goto label364; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 333)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d254)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d254,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d254)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d254)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label364:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 335)
    _r3.i = _r3.i & 255;
    _r3.i = (_r3.i << 24) >> 24;
    XMLVM_TRY_BEGIN(w6023aaab2b1d262)
    // Begin try
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r3.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d262)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d262,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d262)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d262)
    XMLVM_SOURCE_POSITION("UTF_8.java", 336)
    _r1.i = _r1.i + -1;
    label372:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 394)
    _r0.i = _r0.i + 1;
    label374:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 323)
    if (_r0.i < _r2.i) goto label331;
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    goto label55;
    label381:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 338)
    _r4.i = 2047;
    if (_r3.i > _r4.i) goto label417;
    XMLVM_SOURCE_POSITION("UTF_8.java", 340)
    _r4.i = 2;
    if (_r1.i >= _r4.i) goto label396;
    XMLVM_TRY_BEGIN(w6023aaab2b1d280)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 341)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d280)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d280,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d280)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d280)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label396:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 343)
    _r4.i = _r3.i >> 6;
    _r4.i = _r4.i & 31;
    _r4.i = _r4.i + 192;
    _r4.i = (_r4.i << 24) >> 24;
    XMLVM_TRY_BEGIN(w6023aaab2b1d290)
    // Begin try
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 344)
    _r3.i = _r3.i & 63;
    _r3.i = _r3.i + 128;
    _r3.i = (_r3.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r3.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 345)
    _r1.i = _r1.i + -2;
    { XMLVM_MEMCPY(curThread_w6023aaab2b1d290->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d290, sizeof(XMLVM_JMP_BUF)); goto label372; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 347)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d290)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d290,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d290)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d290)
    label417:;
    XMLVM_TRY_BEGIN(w6023aaab2b1d292)
    // Begin try
    _r4.i = 55296;
    if (_r3.i < _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d292->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d292, sizeof(XMLVM_JMP_BUF)); goto label541; };
    _r4.i = 57343;
    if (_r3.i > _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d292->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d292, sizeof(XMLVM_JMP_BUF)); goto label541; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 350)
    _r4.i = _r0.i + 1;
    if (_r2.i > _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d292->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d292, sizeof(XMLVM_JMP_BUF)); goto label439; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 351)
    _r1.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d292)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d292,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d292)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d292)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label439:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 354)
    _r4.i = 4;
    if (_r1.i >= _r4.i) goto label450;
    XMLVM_TRY_BEGIN(w6023aaab2b1d300)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 355)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d300)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d300,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d300)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d300)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label450:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 359)
    _r4.i = 56320;
    if (_r3.i < _r4.i) goto label466;
    XMLVM_SOURCE_POSITION("UTF_8.java", 360)
    _r1.i = 1;
    XMLVM_TRY_BEGIN(w6023aaab2b1d310)
    // Begin try
    _r1.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d310)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d310,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d310)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d310)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label466:;
    XMLVM_TRY_BEGIN(w6023aaab2b1d315)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 363)
    //java_nio_CharBuffer_get__[19]
    XMLVM_CHECK_NPE(13)
    _r4.i = (*(JAVA_CHAR (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r13.o)->tib->vtable[19])(_r13.o);
    _r5.i = 65535;
    _r4.i = _r4.i & _r5.i;
    _r5.i = 56320;
    if (_r4.i >= _r5.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d315->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d315, sizeof(XMLVM_JMP_BUF)); goto label490; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 366)
    XMLVM_SOURCE_POSITION("UTF_8.java", 367)
    _r1.i = 1;
    _r1.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d315)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d315,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d315)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d315)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label490:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 375)
    _r3.i = _r3.i << 10;
    _r3.i = _r3.i + _r4.i;
    _r4.i = -56613888;
    _r3.i = _r3.i + _r4.i;
    _r4.i = _r3.i >> 18;
    _r4.i = _r4.i & 7;
    _r4.i = _r4.i + 240;
    _r4.i = (_r4.i << 24) >> 24;
    XMLVM_TRY_BEGIN(w6023aaab2b1d329)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 377)
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 378)
    _r4.i = _r3.i >> 12;
    _r4.i = _r4.i & 63;
    _r4.i = _r4.i + 128;
    _r4.i = (_r4.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 379)
    _r4.i = _r3.i >> 6;
    _r4.i = _r4.i & 63;
    _r4.i = _r4.i + 128;
    _r4.i = (_r4.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 380)
    _r3.i = _r3.i & 63;
    _r3.i = _r3.i + 128;
    _r3.i = (_r3.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r3.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 381)
    _r1.i = _r1.i + -4;
    _r0.i = _r0.i + 1;
    XMLVM_SOURCE_POSITION("UTF_8.java", 382)
    { XMLVM_MEMCPY(curThread_w6023aaab2b1d329->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d329, sizeof(XMLVM_JMP_BUF)); goto label372; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 386)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d329)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d329,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d329)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d329)
    label541:;
    XMLVM_TRY_BEGIN(w6023aaab2b1d331)
    // Begin try
    _r4.i = 3;
    if (_r1.i >= _r4.i) { XMLVM_MEMCPY(curThread_w6023aaab2b1d331->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w6023aaab2b1d331, sizeof(XMLVM_JMP_BUF)); goto label552; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 387)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d331)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d331,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d331)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d331)
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    _r0 = _r1;
    goto label57;
    label552:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 389)
    _r4.i = _r3.i >> 12;
    _r4.i = _r4.i & 15;
    _r4.i = _r4.i + 224;
    _r4.i = (_r4.i << 24) >> 24;
    XMLVM_TRY_BEGIN(w6023aaab2b1d341)
    // Begin try
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 390)
    _r4.i = _r3.i >> 6;
    _r4.i = _r4.i & 63;
    _r4.i = _r4.i + 128;
    _r4.i = (_r4.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r4.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 391)
    _r3.i = _r3.i & 63;
    _r3.i = _r3.i + 128;
    _r3.i = (_r3.i << 24) >> 24;
    //java_nio_ByteBuffer_put___byte[51]
    XMLVM_CHECK_NPE(14)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_BYTE)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[51])(_r14.o, _r3.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6023aaab2b1d341)
        XMLVM_CATCH_SPECIFIC(w6023aaab2b1d341,java_lang_Object,584)
    XMLVM_CATCH_END(w6023aaab2b1d341)
    XMLVM_RESTORE_EXCEPTION_ENV(w6023aaab2b1d341)
    XMLVM_SOURCE_POSITION("UTF_8.java", 392)
    _r1.i = _r1.i + -3;
    goto label372;
    label584:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 396)
    java_lang_Thread* curThread_w6023aaab2b1d347 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w6023aaab2b1d347->fields.java_lang_Thread.xmlvmException_;
    XMLVM_CHECK_NPE(13)
    java_nio_Buffer_position___int(_r13.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 398)
    XMLVM_THROW_CUSTOM(_r1.o)
    label589:;
    _r11 = _r1;
    _r1 = _r0;
    _r0 = _r11;
    goto label374;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Encoder(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Encoder]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Encoder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("UTF_8.java", 215)
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_charset_UTF_8_Encoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset(_r0.o, _r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

