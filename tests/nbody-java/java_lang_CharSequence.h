#ifndef __JAVA_LANG_CHARSEQUENCE__
#define __JAVA_LANG_CHARSEQUENCE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_lang_CharSequence_length__ 1
#define XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int 0
#define XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int 5
#define XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__ 6
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif

XMLVM_DEFINE_CLASS(java_lang_CharSequence, 0, 0)

extern JAVA_OBJECT __CLASS_java_lang_CharSequence;
extern JAVA_OBJECT __CLASS_java_lang_CharSequence_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_CharSequence_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_CharSequence_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
typedef struct java_lang_CharSequence java_lang_CharSequence;
#endif

void __INIT_java_lang_CharSequence();
void __INIT_IMPL_java_lang_CharSequence();

#endif
