
#include "xmlvm.h"
#include "java_lang_reflect_Array.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION

#include "java_lang_Class.h"

//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_OBJECT java_lang_reflect_Array_multiNewArrayImpl___java_lang_Class_int_int_1ARRAY(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_multiNewArrayImpl___java_lang_Class_int_int_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Array_newArrayImpl___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_newArrayImpl___java_lang_Class_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Array_get___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_get___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Array_getBoolean___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getBoolean___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BYTE java_lang_reflect_Array_getByte___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getByte___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_CHAR java_lang_reflect_Array_getChar___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getChar___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_DOUBLE java_lang_reflect_Array_getDouble___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getDouble___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_FLOAT java_lang_reflect_Array_getFloat___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getFloat___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Array_getInt___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getInt___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Array_getLength___java_lang_Object(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getLength___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_reflect_Array_getLong___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getLong___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_SHORT java_lang_reflect_Array_getShort___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_getShort___java_lang_Object_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_newInstance___java_lang_Class_int_1ARRAY]
    return XMLVMArray_createMultiDimensions(n1, n2);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_newInstance___java_lang_Class_int]
    return XMLVMArray_createSingleDimension(n1, n2);
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_set___java_lang_Object_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_set___java_lang_Object_int_java_lang_Object]
    org_xmlvm_runtime_XMLVMArray* array = n1;
    JAVA_INT index = n2;
    JAVA_OBJECT obj = n3;
    
    JAVA_OBJECT arrayType = array->fields.org_xmlvm_runtime_XMLVMArray.type_;
    if (arrayType == __CLASS_boolean_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_byte_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_char_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_short_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_int_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_float_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_long_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else if (arrayType == __CLASS_double_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    } else {
        JAVA_ARRAY_OBJECT* data = (JAVA_ARRAY_OBJECT*) array->fields.org_xmlvm_runtime_XMLVMArray.array_;
        data[index] = obj;
    }
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setBoolean___java_lang_Object_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setBoolean___java_lang_Object_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setByte___java_lang_Object_int_byte(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BYTE n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setByte___java_lang_Object_int_byte]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setChar___java_lang_Object_int_char(JAVA_OBJECT n1, JAVA_INT n2, JAVA_CHAR n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setChar___java_lang_Object_int_char]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setDouble___java_lang_Object_int_double(JAVA_OBJECT n1, JAVA_INT n2, JAVA_DOUBLE n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setDouble___java_lang_Object_int_double]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setFloat___java_lang_Object_int_float(JAVA_OBJECT n1, JAVA_INT n2, JAVA_FLOAT n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setFloat___java_lang_Object_int_float]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setInt___java_lang_Object_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setInt___java_lang_Object_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setLong___java_lang_Object_int_long(JAVA_OBJECT n1, JAVA_INT n2, JAVA_LONG n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setLong___java_lang_Object_int_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_reflect_Array_setShort___java_lang_Object_int_short(JAVA_OBJECT n1, JAVA_INT n2, JAVA_SHORT n3)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Array_setShort___java_lang_Object_int_short]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

