#ifndef __JAVA_LANG_BOOLEAN__
#define __JAVA_LANG_BOOLEAN__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Boolean 8
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
// Class declarations for java.lang.Boolean
XMLVM_DEFINE_CLASS(java_lang_Boolean, 7, XMLVM_ITABLE_SIZE_java_lang_Boolean)

extern JAVA_OBJECT __CLASS_java_lang_Boolean;
extern JAVA_OBJECT __CLASS_java_lang_Boolean_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Boolean_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Boolean_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Boolean
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Boolean \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_BOOLEAN value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Boolean \
    } java_lang_Boolean

struct java_lang_Boolean {
    __TIB_DEFINITION_java_lang_Boolean* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Boolean;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Boolean
#define XMLVM_FORWARD_DECL_java_lang_Boolean
typedef struct java_lang_Boolean java_lang_Boolean;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Boolean 7
#define XMLVM_VTABLE_IDX_java_lang_Boolean_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Boolean_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Boolean_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Boolean_compareTo___java_lang_Object 6

void __INIT_java_lang_Boolean();
void __INIT_IMPL_java_lang_Boolean();
void __DELETE_java_lang_Boolean(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Boolean(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Boolean();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Boolean();
JAVA_LONG java_lang_Boolean_GET_serialVersionUID();
void java_lang_Boolean_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_lang_Boolean_GET_TYPE();
void java_lang_Boolean_PUT_TYPE(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Boolean_GET_TRUE();
void java_lang_Boolean_PUT_TRUE(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Boolean_GET_FALSE();
void java_lang_Boolean_PUT_FALSE(JAVA_OBJECT v);
void java_lang_Boolean___CLINIT_();
void java_lang_Boolean___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Boolean___INIT____boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_lang_Boolean_booleanValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Boolean_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Boolean_compareTo___java_lang_Boolean(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_lang_Boolean_hashCode__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Boolean_toString__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Boolean_getBoolean___java_lang_String(JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Boolean_parseBoolean___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Boolean_toString___boolean(JAVA_BOOLEAN n1);
JAVA_OBJECT java_lang_Boolean_valueOf___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Boolean_valueOf___boolean(JAVA_BOOLEAN n1);
// Vtable index: 6
JAVA_INT java_lang_Boolean_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
