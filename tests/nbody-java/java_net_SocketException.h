#ifndef __JAVA_NET_SOCKETEXCEPTION__
#define __JAVA_NET_SOCKETEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_SocketException 0
// Implemented interfaces:
// Super Class:
#include "java_io_IOException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.net.SocketException
XMLVM_DEFINE_CLASS(java_net_SocketException, 8, XMLVM_ITABLE_SIZE_java_net_SocketException)

extern JAVA_OBJECT __CLASS_java_net_SocketException;
extern JAVA_OBJECT __CLASS_java_net_SocketException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_SocketException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_SocketException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_SocketException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_SocketException \
    __INSTANCE_FIELDS_java_io_IOException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_SocketException \
    } java_net_SocketException

struct java_net_SocketException {
    __TIB_DEFINITION_java_net_SocketException* tib;
    struct {
        __INSTANCE_FIELDS_java_net_SocketException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_SocketException
#define XMLVM_FORWARD_DECL_java_net_SocketException
typedef struct java_net_SocketException java_net_SocketException;
#endif

#define XMLVM_VTABLE_SIZE_java_net_SocketException 8

void __INIT_java_net_SocketException();
void __INIT_IMPL_java_net_SocketException();
void __DELETE_java_net_SocketException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_SocketException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_SocketException();
JAVA_OBJECT __NEW_INSTANCE_java_net_SocketException();
JAVA_LONG java_net_SocketException_GET_serialVersionUID();
void java_net_SocketException_PUT_serialVersionUID(JAVA_LONG v);
void java_net_SocketException___INIT___(JAVA_OBJECT me);
void java_net_SocketException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
