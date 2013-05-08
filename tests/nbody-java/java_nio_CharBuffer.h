#ifndef __JAVA_NIO_CHARBUFFER__
#define __JAVA_NIO_CHARBUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_CharBuffer 9
// Implemented interfaces:
#include "java_lang_Appendable.h"
#include "java_lang_CharSequence.h"
#include "java_lang_Comparable.h"
#include "java_lang_Readable.h"
// Super Class:
#include "java_nio_Buffer.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Math
#define XMLVM_FORWARD_DECL_java_lang_Math
XMLVM_FORWARD_DECL(java_lang_Math)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_BufferFactory
#define XMLVM_FORWARD_DECL_java_nio_BufferFactory
XMLVM_FORWARD_DECL(java_nio_BufferFactory)
#endif
// Class declarations for java.nio.CharBuffer
XMLVM_DEFINE_CLASS(java_nio_CharBuffer, 33, XMLVM_ITABLE_SIZE_java_nio_CharBuffer)

extern JAVA_OBJECT __CLASS_java_nio_CharBuffer;
extern JAVA_OBJECT __CLASS_java_nio_CharBuffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_CharBuffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_CharBuffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_CharBuffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_CharBuffer \
    __INSTANCE_FIELDS_java_nio_Buffer; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_CharBuffer \
    } java_nio_CharBuffer

struct java_nio_CharBuffer {
    __TIB_DEFINITION_java_nio_CharBuffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_CharBuffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
typedef struct java_nio_CharBuffer java_nio_CharBuffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_CharBuffer 33
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_array__ 7
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_arrayOffset__ 6
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_asReadOnlyBuffer__ 14
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_charAt___int 15
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_compact__ 16
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_duplicate__ 18
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_get__ 19
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_get___char_1ARRAY_int_int 20
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_get___int 21
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_hasArray__ 8
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_isDirect__ 9
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_length__ 22
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_order__ 23
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_protectedArray__ 25
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_protectedArrayOffset__ 24
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_protectedHasArray__ 26
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_put___char 27
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_put___char_1ARRAY_int_int 28
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_put___int_char 29
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_slice__ 31
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_subSequence___int_int 32
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_toString__ 5
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_append___char 11
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_append___java_lang_CharSequence 12
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_append___java_lang_CharSequence_int_int 13
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_read___java_nio_CharBuffer 30
#define XMLVM_VTABLE_IDX_java_nio_CharBuffer_compareTo___java_lang_Object 17

void __INIT_java_nio_CharBuffer();
void __INIT_IMPL_java_nio_CharBuffer();
void __DELETE_java_nio_CharBuffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_CharBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_CharBuffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_CharBuffer();
JAVA_OBJECT java_nio_CharBuffer_allocate___int(JAVA_INT n1);
JAVA_OBJECT java_nio_CharBuffer_wrap___char_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_CharBuffer_wrap___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_nio_CharBuffer_wrap___java_lang_CharSequence(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_CharBuffer_wrap___java_lang_CharSequence_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_nio_CharBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 7
JAVA_OBJECT java_nio_CharBuffer_array__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_INT java_nio_CharBuffer_arrayOffset__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_nio_CharBuffer_asReadOnlyBuffer__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_CHAR java_nio_CharBuffer_charAt___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 16
JAVA_OBJECT java_nio_CharBuffer_compact__(JAVA_OBJECT me);
JAVA_INT java_nio_CharBuffer_compareTo___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 18
JAVA_OBJECT java_nio_CharBuffer_duplicate__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_nio_CharBuffer_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 19
JAVA_CHAR java_nio_CharBuffer_get__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_CharBuffer_get___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 20
JAVA_OBJECT java_nio_CharBuffer_get___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 21
JAVA_CHAR java_nio_CharBuffer_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 8
JAVA_BOOLEAN java_nio_CharBuffer_hasArray__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_nio_CharBuffer_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_BOOLEAN java_nio_CharBuffer_isDirect__(JAVA_OBJECT me);
// Vtable index: 22
JAVA_INT java_nio_CharBuffer_length__(JAVA_OBJECT me);
// Vtable index: 23
JAVA_OBJECT java_nio_CharBuffer_order__(JAVA_OBJECT me);
// Vtable index: 25
JAVA_OBJECT java_nio_CharBuffer_protectedArray__(JAVA_OBJECT me);
// Vtable index: 24
JAVA_INT java_nio_CharBuffer_protectedArrayOffset__(JAVA_OBJECT me);
// Vtable index: 26
JAVA_BOOLEAN java_nio_CharBuffer_protectedHasArray__(JAVA_OBJECT me);
// Vtable index: 27
JAVA_OBJECT java_nio_CharBuffer_put___char(JAVA_OBJECT me, JAVA_CHAR n1);
JAVA_OBJECT java_nio_CharBuffer_put___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 28
JAVA_OBJECT java_nio_CharBuffer_put___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_nio_CharBuffer_put___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 29
JAVA_OBJECT java_nio_CharBuffer_put___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
JAVA_OBJECT java_nio_CharBuffer_put___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_CharBuffer_put___java_lang_String_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 31
JAVA_OBJECT java_nio_CharBuffer_slice__(JAVA_OBJECT me);
// Vtable index: 32
JAVA_OBJECT java_nio_CharBuffer_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 5
JAVA_OBJECT java_nio_CharBuffer_toString__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_OBJECT java_nio_CharBuffer_append___char(JAVA_OBJECT me, JAVA_CHAR n1);
// Vtable index: 12
JAVA_OBJECT java_nio_CharBuffer_append___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 13
JAVA_OBJECT java_nio_CharBuffer_append___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 30
JAVA_INT java_nio_CharBuffer_read___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 17
JAVA_INT java_nio_CharBuffer_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
