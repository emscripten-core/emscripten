#ifndef __JAVA_NIO_BYTEBUFFER__
#define __JAVA_NIO_BYTEBUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_ByteBuffer 8
// Implemented interfaces:
#include "java_lang_Comparable.h"
// Super Class:
#include "java_nio_Buffer.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
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
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_BufferFactory
#define XMLVM_FORWARD_DECL_java_nio_BufferFactory
XMLVM_FORWARD_DECL(java_nio_BufferFactory)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
XMLVM_FORWARD_DECL(org_apache_harmony_luni_platform_Endianness)
#endif
// Class declarations for java.nio.ByteBuffer
XMLVM_DEFINE_CLASS(java_nio_ByteBuffer, 55, XMLVM_ITABLE_SIZE_java_nio_ByteBuffer)

extern JAVA_OBJECT __CLASS_java_nio_ByteBuffer;
extern JAVA_OBJECT __CLASS_java_nio_ByteBuffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_ByteBuffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_ByteBuffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_ByteBuffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_ByteBuffer \
    __INSTANCE_FIELDS_java_nio_Buffer; \
    struct { \
        JAVA_OBJECT order_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_ByteBuffer \
    } java_nio_ByteBuffer

struct java_nio_ByteBuffer {
    __TIB_DEFINITION_java_nio_ByteBuffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_ByteBuffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_ByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ByteBuffer
typedef struct java_nio_ByteBuffer java_nio_ByteBuffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_ByteBuffer 55
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_array__ 7
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_arrayOffset__ 6
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asCharBuffer__ 11
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asDoubleBuffer__ 12
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asFloatBuffer__ 13
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asIntBuffer__ 14
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asLongBuffer__ 15
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asReadOnlyBuffer__ 16
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_asShortBuffer__ 17
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_compact__ 18
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_duplicate__ 20
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_get__ 33
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_get___byte_1ARRAY_int_int 34
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_get___int 35
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getChar__ 21
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getChar___int 22
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getDouble__ 23
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getDouble___int 24
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getFloat__ 25
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getFloat___int 26
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getInt__ 27
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getInt___int 28
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getLong__ 29
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getLong___int 30
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getShort__ 31
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_getShort___int 32
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_hasArray__ 8
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_isDirect__ 9
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_protectedArray__ 37
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_protectedArrayOffset__ 36
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_protectedHasArray__ 38
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_put___byte 51
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_put___byte_1ARRAY_int_int 52
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_put___int_byte 53
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putChar___char 39
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putChar___int_char 40
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putDouble___double 41
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putDouble___int_double 42
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putFloat___float 43
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putFloat___int_float 44
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putInt___int 45
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putInt___int_int 46
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putLong___long 48
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putLong___int_long 47
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putShort___short 50
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_putShort___int_short 49
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_slice__ 54
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_toString__ 5
#define XMLVM_VTABLE_IDX_java_nio_ByteBuffer_compareTo___java_lang_Object 19

void __INIT_java_nio_ByteBuffer();
void __INIT_IMPL_java_nio_ByteBuffer();
void __DELETE_java_nio_ByteBuffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_ByteBuffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_ByteBuffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_ByteBuffer();
JAVA_OBJECT java_nio_ByteBuffer_allocate___int(JAVA_INT n1);
JAVA_OBJECT java_nio_ByteBuffer_allocateDirect___int(JAVA_INT n1);
JAVA_OBJECT java_nio_ByteBuffer_wrap___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_ByteBuffer_wrap___byte_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_nio_ByteBuffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 7
JAVA_OBJECT java_nio_ByteBuffer_array__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_INT java_nio_ByteBuffer_arrayOffset__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_OBJECT java_nio_ByteBuffer_asCharBuffer__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_nio_ByteBuffer_asDoubleBuffer__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_OBJECT java_nio_ByteBuffer_asFloatBuffer__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_OBJECT java_nio_ByteBuffer_asIntBuffer__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_OBJECT java_nio_ByteBuffer_asLongBuffer__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_OBJECT java_nio_ByteBuffer_asReadOnlyBuffer__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_nio_ByteBuffer_asShortBuffer__(JAVA_OBJECT me);
// Vtable index: 18
JAVA_OBJECT java_nio_ByteBuffer_compact__(JAVA_OBJECT me);
JAVA_INT java_nio_ByteBuffer_compareTo___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 20
JAVA_OBJECT java_nio_ByteBuffer_duplicate__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_nio_ByteBuffer_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 33
JAVA_BYTE java_nio_ByteBuffer_get__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_ByteBuffer_get___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 34
JAVA_OBJECT java_nio_ByteBuffer_get___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 35
JAVA_BYTE java_nio_ByteBuffer_get___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 21
JAVA_CHAR java_nio_ByteBuffer_getChar__(JAVA_OBJECT me);
// Vtable index: 22
JAVA_CHAR java_nio_ByteBuffer_getChar___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 23
JAVA_DOUBLE java_nio_ByteBuffer_getDouble__(JAVA_OBJECT me);
// Vtable index: 24
JAVA_DOUBLE java_nio_ByteBuffer_getDouble___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 25
JAVA_FLOAT java_nio_ByteBuffer_getFloat__(JAVA_OBJECT me);
// Vtable index: 26
JAVA_FLOAT java_nio_ByteBuffer_getFloat___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 27
JAVA_INT java_nio_ByteBuffer_getInt__(JAVA_OBJECT me);
// Vtable index: 28
JAVA_INT java_nio_ByteBuffer_getInt___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 29
JAVA_LONG java_nio_ByteBuffer_getLong__(JAVA_OBJECT me);
// Vtable index: 30
JAVA_LONG java_nio_ByteBuffer_getLong___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 31
JAVA_SHORT java_nio_ByteBuffer_getShort__(JAVA_OBJECT me);
// Vtable index: 32
JAVA_SHORT java_nio_ByteBuffer_getShort___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 8
JAVA_BOOLEAN java_nio_ByteBuffer_hasArray__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_nio_ByteBuffer_hashCode__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_BOOLEAN java_nio_ByteBuffer_isDirect__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_ByteBuffer_order__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_ByteBuffer_order___java_nio_ByteOrder(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_nio_ByteBuffer_orderImpl___java_nio_ByteOrder(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 37
JAVA_OBJECT java_nio_ByteBuffer_protectedArray__(JAVA_OBJECT me);
// Vtable index: 36
JAVA_INT java_nio_ByteBuffer_protectedArrayOffset__(JAVA_OBJECT me);
// Vtable index: 38
JAVA_BOOLEAN java_nio_ByteBuffer_protectedHasArray__(JAVA_OBJECT me);
// Vtable index: 51
JAVA_OBJECT java_nio_ByteBuffer_put___byte(JAVA_OBJECT me, JAVA_BYTE n1);
JAVA_OBJECT java_nio_ByteBuffer_put___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 52
JAVA_OBJECT java_nio_ByteBuffer_put___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_nio_ByteBuffer_put___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 53
JAVA_OBJECT java_nio_ByteBuffer_put___int_byte(JAVA_OBJECT me, JAVA_INT n1, JAVA_BYTE n2);
// Vtable index: 39
JAVA_OBJECT java_nio_ByteBuffer_putChar___char(JAVA_OBJECT me, JAVA_CHAR n1);
// Vtable index: 40
JAVA_OBJECT java_nio_ByteBuffer_putChar___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2);
// Vtable index: 41
JAVA_OBJECT java_nio_ByteBuffer_putDouble___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
// Vtable index: 42
JAVA_OBJECT java_nio_ByteBuffer_putDouble___int_double(JAVA_OBJECT me, JAVA_INT n1, JAVA_DOUBLE n2);
// Vtable index: 43
JAVA_OBJECT java_nio_ByteBuffer_putFloat___float(JAVA_OBJECT me, JAVA_FLOAT n1);
// Vtable index: 44
JAVA_OBJECT java_nio_ByteBuffer_putFloat___int_float(JAVA_OBJECT me, JAVA_INT n1, JAVA_FLOAT n2);
// Vtable index: 45
JAVA_OBJECT java_nio_ByteBuffer_putInt___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 46
JAVA_OBJECT java_nio_ByteBuffer_putInt___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 48
JAVA_OBJECT java_nio_ByteBuffer_putLong___long(JAVA_OBJECT me, JAVA_LONG n1);
// Vtable index: 47
JAVA_OBJECT java_nio_ByteBuffer_putLong___int_long(JAVA_OBJECT me, JAVA_INT n1, JAVA_LONG n2);
// Vtable index: 50
JAVA_OBJECT java_nio_ByteBuffer_putShort___short(JAVA_OBJECT me, JAVA_SHORT n1);
// Vtable index: 49
JAVA_OBJECT java_nio_ByteBuffer_putShort___int_short(JAVA_OBJECT me, JAVA_INT n1, JAVA_SHORT n2);
// Vtable index: 54
JAVA_OBJECT java_nio_ByteBuffer_slice__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_nio_ByteBuffer_toString__(JAVA_OBJECT me);
// Vtable index: 19
JAVA_INT java_nio_ByteBuffer_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
