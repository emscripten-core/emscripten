
#include "xmlvm.h"
#include "java_lang_reflect_Field.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include "java_lang_Boolean.h"
#include "java_lang_Byte.h"
#include "java_lang_Character.h"
#include "java_lang_Short.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"

#define VALUE_PTR(THIZ, OBJECT)\
(thiz->fields.java_lang_reflect_Field.modifiers_ & java_lang_reflect_Modifier_STATIC)?\
(char*) THIZ->fields.java_lang_reflect_Field.address_:\
((char*) OBJECT) + THIZ->fields.java_lang_reflect_Field.offset_

//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_OBJECT java_lang_reflect_Field_getSignature__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getSignature__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Field_isSynthetic__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_isSynthetic__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_toGenericString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_toGenericString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Field_isEnumConstant__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_isEnumConstant__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_getGenericType__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getGenericType__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Field_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_equals___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_get___java_lang_Object]
    java_lang_reflect_Field* thiz = (java_lang_reflect_Field*) me;
    JAVA_OBJECT type = thiz->fields.java_lang_reflect_Field.type_;
    char* valuePtr;
    if (thiz->fields.java_lang_reflect_Field.modifiers_ & java_lang_reflect_Modifier_STATIC) {
        valuePtr = (char*) thiz->fields.java_lang_reflect_Field.address_;
    } else {
        valuePtr = ((char*) n1) + thiz->fields.java_lang_reflect_Field.offset_;
    }
    JAVA_OBJECT obj;
    if (type == __CLASS_boolean) {
        JAVA_BOOLEAN value = *((JAVA_BOOLEAN*) valuePtr);
        obj = __NEW_java_lang_Boolean();
        java_lang_Boolean___INIT____boolean(obj, value);
    } else if (type == __CLASS_byte) {
        JAVA_BYTE value = *((JAVA_BYTE*) valuePtr);
        obj = __NEW_java_lang_Byte();
        java_lang_Byte___INIT____byte(obj, value);
    } else if (type == __CLASS_char) {
        JAVA_CHAR value = *((JAVA_CHAR*) valuePtr);
        obj = __NEW_java_lang_Character();
        java_lang_Character___INIT____char(obj, value);
    } else if (type == __CLASS_short) {
        JAVA_SHORT value = *((JAVA_SHORT*) valuePtr);
        obj = __NEW_java_lang_Short();
        java_lang_Short___INIT____short(obj, value);
    } else if (type == __CLASS_int) {
        JAVA_INT value = *((JAVA_INT*) valuePtr);
        obj = __NEW_java_lang_Integer();
        java_lang_Integer___INIT____int(obj, value);
    } else if (type == __CLASS_long) {
        JAVA_LONG value = *((JAVA_LONG*) valuePtr);
        obj = __NEW_java_lang_Long();
        java_lang_Long___INIT____long(obj, value);
    } else if (type == __CLASS_float) {
        JAVA_FLOAT value = *((JAVA_FLOAT*) valuePtr);
        obj = __NEW_java_lang_Float();
        java_lang_Float___INIT____float(obj, value);
    } else if (type == __CLASS_double) {
        JAVA_DOUBLE value = *((JAVA_DOUBLE*) valuePtr);
        obj = __NEW_java_lang_Double();
        java_lang_Double___INIT____double(obj, value);
    } else {
        // Has to be object type if it is not a primitive type
        obj = *((JAVA_OBJECT*) valuePtr);
    }
    return obj;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Field_getBoolean___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getBoolean___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BYTE java_lang_reflect_Field_getByte___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getByte___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_CHAR java_lang_reflect_Field_getChar___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getChar___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_getDeclaringClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getDeclaringClass__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_DOUBLE java_lang_reflect_Field_getDouble___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getDouble___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_FLOAT java_lang_reflect_Field_getFloat___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getFloat___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Field_getInt___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getInt___java_lang_Object]
    java_lang_reflect_Field* thiz = (java_lang_reflect_Field*) me;
    JAVA_OBJECT type = thiz->fields.java_lang_reflect_Field.type_;
    char* valuePtr=VALUE_PTR(thiz, n1);
    if (type == __CLASS_int) {
        JAVA_INT value = *((JAVA_INT*) valuePtr);
        return value;
    }
    XMLVM_ERROR("bad type", __FILE__, __FUNCTION__, __LINE__);
    return 0;
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_reflect_Field_getLong___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getLong___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getName__]
    java_lang_reflect_Field* thiz = (java_lang_reflect_Field*) me;
    return thiz->fields.java_lang_reflect_Field.name_;
    //XMLVM_END_NATIVE
}

JAVA_SHORT java_lang_reflect_Field_getShort___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getShort___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_getType__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_getType__]
    java_lang_reflect_Field* thiz = (java_lang_reflect_Field*) me;
    return thiz->fields.java_lang_reflect_Field.type_;
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Field_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_hashCode__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_set___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_set___java_lang_Object_java_lang_Object]
    java_lang_reflect_Field* thiz = (java_lang_reflect_Field*) me;
    JAVA_OBJECT type = thiz->fields.java_lang_reflect_Field.type_;
    char* valuePtr=VALUE_PTR(thiz, n2);
    memcpy(valuePtr, &n1, 4);
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setBoolean___java_lang_Object_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setBoolean___java_lang_Object_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setByte___java_lang_Object_byte(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BYTE n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setByte___java_lang_Object_byte]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setChar___java_lang_Object_char(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setChar___java_lang_Object_char]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setDouble___java_lang_Object_double(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_DOUBLE n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setDouble___java_lang_Object_double]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setFloat___java_lang_Object_float(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setFloat___java_lang_Object_float]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setInt___java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setInt___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setLong___java_lang_Object_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setLong___java_lang_Object_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Field_setShort___java_lang_Object_short(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_SHORT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_setShort___java_lang_Object_short]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Field_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Field_toString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_java_lang_reflect_Field()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getSignature__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getSignature__] = 
        (VTABLE_PTR) java_lang_reflect_Field_getSignature__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_isSynthetic__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_isSynthetic__] = 
        (VTABLE_PTR) java_lang_reflect_Field_isSynthetic__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_toGenericString__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_toGenericString__] = 
        (VTABLE_PTR) java_lang_reflect_Field_toGenericString__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_isEnumConstant__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_isEnumConstant__] = 
        (VTABLE_PTR) java_lang_reflect_Field_isEnumConstant__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getGenericType__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getGenericType__] = 
        (VTABLE_PTR) java_lang_reflect_Field_getGenericType__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_equals___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_equals___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_equals___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_get___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_get___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_get___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getBoolean___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getBoolean___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getBoolean___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getByte___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getByte___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getByte___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getChar___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getChar___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getChar___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getDeclaringClass__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getDeclaringClass__] = 
        (VTABLE_PTR) java_lang_reflect_Field_getDeclaringClass__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getDouble___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getDouble___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getDouble___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getFloat___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getFloat___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getFloat___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getInt___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getInt___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getInt___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getLong___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getLong___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getLong___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getName__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getName__] = 
        (VTABLE_PTR) java_lang_reflect_Field_getName__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getShort___java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getShort___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_getShort___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_getType__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_getType__] = 
        (VTABLE_PTR) java_lang_reflect_Field_getType__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_hashCode__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_hashCode__] = 
        (VTABLE_PTR) java_lang_reflect_Field_hashCode__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_set___java_lang_Object_java_lang_Object
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_set___java_lang_Object_java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Field_set___java_lang_Object_java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setBoolean___java_lang_Object_boolean
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setBoolean___java_lang_Object_boolean] = 
        (VTABLE_PTR) java_lang_reflect_Field_setBoolean___java_lang_Object_boolean;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setByte___java_lang_Object_byte
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setByte___java_lang_Object_byte] = 
        (VTABLE_PTR) java_lang_reflect_Field_setByte___java_lang_Object_byte;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setChar___java_lang_Object_char
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setChar___java_lang_Object_char] = 
        (VTABLE_PTR) java_lang_reflect_Field_setChar___java_lang_Object_char;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setDouble___java_lang_Object_double
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setDouble___java_lang_Object_double] = 
        (VTABLE_PTR) java_lang_reflect_Field_setDouble___java_lang_Object_double;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setFloat___java_lang_Object_float
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setFloat___java_lang_Object_float] = 
        (VTABLE_PTR) java_lang_reflect_Field_setFloat___java_lang_Object_float;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setInt___java_lang_Object_int
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setInt___java_lang_Object_int] = 
        (VTABLE_PTR) java_lang_reflect_Field_setInt___java_lang_Object_int;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setLong___java_lang_Object_long
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setLong___java_lang_Object_long] = 
        (VTABLE_PTR) java_lang_reflect_Field_setLong___java_lang_Object_long;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_setShort___java_lang_Object_short
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_setShort___java_lang_Object_short] = 
        (VTABLE_PTR) java_lang_reflect_Field_setShort___java_lang_Object_short;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Field_toString__
    __TIB_java_lang_reflect_Field.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Field_toString__] = 
        (VTABLE_PTR) java_lang_reflect_Field_toString__;
#endif
}
