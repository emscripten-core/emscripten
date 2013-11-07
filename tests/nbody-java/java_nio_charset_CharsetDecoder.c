#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"
#include "java_nio_ByteBuffer.h"
#include "java_nio_CharBuffer.h"
#include "java_nio_charset_Charset.h"
#include "java_nio_charset_CoderResult.h"
#include "java_nio_charset_CodingErrorAction.h"

#include "java_nio_charset_CharsetDecoder.h"

#define XMLVM_CURRENT_CLASS_NAME CharsetDecoder
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_charset_CharsetDecoder

__TIB_DEFINITION_java_nio_charset_CharsetDecoder __TIB_java_nio_charset_CharsetDecoder = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_charset_CharsetDecoder, // classInitializer
    "java.nio.charset.CharsetDecoder", // className
    "java.nio.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_nio_charset_CharsetDecoder), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder;
JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_nio_charset_CharsetDecoder_INIT;
static JAVA_INT _STATIC_java_nio_charset_CharsetDecoder_ONGOING;
static JAVA_INT _STATIC_java_nio_charset_CharsetDecoder_END;
static JAVA_INT _STATIC_java_nio_charset_CharsetDecoder_FLUSH;

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

void __INIT_java_nio_charset_CharsetDecoder()
{
    staticInitializerLock(&__TIB_java_nio_charset_CharsetDecoder);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_charset_CharsetDecoder.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_charset_CharsetDecoder.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_charset_CharsetDecoder);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_charset_CharsetDecoder.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_charset_CharsetDecoder.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_charset_CharsetDecoder.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.charset.CharsetDecoder")
        __INIT_IMPL_java_nio_charset_CharsetDecoder();
    }
}

void __INIT_IMPL_java_nio_charset_CharsetDecoder()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_nio_charset_CharsetDecoder.newInstanceFunc = __NEW_INSTANCE_java_nio_charset_CharsetDecoder;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_charset_CharsetDecoder.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_nio_charset_CharsetDecoder.numImplementedInterfaces = 0;
    __TIB_java_nio_charset_CharsetDecoder.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_nio_charset_CharsetDecoder_INIT = 0;
    _STATIC_java_nio_charset_CharsetDecoder_ONGOING = 1;
    _STATIC_java_nio_charset_CharsetDecoder_END = 2;
    _STATIC_java_nio_charset_CharsetDecoder_FLUSH = 3;

    __TIB_java_nio_charset_CharsetDecoder.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_charset_CharsetDecoder.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_charset_CharsetDecoder.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_charset_CharsetDecoder.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_charset_CharsetDecoder.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_charset_CharsetDecoder.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_charset_CharsetDecoder.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_charset_CharsetDecoder.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_charset_CharsetDecoder = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_charset_CharsetDecoder);
    __TIB_java_nio_charset_CharsetDecoder.clazz = __CLASS_java_nio_charset_CharsetDecoder;
    __TIB_java_nio_charset_CharsetDecoder.baseType = JAVA_NULL;
    __CLASS_java_nio_charset_CharsetDecoder_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CharsetDecoder);
    __CLASS_java_nio_charset_CharsetDecoder_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CharsetDecoder_1ARRAY);
    __CLASS_java_nio_charset_CharsetDecoder_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_CharsetDecoder_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_charset_CharsetDecoder]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_charset_CharsetDecoder.classInitialized = 1;
}

void __DELETE_java_nio_charset_CharsetDecoder(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_charset_CharsetDecoder]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetDecoder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.averChars_ = 0;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.maxChars_ = 0;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.cs_ = (java_nio_charset_Charset*) JAVA_NULL;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.malformAction_ = (java_nio_charset_CodingErrorAction*) JAVA_NULL;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.unmapAction_ = (java_nio_charset_CodingErrorAction*) JAVA_NULL;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.replace_ = (java_lang_String*) JAVA_NULL;
    ((java_nio_charset_CharsetDecoder*) me)->fields.java_nio_charset_CharsetDecoder.status_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetDecoder]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_charset_CharsetDecoder()
{    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
java_nio_charset_CharsetDecoder* me = (java_nio_charset_CharsetDecoder*) XMLVM_MALLOC(sizeof(java_nio_charset_CharsetDecoder));
    me->tib = &__TIB_java_nio_charset_CharsetDecoder;
    __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetDecoder(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_charset_CharsetDecoder]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CharsetDecoder()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_INT java_nio_charset_CharsetDecoder_GET_INIT()
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
    return _STATIC_java_nio_charset_CharsetDecoder_INIT;
}

void java_nio_charset_CharsetDecoder_PUT_INIT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
_STATIC_java_nio_charset_CharsetDecoder_INIT = v;
}

JAVA_INT java_nio_charset_CharsetDecoder_GET_ONGOING()
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
    return _STATIC_java_nio_charset_CharsetDecoder_ONGOING;
}

void java_nio_charset_CharsetDecoder_PUT_ONGOING(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
_STATIC_java_nio_charset_CharsetDecoder_ONGOING = v;
}

JAVA_INT java_nio_charset_CharsetDecoder_GET_END()
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
    return _STATIC_java_nio_charset_CharsetDecoder_END;
}

void java_nio_charset_CharsetDecoder_PUT_END(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
_STATIC_java_nio_charset_CharsetDecoder_END = v;
}

JAVA_INT java_nio_charset_CharsetDecoder_GET_FLUSH()
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
    return _STATIC_java_nio_charset_CharsetDecoder_FLUSH;
}

void java_nio_charset_CharsetDecoder_PUT_FLUSH(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_CharsetDecoder)
_STATIC_java_nio_charset_CharsetDecoder_FLUSH = v;
}

void java_nio_charset_CharsetDecoder___INIT____java_nio_charset_Charset_float_float(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2, JAVA_FLOAT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder___INIT____java_nio_charset_Charset_float_float]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.f = n2;
    _r5.f = n3;
    _r1.f = 0.0;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 138)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 140)
    _r0.i = _r4.f > _r1.f ? 1 : (_r4.f == _r1.f ? 0 : -1);
    if (_r0.i <= 0) goto label12;
    _r0.i = _r5.f > _r1.f ? 1 : (_r5.f == _r1.f ? 0 : -1);
    if (_r0.i > 0) goto label24;
    label12:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 142)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "niochar.00"
    _r1.o = xmlvm_create_java_string_from_pool(722);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 144)
    _r0.i = _r4.f > _r5.f ? 1 : (_r4.f == _r5.f ? 0 : -1);
    if (_r0.i <= 0) goto label40;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 146)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "niochar.01"
    _r1.o = xmlvm_create_java_string_from_pool(723);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label40:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 148)
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.averChars_ = _r4.f;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 149)
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.maxChars_ = _r5.f;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 150)
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.cs_ = _r3.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 151)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.status_ = _r0.i;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 152)
    _r0.o = java_nio_charset_CodingErrorAction_GET_REPORT();
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.malformAction_ = _r0.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 153)
    _r0.o = java_nio_charset_CodingErrorAction_GET_REPORT();
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.unmapAction_ = _r0.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 154)
    // "\177775"
    _r0.o = xmlvm_create_java_string_from_pool(724);
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.replace_ = _r0.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 155)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_nio_charset_CharsetDecoder_averageCharsPerByte__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_averageCharsPerByte__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "averageCharsPerByte", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 165)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.averChars_;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_charset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_charset__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "charset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 174)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.cs_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "decode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 212)
    XMLVM_CHECK_NPE(3)
    java_nio_charset_CharsetDecoder_reset__(_r3.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 213)
    XMLVM_CHECK_NPE(4)
    _r0.i = java_nio_Buffer_remaining__(_r4.o);
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_CHECK_NPE(3)
    _r1.f = ((java_nio_charset_CharsetDecoder*) _r3.o)->fields.java_nio_charset_CharsetDecoder.averChars_;
    _r0.f = _r0.f * _r1.f;
    _r0.i = (JAVA_INT) _r0.f;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 214)
    _r0.o = java_nio_CharBuffer_allocate___int(_r0.i);
    label16:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 217)
    _r1.i = 0;
    XMLVM_CHECK_NPE(3)
    _r1.o = java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer_java_nio_CharBuffer_boolean(_r3.o, _r4.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 218)
    XMLVM_CHECK_NPE(3)
    java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult(_r3.o, _r1.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 219)
    XMLVM_CHECK_NPE(1)
    _r2.i = java_nio_charset_CoderResult_isUnderflow__(_r1.o);
    if (_r2.i == 0) goto label56;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 225)
    _r1.i = 1;
    XMLVM_CHECK_NPE(3)
    _r1.o = java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer_java_nio_CharBuffer_boolean(_r3.o, _r4.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 226)
    XMLVM_CHECK_NPE(3)
    java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult(_r3.o, _r1.o);
    label38:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 229)
    XMLVM_CHECK_NPE(3)
    _r1.o = java_nio_charset_CharsetDecoder_flush___java_nio_CharBuffer(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 230)
    XMLVM_CHECK_NPE(3)
    java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult(_r3.o, _r1.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 231)
    XMLVM_CHECK_NPE(1)
    _r1.i = java_nio_charset_CoderResult_isOverflow__(_r1.o);
    if (_r1.i == 0) goto label67;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 232)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_CharsetDecoder_allocateMore___java_nio_CharBuffer(_r3.o, _r0.o);
    goto label38;
    label56:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 221)
    XMLVM_CHECK_NPE(1)
    _r1.i = java_nio_charset_CoderResult_isOverflow__(_r1.o);
    if (_r1.i == 0) goto label16;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 222)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_nio_charset_CharsetDecoder_allocateMore___java_nio_CharBuffer(_r3.o, _r0.o);
    goto label16;
    label67:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 238)
    XMLVM_CHECK_NPE(0)
    java_nio_Buffer_flip__(_r0.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 239)
    _r1.i = 3;
    XMLVM_CHECK_NPE(3)
    ((java_nio_charset_CharsetDecoder*) _r3.o)->fields.java_nio_charset_CharsetDecoder.status_ = _r1.i;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 240)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "checkCoderResult", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 248)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_charset_CoderResult_isMalformed__(_r3.o);
    if (_r0.i == 0) goto label22;
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.malformAction_;
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPORT();
    if (_r0.o != _r1.o) goto label22;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 249)

    
    // Red class access removed: java.nio.charset.MalformedInputException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    _r1.i = java_nio_charset_CoderResult_length__(_r3.o);

    
    // Red class access removed: java.nio.charset.MalformedInputException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label22:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 250)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_nio_charset_CoderResult_isUnmappable__(_r3.o);
    if (_r0.i == 0) goto label44;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 251)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.unmapAction_;
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPORT();
    if (_r0.o != _r1.o) goto label44;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 252)

    
    // Red class access removed: java.nio.charset.UnmappableCharacterException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    _r1.i = java_nio_charset_CoderResult_length__(_r3.o);

    
    // Red class access removed: java.nio.charset.UnmappableCharacterException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label44:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 254)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_allocateMore___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_allocateMore___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "allocateMore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 262)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_nio_Buffer_capacity__(_r2.o);
    if (_r0.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 263)
    _r0.i = 1;
    _r0.o = java_nio_CharBuffer_allocate___int(_r0.i);
    label11:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 268)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label12:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 265)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_nio_Buffer_capacity__(_r2.o);
    _r0.i = _r0.i * 2;
    _r0.o = java_nio_CharBuffer_allocate___int(_r0.i);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 266)
    XMLVM_CHECK_NPE(2)
    java_nio_Buffer_flip__(_r2.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 267)
    XMLVM_CHECK_NPE(0)
    java_nio_CharBuffer_put___java_nio_CharBuffer(_r0.o, _r2.o);
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer_java_nio_CharBuffer_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer_java_nio_CharBuffer_boolean]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "decode", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r4.o = me;
    _r5.o = n1;
    _r6.o = n2;
    _r7.i = n3;
    _r3.i = 2;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 340)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.status_;
    _r1.i = 3;
    if (_r0.i == _r1.i) goto label12;
    if (_r7.i != 0) goto label35;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.status_;
    if (_r0.i != _r3.i) goto label35;
    label12:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 341)

    
    // Red class access removed: java.lang.IllegalStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label18:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 384)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.replace_;
    XMLVM_CHECK_NPE(6)
    java_nio_CharBuffer_put___java_lang_String(_r6.o, _r1.o);
    label23:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 389)
    XMLVM_CHECK_NPE(5)
    _r1.i = java_nio_Buffer_position__(_r5.o);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_nio_charset_CoderResult_length__(_r0.o);
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    java_nio_Buffer_position___int(_r5.o, _r0.i);
    label35:;
    XMLVM_TRY_BEGIN(w4896aaac17b1c29)
    // Begin try
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 350)
    //java_nio_charset_CharsetDecoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer[6]
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_nio_charset_CharsetDecoder*) _r4.o)->tib->vtable[6])(_r4.o, _r5.o, _r6.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4896aaac17b1c29)
    XMLVM_CATCH_END(w4896aaac17b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w4896aaac17b1c29)
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 362)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_nio_charset_CoderResult_isUnderflow__(_r0.o);
    if (_r1.i == 0) goto label62;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 363)
    XMLVM_CHECK_NPE(5)
    _r1.i = java_nio_Buffer_remaining__(_r5.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 364)
    if (_r7.i == 0) goto label83;
    _r2 = _r3;
    label52:;
    XMLVM_CHECK_NPE(4)
    ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.status_ = _r2.i;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 365)
    if (_r7.i == 0) goto label68;
    if (_r1.i <= 0) goto label68;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 366)
    _r0.o = java_nio_charset_CoderResult_malformedForLength___int(_r1.i);
    label62:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 371)
    XMLVM_CHECK_NPE(0)
    _r1.i = java_nio_charset_CoderResult_isOverflow__(_r0.o);
    if (_r1.i == 0) goto label85;
    label68:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 387)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label69:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 351)
    java_lang_Thread* curThread_w4896aaac17b1c54 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4896aaac17b1c54->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 353)

    
    // Red class access removed: java.nio.charset.CoderMalfunctionError::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.CoderMalfunctionError::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    label76:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 354)
    java_lang_Thread* curThread_w4896aaac17b1c61 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4896aaac17b1c61->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 356)

    
    // Red class access removed: java.nio.charset.CoderMalfunctionError::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.CoderMalfunctionError::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    label83:;
    _r2.i = 1;
    goto label52;
    label85:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 375)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.malformAction_;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 376)
    XMLVM_CHECK_NPE(0)
    _r2.i = java_nio_charset_CoderResult_isUnmappable__(_r0.o);
    if (_r2.i == 0) goto label95;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 377)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.unmapAction_;
    label95:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 380)
    _r2.o = java_nio_charset_CodingErrorAction_GET_REPLACE();
    if (_r1.o != _r2.o) goto label114;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 381)
    XMLVM_CHECK_NPE(6)
    _r1.i = java_nio_Buffer_remaining__(_r6.o);
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_nio_charset_CharsetDecoder*) _r4.o)->fields.java_nio_charset_CharsetDecoder.replace_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(2)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r2.o)->tib->vtable[8])(_r2.o);
    if (_r1.i >= _r2.i) goto label18;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 382)
    _r0.o = java_nio_charset_CoderResult_GET_OVERFLOW();
    goto label68;
    label114:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 386)
    _r2.o = java_nio_charset_CodingErrorAction_GET_IGNORE();
    if (_r1.o == _r2.o) goto label23;
    goto label68;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_detectedCharset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_detectedCharset__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "detectedCharset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 450)

    
    // Red class access removed: java.lang.UnsupportedOperationException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.UnsupportedOperationException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_flush___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_flush___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "flush", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 486)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.status_;
    _r1.i = 2;
    if (_r0.i == _r1.i) goto label15;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.status_;
    if (_r0.i == 0) goto label15;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 487)

    
    // Red class access removed: java.lang.IllegalStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label15:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 489)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_charset_CharsetDecoder_implFlush___java_nio_CharBuffer(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 490)
    _r1.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    if (_r0.o != _r1.o) goto label26;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 491)
    _r1.i = 3;
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.status_ = _r1.i;
    label26:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 493)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_implFlush___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_implFlush___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "implFlush", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 507)
    _r0.o = java_nio_charset_CoderResult_GET_UNDERFLOW();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CharsetDecoder_implOnMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_implOnMalformedInput___java_nio_charset_CodingErrorAction]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "implOnMalformedInput", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 520)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CharsetDecoder_implOnUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_implOnUnmappableCharacter___java_nio_charset_CodingErrorAction]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "implOnUnmappableCharacter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 532)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CharsetDecoder_implReplaceWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_implReplaceWith___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "implReplaceWith", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 543)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_CharsetDecoder_implReset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_implReset__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "implReset", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 551)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CharsetDecoder_isAutoDetecting__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_isAutoDetecting__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "isAutoDetecting", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 560)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_CharsetDecoder_isCharsetDetected__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_isCharsetDetected__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "isCharsetDetected", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 588)

    
    // Red class access removed: java.lang.UnsupportedOperationException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.UnsupportedOperationException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_malformedInputAction__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_malformedInputAction__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "malformedInputAction", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 599)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.malformAction_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_nio_charset_CharsetDecoder_maxCharsPerByte__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_maxCharsPerByte__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "maxCharsPerByte", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 610)
    XMLVM_CHECK_NPE(1)
    _r0.f = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.maxChars_;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_onMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_onMalformedInput___java_nio_charset_CodingErrorAction]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "onMalformedInput", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 627)
    if (_r2.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 628)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 630)
    XMLVM_CHECK_NPE(1)
    ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.malformAction_ = _r2.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 631)
    XMLVM_CHECK_NPE(1)
    java_nio_charset_CharsetDecoder_implOnMalformedInput___java_nio_charset_CodingErrorAction(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 632)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "onUnmappableCharacter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 650)
    if (_r2.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 651)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 653)
    XMLVM_CHECK_NPE(1)
    ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.unmapAction_ = _r2.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 654)
    XMLVM_CHECK_NPE(1)
    java_nio_charset_CharsetDecoder_implOnUnmappableCharacter___java_nio_charset_CodingErrorAction(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 655)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_replacement__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_replacement__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "replacement", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 664)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.replace_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_replaceWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_replaceWith___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "replaceWith", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 684)
    if (_r3.o == JAVA_NULL) goto label8;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    if (_r0.i != 0) goto label20;
    label8:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 686)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "niochar.06"
    _r1.o = xmlvm_create_java_string_from_pool(725);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label20:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 688)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r3.o)->tib->vtable[8])(_r3.o);
    _r0.f = (JAVA_FLOAT) _r0.i;
    XMLVM_CHECK_NPE(2)
    _r1.f = ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.maxChars_;
    _r0.i = _r0.f > _r1.f ? 1 : (_r0.f == _r1.f ? 0 : -1);
    if (_r0.i <= 0) goto label43;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 691)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "niochar.07"
    _r1.o = xmlvm_create_java_string_from_pool(726);

    
    // Red class access removed: org.apache.harmony.niochar.internal.nls.Messages::getString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label43:;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 693)
    XMLVM_CHECK_NPE(2)
    ((java_nio_charset_CharsetDecoder*) _r2.o)->fields.java_nio_charset_CharsetDecoder.replace_ = _r3.o;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 694)
    XMLVM_CHECK_NPE(2)
    java_nio_charset_CharsetDecoder_implReplaceWith___java_lang_String(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 695)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_reset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_reset__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "reset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 706)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.status_ = _r0.i;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 707)
    XMLVM_CHECK_NPE(1)
    java_nio_charset_CharsetDecoder_implReset__(_r1.o);
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 708)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_CharsetDecoder_unmappableCharacterAction__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_CharsetDecoder_unmappableCharacterAction__]
    XMLVM_ENTER_METHOD("java.nio.charset.CharsetDecoder", "unmappableCharacterAction", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetDecoder.java", 719)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_CharsetDecoder*) _r1.o)->fields.java_nio_charset_CharsetDecoder.unmapAction_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

