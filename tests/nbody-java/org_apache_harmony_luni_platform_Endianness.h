#ifndef __ORG_APACHE_HARMONY_LUNI_PLATFORM_ENDIANNESS__
#define __ORG_APACHE_HARMONY_LUNI_PLATFORM_ENDIANNESS__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_apache_harmony_luni_platform_Endianness 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for org.apache.harmony.luni.platform.Endianness
XMLVM_DEFINE_CLASS(org_apache_harmony_luni_platform_Endianness, 6, XMLVM_ITABLE_SIZE_org_apache_harmony_luni_platform_Endianness)

extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_1ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_2ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_Endianness_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_platform_Endianness
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_apache_harmony_luni_platform_Endianness \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT displayName_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_platform_Endianness \
    } org_apache_harmony_luni_platform_Endianness

struct org_apache_harmony_luni_platform_Endianness {
    __TIB_DEFINITION_org_apache_harmony_luni_platform_Endianness* tib;
    struct {
        __INSTANCE_FIELDS_org_apache_harmony_luni_platform_Endianness;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_platform_Endianness
typedef struct org_apache_harmony_luni_platform_Endianness org_apache_harmony_luni_platform_Endianness;
#endif

#define XMLVM_VTABLE_SIZE_org_apache_harmony_luni_platform_Endianness 6
#define XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_Endianness_toString__ 5

void __INIT_org_apache_harmony_luni_platform_Endianness();
void __INIT_IMPL_org_apache_harmony_luni_platform_Endianness();
void __DELETE_org_apache_harmony_luni_platform_Endianness(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_apache_harmony_luni_platform_Endianness();
JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_platform_Endianness();
JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
void org_apache_harmony_luni_platform_Endianness_PUT_BIG_ENDIAN(JAVA_OBJECT v);
JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_GET_LITTLE_ENDIAN();
void org_apache_harmony_luni_platform_Endianness_PUT_LITTLE_ENDIAN(JAVA_OBJECT v);
void org_apache_harmony_luni_platform_Endianness___CLINIT_();
void org_apache_harmony_luni_platform_Endianness___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT org_apache_harmony_luni_platform_Endianness_toString__(JAVA_OBJECT me);

#endif
