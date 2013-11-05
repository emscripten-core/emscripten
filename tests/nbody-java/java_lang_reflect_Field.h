#ifndef __JAVA_LANG_REFLECT_FIELD__
#define __JAVA_LANG_REFLECT_FIELD__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_reflect_Field 9
// Implemented interfaces:
#include "java_lang_reflect_Member.h"
// Super Class:
#include "java_lang_reflect_AccessibleObject.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Type
#define XMLVM_FORWARD_DECL_java_lang_reflect_Type
XMLVM_FORWARD_DECL(java_lang_reflect_Type)
#endif
// Class declarations for java.lang.reflect.Field
XMLVM_DEFINE_CLASS(java_lang_reflect_Field, 15, XMLVM_ITABLE_SIZE_java_lang_reflect_Field)

extern JAVA_OBJECT __CLASS_java_lang_reflect_Field;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Field_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Field_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Field_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Field
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_reflect_Field \
    __INSTANCE_FIELDS_java_lang_reflect_AccessibleObject; \
    struct { \
        JAVA_OBJECT clazz_; \
        JAVA_INT offset_; \
        JAVA_OBJECT address_; \
        JAVA_OBJECT name_; \
        JAVA_OBJECT type_; \
        JAVA_INT modifiers_; \
        JAVA_OBJECT signature_; \
        JAVA_OBJECT annotations_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Field \
    } java_lang_reflect_Field

struct java_lang_reflect_Field {
    __TIB_DEFINITION_java_lang_reflect_Field* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_reflect_Field;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Field
#define XMLVM_FORWARD_DECL_java_lang_reflect_Field
typedef struct java_lang_reflect_Field java_lang_reflect_Field;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_reflect_Field 15
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_getSignature__ 10
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_isSynthetic__ 14
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_getDeclaringClass__ 12
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_getModifiers__ 9
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_getName__ 13
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_reflect_Field_toString__ 5

void __INIT_java_lang_reflect_Field();
void __INIT_IMPL_java_lang_reflect_Field();
void __DELETE_java_lang_reflect_Field(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Field(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_reflect_Field();
JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Field();
void xmlvm_init_native_java_lang_reflect_Field();
void java_lang_reflect_Field___INIT___(JAVA_OBJECT me);
void java_lang_reflect_Field___INIT____java_lang_Class_java_lang_String_java_lang_Class_int_int_java_lang_Object_java_lang_String_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5, JAVA_OBJECT n6, JAVA_OBJECT n7, JAVA_OBJECT n8);
// Vtable index: 10
JAVA_OBJECT java_lang_reflect_Field_getSignature__(JAVA_OBJECT me);
// Vtable index: 14
JAVA_BOOLEAN java_lang_reflect_Field_isSynthetic__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Field_toGenericString__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_reflect_Field_isEnumConstant__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Field_getGenericType__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_reflect_Field_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_reflect_Field_get___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_reflect_Field_getBoolean___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BYTE java_lang_reflect_Field_getByte___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_CHAR java_lang_reflect_Field_getChar___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_OBJECT java_lang_reflect_Field_getDeclaringClass__(JAVA_OBJECT me);
JAVA_DOUBLE java_lang_reflect_Field_getDouble___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_FLOAT java_lang_reflect_Field_getFloat___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_reflect_Field_getInt___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_LONG java_lang_reflect_Field_getLong___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_INT java_lang_reflect_Field_getModifiers__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_OBJECT java_lang_reflect_Field_getName__(JAVA_OBJECT me);
JAVA_SHORT java_lang_reflect_Field_getShort___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_reflect_Field_getType__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_reflect_Field_hashCode__(JAVA_OBJECT me);
void java_lang_reflect_Field_set___java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_reflect_Field_setBoolean___java_lang_Object_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_lang_reflect_Field_setByte___java_lang_Object_byte(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BYTE n2);
void java_lang_reflect_Field_setChar___java_lang_Object_char(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_CHAR n2);
void java_lang_reflect_Field_setDouble___java_lang_Object_double(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_DOUBLE n2);
void java_lang_reflect_Field_setFloat___java_lang_Object_float(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_FLOAT n2);
void java_lang_reflect_Field_setInt___java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_reflect_Field_setLong___java_lang_Object_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2);
void java_lang_reflect_Field_setShort___java_lang_Object_short(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_SHORT n2);
// Vtable index: 5
JAVA_OBJECT java_lang_reflect_Field_toString__(JAVA_OBJECT me);

#endif
