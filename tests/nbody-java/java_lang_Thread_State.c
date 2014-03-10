#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"

#include "java_lang_Thread_State.h"

#define XMLVM_CURRENT_CLASS_NAME Thread_State
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Thread_State

__TIB_DEFINITION_java_lang_Thread_State __TIB_java_lang_Thread_State = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Thread_State, // classInitializer
    "java.lang.Thread$State", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Enum<Ljava/lang/Thread$State;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Enum, // extends
    sizeof(java_lang_Thread_State), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Thread_State;
JAVA_OBJECT __CLASS_java_lang_Thread_State_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_State_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Thread_State_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_lang_Thread_State_NEW;
static JAVA_OBJECT _STATIC_java_lang_Thread_State_RUNNABLE;
static JAVA_OBJECT _STATIC_java_lang_Thread_State_BLOCKED;
static JAVA_OBJECT _STATIC_java_lang_Thread_State_WAITING;
static JAVA_OBJECT _STATIC_java_lang_Thread_State_TIMED_WAITING;
static JAVA_OBJECT _STATIC_java_lang_Thread_State_TERMINATED;
static JAVA_OBJECT _STATIC_java_lang_Thread_State__VALUES;

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

void __INIT_java_lang_Thread_State()
{
    staticInitializerLock(&__TIB_java_lang_Thread_State);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Thread_State.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Thread_State.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Thread_State);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Thread_State.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Thread_State.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Thread_State.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Thread$State")
        __INIT_IMPL_java_lang_Thread_State();
    }
}

void __INIT_IMPL_java_lang_Thread_State()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Enum)
    __TIB_java_lang_Thread_State.newInstanceFunc = __NEW_INSTANCE_java_lang_Thread_State;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Thread_State.vtable, __TIB_java_lang_Enum.vtable, sizeof(__TIB_java_lang_Enum.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_Thread_State.numImplementedInterfaces = 2;
    __TIB_java_lang_Thread_State.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Thread_State.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Thread_State.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Thread_State.itableBegin = &__TIB_java_lang_Thread_State.itable[0];
    __TIB_java_lang_Thread_State.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Thread_State.vtable[6];

    _STATIC_java_lang_Thread_State_NEW = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State_RUNNABLE = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State_BLOCKED = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State_WAITING = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State_TIMED_WAITING = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State_TERMINATED = (java_lang_Thread_State*) JAVA_NULL;
    _STATIC_java_lang_Thread_State__VALUES = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_lang_Thread_State.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Thread_State.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Thread_State.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Thread_State.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Thread_State.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Thread_State.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Thread_State.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Thread_State.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Thread_State = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Thread_State);
    __TIB_java_lang_Thread_State.clazz = __CLASS_java_lang_Thread_State;
    __TIB_java_lang_Thread_State.baseType = JAVA_NULL;
    __CLASS_java_lang_Thread_State_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_State);
    __CLASS_java_lang_Thread_State_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_State_1ARRAY);
    __CLASS_java_lang_Thread_State_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Thread_State_2ARRAY);
    java_lang_Thread_State___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Thread_State]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Thread_State.classInitialized = 1;
}

void __DELETE_java_lang_Thread_State(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Thread_State]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Thread_State(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Enum(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Thread_State]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Thread_State()
{    XMLVM_CLASS_INIT(java_lang_Thread_State)
java_lang_Thread_State* me = (java_lang_Thread_State*) XMLVM_MALLOC(sizeof(java_lang_Thread_State));
    me->tib = &__TIB_java_lang_Thread_State;
    __INIT_INSTANCE_MEMBERS_java_lang_Thread_State(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Thread_State]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Thread_State()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_lang_Thread_State_GET_NEW()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_NEW;
}

void java_lang_Thread_State_PUT_NEW(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_NEW = v;
}

JAVA_OBJECT java_lang_Thread_State_GET_RUNNABLE()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_RUNNABLE;
}

void java_lang_Thread_State_PUT_RUNNABLE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_RUNNABLE = v;
}

JAVA_OBJECT java_lang_Thread_State_GET_BLOCKED()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_BLOCKED;
}

void java_lang_Thread_State_PUT_BLOCKED(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_BLOCKED = v;
}

JAVA_OBJECT java_lang_Thread_State_GET_WAITING()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_WAITING;
}

void java_lang_Thread_State_PUT_WAITING(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_WAITING = v;
}

JAVA_OBJECT java_lang_Thread_State_GET_TIMED_WAITING()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_TIMED_WAITING;
}

void java_lang_Thread_State_PUT_TIMED_WAITING(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_TIMED_WAITING = v;
}

JAVA_OBJECT java_lang_Thread_State_GET_TERMINATED()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State_TERMINATED;
}

void java_lang_Thread_State_PUT_TERMINATED(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State_TERMINATED = v;
}

JAVA_OBJECT java_lang_Thread_State_GET__VALUES()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    return _STATIC_java_lang_Thread_State__VALUES;
}

void java_lang_Thread_State_PUT__VALUES(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
_STATIC_java_lang_Thread_State__VALUES = v;
}

JAVA_OBJECT java_lang_Thread_State_values__()
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_State_values__]
    XMLVM_ENTER_METHOD("java.lang.Thread$State", "values", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Thread.java", 74)
    _r0.o = java_lang_Thread_State_GET__VALUES();
    //java_lang_Thread_State_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->tib->vtable[0])(_r0.o);
    _r0.o = _r0.o;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Thread_State_valueOf___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_State_valueOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Thread$State", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Thread.java", 74)
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    _r0.o = __CLASS_java_lang_Thread_State;
    _r1.o = java_lang_Enum_valueOf___java_lang_Class_java_lang_String(_r0.o, _r1.o);
    _r1.o = _r1.o;
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_State___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_State___INIT____java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.Thread$State", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Thread.java", 74)
    XMLVM_CHECK_NPE(0)
    java_lang_Enum___INIT____java_lang_String_int(_r0.o, _r1.o, _r2.i);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_Thread_State___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Thread_State___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Thread$State", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.i = 4;
    _r6.i = 3;
    _r5.i = 2;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Thread.java", 78)
    _r0.o = __NEW_java_lang_Thread_State();
    // "NEW"
    _r1.o = xmlvm_create_java_string_from_pool(814);
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r3.i);
    java_lang_Thread_State_PUT_NEW( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 82)
    _r0.o = __NEW_java_lang_Thread_State();
    // "RUNNABLE"
    _r1.o = xmlvm_create_java_string_from_pool(815);
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r4.i);
    java_lang_Thread_State_PUT_RUNNABLE( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 86)
    _r0.o = __NEW_java_lang_Thread_State();
    // "BLOCKED"
    _r1.o = xmlvm_create_java_string_from_pool(816);
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r5.i);
    java_lang_Thread_State_PUT_BLOCKED( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 90)
    _r0.o = __NEW_java_lang_Thread_State();
    // "WAITING"
    _r1.o = xmlvm_create_java_string_from_pool(817);
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r6.i);
    java_lang_Thread_State_PUT_WAITING( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 94)
    _r0.o = __NEW_java_lang_Thread_State();
    // "TIMED_WAITING"
    _r1.o = xmlvm_create_java_string_from_pool(818);
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r7.i);
    java_lang_Thread_State_PUT_TIMED_WAITING( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 98)
    _r0.o = __NEW_java_lang_Thread_State();
    // "TERMINATED"
    _r1.o = xmlvm_create_java_string_from_pool(819);
    _r2.i = 5;
    XMLVM_CHECK_NPE(0)
    java_lang_Thread_State___INIT____java_lang_String_int(_r0.o, _r1.o, _r2.i);
    java_lang_Thread_State_PUT_TERMINATED( _r0.o);
    XMLVM_SOURCE_POSITION("Thread.java", 74)
    _r0.i = 6;
    XMLVM_CLASS_INIT(java_lang_Thread_State)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Thread_State, _r0.i);
    _r1.o = java_lang_Thread_State_GET_NEW();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r1.o;
    _r1.o = java_lang_Thread_State_GET_RUNNABLE();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r1.o;
    _r1.o = java_lang_Thread_State_GET_BLOCKED();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r1.o;
    _r1.o = java_lang_Thread_State_GET_WAITING();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r1.o;
    _r1.o = java_lang_Thread_State_GET_TIMED_WAITING();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r1.o;
    _r1.i = 5;
    _r2.o = java_lang_Thread_State_GET_TERMINATED();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    java_lang_Thread_State_PUT__VALUES( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

