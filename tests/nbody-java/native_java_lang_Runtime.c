
#include "xmlvm.h"
#include "java_lang_Runtime.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_LONG java_lang_Runtime_freeMemory__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Runtime_freeMemory__]
#ifndef XMLVM_NO_GC
    return GC_get_free_bytes();
#else
    return 0L;
#endif
    //XMLVM_END_NATIVE
}

void java_lang_Runtime_gc__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Runtime_gc__]
#ifndef XMLVM_NO_GC
    GC_gcollect();
#endif
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_Runtime_totalMemory__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Runtime_totalMemory__]
#ifndef XMLVM_NO_GC
    return GC_get_heap_size();
#else
    return 0L;
#endif
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_java_lang_Runtime()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_java_lang_Runtime_freeMemory__
    __TIB_java_lang_Runtime.vtable[XMLVM_VTABLE_IDX_java_lang_Runtime_freeMemory__] = 
        (VTABLE_PTR) java_lang_Runtime_freeMemory__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Runtime_gc__
    __TIB_java_lang_Runtime.vtable[XMLVM_VTABLE_IDX_java_lang_Runtime_gc__] = 
        (VTABLE_PTR) java_lang_Runtime_gc__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Runtime_totalMemory__
    __TIB_java_lang_Runtime.vtable[XMLVM_VTABLE_IDX_java_lang_Runtime_totalMemory__] = 
        (VTABLE_PTR) java_lang_Runtime_totalMemory__;
#endif
}
