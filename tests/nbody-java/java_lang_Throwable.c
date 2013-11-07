#include "xmlvm.h"
#include "java_io_PrintStream.h"
#include "java_lang_Class.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_StackTraceElement.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"

#include "java_lang_Throwable.h"

#define XMLVM_CURRENT_CLASS_NAME Throwable
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Throwable

__TIB_DEFINITION_java_lang_Throwable __TIB_java_lang_Throwable = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Throwable, // classInitializer
    "java.lang.Throwable", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Throwable), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Throwable;
JAVA_OBJECT __CLASS_java_lang_Throwable_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Throwable_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Throwable_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Throwable_serialVersionUID;

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

void __INIT_java_lang_Throwable()
{
    staticInitializerLock(&__TIB_java_lang_Throwable);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Throwable.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Throwable.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Throwable);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Throwable.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Throwable.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Throwable.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Throwable")
        __INIT_IMPL_java_lang_Throwable();
    }
}

void __INIT_IMPL_java_lang_Throwable()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Throwable.newInstanceFunc = __NEW_INSTANCE_java_lang_Throwable;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Throwable.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Throwable.vtable[7] = (VTABLE_PTR) &java_lang_Throwable_getMessage__;
    __TIB_java_lang_Throwable.vtable[5] = (VTABLE_PTR) &java_lang_Throwable_toString__;
    __TIB_java_lang_Throwable.vtable[6] = (VTABLE_PTR) &java_lang_Throwable_getCause__;
    xmlvm_init_native_java_lang_Throwable();
    // Initialize interface information
    __TIB_java_lang_Throwable.numImplementedInterfaces = 1;
    __TIB_java_lang_Throwable.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Throwable.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;
    // Initialize itable for this class
    __TIB_java_lang_Throwable.itableBegin = &__TIB_java_lang_Throwable.itable[0];

    _STATIC_java_lang_Throwable_serialVersionUID = -3042686055658047285;

    __TIB_java_lang_Throwable.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Throwable.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Throwable.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Throwable.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Throwable.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Throwable.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Throwable.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Throwable.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Throwable = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Throwable);
    __TIB_java_lang_Throwable.clazz = __CLASS_java_lang_Throwable;
    __TIB_java_lang_Throwable.baseType = JAVA_NULL;
    __CLASS_java_lang_Throwable_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Throwable);
    __CLASS_java_lang_Throwable_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Throwable_1ARRAY);
    __CLASS_java_lang_Throwable_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Throwable_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Throwable]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Throwable.classInitialized = 1;
}

void __DELETE_java_lang_Throwable(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Throwable]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Throwable(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Throwable*) me)->fields.java_lang_Throwable.detailMessage_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_Throwable*) me)->fields.java_lang_Throwable.cause_ = (java_lang_Throwable*) JAVA_NULL;
    ((java_lang_Throwable*) me)->fields.java_lang_Throwable.stackTrace_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Throwable]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Throwable()
{    XMLVM_CLASS_INIT(java_lang_Throwable)
java_lang_Throwable* me = (java_lang_Throwable*) XMLVM_MALLOC(sizeof(java_lang_Throwable));
    me->tib = &__TIB_java_lang_Throwable;
    __INIT_INSTANCE_MEMBERS_java_lang_Throwable(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Throwable]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Throwable()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_Throwable();
    java_lang_Throwable___INIT___(me);
    return me;
}

JAVA_LONG java_lang_Throwable_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Throwable)
    return _STATIC_java_lang_Throwable_serialVersionUID;
}

void java_lang_Throwable_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Throwable)
_STATIC_java_lang_Throwable_serialVersionUID = v;
}

void java_lang_Throwable___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable___INIT___]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 73)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 62)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Throwable*) _r0.o)->fields.java_lang_Throwable.cause_ = _r0.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 74)
    XMLVM_CHECK_NPE(0)
    java_lang_Throwable_fillInStackTrace__(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 75)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Throwable.java", 85)
    XMLVM_CHECK_NPE(0)
    java_lang_Throwable___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 86)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Throwable*) _r0.o)->fields.java_lang_Throwable.detailMessage_ = _r1.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 87)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable___INIT____java_lang_String_java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("Throwable.java", 99)
    XMLVM_CHECK_NPE(0)
    java_lang_Throwable___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 100)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Throwable*) _r0.o)->fields.java_lang_Throwable.detailMessage_ = _r1.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 101)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Throwable*) _r0.o)->fields.java_lang_Throwable.cause_ = _r2.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 102)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable___INIT____java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Throwable.java", 112)
    XMLVM_CHECK_NPE(1)
    java_lang_Throwable___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 113)
    if (_r2.o != JAVA_NULL) goto label11;
    _r0.o = JAVA_NULL;
    label6:;
    XMLVM_CHECK_NPE(1)
    ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.detailMessage_ = _r0.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 114)
    XMLVM_CHECK_NPE(1)
    ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.cause_ = _r2.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 115)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    //java_lang_Throwable_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r2.o)->tib->vtable[5])(_r2.o);
    goto label6;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Throwable_fillInStackTrace__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Throwable_getMessage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_getMessage__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "getMessage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 139)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.detailMessage_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Throwable_getLocalizedMessage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_getLocalizedMessage__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "getLocalizedMessage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 152)
    //java_lang_Throwable_getMessage__[7]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r1.o)->tib->vtable[7])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Throwable_getStackTraceImpl__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Throwable_getStackTrace__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_getStackTrace__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "getStackTrace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 179)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_Throwable_getInternalStackTrace__(_r1.o);
    //java_lang_StackTraceElement_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->tib->vtable[0])(_r0.o);
    _r1.o = _r1.o;
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable_setStackTrace___java_lang_StackTraceElement_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_setStackTrace___java_lang_StackTraceElement_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "setStackTrace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("Throwable.java", 198)
    //java_lang_StackTraceElement_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r5.o)->tib->vtable[0])(_r5.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 199)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r2.i = 0;
    label8:;
    if (_r2.i < _r1.i) goto label13;
    XMLVM_SOURCE_POSITION("Throwable.java", 204)
    XMLVM_CHECK_NPE(4)
    ((java_lang_Throwable*) _r4.o)->fields.java_lang_Throwable.stackTrace_ = _r0.o;
    XMLVM_SOURCE_POSITION("Throwable.java", 205)
    XMLVM_EXIT_METHOD()
    return;
    label13:;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Throwable.java", 200)
    if (_r3.o != JAVA_NULL) goto label23;
    XMLVM_SOURCE_POSITION("Throwable.java", 201)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    _r2.i = _r2.i + 1;
    goto label8;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable_printStackTrace__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_printStackTrace__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "printStackTrace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 212)
    _r0.o = java_lang_System_GET_err();
    XMLVM_CHECK_NPE(1)
    java_lang_Throwable_printStackTrace___java_io_PrintStream(_r1.o, _r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 213)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Throwable_countDuplicates___java_lang_StackTraceElement_1ARRAY_java_lang_StackTraceElement_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_Throwable)
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_countDuplicates___java_lang_StackTraceElement_1ARRAY_java_lang_StackTraceElement_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "countDuplicates", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = n1;
    _r7.o = n2;
    XMLVM_SOURCE_POSITION("Throwable.java", 226)
    _r0.i = 0;
    XMLVM_SOURCE_POSITION("Throwable.java", 227)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    XMLVM_SOURCE_POSITION("Throwable.java", 228)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r5 = _r2;
    _r2 = _r0;
    _r0 = _r5;
    label6:;
    _r0.i = _r0.i + -1;
    if (_r0.i < 0) goto label14;
    _r1.i = _r1.i + -1;
    if (_r1.i >= 0) goto label15;
    label14:;
    XMLVM_SOURCE_POSITION("Throwable.java", 236)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label15:;
    XMLVM_SOURCE_POSITION("Throwable.java", 229)
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r1.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("Throwable.java", 230)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_lang_StackTraceElement_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_StackTraceElement*) _r3.o)->tib->vtable[1])(_r3.o, _r4.o);
    if (_r3.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Throwable.java", 231)
    _r2.i = _r2.i + 1;
    goto label6;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Throwable_getInternalStackTrace__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_getInternalStackTrace__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "getInternalStackTrace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 247)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.stackTrace_;
    if (_r0.o != JAVA_NULL) goto label10;
    XMLVM_SOURCE_POSITION("Throwable.java", 248)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_Throwable_getStackTraceImpl__(_r1.o);
    XMLVM_CHECK_NPE(1)
    ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.stackTrace_ = _r0.o;
    label10:;
    XMLVM_SOURCE_POSITION("Throwable.java", 250)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.stackTrace_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable_printStackTrace___java_io_PrintStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_printStackTrace___java_io_PrintStream]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "printStackTrace", "?")
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
    _r9.o = me;
    _r10.o = n1;
    _r6.i = 0;
    // "\011at "
    _r7.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_SOURCE_POSITION("Throwable.java", 263)
    //java_lang_Throwable_toString__[5]
    XMLVM_CHECK_NPE(9)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r9.o)->tib->vtable[5])(_r9.o);
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_println___java_lang_String(_r10.o, _r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 266)
    XMLVM_CHECK_NPE(9)
    _r0.o = java_lang_Throwable_getInternalStackTrace__(_r9.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 267)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r2 = _r6;
    label16:;
    if (_r2.i < _r1.i) goto label28;
    XMLVM_SOURCE_POSITION("Throwable.java", 272)
    //java_lang_Throwable_getCause__[6]
    XMLVM_CHECK_NPE(9)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r9.o)->tib->vtable[6])(_r9.o);
    _r8 = _r1;
    _r1 = _r0;
    _r0 = _r8;
    label25:;
    XMLVM_SOURCE_POSITION("Throwable.java", 273)
    if (_r0.o != JAVA_NULL) goto label51;
    XMLVM_SOURCE_POSITION("Throwable.java", 287)
    XMLVM_EXIT_METHOD()
    return;
    label28:;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Throwable.java", 268)
    _r4.o = __NEW_java_lang_StringBuilder();
    // "\011at "
    _r5.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT____java_lang_String(_r4.o, _r7.o);
    XMLVM_CHECK_NPE(4)
    _r3.o = java_lang_StringBuilder_append___java_lang_Object(_r4.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_println___java_lang_String(_r10.o, _r3.o);
    _r2.i = _r2.i + 1;
    goto label16;
    label51:;
    XMLVM_SOURCE_POSITION("Throwable.java", 274)
    // "Caused by: "
    _r2.o = xmlvm_create_java_string_from_pool(46);
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_print___java_lang_String(_r10.o, _r2.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 275)
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_println___java_lang_Object(_r10.o, _r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 276)
    XMLVM_CHECK_NPE(0)
    _r2.o = java_lang_Throwable_getInternalStackTrace__(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 277)
    _r1.i = java_lang_Throwable_countDuplicates___java_lang_StackTraceElement_1ARRAY_java_lang_StackTraceElement_1ARRAY(_r2.o, _r1.o);
    _r3 = _r6;
    label68:;
    XMLVM_SOURCE_POSITION("Throwable.java", 278)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r4.i = _r4.i - _r1.i;
    if (_r3.i < _r4.i) goto label104;
    XMLVM_SOURCE_POSITION("Throwable.java", 281)
    if (_r1.i <= 0) goto label98;
    XMLVM_SOURCE_POSITION("Throwable.java", 282)
    _r3.o = __NEW_java_lang_StringBuilder();
    // "\011... "
    _r4.o = xmlvm_create_java_string_from_pool(47);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____java_lang_String(_r3.o, _r4.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_lang_StringBuilder_append___int(_r3.o, _r1.i);
    // " more"
    _r3.o = xmlvm_create_java_string_from_pool(48);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_println___java_lang_String(_r10.o, _r1.o);
    label98:;
    XMLVM_SOURCE_POSITION("Throwable.java", 285)
    //java_lang_Throwable_getCause__[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r0.o)->tib->vtable[6])(_r0.o);
    _r1 = _r2;
    goto label25;
    label104:;
    XMLVM_SOURCE_POSITION("Throwable.java", 279)
    _r4.o = __NEW_java_lang_StringBuilder();
    // "\011at "
    _r5.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT____java_lang_String(_r4.o, _r7.o);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_Object(_r4.o, _r5.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r4.o)->tib->vtable[5])(_r4.o);
    XMLVM_CHECK_NPE(10)
    java_io_PrintStream_println___java_lang_String(_r10.o, _r4.o);
    _r3.i = _r3.i + 1;
    goto label68;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable_printStackTrace___java_io_PrintWriter(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_printStackTrace___java_io_PrintWriter]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "printStackTrace", "?")
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
    _r9.o = me;
    _r10.o = n1;
    _r6.i = 0;
    // "\011at "
    _r7.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_SOURCE_POSITION("Throwable.java", 299)
    //java_lang_Throwable_toString__[5]
    XMLVM_CHECK_NPE(9)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r9.o)->tib->vtable[5])(_r9.o);

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Throwable.java", 302)
    XMLVM_CHECK_NPE(9)
    _r0.o = java_lang_Throwable_getInternalStackTrace__(_r9.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 303)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r2 = _r6;
    label16:;
    if (_r2.i < _r1.i) goto label28;
    XMLVM_SOURCE_POSITION("Throwable.java", 308)
    //java_lang_Throwable_getCause__[6]
    XMLVM_CHECK_NPE(9)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r9.o)->tib->vtable[6])(_r9.o);
    _r8 = _r1;
    _r1 = _r0;
    _r0 = _r8;
    label25:;
    XMLVM_SOURCE_POSITION("Throwable.java", 309)
    if (_r0.o != JAVA_NULL) goto label51;
    XMLVM_SOURCE_POSITION("Throwable.java", 323)
    XMLVM_EXIT_METHOD()
    return;
    label28:;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Throwable.java", 304)
    _r4.o = __NEW_java_lang_StringBuilder();
    // "\011at "
    _r5.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT____java_lang_String(_r4.o, _r7.o);
    XMLVM_CHECK_NPE(4)
    _r3.o = java_lang_StringBuilder_append___java_lang_Object(_r4.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    _r2.i = _r2.i + 1;
    goto label16;
    label51:;
    XMLVM_SOURCE_POSITION("Throwable.java", 310)
    // "Caused by: "
    _r2.o = xmlvm_create_java_string_from_pool(46);

    
    // Red class access removed: java.io.PrintWriter::print
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Throwable.java", 311)

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Throwable.java", 312)
    XMLVM_CHECK_NPE(0)
    _r2.o = java_lang_Throwable_getInternalStackTrace__(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 313)
    _r1.i = java_lang_Throwable_countDuplicates___java_lang_StackTraceElement_1ARRAY_java_lang_StackTraceElement_1ARRAY(_r2.o, _r1.o);
    _r3 = _r6;
    label68:;
    XMLVM_SOURCE_POSITION("Throwable.java", 314)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r4.i = _r4.i - _r1.i;
    if (_r3.i < _r4.i) goto label104;
    XMLVM_SOURCE_POSITION("Throwable.java", 317)
    if (_r1.i <= 0) goto label98;
    XMLVM_SOURCE_POSITION("Throwable.java", 318)
    _r3.o = __NEW_java_lang_StringBuilder();
    // "\011... "
    _r4.o = xmlvm_create_java_string_from_pool(47);
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT____java_lang_String(_r3.o, _r4.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_lang_StringBuilder_append___int(_r3.o, _r1.i);
    // " more"
    _r3.o = xmlvm_create_java_string_from_pool(48);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    label98:;
    XMLVM_SOURCE_POSITION("Throwable.java", 321)
    //java_lang_Throwable_getCause__[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Throwable*) _r0.o)->tib->vtable[6])(_r0.o);
    _r1 = _r2;
    goto label25;
    label104:;
    XMLVM_SOURCE_POSITION("Throwable.java", 315)
    _r4.o = __NEW_java_lang_StringBuilder();
    // "\011at "
    _r5.o = xmlvm_create_java_string_from_pool(45);
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT____java_lang_String(_r4.o, _r7.o);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_Object(_r4.o, _r5.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r4.o)->tib->vtable[5])(_r4.o);

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    _r3.i = _r3.i + 1;
    goto label68;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Throwable_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_toString__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 327)
    XMLVM_CHECK_NPE(5)
    _r0.o = java_lang_Throwable_getLocalizedMessage__(_r5.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 328)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(5)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r5.o)->tib->vtable[3])(_r5.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_Class_getName__(_r1.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 329)
    if (_r0.o != JAVA_NULL) goto label16;
    _r0 = _r1;
    label15:;
    XMLVM_SOURCE_POSITION("Throwable.java", 330)
    XMLVM_SOURCE_POSITION("Throwable.java", 332)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    _r2.o = __NEW_java_lang_StringBuilder();
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r3.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    _r3.i = _r3.i + 2;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r4.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    _r3.i = _r3.i + _r4.i;
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r3.i);
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r2.o, _r1.o);
    // ": "
    _r2.o = xmlvm_create_java_string_from_pool(49);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 333)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r0.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Throwable_initCause___java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_initCause___java_lang_Throwable]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "initCause", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Throwable.java", 349)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w497aaac20b1b5)
    // Begin try
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_Throwable*) _r2.o)->fields.java_lang_Throwable.cause_;
    if (_r0.o != _r2.o) { XMLVM_MEMCPY(curThread_w497aaac20b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w497aaac20b1b5, sizeof(XMLVM_JMP_BUF)); goto label22; };
    XMLVM_SOURCE_POSITION("Throwable.java", 350)
    if (_r3.o == _r2.o) { XMLVM_MEMCPY(curThread_w497aaac20b1b5->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w497aaac20b1b5, sizeof(XMLVM_JMP_BUF)); goto label11; };
    XMLVM_SOURCE_POSITION("Throwable.java", 351)
    XMLVM_CHECK_NPE(2)
    ((java_lang_Throwable*) _r2.o)->fields.java_lang_Throwable.cause_ = _r3.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w497aaac20b1b5)
        XMLVM_CATCH_SPECIFIC(w497aaac20b1b5,java_lang_Object,19)
    XMLVM_CATCH_END(w497aaac20b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w497aaac20b1b5)
    XMLVM_SOURCE_POSITION("Throwable.java", 352)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label11:;
    XMLVM_TRY_BEGIN(w497aaac20b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("Throwable.java", 354)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "Cause cannot be the receiver"
    _r1.o = xmlvm_create_java_string_from_pool(50);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w497aaac20b1c10)
        XMLVM_CATCH_SPECIFIC(w497aaac20b1c10,java_lang_Object,19)
    XMLVM_CATCH_END(w497aaac20b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w497aaac20b1c10)
    label19:;
    java_lang_Thread* curThread_w497aaac20b1c12 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w497aaac20b1c12->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label22:;
    XMLVM_TRY_BEGIN(w497aaac20b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("Throwable.java", 356)

    
    // Red class access removed: java.lang.IllegalStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "Cause already initialized"
    _r1.o = xmlvm_create_java_string_from_pool(51);

    
    // Red class access removed: java.lang.IllegalStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w497aaac20b1c16)
        XMLVM_CATCH_SPECIFIC(w497aaac20b1c16,java_lang_Object,19)
    XMLVM_CATCH_END(w497aaac20b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w497aaac20b1c16)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Throwable_getCause__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_getCause__]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "getCause", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Throwable.java", 366)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.cause_;
    if (_r0.o != _r1.o) goto label6;
    XMLVM_SOURCE_POSITION("Throwable.java", 367)
    _r0.o = JAVA_NULL;
    label5:;
    XMLVM_SOURCE_POSITION("Throwable.java", 369)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label6:;
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_Throwable*) _r1.o)->fields.java_lang_Throwable.cause_;
    goto label5;
    //XMLVM_END_WRAPPER
}

void java_lang_Throwable_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Throwable_writeObject___java_io_ObjectOutputStream]
    XMLVM_ENTER_METHOD("java.lang.Throwable", "writeObject", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Throwable.java", 374)
    XMLVM_CHECK_NPE(0)
    java_lang_Throwable_getInternalStackTrace__(_r0.o);
    XMLVM_SOURCE_POSITION("Throwable.java", 375)

    
    // Red class access removed: java.io.ObjectOutputStream::defaultWriteObject
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Throwable.java", 376)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

