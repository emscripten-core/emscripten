#ifndef __JAVA_NET_BINDEXCEPTION__
#define __JAVA_NET_BINDEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_BindException 0
// Implemented interfaces:
// Super Class:
#include "java_net_SocketException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.net.BindException
XMLVM_DEFINE_CLASS(java_net_BindException, 8, XMLVM_ITABLE_SIZE_java_net_BindException)

extern JAVA_OBJECT __CLASS_java_net_BindException;
extern JAVA_OBJECT __CLASS_java_net_BindException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_BindException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_BindException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_BindException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_BindException \
    __INSTANCE_FIELDS_java_net_SocketException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_BindException \
    } java_net_BindException

struct java_net_BindException {
    __TIB_DEFINITION_java_net_BindException* tib;
    struct {
        __INSTANCE_FIELDS_java_net_BindException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_BindException
#define XMLVM_FORWARD_DECL_java_net_BindException
typedef struct java_net_BindException java_net_BindException;
#endif

#define XMLVM_VTABLE_SIZE_java_net_BindException 8

void __INIT_java_net_BindException();
void __INIT_IMPL_java_net_BindException();
void __DELETE_java_net_BindException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_BindException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_BindException();
JAVA_OBJECT __NEW_INSTANCE_java_net_BindException();
JAVA_LONG java_net_BindException_GET_serialVersionUID();
void java_net_BindException_PUT_serialVersionUID(JAVA_LONG v);
void java_net_BindException___INIT___(JAVA_OBJECT me);
void java_net_BindException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
