#ifndef __JAVA_UTIL_ARRAYLIST__
#define __JAVA_UTIL_ARRAYLIST__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_ArrayList 78
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
#include "java_util_List.h"
#include "java_util_RandomAccess.h"
// Super Class:
#include "java_util_AbstractList.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
XMLVM_FORWARD_DECL(java_io_ObjectStreamField)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Array
#define XMLVM_FORWARD_DECL_java_lang_reflect_Array
XMLVM_FORWARD_DECL(java_lang_reflect_Array)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collection
#define XMLVM_FORWARD_DECL_java_util_Collection
XMLVM_FORWARD_DECL(java_util_Collection)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.util.ArrayList
XMLVM_DEFINE_CLASS(java_util_ArrayList, 30, XMLVM_ITABLE_SIZE_java_util_ArrayList)

extern JAVA_OBJECT __CLASS_java_util_ArrayList;
extern JAVA_OBJECT __CLASS_java_util_ArrayList_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_ArrayList_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_ArrayList_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_ArrayList
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_ArrayList \
    __INSTANCE_FIELDS_java_util_AbstractList; \
    struct { \
        JAVA_INT firstIndex_; \
        JAVA_INT size_; \
        JAVA_OBJECT array_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_ArrayList \
    } java_util_ArrayList

struct java_util_ArrayList {
    __TIB_DEFINITION_java_util_ArrayList* tib;
    struct {
        __INSTANCE_FIELDS_java_util_ArrayList;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_ArrayList
#define XMLVM_FORWARD_DECL_java_util_ArrayList
typedef struct java_util_ArrayList java_util_ArrayList;
#endif

#define XMLVM_VTABLE_SIZE_java_util_ArrayList 30
#define XMLVM_VTABLE_IDX_java_util_ArrayList_add___int_java_lang_Object 20
#define XMLVM_VTABLE_IDX_java_util_ArrayList_add___java_lang_Object 7
#define XMLVM_VTABLE_IDX_java_util_ArrayList_addAll___int_java_util_Collection 19
#define XMLVM_VTABLE_IDX_java_util_ArrayList_addAll___java_util_Collection 6
#define XMLVM_VTABLE_IDX_java_util_ArrayList_clear__ 8
#define XMLVM_VTABLE_IDX_java_util_ArrayList_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_ArrayList_contains___java_lang_Object 10
#define XMLVM_VTABLE_IDX_java_util_ArrayList_get___int 21
#define XMLVM_VTABLE_IDX_java_util_ArrayList_indexOf___java_lang_Object 22
#define XMLVM_VTABLE_IDX_java_util_ArrayList_isEmpty__ 11
#define XMLVM_VTABLE_IDX_java_util_ArrayList_lastIndexOf___java_lang_Object 23
#define XMLVM_VTABLE_IDX_java_util_ArrayList_remove___int 27
#define XMLVM_VTABLE_IDX_java_util_ArrayList_remove___java_lang_Object 14
#define XMLVM_VTABLE_IDX_java_util_ArrayList_removeRange___int_int 26
#define XMLVM_VTABLE_IDX_java_util_ArrayList_set___int_java_lang_Object 28
#define XMLVM_VTABLE_IDX_java_util_ArrayList_size__ 16
#define XMLVM_VTABLE_IDX_java_util_ArrayList_toArray__ 17
#define XMLVM_VTABLE_IDX_java_util_ArrayList_toArray___java_lang_Object_1ARRAY 18

void __INIT_java_util_ArrayList();
void __INIT_IMPL_java_util_ArrayList();
void __DELETE_java_util_ArrayList(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_ArrayList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_ArrayList();
JAVA_OBJECT __NEW_INSTANCE_java_util_ArrayList();
JAVA_LONG java_util_ArrayList_GET_serialVersionUID();
void java_util_ArrayList_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_util_ArrayList_GET_serialPersistentFields();
void java_util_ArrayList_PUT_serialPersistentFields(JAVA_OBJECT v);
void java_util_ArrayList___CLINIT_();
void java_util_ArrayList___INIT___(JAVA_OBJECT me);
void java_util_ArrayList___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_ArrayList___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_ArrayList_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 20
void java_util_ArrayList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 7
JAVA_BOOLEAN java_util_ArrayList_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 19
JAVA_BOOLEAN java_util_ArrayList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 6
JAVA_BOOLEAN java_util_ArrayList_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 8
void java_util_ArrayList_clear__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_util_ArrayList_clone__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_util_ArrayList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_ArrayList_ensureCapacity___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 21
JAVA_OBJECT java_util_ArrayList_get___int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_ArrayList_growAtEnd___int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_ArrayList_growAtFront___int(JAVA_OBJECT me, JAVA_INT n1);
void java_util_ArrayList_growForInsert___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 22
JAVA_INT java_util_ArrayList_indexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 11
JAVA_BOOLEAN java_util_ArrayList_isEmpty__(JAVA_OBJECT me);
// Vtable index: 23
JAVA_INT java_util_ArrayList_lastIndexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 27
JAVA_OBJECT java_util_ArrayList_remove___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 14
JAVA_BOOLEAN java_util_ArrayList_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 26
void java_util_ArrayList_removeRange___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2);
// Vtable index: 28
JAVA_OBJECT java_util_ArrayList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
// Vtable index: 16
JAVA_INT java_util_ArrayList_size__(JAVA_OBJECT me);
// Vtable index: 17
JAVA_OBJECT java_util_ArrayList_toArray__(JAVA_OBJECT me);
// Vtable index: 18
JAVA_OBJECT java_util_ArrayList_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_ArrayList_trimToSize__(JAVA_OBJECT me);
void java_util_ArrayList_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_ArrayList_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
