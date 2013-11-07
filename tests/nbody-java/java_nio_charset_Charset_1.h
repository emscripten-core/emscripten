#ifndef __JAVA_NIO_CHARSET_CHARSET_1__
#define __JAVA_NIO_CHARSET_CHARSET_1__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_Charset_1 1
// Implemented interfaces:
#include "java_security_PrivilegedAction.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_niochar_CharsetProviderImpl
#define XMLVM_FORWARD_DECL_org_apache_harmony_niochar_CharsetProviderImpl
XMLVM_FORWARD_DECL(org_apache_harmony_niochar_CharsetProviderImpl)
#endif
// Class declarations for java.nio.charset.Charset$1
XMLVM_DEFINE_CLASS(java_nio_charset_Charset_1, 7, XMLVM_ITABLE_SIZE_java_nio_charset_Charset_1)

extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_1;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_1_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_1_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_Charset_1_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_Charset_1
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_Charset_1 \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_Charset_1 \
    } java_nio_charset_Charset_1

struct java_nio_charset_Charset_1 {
    __TIB_DEFINITION_java_nio_charset_Charset_1* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_Charset_1;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset_1
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset_1
typedef struct java_nio_charset_Charset_1 java_nio_charset_Charset_1;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_Charset_1 7
#define XMLVM_VTABLE_IDX_java_nio_charset_Charset_1_run__ 6

void __INIT_java_nio_charset_Charset_1();
void __INIT_IMPL_java_nio_charset_Charset_1();
void __DELETE_java_nio_charset_Charset_1(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset_1(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_Charset_1();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_Charset_1();
void java_nio_charset_Charset_1___INIT___(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT java_nio_charset_Charset_1_run__(JAVA_OBJECT me);

#endif
