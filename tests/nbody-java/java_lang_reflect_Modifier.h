#ifndef __JAVA_LANG_REFLECT_MODIFIER__
#define __JAVA_LANG_REFLECT_MODIFIER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_reflect_Modifier 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for java.lang.reflect.Modifier
XMLVM_DEFINE_CLASS(java_lang_reflect_Modifier, 6, XMLVM_ITABLE_SIZE_java_lang_reflect_Modifier)

extern JAVA_OBJECT __CLASS_java_lang_reflect_Modifier;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Modifier_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Modifier
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_reflect_Modifier \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Modifier \
    } java_lang_reflect_Modifier

struct java_lang_reflect_Modifier {
    __TIB_DEFINITION_java_lang_reflect_Modifier* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_reflect_Modifier;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Modifier
#define XMLVM_FORWARD_DECL_java_lang_reflect_Modifier
typedef struct java_lang_reflect_Modifier java_lang_reflect_Modifier;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_reflect_Modifier 6

void __INIT_java_lang_reflect_Modifier();
void __INIT_IMPL_java_lang_reflect_Modifier();
void __DELETE_java_lang_reflect_Modifier(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Modifier(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_reflect_Modifier();
JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Modifier();
JAVA_INT java_lang_reflect_Modifier_GET_PUBLIC();
void java_lang_reflect_Modifier_PUT_PUBLIC(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_PRIVATE();
void java_lang_reflect_Modifier_PUT_PRIVATE(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_PROTECTED();
void java_lang_reflect_Modifier_PUT_PROTECTED(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_STATIC();
void java_lang_reflect_Modifier_PUT_STATIC(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_FINAL();
void java_lang_reflect_Modifier_PUT_FINAL(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_SYNCHRONIZED();
void java_lang_reflect_Modifier_PUT_SYNCHRONIZED(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_VOLATILE();
void java_lang_reflect_Modifier_PUT_VOLATILE(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_TRANSIENT();
void java_lang_reflect_Modifier_PUT_TRANSIENT(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_NATIVE();
void java_lang_reflect_Modifier_PUT_NATIVE(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_INTERFACE();
void java_lang_reflect_Modifier_PUT_INTERFACE(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_ABSTRACT();
void java_lang_reflect_Modifier_PUT_ABSTRACT(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_STRICT();
void java_lang_reflect_Modifier_PUT_STRICT(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_BRIDGE();
void java_lang_reflect_Modifier_PUT_BRIDGE(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_VARARGS();
void java_lang_reflect_Modifier_PUT_VARARGS(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_SYNTHETIC();
void java_lang_reflect_Modifier_PUT_SYNTHETIC(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_ANNOTATION();
void java_lang_reflect_Modifier_PUT_ANNOTATION(JAVA_INT v);
JAVA_INT java_lang_reflect_Modifier_GET_ENUM();
void java_lang_reflect_Modifier_PUT_ENUM(JAVA_INT v);
void java_lang_reflect_Modifier___INIT___(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_reflect_Modifier_isAbstract___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isFinal___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isInterface___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isNative___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isPrivate___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isProtected___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isPublic___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isStatic___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isStrict___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isSynchronized___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isTransient___int(JAVA_INT n1);
JAVA_BOOLEAN java_lang_reflect_Modifier_isVolatile___int(JAVA_INT n1);
JAVA_OBJECT java_lang_reflect_Modifier_toString___int(JAVA_INT n1);

#endif
