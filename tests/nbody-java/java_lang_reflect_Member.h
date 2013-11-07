#ifndef __JAVA_LANG_REFLECT_MEMBER__
#define __JAVA_LANG_REFLECT_MEMBER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_lang_reflect_Member_getDeclaringClass__ 5
#define XMLVM_ITABLE_IDX_java_lang_reflect_Member_getModifiers__ 6
#define XMLVM_ITABLE_IDX_java_lang_reflect_Member_getName__ 7
#define XMLVM_ITABLE_IDX_java_lang_reflect_Member_isSynthetic__ 8
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif

XMLVM_DEFINE_CLASS(java_lang_reflect_Member, 0, 0)

extern JAVA_OBJECT __CLASS_java_lang_reflect_Member;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Member_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Member_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Member_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Member
#define XMLVM_FORWARD_DECL_java_lang_reflect_Member
typedef struct java_lang_reflect_Member java_lang_reflect_Member;
#endif

void __INIT_java_lang_reflect_Member();
void __INIT_IMPL_java_lang_reflect_Member();
JAVA_INT java_lang_reflect_Member_GET_PUBLIC();
void java_lang_reflect_Member_PUT_PUBLIC(JAVA_INT v);
JAVA_INT java_lang_reflect_Member_GET_DECLARED();
void java_lang_reflect_Member_PUT_DECLARED(JAVA_INT v);

#endif
