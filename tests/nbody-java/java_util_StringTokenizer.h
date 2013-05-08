#ifndef __JAVA_UTIL_STRINGTOKENIZER__
#define __JAVA_UTIL_STRINGTOKENIZER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_StringTokenizer 2
// Implemented interfaces:
#include "java_util_Enumeration.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.util.StringTokenizer
XMLVM_DEFINE_CLASS(java_util_StringTokenizer, 8, XMLVM_ITABLE_SIZE_java_util_StringTokenizer)

extern JAVA_OBJECT __CLASS_java_util_StringTokenizer;
extern JAVA_OBJECT __CLASS_java_util_StringTokenizer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_StringTokenizer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_StringTokenizer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_StringTokenizer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_StringTokenizer \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT string_; \
        JAVA_OBJECT delimiters_; \
        JAVA_BOOLEAN returnDelimiters_; \
        JAVA_INT position_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_StringTokenizer \
    } java_util_StringTokenizer

struct java_util_StringTokenizer {
    __TIB_DEFINITION_java_util_StringTokenizer* tib;
    struct {
        __INSTANCE_FIELDS_java_util_StringTokenizer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_StringTokenizer
#define XMLVM_FORWARD_DECL_java_util_StringTokenizer
typedef struct java_util_StringTokenizer java_util_StringTokenizer;
#endif

#define XMLVM_VTABLE_SIZE_java_util_StringTokenizer 8
#define XMLVM_VTABLE_IDX_java_util_StringTokenizer_hasMoreElements__ 6
#define XMLVM_VTABLE_IDX_java_util_StringTokenizer_nextElement__ 7

void __INIT_java_util_StringTokenizer();
void __INIT_IMPL_java_util_StringTokenizer();
void __DELETE_java_util_StringTokenizer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_StringTokenizer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_StringTokenizer();
JAVA_OBJECT __NEW_INSTANCE_java_util_StringTokenizer();
void java_util_StringTokenizer___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_StringTokenizer___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3);
JAVA_INT java_util_StringTokenizer_countTokens__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_BOOLEAN java_util_StringTokenizer_hasMoreElements__(JAVA_OBJECT me);
JAVA_BOOLEAN java_util_StringTokenizer_hasMoreTokens__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_util_StringTokenizer_nextElement__(JAVA_OBJECT me);
JAVA_OBJECT java_util_StringTokenizer_nextToken__(JAVA_OBJECT me);
JAVA_OBJECT java_util_StringTokenizer_nextToken___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
