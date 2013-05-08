#ifndef __JAVA_NIO_CHARSET_CODINGERRORACTION__
#define __JAVA_NIO_CHARSET_CODINGERRORACTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_charset_CodingErrorAction 0
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
// Class declarations for java.nio.charset.CodingErrorAction
XMLVM_DEFINE_CLASS(java_nio_charset_CodingErrorAction, 6, XMLVM_ITABLE_SIZE_java_nio_charset_CodingErrorAction)

extern JAVA_OBJECT __CLASS_java_nio_charset_CodingErrorAction;
extern JAVA_OBJECT __CLASS_java_nio_charset_CodingErrorAction_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CodingErrorAction_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_charset_CodingErrorAction_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CodingErrorAction
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_charset_CodingErrorAction \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT action_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_charset_CodingErrorAction \
    } java_nio_charset_CodingErrorAction

struct java_nio_charset_CodingErrorAction {
    __TIB_DEFINITION_java_nio_charset_CodingErrorAction* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_charset_CodingErrorAction;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
#define XMLVM_FORWARD_DECL_java_nio_charset_CodingErrorAction
typedef struct java_nio_charset_CodingErrorAction java_nio_charset_CodingErrorAction;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_charset_CodingErrorAction 6
#define XMLVM_VTABLE_IDX_java_nio_charset_CodingErrorAction_toString__ 5

void __INIT_java_nio_charset_CodingErrorAction();
void __INIT_IMPL_java_nio_charset_CodingErrorAction();
void __DELETE_java_nio_charset_CodingErrorAction(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_charset_CodingErrorAction(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_charset_CodingErrorAction();
JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_CodingErrorAction();
JAVA_OBJECT java_nio_charset_CodingErrorAction_GET_IGNORE();
void java_nio_charset_CodingErrorAction_PUT_IGNORE(JAVA_OBJECT v);
JAVA_OBJECT java_nio_charset_CodingErrorAction_GET_REPLACE();
void java_nio_charset_CodingErrorAction_PUT_REPLACE(JAVA_OBJECT v);
JAVA_OBJECT java_nio_charset_CodingErrorAction_GET_REPORT();
void java_nio_charset_CodingErrorAction_PUT_REPORT(JAVA_OBJECT v);
void java_nio_charset_CodingErrorAction___CLINIT_();
void java_nio_charset_CodingErrorAction___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_nio_charset_CodingErrorAction_toString__(JAVA_OBJECT me);

#endif
