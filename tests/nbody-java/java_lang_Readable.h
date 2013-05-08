#ifndef __JAVA_LANG_READABLE__
#define __JAVA_LANG_READABLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_IDX_java_lang_Readable_read___java_nio_CharBuffer 8
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif

XMLVM_DEFINE_CLASS(java_lang_Readable, 0, 0)

extern JAVA_OBJECT __CLASS_java_lang_Readable;
extern JAVA_OBJECT __CLASS_java_lang_Readable_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Readable_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Readable_3ARRAY;
#ifndef XMLVM_FORWARD_DECL_java_lang_Readable
#define XMLVM_FORWARD_DECL_java_lang_Readable
typedef struct java_lang_Readable java_lang_Readable;
#endif

void __INIT_java_lang_Readable();
void __INIT_IMPL_java_lang_Readable();

#endif
