#ifndef __NBODY_NBODY__
#define __NBODY_NBODY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_nbody_nbody 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
XMLVM_FORWARD_DECL(java_io_PrintStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_nbody_NBodySystem
#define XMLVM_FORWARD_DECL_nbody_NBodySystem
XMLVM_FORWARD_DECL(nbody_NBodySystem)
#endif
// Class declarations for nbody.nbody
XMLVM_DEFINE_CLASS(nbody_nbody, 6, XMLVM_ITABLE_SIZE_nbody_nbody)

extern JAVA_OBJECT __CLASS_nbody_nbody;
extern JAVA_OBJECT __CLASS_nbody_nbody_1ARRAY;
extern JAVA_OBJECT __CLASS_nbody_nbody_2ARRAY;
extern JAVA_OBJECT __CLASS_nbody_nbody_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_nbody_nbody
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_nbody_nbody \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_nbody_nbody \
    } nbody_nbody

struct nbody_nbody {
    __TIB_DEFINITION_nbody_nbody* tib;
    struct {
        __INSTANCE_FIELDS_nbody_nbody;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_nbody_nbody
#define XMLVM_FORWARD_DECL_nbody_nbody
typedef struct nbody_nbody nbody_nbody;
#endif

#define XMLVM_VTABLE_SIZE_nbody_nbody 6

void __INIT_nbody_nbody();
void __INIT_IMPL_nbody_nbody();
void __DELETE_nbody_nbody(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_nbody_nbody(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_nbody_nbody();
JAVA_OBJECT __NEW_INSTANCE_nbody_nbody();
void nbody_nbody___INIT___(JAVA_OBJECT me);
void nbody_nbody_main___java_lang_String_1ARRAY(JAVA_OBJECT n1);

#endif
