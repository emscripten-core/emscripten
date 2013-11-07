
#include "xmlvm.h"
#include "java_lang_reflect_Method.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include "java_lang_Class.h"
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_OBJECT java_lang_reflect_Method_getTypeParameters__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getTypeParameters__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_toGenericString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_toGenericString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getGenericParameterTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getGenericParameterTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getGenericExceptionTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getGenericExceptionTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getGenericReturnType__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getGenericReturnType__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getParameterAnnotations__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getParameterAnnotations__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Method_isVarArgs__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_isVarArgs__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Method_isBridge__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_isBridge__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Method_isSynthetic__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_isSynthetic__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getDefaultValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getDefaultValue__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Method_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_equals___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getDeclaringClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getDeclaringClass__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getExceptionTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getExceptionTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_getName__]
    java_lang_reflect_Method* thiz = me;
    return thiz->fields.java_lang_reflect_Method.name_;
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Method_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_hashCode__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY]
    java_lang_reflect_Method* thiz = me;
    java_lang_Class* declaringClass = thiz->fields.java_lang_reflect_Method.clazz_;
    __TIB_DEFINITION_TEMPLATE* tib = declaringClass->fields.java_lang_Class.tib_;
    Func_OOOO dispatcher = tib->methodDispatcherFunc;
    return dispatcher(me, n1, n2);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Method_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Method_toString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_java_lang_reflect_Method()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getTypeParameters__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getTypeParameters__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getTypeParameters__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_toGenericString__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_toGenericString__] = 
        (VTABLE_PTR) java_lang_reflect_Method_toGenericString__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericParameterTypes__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericParameterTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getGenericParameterTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericExceptionTypes__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericExceptionTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getGenericExceptionTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericReturnType__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getGenericReturnType__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getGenericReturnType__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getParameterAnnotations__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getParameterAnnotations__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getParameterAnnotations__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_isVarArgs__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_isVarArgs__] = 
        (VTABLE_PTR) java_lang_reflect_Method_isVarArgs__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_isBridge__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_isBridge__] = 
        (VTABLE_PTR) java_lang_reflect_Method_isBridge__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_isSynthetic__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_isSynthetic__] = 
        (VTABLE_PTR) java_lang_reflect_Method_isSynthetic__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getDefaultValue__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getDefaultValue__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getDefaultValue__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_equals___java_lang_Object
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_equals___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Method_equals___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getDeclaringClass__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getDeclaringClass__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getDeclaringClass__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getExceptionTypes__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getExceptionTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getExceptionTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_getName__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_getName__] = 
        (VTABLE_PTR) java_lang_reflect_Method_getName__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_hashCode__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_hashCode__] = 
        (VTABLE_PTR) java_lang_reflect_Method_hashCode__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY] = 
        (VTABLE_PTR) java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Method_toString__
    __TIB_java_lang_reflect_Method.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Method_toString__] = 
        (VTABLE_PTR) java_lang_reflect_Method_toString__;
#endif
}
