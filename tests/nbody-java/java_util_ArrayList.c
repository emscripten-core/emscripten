#include "xmlvm.h"
#include "java_io_ObjectStreamField.h"
#include "java_lang_Class.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_Integer.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_System.h"
#include "java_lang_reflect_Array.h"
#include "java_util_Collection.h"
#include "java_util_Iterator.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_util_ArrayList.h"

#define XMLVM_CURRENT_CLASS_NAME ArrayList
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_ArrayList

__TIB_DEFINITION_java_util_ArrayList __TIB_java_util_ArrayList = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_ArrayList, // classInitializer
    "java.util.ArrayList", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Object;>Ljava/util/AbstractList<TE;>;Ljava/util/List<TE;>;Ljava/lang/Cloneable;Ljava/io/Serializable;Ljava/util/RandomAccess;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_AbstractList, // extends
    sizeof(java_util_ArrayList), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_ArrayList;
JAVA_OBJECT __CLASS_java_util_ArrayList_1ARRAY;
JAVA_OBJECT __CLASS_java_util_ArrayList_2ARRAY;
JAVA_OBJECT __CLASS_java_util_ArrayList_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_ArrayList_serialVersionUID;
static JAVA_OBJECT _STATIC_java_util_ArrayList_serialPersistentFields;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_java_util_ArrayList()
{
    staticInitializerLock(&__TIB_java_util_ArrayList);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_ArrayList.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_ArrayList.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_ArrayList);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_ArrayList.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_ArrayList.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_ArrayList.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.ArrayList")
        __INIT_IMPL_java_util_ArrayList();
    }
}

void __INIT_IMPL_java_util_ArrayList()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_AbstractList)
    __TIB_java_util_ArrayList.newInstanceFunc = __NEW_INSTANCE_java_util_ArrayList;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_ArrayList.vtable, __TIB_java_util_AbstractList.vtable, sizeof(__TIB_java_util_AbstractList.vtable));
    // Initialize vtable for this class
    __TIB_java_util_ArrayList.vtable[20] = (VTABLE_PTR) &java_util_ArrayList_add___int_java_lang_Object;
    __TIB_java_util_ArrayList.vtable[7] = (VTABLE_PTR) &java_util_ArrayList_add___java_lang_Object;
    __TIB_java_util_ArrayList.vtable[19] = (VTABLE_PTR) &java_util_ArrayList_addAll___int_java_util_Collection;
    __TIB_java_util_ArrayList.vtable[6] = (VTABLE_PTR) &java_util_ArrayList_addAll___java_util_Collection;
    __TIB_java_util_ArrayList.vtable[8] = (VTABLE_PTR) &java_util_ArrayList_clear__;
    __TIB_java_util_ArrayList.vtable[0] = (VTABLE_PTR) &java_util_ArrayList_clone__;
    __TIB_java_util_ArrayList.vtable[10] = (VTABLE_PTR) &java_util_ArrayList_contains___java_lang_Object;
    __TIB_java_util_ArrayList.vtable[21] = (VTABLE_PTR) &java_util_ArrayList_get___int;
    __TIB_java_util_ArrayList.vtable[22] = (VTABLE_PTR) &java_util_ArrayList_indexOf___java_lang_Object;
    __TIB_java_util_ArrayList.vtable[11] = (VTABLE_PTR) &java_util_ArrayList_isEmpty__;
    __TIB_java_util_ArrayList.vtable[23] = (VTABLE_PTR) &java_util_ArrayList_lastIndexOf___java_lang_Object;
    __TIB_java_util_ArrayList.vtable[27] = (VTABLE_PTR) &java_util_ArrayList_remove___int;
    __TIB_java_util_ArrayList.vtable[14] = (VTABLE_PTR) &java_util_ArrayList_remove___java_lang_Object;
    __TIB_java_util_ArrayList.vtable[26] = (VTABLE_PTR) &java_util_ArrayList_removeRange___int_int;
    __TIB_java_util_ArrayList.vtable[28] = (VTABLE_PTR) &java_util_ArrayList_set___int_java_lang_Object;
    __TIB_java_util_ArrayList.vtable[16] = (VTABLE_PTR) &java_util_ArrayList_size__;
    __TIB_java_util_ArrayList.vtable[17] = (VTABLE_PTR) &java_util_ArrayList_toArray__;
    __TIB_java_util_ArrayList.vtable[18] = (VTABLE_PTR) &java_util_ArrayList_toArray___java_lang_Object_1ARRAY;
    // Initialize interface information
    __TIB_java_util_ArrayList.numImplementedInterfaces = 6;
    __TIB_java_util_ArrayList.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 6);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_ArrayList.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_ArrayList.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_lang_Iterable)

    __TIB_java_util_ArrayList.implementedInterfaces[0][2] = &__TIB_java_lang_Iterable;

    XMLVM_CLASS_INIT(java_util_Collection)

    __TIB_java_util_ArrayList.implementedInterfaces[0][3] = &__TIB_java_util_Collection;

    XMLVM_CLASS_INIT(java_util_List)

    __TIB_java_util_ArrayList.implementedInterfaces[0][4] = &__TIB_java_util_List;

    XMLVM_CLASS_INIT(java_util_RandomAccess)

    __TIB_java_util_ArrayList.implementedInterfaces[0][5] = &__TIB_java_util_RandomAccess;
    // Initialize itable for this class
    __TIB_java_util_ArrayList.itableBegin = &__TIB_java_util_ArrayList.itable[0];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_lang_Iterable_iterator__] = __TIB_java_util_ArrayList.vtable[12];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_addAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[6];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_add___java_lang_Object] = __TIB_java_util_ArrayList.vtable[7];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_clear__] = __TIB_java_util_ArrayList.vtable[8];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_containsAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[9];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_contains___java_lang_Object] = __TIB_java_util_ArrayList.vtable[10];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_equals___java_lang_Object] = __TIB_java_util_ArrayList.vtable[1];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_hashCode__] = __TIB_java_util_ArrayList.vtable[4];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_isEmpty__] = __TIB_java_util_ArrayList.vtable[11];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_iterator__] = __TIB_java_util_ArrayList.vtable[12];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_removeAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[13];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_remove___java_lang_Object] = __TIB_java_util_ArrayList.vtable[14];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_retainAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[15];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_size__] = __TIB_java_util_ArrayList.vtable[16];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray__] = __TIB_java_util_ArrayList.vtable[17];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_Collection_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_ArrayList.vtable[18];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___int_java_util_Collection] = __TIB_java_util_ArrayList.vtable[19];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_addAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[6];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_add___int_java_lang_Object] = __TIB_java_util_ArrayList.vtable[20];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_add___java_lang_Object] = __TIB_java_util_ArrayList.vtable[7];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_clear__] = __TIB_java_util_ArrayList.vtable[8];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_containsAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[9];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_contains___java_lang_Object] = __TIB_java_util_ArrayList.vtable[10];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_equals___java_lang_Object] = __TIB_java_util_ArrayList.vtable[1];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_get___int] = __TIB_java_util_ArrayList.vtable[21];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_hashCode__] = __TIB_java_util_ArrayList.vtable[4];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_indexOf___java_lang_Object] = __TIB_java_util_ArrayList.vtable[22];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_isEmpty__] = __TIB_java_util_ArrayList.vtable[11];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_iterator__] = __TIB_java_util_ArrayList.vtable[12];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_lastIndexOf___java_lang_Object] = __TIB_java_util_ArrayList.vtable[23];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator__] = __TIB_java_util_ArrayList.vtable[24];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_listIterator___int] = __TIB_java_util_ArrayList.vtable[25];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_removeAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[13];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___int] = __TIB_java_util_ArrayList.vtable[27];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_remove___java_lang_Object] = __TIB_java_util_ArrayList.vtable[14];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_retainAll___java_util_Collection] = __TIB_java_util_ArrayList.vtable[15];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_set___int_java_lang_Object] = __TIB_java_util_ArrayList.vtable[28];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_size__] = __TIB_java_util_ArrayList.vtable[16];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_subList___int_int] = __TIB_java_util_ArrayList.vtable[29];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray__] = __TIB_java_util_ArrayList.vtable[17];
    __TIB_java_util_ArrayList.itable[XMLVM_ITABLE_IDX_java_util_List_toArray___java_lang_Object_1ARRAY] = __TIB_java_util_ArrayList.vtable[18];

    _STATIC_java_util_ArrayList_serialVersionUID = 8683452581122892189;
    _STATIC_java_util_ArrayList_serialPersistentFields = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_util_ArrayList.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_ArrayList.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_ArrayList.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_ArrayList.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_ArrayList.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_ArrayList.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_ArrayList.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_ArrayList.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_ArrayList = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_ArrayList);
    __TIB_java_util_ArrayList.clazz = __CLASS_java_util_ArrayList;
    __TIB_java_util_ArrayList.baseType = JAVA_NULL;
    __CLASS_java_util_ArrayList_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_ArrayList);
    __CLASS_java_util_ArrayList_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_ArrayList_1ARRAY);
    __CLASS_java_util_ArrayList_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_ArrayList_2ARRAY);
    java_util_ArrayList___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_ArrayList]
    //XMLVM_END_WRAPPER

    __TIB_java_util_ArrayList.classInitialized = 1;
}

void __DELETE_java_util_ArrayList(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_ArrayList]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_ArrayList(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_AbstractList(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_ArrayList*) me)->fields.java_util_ArrayList.firstIndex_ = 0;
    ((java_util_ArrayList*) me)->fields.java_util_ArrayList.size_ = 0;
    ((java_util_ArrayList*) me)->fields.java_util_ArrayList.array_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_ArrayList]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_ArrayList()
{    XMLVM_CLASS_INIT(java_util_ArrayList)
java_util_ArrayList* me = (java_util_ArrayList*) XMLVM_MALLOC(sizeof(java_util_ArrayList));
    me->tib = &__TIB_java_util_ArrayList;
    __INIT_INSTANCE_MEMBERS_java_util_ArrayList(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_ArrayList]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_ArrayList()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_ArrayList();
    java_util_ArrayList___INIT___(me);
    return me;
}

JAVA_LONG java_util_ArrayList_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_ArrayList)
    return _STATIC_java_util_ArrayList_serialVersionUID;
}

void java_util_ArrayList_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_ArrayList)
_STATIC_java_util_ArrayList_serialVersionUID = v;
}

JAVA_OBJECT java_util_ArrayList_GET_serialPersistentFields()
{
    XMLVM_CLASS_INIT(java_util_ArrayList)
    return _STATIC_java_util_ArrayList_serialPersistentFields;
}

void java_util_ArrayList_PUT_serialPersistentFields(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_ArrayList)
_STATIC_java_util_ArrayList_serialPersistentFields = v;
}

void java_util_ArrayList___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList___CLINIT___]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVM_SOURCE_POSITION("ArrayList.java", 690)
    _r0.i = 1;
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_io_ObjectStreamField, _r0.i);
    _r1.i = 0;
    _r2.o = __NEW_java_io_ObjectStreamField();
    XMLVM_SOURCE_POSITION("ArrayList.java", 691)
    // "size"
    _r3.o = xmlvm_create_java_string_from_pool(161);
    _r4.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(_r2.o, _r3.o, _r4.o);
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    java_util_ArrayList_PUT_serialPersistentFields( _r0.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 36)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList___INIT___]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ArrayList.java", 51)
    _r0.i = 10;
    XMLVM_CHECK_NPE(1)
    java_util_ArrayList___INIT____int(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 52)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList___INIT____int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 61)
    XMLVM_CHECK_NPE(1)
    java_util_AbstractList___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 62)
    if (_r2.i >= 0) goto label11;
    XMLVM_SOURCE_POSITION("ArrayList.java", 63)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label11:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 65)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_ArrayList*) _r1.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_CHECK_NPE(1)
    ((java_util_ArrayList*) _r1.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 66)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_ArrayList_newElementArray___int(_r1.o, _r2.i);
    XMLVM_CHECK_NPE(1)
    ((java_util_ArrayList*) _r1.o)->fields.java_util_ArrayList.array_ = _r0.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 67)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList___INIT____java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList___INIT____java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 77)
    XMLVM_CHECK_NPE(4)
    java_util_AbstractList___INIT___(_r4.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 78)
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_ = _r3.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 79)
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_toArray__])(_r5.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 80)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 85)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i / 10;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(4)
    _r1.o = java_util_ArrayList_newElementArray___int(_r4.o, _r1.i);
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_ = _r1.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 86)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r3.i, _r1.o, _r3.i, _r2.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 87)
    _r0.i = 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 88)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_newElementArray___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_newElementArray___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "newElementArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 92)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_add___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_add___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.i = n1;
    _r7.o = n2;
    _r4.i = 1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 110)
    if (_r6.i < 0) goto label7;
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r6.i <= _r0.i) goto label29;
    label7:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 111)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 113)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 114)
    _r2.o = java_lang_Integer_valueOf___int(_r6.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 115)
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label29:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 117)
    if (_r6.i != 0) goto label60;
    XMLVM_SOURCE_POSITION("ArrayList.java", 118)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r0.i != 0) goto label38;
    XMLVM_SOURCE_POSITION("ArrayList.java", 119)
    XMLVM_CHECK_NPE(5)
    java_util_ArrayList_growAtFront___int(_r5.o, _r4.i);
    label38:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 121)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i - _r4.i;
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r1.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.o;
    label47:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 142)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 143)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_AbstractList*) _r5.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(5)
    ((java_util_AbstractList*) _r5.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 144)
    XMLVM_EXIT_METHOD()
    return;
    label60:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 122)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r6.i != _r0.i) goto label87;
    XMLVM_SOURCE_POSITION("ArrayList.java", 123)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i != _r1.i) goto label77;
    XMLVM_SOURCE_POSITION("ArrayList.java", 124)
    XMLVM_CHECK_NPE(5)
    java_util_ArrayList_growAtEnd___int(_r5.o, _r4.i);
    label77:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 126)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.o;
    goto label47;
    label87:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 128)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i != _r1.i) goto label105;
    XMLVM_SOURCE_POSITION("ArrayList.java", 129)
    XMLVM_CHECK_NPE(5)
    java_util_ArrayList_growForInsert___int_int(_r5.o, _r6.i, _r4.i);
    label97:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 139)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i + _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.o;
    goto label47;
    label105:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 130)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i == _r1.i) goto label125;
    XMLVM_SOURCE_POSITION("ArrayList.java", 131)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r0.i <= 0) goto label140;
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i / 2;
    if (_r6.i >= _r0.i) goto label140;
    label125:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 132)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r3.i - _r4.i;
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r3.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r6.i);
    goto label97;
    label140:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 135)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r0.i + _r6.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 136)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r3.i = _r0.i + 1;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    XMLVM_SOURCE_POSITION("ArrayList.java", 137)
    _r4.i = _r4.i - _r6.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r0.i, _r2.o, _r3.i, _r4.i);
    goto label97;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_add___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_add___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "add", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 155)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i != _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("ArrayList.java", 156)
    XMLVM_CHECK_NPE(4)
    java_util_ArrayList_growAtEnd___int(_r4.o, _r3.i);
    label14:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 158)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i + _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 159)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 160)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 161)
    XMLVM_EXIT_METHOD()
    return _r3.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_addAll___int_java_util_Collection(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_addAll___int_java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    _r8.o = me;
    _r9.i = n1;
    _r10.o = n2;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 180)
    if (_r9.i < 0) goto label7;
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    if (_r9.i <= _r0.i) goto label29;
    label7:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 181)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 183)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 184)
    _r2.o = java_lang_Integer_valueOf___int(_r9.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 185)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label29:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 188)
    XMLVM_CHECK_NPE(10)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r10.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_toArray__])(_r10.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 189)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_SOURCE_POSITION("ArrayList.java", 192)
    if (_r1.i != 0) goto label38;
    _r0 = _r7;
    label37:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 193)
    XMLVM_SOURCE_POSITION("ArrayList.java", 228)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label38:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 196)
    if (_r9.i != 0) goto label69;
    XMLVM_SOURCE_POSITION("ArrayList.java", 197)
    XMLVM_CHECK_NPE(8)
    java_util_ArrayList_growAtFront___int(_r8.o, _r1.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 198)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r2.i = _r2.i - _r1.i;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_ = _r2.i;
    label48:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 224)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r3.i + _r9.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r7.i, _r2.o, _r3.i, _r1.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 226)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 227)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    _r0.i = 1;
    goto label37;
    label69:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 199)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    if (_r9.i != _r2.i) goto label88;
    XMLVM_SOURCE_POSITION("ArrayList.java", 200)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r3.i = _r3.i - _r1.i;
    if (_r2.i <= _r3.i) goto label48;
    XMLVM_SOURCE_POSITION("ArrayList.java", 201)
    XMLVM_CHECK_NPE(8)
    java_util_ArrayList_growAtEnd___int(_r8.o, _r1.i);
    goto label48;
    label88:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 204)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i - _r3.i;
    if (_r2.i >= _r1.i) goto label100;
    XMLVM_SOURCE_POSITION("ArrayList.java", 205)
    XMLVM_CHECK_NPE(8)
    java_util_ArrayList_growForInsert___int_int(_r8.o, _r9.i, _r1.i);
    goto label48;
    label100:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 206)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r3.i = _r3.i - _r1.i;
    if (_r2.i > _r3.i) goto label121;
    XMLVM_SOURCE_POSITION("ArrayList.java", 207)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r2.i <= 0) goto label154;
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i / 2;
    if (_r9.i >= _r2.i) goto label154;
    label121:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 208)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r2.i = _r2.i - _r1.i;
    if (_r2.i >= 0) goto label142;
    XMLVM_SOURCE_POSITION("ArrayList.java", 209)
    XMLVM_SOURCE_POSITION("ArrayList.java", 210)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r3.i + _r9.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 211)
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r5.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r2.i = _r3.i - _r2.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 212)
    XMLVM_CHECK_NPE(8)
    _r6.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r6.i = _r6.i - _r9.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r3.i, _r5.o, _r2.i, _r6.i);
    _r2 = _r7;
    label142:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 213)
    XMLVM_SOURCE_POSITION("ArrayList.java", 215)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r4.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r5.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r4.i, _r5.o, _r2.i, _r9.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 216)
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_ = _r2.i;
    goto label48;
    label154:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 218)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r2.i = _r2.i + _r9.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 219)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r5.i = _r2.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    _r6.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    XMLVM_SOURCE_POSITION("ArrayList.java", 220)
    _r6.i = _r6.i - _r9.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r2.i, _r4.o, _r5.i, _r6.i);
    goto label48;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_addAll___java_util_Collection(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_addAll___java_util_Collection]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "addAll", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 241)
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Collection_toArray__])(_r7.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 242)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i != 0) goto label10;
    _r0 = _r5;
    label9:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 243)
    XMLVM_SOURCE_POSITION("ArrayList.java", 252)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 245)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r3.i = _r3.i + _r4.i;
    _r2.i = _r2.i - _r3.i;
    if (_r1.i <= _r2.i) goto label26;
    XMLVM_SOURCE_POSITION("ArrayList.java", 246)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(6)
    java_util_ArrayList_growAtEnd___int(_r6.o, _r1.i);
    label26:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 248)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 249)
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r5.i, _r1.o, _r2.i, _r3.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 250)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(6)
    ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 251)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    _r0.i = 1;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_clear__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_clear__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "clear", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("ArrayList.java", 263)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    if (_r0.i == 0) goto label28;
    XMLVM_SOURCE_POSITION("ArrayList.java", 267)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;
    _r3.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 270)
    _r0.i = 0;
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_CHECK_NPE(4)
    ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 271)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_util_AbstractList*) _r4.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    label28:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 273)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_clone__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "clone", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_TRY_BEGIN(w2588aaac15b1b2)
    // Begin try
    XMLVM_SOURCE_POSITION("ArrayList.java", 286)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_Object_clone__(_r2.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 287)
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_ArrayList*) _r2.o)->fields.java_util_ArrayList.array_;
    //java_lang_Object_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r1.o)->tib->vtable[0])(_r1.o);
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(0)
    ((java_util_ArrayList*) _r0.o)->fields.java_util_ArrayList.array_ = _r2.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w2588aaac15b1b2)
    XMLVM_CATCH_END(w2588aaac15b1b2)
    XMLVM_RESTORE_EXCEPTION_ENV(w2588aaac15b1b2)
    label16:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 290)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label17:;
    java_lang_Thread* curThread_w2588aaac15b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w2588aaac15b1b7->fields.java_lang_Thread.xmlvmException_;
    _r0.o = JAVA_NULL;
    goto label16;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_contains___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_contains___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "contains", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 304)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    if (_r5.o == JAVA_NULL) goto label29;
    XMLVM_SOURCE_POSITION("ArrayList.java", 305)
    XMLVM_SOURCE_POSITION("ArrayList.java", 306)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    label10:;
    if (_r1.i < _r0.i) goto label14;
    label12:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 318)
    _r0.i = 0;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 307)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[1])(_r5.o, _r2.o);
    if (_r2.i == 0) goto label26;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("ArrayList.java", 308)
    goto label13;
    label26:;
    _r1.i = _r1.i + 1;
    goto label10;
    label29:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 312)
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    label31:;
    if (_r1.i >= _r0.i) goto label12;
    XMLVM_SOURCE_POSITION("ArrayList.java", 313)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.o != JAVA_NULL) goto label41;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("ArrayList.java", 314)
    goto label13;
    label41:;
    _r1.i = _r1.i + 1;
    goto label31;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_ensureCapacity___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_ensureCapacity___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "ensureCapacity", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 329)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_ArrayList*) _r2.o)->fields.java_util_ArrayList.array_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r3.i - _r0.i;
    if (_r0.i <= 0) goto label14;
    XMLVM_SOURCE_POSITION("ArrayList.java", 330)
    XMLVM_SOURCE_POSITION("ArrayList.java", 333)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_util_ArrayList*) _r2.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r1.i <= 0) goto label15;
    XMLVM_SOURCE_POSITION("ArrayList.java", 334)
    XMLVM_CHECK_NPE(2)
    java_util_ArrayList_growAtFront___int(_r2.o, _r0.i);
    label14:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 339)
    XMLVM_EXIT_METHOD()
    return;
    label15:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 336)
    XMLVM_CHECK_NPE(2)
    java_util_ArrayList_growAtEnd___int(_r2.o, _r0.i);
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_get___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_get___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "get", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 343)
    if (_r5.i < 0) goto label6;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    if (_r5.i < _r0.i) goto label28;
    label6:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 344)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 346)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 347)
    _r2.o = java_lang_Integer_valueOf___int(_r5.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 348)
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 350)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i + _r5.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_growAtEnd___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_growAtEnd___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "growAtEnd", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.i = n1;
    _r2.i = 12;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 354)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i - _r1.i;
    if (_r0.i < _r6.i) goto label49;
    XMLVM_SOURCE_POSITION("ArrayList.java", 357)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r0.i == 0) goto label43;
    XMLVM_SOURCE_POSITION("ArrayList.java", 358)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 359)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r0.i >= _r1.i) goto label46;
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    label34:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 362)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r3.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    label43:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 364)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r4.i;
    label45:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 382)
    XMLVM_EXIT_METHOD()
    return;
    label46:;
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    goto label34;
    label49:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 368)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i / 2;
    XMLVM_SOURCE_POSITION("ArrayList.java", 369)
    if (_r6.i <= _r0.i) goto label56;
    _r0 = _r6;
    label56:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 370)
    XMLVM_SOURCE_POSITION("ArrayList.java", 372)
    if (_r0.i >= _r2.i) goto label59;
    _r0 = _r2;
    label59:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 373)
    XMLVM_SOURCE_POSITION("ArrayList.java", 375)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(5)
    _r0.o = java_util_ArrayList_newElementArray___int(_r5.o, _r0.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 376)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r1.i == 0) goto label81;
    XMLVM_SOURCE_POSITION("ArrayList.java", 377)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 378)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r4.i;
    label81:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 380)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_ = _r0.o;
    goto label45;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_growAtFront___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_growAtFront___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "growAtFront", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.i = n1;
    _r2.i = 12;
    XMLVM_SOURCE_POSITION("ArrayList.java", 385)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i - _r1.i;
    if (_r0.i < _r6.i) goto label50;
    XMLVM_SOURCE_POSITION("ArrayList.java", 386)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i - _r1.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 389)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r1.i == 0) goto label47;
    XMLVM_SOURCE_POSITION("ArrayList.java", 390)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r3.o, _r0.i, _r4.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 391)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i + _r2.i;
    if (_r1.i <= _r0.i) goto label39;
    XMLVM_SOURCE_POSITION("ArrayList.java", 392)
    _r1 = _r0;
    label39:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 393)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r4.o = JAVA_NULL;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    label47:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 395)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    label49:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 411)
    XMLVM_EXIT_METHOD()
    return;
    label50:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 397)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i / 2;
    XMLVM_SOURCE_POSITION("ArrayList.java", 398)
    if (_r6.i <= _r0.i) goto label57;
    _r0 = _r6;
    label57:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 399)
    XMLVM_SOURCE_POSITION("ArrayList.java", 401)
    if (_r0.i >= _r2.i) goto label60;
    _r0 = _r2;
    label60:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 402)
    XMLVM_SOURCE_POSITION("ArrayList.java", 404)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i + _r0.i;
    XMLVM_CHECK_NPE(5)
    _r1.o = java_util_ArrayList_newElementArray___int(_r5.o, _r1.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 405)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    if (_r2.i == 0) goto label80;
    XMLVM_SOURCE_POSITION("ArrayList.java", 406)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r1.o, _r0.i, _r4.i);
    label80:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 408)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i - _r2.i;
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 409)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_ = _r1.o;
    goto label49;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_growForInsert___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_growForInsert___int_int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "growForInsert", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = me;
    _r7.i = n1;
    _r8.i = n2;
    _r1.i = 12;
    XMLVM_SOURCE_POSITION("ArrayList.java", 417)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i / 2;
    XMLVM_SOURCE_POSITION("ArrayList.java", 418)
    if (_r8.i <= _r0.i) goto label9;
    _r0 = _r8;
    label9:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 419)
    XMLVM_SOURCE_POSITION("ArrayList.java", 421)
    if (_r0.i >= _r1.i) goto label12;
    _r0 = _r1;
    label12:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 422)
    XMLVM_SOURCE_POSITION("ArrayList.java", 424)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i + _r0.i;
    XMLVM_CHECK_NPE(6)
    _r1.o = java_util_ArrayList_newElementArray___int(_r6.o, _r1.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 427)
    _r0.i = _r0.i - _r8.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 430)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r3.i + _r7.i;
    _r4.i = _r0.i + _r7.i;
    _r4.i = _r4.i + _r8.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 431)
    XMLVM_CHECK_NPE(6)
    _r5.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r5.i = _r5.i - _r7.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r1.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 433)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r1.o, _r0.i, _r7.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 434)
    XMLVM_CHECK_NPE(6)
    ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 435)
    XMLVM_CHECK_NPE(6)
    ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_ = _r1.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 436)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_ArrayList_indexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_indexOf___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 441)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    if (_r4.o == JAVA_NULL) goto label31;
    XMLVM_SOURCE_POSITION("ArrayList.java", 442)
    XMLVM_SOURCE_POSITION("ArrayList.java", 443)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    label9:;
    if (_r1.i < _r0.i) goto label13;
    label11:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 455)
    _r0.i = -1;
    label12:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label13:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 444)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r2.o);
    if (_r2.i == 0) goto label28;
    XMLVM_SOURCE_POSITION("ArrayList.java", 445)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r1.i - _r0.i;
    goto label12;
    label28:;
    _r1.i = _r1.i + 1;
    goto label9;
    label31:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 449)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    label33:;
    if (_r1.i >= _r0.i) goto label11;
    XMLVM_SOURCE_POSITION("ArrayList.java", 450)
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.o != JAVA_NULL) goto label46;
    XMLVM_SOURCE_POSITION("ArrayList.java", 451)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r1.i - _r0.i;
    goto label12;
    label46:;
    _r1.i = _r1.i + 1;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_isEmpty__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_isEmpty__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "isEmpty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ArrayList.java", 460)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_ArrayList*) _r1.o)->fields.java_util_ArrayList.size_;
    if (_r0.i != 0) goto label6;
    _r0.i = 1;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 0;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_ArrayList_lastIndexOf___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_lastIndexOf___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 465)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    if (_r4.o == JAVA_NULL) goto label32;
    XMLVM_SOURCE_POSITION("ArrayList.java", 466)
    XMLVM_SOURCE_POSITION("ArrayList.java", 467)
    _r0.i = _r0.i - _r2.i;
    label9:;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r0.i >= _r1.i) goto label15;
    label13:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 479)
    _r0.i = -1;
    label14:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 468)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_lang_Object_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_Object*) _r4.o)->tib->vtable[1])(_r4.o, _r1.o);
    if (_r1.i == 0) goto label29;
    XMLVM_SOURCE_POSITION("ArrayList.java", 469)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r0.i - _r1.i;
    goto label14;
    label29:;
    _r0.i = _r0.i + -1;
    goto label9;
    label32:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 473)
    _r0.i = _r0.i - _r2.i;
    label33:;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    if (_r0.i < _r1.i) goto label13;
    XMLVM_SOURCE_POSITION("ArrayList.java", 474)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.o != JAVA_NULL) goto label47;
    XMLVM_SOURCE_POSITION("ArrayList.java", 475)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r0.i - _r1.i;
    goto label14;
    label47:;
    _r0.i = _r0.i + -1;
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_remove___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_remove___int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    _r8.o = me;
    _r9.i = n1;
    _r7.o = JAVA_NULL;
    _r6.i = 1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 494)
    if (_r9.i < 0) goto label8;
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    if (_r9.i < _r0.i) goto label30;
    label8:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 495)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 497)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 498)
    _r2.o = java_lang_Integer_valueOf___int(_r9.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 499)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label30:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 501)
    if (_r9.i != 0) goto label67;
    XMLVM_SOURCE_POSITION("ArrayList.java", 502)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("ArrayList.java", 503)
    XMLVM_CHECK_NPE(8)
    _r1.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_ = _r3.i;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r7.o;
    label48:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 521)
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r1.i - _r6.i;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_ = _r1.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 525)
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    if (_r1.i != 0) goto label60;
    XMLVM_SOURCE_POSITION("ArrayList.java", 526)
    _r1.i = 0;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_ = _r1.i;
    label60:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 529)
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_;
    _r1.i = _r1.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_AbstractList*) _r8.o)->fields.java_util_AbstractList.modCount_ = _r1.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 530)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label67:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 504)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i - _r6.i;
    if (_r9.i != _r0.i) goto label88;
    XMLVM_SOURCE_POSITION("ArrayList.java", 505)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r1.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    _r0.i = _r0.i - _r6.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 506)
    XMLVM_CHECK_NPE(8)
    _r1.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("ArrayList.java", 507)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r7.o;
    _r0 = _r1;
    goto label48;
    label88:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 509)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r0.i + _r9.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 510)
    XMLVM_CHECK_NPE(8)
    _r1.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("ArrayList.java", 511)
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i / 2;
    if (_r9.i >= _r2.i) goto label126;
    XMLVM_SOURCE_POSITION("ArrayList.java", 512)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r4.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r4.i = _r4.i + 1;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r2.i, _r3.o, _r4.i, _r9.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 514)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = _r2.i + 1;
    XMLVM_CHECK_NPE(8)
    ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_ = _r3.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r7.o;
    _r0 = _r1;
    goto label48;
    label126:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 516)
    XMLVM_CHECK_NPE(8)
    _r2.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    _r3.i = _r0.i + 1;
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_SOURCE_POSITION("ArrayList.java", 517)
    XMLVM_CHECK_NPE(8)
    _r5.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r5.i = _r5.i - _r9.i;
    _r5.i = _r5.i - _r6.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r4.o, _r0.i, _r5.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 518)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(8)
    _r2.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_util_ArrayList*) _r8.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;
    _r2.i = _r2.i - _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r7.o;
    _r0 = _r1;
    goto label48;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_ArrayList_remove___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_remove___java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 535)
    //java_util_ArrayList_indexOf___java_lang_Object[22]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[22])(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 536)
    if (_r0.i < 0) goto label11;
    XMLVM_SOURCE_POSITION("ArrayList.java", 537)
    //java_util_ArrayList_remove___int[27]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[27])(_r1.o, _r0.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 538)
    _r0.i = 1;
    label10:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 540)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    _r0.i = 0;
    goto label10;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_removeRange___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_removeRange___int_int]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "removeRange", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = me;
    _r7.i = n1;
    _r8.i = n2;
    _r5.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("ArrayList.java", 557)
    if (_r7.i >= 0) goto label19;
    XMLVM_SOURCE_POSITION("ArrayList.java", 559)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 561)
    // "luni.0B"
    _r1.o = xmlvm_create_java_string_from_pool(163);
    XMLVM_SOURCE_POSITION("ArrayList.java", 562)
    _r2.o = java_lang_Integer_valueOf___int(_r7.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 563)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    if (_r8.i <= _r0.i) goto label45;
    XMLVM_SOURCE_POSITION("ArrayList.java", 565)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 567)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 568)
    _r2.o = java_lang_Integer_valueOf___int(_r8.i);
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label45:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 569)
    if (_r7.i <= _r8.i) goto label67;
    XMLVM_SOURCE_POSITION("ArrayList.java", 570)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 572)
    // "luni.35"
    _r1.o = xmlvm_create_java_string_from_pool(164);
    XMLVM_SOURCE_POSITION("ArrayList.java", 573)
    _r2.o = java_lang_Integer_valueOf___int(_r7.i);
    _r3.o = java_lang_Integer_valueOf___int(_r8.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label67:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 576)
    if (_r7.i != _r8.i) goto label70;
    label69:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 594)
    XMLVM_EXIT_METHOD()
    return;
    label70:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 579)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    if (_r8.i != _r0.i) goto label101;
    XMLVM_SOURCE_POSITION("ArrayList.java", 580)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i + _r7.i;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r2.i = _r2.i + _r3.i;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    label87:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 592)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r1.i = _r8.i - _r7.i;
    _r0.i = _r0.i - _r1.i;
    XMLVM_CHECK_NPE(6)
    ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 593)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_AbstractList*) _r6.o)->fields.java_util_AbstractList.modCount_ = _r0.i;
    goto label69;
    label101:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 581)
    if (_r7.i != 0) goto label119;
    XMLVM_SOURCE_POSITION("ArrayList.java", 582)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    _r2.i = _r2.i + _r8.i;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 583)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    _r0.i = _r0.i + _r8.i;
    XMLVM_CHECK_NPE(6)
    ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_ = _r0.i;
    goto label87;
    label119:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 586)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i + _r8.i;
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(6)
    _r3.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_SOURCE_POSITION("ArrayList.java", 587)
    _r3.i = _r3.i + _r7.i;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r4.i = _r4.i - _r8.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 588)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.size_;
    _r0.i = _r0.i + _r1.i;
    _r1.i = _r0.i + _r7.i;
    _r1.i = _r1.i - _r8.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 590)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_ArrayList*) _r6.o)->fields.java_util_ArrayList.array_;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label87;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_set___int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_set___int_java_lang_Object]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "set", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.o = n2;
    XMLVM_SOURCE_POSITION("ArrayList.java", 610)
    if (_r5.i < 0) goto label6;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    if (_r5.i < _r0.i) goto label28;
    label6:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 611)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_SOURCE_POSITION("ArrayList.java", 613)
    // "luni.0A"
    _r1.o = xmlvm_create_java_string_from_pool(162);
    XMLVM_SOURCE_POSITION("ArrayList.java", 614)
    _r2.o = java_lang_Integer_valueOf___int(_r5.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 615)
    XMLVM_CHECK_NPE(4)
    _r3.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.size_;
    _r3.o = java_lang_Integer_valueOf___int(_r3.i);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r2.o, _r3.o);
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 617)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    _r1.i = _r1.i + _r5.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("ArrayList.java", 618)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_util_ArrayList*) _r4.o)->fields.java_util_ArrayList.firstIndex_;
    _r2.i = _r2.i + _r5.i;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r6.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 619)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_ArrayList_size__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_size__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "size", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ArrayList.java", 629)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_util_ArrayList*) _r1.o)->fields.java_util_ArrayList.size_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_toArray__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_toArray__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("ArrayList.java", 640)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r0.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 641)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = 0;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 642)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_ArrayList_toArray___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_toArray___java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "toArray", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 663)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r0.i <= _r1.i) goto label42;
    XMLVM_SOURCE_POSITION("ArrayList.java", 664)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(6)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r6.o)->tib->vtable[3])(_r6.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    XMLVM_SOURCE_POSITION("ArrayList.java", 665)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r0.o = java_lang_reflect_Array_newInstance___java_lang_Class_int(_r0.o, _r1.i);
    _r0.o = _r0.o;
    label21:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 667)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    _r3.i = 0;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 668)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i >= _r2.i) goto label41;
    XMLVM_SOURCE_POSITION("ArrayList.java", 671)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    _r2.o = JAVA_NULL;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    label41:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 673)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label42:;
    _r0 = _r6;
    goto label21;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_trimToSize__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_trimToSize__]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "trimToSize", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 683)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    XMLVM_CHECK_NPE(5)
    _r0.o = java_util_ArrayList_newElementArray___int(_r5.o, _r0.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 684)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_;
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.size_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r0.o, _r4.i, _r3.i);
    XMLVM_SOURCE_POSITION("ArrayList.java", 685)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.array_ = _r0.o;
    XMLVM_SOURCE_POSITION("ArrayList.java", 686)
    XMLVM_CHECK_NPE(5)
    ((java_util_ArrayList*) _r5.o)->fields.java_util_ArrayList.firstIndex_ = _r4.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 687)
    XMLVM_CHECK_NPE(5)
    ((java_util_AbstractList*) _r5.o)->fields.java_util_AbstractList.modCount_ = _r4.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 688)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("ArrayList.java", 694)

    
    // Red class access removed: java.io.ObjectOutputStream::putFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 695)
    // "size"
    _r1.o = xmlvm_create_java_string_from_pool(161);
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.size_;

    
    // Red class access removed: java.io.ObjectOutputStream$PutField::put
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 696)

    
    // Red class access removed: java.io.ObjectOutputStream::writeFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 697)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));

    
    // Red class access removed: java.io.ObjectOutputStream::writeInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 698)
    //java_util_ArrayList_iterator__[12]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r3.o)->tib->vtable[12])(_r3.o);
    label24:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 699)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label31;
    XMLVM_SOURCE_POSITION("ArrayList.java", 702)
    XMLVM_EXIT_METHOD()
    return;
    label31:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 700)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);

    
    // Red class access removed: java.io.ObjectOutputStream::writeObject
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label24;
    //XMLVM_END_WRAPPER
}

void java_util_ArrayList_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_ArrayList_readObject___java_io_ObjectInputStream]
    XMLVM_ENTER_METHOD("java.util.ArrayList", "readObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("ArrayList.java", 707)

    
    // Red class access removed: java.io.ObjectInputStream::readFields
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ArrayList.java", 708)
    // "size"
    _r1.o = xmlvm_create_java_string_from_pool(161);

    
    // Red class access removed: java.io.ObjectInputStream$GetField::get
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.size_ = _r0.i;
    XMLVM_SOURCE_POSITION("ArrayList.java", 709)

    
    // Red class access removed: java.io.ObjectInputStream::readInt
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    _r0.o = java_util_ArrayList_newElementArray___int(_r3.o, _r0.i);
    XMLVM_CHECK_NPE(3)
    ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_ = _r0.o;
    _r0 = _r2;
    label24:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 710)
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.size_;
    if (_r0.i < _r1.i) goto label29;
    XMLVM_SOURCE_POSITION("ArrayList.java", 713)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_SOURCE_POSITION("ArrayList.java", 711)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_util_ArrayList*) _r3.o)->fields.java_util_ArrayList.array_;

    
    // Red class access removed: java.io.ObjectInputStream::readObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r2.o;
    _r0.i = _r0.i + 1;
    goto label24;
    //XMLVM_END_WRAPPER
}

