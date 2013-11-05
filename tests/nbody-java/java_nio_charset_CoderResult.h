#ifndef __JAVA_NIO_CHARSET_CODERRESULT__
#define __JAVA_NIO_CHARSET_CODERRESULT__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_CoderResult 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_WeakHashMap
#define XMLVM_FORWARD_DECL_java_util_WeakHashMap
XMLVM_FORWARD_DECL(java_util_WeakHashMap)
#endif
// Class declarations for java.nio.charset.CoderResult
XMLVM_DEFINE_CLASS(java_nio_charset_CoderResult, 6, XMLVM_ITABLE_SIZE_java_nio_charset_CoderResult)

extern JAVA_OBJECT __CLASS_java_nio_charset_CoderResult;
extern JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CoderResult_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CoderResult
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_CoderResult \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_INT type_; \
        JAVA_INT length_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CoderResult \
    } java_nio_charset_CoderResult

struct java_nio_charset_CoderResult {
    __TIB_DEFINITION_java_nio_charset_CoderResult* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_CoderResult;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
#define XMLVM_FORWARD_DECL_java_nio_charset_CoderResult
typedef struct java_nio_charset_CoderResult java_nio_charset_CoderResult;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_CoderResult 6
#define XMLVM_VTABLE_IDX_java_nio_charset_CoderResult_toString__ 5

void __INIT_java_nio_charset_CoderResult();
void __INIT_IMPL_java_nio_charset_CoderResult();
void __DELETE_java_nio_charset_CoderResult(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_CoderResult(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_CoderResult();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CoderResult();
JAVA_INT java_nio_charset_CoderResult_GET_TYPE_UNDERFLOW();
void java_nio_charset_CoderResult_PUT_TYPE_UNDERFLOW(JAVA_INT v);
JAVA_INT java_nio_charset_CoderResult_GET_TYPE_OVERFLOW();
void java_nio_charset_CoderResult_PUT_TYPE_OVERFLOW(JAVA_INT v);
JAVA_INT java_nio_charset_CoderResult_GET_TYPE_MALFORMED_INPUT();
void java_nio_charset_CoderResult_PUT_TYPE_MALFORMED_INPUT(JAVA_INT v);
JAVA_INT java_nio_charset_CoderResult_GET_TYPE_UNMAPPABLE_CHAR();
void java_nio_charset_CoderResult_PUT_TYPE_UNMAPPABLE_CHAR(JAVA_INT v);
JAVA_OBJECT java_nio_charset_CoderResult_GET_UNDERFLOW();
void java_nio_charset_CoderResult_PUT_UNDERFLOW(JAVA_OBJECT v);
JAVA_OBJECT java_nio_charset_CoderResult_GET_OVERFLOW();
void java_nio_charset_CoderResult_PUT_OVERFLOW(JAVA_OBJECT v);
JAVA_OBJECT java_nio_charset_CoderResult_GET__malformedErrors();
void java_nio_charset_CoderResult_PUT__malformedErrors(JAVA_OBJECT v);
JAVA_OBJECT java_nio_charset_CoderResult_GET__unmappableErrors();
void java_nio_charset_CoderResult_PUT__unmappableErrors(JAVA_OBJECT v);
void java_nio_charset_CoderResult___CLINIT_();
void java_nio_charset_CoderResult___INIT____int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
JAVA_OBJECT java_nio_charset_CoderResult_malformedForLength___int(JAVA_INT n1);
JAVA_OBJECT java_nio_charset_CoderResult_unmappableForLength___int(JAVA_INT n1);
JAVA_BOOLEAN java_nio_charset_CoderResult_isUnderflow__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CoderResult_isError__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CoderResult_isMalformed__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CoderResult_isOverflow__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_charset_CoderResult_isUnmappable__(JAVA_OBJECT me);
JAVA_INT java_nio_charset_CoderResult_length__(JAVA_OBJECT me);
void java_nio_charset_CoderResult_throwException__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_nio_charset_CoderResult_toString__(JAVA_OBJECT me);

#endif
