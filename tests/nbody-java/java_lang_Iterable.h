#ifndef __JAVA_LANG_ITERABLE__
#define __JAVA_LANG_ITERABLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__ 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif

XMLVM_DEFINE_CLASS(java_lang_Iterable, 0, 0)

extern JAVA_OBJECT __CLASS_java_lang_Iterable;
extern JAVA_OBJECT __CLASS_java_lang_Iterable_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Iterable_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Iterable_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_lang_Iterable
#define XMLVM_FORWARD_DECL_java_lang_Iterable
typedef struct java_lang_Iterable java_lang_Iterable;
#endif

void __INIT_java_lang_Iterable();
void __INIT_IMPL_java_lang_Iterable();

#endif
