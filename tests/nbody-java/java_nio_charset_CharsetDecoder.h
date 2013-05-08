#ifndef __JAVA_NIO_CHARSET_CHARSETDECODER__
#define __JAVA_NIO_CHARSET_CHARSETDECODER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_CharsetDecoder 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
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
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
#define XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
XMLVM_FORWARD_DECL(java_nio_charset_CoderResult)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
#define XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
XMLVM_FORWARD_DECL(java_nio_charset_CodingErrorAction)
#endif
// Class declarations for java.nio.charset.CharsetDecoder
XMLVM_DEFINE_CLASS(java_nio_charset_CharsetDecoder, 7, XMLVM_ITABLE_SIZE_java_nio_charset_CharsetDecoder)

extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CharsetDecoder_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CharsetDecoder
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_CharsetDecoder \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_FLOAT averChars_; \
        JAVA_FLOAT maxChars_; \
        JAVA_OBJECT cs_; \
        JAVA_OBJECT malformAction_; \
        JAVA_OBJECT unmapAction_; \
        JAVA_OBJECT replace_; \
        JAVA_INT status_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CharsetDecoder \
    } java_nio_charset_CharsetDecoder

struct java_nio_charset_CharsetDecoder {
    __TIB_DEFINITION_java_nio_charset_CharsetDecoder* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_CharsetDecoder;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CharsetDecoder
#define XMLVM_FORWARD_DECL_java_nio_charset_CharsetDecoder
typedef struct java_nio_charset_CharsetDecoder java_nio_charset_CharsetDecoder;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_CharsetDecoder 7
#define XMLVM_VTABLE_IDX_java_nio_charset_CharsetDecoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer 6

void __INIT_java_nio_charset_CharsetDecoder();
void __INIT_IMPL_java_nio_charset_CharsetDecoder();
void __DELETE_java_nio_charset_CharsetDecoder(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_CharsetDecoder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_CharsetDecoder();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CharsetDecoder();
JAVA_INT java_nio_charset_CharsetDecoder_GET_INIT();
void java_nio_charset_CharsetDecoder_PUT_INIT(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetDecoder_GET_ONGOING();
void java_nio_charset_CharsetDecoder_PUT_ONGOING(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetDecoder_GET_END();
void java_nio_charset_CharsetDecoder_PUT_END(JAVA_INT v);
JAVA_INT java_nio_charset_CharsetDecoder_GET_FLUSH();
void java_nio_charset_CharsetDecoder_PUT_FLUSH(JAVA_INT v);
void java_nio_charset_CharsetDecoder___INIT____java_nio_charset_Charset_float_float(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2, JAVA_FLOAT n3);
JAVA_FLOAT java_nio_charset_CharsetDecoder_averageCharsPerByte__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_charset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetDecoder_checkCoderResult___java_nio_charset_CoderResult(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_allocateMore___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer_java_nio_CharBuffer_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3);
// Vtable index: 6
JAVA_OBJECT java_nio_charset_CharsetDecoder_decodeLoop___java_nio_ByteBuffer_java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_nio_charset_CharsetDecoder_detectedCharset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_flush___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_implFlush___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetDecoder_implOnMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetDecoder_implOnUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetDecoder_implReplaceWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_charset_CharsetDecoder_implReset__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CharsetDecoder_isAutoDetecting__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CharsetDecoder_isCharsetDetected__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_malformedInputAction__(JAVA_OBJECT me);
JAVA_FLOAT java_nio_charset_CharsetDecoder_maxCharsPerByte__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_onMalformedInput___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_replacement__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_replaceWith___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_charset_CharsetDecoder_reset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_charset_CharsetDecoder_unmappableCharacterAction__(JAVA_OBJECT me);

#endif
