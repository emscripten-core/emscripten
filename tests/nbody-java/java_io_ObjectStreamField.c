#include "xmlvm.h"
#include "java_lang_Boolean.h"
#include "java_lang_Byte.h"
#include "java_lang_Character.h"
#include "java_lang_Class.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_ClassNotFoundException.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "java_lang_Integer.h"
#include "java_lang_Long.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Short.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_ref_WeakReference.h"

#include "java_io_ObjectStreamField.h"

#define XMLVM_CURRENT_CLASS_NAME ObjectStreamField
#define XMLVM_CURRENT_PKG_CLASS_NAME java_io_ObjectStreamField

__TIB_DEFINITION_java_io_ObjectStreamField __TIB_java_io_ObjectStreamField = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_io_ObjectStreamField, // classInitializer
    "java.io.ObjectStreamField", // className
    "java.io", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/lang/Comparable<Ljava/lang/Object;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_io_ObjectStreamField), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_io_ObjectStreamField;
JAVA_OBJECT __CLASS_java_io_ObjectStreamField_1ARRAY;
JAVA_OBJECT __CLASS_java_io_ObjectStreamField_2ARRAY;
JAVA_OBJECT __CLASS_java_io_ObjectStreamField_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_io_ObjectStreamField_FIELD_IS_NOT_RESOLVED;
static JAVA_INT _STATIC_java_io_ObjectStreamField_FIELD_IS_ABSENT;

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

void __INIT_java_io_ObjectStreamField()
{
    staticInitializerLock(&__TIB_java_io_ObjectStreamField);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_io_ObjectStreamField.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_io_ObjectStreamField.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_io_ObjectStreamField);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_io_ObjectStreamField.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_io_ObjectStreamField.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_io_ObjectStreamField.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.io.ObjectStreamField")
        __INIT_IMPL_java_io_ObjectStreamField();
    }
}

void __INIT_IMPL_java_io_ObjectStreamField()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_io_ObjectStreamField.newInstanceFunc = __NEW_INSTANCE_java_io_ObjectStreamField;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_io_ObjectStreamField.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_io_ObjectStreamField.vtable[6] = (VTABLE_PTR) &java_io_ObjectStreamField_compareTo___java_lang_Object;
    __TIB_java_io_ObjectStreamField.vtable[1] = (VTABLE_PTR) &java_io_ObjectStreamField_equals___java_lang_Object;
    __TIB_java_io_ObjectStreamField.vtable[4] = (VTABLE_PTR) &java_io_ObjectStreamField_hashCode__;
    __TIB_java_io_ObjectStreamField.vtable[5] = (VTABLE_PTR) &java_io_ObjectStreamField_toString__;
    // Initialize interface information
    __TIB_java_io_ObjectStreamField.numImplementedInterfaces = 1;
    __TIB_java_io_ObjectStreamField.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_io_ObjectStreamField.implementedInterfaces[0][0] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_io_ObjectStreamField.itableBegin = &__TIB_java_io_ObjectStreamField.itable[0];
    __TIB_java_io_ObjectStreamField.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_io_ObjectStreamField.vtable[6];

    _STATIC_java_io_ObjectStreamField_FIELD_IS_NOT_RESOLVED = -1;
    _STATIC_java_io_ObjectStreamField_FIELD_IS_ABSENT = -2;

    __TIB_java_io_ObjectStreamField.declaredFields = &__field_reflection_data[0];
    __TIB_java_io_ObjectStreamField.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_io_ObjectStreamField.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_io_ObjectStreamField.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_io_ObjectStreamField.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_io_ObjectStreamField.methodDispatcherFunc = method_dispatcher;
    __TIB_java_io_ObjectStreamField.declaredMethods = &__method_reflection_data[0];
    __TIB_java_io_ObjectStreamField.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_io_ObjectStreamField = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_io_ObjectStreamField);
    __TIB_java_io_ObjectStreamField.clazz = __CLASS_java_io_ObjectStreamField;
    __TIB_java_io_ObjectStreamField.baseType = JAVA_NULL;
    __CLASS_java_io_ObjectStreamField_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_ObjectStreamField);
    __CLASS_java_io_ObjectStreamField_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_ObjectStreamField_1ARRAY);
    __CLASS_java_io_ObjectStreamField_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_io_ObjectStreamField_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_io_ObjectStreamField]
    //XMLVM_END_WRAPPER

    __TIB_java_io_ObjectStreamField.classInitialized = 1;
}

void __DELETE_java_io_ObjectStreamField(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_io_ObjectStreamField]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamField(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.name_ = (java_lang_String*) JAVA_NULL;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.type_ = (java_lang_Object*) JAVA_NULL;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.offset_ = 0;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.typeString_ = (java_lang_String*) JAVA_NULL;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.unshared_ = 0;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.isDeserialized_ = 0;
    ((java_io_ObjectStreamField*) me)->fields.java_io_ObjectStreamField.assocFieldID_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_io_ObjectStreamField]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_io_ObjectStreamField()
{    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
java_io_ObjectStreamField* me = (java_io_ObjectStreamField*) XMLVM_MALLOC(sizeof(java_io_ObjectStreamField));
    me->tib = &__TIB_java_io_ObjectStreamField;
    __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamField(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_io_ObjectStreamField]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_io_ObjectStreamField()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_INT java_io_ObjectStreamField_GET_FIELD_IS_NOT_RESOLVED()
{
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    return _STATIC_java_io_ObjectStreamField_FIELD_IS_NOT_RESOLVED;
}

void java_io_ObjectStreamField_PUT_FIELD_IS_NOT_RESOLVED(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
_STATIC_java_io_ObjectStreamField_FIELD_IS_NOT_RESOLVED = v;
}

JAVA_INT java_io_ObjectStreamField_GET_FIELD_IS_ABSENT()
{
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    return _STATIC_java_io_ObjectStreamField_FIELD_IS_ABSENT;
}

void java_io_ObjectStreamField_PUT_FIELD_IS_ABSENT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
_STATIC_java_io_ObjectStreamField_FIELD_IS_ABSENT = v;
}

JAVA_LONG java_io_ObjectStreamField_getFieldID___org_apache_harmony_misc_accessors_ObjectAccessor_java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getFieldID___org_apache_harmony_misc_accessors_ObjectAccessor_java_lang_Class]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getFieldID", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r4.o = me;
    _r5.o = n1;
    _r6.o = n2;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 58)
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.assocFieldID_;
    _r2.l = -1;
    _r0.i = _r0.l > _r2.l ? 1 : (_r0.l == _r2.l ? 0 : -1);
    if (_r0.i == 0) goto label11;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 59)
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.assocFieldID_;
    label10:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 66)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label11:;
    XMLVM_TRY_BEGIN(w308aaab9b1c15)
    // Begin try
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 62)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.name_;

    
    // Red class access removed: org.apache.harmony.misc.accessors.ObjectAccessor::getFieldID
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(4)
    ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.assocFieldID_ = _r0.l;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w308aaab9b1c15)
    XMLVM_CATCH_END(w308aaab9b1c15)
    XMLVM_RESTORE_EXCEPTION_ENV(w308aaab9b1c15)
    label19:;
    XMLVM_CHECK_NPE(4)
    _r0.l = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.assocFieldID_;
    goto label10;
    label22:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 64)
    java_lang_Thread* curThread_w308aaab9b1c21 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w308aaab9b1c21->fields.java_lang_Thread.xmlvmException_;
    _r0.l = -2;
    XMLVM_CHECK_NPE(4)
    ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.assocFieldID_ = _r0.l;
    goto label19;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 80)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 55)
    _r0.l = -1;
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.assocFieldID_ = _r0.l;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 81)
    if (_r3.o == JAVA_NULL) goto label11;
    if (_r4.o != JAVA_NULL) goto label17;
    label11:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 82)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 84)
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.name_ = _r3.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 85)
    _r0.o = __NEW_java_lang_ref_WeakReference();
    XMLVM_CHECK_NPE(0)
    java_lang_ref_WeakReference___INIT____java_lang_Object(_r0.o, _r4.o);
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 86)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField___INIT____java_lang_String_java_lang_Class_boolean]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 103)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 55)
    _r0.l = -1;
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.assocFieldID_ = _r0.l;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 104)
    if (_r3.o == JAVA_NULL) goto label11;
    if (_r4.o != JAVA_NULL) goto label17;
    label11:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 105)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 107)
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.name_ = _r3.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 108)
    XMLVM_CHECK_NPE(4)
    _r0.o = java_lang_Class_getClassLoader__(_r4.o);
    if (_r0.o != JAVA_NULL) goto label31;
    _r0 = _r4;
    label26:;
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 110)
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.unshared_ = _r5.i;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 111)
    XMLVM_EXIT_METHOD()
    return;
    label31:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 109)
    _r0.o = __NEW_java_lang_ref_WeakReference();
    XMLVM_CHECK_NPE(0)
    java_lang_ref_WeakReference___INIT____java_lang_Object(_r0.o, _r4.o);
    goto label26;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 122)
    XMLVM_CHECK_NPE(2)
    java_lang_Object___INIT___(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 55)
    _r0.l = -1;
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.assocFieldID_ = _r0.l;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 123)
    if (_r4.o != JAVA_NULL) goto label15;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 124)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label15:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 126)
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.name_ = _r4.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 127)
    _r0.i = 46;
    _r1.i = 47;
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_String_replace___char_char(_r3.o, _r0.i, _r1.i);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_intern__(_r0.o);
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.typeString_ = _r0.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 128)
    XMLVM_CHECK_NPE(2)
    java_io_ObjectStreamField_defaultResolve__(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 129)
    _r0.i = 1;
    XMLVM_CHECK_NPE(2)
    ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.isDeserialized_ = _r0.i;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 130)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_io_ObjectStreamField_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 145)
    _r3.o = _r3.o;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 146)
    XMLVM_CHECK_NPE(2)
    _r0.i = java_io_ObjectStreamField_isPrimitive__(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 147)
    XMLVM_CHECK_NPE(3)
    _r1.i = java_io_ObjectStreamField_isPrimitive__(_r3.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 151)
    if (_r0.i == _r1.i) goto label18;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 152)
    if (_r0.i == 0) goto label16;
    _r0.i = -1;
    label15:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 156)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0.i = 1;
    goto label15;
    label18:;
    XMLVM_CHECK_NPE(2)
    _r0.o = java_io_ObjectStreamField_getName__(_r2.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_io_ObjectStreamField_getName__(_r3.o);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_compareTo___java_lang_String(_r0.o, _r1.o);
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_io_ObjectStreamField_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 161)
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    _r0.i = XMLVM_ISA(_r2.o, __CLASS_java_io_ObjectStreamField);
    if (_r0.i == 0) goto label12;
    //java_io_ObjectStreamField_compareTo___java_lang_Object[6]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_io_ObjectStreamField*) _r1.o)->tib->vtable[6])(_r1.o, _r2.o);
    if (_r0.i != 0) goto label12;
    _r0.i = 1;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    _r0.i = 0;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_io_ObjectStreamField_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_hashCode__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 166)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_io_ObjectStreamField_getName__(_r1.o);
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_ObjectStreamField_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getName__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 175)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.name_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_io_ObjectStreamField_getOffset__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getOffset__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getOffset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 184)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.offset_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_ObjectStreamField_getTypeInternal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getTypeInternal__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getTypeInternal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 194)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.type_;
    XMLVM_CLASS_INIT(java_lang_ref_WeakReference)
    _r0.i = XMLVM_ISA(_r0.o, __CLASS_java_lang_ref_WeakReference);
    if (_r0.i == 0) goto label18;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 195)
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.type_;
    _r1.o = _r1.o;
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_ref_Reference_get__(_r1.o);
    _r1.o = _r1.o;
    _r0 = _r1;
    label17:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 197)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label18:;
    XMLVM_CHECK_NPE(1)
    _r1.o = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.type_;
    _r1.o = _r1.o;
    _r0 = _r1;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_ObjectStreamField_getType__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getType__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 206)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_io_ObjectStreamField_getTypeInternal__(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 207)
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_io_ObjectStreamField*) _r2.o)->fields.java_io_ObjectStreamField.isDeserialized_;
    if (_r1.i == 0) goto label16;
    XMLVM_CHECK_NPE(0)
    _r1.i = java_lang_Class_isPrimitive__(_r0.o);
    if (_r1.i != 0) goto label16;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 208)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = __CLASS_java_lang_Object;
    label16:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 210)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_io_ObjectStreamField_getTypeCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getTypeCode__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getTypeCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 233)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_io_ObjectStreamField_getTypeInternal__(_r2.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 234)
    _r1.o = java_lang_Integer_GET_TYPE();
    if (_r0.o != _r1.o) goto label11;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 235)
    _r0.i = 73;
    label10:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 261)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 237)
    _r1.o = java_lang_Byte_GET_TYPE();
    if (_r0.o != _r1.o) goto label18;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 238)
    _r0.i = 66;
    goto label10;
    label18:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 240)
    _r1.o = java_lang_Character_GET_TYPE();
    if (_r0.o != _r1.o) goto label25;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 241)
    _r0.i = 67;
    goto label10;
    label25:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 243)
    _r1.o = java_lang_Short_GET_TYPE();
    if (_r0.o != _r1.o) goto label32;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 244)
    _r0.i = 83;
    goto label10;
    label32:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 246)
    _r1.o = java_lang_Boolean_GET_TYPE();
    if (_r0.o != _r1.o) goto label39;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 247)
    _r0.i = 90;
    goto label10;
    label39:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 249)
    _r1.o = java_lang_Long_GET_TYPE();
    if (_r0.o != _r1.o) goto label46;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 250)
    _r0.i = 74;
    goto label10;
    label46:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 252)
    _r1.o = java_lang_Float_GET_TYPE();
    if (_r0.o != _r1.o) goto label53;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 253)
    _r0.i = 70;
    goto label10;
    label53:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 255)
    _r1.o = java_lang_Double_GET_TYPE();
    if (_r0.o != _r1.o) goto label60;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 256)
    _r0.i = 68;
    goto label10;
    label60:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 258)
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_Class_isArray__(_r0.o);
    if (_r0.i == 0) goto label69;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 259)
    _r0.i = 91;
    goto label10;
    label69:;
    _r0.i = 76;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_ObjectStreamField_getTypeString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_getTypeString__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "getTypeString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 272)
    XMLVM_CHECK_NPE(4)
    _r0.i = java_io_ObjectStreamField_isPrimitive__(_r4.o);
    if (_r0.i == 0) goto label8;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 273)
    _r0.o = JAVA_NULL;
    label7:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 281)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label8:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 275)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.typeString_;
    if (_r0.o != JAVA_NULL) goto label41;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 276)
    XMLVM_CHECK_NPE(4)
    _r0.o = java_io_ObjectStreamField_getTypeInternal__(_r4.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 277)
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getName__(_r0.o);
    _r2.i = 46;
    _r3.i = 47;
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_String_replace___char_char(_r1.o, _r2.i, _r3.i);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 278)
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_Class_isArray__(_r0.o);
    if (_r0.i == 0) goto label44;
    _r0 = _r1;
    label35:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 279)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_intern__(_r0.o);
    XMLVM_CHECK_NPE(4)
    ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.typeString_ = _r0.o;
    label41:;
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_io_ObjectStreamField*) _r4.o)->fields.java_io_ObjectStreamField.typeString_;
    goto label7;
    label44:;
    _r0.o = __NEW_java_lang_StringBuilder();
    // "L"
    _r2.o = xmlvm_create_java_string_from_pool(18);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    _r1.i = 59;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label35;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_io_ObjectStreamField_isPrimitive__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_isPrimitive__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "isPrimitive", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 291)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_io_ObjectStreamField_getTypeInternal__(_r1.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 292)
    if (_r0.o == JAVA_NULL) goto label14;
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_Class_isPrimitive__(_r0.o);
    if (_r0.i == 0) goto label14;
    _r0.i = 1;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 0;
    goto label13;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField_setOffset___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_setOffset___int]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "setOffset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 302)
    XMLVM_CHECK_NPE(0)
    ((java_io_ObjectStreamField*) _r0.o)->fields.java_io_ObjectStreamField.offset_ = _r1.i;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 303)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_io_ObjectStreamField_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_toString__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 313)
    _r0.o = __NEW_java_lang_StringBuilder();
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(2)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[3])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_Class_getName__(_r1.o);
    _r1.o = java_lang_String_valueOf___java_lang_Object(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    _r1.i = 40;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_CHECK_NPE(2)
    _r1.o = java_io_ObjectStreamField_getName__(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    _r1.i = 58;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 314)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_io_ObjectStreamField_getTypeInternal__(_r2.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_Object(_r0.o, _r1.o);
    _r1.i = 41;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField_sortFields___java_io_ObjectStreamField_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_io_ObjectStreamField)
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_sortFields___java_io_ObjectStreamField_1ARRAY]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "sortFields", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 326)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r1.i = 1;
    if (_r0.i <= _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 327)

    
    // Red class access removed: java.io.ObjectStreamField$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.ObjectStreamField$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 332)

    
    // Red class access removed: java.util.Arrays::sort
    XMLVM_RED_CLASS_DEPENDENCY();
    label12:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 334)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField_resolve___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_resolve___java_lang_ClassLoader]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "resolve", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    _r4.i = 0;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 337)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_io_ObjectStreamField*) _r5.o)->fields.java_io_ObjectStreamField.typeString_;
    if (_r0.o != JAVA_NULL) goto label22;
    XMLVM_CHECK_NPE(5)
    _r0.i = java_io_ObjectStreamField_isPrimitive__(_r5.o);
    if (_r0.i == 0) goto label22;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 339)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_io_ObjectStreamField_getTypeCode__(_r5.o);
    _r0.o = java_lang_String_valueOf___char(_r0.i);
    XMLVM_CHECK_NPE(5)
    ((java_io_ObjectStreamField*) _r5.o)->fields.java_io_ObjectStreamField.typeString_ = _r0.o;
    label22:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 342)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_io_ObjectStreamField*) _r5.o)->fields.java_io_ObjectStreamField.typeString_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    if (_r0.i != _r3.i) goto label37;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 343)
    XMLVM_CHECK_NPE(5)
    _r0.i = java_io_ObjectStreamField_defaultResolve__(_r5.o);
    if (_r0.i == 0) goto label37;
    label36:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 360)
    XMLVM_EXIT_METHOD()
    return;
    label37:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 348)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_io_ObjectStreamField*) _r5.o)->fields.java_io_ObjectStreamField.typeString_;
    _r1.i = 47;
    _r2.i = 46;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replace___char_char(_r0.o, _r1.i, _r2.i);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 349)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r0.o)->tib->vtable[6])(_r0.o, _r4.i);
    _r2.i = 76;
    if (_r1.i != _r2.i) goto label64;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 351)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    _r1.i = _r1.i - _r3.i;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_substring___int_int(_r0.o, _r3.i, _r1.i);
    label64:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 354)
    _r1.i = 0;
    XMLVM_TRY_BEGIN(w308aaac26b1c42)
    // Begin try
    _r0.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r0.o, _r1.i, _r6.o);
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 355)
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getClassLoader__(_r0.o);
    if (_r1.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w308aaac26b1c42->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w308aaac26b1c42, sizeof(XMLVM_JMP_BUF)); goto label80; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w308aaac26b1c42)
        XMLVM_CATCH_SPECIFIC(w308aaac26b1c42,java_lang_ClassNotFoundException,78)
    XMLVM_CATCH_END(w308aaac26b1c42)
    XMLVM_RESTORE_EXCEPTION_ENV(w308aaac26b1c42)
    label75:;
    XMLVM_TRY_BEGIN(w308aaac26b1c44)
    // Begin try
    XMLVM_CHECK_NPE(5)
    ((java_io_ObjectStreamField*) _r5.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    { XMLVM_MEMCPY(curThread_w308aaac26b1c44->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w308aaac26b1c44, sizeof(XMLVM_JMP_BUF)); goto label36; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w308aaac26b1c44)
        XMLVM_CATCH_SPECIFIC(w308aaac26b1c44,java_lang_ClassNotFoundException,78)
    XMLVM_CATCH_END(w308aaac26b1c44)
    XMLVM_RESTORE_EXCEPTION_ENV(w308aaac26b1c44)
    label78:;
    XMLVM_TRY_BEGIN(w308aaac26b1c46)
    // Begin try
    java_lang_Thread* curThread_w308aaac26b1c46aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w308aaac26b1c46aa->fields.java_lang_Thread.xmlvmException_;
    { XMLVM_MEMCPY(curThread_w308aaac26b1c46->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w308aaac26b1c46, sizeof(XMLVM_JMP_BUF)); goto label36; };
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 356)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w308aaac26b1c46)
        XMLVM_CATCH_SPECIFIC(w308aaac26b1c46,java_lang_ClassNotFoundException,78)
    XMLVM_CATCH_END(w308aaac26b1c46)
    XMLVM_RESTORE_EXCEPTION_ENV(w308aaac26b1c46)
    label80:;
    XMLVM_TRY_BEGIN(w308aaac26b1c48)
    // Begin try
    _r1.o = __NEW_java_lang_ref_WeakReference();
    XMLVM_CHECK_NPE(1)
    java_lang_ref_WeakReference___INIT____java_lang_Object(_r1.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w308aaac26b1c48)
        XMLVM_CATCH_SPECIFIC(w308aaac26b1c48,java_lang_ClassNotFoundException,78)
    XMLVM_CATCH_END(w308aaac26b1c48)
    XMLVM_RESTORE_EXCEPTION_ENV(w308aaac26b1c48)
    _r0 = _r1;
    goto label75;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_io_ObjectStreamField_isUnshared__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_isUnshared__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "isUnshared", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 368)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_io_ObjectStreamField*) _r1.o)->fields.java_io_ObjectStreamField.unshared_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_io_ObjectStreamField_setUnshared___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_setUnshared___boolean]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "setUnshared", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 372)
    XMLVM_CHECK_NPE(0)
    ((java_io_ObjectStreamField*) _r0.o)->fields.java_io_ObjectStreamField.unshared_ = _r1.i;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 373)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_io_ObjectStreamField_defaultResolve__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_io_ObjectStreamField_defaultResolve__]
    XMLVM_ENTER_METHOD("java.io.ObjectStreamField", "defaultResolve", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    _r2.i = 0;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 380)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.typeString_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r0.o)->tib->vtable[6])(_r0.o, _r2.i);
    switch (_r0.i) {
    case 66: goto label23;
    case 67: goto label29;
    case 68: goto label59;
    case 70: goto label53;
    case 73: goto label17;
    case 74: goto label47;
    case 83: goto label35;
    case 90: goto label41;
    }
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 406)
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = __CLASS_java_lang_Object;
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r2;
    label16:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 407)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label17:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 382)
    _r0.o = java_lang_Integer_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 383)
    goto label16;
    label23:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 385)
    _r0.o = java_lang_Byte_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 386)
    goto label16;
    label29:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 388)
    _r0.o = java_lang_Character_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 389)
    goto label16;
    label35:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 391)
    _r0.o = java_lang_Short_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 392)
    goto label16;
    label41:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 394)
    _r0.o = java_lang_Boolean_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 395)
    goto label16;
    label47:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 397)
    _r0.o = java_lang_Long_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 398)
    goto label16;
    label53:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 400)
    _r0.o = java_lang_Float_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 401)
    goto label16;
    label59:;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 403)
    _r0.o = java_lang_Double_GET_TYPE();
    XMLVM_CHECK_NPE(3)
    ((java_io_ObjectStreamField*) _r3.o)->fields.java_io_ObjectStreamField.type_ = _r0.o;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("ObjectStreamField.java", 404)
    goto label16;
    label66:;
    //XMLVM_END_WRAPPER
}

