#ifndef __JAVA_IO_OBJECTSTREAMFIELD__
#define __JAVA_IO_OBJECTSTREAMFIELD__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_ObjectStreamField 8
// Implemented interfaces:
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Boolean
#define XMLVM_FORWARD_DECL_java_lang_Boolean
XMLVM_FORWARD_DECL(java_lang_Boolean)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Byte
#define XMLVM_FORWARD_DECL_java_lang_Byte
XMLVM_FORWARD_DECL(java_lang_Byte)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassLoader
#define XMLVM_FORWARD_DECL_java_lang_ClassLoader
XMLVM_FORWARD_DECL(java_lang_ClassLoader)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassNotFoundException
#define XMLVM_FORWARD_DECL_java_lang_ClassNotFoundException
XMLVM_FORWARD_DECL(java_lang_ClassNotFoundException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
XMLVM_FORWARD_DECL(java_lang_Double)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
XMLVM_FORWARD_DECL(java_lang_Float)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Short
#define XMLVM_FORWARD_DECL_java_lang_Short
XMLVM_FORWARD_DECL(java_lang_Short)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_WeakReference
#define XMLVM_FORWARD_DECL_java_lang_ref_WeakReference
XMLVM_FORWARD_DECL(java_lang_ref_WeakReference)
#endif
// Class declarations for java.io.ObjectStreamField
XMLVM_DEFINE_CLASS(java_io_ObjectStreamField, 7, XMLVM_ITABLE_SIZE_java_io_ObjectStreamField)

extern JAVA_OBJECT __CLASS_java_io_ObjectStreamField;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamField_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamField_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamField_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_ObjectStreamField
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_ObjectStreamField \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT name_; \
        JAVA_OBJECT type_; \
        JAVA_INT offset_; \
        JAVA_OBJECT typeString_; \
        JAVA_BOOLEAN unshared_; \
        JAVA_BOOLEAN isDeserialized_; \
        JAVA_LONG assocFieldID_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_ObjectStreamField \
    } java_io_ObjectStreamField

struct java_io_ObjectStreamField {
    __TIB_DEFINITION_java_io_ObjectStreamField* tib;
    struct {
        __INSTANCE_FIELDS_java_io_ObjectStreamField;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
typedef struct java_io_ObjectStreamField java_io_ObjectStreamField;
#endif

#define XMLVM_VTABLE_SIZE_java_io_ObjectStreamField 7
#define XMLVM_VTABLE_IDX_java_io_ObjectStreamField_compareTo___java_lang_Object 6
#define XMLVM_VTABLE_IDX_java_io_ObjectStreamField_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_io_ObjectStreamField_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_io_ObjectStreamField_toString__ 5

void __INIT_java_io_ObjectStreamField();
void __INIT_IMPL_java_io_ObjectStreamField();
void __DELETE_java_io_ObjectStreamField(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamField(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_ObjectStreamField();
JAVA_OBJECT __NEW_INSTANCE_java_io_ObjectStreamField();
JAVA_INT java_io_ObjectStreamField_GET_FIELD_IS_NOT_RESOLVED();
void java_io_ObjectStreamField_PUT_FIELD_IS_NOT_RESOLVED(JAVA_INT v);
JAVA_INT java_io_ObjectStreamField_GET_FIELD_IS_ABSENT();
void java_io_ObjectStreamField_PUT_FIELD_IS_ABSENT(JAVA_INT v);
JAVA_LONG java_io_ObjectStreamField_getFieldID___org_apache_harmony_misc_accessors_ObjectAccessor_java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3);
void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 6
JAVA_INT java_io_ObjectStreamField_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 1
JAVA_BOOLEAN java_io_ObjectStreamField_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_io_ObjectStreamField_hashCode__(JAVA_OBJECT me);
JAVA_OBJECT java_io_ObjectStreamField_getName__(JAVA_OBJECT me);
JAVA_INT java_io_ObjectStreamField_getOffset__(JAVA_OBJECT me);
JAVA_OBJECT java_io_ObjectStreamField_getTypeInternal__(JAVA_OBJECT me);
JAVA_OBJECT java_io_ObjectStreamField_getType__(JAVA_OBJECT me);
JAVA_CHAR java_io_ObjectStreamField_getTypeCode__(JAVA_OBJECT me);
JAVA_OBJECT java_io_ObjectStreamField_getTypeString__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_ObjectStreamField_isPrimitive__(JAVA_OBJECT me);
void java_io_ObjectStreamField_setOffset___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 5
JAVA_OBJECT java_io_ObjectStreamField_toString__(JAVA_OBJECT me);
void java_io_ObjectStreamField_sortFields___java_io_ObjectStreamField_1ARRAY(JAVA_OBJECT n1);
void java_io_ObjectStreamField_resolve___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_ObjectStreamField_isUnshared__(JAVA_OBJECT me);
void java_io_ObjectStreamField_setUnshared___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_io_ObjectStreamField_defaultResolve__(JAVA_OBJECT me);

#endif
