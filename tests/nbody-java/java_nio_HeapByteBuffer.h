#ifndef __JAVA_NIO_HEAPBYTEBUFFER__
#define __JAVA_NIO_HEAPBYTEBUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_HeapByteBuffer 8
// Implemented interfaces:
// Super Class:
#include "java_nio_ByteBuffer.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
XMLVM_FORWARD_DECL(java_lang_Double)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
XMLVM_FORWARD_DECL(java_lang_Float)
#endif
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
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
XMLVM_FORWARD_DECL(org_apache_harmony_luni_platform_Endianness)
#endif
// Class declarations for java.nio.HeapByteBuffer
XMLVM_DEFINE_CLASS(java_nio_HeapByteBuffer, 55, XMLVM_ITABLE_SIZE_java_nio_HeapByteBuffer)

extern JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer;
extern JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_HeapByteBuffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_HeapByteBuffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_HeapByteBuffer \
    __INSTANCE_FIELDS_java_nio_ByteBuffer; \
    struct { \
        JAVA_OBJECT backingArray_; \
        JAVA_INT offset_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_HeapByteBuffer \
    } java_nio_HeapByteBuffer

struct java_nio_HeapByteBuffer {
    __TIB_DEFINITION_java_nio_HeapByteBuffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_HeapByteBuffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_HeapByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_HeapByteBuffer
typedef struct java_nio_HeapByteBuffer java_nio_HeapByteBuffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_HeapByteBuffer 55
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_get___byte_1ARRAY_int_int 34
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_get__ 33
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_get___int 35
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getDouble__ 23
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getDouble___int 24
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getFloat__ 25
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getFloat___int 26
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getInt__ 27
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getInt___int 28
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getLong__ 29
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getLong___int 30
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getShort__ 31
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getShort___int 32
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_isDirect__ 9
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asCharBuffer__ 11
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asDoubleBuffer__ 12
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asFloatBuffer__ 13
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asIntBuffer__ 14
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asLongBuffer__ 15
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_asShortBuffer__ 17
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getChar__ 21
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_getChar___int 22
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_putChar___char 39
#define XMLVM_VTABLE_IDX_java_nio_HeapByteBuffer_putChar___int_char 40

void __INIT_java_nio_HeapByteBuffer();
void __INIT_IMPL_java_nio_HeapByteBuffer();
void __DELETE_java_nio_HeapByteBuffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_HeapByteBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_HeapByteBuffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_HeapByteBuffer();
void java_nio_HeapByteBuffer___INIT____byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_nio_HeapByteBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_nio_HeapByteBuffer___INIT____byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 34
JAVA_OBJECT java_nio_HeapByteBuffer_get___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 33
JAVA_BYTE java_nio_HeapByteBuffer_get__(JAVA_OBJECT me);
// Vtable index: 35
JAVA_BYTE java_nio_HeapByteBuffer_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 23
JAVA_DOUBLE java_nio_HeapByteBuffer_getDouble__(JAVA_OBJECT me);
// Vtable index: 24
JAVA_DOUBLE java_nio_HeapByteBuffer_getDouble___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 25
JAVA_FLOAT java_nio_HeapByteBuffer_getFloat__(JAVA_OBJECT me);
// Vtable index: 26
JAVA_FLOAT java_nio_HeapByteBuffer_getFloat___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 27
JAVA_INT java_nio_HeapByteBuffer_getInt__(JAVA_OBJECT me);
// Vtable index: 28
JAVA_INT java_nio_HeapByteBuffer_getInt___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 29
JAVA_LONG java_nio_HeapByteBuffer_getLong__(JAVA_OBJECT me);
// Vtable index: 30
JAVA_LONG java_nio_HeapByteBuffer_getLong___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 31
JAVA_SHORT java_nio_HeapByteBuffer_getShort__(JAVA_OBJECT me);
// Vtable index: 32
JAVA_SHORT java_nio_HeapByteBuffer_getShort___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 9
JAVA_BOOLEAN java_nio_HeapByteBuffer_isDirect__(JAVA_OBJECT me);
JAVA_INT java_nio_HeapByteBuffer_loadInt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_LONG java_nio_HeapByteBuffer_loadLong___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_SHORT java_nio_HeapByteBuffer_loadShort___int(JAVA_OBJECT me, JAVA_INT n1);
void java_nio_HeapByteBuffer_store___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
void java_nio_HeapByteBuffer_store___int_long(JAVA_OBJECT me, JAVA_INT n1, JAVA_LONG n2);
void java_nio_HeapByteBuffer_store___int_short(JAVA_OBJECT me, JAVA_INT n1, JAVA_SHORT n2);
// Vtable index: 11
JAVA_OBJECT java_nio_HeapByteBuffer_asCharBuffer__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_nio_HeapByteBuffer_asDoubleBuffer__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_OBJECT java_nio_HeapByteBuffer_asFloatBuffer__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_nio_HeapByteBuffer_asIntBuffer__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_OBJECT java_nio_HeapByteBuffer_asLongBuffer__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_nio_HeapByteBuffer_asShortBuffer__(JAVA_OBJECT me);
// Vtable index: 21
JAVA_CHAR java_nio_HeapByteBuffer_getChar__(JAVA_OBJECT me);
// Vtable index: 22
JAVA_CHAR java_nio_HeapByteBuffer_getChar___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 39
JAVA_OBJECT java_nio_HeapByteBuffer_putChar___char(JAVA_OBJECT me, JAVA_CHAR n1);
// Vtable index: 40
JAVA_OBJECT java_nio_HeapByteBuffer_putChar___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);

#endif
