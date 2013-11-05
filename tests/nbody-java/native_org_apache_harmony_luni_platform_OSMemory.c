
#include "xmlvm.h"
#include "org_apache_harmony_luni_platform_OSMemory.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLittleEndianImpl__()
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_isLittleEndianImpl__]
    struct {
        int  x;
        char c[sizeof(int)];
    } u;
    u.x = 1;
    return u.c[0] != 0;
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_getPointerSizeImpl__()
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getPointerSizeImpl__]
    return sizeof(JAVA_OBJECT);
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_malloc___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_malloc___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_free___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_free___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_BYTE n2, JAVA_LONG n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BYTE org_apache_harmony_luni_platform_OSMemory_getByte___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getByte___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setByte___long_byte(JAVA_OBJECT me, JAVA_LONG n1, JAVA_BYTE n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setByte___long_byte]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_SHORT org_apache_harmony_luni_platform_OSMemory_getShort___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getShort___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setShort___long_short(JAVA_OBJECT me, JAVA_LONG n1, JAVA_SHORT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setShort___long_short]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_getInt___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getInt___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setInt___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setInt___long_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_getLong___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getLong___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setLong___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setLong___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_FLOAT org_apache_harmony_luni_platform_OSMemory_getFloat___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getFloat___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setFloat___long_float(JAVA_OBJECT me, JAVA_LONG n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setFloat___long_float]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_DOUBLE org_apache_harmony_luni_platform_OSMemory_getDouble___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getDouble___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setDouble___long_double(JAVA_OBJECT me, JAVA_LONG n1, JAVA_DOUBLE n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setDouble___long_double]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_getAddress___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_getAddress___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_setAddress___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_setAddress___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_mmapImpl___long_long_long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_mmapImpl___long_long_long_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_apache_harmony_luni_platform_OSMemory_unmapImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_unmapImpl___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_loadImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_loadImpl___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLoadedImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_isLoadedImpl___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_flushImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSMemory_flushImpl___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_apache_harmony_luni_platform_OSMemory()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_malloc___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_malloc___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_malloc___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_free___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_free___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_free___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getByte___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getByte___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getByte___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setByte___long_byte
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setByte___long_byte] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setByte___long_byte;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getShort___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getShort___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getShort___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setShort___long_short
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setShort___long_short] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setShort___long_short;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getInt___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getInt___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getInt___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setInt___long_int
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setInt___long_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setInt___long_int;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getLong___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getLong___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getLong___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setLong___long_long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setLong___long_long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setLong___long_long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getFloat___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getFloat___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getFloat___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setFloat___long_float
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setFloat___long_float] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setFloat___long_float;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getDouble___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getDouble___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getDouble___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setDouble___long_double
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setDouble___long_double] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setDouble___long_double;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getAddress___long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_getAddress___long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_getAddress___long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setAddress___long_long
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSMemory_setAddress___long_long] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSMemory_setAddress___long_long;
#endif
}
