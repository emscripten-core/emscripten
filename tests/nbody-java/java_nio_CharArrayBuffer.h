#ifndef __JAVA_NIO_CHARARRAYBUFFER__
#define __JAVA_NIO_CHARARRAYBUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_CharArrayBuffer 9
// Implemented interfaces:
// Super Class:
#include "java_nio_CharBuffer.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
// Class declarations for java.nio.CharArrayBuffer
XMLVM_DEFINE_CLASS(java_nio_CharArrayBuffer, 33, XMLVM_ITABLE_SIZE_java_nio_CharArrayBuffer)

extern JAVA_OBJECT __CLASS_java_nio_CharArrayBuffer;
extern JAVA_OBJECT __CLASS_java_nio_CharArrayBuffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_CharArrayBuffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_CharArrayBuffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_CharArrayBuffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_CharArrayBuffer \
    __INSTANCE_FIELDS_java_nio_CharBuffer; \
    struct { \
        JAVA_OBJECT backingArray_; \
        JAVA_INT offset_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_CharArrayBuffer \
    } java_nio_CharArrayBuffer

struct java_nio_CharArrayBuffer {
    __TIB_DEFINITION_java_nio_CharArrayBuffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_CharArrayBuffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_CharArrayBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharArrayBuffer
typedef struct java_nio_CharArrayBuffer java_nio_CharArrayBuffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_CharArrayBuffer 33
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_get__ 19
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_get___int 21
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_get___char_1ARRAY_int_int 20
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_isDirect__ 9
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_order__ 23
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_subSequence___int_int 32
#define XMLVM_VTABLE_IDX_java_nio_CharArrayBuffer_toString__ 5

void __INIT_java_nio_CharArrayBuffer();
void __INIT_IMPL_java_nio_CharArrayBuffer();
void __DELETE_java_nio_CharArrayBuffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_CharArrayBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_CharArrayBuffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_CharArrayBuffer();
void java_nio_CharArrayBuffer___INIT____char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_CharArrayBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_nio_CharArrayBuffer___INIT____int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3);
// Vtable index: 19
JAVA_CHAR java_nio_CharArrayBuffer_get__(JAVA_OBJECT me);
// Vtable index: 21
JAVA_CHAR java_nio_CharArrayBuffer_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 20
JAVA_OBJECT java_nio_CharArrayBuffer_get___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 9
JAVA_BOOLEAN java_nio_CharArrayBuffer_isDirect__(JAVA_OBJECT me);
// Vtable index: 23
JAVA_OBJECT java_nio_CharArrayBuffer_order__(JAVA_OBJECT me);
// Vtable index: 32
JAVA_OBJECT java_nio_CharArrayBuffer_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 5
JAVA_OBJECT java_nio_CharArrayBuffer_toString__(JAVA_OBJECT me);

#endif
