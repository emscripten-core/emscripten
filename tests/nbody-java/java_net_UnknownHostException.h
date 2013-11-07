#ifndef __JAVA_NET_UNKNOWNHOSTEXCEPTION__
#define __JAVA_NET_UNKNOWNHOSTEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_net_UnknownHostException 0
// Implemented interfaces:
// Super Class:
#include "java_io_IOException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.net.UnknownHostException
XMLVM_DEFINE_CLASS(java_net_UnknownHostException, 8, XMLVM_ITABLE_SIZE_java_net_UnknownHostException)

extern JAVA_OBJECT __CLASS_java_net_UnknownHostException;
extern JAVA_OBJECT __CLASS_java_net_UnknownHostException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_net_UnknownHostException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_net_UnknownHostException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_net_UnknownHostException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_net_UnknownHostException \
    __INSTANCE_FIELDS_java_io_IOException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_net_UnknownHostException \
    } java_net_UnknownHostException

struct java_net_UnknownHostException {
    __TIB_DEFINITION_java_net_UnknownHostException* tib;
    struct {
        __INSTANCE_FIELDS_java_net_UnknownHostException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_net_UnknownHostException
#define XMLVM_FORWARD_DECL_java_net_UnknownHostException
typedef struct java_net_UnknownHostException java_net_UnknownHostException;
#endif

#define XMLVM_VTABLE_SIZE_java_net_UnknownHostException 8

void __INIT_java_net_UnknownHostException();
void __INIT_IMPL_java_net_UnknownHostException();
void __DELETE_java_net_UnknownHostException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_net_UnknownHostException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_net_UnknownHostException();
JAVA_OBJECT __NEW_INSTANCE_java_net_UnknownHostException();
JAVA_LONG java_net_UnknownHostException_GET_serialVersionUID();
void java_net_UnknownHostException_PUT_serialVersionUID(JAVA_LONG v);
void java_net_UnknownHostException___INIT___(JAVA_OBJECT me);
void java_net_UnknownHostException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
