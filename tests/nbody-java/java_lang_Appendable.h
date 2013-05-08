#ifndef __JAVA_LANG_APPENDABLE__
#define __JAVA_LANG_APPENDABLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_lang_Appendable_append___char 2
#define XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence 3
#define XMLVM_ITABLE_IDX_java_lang_Appendable_append___java_lang_CharSequence_int_int 4
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif

XMLVM_DEFINE_CLASS(java_lang_Appendable, 0, 0)

extern JAVA_OBJECT __CLASS_java_lang_Appendable;
extern JAVA_OBJECT __CLASS_java_lang_Appendable_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Appendable_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Appendable_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_lang_Appendable
#define XMLVM_FORWARD_DECL_java_lang_Appendable
typedef struct java_lang_Appendable java_lang_Appendable;
#endif

void __INIT_java_lang_Appendable();
void __INIT_IMPL_java_lang_Appendable();

#endif
