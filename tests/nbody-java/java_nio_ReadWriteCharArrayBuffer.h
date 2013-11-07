#ifndef __JAVA_NIO_READWRITECHARARRAYBUFFER__
#define __JAVA_NIO_READWRITECHARARRAYBUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_ReadWriteCharArrayBuffer 9
// Implemented interfaces:
// Super Class:
#include "java_nio_CharArrayBuffer.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif
// Class declarations for java.nio.ReadWriteCharArrayBuffer
XMLVM_DEFINE_CLASS(java_nio_ReadWriteCharArrayBuffer, 33, XMLVM_ITABLE_SIZE_java_nio_ReadWriteCharArrayBuffer)

extern JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer;
extern JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_ReadWriteCharArrayBuffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_ReadWriteCharArrayBuffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_ReadWriteCharArrayBuffer \
    __INSTANCE_FIELDS_java_nio_CharArrayBuffer; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_ReadWriteCharArrayBuffer \
    } java_nio_ReadWriteCharArrayBuffer

struct java_nio_ReadWriteCharArrayBuffer {
    __TIB_DEFINITION_java_nio_ReadWriteCharArrayBuffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_ReadWriteCharArrayBuffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_ReadWriteCharArrayBuffer
#define XMLVM_FORWARD_DECL_java_nio_ReadWriteCharArrayBuffer
typedef struct java_nio_ReadWriteCharArrayBuffer java_nio_ReadWriteCharArrayBuffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_ReadWriteCharArrayBuffer 33
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_asReadOnlyBuffer__ 14
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_compact__ 16
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_duplicate__ 18
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_isReadOnly__ 10
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_protectedArray__ 25
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_protectedArrayOffset__ 24
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_protectedHasArray__ 26
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_put___char 27
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_put___int_char 29
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_put___char_1ARRAY_int_int 28
#define XMLVM_VTABLE_IDX_java_nio_ReadWriteCharArrayBuffer_slice__ 31

void __INIT_java_nio_ReadWriteCharArrayBuffer();
void __INIT_IMPL_java_nio_ReadWriteCharArrayBuffer();
void __DELETE_java_nio_ReadWriteCharArrayBuffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_ReadWriteCharArrayBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_ReadWriteCharArrayBuffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_ReadWriteCharArrayBuffer();
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_copy___java_nio_CharArrayBuffer_int(JAVA_OBJECT n1, JAVA_INT n2);
void java_nio_ReadWriteCharArrayBuffer___INIT____char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_ReadWriteCharArrayBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_nio_ReadWriteCharArrayBuffer___INIT____int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3);
// Vtable index: 14
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_asReadOnlyBuffer__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_compact__(JAVA_OBJECT me);
// Vtable index: 18
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_duplicate__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_nio_ReadWriteCharArrayBuffer_isReadOnly__(JAVA_OBJECT me);
// Vtable index: 25
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_protectedArray__(JAVA_OBJECT me);
// Vtable index: 24
JAVA_INT java_nio_ReadWriteCharArrayBuffer_protectedArrayOffset__(JAVA_OBJECT me);
// Vtable index: 26
JAVA_BOOLEAN java_nio_ReadWriteCharArrayBuffer_protectedHasArray__(JAVA_OBJECT me);
// Vtable index: 27
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___char(JAVA_OBJECT me, JAVA_CHAR n1);
// Vtable index: 29
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
// Vtable index: 28
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_put___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 31
JAVA_OBJECT java_nio_ReadWriteCharArrayBuffer_slice__(JAVA_OBJECT me);

#endif
