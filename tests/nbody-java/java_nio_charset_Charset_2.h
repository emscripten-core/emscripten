#ifndef __JAVA_NIO_CHARSET_CHARSET_2__
#define __JAVA_NIO_CHARSET_CHARSET_2__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_Charset_2 1
// Implemented interfaces:
#include "java_security_PrivilegedAction.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassLoader
#define XMLVM_FORWARD_DECL_java_lang_ClassLoader
XMLVM_FORWARD_DECL(java_lang_ClassLoader)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
XMLVM_FORWARD_DECL(java_lang_Thread)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
// Class declarations for java.nio.charset.Charset$2
XMLVM_DEFINE_CLASS(java_nio_charset_Charset_2, 7, XMLVM_ITABLE_SIZE_java_nio_charset_Charset_2)

extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_2;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_2_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_Charset_2
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_Charset_2 \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT val_t_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_Charset_2 \
    } java_nio_charset_Charset_2

struct java_nio_charset_Charset_2 {
    __TIB_DEFINITION_java_nio_charset_Charset_2* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_Charset_2;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset_2
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset_2
typedef struct java_nio_charset_Charset_2 java_nio_charset_Charset_2;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_Charset_2 7
#define XMLVM_VTABLE_IDX_java_nio_charset_Charset_2_run__ 6

void __INIT_java_nio_charset_Charset_2();
void __INIT_IMPL_java_nio_charset_Charset_2();
void __DELETE_java_nio_charset_Charset_2(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset_2(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_Charset_2();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_Charset_2();
void java_nio_charset_Charset_2___INIT____java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_OBJECT java_nio_charset_Charset_2_run__(JAVA_OBJECT me);

#endif
