#ifndef __NBODY_NBODYSYSTEM__
#define __NBODY_NBODYSYSTEM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_nbody_NBodySystem 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Math
#define XMLVM_FORWARD_DECL_java_lang_Math
XMLVM_FORWARD_DECL(java_lang_Math)
#endif
#ifndef XMLVM_FORWARD_DECL_nbody_Body
#define XMLVM_FORWARD_DECL_nbody_Body
XMLVM_FORWARD_DECL(nbody_Body)
#endif
// Class declarations for nbody.NBodySystem
XMLVM_DEFINE_CLASS(nbody_NBodySystem, 6, XMLVM_ITABLE_SIZE_nbody_NBodySystem)

extern JAVA_OBJECT __CLASS_nbody_NBodySystem;
extern JAVA_OBJECT __CLASS_nbody_NBodySystem_1ARRAY;
extern JAVA_OBJECT __CLASS_nbody_NBodySystem_2ARRAY;
extern JAVA_OBJECT __CLASS_nbody_NBodySystem_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_nbody_NBodySystem
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_nbody_NBodySystem \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT bodies_; \
        __ADDITIONAL_INSTANCE_FIELDS_nbody_NBodySystem \
    } nbody_NBodySystem

struct nbody_NBodySystem {
    __TIB_DEFINITION_nbody_NBodySystem* tib;
    struct {
        __INSTANCE_FIELDS_nbody_NBodySystem;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_nbody_NBodySystem
#define XMLVM_FORWARD_DECL_nbody_NBodySystem
typedef struct nbody_NBodySystem nbody_NBodySystem;
#endif

#define XMLVM_VTABLE_SIZE_nbody_NBodySystem 6

void __INIT_nbody_NBodySystem();
void __INIT_IMPL_nbody_NBodySystem();
void __DELETE_nbody_NBodySystem(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_nbody_NBodySystem(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_nbody_NBodySystem();
JAVA_OBJECT __NEW_INSTANCE_nbody_NBodySystem();
void nbody_NBodySystem___INIT___(JAVA_OBJECT me);
void nbody_NBodySystem_advance___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
JAVA_DOUBLE nbody_NBodySystem_energy__(JAVA_OBJECT me);

#endif
