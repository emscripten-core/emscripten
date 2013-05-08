#ifndef __NBODY_BODY__
#define __NBODY_BODY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_nbody_Body 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for nbody.Body
XMLVM_DEFINE_CLASS(nbody_Body, 6, XMLVM_ITABLE_SIZE_nbody_Body)

extern JAVA_OBJECT __CLASS_nbody_Body;
extern JAVA_OBJECT __CLASS_nbody_Body_1ARRAY;
extern JAVA_OBJECT __CLASS_nbody_Body_2ARRAY;
extern JAVA_OBJECT __CLASS_nbody_Body_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_nbody_Body
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_nbody_Body \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_DOUBLE x_; \
        JAVA_DOUBLE y_; \
        JAVA_DOUBLE z_; \
        JAVA_DOUBLE vx_; \
        JAVA_DOUBLE vy_; \
        JAVA_DOUBLE vz_; \
        JAVA_DOUBLE mass_; \
        __ADDITIONAL_INSTANCE_FIELDS_nbody_Body \
    } nbody_Body

struct nbody_Body {
    __TIB_DEFINITION_nbody_Body* tib;
    struct {
        __INSTANCE_FIELDS_nbody_Body;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_nbody_Body
#define XMLVM_FORWARD_DECL_nbody_Body
typedef struct nbody_Body nbody_Body;
#endif

#define XMLVM_VTABLE_SIZE_nbody_Body 6

void __INIT_nbody_Body();
void __INIT_IMPL_nbody_Body();
void __DELETE_nbody_Body(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_nbody_Body(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_nbody_Body();
JAVA_OBJECT __NEW_INSTANCE_nbody_Body();
JAVA_DOUBLE nbody_Body_GET_PI();
void nbody_Body_PUT_PI(JAVA_DOUBLE v);
JAVA_DOUBLE nbody_Body_GET_SOLAR_MASS();
void nbody_Body_PUT_SOLAR_MASS(JAVA_DOUBLE v);
JAVA_DOUBLE nbody_Body_GET_DAYS_PER_YEAR();
void nbody_Body_PUT_DAYS_PER_YEAR(JAVA_DOUBLE v);
void nbody_Body___INIT___(JAVA_OBJECT me);
JAVA_OBJECT nbody_Body_jupiter__();
JAVA_OBJECT nbody_Body_saturn__();
JAVA_OBJECT nbody_Body_uranus__();
JAVA_OBJECT nbody_Body_neptune__();
JAVA_OBJECT nbody_Body_sun__();
JAVA_OBJECT nbody_Body_offsetMomentum___double_double_double(JAVA_OBJECT me, JAVA_DOUBLE n1, JAVA_DOUBLE n2, JAVA_DOUBLE n3);

#endif
