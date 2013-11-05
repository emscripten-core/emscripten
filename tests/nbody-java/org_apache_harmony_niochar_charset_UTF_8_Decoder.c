#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Object.h"
#include "java_nio_ByteBuffer.h"
#include "java_nio_CharBuffer.h"
#include "java_nio_charset_Charset.h"
#include "java_nio_charset_CoderResult.h"
#include "org_apache_harmony_niochar_charset_UTF_8.h"

#include "org_apache_harmony_niochar_charset_UTF_8_Decoder.h"

#define XMLVM_CURRENT_CLASS_NAME UTF_8_Decoder
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_niochar_charset_UTF_8_Decoder

__TIB_DEFINITION_org_apache_harmony_niochar_charset_UTF_8_Decoder __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_niochar_charset_UTF_8_Decoder, // classInitializer
    "org.apache.harmony.niochar.charset.UTF_8$Decoder", // className
    "org.apache.harmony.niochar.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_charset_CharsetDecoder, // extends
    sizeof(org_apache_harmony_niochar_charset_UTF_8_Decoder), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_3ARRAY;
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

void __INIT_org_apache_harmony_niochar_charset_UTF_8_Decoder()
{
    staticInitializerLock(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.niochar.charset.UTF_8$Decoder")
        __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8_Decoder();
    }
}

void __INIT_IMPL_org_apache_harmony_niochar_charset_UTF_8_Decoder()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8_Decoder;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.vtable, __TIB_java_nio_charset_CharsetDecoder.vtable, sizeof(__TIB_java_nio_charset_CharsetDecoder.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.vtable[6] = (VTABLE_PTR) &org_apache_harmony_niochar_charset_UTF_8_Decoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer;
    // Initialize interface information
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.numImplementedInterfaces = 0;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder);
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.clazz = __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder;
    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_1ARRAY);
    __CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_charset_UTF_8_Decoder_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_niochar_charset_UTF_8_Decoder]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder.classInitialized = 1;
}

void __DELETE_org_apache_harmony_niochar_charset_UTF_8_Decoder(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_niochar_charset_UTF_8_Decoder]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Decoder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetDecoder(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_niochar_charset_UTF_8_Decoder*) me)->fields.org_apache_harmony_niochar_charset_UTF_8_Decoder.this_0_ = (org_apache_harmony_niochar_charset_UTF_8*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Decoder]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_niochar_charset_UTF_8_Decoder()
{    XMLVM_CLASS_INIT(org_apache_harmony_niochar_charset_UTF_8_Decoder)
org_apache_harmony_niochar_charset_UTF_8_Decoder* me = (org_apache_harmony_niochar_charset_UTF_8_Decoder*) XMLVM_MALLOC(sizeof(org_apache_harmony_niochar_charset_UTF_8_Decoder));
    me->tib = &__TIB_org_apache_harmony_niochar_charset_UTF_8_Decoder;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_charset_UTF_8_Decoder(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_niochar_charset_UTF_8_Decoder]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_niochar_charset_UTF_8_Decoder()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Decoder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r0.f = 1.0;
    XMLVM_SOURCE_POSITION("UTF_8.java", 111)
    XMLVM_CHECK_NPE(1)
    ((org_apache_harmony_niochar_charset_UTF_8_Decoder*) _r1.o)->fields.org_apache_harmony_niochar_charset_UTF_8_Decoder.this_0_ = _r2.o;
    XMLVM_SOURCE_POSITION("UTF_8.java", 110)
    XMLVM_CHECK_NPE(1)
    java_nio_charset_CharsetDecoder___INIT____java_nio_charset_Charset_float_float(_r1.o, _r3.o, _r0.f, _r0.f);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_charset_UTF_8_Decoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Decoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Decoder", "decodeLoop", "?")
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
    volatile XMLVMElem _r15;
    _r13.o = me;
    _r14.o = n1;
    _r15.o = n2;
    XMLVM_SOURCE_POSITION("UTF_8.java", 114)
    XMLVM_CHECK_NPE(15)
    _r0.i = java_nio_Buffer_remaining__(_r15.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 115)
    XMLVM_CHECK_NPE(14)
    _r1.i = java_nio_Buffer_position__(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 116)
    XMLVM_CHECK_NPE(14)
    _r2.i = java_nio_Buffer_limit__(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 117)
    //java_nio_ByteBuffer_hasArray__[8]
    XMLVM_CHECK_NPE(14)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[8])(_r14.o);
    if (_r3.i == 0) goto label392;
    //java_nio_CharBuffer_hasArray__[8]
    XMLVM_CHECK_NPE(15)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[8])(_r15.o);
    if (_r3.i == 0) goto label392;
    XMLVM_SOURCE_POSITION("UTF_8.java", 118)
    //java_nio_ByteBuffer_array__[7]
    XMLVM_CHECK_NPE(14)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[7])(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 119)
    //java_nio_CharBuffer_array__[7]
    XMLVM_CHECK_NPE(15)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[7])(_r15.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 120)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r5.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r2.i = _r2.i + _r5.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 122)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r5.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r1.i = _r1.i + _r5.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 123)
    XMLVM_CHECK_NPE(15)
    _r5.i = java_nio_Buffer_position__(_r15.o);
    //java_nio_CharBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(15)
    _r6.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[6])(_r15.o);
    _r5.i = _r5.i + _r6.i;
    _r12 = _r5;
    _r5 = _r0;
    _r0 = _r12;
    label54:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 127)
    if (_r1.i >= _r2.i) goto label58;
    if (_r5.i > 0) goto label82;
    label58:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 163)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r3.i = _r1.i - _r3.i;
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r3.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 164)
    //java_nio_CharBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(15)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[6])(_r15.o);
    _r0.i = _r0.i - _r3.i;
    XMLVM_CHECK_NPE(15)
    java_nio_Buffer_position___int(_r15.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 165)
    if (_r5.i != 0) goto label229;
    if (_r1.i >= _r2.i) goto label229;
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    label81:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 205)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label82:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 128)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r6.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("UTF_8.java", 129)
    if (_r6.i >= 0) goto label387;
    XMLVM_SOURCE_POSITION("UTF_8.java", 130)
    _r6.i = _r6.i & 127;
    XMLVM_SOURCE_POSITION("UTF_8.java", 131)
    _r7.o = org_apache_harmony_niochar_charset_UTF_8_access$0__();
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r6.i);
    _r7.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i];
    XMLVM_SOURCE_POSITION("UTF_8.java", 133)
    _r8.i = -1;
    if (_r7.i != _r8.i) goto label119;
    XMLVM_SOURCE_POSITION("UTF_8.java", 134)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r1.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 135)
    //java_nio_CharBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(15)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[6])(_r15.o);
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(15)
    java_nio_Buffer_position___int(_r15.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 136)
    _r0.i = 1;
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r0.i);
    goto label81;
    label119:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 138)
    _r8.i = _r2.i - _r1.i;
    _r9.i = _r7.i + 1;
    if (_r8.i < _r9.i) goto label58;
    XMLVM_SOURCE_POSITION("UTF_8.java", 142)
    _r8.i = 0;
    _r12 = _r8;
    _r8 = _r6;
    _r6 = _r12;
    label129:;
    if (_r6.i < _r7.i) goto label169;
    XMLVM_SOURCE_POSITION("UTF_8.java", 151)
    _r6.o = org_apache_harmony_niochar_charset_UTF_8_access$1__();
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r7.i);
    _r6.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    _r6.i = _r8.i - _r6.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 152)
    _r8.o = org_apache_harmony_niochar_charset_UTF_8_access$2__();
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r7.i);
    _r8.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    if (_r6.i >= _r8.i) goto label212;
    XMLVM_SOURCE_POSITION("UTF_8.java", 154)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r1.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 155)
    //java_nio_CharBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(15)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[6])(_r15.o);
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(15)
    java_nio_Buffer_position___int(_r15.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 156)
    _r0.i = 1;
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r0.i);
    goto label81;
    label169:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 143)
    _r9.i = _r1.i + _r6.i;
    _r9.i = _r9.i + 1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    _r9.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i];
    _r9.i = _r9.i & 255;
    _r10.i = _r9.i & 192;
    _r11.i = 128;
    if (_r10.i == _r11.i) goto label206;
    XMLVM_SOURCE_POSITION("UTF_8.java", 144)
    XMLVM_SOURCE_POSITION("UTF_8.java", 145)
    //java_nio_ByteBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(14)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[6])(_r14.o);
    _r1.i = _r1.i - _r2.i;
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r1.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 146)
    //java_nio_CharBuffer_arrayOffset__[6]
    XMLVM_CHECK_NPE(15)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[6])(_r15.o);
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(15)
    java_nio_Buffer_position___int(_r15.o, _r0.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 147)
    _r0.i = _r6.i + 1;
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r0.i);
    goto label81;
    label206:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 149)
    _r8.i = _r8.i << 6;
    _r8.i = _r8.i + _r9.i;
    _r6.i = _r6.i + 1;
    goto label129;
    label212:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 158)
    _r1.i = _r1.i + _r7.i;
    _r12 = _r6;
    _r6 = _r1;
    _r1 = _r12;
    label216:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 160)
    _r7.i = _r0.i + 1;
    _r1.i = _r1.i & 0xffff;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r1.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 161)
    _r0.i = _r5.i + -1;
    _r1.i = _r6.i + 1;
    _r5 = _r0;
    _r0 = _r7;
    goto label54;
    label229:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 166)
    _r0.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    goto label81;
    label233:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 170)
    if (_r1.i != 0) goto label243;
    XMLVM_TRY_BEGIN(w4520aaab2b1d161)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 171)
    _r1.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d161)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d161,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d161)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d161)
    XMLVM_SOURCE_POSITION("UTF_8.java", 207)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    goto label81;
    label243:;
    XMLVM_TRY_BEGIN(w4520aaab2b1d167)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 174)
    //java_nio_ByteBuffer_get__[33]
    XMLVM_CHECK_NPE(14)
    _r3.i = (*(JAVA_BYTE (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[33])(_r14.o);
    XMLVM_SOURCE_POSITION("UTF_8.java", 175)
    if (_r3.i >= 0) { XMLVM_MEMCPY(curThread_w4520aaab2b1d167->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4520aaab2b1d167, sizeof(XMLVM_JMP_BUF)); goto label383; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 176)
    _r3.i = _r3.i & 127;
    XMLVM_SOURCE_POSITION("UTF_8.java", 177)
    _r4.o = org_apache_harmony_niochar_charset_UTF_8_access$0__();
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r4.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_SOURCE_POSITION("UTF_8.java", 178)
    _r5.i = -1;
    if (_r4.i != _r5.i) { XMLVM_MEMCPY(curThread_w4520aaab2b1d167->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4520aaab2b1d167, sizeof(XMLVM_JMP_BUF)); goto label271; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 179)
    _r1.i = 1;
    _r1.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d167)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d167,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d167)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d167)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    goto label81;
    label271:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 181)
    _r5.i = _r2.i - _r0.i;
    _r6.i = _r4.i + 1;
    if (_r5.i >= _r6.i) goto label285;
    XMLVM_TRY_BEGIN(w4520aaab2b1d176)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 182)
    _r1.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d176)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d176,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d176)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d176)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    goto label81;
    label285:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 186)
    _r5.i = 0;
    _r12 = _r5;
    _r5 = _r3;
    _r3 = _r12;
    label289:;
    if (_r3.i < _r4.i) goto label318;
    XMLVM_TRY_BEGIN(w4520aaab2b1d188)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 194)
    _r3.o = org_apache_harmony_niochar_charset_UTF_8_access$1__();
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r3.i = _r5.i - _r3.i;
    XMLVM_SOURCE_POSITION("UTF_8.java", 195)
    _r5.o = org_apache_harmony_niochar_charset_UTF_8_access$2__();
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r4.i);
    _r5.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    if (_r3.i >= _r5.i) { XMLVM_MEMCPY(curThread_w4520aaab2b1d188->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4520aaab2b1d188, sizeof(XMLVM_JMP_BUF)); goto label348; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 197)
    _r1.i = 1;
    _r1.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d188)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d188,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d188)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d188)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    goto label81;
    label318:;
    XMLVM_TRY_BEGIN(w4520aaab2b1d193)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 187)
    //java_nio_ByteBuffer_get__[33]
    XMLVM_CHECK_NPE(14)
    _r6.i = (*(JAVA_BYTE (*)(JAVA_OBJECT)) ((java_nio_ByteBuffer*) _r14.o)->tib->vtable[33])(_r14.o);
    _r6.i = _r6.i & 255;
    _r7.i = _r6.i & 192;
    _r8.i = 128;
    if (_r7.i == _r8.i) { XMLVM_MEMCPY(curThread_w4520aaab2b1d193->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4520aaab2b1d193, sizeof(XMLVM_JMP_BUF)); goto label342; };
    XMLVM_SOURCE_POSITION("UTF_8.java", 188)
    XMLVM_SOURCE_POSITION("UTF_8.java", 190)
    _r1.i = _r3.i + 1;
    _r1.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d193)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d193,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d193)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d193)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("UTF_8.java", 189)
    goto label81;
    label342:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 192)
    _r5.i = _r5.i << 6;
    _r5.i = _r5.i + _r6.i;
    _r3.i = _r3.i + 1;
    goto label289;
    label348:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 199)
    _r0.i = _r0.i + _r4.i;
    _r12 = _r3;
    _r3 = _r0;
    _r0 = _r12;
    label352:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 201)
    _r3.i = _r3.i + 1;
    _r0.i = _r0.i & 0xffff;
    XMLVM_TRY_BEGIN(w4520aaab2b1d214)
    // Begin try
    XMLVM_SOURCE_POSITION("UTF_8.java", 202)
    //java_nio_CharBuffer_put___char[27]
    XMLVM_CHECK_NPE(15)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_nio_CharBuffer*) _r15.o)->tib->vtable[27])(_r15.o, _r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d214)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d214,java_lang_Object,380)
    XMLVM_CATCH_END(w4520aaab2b1d214)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d214)
    XMLVM_SOURCE_POSITION("UTF_8.java", 203)
    _r0.i = _r1.i + -1;
    _r1 = _r0;
    _r0 = _r3;
    label362:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 169)
    if (_r0.i < _r2.i) goto label233;
    XMLVM_TRY_BEGIN(w4520aaab2b1d222)
    // Begin try
    _r1.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4520aaab2b1d222)
        XMLVM_CATCH_SPECIFIC(w4520aaab2b1d222,java_lang_Object,372)
    XMLVM_CATCH_END(w4520aaab2b1d222)
    XMLVM_RESTORE_EXCEPTION_ENV(w4520aaab2b1d222)
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r0.i);
    _r0 = _r1;
    goto label81;
    label372:;
    XMLVM_SOURCE_POSITION("UTF_8.java", 206)
    java_lang_Thread* curThread_w4520aaab2b1d228 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4520aaab2b1d228->fields.java_lang_Thread.xmlvmException_;
    _r12 = _r1;
    _r1 = _r0;
    _r0 = _r12;
    label376:;
    XMLVM_CHECK_NPE(14)
    java_nio_Buffer_position___int(_r14.o, _r1.i);
    XMLVM_SOURCE_POSITION("UTF_8.java", 208)
    XMLVM_THROW_CUSTOM(_r0.o)
    label380:;
    java_lang_Thread* curThread_w4520aaab2b1d237 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4520aaab2b1d237->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r3;
    goto label376;
    label383:;
    _r12 = _r3;
    _r3 = _r0;
    _r0 = _r12;
    goto label352;
    label387:;
    _r12 = _r6;
    _r6 = _r1;
    _r1 = _r12;
    goto label216;
    label392:;
    _r12 = _r1;
    _r1 = _r0;
    _r0 = _r12;
    goto label362;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Decoder(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset_org_apache_harmony_niochar_charset_UTF_8_Decoder]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.charset.UTF_8$Decoder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("UTF_8.java", 109)
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_charset_UTF_8_Decoder___INIT____org_apache_harmony_niochar_charset_UTF_8_java_nio_charset_Charset(_r0.o, _r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

