#ifndef __JAVA_NET_CONNECTEXCEPTION__
#define __JAVA_NET_CONNECTEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_ConnectException 0
// Implemented interfaces:
// Super Class:
#include "java_net_SocketException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.net.ConnectException
XMLVM_DEFINE_CLASS(java_net_ConnectException, 8, XMLVM_ITABLE_SIZE_java_net_ConnectException)

extern JAVA_OBJECT __CLASS_java_net_ConnectException;
extern JAVA_OBJECT __CLASS_java_net_ConnectException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_ConnectException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_ConnectException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_ConnectException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_ConnectException \
    __INSTANCE_FIELDS_java_net_SocketException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_ConnectException \
    } java_net_ConnectException

struct java_net_ConnectException {
    __TIB_DEFINITION_java_net_ConnectException* tib;
    struct {
        __INSTANCE_FIELDS_java_net_ConnectException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_ConnectException
#define XMLVM_FORWARD_DECL_java_net_ConnectException
typedef struct java_net_ConnectException java_net_ConnectException;
#endif

#define XMLVM_VTABLE_SIZE_java_net_ConnectException 8

void __INIT_java_net_ConnectException();
void __INIT_IMPL_java_net_ConnectException();
void __DELETE_java_net_ConnectException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_ConnectException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_ConnectException();
JAVA_OBJECT __NEW_INSTANCE_java_net_ConnectException();
JAVA_LONG java_net_ConnectException_GET_serialVersionUID();
void java_net_ConnectException_PUT_serialVersionUID(JAVA_LONG v);
void java_net_ConnectException___INIT___(JAVA_OBJECT me);
void java_net_ConnectException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
