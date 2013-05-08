
#include "xmlvm.h"
#include "java_lang_reflect_Constructor.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_OBJECT java_lang_reflect_Constructor_getTypeParameters__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getTypeParameters__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_toGenericString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_toGenericString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getGenericParameterTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getGenericParameterTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getGenericExceptionTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getGenericExceptionTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getParameterAnnotations__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getParameterAnnotations__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Constructor_isVarArgs__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_isVarArgs__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Constructor_isSynthetic__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_isSynthetic__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_reflect_Constructor_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_equals___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getDeclaringClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getDeclaringClass__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getExceptionTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getExceptionTypes__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Constructor_getModifiers__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getModifiers__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getName__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_getParameterTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_getParameterTypes__]
    java_lang_reflect_Constructor* thiz = (java_lang_reflect_Constructor*) me;
    return thiz->fields.java_lang_reflect_Constructor.parameterTypes_;
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_reflect_Constructor_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_hashCode__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY]
    java_lang_reflect_Constructor* thiz = (java_lang_reflect_Constructor*) me;
    Func_OOO dispatcher = (Func_OOO) thiz->fields.java_lang_reflect_Constructor.address_;
    return (*dispatcher)(me, n1);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_reflect_Constructor_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_reflect_Constructor_toString__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_java_lang_reflect_Constructor()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getTypeParameters__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getTypeParameters__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getTypeParameters__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_toGenericString__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_toGenericString__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_toGenericString__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getGenericParameterTypes__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getGenericParameterTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getGenericParameterTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getGenericExceptionTypes__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getGenericExceptionTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getGenericExceptionTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getParameterAnnotations__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getParameterAnnotations__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getParameterAnnotations__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_isVarArgs__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_isVarArgs__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_isVarArgs__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_isSynthetic__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_isSynthetic__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_isSynthetic__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_equals___java_lang_Object
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_equals___java_lang_Object] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_equals___java_lang_Object;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getDeclaringClass__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getDeclaringClass__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getDeclaringClass__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getExceptionTypes__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getExceptionTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getExceptionTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getModifiers__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getModifiers__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getModifiers__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getName__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getName__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getName__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getParameterTypes__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_getParameterTypes__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_getParameterTypes__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_hashCode__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_hashCode__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_hashCode__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_newInstance___java_lang_Object_1ARRAY;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_toString__
    __TIB_java_lang_reflect_Constructor.vtable[XMLVM_VTABLE_IDX_java_lang_reflect_Constructor_toString__] = 
        (VTABLE_PTR) java_lang_reflect_Constructor_toString__;
#endif
}
