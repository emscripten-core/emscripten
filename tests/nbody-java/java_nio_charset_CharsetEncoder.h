#ifndef __JAVA_NIO_CHARSET_CHARSETENCODER__
#define __JAVA_NIO_CHARSET_CHARSETENCODER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_CharsetEncoder 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ByteBuffer
XMLVM_FORWARD_DECL(java_nio_ByteBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CharsetDecoder
#define XMLVM_FORWARD_DECL_java_nio_charset_CharsetDecoder
XMLVM_FORWARD_DECL(java_nio_charset_CharsetDecoder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
#define XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
XMLVM_FORWARD_DECL(java_nio_charset_CoderResult)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
#define XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
XMLVM_FORWARD_DECL(java_nio_charset_CodingErrorAction)
#endif
// Class declarations for java.nio.charset.CharsetEncoder
XMLVM_DEFINE_CLASS(java_nio_charset_CharsetEncoder, 7, XMLVM_ITABLE_SIZE_java_nio_charset_CharsetEncoder)

extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetEncoder;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetEncoder_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetEncoder_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetEncoder_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CharsetEncoder
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_CharsetEncoder \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT cs_; \
        JAVA_FLOAT averBytes_; \
        JAVA_FLOAT maxBytes_; \
        JAVA_OBJECT replace_; \
        JAVA_INT status_; \
        JAVA_BOOLEAN finished_; \
        JAVA_OBJECT malformAction_; \
        JAVA_OBJECT unmapAction_; \
        JAVA_OBJECT decoder_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CharsetEncoder \
    } java_nio_charset_CharsetEncoder

struct java_nio_charset_CharsetEncoder {
    __TIB_DEFINITION_java_nio_charset_CharsetEncoder* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_CharsetEncoder;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CharsetEncoder
#define XMLVM_FORWARD_DECL_java_nio_charset_CharsetEncoder
typedef struct java_nio_charset_CharsetEncoder java_nio_charset_CharsetEncoder;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_CharsetEncoder 7
#define XMLVM_VTABLE_IDX_java_nio_charset_CharsetEncoder_encodeLoop___java_nio_CharBuffer_java_nio_ByteBuffer 6

void __INIT_java_nio_charset_CharsetEncoder();
void __INIT_IMPL_java_nio_charset_CharsetEncoder();
void __DELETE_java_nio_charset_CharsetEncoder(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetEncoder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_CharsetEncoder();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CharsetEncoder();
JAVA_INT java_nio_charset_CharsetEncoder_GET_READY();
void java_nio_charset_CharsetEncoder_PUT_READY(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetEncoder_GET_ONGOING();
void java_nio_charset_CharsetEncoder_PUT_ONGOING(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetEncoder_GET_END();
void java_nio_charset_CharsetEncoder_PUT_END(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetEncoder_GET_FLUSH();
void java_nio_charset_CharsetEncoder_PUT_FLUSH(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetEncoder_GET_INIT();
void java_nio_charset_CharsetEncoder_PUT_INIT(JAVA_INT v);
void java_nio_charset_CharsetEncoder___INIT____java_nio_charset_Charset_float_float(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2, JAVA_FLOAT n3);
void java_nio_charset_CharsetEncoder___INIT____java_nio_charset_Charset_float_float_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2, JAVA_FLOAT n3, JAVA_OBJECT n4);
JAVA_FLOAT java_nio_charset_CharsetEncoder_averageBytesPerChar__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CharsetEncoder_canEncode___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_BOOLEAN java_nio_charset_CharsetEncoder_implCanEncode___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_nio_charset_CharsetEncoder_canEncode___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_charset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetEncoder_encode___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetEncoder_checkCoderResult___java_nio_charset_CoderResult(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_allocateMore___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_encode___java_nio_CharBuffer_java_nio_ByteBuffer_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3);
// Vtable index: 6
JAVA_OBJECT java_nio_charset_CharsetEncoder_encodeLoop___java_nio_CharBuffer_java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_nio_charset_CharsetEncoder_flush___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_implFlush___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetEncoder_implOnMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetEncoder_implOnUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetEncoder_implReplaceWith___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetEncoder_implReset__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CharsetEncoder_isLegalReplacement___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_malformedInputAction__(JAVA_OBJECT me);
JAVA_FLOAT java_nio_charset_CharsetEncoder_maxBytesPerChar__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetEncoder_onMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_replacement__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetEncoder_replaceWith___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetEncoder_reset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetEncoder_unmappableCharacterAction__(JAVA_OBJECT me);

#endif
