#ifndef __JAVA_LANG_REFLECT_ARRAY__
#define __JAVA_LANG_REFLECT_ARRAY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_reflect_Array 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
// Class declarations for java.lang.reflect.Array
XMLVM_DEFINE_CLASS(java_lang_reflect_Array, 6, XMLVM_ITABLE_SIZE_java_lang_reflect_Array)

extern JAVA_OBJECT __CLASS_java_lang_reflect_Array;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Array_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Array_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Array_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Array
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_reflect_Array \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Array \
    } java_lang_reflect_Array

struct java_lang_reflect_Array {
    __TIB_DEFINITION_java_lang_reflect_Array* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_reflect_Array;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Array
#define XMLVM_FORWARD_DECL_java_lang_reflect_Array
typedef struct java_lang_reflect_Array java_lang_reflect_Array;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_reflect_Array 6

void __INIT_java_lang_reflect_Array();
void __INIT_IMPL_java_lang_reflect_Array();
void __DELETE_java_lang_reflect_Array(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Array(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_reflect_Array();
JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Array();
void java_lang_reflect_Array___INIT___(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Array_multiNewArrayImpl___java_lang_Class_int_int_1ARRAY(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_lang_reflect_Array_newArrayImpl___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_reflect_Array_get___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_BOOLEAN java_lang_reflect_Array_getBoolean___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_BYTE java_lang_reflect_Array_getByte___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_CHAR java_lang_reflect_Array_getChar___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_DOUBLE java_lang_reflect_Array_getDouble___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_FLOAT java_lang_reflect_Array_getFloat___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_INT java_lang_reflect_Array_getInt___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_INT java_lang_reflect_Array_getLength___java_lang_Object(JAVA_OBJECT n1);
JAVA_LONG java_lang_reflect_Array_getLong___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_SHORT java_lang_reflect_Array_getShort___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_reflect_Array_set___java_lang_Object_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3);
void java_lang_reflect_Array_setBoolean___java_lang_Object_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BOOLEAN n3);
void java_lang_reflect_Array_setByte___java_lang_Object_int_byte(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BYTE n3);
void java_lang_reflect_Array_setChar___java_lang_Object_int_char(JAVA_OBJECT n1, JAVA_INT n2, JAVA_CHAR n3);
void java_lang_reflect_Array_setDouble___java_lang_Object_int_double(JAVA_OBJECT n1, JAVA_INT n2, JAVA_DOUBLE n3);
void java_lang_reflect_Array_setFloat___java_lang_Object_int_float(JAVA_OBJECT n1, JAVA_INT n2, JAVA_FLOAT n3);
void java_lang_reflect_Array_setInt___java_lang_Object_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
void java_lang_reflect_Array_setLong___java_lang_Object_int_long(JAVA_OBJECT n1, JAVA_INT n2, JAVA_LONG n3);
void java_lang_reflect_Array_setShort___java_lang_Object_int_short(JAVA_OBJECT n1, JAVA_INT n2, JAVA_SHORT n3);

#endif
