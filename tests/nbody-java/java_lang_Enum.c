#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Exception.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_reflect_Method.h"
#include "java_security_AccessController.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_lang_Enum.h"

#define XMLVM_CURRENT_CLASS_NAME Enum
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Enum

__TIB_DEFINITION_java_lang_Enum __TIB_java_lang_Enum = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Enum, // classInitializer
    "java.lang.Enum", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<E:Ljava/lang/Enum<TE;>;>Ljava/lang/Object;Ljava/io/Serializable;Ljava/lang/Comparable<TE;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Enum), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Enum;
JAVA_OBJECT __CLASS_java_lang_Enum_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Enum_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Enum_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Enum_serialVersionUID;

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

void __INIT_java_lang_Enum()
{
    staticInitializerLock(&__TIB_java_lang_Enum);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Enum.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Enum.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Enum);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Enum.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Enum.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Enum.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Enum")
        __INIT_IMPL_java_lang_Enum();
    }
}

void __INIT_IMPL_java_lang_Enum()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Enum.newInstanceFunc = __NEW_INSTANCE_java_lang_Enum;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Enum.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Enum.vtable[5] = (VTABLE_PTR) &java_lang_Enum_toString__;
    __TIB_java_lang_Enum.vtable[1] = (VTABLE_PTR) &java_lang_Enum_equals___java_lang_Object;
    __TIB_java_lang_Enum.vtable[4] = (VTABLE_PTR) &java_lang_Enum_hashCode__;
    __TIB_java_lang_Enum.vtable[0] = (VTABLE_PTR) &java_lang_Enum_clone__;
    __TIB_java_lang_Enum.vtable[2] = (VTABLE_PTR) &java_lang_Enum_finalize_java_lang_Enum__;
    __TIB_java_lang_Enum.vtable[6] = (VTABLE_PTR) &java_lang_Enum_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Enum.numImplementedInterfaces = 2;
    __TIB_java_lang_Enum.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Enum.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Enum.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Enum.itableBegin = &__TIB_java_lang_Enum.itable[0];
    __TIB_java_lang_Enum.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Enum.vtable[6];

    _STATIC_java_lang_Enum_serialVersionUID = -4300926546619394005;

    __TIB_java_lang_Enum.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Enum.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Enum.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Enum.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Enum.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Enum.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Enum.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Enum.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Enum = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Enum);
    __TIB_java_lang_Enum.clazz = __CLASS_java_lang_Enum;
    __TIB_java_lang_Enum.baseType = JAVA_NULL;
    __CLASS_java_lang_Enum_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Enum);
    __CLASS_java_lang_Enum_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Enum_1ARRAY);
    __CLASS_java_lang_Enum_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Enum_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Enum]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Enum.classInitialized = 1;
}

void __DELETE_java_lang_Enum(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Enum]
    //XMLVM_END_WRAPPER
    // Call the finalizer
    (*(void (*)(JAVA_OBJECT)) ((java_lang_Object*) me)->tib->vtable[XMLVM_VTABLE_IDX_java_lang_Object_finalize_java_lang_Object__])(me);
}

void __INIT_INSTANCE_MEMBERS_java_lang_Enum(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 1 || derivedClassWillRegisterFinalizer);
    ((java_lang_Enum*) me)->fields.java_lang_Enum.name_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_Enum*) me)->fields.java_lang_Enum.ordinal_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Enum]
    //XMLVM_END_WRAPPER
    if (!derivedClassWillRegisterFinalizer) {
        // Tell the GC to finalize us
        XMLVM_FINALIZE(me, __DELETE_java_lang_Enum);
    }
}

JAVA_OBJECT __NEW_java_lang_Enum()
{    XMLVM_CLASS_INIT(java_lang_Enum)
java_lang_Enum* me = (java_lang_Enum*) XMLVM_MALLOC(sizeof(java_lang_Enum));
    me->tib = &__TIB_java_lang_Enum;
    __INIT_INSTANCE_MEMBERS_java_lang_Enum(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Enum]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Enum()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Enum_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Enum)
    return _STATIC_java_lang_Enum_serialVersionUID;
}

void java_lang_Enum_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Enum)
_STATIC_java_lang_Enum_serialVersionUID = v;
}

void java_lang_Enum___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum___INIT____java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Enum", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Enum.java", 49)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Enum.java", 50)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Enum*) _r0.o)->fields.java_lang_Enum.name_ = _r1.o;
    XMLVM_SOURCE_POSITION("Enum.java", 51)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Enum*) _r0.o)->fields.java_lang_Enum.ordinal_ = _r2.i;
    XMLVM_SOURCE_POSITION("Enum.java", 52)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_name__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_name__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "name", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 62)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Enum*) _r1.o)->fields.java_lang_Enum.name_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Enum_ordinal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_ordinal__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "ordinal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 72)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Enum*) _r1.o)->fields.java_lang_Enum.ordinal_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_toString__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 83)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Enum*) _r1.o)->fields.java_lang_Enum.name_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Enum_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Enum", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Enum.java", 98)
    if (_r1.o != _r2.o) goto label4;
    _r0.i = 1;
    label3:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label4:;
    _r0.i = 0;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Enum_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 103)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Enum*) _r2.o)->fields.java_lang_Enum.ordinal_;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_Enum*) _r2.o)->fields.java_lang_Enum.name_;
    if (_r1.o != JAVA_NULL) goto label9;
    _r1.i = 0;
    label7:;
    _r0.i = _r0.i + _r1.i;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_lang_Enum*) _r2.o)->fields.java_lang_Enum.name_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[4])(_r1.o);
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_clone__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "clone", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 117)

    
    // Red class access removed: java.lang.CloneNotSupportedException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.4C"
    _r1.o = xmlvm_create_java_string_from_pool(832);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r1.o);

    
    // Red class access removed: java.lang.CloneNotSupportedException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Enum_compareTo___java_lang_Enum(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_compareTo___java_lang_Enum]
    XMLVM_ENTER_METHOD("java.lang.Enum", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Enum.java", 135)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Enum*) _r2.o)->fields.java_lang_Enum.ordinal_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Enum*) _r3.o)->fields.java_lang_Enum.ordinal_;
    _r0.i = _r0.i - _r1.i;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_getDeclaringClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_getDeclaringClass__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "getDeclaringClass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 145)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r3.o)->tib->vtable[3])(_r3.o);
    XMLVM_SOURCE_POSITION("Enum.java", 146)
    XMLVM_CHECK_NPE(0)
    _r1.o = java_lang_Class_getSuperclass__(_r0.o);
    XMLVM_SOURCE_POSITION("Enum.java", 147)
    XMLVM_CLASS_INIT(java_lang_Enum)
    _r2.o = __CLASS_java_lang_Enum;
    if (_r2.o != _r1.o) goto label13;
    label12:;
    XMLVM_SOURCE_POSITION("Enum.java", 150)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label13:;
    _r0 = _r1;
    goto label12;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_valueOf___java_lang_Class_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_Enum)
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_valueOf___java_lang_Class_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Enum", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = n1;
    _r6.o = n2;
    XMLVM_SOURCE_POSITION("Enum.java", 169)
    if (_r5.o == JAVA_NULL) goto label4;
    if (_r6.o != JAVA_NULL) goto label16;
    label4:;
    XMLVM_SOURCE_POSITION("Enum.java", 171)
    _r0.o = __NEW_java_lang_NullPointerException();
    // "luni.4D"
    _r1.o = xmlvm_create_java_string_from_pool(833);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("Enum.java", 173)
    _r0.o = java_lang_Enum_getValues___java_lang_Class(_r5.o);
    XMLVM_SOURCE_POSITION("Enum.java", 174)
    if (_r0.o != JAVA_NULL) goto label34;
    XMLVM_SOURCE_POSITION("Enum.java", 176)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.4E"
    _r1.o = xmlvm_create_java_string_from_pool(834);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object(_r1.o, _r5.o);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label34:;
    XMLVM_SOURCE_POSITION("Enum.java", 178)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r2.i = 0;
    label36:;
    if (_r2.i < _r1.i) goto label50;
    XMLVM_SOURCE_POSITION("Enum.java", 184)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.4F"
    _r1.o = xmlvm_create_java_string_from_pool(835);
    XMLVM_SOURCE_POSITION("Enum.java", 185)
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_java_lang_Object_java_lang_Object(_r1.o, _r6.o, _r5.o);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label50:;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Enum.java", 179)
    XMLVM_CHECK_NPE(3)
    _r4.o = ((java_lang_Enum*) _r3.o)->fields.java_lang_Enum.name_;
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(4)
    _r4.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[1])(_r4.o, _r6.o);
    if (_r4.i == 0) goto label61;
    XMLVM_SOURCE_POSITION("Enum.java", 180)
    XMLVM_EXIT_METHOD()
    return _r3.o;
    label61:;
    _r2.i = _r2.i + 1;
    goto label36;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Enum_getValues___java_lang_Class(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Enum)
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_getValues___java_lang_Class]
    XMLVM_ENTER_METHOD("java.lang.Enum", "getValues", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r3.o = n1;
    _r2.o = JAVA_NULL;
    XMLVM_TRY_BEGIN(w6397aaac13b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("Enum.java", 196)

    
    // Red class access removed: java.lang.Enum$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.Enum$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedExceptionAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Enum.java", 204)
    _r1.o = JAVA_NULL;
    //java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY[15]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_reflect_Method*) _r0.o)->tib->vtable[15])(_r0.o, _r3.o, _r1.o);
    _r3.o = _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w6397aaac13b1b3)
        XMLVM_CATCH_SPECIFIC(w6397aaac13b1b3,java_lang_Exception,21)
    XMLVM_CATCH_END(w6397aaac13b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w6397aaac13b1b3)
    _r0 = _r3;
    label20:;
    XMLVM_SOURCE_POSITION("Enum.java", 206)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label21:;
    java_lang_Thread* curThread_w6397aaac13b1b9 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w6397aaac13b1b9->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r2;
    goto label20;
    //XMLVM_END_WRAPPER
}

void java_lang_Enum_finalize_java_lang_Enum__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_finalize_java_lang_Enum__]
    XMLVM_ENTER_METHOD("java.lang.Enum", "finalize", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Enum.java", 219)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Enum_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Enum_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Enum", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Enum.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Enum_compareTo___java_lang_Enum(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

