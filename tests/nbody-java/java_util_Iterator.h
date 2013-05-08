#ifndef __JAVA_UTIL_ITERATOR__
#define __JAVA_UTIL_ITERATOR__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__ 0
#define XMLVM_ITABLE_IDX_java_util_Iterator_next__ 1
#define XMLVM_ITABLE_IDX_java_util_Iterator_remove__ 2
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:

XMLVM_DEFINE_CLASS(java_util_Iterator, 0, 0)

extern JAVA_OBJECT __CLASS_java_util_Iterator;
extern JAVA_OBJECT __CLASS_java_util_Iterator_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Iterator_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Iterator_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
typedef struct java_util_Iterator java_util_Iterator;
#endif

void __INIT_java_util_Iterator();
void __INIT_IMPL_java_util_Iterator();

#endif
