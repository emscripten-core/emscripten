#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"

#include "java_lang_reflect_Member.h"

__TIB_DEFINITION_java_lang_reflect_Member __TIB_java_lang_reflect_Member = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_Member, // classInitializer
    "java.lang.reflect.Member", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    -1, // sizeInstance
    XMLVM_TYPE_INTERFACE};

JAVA_OBJECT __CLASS_java_lang_reflect_Member;
JAVA_OBJECT __CLASS_java_lang_reflect_Member_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Member_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Member_3ARRAY;
static JAVA_INT _STATIC_java_lang_reflect_Member_PUBLIC;
static JAVA_INT _STATIC_java_lang_reflect_Member_DECLARED;

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

void __INIT_java_lang_reflect_Member()
{
    staticInitializerLock(&__TIB_java_lang_reflect_Member);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_Member.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_Member.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_Member);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_Member.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_Member.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_Member.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.Member")
        __INIT_IMPL_java_lang_reflect_Member();
    }
}

void __INIT_IMPL_java_lang_reflect_Member()
{
    __TIB_java_lang_reflect_Member.numInterfaces = 0;
    _STATIC_java_lang_reflect_Member_PUBLIC = 0;
    _STATIC_java_lang_reflect_Member_DECLARED = 1;
    __TIB_java_lang_reflect_Member.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_Member.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    //__TIB_java_lang_reflect_Member.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_Member.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_Member.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);

    __CLASS_java_lang_reflect_Member = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_Member);
    __TIB_java_lang_reflect_Member.clazz = __CLASS_java_lang_reflect_Member;
    __TIB_java_lang_reflect_Member.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_Member_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Member);
    __CLASS_java_lang_reflect_Member_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Member_1ARRAY);
    __CLASS_java_lang_reflect_Member_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Member_2ARRAY);

    __TIB_java_lang_reflect_Member.classInitialized = 1;
}

JAVA_INT java_lang_reflect_Member_GET_PUBLIC()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Member)
    return _STATIC_java_lang_reflect_Member_PUBLIC;
}

void java_lang_reflect_Member_PUT_PUBLIC(JAVA_INT v)
{
    _STATIC_java_lang_reflect_Member_PUBLIC = v;
}

JAVA_INT java_lang_reflect_Member_GET_DECLARED()
{
    XMLVM_CLASS_INIT(java_lang_reflect_Member)
    return _STATIC_java_lang_reflect_Member_DECLARED;
}

void java_lang_reflect_Member_PUT_DECLARED(JAVA_INT v)
{
    _STATIC_java_lang_reflect_Member_DECLARED = v;
}

