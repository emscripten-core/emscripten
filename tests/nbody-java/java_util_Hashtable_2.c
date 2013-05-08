#include "xmlvm.h"
#include "java_util_Hashtable.h"

#include "java_util_Hashtable_2.h"

#define XMLVM_CURRENT_CLASS_NAME Hashtable_2
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Hashtable_2

__TIB_DEFINITION_java_util_Hashtable_2 __TIB_java_util_Hashtable_2 = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Hashtable_2, // classInitializer
    "java.util.Hashtable$2", // className
    "java.util", // package
    "java.util.Hashtable", // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/util/Iterator<Ljava/lang/Object;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_Hashtable_2), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Hashtable_2;
JAVA_OBJECT __CLASS_java_util_Hashtable_2_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_2_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Hashtable_2_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION


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

void __INIT_java_util_Hashtable_2()
{
    staticInitializerLock(&__TIB_java_util_Hashtable_2);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Hashtable_2.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Hashtable_2.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Hashtable_2);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Hashtable_2.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Hashtable_2.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Hashtable_2.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Hashtable$2")
        __INIT_IMPL_java_util_Hashtable_2();
    }
}

void __INIT_IMPL_java_util_Hashtable_2()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_Hashtable_2.newInstanceFunc = __NEW_INSTANCE_java_util_Hashtable_2;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Hashtable_2.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_util_Hashtable_2.vtable[6] = (VTABLE_PTR) &java_util_Hashtable_2_hasNext__;
    __TIB_java_util_Hashtable_2.vtable[7] = (VTABLE_PTR) &java_util_Hashtable_2_next__;
    __TIB_java_util_Hashtable_2.vtable[8] = (VTABLE_PTR) &java_util_Hashtable_2_remove__;
    // Initialize interface information
    __TIB_java_util_Hashtable_2.numImplementedInterfaces = 1;
    __TIB_java_util_Hashtable_2.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_util_Iterator)

    __TIB_java_util_Hashtable_2.implementedInterfaces[0][0] = &__TIB_java_util_Iterator;
    // Initialize itable for this class
    __TIB_java_util_Hashtable_2.itableBegin = &__TIB_java_util_Hashtable_2.itable[0];
    __TIB_java_util_Hashtable_2.itable[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__] = __TIB_java_util_Hashtable_2.vtable[6];
    __TIB_java_util_Hashtable_2.itable[XMLVM_ITABLE_IDX_java_util_Iterator_next__] = __TIB_java_util_Hashtable_2.vtable[7];
    __TIB_java_util_Hashtable_2.itable[XMLVM_ITABLE_IDX_java_util_Iterator_remove__] = __TIB_java_util_Hashtable_2.vtable[8];


    __TIB_java_util_Hashtable_2.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Hashtable_2.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Hashtable_2.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Hashtable_2.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Hashtable_2.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Hashtable_2.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Hashtable_2.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Hashtable_2.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Hashtable_2 = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Hashtable_2);
    __TIB_java_util_Hashtable_2.clazz = __CLASS_java_util_Hashtable_2;
    __TIB_java_util_Hashtable_2.baseType = JAVA_NULL;
    __CLASS_java_util_Hashtable_2_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_2);
    __CLASS_java_util_Hashtable_2_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_2_1ARRAY);
    __CLASS_java_util_Hashtable_2_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Hashtable_2_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Hashtable_2]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Hashtable_2.classInitialized = 1;
}

void __DELETE_java_util_Hashtable_2(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Hashtable_2]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Hashtable_2(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Hashtable_2]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Hashtable_2()
{    XMLVM_CLASS_INIT(java_util_Hashtable_2)
java_util_Hashtable_2* me = (java_util_Hashtable_2*) XMLVM_MALLOC(sizeof(java_util_Hashtable_2));
    me->tib = &__TIB_java_util_Hashtable_2;
    __INIT_INSTANCE_MEMBERS_java_util_Hashtable_2(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Hashtable_2]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable_2()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_Hashtable_2();
    java_util_Hashtable_2___INIT___(me);
    return me;
}

void java_util_Hashtable_2___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_2___INIT___]
    XMLVM_ENTER_METHOD("java.util.Hashtable$2", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 70)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Hashtable.java", 1)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Hashtable_2_hasNext__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_2_hasNext__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$2", "hasNext", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 73)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Hashtable_2_next__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_2_next__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$2", "next", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 77)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Hashtable_2_remove__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Hashtable_2_remove__]
    XMLVM_ENTER_METHOD("java.util.Hashtable$2", "remove", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Hashtable.java", 81)

    
    // Red class access removed: java.lang.IllegalStateException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalStateException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

